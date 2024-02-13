

#include "../network/include/network/IOCPConnection.h"
#include "../network/include/network/IOCP.h"
#include "../Common/Console/Console.h"
#include "../Common/Network/MsgBase.h"

#include "NetJhcServer.h"
#include "ExternClass.h"

NetJhcServer::NetJhcServer()
{
	m_pConnection = nullptr;
	m_bUse = true;
}

NetJhcServer::~NetJhcServer(void)
{
}

#define DEFAULT_WRITE_KEY					"wnd@$rmq*(wk :vorlwl"
#define DEFAULT_READ_KEY					"rnd!*rmr%! rkrtjd$%)"

bool NetJhcServer::Init(IOCP* iocp)
{
	THREAD_SYNCHRONIZE(m_Lock);
	m_pIOCP = iocp;
	m_pConnection = new IOCPConnection(nullptr,false);
	//m_pConnection = new IOCPConnection(nullptr,true, DEFAULT_READ_KEY, DEFAULT_WRITE_KEY);
	m_pConnection->SetID(OBJ_CONNECTION);
	return true;
}

void NetJhcServer::Stop()
{
	THREAD_SYNCHRONIZE(m_Lock);
	m_bUse = false;
	Close();
}

void NetJhcServer::Close()
{
	THREAD_SYNCHRONIZE(m_Lock);

	if (!m_pConnection)
		return;

	m_pConnection->Close();
	delete m_pConnection;
	m_pConnection = nullptr;
}

bool NetJhcServer::Reconnect()
{
	THREAD_SYNCHRONIZE(m_Lock);
	if (!m_bUse) return false;

	Close();
	Init(m_pIOCP);

	if (m_pConnection->SyncConnect(Addr("127.0.0.1",10000)))
	{
		m_pConnection->SetTag(static_cast<Object*>(this));
		m_pIOCP->AddObject(m_pConnection);
		_ctprint("JhcServer Connected\n");
		return true;
	}

	return false;
}

bool NetJhcServer::Connect()
{
	THREAD_SYNCHRONIZE(m_Lock);
	if (!m_pConnection)
		return false;
	for (int i = 0; i < 3; ++i)
	{
		if (m_pConnection->SyncConnect(Addr("127.0.0.1", 9990)))
		{
			m_pConnection->SetTag(static_cast<Object*>(this));
			m_pIOCP->AddObject(m_pConnection);
			_ctprint("JhcServer Connected\n");
			_ctprint("/help 명령으로 기능 테스트를 할 수 있습니다..\n");
			return true;
		}
		Sleep(100);
	}
	_ctprint("서버 실행 후 '/connect' 명령을 입력해주세요.\n");
	return false;
}

void NetJhcServer::onDisconnect()
{
}

bool NetJhcServer::IsConnect()
{
	THREAD_SYNCHRONIZE(m_Lock);
	if (!m_bUse)
		return false;

	if (m_pConnection && m_pConnection->IsConnected()) {
		return true;
	}
	return Reconnect();

}


void NetJhcServer::Send(struct Message* pMsg)
{
	THREAD_SYNCHRONIZE(m_Lock);
	if (IsConnect()) {
		m_pConnection->Write(pMsg, pMsg->size);
	}
}
