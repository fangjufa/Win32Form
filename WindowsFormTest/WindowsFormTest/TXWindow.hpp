#pragma once

//#include <gl/GL.h>

#include<Windows.h>
#include<iostream>
#include<vector>
//#include<WinUser.h>

//#include "stdafx.h"
//#include "resource.h"
//
//#include "Debug.h"

using namespace std;


struct ALLMONITORINFO
{
	HMONITOR hMonitor;
	RECT     rect;
	bool     isPrimary;
};

class TXWindow
{
public:
	TXWindow()
	{
		_hWnd = NULL;
		szTitle = L"显示窗口";
		szWindowClass = L"oglWindow";
		width = 1920;
		height = 1080;
		startx = 1920;
		starty = 0;
		isFullScreen = false;
	}

	~TXWindow()
	{
		//Close();
	}

	HINSTANCE _hInst;					// 当前实例
	TCHAR* szTitle;					// 标题栏文本
	TCHAR* szWindowClass;			// 主窗口类名

	//生成的窗口句柄
	HWND _hWnd;
	bool isFullScreen;


	/// <summary> 全屏时窗口的宽. </summary>
	UINT width;

	/// <summary> 全屏时窗口的高. </summary>
	UINT height;

	/// <summary> 全屏时的初始位置x. </summary>
	int startx;

	/// <summary> 全屏时的初始位置y. </summary>
	int starty;

	///-------------------------------------------------------------------------------------------------
	/// <summary> 创建一个窗口. </summary>
	///
	/// <remarks> Xian.dai, 2017/3/23. </remarks>
	///
	/// <returns>
	/// true if it succeeds, false if it fails.
	/// </returns>
	///-------------------------------------------------------------------------------------------------
	BOOL Create(WNDPROC proc)
	{
		if (_hWnd != NULL)//如果已经创建了一个窗口那么不再重复创建
		{
			return FALSE;
		}

		//返回模块的句柄
		_hInst = GetModuleHandle(NULL);

		WNDCLASSEX wcex;
		memset(&wcex, 0, sizeof(wcex));

		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS | CS_OWNDC;

		//设置其回调函数
		if (proc != NULL)
			wcex.lpfnWndProc = proc;
		else
			wcex.lpfnWndProc = this->MyWndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = _hInst;

		wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		//wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_WIN01);//不要菜单栏
		wcex.lpszClassName = szWindowClass;//主窗口类名
		//wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

		//注册窗口类,这个是必须的，要不然下面的CreateWindow函数总返回NULL
		ATOM atom = RegisterClassEx(&wcex);

		//获取显示器信息。
		mInfo.clear();
		//get number of monitors
		mInfo.reserve(GetSystemMetrics(SM_CMONITORS));
		
		EnumDisplayMonitors(NULL, NULL, this->MonitorEnumProc, reinterpret_cast<LPARAM>(&mInfo));

		cout << "the number of monitors:" << mInfo.size() << endl;


		if(mInfo.size() == 1)
		{
			RECT rect = mInfo[0].rect;
			width = rect.right - rect.left;
			height = rect.bottom - rect.top;
			startx = rect.left;
			starty = rect.top;
		}
		for (int i = 0; i < mInfo.size(); i++)
		{
			//获取特定显示器的信息，最重要的是分辨率，起始坐标等。
			if(!mInfo[i].isPrimary)
			{
				RECT rect = mInfo[0].rect;
				width = rect.right - rect.left;
				height = rect.bottom - rect.top;
				startx = rect.left;
				starty = rect.top;
				break;
			}
		}

		//获取以像素为单位的屏幕尺寸。
		//打印得出确实得到了长宽。
		cout << "width:" << width << "  height:" << height << endl;

		_hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
			0, 0, width, height, NULL, NULL, _hInst, NULL);

		if (!_hWnd)
		{
			return FALSE;
		}
		ToggleFullScreen();

		ShowWindow(_hWnd,  SW_SHOW);
		UpdateWindow(_hWnd);
	
