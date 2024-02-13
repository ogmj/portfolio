#pragma once
#include "Object.h"
#include "../network/include/network/Addr.h"
#include "ExternClass.h"

//#include "../Common/Util/ServerTime.h"
//#include "../Common/Util/MemoryPool.h"

class Player : public Object, public MemoryPool<Player>
{
public:
	Player(); //test¿ë
	Player(class StreamSocketConnection* pConnection);
	virtual ~Player(void);

	virtual Player * ToPlayer()	{ return this; }
	//virtual void	onDelete();
	virtual bool Proc(unsigned int tm);
	virtual void Send(struct Message* pMsg);

	//static Player* MakePlayer(class StreamSocketConnection* pConnection);
	//void SetIsInGlobalContainer(bool isSet);
	//bool IsInGlobalContainer() const { return m_IsInGlobalContainer; }
	//void SetDisconnectFlag(bool f) { m_isDisconnect = f; }
	//bool IsDisconnect() { return m_isDisconnect; }

	void Disconnect();
	void Init();

	void SetLastPacketTick(SERVER_TIME time)
	{
		m_LastGlobalPacketTick = time;
	}
	SERVER_TIME GetLastPacketTick()
	{
		return m_LastGlobalPacketTick;
	}
	int m_nKey;
	CriticalSection m_player_lock;
protected:
	
	bool m_IsInGlobalContainer;
	char m_szNickName[32];
	bool m_isDisconnect;
	SERVER_TIME m_LastGlobalPacketTick;

private:
	class StreamSocketConnection* m_pConnection;
	Addr m_myAddr;
};
