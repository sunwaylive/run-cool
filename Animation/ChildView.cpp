
// ChildView.cpp : CChildView ���ʵ��
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



// CChildView ��Ϣ�������

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
	CPaintDC dc(this); // ���ڻ��Ƶ��豸������

	//---------˫������ͼ---------------
	CDC bufferDC;
	CBitmap bufferBmp;

	//��ȡ���ڿͻ�����С
	CRect cltRect;
	this->GetClientRect(&cltRect);

	bufferDC.CreateCompatibleDC(NULL);
	bufferBmp.CreateCompatibleBitmap(&dc,
		cltRect.Width(), cltRect.Height());
	bufferDC.SelectObject(bufferBmp);

	//���Ƴ���
	m_scene.StickScene(bufferDC, cltRect);

	//��������
	m_bottom_player.StickCharacter(bufferDC);
  m_top_player.StickCharacter(bufferDC);

  //�ֱ���������������ϰ�
  for (size_t i = 0; i < m_v_top_barrier.size(); ++i)
    m_v_top_barrier[i]->StickBarrier(bufferDC);

  for (size_t i = 0; i < m_v_bottom_barrier.size(); ++i)
    m_v_bottom_barrier[i]->StickBarrier(bufferDC);

  

  //�����ͻ���
  dc.BitBlt(0, 0, cltRect.Width(), cltRect.Height(), &bufferDC, 0, 0, SRCCOPY);

	//�ͷ��ڴ���Դ
	bufferBmp.DeleteObject();
	bufferDC.DeleteDC();
}


int CChildView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	//-----------��ʼ������------------

	//���������1,2 ��ʼ��ʧ��
	if(!m_scene.InitScene() ||
		!m_bottom_player.InitCharacter() || !m_top_player.InitCharacter())
	{
		AfxMessageBox(L"ͼƬ��Դ����ʧ��");
		exit(0);
	}

  //������Ϸֻ������һ�ε�״̬
  m_cur_score = 0.0f;
  m_highest_score = 0.0f;
  m_start_time = timeGetTime();
  m_end_time = timeGetTime();
  m_is_watching = false;
  m_is_being_watched = false;

  ResetGame();
  
	//���ö�ʱ��
	SetTimer(ID_TIMER_BG, 10, NULL);
	SetTimer(ID_TIMER_CHARACTER, 40, NULL);
  SetTimer(ID_TIMER_GEN_BARRIER, 3000, NULL);
  SetTimer(ID_TIMER_DISPLAY_SCORE, 1, NULL);

  //�¿�һ���߳�������
  global_view = this; //��ʼ��ȫ�ֱ���
  HANDLE udp_thread;
  udp_thread = CreateThread(NULL, 0, UdpServerFun, NULL, 0, NULL);

	return 0;
}

