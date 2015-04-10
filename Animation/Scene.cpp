#include "StdAfx.h"
#include "Scene.h"


CScene::CScene(void)
{
}

CScene::~CScene(void)
{
}

//初始化场景
bool CScene::InitScene()
{
	this->m_imgStt.Load(L"res\\bgStart.jpg");
	this->m_imgNxt.Load(L"res\\bgNext.jpg");

	//如果加载失败, 返回false
	if(this->m_imgStt.IsNull() ||
		this->m_imgNxt.IsNull())
	{
		return false;
	}

	//开始为真, 背景起始坐标为0
	this->m_isStart = true;
	this->m_bgX = 0;

	//播放背景音乐
	mciSendString(L"open res\\bgm.mp3 alias bgm", NULL, 0, NULL);
	
	return true;
}

//绘制场景
void CScene::StickScene(CDC &bufferDC, CRect rClient)
{
	//设置缩放图片的模式为:COLORONCOLOR, 以消除像素重叠
	bufferDC.SetStretchBltMode(COLORONCOLOR);

	//如果到了左边界, 回到起点
	if(m_bgX <= -rClient.Width())
	{
		m_bgX = 0;

		if(m_isStart)
			m_isStart = false;
	}

	//客户区宽度
	int cltWth = rClient.Width();

	rClient.right = cltWth + m_bgX;
	rClient.left = m_bgX;

	//如果是开始就绘制起始背景
	if(m_isStart)
	{
		this->m_imgStt.StretchBlt(bufferDC, rClient, SRCCOPY);
	}
	//将下一张背景作为起始背景
	else
	{
		this->m_imgNxt.StretchBlt(bufferDC, rClient, SRCCOPY);
	}

	//绘制下一张背景
	rClient.left += cltWth;
	rClient.right += cltWth;
	m_imgNxt.StretchBlt(bufferDC, rClient, SRCCOPY);
}

//移动背景
void CScene::MoveBg()
{
	//移动背景
	m_bgX -= 6;
}

//释放内存资源
void CScene::ReleaseScene()
{
	if(!m_imgStt.IsNull())
		this->m_imgStt.Destroy();

	if(!m_imgNxt.IsNull())
		this->m_imgNxt.Destroy();

	mciSendString(L"close bgm", NULL, 0, NULL);
}