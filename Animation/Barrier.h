#pragma  once
#include <atlimage.h>

class CBarrier
{
private:
  CImage m_imgBarrier; //�ϰ�����ͼ
  CSize m_sBarrier; //�ϰ���С

  CPoint m_leftTop; //�ϰ���λ��(���Ͻǵ�)
  int m_curFrame;   //�ϰ��ĵ�ǰ֡
  int m_barrierType;//�ϰ�������

public:
  int GetType() const;

  //��ʼ���ϰ�
  bool InitBarrier(int barrierType);

  //��ȡ�ϰ���С
  CSize GetBarrierSize() const;

  //�趨�ϰ����λ��
  void SetBarrierPos(int x, int y);
  CPoint GetBarrierPos() const;

  //�ж��ϰ��ܷ������ƶ�
  bool CanLeft();

  //��ǰ�ƶ�
  void MoveLeft(int areaID);
  
  //��һ֡
  void NextFrame();

  //�����ϰ���
  void StickBarrier(CDC &bufferDC);

  //�ͷ��ڴ���Դ
  void ReleaseBarrier();

public:
  CBarrier(void);
  ~CBarrier(void);
};