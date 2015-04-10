
// ChildView.h : CChildView 类的接口
#pragma once
#include "Scene.h"
#include "Character.h"
#include "Barrier.h"
#include "GameSocket.h"

#include <windows.h>
#include <vector>
#include <time.h>
#include <fstream>
#include <string>

//计时器ID
#define ID_TIMER_BG 100//变换背景
#define ID_TIMER_CHARACTER 101//变换人物
#define ID_TIMER_GEN_BARRIER 102 //生成障碍物
#define ID_TIMER_DISPLAY_SCORE 103

// CChildView 窗口

class CChildView : public CWnd
{
public:
  friend class GameSocket;

// 构造
public:
	CChildView();
  virtual ~CChildView();

private:
  int nKey;
  bool m_is_pause;
  bool m_is_watching;
  bool m_is_being_watched;

  float m_cur_score;
  float m_highest_score;

  DWORD m_start_time;
  DWORD m_end_time;

  CScene		m_scene;//场景
  CCharacter m_top_player;//人物1
	CCharacter	m_bottom_player;//人物2
  bool m_is_top_collide;
  bool m_is_bottom_collide;
  CPoint m_top_touchPoint;
  CPoint m_bottom_touchPoint;

  std::vector<CBarrier*> m_v_bottom_barrier;
  std::vector<CBarrier*> m_v_top_barrier;

private:
  void StartGame();

  void PauseGame();

  void ExitGame();

  //初始设定任务状态
  void ResetGame();

  //存档游戏
  void SaveGame();

  //加载游戏
  void LoadGameFile();

  //删除移出边界的障碍物
  void DelUnseenBarrier(std::vector<CBarrier*> &v_b);

  //下半部分为区域1， 上半部分为区域2
  void GenBarrier(std::vector<CBarrier*> &v_b, int areaID = 1);
  
  //观战模式下，产生障碍
  void ReceiveDataFromTCP();
  void SendPlayerToTCP(int playerID);
  void SendBarrierToTCP(int barrierID, int areaID);
  void SendGameStatus();
  void UpdateClient();

  //释放障碍的资源
  void ReleaseBarrier();

  //碰撞检测
  bool CheckCollison(CCharacter &character, std::vector<CBarrier*> &v_b, CPoint &touchPoint);

  //显示得分
  void ShowScore();

  //显示碰撞点
  void ShowCollisonPoint();

  void InitSocket();

  void ReleaseSocket();

// 重写
protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
  
	// 生成的消息映射函数
protected:
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnTimer(UINT_PTR nIDEvent);


  afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnDestroy();
};

