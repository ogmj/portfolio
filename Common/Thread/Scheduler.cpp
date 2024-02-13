#include "Scheduler.h"
#include <vector>
#include "../../network/include/thread/Sync.h"
#include <process.h>
#include <assert.h>
#include "../Util/w32_util.h"
#include "../Console/Console.h"
#include <typeinfo>

#include <memory>
#include "../Util/ServerTime.h"

static void (*s_pfInitFunc)( int ) = NULL;

Scheduler::Scheduler()
{
	m_nInstructionCount = 0;
}

Scheduler::~Scheduler()
{
	DeInit();
}

__declspec( thread ) static volatile bool	s_bIsDestroyer = false;

const __int64 OBJECT_UPDATE_TIME[] =
{
	0x7fffffff,
	600000*(__int64)10000,
	60000*10000,
	5000*10000,
	1000*10000,
	80*10000,
	80*10000,
	0,
};


class ObjectDestroyerInfo
{
public:
	ObjectDestroyerInfo() : destroyLock( "ObjectDestroyerInfo_destroyLock" )
	{
		bReqStop = false;
		bIsFinished = false;
	}

	~ObjectDestroyerInfo()
	{
	}

	std::vector< SchedulerObject* >	vDeleteList;
	std::vector< SchedulerObject* >	vWaitList;
	CriticalSection					destroyLock;

	HANDLE								hThread;
	volatile bool						bReqStop;
	volatile bool						bIsFinished;
};

class SchedulerInfo
{
public:
	SchedulerInfo() : modifyLock( "SchedulerInfo_modifyLock" )
	{
		vObjectList.reserve( 4096 );
		vModifyList.reserve( 512 );

		nObjectCount = 0;
		nInstructionCnt = 0;
		bStop = false;
	}

	std::vector< SchedulerObject* >	vObjectList;
	using SchedulerObjectIt = std::vector< SchedulerObject* >::iterator;
	std::vector< SchedulerObject* >	vModifyList;

	CriticalSection			modifyLock;

	volatile unsigned			nInstructionCnt;
	volatile unsigned			nObjectCount;

	volatile bool				bStop;
	HANDLE						hThread;
	unsigned					nThreadIndex;
};

class _SchedulerImpl
{
public:
	static void SetTimer( SchedulerObject& pObj, __int64 t, unsigned tick  )
	{
		pObj.last_proc_time = t;
		pObj.last_proc_tick = tick;
	}

	static bool IsDeleteable( SchedulerObject & pPtr )
	{
		if( !pPtr.IsDeleteable() )
			return false;
		if( !pPtr.SchedulerObject::IsDeleteable() ) 
			return false;
		return true;

	}
	static bool ProcDelete(  SchedulerObject & pPtr )		{	return pPtr.ProcDelete();		}
};

bool debug_exist( std::vector< SchedulerObject* > & v, SchedulerObject * p )
{
	for( unsigned i = 0; i < v.size(); i++ )
	{
		if( v[i]->GetPriorityQueueIndex() < 0 )
		{
			return true;
		}
		if( v[i] == p )
		{
			return true;
		}
	}
	return false;
}

bool debug_index( std::vector< SchedulerObject* > & v )
{
	size_t sz = v.size();

	for( unsigned i = 0; i < sz; i++ )
	{
		if( v[i]->GetPriorityQueueIndex() < 0 )
		{
			return true;
		}
		if( v[i]->GetPriorityQueueIndex() != i )
		{
			return true;
		}
	}
	return false;
}

