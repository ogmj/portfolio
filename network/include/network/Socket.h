#pragma once

#include <winsock2.h>

using TSOCKET = SOCKET;
#pragma warning(disable:4996)

class Socket
{
public:
	Socket(TSOCKET socket = INVALID_SOCKET);
	~Socket();
	bool IsValidSocket() const;
	bool CreateStreamSocket(bool bIsOverlapped = false);
	bool CreateDatagramSocket(bool bIsOverlapped = false);
	virtual bool Destroy();
	TSOCKET GetSocketHandle() const;
	operator TSOCKET() const;
	void operator =(TSOCKET sock);
	bool operator ==(const TSOCKET& rh);
private:
	TSOCKET m_socket;
};
