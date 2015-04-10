#include "StdAfx.h"
#include "Character.h"
#include <iostream>

CCharacter::CCharacter(void)
{
}

CCharacter::~CCharacter(void)
{
}

//��ʼ������
bool CCharacter::InitCharacter()
{
	int i;
	CString path;

	//��ʼ��ÿһ֡
	for(i=0; i<this->MAXFRAME; i++){
		//һ��С���ɡ�����ȡ����ÿһ֡png��·��
		path.Format(L"res\\%d.png", i+1);
		this->m_imgCharacter[i].Load(path);

		//�������ʧ��
		if(this->m_imgCharacter[i].IsNull())
			return false;
	}

	//��ʼ�������С
	int w = m_imgCharacter[0].GetWidth();
	int h = m_imgCharacter[0].GetHeight();

  //��С��ҵĽ�ɫͼƬ
	this->m_sCharacter.SetSize(w / 2, h / 2);

	//��ʼ������λ��
	this->m_leftTop.SetPoint(0,	VIEWHEIGHT - h - ELEVATION);

	//��ʼ��Ϊ��1֡
	this->m_curFrame = 0;

  //��ֱ�����ٶ�Ϊ0
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
  //��ɫ�ڴ�ֱ����ֹ
  if(abs(v_speed) < 1e-4 && IsOnGround(ground)) return;

  //��ǰ��ɫ�����ƶ����ҽŲ��Ѿ������ƽ�棬��ֹͣ�˶�
  if(v_speed < 0 && (m_leftTop.y + m_sCharacter.cy >= ground) ){
    v_speed = 0;
    m_leftTop.y = ground - m_sCharacter.cy;
    return;
  }

  //��ɫ�ڴ�ֱ�����ϻ����ƶ�
  if (this->m_leftTop.y >= sky)//��û�����ϱ߽�
  {
    this->m_leftTop.y -= static_cast<int>(this->v_speed);
    this->v_speed -= GRAVITY;//ÿ֡��ɫ���ٶȼ�С
  }else{//�����ϱ߽�
    this->m_leftTop.y = sky;
    this->v_speed = 0;
  }
}

//��ǰ�ƶ�(����ƶ����˿ͻ����м�, �������ƶ���)
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

//��һ֡
void CCharacter::NextFrame()
{
	//------------------------------------------
	// ������ֱ��ʹ����������, ����%����������
	// �ȼ�Ч�ʲ���, ����ʹ�ü򵥵��жϲ�������
	//------------------------------------------

	//������һ֡
	this->m_curFrame++;

	if(this->m_curFrame == this->MAXFRAME)
		this->m_curFrame = 0;
}

//��������
void CCharacter::StickCharacter(CDC& bufferDC)
{
	int i = this->m_curFrame;
	//͸����ͼ
	this->m_imgCharacter[i].TransparentBlt(bufferDC,
		this->m_leftTop.x, this->m_leftTop.y,
		this->m_sCharacter.cx, this->m_sCharacter.cy,
		RGB(0, 0, 0));
}

//�ͷ��ڴ���Դ
void CCharacter::ReleaseCharacter()
{
	for(int i=0; i<this->MAXFRAME; i++)
		this->m_imgCharacter[i].Destroy();
}