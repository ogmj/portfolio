
#include <windows.h>
#include <Windowsx.h>
#include <stdio.h>
#include <psapi.h>
#include <process.h>
#include <time.h>
//#include <direct.h>

#include "Interface.h"

#include "../../Common/Console/Console.h"
#include "../../Common/Font/Font.h"
#include "../../Common/Util/StringUtil.h"

#include <crtdbg.h>

#include <string>
#include <list>
#include <vector>

#pragma comment(lib, "ws2_32.lib")

// APP Global 
extern HWND hWnd;
extern HWND	hEdit;
extern HINSTANCE hInst;
extern RECT rectWindow;
extern Interface::ViewMode		viewMode = Interface::VIEW_LOG;

bool								bIsWhileCommandSearch;
std::list< std::string >			listCommand;
std::list< std::string >::iterator	listCommandIt;
Interface* pInterface;

SYSTEMTIME			 g_startTime;
static CriticalSection			s_connectionLock( "ConnectionLock" );


ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
void				DrawConsole( HDC dc, Console * pConsole );
void				DrawTitle(HDC dc, Interface* pIntf);


const int			COMMAND_BUF = 128;	// 최대 저장 가능 커맨드
const int			CONSOLE_FONT_HEIGHT = 14;	// 콘솔 폰트 세로 크기
const int			INPUT_BOX_HEIGHT = 20;	// 하단의 입력창 세로 크기
const int			CONSOLE_LINE_GAP = 3;	// 라인간 공백
const char*			CONSOLE_FONT = "Console";
const int			LINE_HEIGHT = CONSOLE_FONT_HEIGHT + CONSOLE_LINE_GAP;

RECT				rectWindow;

Console& GetMainConsole()
{
	static Console	inst(120, 100);
	return inst;
}

void Interface::DoCommand()
{
	//에디트 박스 메시지 입력 처리부분
	char buf[512];
	memset(&buf, 0, sizeof(buf));

	GetWindowText(hEdit, buf, sizeof(buf));
	buf[sizeof(buf) - 1] = 0;
	char* p;
	if (p = strstr(buf, "\r\n"))
	{
		SetWindowText(hEdit, "");

		std::string str_command = buf;

		StringUtil::Replace(str_command, "\r\n", "\n");
		char t[] = { 10, 0 };
		StringUtil::Replace(str_command, t, "");
		ProcCommand(str_command.c_str());

		std::vector<std::string> vec_command;
		StringUtil::DivideCommand(vec_command, str_command);
		onCommand(vec_command);
		InvalidateRect(hWnd, NULL, false);
	}
}

void Interface::onWmSize(int width, int height)
{
	static bool bInit;

	if (!bInit)
	{
		static Font hFont(CONSOLE_FONT, CONSOLE_FONT_HEIGHT / 2, CONSOLE_FONT_HEIGHT);

		hEdit = CreateWindow("edit", NULL, WS_CHILD | WS_VISIBLE | WS_DLGFRAME | ES_MULTILINE | ES_WANTRETURN | ES_AUTOVSCROLL, 0, height - INPUT_BOX_HEIGHT, width, INPUT_BOX_HEIGHT, hWnd, (HMENU)1, hInst, NULL);
		SendMessage(hEdit, WM_SETFONT, (WPARAM)(HFONT)hFont, 1);
		ShowWindow(hEdit, SW_SHOW);
		SetFocus(hEdit);

		bInit = true;
	}

	rectWindow.right = width;
	rectWindow.bottom = height - INPUT_BOX_HEIGHT;

	GetMainConsole().SetScreenHeight((height - INPUT_BOX_HEIGHT) / LINE_HEIGHT);
	Console::Inst().SetScreenHeight((height - INPUT_BOX_HEIGHT) / LINE_HEIGHT);

	SetWindowPos(hEdit, NULL, -2, height - INPUT_BOX_HEIGHT + 3, width + 6, INPUT_BOX_HEIGHT, 0);
}


Interface::ViewMode Interface::GetViewMode()
{
	return viewMode;
}

HWND Interface::GetViewWindow()
{
	return hWnd;
}

const RECT & Interface::GetViewRect()
{
	return rectWindow;
}

void Interface::DoDrawConsole(HDC dc, Console * pConsole)
{
	DrawConsole( dc, pConsole );
}

void Interface::onDrawStatus(HDC dc)
{
	Console statConsole;
	statConsole.Printf( "\n" );
	DoDrawConsole( dc, &statConsole );
}

void Interface::onDrawUser(HDC dc)
{
	TextOut( dc, 10, 10, "No user screen", 14 );
}

void Interface::onDrawLog(HDC dc)
{
	DoDrawConsole( dc, &Console::Inst() );
	//InvalidateRect( GetViewWindow(), NULL, false );
}

void Interface::SetViewMode(Interface::ViewMode mode)
{
	if( viewMode != mode )
	{
		viewMode = mode;
		InvalidateRect( hWnd, NULL, true );
	}
}


