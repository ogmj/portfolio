
#include "Player.h"
#include "ExternClass.h"
#include "JhcServerNetworkManager.h"
#include "../Common/Network/Message.h"
#include "../Common/Console/Console.h"
#include <string>


JhcServerNetworkManager::JhcServerNetworkManager(void)
{
}

JhcServerNetworkManager::~JhcServerNetworkManager(void)
{
}

void JhcServerNetworkManager::onMsg( struct Message* pMsg, class Player* pPlayer, bool client_call )
{
	
	//_ctprint("%d receive\n", pMsg->msg );
	pPlayer->SetLastPacketTick(g_GlobalTime.GetTime());

	switch( pMsg->msg )
	{
		case CS_PING:
			onPing( static_cast< CS_Ping* >( pMsg ), pPlayer );
			break;
		case CS_ECHO:
			onEcho(static_cast<CS_Echo*>(pMsg), pPlayer);
			break;
		default:
			break;
	}
}
void JhcServerNetworkManager::onPing( struct CS_Ping* pMsg, class Player* pPlayer )
{
	PERF("onPing");
	SC_Ping msg;
	msg.timestamp = pMsg->timestamp;
	pPlayer->Send( &msg );
}

void JhcServerNetworkManager::onEcho(struct CS_Echo* pMsg, class Player* pPlayer)
{
	PERF("onEcho");
	_ctprint("receive echo %s\n", pMsg->echo_message);
	SC_Echo msg;
	strncpy_s(msg.echo_message, sizeof(msg.echo_message), pMsg->echo_message, sizeof(pMsg->echo_message));
	pPlayer->Send(&msg);
}

