#pragma once
#include "../Common/Interface/Interface.h"
#include "../Common/Util/ServerTime.h"
//#include "../Common/Util/Singleton.h"
#include "../Common/Debugging/Perf.h"
#include "../Common/Util/MemoryPool.h"

#include "JhcServerLoop.h"
#include "JContainer.h"
#include "Player.h"

#include <stack>

extern ServerTime g_GlobalTime;
extern JhcServerLoop g_JhcServerLoop;
extern JContainer<Player> g_PlayerMng;
