//----------------------------------------------
//					������CScene
//----------------------------------------------

#pragma once
#include <atlimage.h>
#include <Mmsystem.h>
#pragma comment(lib, "Winmm.lib")

class CScene
{
//��Ա����
private:
	CImage	m_imgStt;//��ʼ����
	CImage	m_imgNxt;//��������
	int		m_bgX;//������x����

	bool	m_isStart;//�Ƿ�ʼ

//��Ա����
public:
	bool InitScene();//��ʼ������
	void MoveBg();//�ƶ�����
	////���Ƴ���(ע������bufferDC�����ò���)
	void StickScene(CDC &bufferDC, CRect rClient);
	void ReleaseScene();//�ͷ��ڴ���Դ

//����������
public:
	CScene(void);
	~CScene(void);
};

