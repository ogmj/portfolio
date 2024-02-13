#pragma once

#include "../Util/w32_util.h"
#include <map>

/*
	----------------------------+-------------+
	Priority                    | time        |
	----------------------------+-------------+
	UPDATE_PRIORITY_NOHIGH		|  200ms	  |
	UPDATE_PRIORITY_HIGHEST		|  80 ms	  |
	UPDATE_PRIORITY_HIGH		|   1 second  |
	UPDATE_PRIORITY_NORMAL		|   5 second  | 
	UPDATE_PRIORITY_LOW			|   1 minute  |
	UPDATE_PRIORITY_LOWEST		|  10 minute  |	
	UPDATE_PRIORITY_IDLE		|     no proc |
*/


class SchedulerObject
{
public:
	enum OBJECT_PRIORITY
	{
		UPDATE_PRIORITY_NULL		= -1,
		UPDATE_PRIORITY_IDLE		= 0,
		UPDATE_PRIORITY_LOWEST,
		UPDATE_PRIORITY_LOW,
		UPDATE_PRIORITY_NORMAL,
		UPDATE_PRIORITY_HIGH,
		UPDATE_PRIORITY_HIGHEST,
		UPDATE_PRIORITY_NOHIGH,
		UPDATE_PRIORITY_DUMMY,	// enum의 마지막은 항상 UPDATE_PRIORITY_DUMMY 여야함.
	};

	SchedulerObject() : priority( UPDATE_PRIORITY_IDLE ), priority_queue_index( -1 ), pending_priority_queue_index( -1 ), pending_priority( UPDATE_PRIORITY_NULL ), pending_thread_index( -1 ), m_bIsEnable( true ), thread_index( -1 ), bIsDeleteRequested( false ), last_proc_time( 0 ), last_proc_tick( 0 ), isConcurrent( false )
	{
		spinlock_init( &lock );
	}
	
	virtual void					onProcess( int nThreadIdx )	{};

	inline bool						IsEnable() const		{ return m_bIsEnable; }
	inline void						Disable()				{ m_bIsEnable = false; }

	inline bool						IsDeleteRequested() const	{ return bIsDeleteRequested; }
	
	inline
	volatile OBJECT_PRIORITY		GetPriority() const				{ return priority;	};

	inline volatile size_t			GetPriorityQueueIndex() const	{ return priority_queue_index; }
	inline volatile int				GetPendingPriority() const		{ return pending_priority; }
	inline volatile int				GetFinalPriority() const
	{
		return ( pending_priority == UPDATE_PRIORITY_NULL ? priority : pending_priority );
	}

	inline void						EnterSpinLock()					{ spinlock_enter( &lock ); }
	inline void						LeaveSpinLock()					{ spinlock_leave( &lock ); }

	inline volatile __int64			GetLastProcTime() const			{ return last_proc_time;	 }
	inline volatile unsigned		GetLastProcTick() const			{ return last_proc_tick;	 }


	volatile bool					bIsDeleteRequested;
	bool							isConcurrent;

	// 시간
	volatile __int64				last_proc_time;
	volatile unsigned				last_proc_tick;
	volatile int					lock;

	volatile OBJECT_PRIORITY		pending_priority;		// 변경되고자 하는 priority	
	volatile int					pending_thread_index;
	volatile size_t					pending_priority_queue_index;	// priority queue 에서의 위치
	
	volatile OBJECT_PRIORITY		priority;				// 현재 priority
	volatile int					thread_index;
	volatile size_t					priority_queue_index;	// priority queue에서의 위치
	
protected:

	virtual bool				ProcDelete()			{ return false;				}

	virtual bool IsDeleteable()
	{
		return  ( pending_priority == UPDATE_PRIORITY_NULL && priority == UPDATE_PRIORITY_IDLE ) ;
	}

	virtual ~SchedulerObject()
	{
	}

private:

	friend class					_SchedulerImpl;

	volatile bool					m_bIsEnable;
};
