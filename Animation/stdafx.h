
// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�

#pragma once

#ifndef _SECURE_ATL
#define _SECURE_ATL 1
#endif

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // �� Windows ͷ���ų�����ʹ�õ�����
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // ĳЩ CString ���캯��������ʽ��

// �ر� MFC ��ĳЩ�����������ɷ��ĺ��Եľ�����Ϣ������
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC ��������ͱ�׼���
#include <afxext.h>         // MFC ��չ


#include <afxdisp.h>        // MFC �Զ�����



#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC �� Internet Explorer 4 �����ؼ���֧��
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // MFC �� Windows �����ؼ���֧��
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h>     // �������Ϳؼ����� MFC ֧��

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

//�������˿���̨���Թ��ߣ���������/Linker/System/Subsystem���￴��
#ifdef _DEBUG
#pragma comment( linker, "/subsystem:console /entry:wWinMainCRTStartup" )
#endif

//��Ϸ�浵·��
const char GAME_FILE[] = "SaveGame\\game.txt";
const char GAME_CONFIG[] = "GameConfig\\ServerIP.txt";
//����߶�
#define ELEVATION 42

//�ϰ���������Ŀ
const int TOTAL_BARRIER_TYPE = 2;

//��ʾ�÷ֵ��ı�����Ĵ�С
const int SCORE_TEXT_WIDTH = 260;
const int SCORE_TEXT_HEIGHT = 50;

//�ӿڿͻ������
const int VIEWWIDTH = 790;

//�ӿڿͻ����߶�
const int VIEWHEIGHT = 568;

//�������ٶ�
const int GRAVITY = 1;

//��ɫ�����������ٶ�
const int JUMP_BEGIN_VELOCITY = 20;

//�ϰ벿�ֵ��棬��յ�y����
const int TOP_GROUND_PLANE = ( VIEWHEIGHT - 2 * ELEVATION ) / 2;
const int TOP_SKY_PLANE = 0;

//�°벿�ֵ��棬��յ�y����
const int BOTTOM_GROUND_PLANE = (VIEWHEIGHT - ELEVATION);
const int BOTTOM_SKY_PLANE = TOP_GROUND_PLANE + ELEVATION;

const int TOP_AREA = 1;
const int BOTTOM_AREA = 2;

const int TOP_PLAYER = 1;
const int BOTTOM_PLAYER = 2;

const int TCP_PORT = 18674;
const int UDP_PORT = 18672;
const int PACKAGE_SIZE = 256;