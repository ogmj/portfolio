
#include "../network/include/network/IOCPConnection.h"
#include "../network/include/network/IOCP.h"
#include "../Common/Network/Message.h"
#include "Player.h"

//test용
Player::Player(): m_pConnection(nullptr), m_nKey(0), m_isDisconnect(false), m_player_lock("Player")
{
	m_IsInGlobalContainer = 0;
}

Player::Player(class StreamSocketConnection* pConnection) : m_pConnection(pConnection), m_nKey(0), m_isDisconnect(false)
{
	m_IsInGlobalContainer = 0;
}

Player::~Player(void)
{
	if (m_pConnection != nullptr)
	{
		delete m_pConnection;
	}
	m_pConnection = nullptr;
}

void Player::Init()
{
	m_LastGlobalPacketTick = 0;
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

bool Player::Proc(unsigned int tm)
{
	//주기적으로 처리해야 하는 일을 처리하자.
	return true;
}
