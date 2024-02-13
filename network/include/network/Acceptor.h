#pragma once

#include "Addr.h"
#include "SocketObject.h"

class Connection;

class Acceptor : public SocketObject, public BaseObject
{
public:
	Acceptor( int backlog = SOMAXCONN ) : BaseObject( OBJ_ACCEPTOR )
	{
		m_socket.CreateStreamSocket( true );
		m_bIsStarted = false;
		m_nBacklog = backlog;
	}

	virtual ~Acceptor()
	{
		if( m_bIsStarted ) EndAccept();
	}

	virtual bool	StartAccept( unsigned nPort )
	{
		return StartAccept(Addr( "0.0.0.0", nPort ) );
	}
	virtual bool	StartAccept( const Addr& addr );
	virtual bool	EndAccept();

	Socket			Accept(Addr& peerAddr );

protected:

	volatile bool	m_bIsStarted;
	int				m_nBacklog;
};
