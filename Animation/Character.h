//----------------------------------------------
//					人物类CCharacter
//----------------------------------------------

#pragma once
#include<atlimage.h>

class CCharacter
{
  //私有成员变量
private:
  enum{
    MAXFRAME = 16 //最大帧数:16
  };

private:
  CImage	m_imgCharacter[MAXFRAME];//人物
  CSize	m_sCharacter;     //人物大小

public:
  CPoint	m_leftTop;      //人物的位置(左上角点)
  int		m_curFrame;       //人物的当前帧
  int v_speed;//角色垂直向上的速度

  //成员函数
public:
  //初始化人物
  bool InitCharacter();

  //获取玩家角色的大小
  CSize GetCharSize() const;

  //设定人物的位置
  void SetCharPos(int x, int y);
  CPoint GetCharPos() const;

  bool IsOnGround(int ground) const ;

  //设定人物向上跳跃的速度
  void SetVerticalSpeed(int s);
  int GetVerticalSpeed() const;
  void MoveVertical(int ground, int sky);

  //向前移动
  void MoveFront();

  //下一帧
  void NextFrame();

  //绘制人物(注：这里bufferDC是引用参数)
  void StickCharacter(CDC& bufferDC);

  //释放内存资源
  void ReleaseCharacter();

  //构造与析构
public:
  CCharacter(void);
  ~CCharacter(void);
};