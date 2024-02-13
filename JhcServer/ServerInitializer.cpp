#include "../network/include/network/IOCPAcceptor.h"
#include "../network/include/network/IOCP.h"
#include "ServerInitializer.h"
#include "../Common/Console/Console.h"
#include "../Common/Thread/Scheduler.h"
#include "ExternClass.h"


ServerInitializer::ServerInitializer(class IOCP* iocp, class IOCPAcceptor* acceptor)
{
	m_pIOCP = iocp;
	m_pAcceptor = acceptor;
}

ServerInitializer::~ServerInitializer()
{
}

void ServerInitializer::onInitServer()
{

	_ctprint("Start IOCP ... %s\n", m_pIOCP->Init() ? "ok" : "failed");
	m_pIOCP->StartThreadPool(8);

	Singleton<Scheduler>::GetStaticInstance().Init(8);
	Singleton<Scheduler>::GetStaticInstance().SetObjectPriority(g_JhcServerLoop, SchedulerObject::UPDATE_PRIORITY_NORMAL);
}

void ServerInitializer::onInitNetwork()
{
	int newPort = 9990;
	_ctprint("Start JhcServer Connection listener( %d port ) ... %s\n", newPort, m_pAcceptor->StartAccept(newPort) ? "ok" : "failed");
	m_pIOCP->AddObject(m_pAcceptor);
}

void ServerInitializer::onInitDB()
{
}

void ServerInitializer::onInitResource()
{
}

bool ServerInitializer::onProcess()
{
	onInitServer();
	onInitNetwork();

	_ctprint("Server On\n");
	return true;
}

