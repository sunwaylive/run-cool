#include "stdafx.h"
#include "Barrier.h"
#include <iostream>

CBarrier::CBarrier(void)
{
}

CBarrier::~CBarrier(void)
{
}

int CBarrier::GetType() const
{
  return this->m_barrierType;
}

bool CBarrier::InitBarrier( int barrierType )
{  
  CString path;
  m_barrierType = barrierType;
  path.Format(L"res\\%d_barrier.png", m_barrierType);
  this->m_imgBarrier.Load(path);
  if(this->m_imgBarrier.IsNull()) return false;

  //根据障碍物图片大小 初始化障碍物的大小
  int w = this->m_imgBarrier.GetWidth();
  int h = this->m_imgBarrier.GetHeight();
  //这里缩放障碍物
  this->m_sBarrier.SetSize(w, h);

  //初始化障碍物的位置
  this->m_leftTop.SetPoint(VIEWWIDTH - m_sBarrier.cx, VIEWHEIGHT - m_sBarrier.cy - ELEVATION);

  return true;
}

CSize CBarrier::GetBarrierSize() const
{
  return this->m_sBarrier;
}

void CBarrier::SetBarrierPos(int x, int y)
{
  this->m_leftTop.SetPoint(x, y);
}

CPoint CBarrier::GetBarrierPos() const
{
  return this->m_leftTop;
}

bool CBarrier::CanLeft()
{
  if (this->m_leftTop.x <= 0) 
    return false;
  else 
    return true;
}

//向左移动障碍物
void CBarrier::MoveLeft(int areaID)
{
  int border = 0; //屏幕左边边界
  if(this->m_leftTop.x > border){
    this->m_leftTop.x -= 2 * areaID;
  }
}

//绘制障碍物
void CBarrier::StickBarrier(CDC &bufferDC)
{
  //透明贴图
  this->m_imgBarrier.TransparentBlt(bufferDC,
    this->m_leftTop.x, this->m_leftTop.y,
    this->m_sBarrier.cx, this->m_sBarrier.cy,
    RGB(0, 0, 0));
}

//释放内存资源
void CBarrier::ReleaseBarrier()
{
  this->m_imgBarrier.Destroy();
}