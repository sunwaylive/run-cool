
// ChildView.cpp : CChildView 类的实现
//

#pragma  once
#include "stdafx.h"
#include "Animation.h"
#include "ChildView.h"
#include "GameSocket.h"
#include <string>
#include <iostream>
#include <sstream>
#include <vector>

using namespace std;
using std::endl;
using std::cout;
using std::cin;
using std::string;
using std::stringstream;
using std::vector;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern SOCKET send_sock;
extern SOCKET recv_sock;
CChildView *global_view;

DWORD WINAPI UdpServerFun(LPVOID pM)  
{  
  GameSocket::GetInstance()->ListenForBroadcastUDP();
  return 0;  
} 


// CChildView

CChildView::CChildView()
{
}

CChildView::~CChildView()
{
}


BEGIN_MESSAGE_MAP(CChildView, CWnd)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_WM_DESTROY()
  ON_WM_CHAR()
END_MESSAGE_MAP()



// CChildView 消息处理程序

BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
		::LoadCursor(NULL, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW+1), NULL);

	return TRUE;
}

void CChildView::OnPaint() 
{
	CPaintDC dc(this); // 用于绘制的设备上下文

	//---------双缓冲贴图---------------
	CDC bufferDC;
	CBitmap bufferBmp;

	//获取窗口客户区大小
	CRect cltRect;
	this->GetClientRect(&cltRect);

	bufferDC.CreateCompatibleDC(NULL);
	bufferBmp.CreateCompatibleBitmap(&dc,
		cltRect.Width(), cltRect.Height());
	bufferDC.SelectObject(bufferBmp);

	//绘制场景
	m_scene.StickScene(bufferDC, cltRect);

	//绘制人物
	m_bottom_player.StickCharacter(bufferDC);
  m_top_player.StickCharacter(bufferDC);

  //分别绘制两个场景的障碍
  for (size_t i = 0; i < m_v_top_barrier.size(); ++i)
    m_v_top_barrier[i]->StickBarrier(bufferDC);

  for (size_t i = 0; i < m_v_bottom_barrier.size(); ++i)
    m_v_bottom_barrier[i]->StickBarrier(bufferDC);

  

  //贴到客户区
  dc.BitBlt(0, 0, cltRect.Width(), cltRect.Height(), &bufferDC, 0, 0, SRCCOPY);

	//释放内存资源
	bufferBmp.DeleteObject();
	bufferDC.DeleteDC();
}


int CChildView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	//-----------初始化工作------------

	//场景，玩家1,2 初始化失败
	if(!m_scene.InitScene() ||
		!m_bottom_player.InitCharacter() || !m_top_player.InitCharacter())
	{
		AfxMessageBox(L"图片资源加载失败");
		exit(0);
	}

  //整个游戏只能这是一次的状态
  m_cur_score = 0.0f;
  m_highest_score = 0.0f;
  m_start_time = timeGetTime();
  m_end_time = timeGetTime();
  m_is_watching = false;
  m_is_being_watched = false;

  ResetGame();
  
	//设置定时器
	SetTimer(ID_TIMER_BG, 10, NULL);
	SetTimer(ID_TIMER_CHARACTER, 40, NULL);
  SetTimer(ID_TIMER_GEN_BARRIER, 3000, NULL);
  SetTimer(ID_TIMER_DISPLAY_SCORE, 1, NULL);

  //新开一个线程来监听
  global_view = this; //初始化全局变量
  HANDLE udp_thread;
  udp_thread = CreateThread(NULL, 0, UdpServerFun, NULL, 0, NULL);

	return 0;
}

