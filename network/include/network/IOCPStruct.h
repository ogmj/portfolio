#pragma once

#include "../../include/thread/Sync.h"


#include <windows.h>
#include <Mswsock.h>

const int	IOCP_BUFFER_SIZE = 16384;	// 한번에 이 이상은 보내지 않음

const char	IOCP_NULL		= 0x01;
const char	IOCP_CONNECT	= 0x02;
const char	IOCP_ACCEPT	= 0x03;
const char	IOCP_DATAGRAM	= 0x04;
const char	IOCP_SEND		= 0x05;
const char	IOCP_RECV		= 0x06;
const char	IOCP_CLOSE		= 0x07;
const char	IOCP_DATAGRAMSEND		= 0x08;
const char	IOCP_DATAGRAMRECV		= 0x09;

const int	IOCP_HEAP_BLOCK	= 1024;

const int	IOCP_EVENT_STOP					= 0xffffffff;
const int	IOCP_EVENT_CONNECTION_CLOSED		= 0xfffffffe;

class OverlappedAllocator;

class CUSTOM_OVERLAPPED : public OVERLAPPED
{
public:
	CUSTOM_OVERLAPPED( HANDLE _hFileHandle = NULL, BaseObject * _pObj = NULL, char *_pBuf = NULL )
	{
		Init( _hFileHandle, _pObj, _pBuf );
	}

	~CUSTOM_OVERLAPPED()
	{
	}

	HANDLE				hFileHandle;
	DWORD				dwSize;
	BaseObject* pObj;
	char				cFlag;
	char* pBuf;
	OverlappedAllocator* pAllocator;

	void Init(HANDLE _hFileHandle = NULL, BaseObject* _pObj = NULL, char* _pBuf = NULL)
	{
		Internal = 0;
		InternalHigh = 0;
		Offset = 0;
		OffsetHigh = 0;
		hEvent = NULL;
		dwSize = 0;
		hFileHandle = _hFileHandle;
		cFlag = IOCP_NULL;
		pObj = _pObj;
		pBuf = _pBuf;
		pAllocator = 0;
	}
};

class OverlappedAllocator : public Sync
{
public:
	OverlappedAllocator();
	virtual ~OverlappedAllocator();

	virtual CUSTOM_OVERLAPPED* allocOverlapped();
	virtual void freeOverlapped(CUSTOM_OVERLAPPED* ptr );

	virtual void Lock( const char* szFileName = "", int nLineNumber = 0 );
	virtual bool IsLocked();
	virtual void UnLock();

	static OverlappedAllocator* getHeapOverlappedAllocator();
private:
	bool m_bUseHeap;
	class _OverlappedAllocatorData * m_pData;
};