#pragma once

#include <string>
#include <vector>
#include <windows.h>
#include "../Console/Console.h"

class Interface
{
public:
	Interface() {};
	virtual ~Interface() {};

	virtual bool onInit() { return false; };
	virtual bool onDeInit() { return false; };

	
	virtual bool onKeyDown( int nVkCode )	{ return false; }
	/*
	virtual bool onMessage( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )		{ return false; }
	virtual bool onCommand( const char *szFullCmd,
		std::vector< std::string > & vList,
	*/

	void onWmSize(int width, int height);
	virtual void DoCommand();
	virtual void onCommand(std::vector<std::string>& _vec_command) = 0;

	// 기본 커맨드 구현 모음
	//static void			ProcCommand(const char *szCommand, struct JFrameCommandResultReceiver & receiver, const char *szPeerAddr);

	// 뷰 관련
	enum   ViewMode		{ VIEW_CONSOLE, VIEW_STATUS, VIEW_LOG, VIEW_ABOUT, VIEW_USER };
	static ViewMode		GetViewMode();
	static void			SetViewMode( ViewMode mode );
	static HWND			GetViewWindow();
	static const RECT &	GetViewRect();

	static void			AddWatchList( const std::string & strEnv );
	static void			DelWatchList( const std::string & strEnv );
	static bool			IsExistWatchList( const std::string & strEnv );

	// 콘솔 출력
	virtual void DoDrawConsole( HDC dc, class Console * pConsole );
	//유저 정의 콘솔 가져오기
	virtual Console *GetUserConsole(){return NULL;};

	// 뷰 드로잉 구현
	virtual void onDrawStatus( HDC dc );
	virtual void onDrawUser( HDC dc );
	virtual void onDrawLog( HDC dc );

};

void				ProcCommand(const char* buf);
void				ProcCommandUp();
void				ProcCommandDown();
