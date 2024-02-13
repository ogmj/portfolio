#pragma once
#include "Object.h"
#include "../network/include/network/Addr.h"

#include "ExternClass.h"

class Player : public Object
{
public:
	Player(void);
	virtual ~Player(void);

	virtual Player * ToPlayer()	{ return this; }
	virtual void Send(struct Message* pMsg);

	static Player* MakePlayer(class StreamSocketConnection* pConnection);
	void Disconnect();


protected:
	
	char m_szNickName[32];


private:
	class StreamSocketConnection* m_pConnection;
	Addr m_myAddr;
};
