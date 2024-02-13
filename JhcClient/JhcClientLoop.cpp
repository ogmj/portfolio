
#include <vector>
#include "JhcClientLoop.h"
#include "Player.h"
#include "ExternClass.h"

JhcClientLoop::JhcClientLoop(void)
{	
}

JhcClientLoop::~JhcClientLoop(void)
{
}

void JhcClientLoop::onProcess( int nThreadIdx )
{
	ProcObject( g_GlobalTime.GetTime() );
}

void JhcClientLoop::ProcObject(unsigned int tm )
{

}