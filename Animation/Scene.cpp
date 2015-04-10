#include "StdAfx.h"
#include "Scene.h"


CScene::CScene(void)
{
}

CScene::~CScene(void)
{
}

//��ʼ������
bool CScene::InitScene()
{
	this->m_imgStt.Load(L"res\\bgStart.jpg");
	this->m_imgNxt.Load(L"res\\bgNext.jpg");

	//�������ʧ��, ����false
	if(this->m_imgStt.IsNull() ||
		this->m_imgNxt.IsNull())
	{
		return false;
	}

	//��ʼΪ��, ������ʼ����Ϊ0
	this->m_isStart = true;
	this->m_bgX = 0;

	//���ű�������
	mciSendString(L"open res\\bgm.mp3 alias bgm", NULL, 0, NULL);
	
	return true;
}

//���Ƴ���
void CScene::StickScene(CDC &bufferDC, CRect rClient)
{
	//��������ͼƬ��ģʽΪ:COLORONCOLOR, �����������ص�
	bufferDC.SetStretchBltMode(COLORONCOLOR);

	//���������߽�, �ص����
	if(m_bgX <= -rClient.Width())
	{
		m_bgX = 0;

		if(m_isStart)
			m_isStart = false;
	}

	//�ͻ������
	int cltWth = rClient.Width();

	rClient.right = cltWth + m_bgX;
	rClient.left = m_bgX;

	//����ǿ�ʼ�ͻ�����ʼ����
	if(m_isStart)
	{
		this->m_imgStt.StretchBlt(bufferDC, rClient, SRCCOPY);
	}
	//����һ�ű�����Ϊ��ʼ����
	else
	{
		this->m_imgNxt.StretchBlt(bufferDC, rClient, SRCCOPY);
	}

	//������һ�ű���
	rClient.left += cltWth;
	rClient.right += cltWth;
	m_imgNxt.StretchBlt(bufferDC, rClient, SRCCOPY);
}

//�ƶ�����
void CScene::MoveBg()
{
	//�ƶ�����
	m_bgX -= 6;
}

//�ͷ��ڴ���Դ
void CScene::ReleaseScene()
{
	if(!m_imgStt.IsNull())
		this->m_imgStt.Destroy();

	if(!m_imgNxt.IsNull())
		this->m_imgNxt.Destroy();

	mciSendString(L"close bgm", NULL, 0, NULL);
}