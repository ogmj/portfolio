#pragma once

#include "Socket.h"

/// NULL Object
const int OBJ_NULL			= 0x00;
/// Acceptor
const int OBJ_ACCEPTOR		= 0x01;
/// Conenctor
const int OBJ_CONNECTOR	= 0x02;
/// Connection
const int OBJ_CONNECTION	= 0x03;
/// Datagram
const int OBJ_DATAGRAM		= 0x04;

class BaseObject
{
public:
	BaseObject(int id = OBJ_NULL);
	virtual ~BaseObject();

	void* GetTag();
	void	SetTag(void* tag);

	void	SetLastError(int e);
	int		GetLastError();


	void	SetID(int id);
	int		GetID();

protected:

	int		m_nID;
	int		m_nLastError;
	void *	m_pTag;
};

class SocketObject
{
public:
	SocketObject();
	virtual ~SocketObject();

	const Socket& GetXSocket();
	const TSOCKET	  GetSocketHandle();
	bool			  CloseSocket();
	bool			  IsValidSocket();

protected:
	Socket	m_socket;	
};