//**********************
void CChildView::OnTimer(UINT_PTR nIDEvent)
{
  //改变游戏的暂停状态
  switch(nKey){
  case 'p':
  case 'P':
    {
      nKey = 0;
      m_is_pause = !m_is_pause;
      UpdateClient();

      if (m_is_pause) PauseGame();
      else StartGame();
      break;
    }
  case 's':
  case 'S':
    {
      nKey = 0;
      PauseGame();
      SaveGame();
      StartGame();
      break;
    }
  case 'l':
  case 'L':
    {
      nKey = 0;
      PauseGame();
      LoadGameFile();
      StartGame();

      ShowScore();
      PauseGame();
      //加载完之后重绘一次
      InvalidateRect(NULL, false);
      //不重绘显示得分区域
      CRect r(VIEWWIDTH - SCORE_TEXT_WIDTH, 0, VIEWWIDTH, SCORE_TEXT_HEIGHT);
      ValidateRect(&r);      
      break;
    }
  case 'w':
  case 'W':
    {
      nKey = 0;
      GameSocket::GetInstance()->BroadcastUDP();
      break;
    }
    //for debug
  case 'f':
  case 'F':
    {
      nKey = 0;
      UpdateClient();
      break;
    }
  default:
    {
      break;
    }
  }

  if(m_is_pause)//暂停游戏
    return;
  
	switch(nIDEvent)
	{
  case ID_TIMER_DISPLAY_SCORE:
    {
      ShowScore();
      return;
      break;
    }
	//移动背景和障碍
	case ID_TIMER_BG:
    {
      m_scene.MoveBg();
      for(size_t i = 0; i < m_v_bottom_barrier.size(); ++i) m_v_bottom_barrier[i]->MoveLeft(BOTTOM_AREA);
      for(size_t i = 0; i < m_v_top_barrier.size(); ++i) m_v_top_barrier[i]->MoveLeft(TOP_AREA);
      //删除移出边界的障碍物
      DelUnseenBarrier(m_v_bottom_barrier);
      DelUnseenBarrier(m_v_top_barrier);
      break;
    }
	//移动人物并切换到下一帧
  case ID_TIMER_CHARACTER:
    {
      m_bottom_player.MoveFront();
      m_bottom_player.MoveVertical(BOTTOM_GROUND_PLANE, BOTTOM_SKY_PLANE);
      m_bottom_player.NextFrame();

      m_top_player.MoveFront();
      m_top_player.MoveVertical(TOP_GROUND_PLANE, TOP_SKY_PLANE);
      m_top_player.NextFrame();
      break;
    }
    //产生障碍
  case ID_TIMER_GEN_BARRIER:
    {
      GenBarrier(m_v_bottom_barrier, 1);
      GenBarrier(m_v_top_barrier, 2);
      break;
    }
  default:
    break;
	}

  //碰撞检测
  m_is_top_collide = CheckCollison(m_top_player, m_v_top_barrier, m_top_touchPoint);
  m_is_bottom_collide = CheckCollison(m_bottom_player, m_v_bottom_barrier, m_bottom_touchPoint);
  if( m_is_top_collide || m_is_bottom_collide){
    /*KillTimer(ID_TIMER_BG);
    KillTimer(ID_TIMER_CHARACTER);
    KillTimer(ID_TIMER_GEN_BARRIER);*/
    ShowCollisonPoint();

    m_is_pause = true;
    m_end_time = timeGetTime();
    CString score_str;
    float score = (m_end_time - m_start_time) * 1.0f / 1000 + m_cur_score;
    m_highest_score = max(score, m_highest_score);
    score_str.Format(_T("当前得分：%.2f. 最高得分：%.2f. 再来一次！"), score, m_highest_score);
    m_is_pause = true;
    UpdateClient();

    if(MessageBox(score_str, _T("游戏结束！"), MB_OKCANCEL) == IDOK){
      ResetGame();
      UpdateClient();
    }else{
      ExitGame();
    }
  }else{
    //如果没有发生碰撞
    switch(nKey){
      //控制下半部分玩家跳跃
    case 'k':
    case 'K':
      {
        nKey = 0;
        //防止连续跳跃
        if(m_bottom_player.IsOnGround(BOTTOM_GROUND_PLANE)){
          m_bottom_player.SetVerticalSpeed(JUMP_BEGIN_VELOCITY);
        }
        UpdateClient();
        break;
      }
      //控制上半部分玩家跳跃
    case 'i':
    case 'I':
      {
        nKey = 0;
        //防止连续跳跃
        if (m_top_player.IsOnGround(TOP_GROUND_PLANE)){
          m_top_player.SetVerticalSpeed(JUMP_BEGIN_VELOCITY);
        }
        UpdateClient();
        break;
      }
    default:
      nKey = 0;
      break;
    }
  }

	//重绘客户区
	InvalidateRect(NULL, false);
  //不重绘显示得分区域
  CRect r(VIEWWIDTH - SCORE_TEXT_WIDTH, 0, VIEWWIDTH, SCORE_TEXT_HEIGHT);
  ValidateRect(&r);

	CWnd::OnTimer(nIDEvent);
}

