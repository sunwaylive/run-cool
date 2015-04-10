//----------------------------------------------
//					场景类CScene
//----------------------------------------------

#pragma once
#include <atlimage.h>
#include <Mmsystem.h>
#pragma comment(lib, "Winmm.lib")

class CScene
{
//成员变量
private:
	CImage	m_imgStt;//起始背景
	CImage	m_imgNxt;//滚动背景
	int		m_bgX;//背景的x坐标

	bool	m_isStart;//是否开始

//成员函数
public:
	bool InitScene();//初始化场景
	void MoveBg();//移动背景
	////绘制场景(注：这里bufferDC是引用参数)
	void StickScene(CDC &bufferDC, CRect rClient);
	void ReleaseScene();//释放内存资源

//构造与析构
public:
	CScene(void);
	~CScene(void);
};

