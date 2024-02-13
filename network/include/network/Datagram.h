#pragma once

#include "Addr.h"
#include "SocketObject.h"

class Datagram : public BaseObject
{
public:
	Datagram() {};
	~Datagram() {}

	virtual bool Open(const Addr& addr) = 0;
	virtual bool Close() = 0;
	virtual bool IsOpened() = 0;

	virtual void GetMyAddress(Addr&addr) = 0;
	virtual const Addr& GetMyAddress() = 0;
};

class SocketDatagram : public Datagram, public SocketObject
{
public:
	SocketDatagram(bool bUseOverlapped = false);
	SocketDatagram(Socket sock);
	~SocketDatagram();

	virtual bool Open(const Addr& addr) = 0;
	virtual bool Close() = 0;
	virtual bool IsOpened();

	virtual void GetMyAddress(Addr& addr);
	virtual const Addr& GetMyAddress();

	virtual int SendTo(const Addr& addr, const void *buf, unsigned len) = 0;
	virtual int RecvFrom(Addr& addr, void *buf, unsigned len) = 0;

protected:
	Addr m_MyAddr;
	bool m_bIsOpened;
};
