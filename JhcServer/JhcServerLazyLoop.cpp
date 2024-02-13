#include "JhcServerlazyLoop.h"

void lazyLoop::Init(LPLazyQueueHandler _func_handler)
{
	m_lazy_func_handler = _func_handler;
}

void lazyLoop::Run(int _interval)
{
	m_thread_interval = _interval;
	//m_thread_manager = std::make_shared<std::thread>(lazyloop, this);
}


void lazyLoop::Stop()
{
	//m_thread_manager->join();
}

unsigned __stdcall lazyloop(lazyLoop* pLoop)
{

	//long thread_id = InterlockedIncrement(&pLoop->concurrency);

	//while (!pLoop->IsTerminate())
	//{
		//현재 틱을 구한다.

		//pLoop->Process();
		//Sleep(pLoop->interval());
	//}

	//InterlockedDecrement(&pLoop->concurrency);
	return 0;
}