void CChildView::StartGame()
{
  mciSendString(L"play bgm repeat", NULL, 0, NULL);
  m_is_pause = false;
  m_start_time = timeGetTime();//从当前时间开始游戏
}

void CChildView::PauseGame() 
{
  // TODO: Add your command handler code here
  mciSendString(L"pause bgm", NULL, 0, NULL);
  m_is_pause = true;
  //记录当前得分
  m_end_time = timeGetTime();
  m_cur_score = m_cur_score + (m_end_time - m_start_time) * 1.0f / 1000;
}

void CChildView::ExitGame()
{
  _exit(1);
}

void CChildView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
  // TODO: Add your message handler code here and/or call default
  nKey = nChar;
  CWnd::OnChar(nChar, nRepCnt, nFlags);
}

void CChildView::OnDestroy()
{
	CWnd::OnDestroy();

	//关闭计时器
	KillTimer(ID_TIMER_BG);
	KillTimer(ID_TIMER_CHARACTER);
  KillTimer(ID_TIMER_GEN_BARRIER);

	//释放内存资源
	m_scene.ReleaseScene();
	m_bottom_player.ReleaseCharacter();
  m_top_player.ReleaseCharacter();

  ReleaseBarrier();
}

//*********************  Helper  *********************
void CChildView::ResetGame()
{
  nKey = 0;//清空当前记录的按键，否则一直是上一次的按键
  m_is_pause = true;
  m_is_top_collide = false;
  m_is_bottom_collide = false;
  m_top_touchPoint = CPoint(-1, -1);
  m_bottom_touchPoint = CPoint(-1, -1);

  //设置人物1,2的位置
  m_bottom_player.SetCharPos(0, VIEWHEIGHT - ELEVATION - m_bottom_player.GetCharSize().cy);
  m_top_player.SetCharPos(0, (VIEWHEIGHT - 2 * ELEVATION) / 2 - m_top_player.GetCharSize().cy);

  m_cur_score = 0.0f;
  ShowScore();
  m_end_time = timeGetTime();
  
  ReleaseBarrier();
}

void CChildView::SaveGame()
{
  //选择存档目录
  //OPENFILENAME ofn;       // 公共对话框结构。
  //WCHAR szFile[MAX_PATH] = {}; // 保存获取文件名称的缓冲区。          

  //// 初始化选择文件对话框。
  //ZeroMemory(&ofn, sizeof(ofn));
  //ofn.lStructSize = sizeof(ofn);
  //ofn.hwndOwner = NULL;
  //ofn.lpstrFile = szFile;

  //ofn.nMaxFile = sizeof(szFile);
  //ofn.lpstrFilter = _T("All/0*.*/0Text/0*.TXT/0");
  //ofn.nFilterIndex = 1;
  //ofn.lpstrFileTitle = NULL;
  //ofn.nMaxFileTitle = 0;
  //ofn.lpstrInitialDir = NULL;
  //ofn.Flags = OFN_OVERWRITEPROMPT;

  //// 显示打开选择文件对话框。
  //std::string str_game_file;
  //if ( GetSaveFileName(&ofn) ){
  //  size_t len = wcslen(szFile) + 1;
  //  size_t converted = 0;
  //  char *CStr;
  //  CStr=(char*)malloc(len * sizeof(char));
  //  wcstombs_s(&converted, CStr, len, szFile, _TRUNCATE);
  //  str_game_file = std::string(CStr);
  //  free(CStr);
  //}else{
  //  return;
  //}

  std::ofstream save_game_file;
  save_game_file.open(GAME_FILE);

  int cnt = 0;
  //********************存档Top Game Area的信息********************
  save_game_file<<"#" <<++cnt <<" Top Player Position" << endl
    << m_top_player.GetCharPos().x<<endl
    << m_top_player.GetCharPos().y << endl << endl; 

  save_game_file<<"#" <<++cnt <<" Top Player Vertical Speed" <<endl
    << m_top_player.GetVerticalSpeed() <<endl <<endl;

  save_game_file<<"#" <<++cnt <<" Top Barrier Size" <<endl
    << m_v_top_barrier.size() <<endl <<endl;

  save_game_file<<"# Barrier Information: barrier_type, barrier's left top x , y" <<endl;
  for (size_t i = 0; i < m_v_top_barrier.size(); ++i)
  {
    save_game_file<<m_v_top_barrier[i]->GetType()<<endl
      <<m_v_top_barrier[i]->GetBarrierPos().x <<endl
      <<m_v_top_barrier[i]->GetBarrierPos().y <<endl;
  }
  save_game_file<<endl;

  //********************存档Bottom Game Area的信息********************
  save_game_file<<"#" <<++cnt <<" Bottom Player Position" <<endl
    <<m_bottom_player.GetCharPos().x <<endl
    <<m_bottom_player.GetCharPos().y <<endl <<endl;

  save_game_file<<"#" <<++cnt <<" Bottom Player Vertical Speed" <<endl
    <<m_bottom_player.GetVerticalSpeed() <<endl <<endl;

  save_game_file<<"#" <<++cnt <<" Top Barrier Size" <<endl
    << m_v_bottom_barrier.size() <<endl <<endl;

  save_game_file<<"# Barrier Information: barrier_type, barrier's left top x, y" <<endl;
  for (size_t i = 0; i < m_v_bottom_barrier.size(); ++i)
  {
    save_game_file<<m_v_bottom_barrier[i]->GetType()<<endl
      <<m_v_bottom_barrier[i]->GetBarrierPos().x <<endl
      <<m_v_bottom_barrier[i]->GetBarrierPos().y <<endl;
  }
  save_game_file<<endl;

  //********************存档得分信息********************
  save_game_file<<"#" <<++cnt <<" Current Score" <<endl
    <<m_cur_score <<endl <<endl;

  save_game_file<<"#" <<++cnt <<" Highest Score" <<endl
    <<m_highest_score <<endl <<endl;

  save_game_file.close();
}

