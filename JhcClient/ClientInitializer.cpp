#include "../network/include/network/IOCPAcceptor.h"
#include "../network/include/network/IOCP.h"

#include "../Common/Console/Console.h"

#include "ExternClass.h"
#include "ClientInitializer.h"
#include "NetJhcServer.h"
#include "../Common/Thread/Scheduler.h"

ClientInitializer::ClientInitializer(class IOCP* iocp)
{
	m_pIOCP = iocp;
}

ClientInitializer::~ClientInitializer()
{
}

void ClientInitializer::onInitClient()
{
	Singleton<NetJhcServer>::GetStaticInstance().Init(m_pIOCP);
	m_pIOCP->Init();
	m_pIOCP->StartThreadPool(1);

	Singleton<Scheduler>::GetStaticInstance().Init(1);
	Singleton<Scheduler>::GetStaticInstance().SetObjectPriority(g_JhcClientLoop, SchedulerObject::UPDATE_PRIORITY_NORMAL);

}

void ClientInitializer::onNetworkConnect()
{
	Singleton<NetJhcServer>::GetStaticInstance().Connect();
}

bool ClientInitializer::onProcess()
{
	onInitClient();
	onNetworkConnect();
	return true;
}
