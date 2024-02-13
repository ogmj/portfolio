#pragma once

//#include "../../lib/network/Acceptor.h"
#include "../../include/network/Acceptor.h"
#include "../thread/Sync.h"
#include <list>

class IOCPAcceptor : public Acceptor
{
public:
	IOCPAcceptor( int backlog = SOMAXCONN );
	virtual ~IOCPAcceptor();

protected:

	class CUSTOM_OVERLAPPED*	m_pAcceptOverlapped;

	friend class IOCP;

	bool		pendAcceptRequest();
	void		deleteFromPendingList( SOCKET hSocket );

	CriticalSection		m_Lock;
	std::list< Socket* >	m_lstPendingSocket;
};
