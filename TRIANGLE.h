#pragma once
#include "afxdialogex.h"
#include<string>

// TRIANGLE 对话框


class TRIANGLE : public CDialogEx
{
	DECLARE_DYNAMIC(TRIANGLE)
public:
	TRIANGLE(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~TRIANGLE();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TRIANGLE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
private:
	CEdit x1,y1,z1;
	CEdit x2,y2,z2;
	CEdit x3,y3,z3;
	double get_position_data(CEdit& goal)
	{
		CString   str;
		goal.GetWindowTextW(str);
		std::string temp = (CStringA)str;
		//std::string temp = CT2A(str.GetString());
		const char* tempchar = temp.data();
		double result = atof(tempchar);
		//std::cout <<"getposition run " << result << std::endl;
		return result;
	}
	point_3 get_a_point(CEdit& x0, CEdit& y0, CEdit& z0)
	{
		point_3 P;
		P.x = get_position_data(x0);
		P.y = get_position_data(y0);
		P.z = get_position_data(z0);
		return P;
	}
	point_3 a, b, c, n1, n2;
public:
	void double_CString(double data, CStatic& goal)
	{
		CString cst;
		cst.Format(_T("%lf"), data);
		goal.SetWindowTextW(cst);
	}
	void get_handled_points()
	{
		a = get_a_point(x1, y1, z1);
		b = get_a_point(x2, y2, z2);
		c = get_a_point(x3, y3, z3);
		n1 = b - a;
		n2 = c - a;
	}
	double get_size()
	{
		//test_point_information(n1);
		//test_point_information(n2);
		return n1 * n2 *0.5;
	}
	double  get_circumference()
	{
		return n1.mod() + n2.mod() + (n1 - n2).mod();
	}
	CStatic SIZE;
	CStatic CIRCUMFERENCE;
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
};