void CChildView::LoadGameFile()
{
  //弹出文件选择框,加载当前得分
  //OPENFILENAME ofn;       // common dialog box structure 
  //WCHAR szFile[260] = {};       // buffer for file name 
  //HWND hwnd = this->m_hWnd;              // owner window 
  //HANDLE hf;              // file handle 

  //// Initialize OPENFILENAME 
  //ZeroMemory(&ofn, sizeof(OPENFILENAME)); 
  //ofn.lStructSize = sizeof(OPENFILENAME); 
  //ofn.hwndOwner = NULL; 
  //ofn.lpstrFile = szFile; 
  //ofn.nMaxFile = sizeof(szFile); 
  //ofn.lpstrFilter = _T("All/0*.*/0Text/0*.TXT/0"); 
  //ofn.nFilterIndex = 1; 
  //ofn.lpstrFileTitle = NULL; 
  //ofn.nMaxFileTitle = 0; 
  //ofn.lpstrInitialDir = NULL; 
  //ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST; 

  //// Display the Open dialog box.  
  //std::string str_game_file;
  //if (GetOpenFileName(&ofn)==TRUE){
  //  size_t len = wcslen(szFile) + 1;
  //  size_t converted = 0;
  //  char *CStr;
  //  CStr=(char*)malloc(len * sizeof(char));
  //  wcstombs_s(&converted, CStr, len, szFile, _TRUNCATE);
  //  str_game_file = std::string(CStr);
  //  free(CStr);
  //}else{
  //  return;
  //}
  //cout<<str_game_file<<endl;
  //return;

  //上面是windows API转换，下面是读取文件
  std::ifstream load_game_file;
  load_game_file.open(GAME_FILE);

  //释放之前的游戏状态
  ReleaseBarrier();

  std::string value;
  int x_pos, y_pos, v_speed;
  int barrier_num, barrier_type;
  float curScore, highestScore;
  
  //********************加载Top Game Area的信息********************
  //获取Top player的位置
  load_game_file.ignore(1000, '\n');
  getline(load_game_file, value);
  x_pos = atoi(value.c_str());
  getline(load_game_file, value);
  y_pos = atoi(value.c_str());
  m_top_player.SetCharPos(x_pos, y_pos);

  //获取Top Player的垂直速度
  load_game_file.ignore(1000, '\n');
  load_game_file.ignore(1000, '\n');
  getline(load_game_file, value);
  v_speed = atoi(value.c_str());
  m_top_player.SetVerticalSpeed(v_speed);

  //获取障碍物的数量
  load_game_file.ignore(1000, '\n');
  load_game_file.ignore(1000, '\n');
  getline(load_game_file, value);
  barrier_num = atoi(value.c_str());

  //获取所有障碍物的所有信息
  load_game_file.ignore(1000, '\n');
  load_game_file.ignore(1000, '\n');
  for (int i = 0; i < barrier_num; ++i)
  {
    getline(load_game_file, value);
    barrier_type = atoi(value.c_str());
    getline(load_game_file, value);
    x_pos = atoi(value.c_str());
    getline(load_game_file, value);
    y_pos = atoi(value.c_str());

    CBarrier *newBarrier = new CBarrier;
    newBarrier->InitBarrier(barrier_type);
    newBarrier->SetBarrierPos(x_pos, y_pos);
    m_v_top_barrier.push_back(newBarrier);
  }

  //********************加载Bottom Game Area的信息********************
  //获取Bottom player的位置
  load_game_file.ignore(1000, '\n');
  load_game_file.ignore(1000, '\n');
  getline(load_game_file, value);
  x_pos = atoi(value.c_str());
  getline(load_game_file, value);
  y_pos = atoi(value.c_str());
  m_bottom_player.SetCharPos(x_pos, y_pos);

  //获取Bottom Player的垂直速度
  load_game_file.ignore(1000, '\n');
  load_game_file.ignore(1000, '\n');
  getline(load_game_file, value);
  v_speed = atoi(value.c_str());
  m_bottom_player.SetVerticalSpeed(v_speed);
  
  //获取障碍物的数量
  load_game_file.ignore(1000, '\n');
  load_game_file.ignore(1000, '\n');
  getline(load_game_file, value);
  barrier_num = atoi(value.c_str());

  //获取所有障碍物的所有信息
  load_game_file.ignore(1000, '\n');
  load_game_file.ignore(1000, '\n');
  for (int i = 0; i < barrier_num; ++i)
  {
    getline(load_game_file, value);
    barrier_type = atoi(value.c_str());
    getline(load_game_file, value);
    x_pos = atoi(value.c_str());
    getline(load_game_file, value);
    y_pos = atoi(value.c_str());

    CBarrier *newBarrier = new CBarrier;
    newBarrier->InitBarrier(barrier_type);
    newBarrier->SetBarrierPos(x_pos, y_pos);
    m_v_bottom_barrier.push_back(newBarrier);
  }

  //********************加载得分信息********************
  load_game_file.ignore(1000, '\n');
  load_game_file.ignore(1000, '\n');
  getline(load_game_file, value);
  curScore = atof(value.c_str());
  m_cur_score = curScore;

  load_game_file.ignore(1000, '\n');
  load_game_file.ignore(1000, '\n');
  getline(load_game_file, value);
  highestScore = atof(value.c_str());
  m_highest_score = highestScore;

  load_game_file.close();
}

