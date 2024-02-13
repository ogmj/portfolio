#pragma once

#include "../Network/Connection.h"
#include "Datagram.h"

class NetworkEventReceiver
{
public:
	NetworkEventReceiver() {};
	virtual ~NetworkEventReceiver() {};
	virtual Connection* createConnection( const Socket & socket )											{ return NULL; }
	virtual Connection* createConnection( int nID, class Acceptor * pAcceptor, const Socket & socket )	{ return NULL; }
	virtual bool onAccept( int nID, class Acceptor * pAcceptor, class Connection* pConnection )		{ return true; }
	virtual void onAccepted( int nID, class Connection* pConnection )									{}
	virtual void onConnect( int nID, class Connection* pConnection )										{}
	virtual void onCantConnect( int nID, class Connection* pConnection )									{}
	virtual void onRead( int nID, class Connection* pConnection )										{}
	virtual void onRead( int nID, class Datagram * pDatagram, unsigned int size )							{}
	virtual void onWrite( int nID, class Connection* pConnection )										{}
	virtual void onWrite( int nID, class Datagram * pDatagram, unsigned int size )						{}
	virtual void onDisconnect( int nID, class Connection* pConnection )									{}
	virtual void onError( const char * szError )															{}
};


class NetworkEvent
{
public:
	NetworkEvent(NetworkEventReceiver* pReceiver);
	virtual ~NetworkEvent();

	virtual bool AddObject( BaseObject * pObj )			= 0;
	virtual bool DelObject( BaseObject * pObj )			= 0;

protected:
	NetworkEventReceiver * m_pReceiver;
};
