#pragma once

#include "../../Common/Util/w32_util.h"
//전처리기 WIN32_LEAN_AND_MEAN로 제거된 timeGetTime 해결법
#include <windows.h>
#include <mmsystem.h>

class ServerTime
{
public:
	ServerTime()
	{
		m_unStartTime	= timeGetTime();
		m_nGap			= 2000000;
		m_nTimeRate		= 1;
	}

	inline void Reset()
	{
		m_unStartTime = timeGetTime();
	}

	inline void SetTimeRate( int r )
	{
		m_nTimeRate = r;
	}

	inline unsigned int GetTime() const
	{
		unsigned t = timeGetTime();
		return ( t - m_unStartTime ) / m_nTimeRate + m_nGap;
	}

	inline void SetGap( int g )
	{
		m_nGap = g;
	}

protected:
	int			m_nTimeRate;
	int			m_nGap;
	unsigned	m_unStartTime;
};
