#pragma  once
#include <atlimage.h>

class CBarrier
{
private:
  CImage m_imgBarrier; //障碍的贴图
  CSize m_sBarrier; //障碍大小

  CPoint m_leftTop; //障碍的位置(左上角点)
  int m_curFrame;   //障碍的当前帧
  int m_barrierType;//障碍的类型

public:
  int GetType() const;

  //初始化障碍
  bool InitBarrier(int barrierType);

  //获取障碍大小
  CSize GetBarrierSize() const;

  //设定障碍物的位置
  void SetBarrierPos(int x, int y);
  CPoint GetBarrierPos() const;

  //判断障碍能否向左移动
  bool CanLeft();

  //向前移动
  void MoveLeft(int areaID);
  
  //下一帧
  void NextFrame();

  //绘制障碍物
  void StickBarrier(CDC &bufferDC);

  //释放内存资源
  void ReleaseBarrier();

public:
  CBarrier(void);
  ~CBarrier(void);
};