#pragma once

#include "Addr.h"
#include "SocketObject.h"
#include "Stream.h"

class Connection : public BaseObject
{
public:
	Connection() {}
	virtual ~Connection() {}

	virtual bool	Connect( const Addr& addr )	= 0;
	virtual bool	Close()							= 0;
	virtual bool	IsConnected()					= 0;

	virtual void	GetPeerAddress(Addr& addr )	= 0;
	virtual void	GetMyAddress(Addr& addr )	= 0;

	virtual const Addr&	GetPeerAddress()		= 0;
	virtual const Addr&	GetMyAddress()			= 0;
};

class SocketConnection : public Connection,
						   public SocketObject
{
public:
	SocketConnection() {};
	~SocketConnection() {};
};

class StreamSocketConnection : public SocketConnection,
								 public StreamIntf
{
public:
	typedef void (*PCALLBACK_ERROR)( const char *pInfo, TSOCKET socket_handle, int errCode );

	StreamSocketConnection(bool bIsOverlapped = false);
	StreamSocketConnection(Socket sock);
	virtual ~StreamSocketConnection();
	virtual bool	Connect( const Addr& addr )			= 0;
	virtual bool	Close()									= 0;
	virtual bool	IsConnected();
	virtual bool	IsValid();
	virtual int		Size();
	virtual void			GetPeerAddress(Addr& addr);
	virtual void			GetMyAddress(Addr& addr);
	virtual const Addr&	GetPeerAddress();
	virtual const Addr&	GetMyAddress();
	bool			EnableNoDelay();
	bool			DisableNoDelay();
	void			SetCallbackError(PCALLBACK_ERROR pFunc);
	const PCALLBACK_ERROR	GetCallbackError() const;
protected:

	Addr	m_myAddr;
	Addr	m_peerAddr;

	bool	m_bIsConnected;

	PCALLBACK_ERROR	m_pCallback_Error;
};
