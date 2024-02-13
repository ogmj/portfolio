#pragma once

#include "NetworkEvent.h"

class IOCP : public NetworkEvent
{
public:
	IOCP( NetworkEventReceiver * pReceiver );
	virtual ~IOCP();

	bool Init();
	bool DeInit();

	bool AddObject( BaseObject * pObj );
	bool DelObject( BaseObject * pObj );

	bool StartThreadPool( unsigned nThreadNum, void (*init_func)( int ) = NULL );
	bool IncreaseThreadPool( unsigned nThreadNum );
	bool EndThreadPool();

	class OverlappedAllocator* getOverlappedAllocator();
	class CUSTOM_OVERLAPPED* allocOverlapped();
	void freeOverlapped( class CUSTOM_OVERLAPPED* ptr );

	void Pause();
	void Resume();
	bool IsPaused()					{ return m_bIsPaused; }
	long GetTotalThreadCount()		{ return m_nThreadNum; }
	long GetActiveThreadCount()		{ return m_lActiveThreadCount; }
	long GetInstructionCount()		{ return m_lInstructionCount; }

private:

	static unsigned __stdcall IOCPWorkerThread( void* pArg );

	static bool onAcceptEvent( struct IOCPTAG * pTag, int nThreadNum, class CUSTOM_OVERLAPPED* pOverlapped, bool bIsSuccess = true );
	static bool onConnectionEvent( struct IOCPTAG * pTag, int nThreadNum, class CUSTOM_OVERLAPPED* pOverlapped, int nSize );
	static bool onDatagramEvent( struct IOCPTAG * pTag, int nThreadNum, class CUSTOM_OVERLAPPED* pOverlapped, int nSize );


	struct IOCPTAG * m_pTag;

	volatile bool m_bIsPaused;
	volatile long m_lCurrentThreadCount;
	volatile long m_lActiveThreadCount;
	volatile long m_lInstructionCount;
	unsigned m_nThreadNum;
};