void Scheduler::SetObjectPriority( SchedulerObject & obj, SchedulerObject::OBJECT_PRIORITY priority )
{
	unsigned target_thread = 0;

	obj.EnterSpinLock();
	if( obj.thread_index != -1 )			target_thread = obj.thread_index;
	else if( obj.pending_thread_index != -1 )	target_thread = obj.pending_thread_index;
	obj.LeaveSpinLock();

	std::shared_ptr<SchedulerInfo> pNewThread = m_vec_pSchedule[target_thread];

	// EnterSpinLock 하기 이전에 Lock을 해야한다.
	THREAD_SYNCHRONIZE( pNewThread->modifyLock );
	SPINLOCK_SYNCHRONIZE( obj.lock );

	// 삭제요청된 객체라면 IDLE이 되도록 유도해야 한다.
	if( obj.bIsDeleteRequested )
	{
		// 이미 idle 요청 중이면 무시
		if( obj.GetFinalPriority() == SchedulerObject::UPDATE_PRIORITY_IDLE )	return;

		// idle 요청이 아니라면 무시
		if( priority != SchedulerObject::UPDATE_PRIORITY_IDLE )					return;

		// 이미 idel이면 무시
		if( obj.GetPriority() == SchedulerObject::UPDATE_PRIORITY_IDLE )			return;
	}

	// 이미 해당 상태로 진행중이면
	if( obj.GetFinalPriority() == priority ) return;

	// pending 된적이 있다면
	if( obj.GetPendingPriority() != SchedulerObject::UPDATE_PRIORITY_NULL )
	{
		// 현재 pending 된 mode 라면..
		if( obj.priority == priority )
		{
			// { pending queue 에서 제거
			pNewThread->vObjectList.back()->pending_priority_queue_index = obj.pending_priority_queue_index;
			pNewThread->vModifyList[ obj.pending_priority_queue_index ] = pNewThread->vObjectList.back();
			obj.pending_priority_queue_index = -1;
			// }

			// pending 취소
			obj.pending_priority		= SchedulerObject::UPDATE_PRIORITY_NULL;
			obj.pending_thread_index	= -1;
			return;
		}

		// pending 상태만 변경
		obj.pending_priority		= priority;
		obj.pending_thread_index	= target_thread;

		return;
	}

	// 놀던 녀셕이면 신규등록
	if( ( obj.GetPriority() == SchedulerObject::UPDATE_PRIORITY_IDLE && priority != SchedulerObject::UPDATE_PRIORITY_IDLE ) ||
		( obj.GetPriority() != SchedulerObject::UPDATE_PRIORITY_IDLE && priority == SchedulerObject::UPDATE_PRIORITY_IDLE ) )
	{
		pNewThread->vModifyList.emplace_back( &obj);

		obj.pending_priority_queue_index = pNewThread->vModifyList.size() - 1;
		obj.pending_thread_index = target_thread;
		obj.pending_priority = priority;

		return;
	}

	// 아니면 priority만 슬쩍 변경
	obj.priority = priority;
}

unsigned __stdcall ObjectDestroyer( std::shared_ptr<ObjectDestroyerInfo> pInfo)
{
	s_bIsDestroyer = true;

	int nDestroyedCount = 0;
	int nDestroyerLoopCount = 0;

	unsigned nProcTime = 1000;
	DWORD dwPrevTickCount = GetTickCount();
	DWORD dwCurrTickCount = 0;
	std::vector< SchedulerObject* >::iterator it;

	while( true )
	{
		++nDestroyerLoopCount;

		pInfo->destroyLock.Lock();

		if( pInfo->bReqStop && pInfo->vDeleteList.empty() )
		{
			pInfo->destroyLock.UnLock();
			break;
		}

		// { 이 리스트 순회 도중 vWaitList에 push_back을 제외한 어떠한 시도도 하면 안됨.
		for( it = pInfo->vDeleteList.begin(); it < pInfo->vDeleteList.end(); ++it )
		{
			(*it)->EnterSpinLock();

			if( !pInfo->bReqStop && !_SchedulerImpl::IsDeleteable( *(*it) ) )
			{
				(*it)->LeaveSpinLock();
				pInfo->vWaitList.push_back( *it );
				continue;
			}

			++nDestroyedCount;
			(*it)->Disable();
			(*it)->LeaveSpinLock();

			_SchedulerImpl::ProcDelete( *(*it) );
		}
		pInfo->vDeleteList.erase( pInfo->vDeleteList.begin(), pInfo->vDeleteList.end() );
		// }

		if( !pInfo->vWaitList.empty() )
		{
			pInfo->vDeleteList.swap( pInfo->vWaitList );
		}
		pInfo->destroyLock.UnLock();

		dwCurrTickCount = GetTickCount();

		if( dwCurrTickCount - dwPrevTickCount < nProcTime )
		{
			Sleep( nProcTime - ( dwCurrTickCount - dwPrevTickCount ) );
		}

		dwPrevTickCount = GetTickCount();
	}

	pInfo->bIsFinished = true;

	return 0;
}

