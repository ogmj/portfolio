#pragma once
#include "../Common/Thread/SchedulerObject.h"

class JhcClientLoop : public SchedulerObject
{
public:
	JhcClientLoop(void);
	virtual ~JhcClientLoop(void);

	virtual void onProcess( int nThreadIdx );
	
protected:
	void ProcObject(unsigned int tm );

};