//删除移出边界的障碍物
void CChildView::DelUnseenBarrier( std::vector<CBarrier*> &v_b)
{
  std::vector<CBarrier*> newVec;
  for (size_t i = 0; i < v_b.size(); ++i)
  {
    CBarrier *tmp = v_b[i];
    if(tmp == NULL) continue; 

    if(!tmp->CanLeft()){
      tmp->ReleaseBarrier();
      delete tmp;
      tmp = NULL;
    }else{
      newVec.push_back(v_b[i]);
    }
  }
  std::swap(v_b, newVec);
}

void CChildView::GenBarrier(std::vector<CBarrier*> &v_b, int areaID)
{
  //如果正在观察其他人玩游戏，则由TCP端接收到的数据负责产生障碍
  if (m_is_watching){
    return;
  }

  //50%概率不产生障碍,随机种子需要设置
  if (rand() % 2 == 0) return;

  CBarrier *barrier = new CBarrier;
  int bType = rand() % TOTAL_BARRIER_TYPE + 1;
  barrier->InitBarrier(bType);
  //默认是为下半部分的区域产生障碍，如果是为上半部分的区域产生障碍，则需要加上偏移
  if (areaID == 2){
    barrier->SetBarrierPos(VIEWWIDTH - barrier->GetBarrierSize().cx,
    (VIEWHEIGHT - 2 * ELEVATION) / 2 - barrier->GetBarrierSize().cy);
  }
  v_b.push_back(barrier);

  //如果正在被观察玩游戏，需要在创建出barrier之后，发送给被观察者
  if (m_is_being_watched){
    UpdateClient();
  }
}

