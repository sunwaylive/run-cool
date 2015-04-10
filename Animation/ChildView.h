
// ChildView.h : CChildView ��Ľӿ�
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

//��ʱ��ID
#define ID_TIMER_BG 100//�任����
#define ID_TIMER_CHARACTER 101//�任����
#define ID_TIMER_GEN_BARRIER 102 //�����ϰ���
#define ID_TIMER_DISPLAY_SCORE 103

// CChildView ����

class CChildView : public CWnd
{
public:
  friend class GameSocket;

// ����
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

  CScene		m_scene;//����
  CCharacter m_top_player;//����1
	CCharacter	m_bottom_player;//����2
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

  //��ʼ�趨����״̬
  void ResetGame();

  //�浵��Ϸ
  void SaveGame();

  //������Ϸ
  void LoadGameFile();

  //ɾ���Ƴ��߽���ϰ���
  void DelUnseenBarrier(std::vector<CBarrier*> &v_b);

  //�°벿��Ϊ����1�� �ϰ벿��Ϊ����2
  void GenBarrier(std::vector<CBarrier*> &v_b, int areaID = 1);
  
  //��սģʽ�£������ϰ�
  void ReceiveDataFromTCP();
  void SendPlayerToTCP(int playerID);
  void SendBarrierToTCP(int barrierID, int areaID);
  void SendGameStatus();
  void UpdateClient();

  //�ͷ��ϰ�����Դ
  void ReleaseBarrier();

  //��ײ���
  bool CheckCollison(CCharacter &character, std::vector<CBarrier*> &v_b, CPoint &touchPoint);

  //��ʾ�÷�
  void ShowScore();

  //��ʾ��ײ��
  void ShowCollisonPoint();

  void InitSocket();

  void ReleaseSocket();

// ��д
protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
  
	// ���ɵ���Ϣӳ�亯��
protected:
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnTimer(UINT_PTR nIDEvent);


  afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnDestroy();
};

