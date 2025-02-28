#pragma once
#include <easyx.h>
//把像素的颜色拆解出来 ARGB
typedef struct _ARGB
{
	byte  a;
	byte  r;
	byte  g;
	byte  b;
}ARGB;
//把颜色拆解
ARGB color2Argb(DWORD c)
{
	ARGB res;
	res.r = (byte)c;
	res.g = (byte)(c >> 8);
	res.b = (byte)(c >> 16);
	res.a = (byte)(c >> 24);
	return res;
}
DWORD argb2Color(ARGB c)
{
	DWORD t = RGB(c.r, c.g, c.b);
	return ((DWORD)c.a) << 24 | t;
}
//把彩色图转成黑白图
void toGray(IMAGE* src)
{
	DWORD* psrc = GetImageBuffer(src);
	for (int i = 0; i < src->getwidth() * src->getheight(); ++i)
	{
		//获取每一个像素点的颜色值
		ARGB t = color2Argb(psrc[i]);
		//灰色图，求三个或者四个颜色值的均值
		byte arv = (t.r + t.g + t.b) / 3;
		ARGB res = { t.a,arv,arv,arv };
		psrc[i] = argb2Color(res);
	}
}
//@png透明贴图
void DrawImg(int x, int y, IMAGE* src)
{
	//变量初始化
	DWORD* pwin = GetImageBuffer();    //窗口缓冲区指针
	DWORD* psrc = GetImageBuffer(src); //图片缓冲区指针
	int win_w = getwidth();            //窗口宽高
	int win_h = getheight();
	int src_w = src->getwidth();       //图片宽高
	int src_h = src->getheight();

	//计算贴图的实际长宽
	int real_w = (x + src_w > win_w) ? win_w - x : src_w;     //处理超出右边界
	int real_h = (y + src_h > win_h) ? win_h - y : src_h;     //处理超出下边界
	if (x < 0) { psrc += -x;           real_w -= -x; x = 0; } //处理超出左边界
	if (y < 0) { psrc += (src_w * -y); real_h -= -y; y = 0; } //处理超出上边界

	//修正贴图起始位置
	pwin += (win_w * y + x);

	//实现透明贴图
	for (int iy = 0; iy < real_h; ++iy)
	{
		for (int ix = 0; ix < real_w; ++ix)
		{
			byte a = (byte)(psrc[ix] >> 24); //计算透明通道的值[0,256) 0为完全透明 255为完全不透明
			if (a > 100)
			{
				pwin[ix] = psrc[ix];
			}
		}
		//换到下一行
		pwin += win_w;
		psrc += src_w;
	}
}