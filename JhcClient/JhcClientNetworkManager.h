#pragma once

struct JhcClientNetworkManager
{
public:
	JhcClientNetworkManager(void);
	~JhcClientNetworkManager(void);

	void onMsg( struct Message* pMsg, class NetJhcServer* pJhcServer);
private:
	void onPing(struct SC_Ping* pMsg, class NetJhcServer* pJhcServer);
	void onEcho(struct SC_Echo* pMsg, class NetJhcServer* pJhcServer);

};