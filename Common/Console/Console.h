#pragma once

// 가상 콘솔.
// 
// 일정 크기의 버퍼를 가지고 있고 거기다 Printf()로 찍어댈수 있으며,
// 화면 출력을 위해 ScreenXXX() 계열 함수를 지원한다.
//
//  . _cprint( "blah blah" ); 라고 하면 언제 어디서나 찍을 수 있다.
//

#include "../../network/include/thread/Sync.h"
#include <stdio.h>
#include <stdarg.h>

const int DEFAULT_CONSOLE_HEIGHT = 1024;
extern HWND hWnd;

class Console
{
public:
	Console( int _n_width = 120, int _n_height = 25 ) : m_lock( "console" )
	{
		m_nWidth = _n_width;
		m_nScreenHeight = m_nHeight = _n_height;
		m_bIsFull = false;
		m_nCount = 0;
		//m_n_new_adjust = 0;
		m_nScreenAdjust = 0;

		m_p_lines = new char*[_n_height];

		for ( int i = 0; i < _n_height; i++ )
		{
			m_p_lines[i] = new char[_n_width +1];
			memset(m_p_lines[i], 0, _n_width +1 );
		}

		m_nX = 0;
		m_nY = 0;
//		pHookfunctor = NULL;
	}

	~Console()
	{
		for (int i = 0; i < m_nHeight; i++)
		{
			delete[] m_p_lines[i];
		}

		delete [] m_p_lines;
	}

	void Write( const char *_str )
	{
		_Printf( _str );
	}

	void Printf( const char *_str, ... )
	{
		THREAD_SYNCHRONIZE( m_lock );

		char sz_buf[2048];
		va_list va;
		va_start( va, _str );
		_vsnprintf(sz_buf, sizeof(sz_buf), _str, va );
		va_end( va );

		_Printf(sz_buf);
		InvalidateRect(hWnd, NULL, false);
	}

	void PrintfWithTimestamp( const char *str, ... )
	{
		THREAD_SYNCHRONIZE( m_lock );

		char sz_buf[2048];

		SYSTEMTIME st;
		GetLocalTime( &st );

		int n_written_count = sprintf(sz_buf, "%04d/%02d/%02d %02d:%02d:%02d ", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond );

		va_list va;
		va_start( va, str );
		_vsnprintf(sz_buf + n_written_count, sizeof(sz_buf) - n_written_count, str, va );
		va_end( va );

		_Printf(sz_buf);
		InvalidateRect(hWnd, NULL, false);
	}

	const char* GetLine( unsigned _line )
	{
		m_lock.Lock();
		

		if ( _line > (unsigned)m_nHeight)
		{
			m_lock.UnLock();
			return "~";
		}

		int nl = _line;

		if (m_bIsFull)
		{
			nl = m_nY + _line;
			nl %= m_nHeight;
		}

		m_lock.UnLock();

		return m_p_lines[nl];
	}

	int GetWidth()	const	{ return m_nWidth; };
	int GetHeight()	const	{ return m_nHeight; };
	
	// 현재 저장중인 라인 수를 리턴
	int GetCount()  const	{ return m_nCount; }

	void SetScreenHeight( unsigned _height )	{ m_nScreenHeight = _height; };
	int  GetScreenHeight() const			{ return m_nScreenHeight; };
	void ScreenUp()							{ onScreenPos(m_nScreenAdjust - 1 ); }
	void ScreenDown()						{ onScreenPos(m_nScreenAdjust + 1 ); }
	void ScreenPageUp()						{ onScreenPos(m_nScreenAdjust - m_nScreenHeight / 3 ); }
	void ScreenPageDown()					{ onScreenPos(m_nScreenAdjust + m_nScreenHeight / 3 ); }
	void ScreenHome()						{ onScreenPos( 0 - m_nCount + m_nScreenHeight - 1 );	}
	void ScreenEnd()						{ m_nScreenAdjust = 0; }
	const char* GetScreenLine( unsigned l )	{ return GetLine( l + m_nScreenAdjust); }

	void Clear()
	{
		m_nCount = 0;
		for ( int i = 0; i < m_nHeight; i++ ) m_p_lines[i][0] = 0;
		m_nX = 0;
		m_nY = 0;
		m_bIsFull = false;
	}

	static Console& Inst()
	{
		static Console console(120, DEFAULT_CONSOLE_HEIGHT);
		return console;
	}

private:
	int m_nCount;
	int m_nWidth;
	int m_nHeight;
	int m_nX;
	int m_nY;
	bool m_bIsFull;
	int m_nScreenHeight;
	int m_nScreenAdjust;

	void _Printf( const char *_sz_buf )
	{
		unsigned feed;

		char *p = const_cast< char* >(_sz_buf);
		char *sp = p;
		for ( ;;*p++)
		{	
			if ( *p == '\n' )
			{	
				feed = putString( sp, (unsigned)(p - sp) );
				
				sp = p;

				addY();
				m_p_lines[m_nY][0] = 0;

				if ( feed )
				{
					sp -= feed;
					_Printf( sp );
					return;
				}

				sp++;

				continue;
			}

			if ( !*p )
			{
				if ( p - sp ) 
				{
					feed = putString( sp, (unsigned)(p - sp) );
					if ( feed )
					{
						addY();
						_Printf( sp + feed );
					}
				}
				break;
			}
		}
	}

	unsigned putString( const char *_str, unsigned _len )
	{
		int y = m_nY;

		unsigned newlen = _len;

		if (m_nX + _len >= (unsigned)m_nWidth)
		{
			newlen = m_nWidth - m_nX;
		}

		strncpy( &m_p_lines[y][m_nX], _str, newlen );

		m_nX += _len;

		m_p_lines[y][(m_nX > m_nWidth ? m_nWidth : m_nX)] = '\0';

		return _len - newlen;
	}

	void addY()
	{
		m_nX = 0;
		if ( ++m_nY == m_nHeight)
		{	
			m_nY = 0;
			m_bIsFull = true;
		}
		if (m_nCount < m_nHeight)
			++m_nCount;
	}

	void onScreenPos( int _n_new_adjust )
	{
		m_lock.Lock();

		// 위로 튕기는 것
		if (m_nCount - m_nScreenHeight + _n_new_adjust < 0 )
		{
			_n_new_adjust = 0 - m_nCount + m_nScreenHeight - 1;
		}

		// 아래로 튕기는 것
		if (_n_new_adjust > 0 )
			_n_new_adjust = 0;

		m_nScreenAdjust = _n_new_adjust;

		m_lock.UnLock();
	}

	CriticalSection m_lock;
	char **m_p_lines;
};

#define _cprint	Console::Inst().Printf
#define _ctprint Console::Inst().PrintfWithTimestamp
