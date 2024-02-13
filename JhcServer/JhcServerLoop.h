#pragma once
#include "../Common/Thread/SchedulerObject.h"

class JhcServerLoop : public SchedulerObject
{
public:
	JhcServerLoop(void);
	virtual ~JhcServerLoop(void);

	virtual void onProcess( int nThreadIdx );
	
protected:
	void ProcObject(unsigned int tm );

};

