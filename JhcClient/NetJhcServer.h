#pragma once
#include "Object.h"
#include "../network/include/thread/Sync.h"

class NetJhcServer : public Object
{
public:
	NetJhcServer();
	virtual ~NetJhcServer();
	bool Init(class IOCP* iocp);

	virtual NetJhcServer* ToJhcServer() { return this; }
	void Use();
	bool Connect();
	bool Reconnect();
	bool IsConnect();
	void onDisconnect();

	void Stop();
	void Close();

	void Send(struct Message* pMsg);
	class IOCPConnection* GetConnect()
	{
		return m_pConnection;
	}
protected:

	class IOCPConnection* m_pConnection;
	bool m_bUse;
	CriticalSection m_Lock;
private:
	class IOCP* m_pIOCP;
};