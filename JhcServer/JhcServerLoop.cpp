
#include <vector>
#include "JhcServerLoop.h"
#include "Player.h"
#include "ExternClass.h"

JhcServerLoop::JhcServerLoop(void)
{	
}

JhcServerLoop::~JhcServerLoop(void)
{
}

void JhcServerLoop::onProcess( int nThreadIdx )
{
	ProcObject( g_GlobalTime.GetTime() );
}

void JhcServerLoop::ProcObject(unsigned int tm )
{
	std::vector< std::shared_ptr<Player>> v_delete_player;

	g_PlayerMng.Proc(tm, v_delete_player);
	for (auto& i : v_delete_player)
	{
		i->Delete();
	}
}