#pragma once

#include "Addr.h"
#include <winsock2.h>
#include <string>

namespace Util
{
	bool InitNetwork();
	void DeInitNetwork();
	bool QueryDNS(Addr& addr );
	const char* QueryLocalIP(const char* defaultIP);
	bool ConvAddr( const Addr& addr, struct sockaddr_in & addr_in );
	bool ConvAddr( const struct sockaddr_in & addr_in, Addr& addr );
	std::string GetWin32ErrorInfo( int nErrNo );
};
