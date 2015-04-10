//----------------------------------------------
//					������CCharacter
//----------------------------------------------

#pragma once
#include<atlimage.h>

class CCharacter
{
  //˽�г�Ա����
private:
  enum{
    MAXFRAME = 16 //���֡��:16
  };

private:
  CImage	m_imgCharacter[MAXFRAME];//����
  CSize	m_sCharacter;     //�����С

public:
  CPoint	m_leftTop;      //�����λ��(���Ͻǵ�)
  int		m_curFrame;       //����ĵ�ǰ֡
  int v_speed;//��ɫ��ֱ���ϵ��ٶ�

  //��Ա����
public:
  //��ʼ������
  bool InitCharacter();

  //��ȡ��ҽ�ɫ�Ĵ�С
  CSize GetCharSize() const;

  //�趨�����λ��
  void SetCharPos(int x, int y);
  CPoint GetCharPos() const;

  bool IsOnGround(int ground) const ;

  //�趨����������Ծ���ٶ�
  void SetVerticalSpeed(int s);
  int GetVerticalSpeed() const;
  void MoveVertical(int ground, int sky);

  //��ǰ�ƶ�
  void MoveFront();

  //��һ֡
  void NextFrame();

  //��������(ע������bufferDC�����ò���)
  void StickCharacter(CDC& bufferDC);

  //�ͷ��ڴ���Դ
  void ReleaseCharacter();

  //����������
public:
  CCharacter(void);
  ~CCharacter(void);
};