//玩家信息格式: Player playerID curFrame vSpeed xPos yPos
void CChildView::SendPlayerToTCP( int playerID )
{
  char sendBuf[PACKAGE_SIZE] = {};
  if(playerID == TOP_PLAYER){
    sprintf(sendBuf, "Player %d %d %d %d %d ", playerID, m_top_player.m_curFrame,
      m_top_player.v_speed, m_top_player.m_leftTop.x, m_top_player.m_leftTop.y);
  }else if(playerID == BOTTOM_PLAYER){
    sprintf(sendBuf, "Player %d %d %d %d %d ", playerID, m_bottom_player.m_curFrame,
      m_bottom_player.v_speed, m_bottom_player.m_leftTop.x, m_bottom_player.m_leftTop.y);
  }
  send(send_sock, sendBuf, strlen(sendBuf)+ 1, 0);
  return;
}

//障碍物的信息格式: Barrier areaID barrierType
void CChildView::SendBarrierToTCP(int barrierType, int areaID)
{
  char sendBuf[PACKAGE_SIZE] = {};
  sprintf(sendBuf, "Barrier %d %d ", areaID, barrierType);
  send (send_sock, sendBuf, strlen(sendBuf) + 1, 0);
  return;
}

//游戏状态的信息格式: GameStatus curScore highestScore startTime endTime isPause 
void CChildView::SendGameStatus()
{
  char sendBuf[PACKAGE_SIZE] = {};
  int i_pause = m_is_pause ? 1 : 0;
  sprintf(sendBuf, "GameStatus %.2f %.2f %ld %ld %d ", 
    m_cur_score, m_highest_score, m_start_time, m_end_time ,i_pause);
  send(send_sock, sendBuf, strlen(sendBuf) + 1, 0);
  return;
}

void CChildView::UpdateClient()
{
  //如果没有被观察， 则不需要更新
  if(!m_is_being_watched) return;

  char allBuf[1024] = {};
  char topPlayerBuf[PACKAGE_SIZE] = {};
  sprintf(topPlayerBuf, "Player %d %d %d %d %d ", TOP_PLAYER, m_top_player.m_curFrame,
    m_top_player.v_speed, m_top_player.m_leftTop.x, m_top_player.m_leftTop.y);
  strcat(allBuf, topPlayerBuf);

  char bottomPlayerBuf[PACKAGE_SIZE] = {};
  sprintf(bottomPlayerBuf, "Player %d %d %d %d %d ", BOTTOM_PLAYER, m_bottom_player.m_curFrame,
    m_bottom_player.v_speed, m_bottom_player.m_leftTop.x, m_bottom_player.m_leftTop.y);
  strcat(allBuf, bottomPlayerBuf);

  char gameStatusBuf[PACKAGE_SIZE] = {};
  int i_pause = m_is_pause ? 1 : 0;
  sprintf(gameStatusBuf, "GameStatus %.2f %.2f %d ", 
    m_cur_score, m_highest_score, i_pause);
  strcat(allBuf, gameStatusBuf);

  for (size_t i = 0; i < m_v_top_barrier.size(); ++i){
    CBarrier *tmp = m_v_top_barrier[i];
    if (tmp == NULL) continue;

    char eachBarrierBuf[PACKAGE_SIZE] = {};
    sprintf(eachBarrierBuf, "Barrier %d %d %d %d ", 
      TOP_AREA, tmp->GetType(), tmp->GetBarrierPos().x, tmp->GetBarrierPos().y);
    strcat(allBuf, eachBarrierBuf);
  }

  for (size_t i = 0; i < m_v_bottom_barrier.size(); ++i){
    CBarrier *tmp = m_v_bottom_barrier[i];
    if (tmp == NULL) continue;

    char eachBarrierBuf[PACKAGE_SIZE] = {};
    sprintf(eachBarrierBuf, "Barrier %d %d %d %d ", 
      BOTTOM_AREA, tmp->GetType(), tmp->GetBarrierPos().x, tmp->GetBarrierPos().y);
    strcat(allBuf, eachBarrierBuf);
  }

  send(send_sock, allBuf, strlen(allBuf) + 1, 0);
}

