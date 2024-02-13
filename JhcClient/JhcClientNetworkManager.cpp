//#include "StatsServer.h"
#include "JhcClientNetworkManager.h"
#include "NetJhcServer.h"

#include "../Common/Network/Message.h"
#include "../Common/Util/Type.h"
#include "../Common/Console/Console.h"


JhcClientNetworkManager::JhcClientNetworkManager(void)
{
}

JhcClientNetworkManager::~JhcClientNetworkManager(void)
{
}

void JhcClientNetworkManager::onMsg( struct Message* pMsg, class NetJhcServer* pJhcServer)
{	
	//_cprint("%d receive\n", pMsg->msg );
	
	switch( pMsg->msg )
	{
		case SC_PING:
			onPing( static_cast< SC_Ping* >( pMsg ), pJhcServer);
			break;
		case SC_ECHO:
			onEcho( static_cast<SC_Echo*>(pMsg), pJhcServer);
			break;

		default:
			break;
	}
}
void JhcClientNetworkManager::onPing( struct SC_Ping* pMsg, class NetJhcServer* pJhcServer)
{
	CS_Ping msg;
	msg.timestamp = pMsg->timestamp;
	pJhcServer->Send(&msg);
}

void JhcClientNetworkManager::onEcho(struct SC_Echo* pMsg, class NetJhcServer* pJhcServer)
{
	_ctprint("return echo %s\n", pMsg->echo_message);
}