unsigned __stdcall SchedulerEngine(std::shared_ptr<SchedulerInfo> pInfo)
{
	DWORD	nLoopDelayCount;
	DWORD	nTmp32;
	__int64 nTmp64;
	volatile unsigned & tick = pInfo->nInstructionCnt;

	SchedulerInfo::SchedulerObjectIt it;

	if( s_pfInitFunc )
	{
		s_pfInitFunc( pInfo->nThreadIndex );
	}

	while( !pInfo->bStop )
	{
		tick++;

		nLoopDelayCount = timeGetTime();

		// process
		for( it = pInfo->vObjectList.begin(); it != pInfo->vObjectList.end() && !pInfo->bStop; it++ )
		{
			SchedulerObject* pObj = (*it);
			if( !(*it)->IsEnable() ) continue;

			// DEBUG
			if( pInfo->vObjectList[ pObj->GetPriorityQueueIndex() ] != pObj ) assert( 0 );

			// 이미 수행된 녀석이면 스킵
			if( pObj->GetLastProcTick() == tick ) continue;

			if( pObj->bIsDeleteRequested ) continue;

			// { 이전 수행 시간과의 차이가 OBJECT_UPDATE_TIME 만큼 있을때만 process
			nTmp64 = getElapsedPicoSecond();

			if( pObj->GetLastProcTime() + OBJECT_UPDATE_TIME[ pObj->GetPriority() ] < nTmp64 )
			{
				_SchedulerImpl::SetTimer( *pObj, nTmp64, tick );

				DWORD dwTm = timeGetTime();
				pObj->onProcess( pInfo->nThreadIndex );
				DWORD dwTm2 = timeGetTime();

				if( (dwTm2 - dwTm) > 2000 )
				{
					_cprint( "SCHEDULER PROC : %s %dms\n",  typeid( *pObj ).name(), dwTm2 - dwTm );
				}
			}
		}

		SchedulerObject *pObj;
		{
			// modify
			THREAD_SYNCHRONIZE( pInfo->modifyLock );

			for(it = pInfo->vModifyList.begin(); it != pInfo->vModifyList.end(); it++ )
			{
				pObj = (*it);

				// lock object
				SPINLOCK_SYNCHRONIZE( pObj->lock );

				// pending 되었던 정보가 제거된 것임
				if( pObj->pending_thread_index == -1 ) continue;
				
				// 쓰레드 ID가 일치 하지 않으면 에러임.
				if( pObj->priority != SchedulerObject::UPDATE_PRIORITY_IDLE && pObj->thread_index != pInfo->nThreadIndex )
				{
					assert( 0 );
					continue;
				}

				// if current thread's priority queue doesn't own this obj, skip it.
				if( pObj->pending_thread_index != pInfo->nThreadIndex )
				{
					assert( 0 );
					continue;
				}

				// 이 상황이 된다면 논리적 에러가 있는것임.
				if( pObj->GetPendingPriority() == SchedulerObject::UPDATE_PRIORITY_NULL )
				{
					assert( 0 );
					continue;
				}

				// 제거되어야할 녀석일 경우
				if( pObj->GetPendingPriority() == SchedulerObject::UPDATE_PRIORITY_IDLE )
				{
					if( pObj->GetPriority() != SchedulerObject::UPDATE_PRIORITY_IDLE )
					{
						// move last object to here
						if(pObj != pInfo->vObjectList.back() )
						{
							pInfo->vObjectList.back()->priority_queue_index = pObj->GetPriorityQueueIndex();
							pInfo->vObjectList[ pObj->GetPriorityQueueIndex() ] = pInfo->vObjectList.back();
						}

						// pop back last object
						pInfo->vObjectList.pop_back();

						// idle 상태로..
						pObj->priority				= SchedulerObject::UPDATE_PRIORITY_IDLE;
						pObj->thread_index			= -1;
						pObj->priority_queue_index	= -1;

						--pInfo->nObjectCount;
					}
					else
					{
						// 이 상황이라면 논리적 에러가 있는 것임.
						assert( 0 );
					}
				}
				else
				{
					pObj->thread_index	= pInfo->nThreadIndex;

					if( pObj->priority == SchedulerObject::UPDATE_PRIORITY_IDLE )
					{
						pInfo->vObjectList.emplace_back( pObj );
						pObj->priority_queue_index = pInfo->vObjectList.size() - 1;

						// priority 설정
						pObj->priority		= pObj->pending_priority;

						++pInfo->nObjectCount;

					}
				}

				// pending된 정보를 지움
				pObj->pending_priority		= SchedulerObject::UPDATE_PRIORITY_NULL;
				pObj->pending_thread_index	= -1;
			}

			pInfo->vModifyList.erase( pInfo->vModifyList.begin(), pInfo->vModifyList.end() );
		}

		nTmp32 = timeGetTime();
		if( nLoopDelayCount + OBJECT_UPDATE_TIME[ SchedulerObject::UPDATE_PRIORITY_HIGHEST ]/10000 > nTmp32 )
		{
			Sleep( (DWORD)( nLoopDelayCount + OBJECT_UPDATE_TIME[ SchedulerObject::UPDATE_PRIORITY_HIGHEST ]/10000 - nTmp32 ) );
		}
	}

	pInfo->bStop = false;

	return 0;
}