		return TRUE;
	}

	void Close()
	{
		if (_hWnd != NULL)
		{
			CloseWindow(_hWnd);
			DestroyWindow(_hWnd);
		}
		_hWnd = NULL;

	}

	void ToggleFullScreen()
	{
		ToggleFullScreen(width, height, startx, starty);
	}

	void ToggleFullScreen(int width, int height, int startx, int starty)
	{
		if (_hWnd == NULL)
		{
			cout<<"TXWindow.ChangeToFullScreenInRun():错误，_hWnd为空！ \r\n";
			return;
		}

		//从全屏退到非全屏有问题。
		if(isFullScreen)
		{

			SetWindowLong(_hWnd, GWL_STYLE, GetWindowLong(_hWnd, GWL_STYLE) | WS_OVERLAPPEDWINDOW);

			//GWL_EXSTYLE  extended style扩展格式。WS_EX_WINDOWEDGE  使得窗口显示，并不覆盖下面的任务栏。
			SetWindowLong(_hWnd, GWL_EXSTYLE, GetWindowLong(_hWnd, GWL_EXSTYLE) | WS_EX_WINDOWEDGE);

			//在多显示器上不知道会不会有问题，是否会在主屏上显示
			//SetWindowPos(_hWnd, NULL, startx, starty, width, height, SWP_SHOWWINDOW);// SWP_FRAMECHANGED);
		}
		else
		{
			LONG curWinStyle = GetWindowLong(_hWnd, GWL_STYLE);
			curWinStyle = curWinStyle & ~WS_OVERLAPPEDWINDOW;
			SetWindowLong(_hWnd, GWL_STYLE, curWinStyle | WS_POPUP);
			MoveWindow(_hWnd, startx, starty, width, height, TRUE);
		}
		isFullScreen = !isFullScreen;
	}

	RECT GetMonitorRECT(HMONITOR hMonitor)
	{
		for (int i = 0; i < mInfo.size(); i++)
		{
			if(mInfo[i].hMonitor == hMonitor)
			{
				return mInfo[i].rect;
			}
		}
		return RECT();
	}
private:
	//所有的显示器信息。
	vector<ALLMONITORINFO> mInfo;

	///-------------------------------------------------------------------------------------------------
	/// <summary> 一个WndProc消息处理函数模板. </summary>
	///
	/// <remarks> Xian Dai, 2017/3/31. </remarks>
	///
	/// <param name="hWnd">    Handle of the window. </param>
	/// <param name="message"> The message. </param>
	/// <param name="wParam">  The wParam field of the
	/// 					   message. </param>
	/// <param name="lParam">  The lParam field of the
	/// 					   message. </param>
	///
	/// <returns> A CALLBACK. </returns>
	///-------------------------------------------------------------------------------------------------
	static	LRESULT CALLBACK  MyWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		int wmId, wmEvent;
		//dxlib::Debug::Log("WndProc():进入消息处理函数 message = %d \r\n", message);
		switch (message)
		{
		case WM_COMMAND:
			break;
		case WM_PAINT:
			//hdc = BeginPaint(hWnd, &ps);
			//// TODO:  在此添加任意绘图代码...
			//EndPaint(hWnd, &ps);
			break;

		case WM_KEYDOWN:
			//处理键盘上某一键按下的消息
		 
			return 0;

		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		return 0;
	}

	///----------------------------------------------------------------------------------------
	///获取所有显示器的信息，并把它们存储在vector数组内。
	///该方法会作为EnumDisplayMonitors的回调函数，该vector数组也会以此形式传出去。
	///
	///----------------------------------------------------------------------------------------
	static BOOL CALLBACK MonitorEnumProc(__in  HMONITOR hMonitor, __in  HDC hdcMonitor, __in  LPRECT lprcMonitor, __in  LPARAM dwData)
	{
		vector<ALLMONITORINFO>& infoArray = *reinterpret_cast<vector<ALLMONITORINFO>* >(dwData);

		ALLMONITORINFO monitorInfo;
		monitorInfo.hMonitor = hMonitor;
		monitorInfo.rect = *lprcMonitor;

		HMONITOR priMonitor = MonitorFromWindow(nullptr, MONITOR_DEFAULTTOPRIMARY);
		if (priMonitor == hMonitor)
			monitorInfo.isPrimary = true;
		else
			monitorInfo.isPrimary = false;
		
		infoArray.push_back(monitorInfo);
		return TRUE;
	}

};