void CChildView::ReceiveDataFromTCP()
{
  //设置进入观察者模式
  m_is_watching = true;
  char recvBuf[PACKAGE_SIZE] = {};

  //会阻塞在这里,一直等到接受到数据
  recv(recv_sock, recvBuf, sizeof(recvBuf), 0);
  cout<<endl <<"sucess2" <<endl;

  //释放之前的障碍信息
  ReleaseBarrier();
  //解析获得的字符串
  string buf(recvBuf);  
  string token;
  stringstream ss(buf);
  vector<string> tokens;
  //自动按空格分割，将每个单词放入vector
  while (ss >> token){
    tokens.push_back(token);
  }

  for (int i = 0; i < tokens.size(); ++i)
    cout<<tokens[i]<<" ";
  cout<<endl;

  //更新相应的信息
  vector<string>::iterator iter = tokens.begin();
  while(iter != tokens.end()){
    if (iter->compare("Player") == 0){
      //玩家信息格式: Player playerID curFrame vSpeed xPos yPos
      int playerID = stoi(*++iter);
      if (playerID == TOP_PLAYER){
        m_top_player.m_curFrame = stoi(*++iter);
        m_top_player.v_speed = stoi(*++iter);
        m_top_player.m_leftTop.x = stoi(*++iter);
        m_top_player.m_leftTop.y = stoi(*++iter);
      }
      else if(playerID == BOTTOM_PLAYER){
        m_bottom_player.m_curFrame = stoi(*++iter);
        m_bottom_player.v_speed = stoi(*++iter);
        m_bottom_player.m_leftTop.x = stoi(*++iter);
        m_bottom_player.m_leftTop.y = stoi(*++iter);
      }
    }else if(iter->compare("Barrier") == 0){
      //障碍物的信息格式: Barrier areaID barrierType
      int areaID = stoi(*++iter);
      int barrierID = stoi(*++iter);
      int leftTopX = stoi(*++iter);
      int leftTopY = stoi(*++iter);
      CBarrier *barrier = new CBarrier;
      barrier->InitBarrier(barrierID);
      barrier->SetBarrierPos(leftTopX, leftTopY);

      if (areaID == TOP_AREA){
        m_v_top_barrier.push_back(barrier);
      }else if (areaID == BOTTOM_AREA){
        m_v_bottom_barrier.push_back(barrier);
      }
    }else if (iter->compare("GameStatus") == 0){
      //游戏状态的信息格式: GameStatus curScore highestScore startTime endTime isPause
      m_cur_score = stof(*++iter);
      m_highest_score = stof(*++iter);
      /*m_start_time = stoul(*++iter);
      m_end_time = stoul(*++iter);*/
      m_is_pause = stoi(*++iter) == 1 ? true : false;
    }
    //每次解析完，后移iter
    ++iter;
  }
}

void CChildView::ReleaseBarrier()
{
  for(size_t i = 0; i < m_v_top_barrier.size(); ++i){
    m_v_top_barrier[i]->ReleaseBarrier();
    delete m_v_top_barrier[i];
    m_v_top_barrier[i] = NULL;
  }
  m_v_top_barrier.clear();

  for(size_t i = 0; i < m_v_bottom_barrier.size(); ++i){
    m_v_bottom_barrier[i]->ReleaseBarrier();
    delete m_v_bottom_barrier[i];
    m_v_bottom_barrier[i] = NULL;
  }
  m_v_bottom_barrier.clear();
}

