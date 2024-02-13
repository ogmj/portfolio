#pragma once

#include "../Network/Connection.h"
#include "../Thread/Sync.h"
#include "IOCPStruct.h"
#include <list>

// send 시 최대 pending 가능한 패킷 수.
// 이 갯수 이상보다 많은 write 가 일어날 경우
// 이후의 것은 queue 에 쌓인 후 이전 것이 완료된 후에 처리된다.
const int MAX_PENDING_PACKET = 1; 

class IOCPConnection : public StreamSocketConnection
{
public:
	IOCPConnection( class OverlappedAllocator *pAllocator, bool bUseCipher = false, const char* szCipherSendkey = NULL, const char* szCipherRecvkey = NULL );
	IOCPConnection( class OverlappedAllocator *pAllocator, Socket sock, bool bUseCipher = false, const char* szCipherSendkey = NULL, const char* szCipherRecvkey = NULL );
	virtual ~IOCPConnection();

	void ClearCipher();

	bool Close();	   //전송중인 버퍼를 무시하고 접속 종료
	void AsyncClose(); //이 메소드 호출이전의 전송중인 버퍼를 모두 전송후 종료
	
	void SetCipher( const char* szCipherSendkey, const char* szCipherRecvkey );

	volatile bool CheckDisconnectSignal()		{ return m_bDisconnectSignal;		}
	volatile LONG GetPendingQueryCount()		{ return m_nPendingQueryCount;		}
	volatile LONG GetPendingRecvQueryCount()	{ return m_nPendingRecvQueryCount;	}
	volatile LONG GetPendingSendQueryCount()	{ return m_nPendingSendQueryCount;	}

	bool Connect( const Addr& addr );
	bool SyncConnect( const Addr& addr );
	
	int	 Write( const void* szBuf, size_t nLen );

	virtual int	 Read( void* szBuf, size_t nLen );
	virtual int	 Peek( void* szBuf, size_t nLen );

	virtual const char* GetBuf();
	virtual int	 Size();

	void		 IncVar();
	void		 DecVar();
	volatile int GetVar();

	volatile int m_nVarCount;
	void		 IncVarCount();
	volatile int GetVarCount();
	void		 SetMaxWaitBufLen(DWORD dwVal);
	DWORD   	 GetMaxWaitBufLen();
	int  DEBUG_GetFlag();
	int  DEBUG_GetLastError();

protected:

	//소켓 옵션의 커스트 마이징을 위한 virtual 함수
	virtual void onInitSocket();

	// 이하 함수들은 구현을 위해 IOCP 에서 호출하는 것들.
	// 수정시 IOCPConnection 의 lock 전략을 충분히 이해할 것..
	bool onConnectCompletionEvent( bool bFlag );
	void onSendCompletionEvent( int nSize );
	void onRecvCompletionEvent( int nSize );
	void decreaseQueryCount();
	void onConnect( void *pBuf );
	void onDisconnect( int nFlag );

	bool pendRecvRequest( bool bDecreaseQueryCount = false );

	friend class IOCP;

private:

	// 이하는 절대 외부에서 호출/사용 금지.

	// 공통적인 부분들의 초기화
	void	init( OverlappedAllocator *pAllocator, bool bUseCipher, const char* szCipherSendKey, const char* szCipherRecvKey );

	// Send Queue 에 쌓인 내용중 WriteFile() 되지 않는 내용이 있다면 호출
	bool	procWriteFile();

	int m_nFlag;
	int m_nLastError;

	// 현재 WriteFile() 되었으나 I/O 완료 통지가 되지 않은 갯수
	volatile int	 m_bSendPending;
	
	// Send Queue 에 쌓인 내용중 현재 전송된 크기.
	volatile int	 m_nSendIndicator;

	volatile LONG	 m_nPendingQueryCount;
	volatile LONG	 m_nPendingRecvQueryCount;
	volatile LONG	 m_nPendingSendQueryCount;

	struct IQueue	*m_pSendQueue;
	struct IQueue	*m_pWaitQueue;
	struct IQueue	*m_pRecvQueue;

	class CUSTOM_OVERLAPPED*	m_pSendOverlapped;
	class CUSTOM_OVERLAPPED*	m_pRecvOverlapped;
	class CUSTOM_OVERLAPPED*	m_pConnectOverlapped;

	//critical section 이어야함. (같은 스레드가 재진입 하기때문)
	CriticalSection	m_sendCS;
	CriticalSection	m_recvCS;
	CriticalSection	m_closeCS;
	
	WSABUF				m_RecvWSABUF;
	DWORD				m_dwRecvFlag;
	WSABUF				m_SendWSABUF;
	DWORD				m_dwSendFlag;
	DWORD				m_dwMaxWaitBufLen;

	HANDLE m_hIOCP;

	volatile bool		m_bAsyncCloseSignal;
	volatile bool		m_bIsPostedDisconnectEvent;
	volatile bool 		m_bDisconnectSignal;

	bool				m_bIsUsingCipher;

	OverlappedAllocator *m_pAllocator;

	struct ICipher *m_pSendCipher;
	struct ICipher *m_pRecvCipher;

	volatile int		m_nVar;	
};

class IOCPConnectionCloser
{
public:

	struct DeleteHandler
	{
		virtual void onDelete( IOCPConnection* pConn ) = 0;
	};

	IOCPConnectionCloser( DeleteHandler* pDeleter = NULL );
	~IOCPConnectionCloser();

	void DeInit();

	void Add( IOCPConnection* pConn );
	bool Has( IOCPConnection* pConn );

	void Process();

private:

	std::list< IOCPConnection* > m_closingConns;
	CriticalSection m_cs;

	DeleteHandler* m_pDel;
};

extern volatile int							g_nConnectionCount;

struct IOCP_STATISTICS
{
	static int GetCountSocketConnection()
	{
		return g_nConnectionCount;
	}
};
