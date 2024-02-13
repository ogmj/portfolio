#pragma once

#include <thread>
#include <vector>
#include "SchedulerObject.h"

class Scheduler
{
public:

					Scheduler();
	virtual			~Scheduler();

	virtual bool	Init( unsigned thread_count = 1, void (*init_func)( int ) = 0 );
	bool			DeInit();

	void			DeleteObject( SchedulerObject & obj );
	void			SetObjectPriority( SchedulerObject & obj, SchedulerObject::OBJECT_PRIORITY priority );

	unsigned		GetThreadCount() const		{ return m_nThreadCount; }

private:

	unsigned		m_nThreadCount;
	volatile int	m_nInstructionCount;

	std::vector<std::shared_ptr<class SchedulerInfo>> m_vec_pSchedule;
	std::vector<std::shared_ptr<std::thread>> m_t_vec_ScheduleMng;

	std::shared_ptr<class ObjectDestroyerInfo> m_pObjectDestroyer;
	std::shared_ptr<std::thread> m_t_ObjectDestoryerMng;
};