bool Scheduler::Init( unsigned thread_count, void (*init_func)( int ) )
{	
	if (m_t_vec_ScheduleMng.empty() == false)
	{
		return false;
	}
	
	m_nThreadCount = thread_count;
	
	s_pfInitFunc = init_func;

	for( unsigned i = 0; i < m_nThreadCount; i++ )
	{	
		std::shared_ptr<SchedulerInfo> pSchedulerInfo = std::make_shared<SchedulerInfo>();
		std::shared_ptr<std::thread> t = std::make_shared<std::thread>(SchedulerEngine, pSchedulerInfo);

		pSchedulerInfo->nThreadIndex = i;
		pSchedulerInfo->hThread = t->native_handle();
		m_vec_pSchedule.emplace_back(pSchedulerInfo);
		m_t_vec_ScheduleMng.emplace_back(t);
	}

	m_pObjectDestroyer = std::make_shared<ObjectDestroyerInfo>();
	m_t_ObjectDestoryerMng = std::make_shared<std::thread>(::ObjectDestroyer, m_pObjectDestroyer);

	return true;
}

bool Scheduler::DeInit()
{
	if (m_t_vec_ScheduleMng.empty())
		return false;

	for (auto& i : m_vec_pSchedule)
	{
		if (i->hThread)
		{
			i->bStop = true;
		}
	}
	// 쓰레드가 끝나기를 기다린다.
	for (auto& i : m_vec_pSchedule)
	{
		while (i->bStop)
		{
			Sleep(50);
		}
	}
	m_vec_pSchedule.clear();
	for (auto &i: m_t_vec_ScheduleMng)
	{
		i->join();
	}
	m_t_vec_ScheduleMng.clear();

	m_pObjectDestroyer->bReqStop = true;
	while (!m_pObjectDestroyer->bIsFinished)
	{
		Sleep(50);
	}
	m_t_ObjectDestoryerMng->join();

	return true;
}

void Scheduler::DeleteObject( SchedulerObject & obj )
{
	m_pObjectDestroyer->destroyLock.Lock();
	assert( !obj.bIsDeleteRequested );
	obj.bIsDeleteRequested = true;

	SetObjectPriority( obj, SchedulerObject::UPDATE_PRIORITY_IDLE );

	if( s_bIsDestroyer )
	{
		m_pObjectDestroyer->vWaitList.push_back( &obj );
	}
	else
	{
		m_pObjectDestroyer->vDeleteList.push_back( &obj );
	}
	m_pObjectDestroyer->destroyLock.UnLock();
}
