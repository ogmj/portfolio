#include <atomic>

#include "JhcServerInterface.h"
#include "ServerInitializer.h"
#include "JhcServerNetworkManager.h"
#include "ExternClass.h"

#include "../network/include/network/IOCP.h"
#include "../network/include/network/IOCPAcceptor.h"
#include "../network/include/network/IOCPConnection.h"
#include "../network/include/network/NetworkEvent.h"
#include "../network/include/network/Util.h"

#include "../Common/Thread/Scheduler.h"
#include "../Common/Console/Console.h"
#include "../Common/Util/StringUtil.h"
#include "../Common/Network/Message.h"

#include "Object.h"
#include "Player.h"
#include "JContainer.h"

extern HWND hWnd;
extern HWND	hEdit;

extern ServerTime g_GlobalTime;
OverlappedAllocator	g_Allocator;
IOCPAcceptor*		s_pAcceptor;

std::shared_ptr<std::thread> g_tView;
static bool g_bView;

class JhcServerNetworkEventReceiver : public NetworkEventReceiver
{
public:
	JhcServerNetworkEventReceiver() {};
	~JhcServerNetworkEventReceiver() {};
	virtual Connection* createConnection(const Socket& socket)
	{
		return new IOCPConnection(&g_Allocator, socket, false, nullptr, nullptr);
	}
	virtual bool onAccept(int nID, class Acceptor* pAcceptor, class Connection* pConnection);
	virtual void onAccepted(int nID, class Connection* pConnection);
	virtual void onError(const char* szError)
	{
		if (szError)
		{
			_cprint("Network error message : [%s]", szError);
		}
	}

	virtual void onDisconnect(int nID, class Connection* pConnection);
	virtual void onRead(int nID, class Connection* pConnection);
	virtual void onConnect(int nID, class Connection* pConnection)
	{
	}
	virtual void onCantConnect(int nID, class Connection* pConnection)
	{
	}
} g_IOCPEventHandler;

IOCP	g_IOCP(&g_IOCPEventHandler);

static std::atomic<int> g_index_key(0);
JContainer<Player> g_PlayerMng;

bool JhcServerNetworkEventReceiver::onAccept(int nID, class Acceptor* pAcceptor, class Connection* pConnection)
{
	StreamSocketConnection* pStream = static_cast<StreamSocketConnection*>(pConnection);
	pStream->EnableNoDelay();

	if (pAcceptor == s_pAcceptor)
	{
		std::shared_ptr<Player> player = std::make_shared<Player>(pStream);
		if (g_index_key == 0)
		{
			++g_index_key;
		}
		else
		{
			g_index_key = g_index_key + 10;
		}
		player->m_nKey = g_index_key;
		pStream->SetTag(player->ToPlayer());
		g_PlayerMng.Add(player);
		player->SetLastPacketTick(g_GlobalTime.GetTime());

		_ctprint("Client Connected\n");
	}
	else
	{
		//Sync
		pStream->Close();
	}

	return true;
}

void JhcServerNetworkEventReceiver::onAccepted(int nID, class Connection* pConnection)
{
	// accept 이후에 바로 무언가 패킷을 쏴야할 때 이곳을 이용함. onAccept는 IOCP에 등록되기 전 이벤트임..
	Object* pPlayer = reinterpret_cast<Player*>(pConnection->GetTag());
	if (!pPlayer)
	{
		return;
	}

	//SC_Ping msg;
	//msg.timestamp = g_GlobalTime.GetTime();
	//IOCPConnection* pStream = static_cast<IOCPConnection*>(pConnection);
	//pStream->Write(&msg, msg.size);

}

void JhcServerNetworkEventReceiver::onDisconnect(int nID, class Connection* pConnection)
{
	StreamSocketConnection* pStream = static_cast<StreamSocketConnection*>(pConnection);

	Object* pObj = reinterpret_cast<Object*>(pStream->GetTag());
	if (!pObj)
	{
		return;
	}
	if (pObj->ToPlayer())
	{
		std::shared_ptr<Player> p;
		g_PlayerMng.Delete(pObj->ToPlayer()->m_nKey, p);
	}
}

void JhcServerNetworkEventReceiver::onRead(int nID, class Connection* pConnection)
{
	StreamSocketConnection* pStream = static_cast<StreamSocketConnection*>(pConnection);

	Object* pObj = reinterpret_cast<Object*>(pStream->GetTag());

	char buf[65535];

	while (pStream)
	{
		if (pStream == NULL)
		{
			return;
		}
		int size = pStream->Size();
		if (size < 0)
		{
			pStream->Close();
			break;
		}
		if (size < sizeof(Message))
		{
			return;
		}
		int len = pStream->Peek(buf, sizeof(Message));

		if (len < 0)
		{
			pStream->Close();
			break;
		}
		Message* pMsg = reinterpret_cast<Message*>(buf);
		if (!pMsg)
		{
			pStream->Close();
			break;
		}

		if (pMsg->size <= 0)
		{
			pStream->Close();
			break;
		}
		if (pMsg->size < sizeof(Message))
		{
			pStream->Close();
			break;
		}

		if (pMsg->size > 65535)
		{
			pStream->Close();
			break;
		}
		if (pStream->Size() < pMsg->size)
			return;

		pStream->Read(buf, pMsg->size);

		JhcServerNetworkManager x;
		if (pObj->ToPlayer() && pObj)
		{
			x.onMsg(pMsg, pObj->ToPlayer());
		}
	}
}

