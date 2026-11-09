
// 综合计算工具.h: PROJECT_NAME 应用程序的主头文件
//

#pragma once

#ifndef __AFXWIN_H__
	#error "在包含此文件之前包含 'pch.h' 以生成 PCH"
#endif

#include "resource.h"		// 主符号
//using namespace std;

class point_2
{
public:
	double x = 0, y = 0;
	point_2 operator+(point_2& p)
	{
		point_2 temp;
		temp.x = this->x + p.x;
		temp.y = this->y + p.y;
		return temp;
	}
	point_2 operator-(point_2& p)
	{
		point_2 temp;
		temp.x = this->x - p.x;
		temp.y = this->y - p.y;
		return temp;
	}
	point_2 operator/(short num)
	{
		point_2 temp;
		temp.x = this->x /num;
		temp.y = this->y /num;
		return temp;
	}
	point_2 pow_point_2(short times)
	{
		point_2 result;
		result.x = pow(this->x, times);
		result.y = pow(this->y, times);
		return result;
	}
};
class point_3 :public point_2
{
public:
	double  z=0;
	point_3 operator-(point_3& p)
	{
		point_3 temp;
		temp.x = this->x - p.x;
		temp.y = this->y - p.y;
		temp.z = this->z - p.z;
		return temp;
	}
	double add_tempvalue(double& y1, double& z2, double& y2, double& z1)
	{
		return y1 * z2 - z1 * y2;
	}
	/*void test_point_information(point n1)
	{
		std::cout << "\n0000\n" << n1.x << std::endl << n1.y << std::endl << n1.z << "\n0001\n";
	}*/
	point_3 add(point_3& a, point_3& b)
	{
		point_3 result;
		result.x = add_tempvalue(a.y, b.z, b.y, a.z);
		result.y = -add_tempvalue(a.x, b.z, b.x, a.z);
		result.z = add_tempvalue(a.x, b.y, b.x, a.y);
		//test_point_information(result);
		return result;
	}
	double mod()
	{
		return sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));
	}
	double operator*(point_3& p)
	{
		return add(*this, p).mod();
	}

};

// C综合计算工具App:
// 有关此类的实现，请参阅 综合计算工具.cpp
//



class C综合计算工具App : public CWinApp
{
public:
	C综合计算工具App();
	
// 重写
public:
	virtual BOOL InitInstance();
	//HICON m_hIcon;

// 实现

	DECLARE_MESSAGE_MAP()
};

extern C综合计算工具App theApp;