//**********************
void CChildView::OnTimer(UINT_PTR nIDEvent)
{
  //�ı���Ϸ����ͣ״̬
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
      //������֮���ػ�һ��
      InvalidateRect(NULL, false);
      //���ػ���ʾ�÷�����
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

  if(m_is_pause)//��ͣ��Ϸ
    return;
  
	switch(nIDEvent)
	{
  case ID_TIMER_DISPLAY_SCORE:
    {
      ShowScore();
      return;
      break;
    }
	//�ƶ��������ϰ�
	case ID_TIMER_BG:
    {
      m_scene.MoveBg();
      for(size_t i = 0; i < m_v_bottom_barrier.size(); ++i) m_v_bottom_barrier[i]->MoveLeft(BOTTOM_AREA);
      for(size_t i = 0; i < m_v_top_barrier.size(); ++i) m_v_top_barrier[i]->MoveLeft(TOP_AREA);
      //ɾ���Ƴ��߽���ϰ���
      DelUnseenBarrier(m_v_bottom_barrier);
      DelUnseenBarrier(m_v_top_barrier);
      break;
    }
	//�ƶ����ﲢ�л�����һ֡
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
    //�����ϰ�
  case ID_TIMER_GEN_BARRIER:
    {
      GenBarrier(m_v_bottom_barrier, 1);
      GenBarrier(m_v_top_barrier, 2);
      break;
    }
  default:
    break;
	}

  //��ײ���
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
    score_str.Format(_T("��ǰ�÷֣�%.2f. ��ߵ÷֣�%.2f. ����һ�Σ�"), score, m_highest_score);
    m_is_pause = true;
    UpdateClient();

    if(MessageBox(score_str, _T("��Ϸ������"), MB_OKCANCEL) == IDOK){
      ResetGame();
      UpdateClient();
    }else{
      ExitGame();
    }
  }else{
    //���û�з�����ײ
    switch(nKey){
      //�����°벿�������Ծ
    case 'k':
    case 'K':
      {
        nKey = 0;
        //��ֹ������Ծ
        if(m_bottom_player.IsOnGround(BOTTOM_GROUND_PLANE)){
          m_bottom_player.SetVerticalSpeed(JUMP_BEGIN_VELOCITY);
        }
        UpdateClient();
        break;
      }
      //�����ϰ벿�������Ծ
    case 'i':
    case 'I':
      {
        nKey = 0;
        //��ֹ������Ծ
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

	//�ػ�ͻ���
	InvalidateRect(NULL, false);
  //���ػ���ʾ�÷�����
  CRect r(VIEWWIDTH - SCORE_TEXT_WIDTH, 0, VIEWWIDTH, SCORE_TEXT_HEIGHT);
  ValidateRect(&r);

	CWnd::OnTimer(nIDEvent);
}

void CChildView::StartGame()
{
  mciSendString(L"play bgm repeat", NULL, 0, NULL);
  m_is_pause = false;
  m_start_time = timeGetTime();//�ӵ�ǰʱ�俪ʼ��Ϸ
}

void CChildView::PauseGame() 
{
  // TODO: Add your command handler code here
  mciSendString(L"pause bgm", NULL, 0, NULL);
  m_is_pause = true;
  //��¼��ǰ�÷�
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

	//�رռ�ʱ��
	KillTimer(ID_TIMER_BG);
	KillTimer(ID_TIMER_CHARACTER);
  KillTimer(ID_TIMER_GEN_BARRIER);

	//�ͷ��ڴ���Դ
	m_scene.ReleaseScene();
	m_bottom_player.ReleaseCharacter();
  m_top_player.ReleaseCharacter();

  ReleaseBarrier();
}

//*********************  Helper  *********************
void CChildView::ResetGame()
{
  nKey = 0;//��յ�ǰ��¼�İ���������һֱ����һ�εİ���
  m_is_pause = true;
  m_is_top_collide = false;
  m_is_bottom_collide = false;
  m_top_touchPoint = CPoint(-1, -1);
  m_bottom_touchPoint = CPoint(-1, -1);

  //��������1,2��λ��
  m_bottom_player.SetCharPos(0, VIEWHEIGHT - ELEVATION - m_bottom_player.GetCharSize().cy);
  m_top_player.SetCharPos(0, (VIEWHEIGHT - 2 * ELEVATION) / 2 - m_top_player.GetCharSize().cy);

  m_cur_score = 0.0f;
  ShowScore();
  m_end_time = timeGetTime();
  
  ReleaseBarrier();
}

void CChildView::SaveGame()
{
  //ѡ��浵Ŀ¼
  //OPENFILENAME ofn;       // �����Ի���ṹ��
  //WCHAR szFile[MAX_PATH] = {}; // �����ȡ�ļ����ƵĻ�������          

  //// ��ʼ��ѡ���ļ��Ի���
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

  //// ��ʾ��ѡ���ļ��Ի���
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
  //********************�浵Top Game Area����Ϣ********************
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

  //********************�浵Bottom Game Area����Ϣ********************
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

  //********************�浵�÷���Ϣ********************
  save_game_file<<"#" <<++cnt <<" Current Score" <<endl
    <<m_cur_score <<endl <<endl;

  save_game_file<<"#" <<++cnt <<" Highest Score" <<endl
    <<m_highest_score <<endl <<endl;

  save_game_file.close();
}

void CChildView::LoadGameFile()
{
  //�����ļ�ѡ���,���ص�ǰ�÷�
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

  //������windows APIת���������Ƕ�ȡ�ļ�
  std::ifstream load_game_file;
  load_game_file.open(GAME_FILE);

  //�ͷ�֮ǰ����Ϸ״̬
  ReleaseBarrier();

  std::string value;
  int x_pos, y_pos, v_speed;
  int barrier_num, barrier_type;
  float curScore, highestScore;
  
  //********************����Top Game Area����Ϣ********************
  //��ȡTop player��λ��
  load_game_file.ignore(1000, '\n');
  getline(load_game_file, value);
  x_pos = atoi(value.c_str());
  getline(load_game_file, value);
  y_pos = atoi(value.c_str());
  m_top_player.SetCharPos(x_pos, y_pos);

  //��ȡTop Player�Ĵ�ֱ�ٶ�
  load_game_file.ignore(1000, '\n');
  load_game_file.ignore(1000, '\n');
  getline(load_game_file, value);
  v_speed = atoi(value.c_str());
  m_top_player.SetVerticalSpeed(v_speed);

  //��ȡ�ϰ��������
  load_game_file.ignore(1000, '\n');
  load_game_file.ignore(1000, '\n');
  getline(load_game_file, value);
  barrier_num = atoi(value.c_str());

  //��ȡ�����ϰ����������Ϣ
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

  //********************����Bottom Game Area����Ϣ********************
  //��ȡBottom player��λ��
  load_game_file.ignore(1000, '\n');
  load_game_file.ignore(1000, '\n');
  getline(load_game_file, value);
  x_pos = atoi(value.c_str());
  getline(load_game_file, value);
  y_pos = atoi(value.c_str());
  m_bottom_player.SetCharPos(x_pos, y_pos);

  //��ȡBottom Player�Ĵ�ֱ�ٶ�
  load_game_file.ignore(1000, '\n');
  load_game_file.ignore(1000, '\n');
  getline(load_game_file, value);
  v_speed = atoi(value.c_str());
  m_bottom_player.SetVerticalSpeed(v_speed);
  
  //��ȡ�ϰ��������
  load_game_file.ignore(1000, '\n');
  load_game_file.ignore(1000, '\n');
  getline(load_game_file, value);
  barrier_num = atoi(value.c_str());

  //��ȡ�����ϰ����������Ϣ
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

  //********************���ص÷���Ϣ********************
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

//ɾ���Ƴ��߽���ϰ���
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
  //������ڹ۲�����������Ϸ������TCP�˽��յ������ݸ�������ϰ�
  if (m_is_watching){
    return;
  }

  //50%���ʲ������ϰ�,���������Ҫ����
  if (rand() % 2 == 0) return;

  CBarrier *barrier = new CBarrier;
  int bType = rand() % TOTAL_BARRIER_TYPE + 1;
  barrier->InitBarrier(bType);
  //Ĭ����Ϊ�°벿�ֵ���������ϰ��������Ϊ�ϰ벿�ֵ���������ϰ�������Ҫ����ƫ��
  if (areaID == 2){
    barrier->SetBarrierPos(VIEWWIDTH - barrier->GetBarrierSize().cx,
    (VIEWHEIGHT - 2 * ELEVATION) / 2 - barrier->GetBarrierSize().cy);
  }
  v_b.push_back(barrier);

  //������ڱ��۲�����Ϸ����Ҫ�ڴ�����barrier֮�󣬷��͸����۲���
  if (m_is_being_watched){
    UpdateClient();
  }
}

//�����Ϣ��ʽ: Player playerID curFrame vSpeed xPos yPos
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

//�ϰ������Ϣ��ʽ: Barrier areaID barrierType
void CChildView::SendBarrierToTCP(int barrierType, int areaID)
{
  char sendBuf[PACKAGE_SIZE] = {};
  sprintf(sendBuf, "Barrier %d %d ", areaID, barrierType);
  send (send_sock, sendBuf, strlen(sendBuf) + 1, 0);
  return;
}

//��Ϸ״̬����Ϣ��ʽ: GameStatus curScore highestScore startTime endTime isPause 
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
  //���û�б��۲죬 ����Ҫ����
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
  //���ý���۲���ģʽ
  m_is_watching = true;
  char recvBuf[PACKAGE_SIZE] = {};

  //������������,һֱ�ȵ����ܵ�����
  recv(recv_sock, recvBuf, sizeof(recvBuf), 0);
  cout<<endl <<"sucess2" <<endl;

  //�ͷ�֮ǰ���ϰ���Ϣ
  ReleaseBarrier();
  //������õ��ַ���
  string buf(recvBuf);  
  string token;
  stringstream ss(buf);
  vector<string> tokens;
  //�Զ����ո�ָ��ÿ�����ʷ���vector
  while (ss >> token){
    tokens.push_back(token);
  }

  for (int i = 0; i < tokens.size(); ++i)
    cout<<tokens[i]<<" ";
  cout<<endl;

  //������Ӧ����Ϣ
  vector<string>::iterator iter = tokens.begin();
  while(iter != tokens.end()){
    if (iter->compare("Player") == 0){
      //�����Ϣ��ʽ: Player playerID curFrame vSpeed xPos yPos
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
      //�ϰ������Ϣ��ʽ: Barrier areaID barrierType
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
      //��Ϸ״̬����Ϣ��ʽ: GameStatus curScore highestScore startTime endTime isPause
      m_cur_score = stof(*++iter);
      m_highest_score = stof(*++iter);
      /*m_start_time = stoul(*++iter);
      m_end_time = stoul(*++iter);*/
      m_is_pause = stoi(*++iter) == 1 ? true : false;
    }
    //ÿ�ν����꣬����iter
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
�������������ĵ���x����ľ���ľ���ֵС�ڵ��ھ��ο�Ⱥ͵Ķ���֮һ��ͬʱy����ľ���ľ���ֵС�ڵ��ھ��θ߶�
�͵Ķ���֮һ����������ѧ���ʽ�� 
  x����| (x1 + w1 / 2) �C (x2 + w2/2) | < |(w1 + w2) / 2|
  y����| (y1 + h1 / 2) �C (y2 + h2/2) | < |(h1 + h2) / 2| 
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
        return true;//������ײ
    }else{
      return false;
    }
  }
  //���û���ϰ�����û����ײ
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
  str.Format(_T("��ǰ�÷�: %.2f, ��ߵ÷֣� %.2f"), score, m_highest_score);

  //pDC->FillSolidRect(460,300,120,30,RGB(0,255,0));

  pDC->SetTextColor(RGB(255,0,0));
  pDC->TextOut(VIEWWIDTH - SCORE_TEXT_WIDTH , 0, str);
  
  //pDC->FillSolidRect(460,360,120,30,RGB(0,0,255));
  //pen.CreatePen(PS_SOLID,3,RGB(255,255,255));
  //str.Format("ˮƽ:%d",nLevel);
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

  CBrush *brush = CBrush::FromHandle((HBRUSH)GetStockObject(NULL_BRUSH));//͸����ˢ
  CBrush *old_brush = pDC->SelectObject(brush);

  //������ײ��
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
      m_bottom_touchPoint.x, m_bottom_touchPoint.y);//���������1����
    rect.InflateRect(radius, radius);//��������
    pDC->Ellipse(rect);
  }

  pDC->SelectObject(old_pen);
  pDC->SelectObject(old_brush);
  ReleaseDC(pDC);
}