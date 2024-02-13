#pragma once

struct JhcServerNetworkManager
{
public:
	JhcServerNetworkManager(void);
	~JhcServerNetworkManager(void);

	void onMsg( struct Message* pMsg, class Player* pPlayer, bool client_call = true );
private:
	void onPing( struct CS_Ping* pMsg, class Player* pPlayer );
	void onEcho(struct CS_Echo* pMsg, class Player* pPlayer);
};