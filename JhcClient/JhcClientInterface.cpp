
#include "../Common/Thread/Scheduler.h"
#include "../Common/Console/Console.h"
#include "../Common/Util/StringUtil.h"
#include "../Common/Network/Message.h"

#include "../network/include/network/IOCP.h"
#include "../network/include/network/IOCPAcceptor.h"
#include "../network/include/network/IOCPConnection.h"
#include "../network/include/network/NetworkEvent.h"
#include "../network/include/network/Util.h"

#include "JhcClientInterface.h"
#include "ClientInitializer.h"
#include "Object.h"
#include "JhcClientNetworkManager.h"
#include "NetJhcServer.h"
#include "ExternClass.h"

#include <chrono>
#include <thread>
extern HWND hWnd;
extern HWND	hEdit;

std::shared_ptr<std::thread> g_tView;
static bool g_bView;

class JhcClientNetworkEventReceiver : public NetworkEventReceiver
{
public:
	JhcClientNetworkEventReceiver() {};
	~JhcClientNetworkEventReceiver() {};
	virtual void onError(const char* szError) {
		if (szError) _cprint("Network error message : [%s]", szError);
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

void JhcClientNetworkEventReceiver::onDisconnect(int nID, class Connection* pConnection)
{
}

void JhcClientNetworkEventReceiver::onRead(int nID, class Connection* pConnection)
{
	StreamSocketConnection* pStream = static_cast<StreamSocketConnection*>(pConnection);

	Object* pObj = reinterpret_cast<Object*>(pStream->GetTag());
	Object::setCurrentThreadObject(pObj);

	char buf[65535 * 2];

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

		if (pMsg->size >= 65535 * 2)
		{
			pStream->Close();
			break;
		}
		if (pStream->Size() < pMsg->size)
			return;

		pStream->Read(buf, pMsg->size);

		JhcClientNetworkManager x;
		if (pObj->ToJhcServer() && pObj)
		{
			x.onMsg(pMsg, pObj->ToJhcServer());
		}
	}

}

IOCPConnection* pConnection = NULL;


bool JhcClientInterface::onInit()
{
	Util::InitNetwork();

	//ClientInitializer InitClient = ClientInitializer(&g_IOCP);
	//std::shared_ptr<std::thread> t_Init_Client= std::make_shared<std::thread>([&InitClient]()
	
	//초기화 갈무리
	auto ClientInit = std::make_unique<ClientInitializer>(&g_IOCP);
	std::shared_ptr<std::thread> t_Init_Client= std::make_shared<std::thread>([Init = std::move(ClientInit)]() -> void
	{
		Init->onProcess();
	});
	t_Init_Client->join();

	g_bView = true;
	g_tView = std::make_shared<std::thread>([]() 
	{
		while (1)
		{
			InvalidateRect(hWnd, NULL, false);
			InvalidateRect(hEdit, NULL, false);
			//std::this_thread::sleep_for(0.5s);
			//std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			for (int lc = 0; lc < 10; ++lc)
			{
				if (g_bView == false) return;
				//Sleep(100);
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}
		}
	});

	return true;
}

bool JhcClientInterface::onDeInit()
{
	//디스커넥트 처리

	g_IOCP.EndThreadPool();
	g_IOCP.DeInit();
	Singleton<Scheduler>::GetStaticInstance().DeInit();

	g_bView = false;
	g_tView->join();
	return true;
}

void JhcClientInterface::onCommand(std::vector<std::string>& _vec_command)
{
	if (_vec_command.empty() == false)
	{
		if (_vec_command[0].compare("/test") == 0)
		{
			_cprint("test\n");
		}
		else if (_vec_command[0].compare("/help") == 0)
		{
			_ctprint("---help-------------------------\n");
			_ctprint("1. /echo 메시지\n");
			_ctprint("--------------------------------\n");
		}
		else if (_vec_command[0].compare("/echo") == 0)
		{
			CS_Echo msg;
			strncpy_s(msg.echo_message, sizeof(msg.echo_message), _vec_command[1].c_str(), sizeof(msg.echo_message)-1);
			Singleton<NetJhcServer>::GetStaticInstance().Send(&msg);
			_ctprint("send echo %s\n", msg.echo_message);
		}
		else if (_vec_command[0].compare("/format") == 0)
		{
			//size_t n_size = _vec_command.size() - 1;
			//std::string s = "";
			//for (auto i = 1; i < n_size; ++i)
			//{
			//	s = StringUtil::format("%s %s", s, _vec_command.at(i).c_str());
			//}
			//_cprint(s.c_str());
			if (_vec_command.size() > 3)
			{
				_cprint(StringUtil::format("%s %s %s %s", _vec_command[0].c_str(), _vec_command[1].c_str(), _vec_command[2].c_str(), _vec_command[3].c_str()).c_str());
			}
			_cprint("\n");
		}
		else if (_vec_command[0].compare("/connect") == 0)
		{
			Singleton<NetJhcServer>::GetStaticInstance().Connect();
		}
	}
}