/*
即两个矩形中心点在x方向的距离的绝对值小于等于矩形宽度和的二分之一，同时y方向的距离的绝对值小于等于矩形高度
和的二分之一。下面是数学表达式： 
  x方向：| (x1 + w1 / 2) – (x2 + w2/2) | < |(w1 + w2) / 2|
  y方向：| (y1 + h1 / 2) – (y2 + h2/2) | < |(h1 + h2) / 2| 
*/
bool CChildView::CheckCollison( CCharacter &character, std::vector<CBarrier*> &v_b, CPoint &touchPoint )
{
  int x1 = character.GetCharPos().x;
  int y1 = character.GetCharPos().y;
  int w1 = character.GetCharSize().cx;
  int h1 = character.GetCharSize().cy;

  for(size_t i = 0; i < v_b.size(); ++i){
    CBarrier *pB = v_b[i];
    if (pB == NULL) continue;

    int x2 = pB->GetBarrierPos().x;
    int y2 = pB->GetBarrierPos().y;
    int w2 = pB->GetBarrierSize().cx;
    int h2 = pB->GetBarrierSize().cy;

    if( abs((x1+w1/2) - (x2+w2/2)) < abs( (w1+w2)/2 ) 
      && abs( (y1+h1/2) - (y2+h2/2) ) < abs( (h1+h2)/2 ) ) {
        touchPoint.x = ( (x1+w1/2) + (x2+w2/2) ) / 2;
        touchPoint.y = ( (y1+h1/2) + (y2+h2/2) ) / 2;
        return true;//发生碰撞
    }else{
      return false;
    }
  }
  //如果没有障碍，则没有碰撞
  return false;
}

void CChildView::ShowScore()
{
  CDC* pDC = GetDC();
  if (pDC == NULL)
    return;

  float score = 0.f;
  if (m_is_pause) {
    score = m_cur_score;
  }else{
    m_end_time = timeGetTime();
    score = m_cur_score + (m_end_time - m_start_time) * 1.0f / 1000 ;
  }

  CString str;
  m_highest_score = max(score, m_highest_score);
  str.Format(_T("当前得分: %.2f, 最高得分： %.2f"), score, m_highest_score);

  //pDC->FillSolidRect(460,300,120,30,RGB(0,255,0));

  pDC->SetTextColor(RGB(255,0,0));
  pDC->TextOut(VIEWWIDTH - SCORE_TEXT_WIDTH , 0, str);
  
  //pDC->FillSolidRect(460,360,120,30,RGB(0,0,255));
  //pen.CreatePen(PS_SOLID,3,RGB(255,255,255));
  //str.Format("水平:%d",nLevel);
  //pDC->SetTextColor(RGB(255,255,255));
  //pDC->TextOut(460,370,str);

  ReleaseDC(pDC);
}

void CChildView::ShowCollisonPoint()
{
  CDC *pDC = GetDC();
  if(pDC == NULL) return;

  CPen pen(PS_SOLID, 5, RGB(255, 0, 0));  
  CPen *old_pen = pDC->SelectObject(&pen);

  CBrush *brush = CBrush::FromHandle((HBRUSH)GetStockObject(NULL_BRUSH));//透明画刷
  CBrush *old_brush = pDC->SelectObject(brush);

  //绘制碰撞点
  if (m_is_top_collide)//m_is_top_collide
  {
    int top_player_center_x = m_top_player.GetCharPos().x + m_top_player.GetCharSize().cx / 2;
    int top_player_center_y = m_top_player.GetCharPos().y + m_top_player.GetCharSize().cy / 2;
    int radius = max(abs(m_top_touchPoint.x - top_player_center_x), abs(m_top_touchPoint.y - top_player_center_y));
    CRect rect( m_top_touchPoint.x - radius, m_top_touchPoint.y - radius,
      m_top_touchPoint.x + radius, m_top_touchPoint.y + radius );
    pDC->Ellipse(rect);
  }

  if (m_is_bottom_collide)
  {
    int bottom_player_center_x = m_bottom_player.GetCharPos().x + m_bottom_player.GetCharSize().cx / 2;
    int bottom_player_center_y = m_bottom_player.GetCharPos().y + m_bottom_player.GetCharSize().cy / 2;
    int radius = max(abs(m_bottom_touchPoint.x - bottom_player_center_x), abs(m_bottom_touchPoint.y - bottom_player_center_y));
    CRect rect;
    rect.SetRect(m_bottom_touchPoint.x, m_bottom_touchPoint.y,
      m_bottom_touchPoint.x, m_bottom_touchPoint.y);//将矩形设成1个点
    rect.InflateRect(radius, radius);//向外膨胀
    pDC->Ellipse(rect);
  }

  pDC->SelectObject(old_pen);
  pDC->SelectObject(old_brush);
  ReleaseDC(pDC);
}