IOCPConnection* pConnection = NULL;

bool JhcServerInterface::onInit()
{

	//::SetWindowText(hWnd, "JhcServer");
	Util::InitNetwork();

	s_pAcceptor = new IOCPAcceptor(10);

	srand(g_GlobalTime.GetTime());
	
	//ServerInitializer ServerInit(&g_IOCP, s_pAcceptor);
	//std::shared_ptr<std::thread> p_t_ServerInit = std::make_shared<std::thread>([&ServerInit]()

	//초기화 갈무리
	auto ServerInit = std::make_unique<ServerInitializer>(&g_IOCP, s_pAcceptor);
	std::shared_ptr<std::thread> p_t_ServerInit = std::make_shared<std::thread>([Init = std::move(ServerInit)]() -> void
	{
		Init->onProcess();
	});
	p_t_ServerInit->join();

	g_bView = true;
	g_tView = std::make_shared<std::thread>([]() 
	{
		while (1)
		{
			InvalidateRect(hWnd, NULL, false);
			InvalidateRect(hEdit, NULL, false);
			for (int lc = 0; lc < 10; ++lc)
			{
				if (g_bView == false)
					return;
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}
		}
	});

	return true;
}

bool JhcServerInterface::onDeInit()
{
	s_pAcceptor->EndAccept();
	while (1)
	{
		std::vector< std::shared_ptr<Player>> vPlayer;
		vPlayer.clear();
		g_PlayerMng.Enum(vPlayer);
		if (vPlayer.empty() == true)
		{
			break;
		}
		for (auto& i : vPlayer)
		{
			if (i->ToPlayer())
			{
				i->Disconnect();
			}
		}
		Sleep(1000);
	}
	g_IOCP.EndThreadPool();
	g_IOCP.DeInit();

	Singleton<Scheduler>::GetStaticInstance().DeInit();

	g_bView = false;
	g_tView->join();

	delete s_pAcceptor;
	return true;
}

CriticalSection lock1;
CriticalSection lock2;
CriticalSection lock3;

void JhcServerInterface::onCommand(std::vector<std::string>& _vec_command)
{
	if (_vec_command.empty() == false)
	{
		if (_vec_command[0].compare("/test") == 0)
		{
			int num = 10;
			int &&num1 = 4+6;


			int* ptr = &num;
			//*ptr = 12;

			_cprint("test\n");
		}
		else if (_vec_command[0].compare("/tt") == 0)
		{
			extern thread_local std::stack<std::string> lock_stack;
			{
				THREAD_SYNCHRONIZE(lock1);
				{
					THREAD_SYNCHRONIZE(lock2);
					{
						THREAD_SYNCHRONIZE(lock3);
						_cprint("tt\n");
					}
				}

			}
		}
		else if (_vec_command[0].compare("/test_player_make_shared") == 0)
		{
			if (_vec_command.size() == 2)
			{
				int cnt = atoi(_vec_command[1].c_str());
				PERF("test_player_make_shared");
				for (auto i = 0; i <= cnt; ++i)
				{
					std::shared_ptr<Player> player = std::make_shared<Player>();
				}
			}
		}
		else if (_vec_command[0].compare("/test_player_memory_pool") == 0)
		{
			if (_vec_command.size() == 2)
			{
				int cnt = atoi(_vec_command[1].c_str());
				PERF("test_player_memory_pool");
				for (auto i = 0; i <= cnt; ++i)
				{
					std::shared_ptr<Player> p_player(new Player());

					auto a = std::move(p_player);
					int b = 0;
					++b;
				}
			}
		}
		else if (_vec_command[0].compare("/profile") == 0)
		{
			Singleton<perf_mng>::GetStaticInstance().profile();

		}
		else if (_vec_command[0].compare("/test_player_perf") == 0)
		{
			Singleton<perf_mng>:: GetInstance()->report("11");
		}
		else if (_vec_command[0].compare("/format") == 0)
		{
			if (_vec_command.size() > 3)
			{
				_cprint(StringUtil::format("%s %s %s %s", _vec_command[0].c_str(), _vec_command[1].c_str(), _vec_command[2].c_str(), _vec_command[3].c_str()).c_str());
			}
			_cprint("\n");
		}
	}
}