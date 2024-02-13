
#include "../network/include/network/Connection.h"
#include "../network/include/network/IOCP.h"
#include "Player.h"
#include "../Common/Network/Message.h"

Player::Player(void) : m_pConnection(NULL)
{
}

Player::~Player(void)
{
	delete m_pConnection;
	m_pConnection = NULL;
}


Player* Player::MakePlayer(class StreamSocketConnection* pConnection)
{
	Player* pPlayer = new Player;
	pPlayer->m_pConnection = pConnection;

	return pPlayer;
}

void Player::Disconnect()
{
	m_pConnection->Close();
}

void Player::Send(struct Message* pMsg)
{
	if (m_pConnection->IsConnected() && m_pConnection)
	{
		m_pConnection->Write(pMsg, pMsg->size);
	}
}

