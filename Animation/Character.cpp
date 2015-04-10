#include "StdAfx.h"
#include "Character.h"
#include <iostream>

CCharacter::CCharacter(void)
{
}

CCharacter::~CCharacter(void)
{
}

//初始化人物
bool CCharacter::InitCharacter()
{
	int i;
	CString path;

	//初始化每一帧
	for(i=0; i<this->MAXFRAME; i++){
		//一个小技巧——获取人物每一帧png的路径
		path.Format(L"res\\%d.png", i+1);
		this->m_imgCharacter[i].Load(path);

		//如果加载失败
		if(this->m_imgCharacter[i].IsNull())
			return false;
	}

	//初始化人物大小
	int w = m_imgCharacter[0].GetWidth();
	int h = m_imgCharacter[0].GetHeight();

  //缩小玩家的角色图片
	this->m_sCharacter.SetSize(w / 2, h / 2);

	//初始化人物位置
	this->m_leftTop.SetPoint(0,	VIEWHEIGHT - h - ELEVATION);

	//初始化为第1帧
	this->m_curFrame = 0;

  //垂直方向速度为0
  this->v_speed = 0;

	return true;
}

CSize CCharacter::GetCharSize() const
{
  return this->m_sCharacter;
}

void CCharacter::SetCharPos(int x, int y)
{
  this->m_leftTop.SetPoint(x, y);
}

CPoint CCharacter::GetCharPos() const
{
  return m_leftTop;
}

void CCharacter::SetVerticalSpeed( int s )
{
  this->v_speed = s;
}

int CCharacter::GetVerticalSpeed() const
{
  return this->v_speed;
}

void CCharacter::MoveVertical(int ground, int sky)
{
  //角色在垂直方向静止
  if(abs(v_speed) < 1e-4 && IsOnGround(ground)) return;

  //当前角色往下移动，且脚步已经到达地平面，则停止运动
  if(v_speed < 0 && (m_leftTop.y + m_sCharacter.cy >= ground) ){
    v_speed = 0;
    m_leftTop.y = ground - m_sCharacter.cy;
    return;
  }

  //角色在垂直方向上还在移动
  if (this->m_leftTop.y >= sky)//还没跳出上边界
  {
    this->m_leftTop.y -= static_cast<int>(this->v_speed);
    this->v_speed -= GRAVITY;//每帧角色的速度减小
  }else{//跳出上边界
    this->m_leftTop.y = sky;
    this->v_speed = 0;
  }
}

//向前移动(如果移动到了客户区中间, 不继续移动了)
void CCharacter::MoveFront()
{
  int border = (VIEWWIDTH - m_sCharacter.cx) / 8;

	if(this->m_leftTop.x <= border)
	{
		this->m_leftTop.x += 4;
	}
}

bool CCharacter::IsOnGround(int ground) const
{
  if( abs(m_leftTop.y + m_sCharacter.cy - ground) <= 5)
    return true;
  else
    return false;
}

//下一帧
void CCharacter::NextFrame()
{
	//------------------------------------------
	// 本可以直接使用求余运算, 但是%求余运算速
	// 度及效率不好, 所以使用简单的判断操作代替
	//------------------------------------------

	//进入下一帧
	this->m_curFrame++;

	if(this->m_curFrame == this->MAXFRAME)
		this->m_curFrame = 0;
}

//绘制人物
void CCharacter::StickCharacter(CDC& bufferDC)
{
	int i = this->m_curFrame;
	//透明贴图
	this->m_imgCharacter[i].TransparentBlt(bufferDC,
		this->m_leftTop.x, this->m_leftTop.y,
		this->m_sCharacter.cx, this->m_sCharacter.cy,
		RGB(0, 0, 0));
}

//释放内存资源
void CCharacter::ReleaseCharacter()
{
	for(int i=0; i<this->MAXFRAME; i++)
		this->m_imgCharacter[i].Destroy();
}