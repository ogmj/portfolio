#pragma once
#include <windows.h>
#include "../Common/Network/Message.h"
#include "../network/include/thread/Sync.h"
#include <memory>
#include <thread>
#include <list>
class lazyLoop
{
public:
	lazyLoop() {};
	~lazyLoop() {};
	
	//typedef void (WINAPI* LazyQueueHandler)(Message* pMessage);
	//typedef LazyQueueHandler LPLazyQueueHandler;
	using LazyQueueHandler = void(WINAPI*)(Message* pMessage);
	using LPLazyQueueHandler = LazyQueueHandler;

	void Init(LPLazyQueueHandler _func_handler);
	void Run(int _interval);
	void Stop();
	//bool IsTerminate();
	//void Process();
	inline int Interval() { return m_thread_interval; };


	//void push_packet(Message* _p_msg);
	//Message* pop_packet();

private:
	std::list< struct Message* > m_lazylist;

	int	m_thread_interval;
	int m_pending_count;
	CriticalSection m_sync;
	LPLazyQueueHandler m_lazy_func_handler;
	std::shared_ptr<std::thread> m_thread_manager;
};