void DrawConsole( HDC dc, Console * pConsole )
{
	struct _Console
	{
		_Console( HDC dc )
		{
			consoleDC = CreateCompatibleDC( dc );
			consoleBitmap = CreateCompatibleBitmap( dc, 1600, 1200 );
			oldObject = SelectObject( consoleDC, consoleBitmap );

			consoleFont = Font::CreateCustomFont( "굴림체", CONSOLE_FONT_HEIGHT/2, CONSOLE_FONT_HEIGHT );
			consoleBrush = CreateSolidBrush( RGB( 0,0,0) );
			oldFont = SelectFont( consoleDC, consoleFont );
			SetTextColor( consoleDC, RGB( 255, 255, 255 ) );
			SetBkMode( consoleDC, TRANSPARENT );
		}

		~_Console()
		{
			SelectObject( consoleDC, oldObject );
			SelectFont( consoleDC, oldFont );
			DeleteDC( consoleDC );
			DeleteObject( consoleBitmap );
			DeleteObject( consoleBrush );
			DeleteObject( consoleFont );
		}

		void Clear( const RECT & rect )
		{
			FillRect( consoleDC, &rect, consoleBrush );
		}

		HFONT oldFont;
		HFONT consoleFont;
		HDC consoleDC;
		HBITMAP consoleBitmap;
		HBRUSH consoleBrush;

		HGDIOBJ oldObject;
	};

	static _Console _sc( dc );

	RECT rect;
	GetWindowRect( hWnd, &rect );
	rect.right -= rect.left;
	rect.bottom -= rect.top;
	int nY = rect.bottom;
	nY -= ( INPUT_BOX_HEIGHT + 2 );

	RECT scRect;
	scRect.bottom = 1200;
	scRect.top = 0;
	scRect.left = 0;
	scRect.right = 1600;
	_sc.Clear( scRect );

	if( (pConsole->GetCount()+CONSOLE_LINE_GAP) * (LINE_HEIGHT) < nY )
	{
		for( int i = 0; i < pConsole->GetHeight(); i++ )
		{
			TextOut( _sc.consoleDC, 5, 5 + i*LINE_HEIGHT, pConsole->GetScreenLine(i), (int)strlen( pConsole->GetScreenLine(i) ) );
		}
	}
	else
	{
		int cnt = 3;
		for( int i = pConsole->GetCount(); i >= 0; i--, cnt++ )
		{
			TextOut( _sc.consoleDC, 5, nY - cnt * LINE_HEIGHT + CONSOLE_LINE_GAP*2, pConsole->GetScreenLine(i), (int)strlen( pConsole->GetScreenLine(i) ) );
		}
	}

	BitBlt( dc, 0, 0, 1600, 1200, _sc.consoleDC, 0, 0, SRCCOPY );
}


static void DrawTitle(HDC dc, Interface* pIntf)
{
	static bool bInit;

	struct _Logo
	{
		~_Logo()
		{
			DeleteObject( bufferBitmap );
			DeleteObject( logoBitmap );
			DeleteObject( creditBitmap );
			DeleteDC( creditDC );
			DeleteDC( logoDC );
			DeleteDC( memDC );
		}

		HBITMAP bufferBitmap;
		HBITMAP logoBitmap;
		HBITMAP creditBitmap;
		HDC memDC;
		HDC logoDC;
		HDC creditDC;
	};
	static _Logo _scr;


	if( !bInit )
	{

		bInit = true;
	}

	BitBlt( dc, 0, 0, 1600, 1200, _scr.memDC, 0, 0, SRCCOPY );
}

void ProcCommand( const char * buf )
{
	// { action history
	bIsWhileCommandSearch = false;
	if( strlen(buf) ) listCommand.push_back( std::string( buf ) );
	if( listCommand.size() > COMMAND_BUF )
	{
		listCommand.pop_front();
	}
	// }

	InvalidateRect( hWnd, NULL, false );
}

void ProcCommandUp()
{
	if( listCommand.empty() ) return;
	if( !bIsWhileCommandSearch ) 
	{
		listCommandIt = listCommand.end();
	}

	if( listCommandIt != listCommand.begin() )
	{
		listCommandIt--;
	}

	SetWindowText( hEdit, listCommandIt->c_str() );
	SendMessage( hEdit, EM_SETSEL, listCommandIt->size(), listCommandIt->size() );
	bIsWhileCommandSearch = true;
}

void ProcCommandDown()
{
	if( listCommand.empty() ) return;
	if( !bIsWhileCommandSearch )
	{
		listCommandIt = listCommand.end();
		listCommandIt--;
	}

	if( &(*listCommandIt) != &listCommand.back() )
	{
		listCommandIt++;
		bIsWhileCommandSearch = true;
		SetWindowText( hEdit, (*listCommandIt).c_str() );
	}
	else
	{	
		bIsWhileCommandSearch = false;
		SetWindowText( hEdit, "" );
	}

	SendMessage( hEdit, EM_SETSEL, listCommandIt->size(), listCommandIt->size() );
}

static std::string & getUpTime()
{
	static std::string uptime;

	SYSTEMTIME stm;
	GetLocalTime( &stm );

	unsigned __int64 tm1, tm2;
	SystemTimeToFileTime( &g_startTime,	(LPFILETIME)&tm1 );
	SystemTimeToFileTime( &stm,			(LPFILETIME)&tm2 );

	tm2 -= tm1;

	FileTimeToSystemTime( (LPFILETIME)&tm2, &stm );

	stm.wYear -= 1601;
	stm.wMonth--;
	stm.wDayOfWeek--;
	stm.wDay--;

	std::string strTmp;
	char szTmp[64];
	if( stm.wMonth ) { sprintf( szTmp, "%d Month  ", stm.wMonth ); strTmp += szTmp; }
	if( stm.wDay ) { sprintf( szTmp, "%d Day  ", stm.wDay ); strTmp += szTmp; }
	if( stm.wHour ) { sprintf( szTmp, "%d Hour  ", stm.wHour ); strTmp += szTmp; }
	if( stm.wMinute ) { sprintf( szTmp, "%d Minute  ", stm.wMinute ); strTmp += szTmp; }
	sprintf( szTmp, "%d Second", stm.wSecond); strTmp += szTmp;

	uptime = strTmp;

	return uptime;
}


CriticalSection g_l1( "Lock1" );
CriticalSection g_l2( "Lock2" );

