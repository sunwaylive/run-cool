
// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件

#pragma once

#ifndef _SECURE_ATL
#define _SECURE_ATL 1
#endif

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // 从 Windows 头中排除极少使用的资料
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // 某些 CString 构造函数将是显式的

// 关闭 MFC 对某些常见但经常可放心忽略的警告消息的隐藏
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC 核心组件和标准组件
#include <afxext.h>         // MFC 扩展


#include <afxdisp.h>        // MFC 自动化类



#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC 对 Internet Explorer 4 公共控件的支持
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // MFC 对 Windows 公共控件的支持
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h>     // 功能区和控件条的 MFC 支持

#define GROUNDSTART 42




#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif

//这句添加了控制台调试工具，工程属性/Linker/System/Subsystem这里看到
#ifdef _DEBUG
#pragma comment( linker, "/subsystem:console /entry:wWinMainCRTStartup" )
#endif

//游戏存档路径
const char GAME_FILE[] = "SaveGame\\game.txt";
const char GAME_CONFIG[] = "GameConfig\\ServerIP.txt";
//地面高度
#define ELEVATION 42

//障碍的类型数目
const int TOTAL_BARRIER_TYPE = 2;

//显示得分的文本区域的大小
const int SCORE_TEXT_WIDTH = 260;
const int SCORE_TEXT_HEIGHT = 50;

//视口客户区宽度
const int VIEWWIDTH = 790;

//视口客户区高度
const int VIEWHEIGHT = 568;

//重力加速度
const int GRAVITY = 1;

//角色向上起跳的速度
const int JUMP_BEGIN_VELOCITY = 20;

//上半部分地面，天空的y坐标
const int TOP_GROUND_PLANE = ( VIEWHEIGHT - 2 * ELEVATION ) / 2;
const int TOP_SKY_PLANE = 0;

//下半部分地面，天空的y坐标
const int BOTTOM_GROUND_PLANE = (VIEWHEIGHT - ELEVATION);
const int BOTTOM_SKY_PLANE = TOP_GROUND_PLANE + ELEVATION;

const int TOP_AREA = 1;
const int BOTTOM_AREA = 2;

const int TOP_PLAYER = 1;
const int BOTTOM_PLAYER = 2;

const int TCP_PORT = 18674;
const int UDP_PORT = 18672;
const int PACKAGE_SIZE = 256;