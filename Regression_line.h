#pragma once
#include "afxdialogex.h"
#include<string>

// Regression_line 对话框


class Regression_line : public CDialogEx
{
	DECLARE_DYNAMIC(Regression_line)

public:
	Regression_line(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~Regression_line();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_Regression_line };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	DECLARE_MESSAGE_MAP()
private:
	CEdit x[5], y[5], x0;
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
	point_2 get_a_point(CEdit& x0, CEdit& y0)
	{
		point_2 P;
		P.x = get_position_data(x0);
		P.y = get_position_data(y0);
		return P;
	}
	point_2 points[2][5];
	point_2 get_average()
	{
		point_2 all;
		for (short  j = 0; j <= page; j++)
		{
			for (short i = 0; i <= 4; i++)
			{
				all = all + points[j][i];
			}
		}
		return all / elementnumber;
	}
	point_2 get_regression_2(point_2 average)
	{
		point_2 result;
		if (page == 1)
		{
			for (short i = 0; i <= 4; i++)
			{
				result = result + (average - points[0][i]).pow_point_2(2);
			}
		}
		for (short i = 0; i < (elementnumber-1)%5+1; i++)
		{
			result = result + (average - points[page][i]).pow_point_2(2);
			//std::printf("i=%d , result=%lf  %lf\n",i,result.x,result.y);
		}
		return result ;
	}
	double get_regression_1(point_2 average)
	{
		double result=0;
		if(page==1)
			for (short i = 0; i <= 4; i++)
			{
				result += (points[0][i].x * points[0][i].y);
			}
		for (short i = 0; i < elementnumber%5; i++)
		{
			result += (points[page][i].x * points[page][i].y);
		}
		return result-elementnumber * average.x * average.y;
	}
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedADDpage();
	afx_msg void OnBnClickedCalculate();
	short get_points(short group)
	{
		short i = 0;
		for (i; i <= 4; i++)
		{
			points[group][i] = get_a_point(x[i], y[i]);
			if (points[group][i].x == 0 && points[group][i].y == 0)
			{
				break;
			}
		}
		return i;
	}
	short elementnumber = 0,page=0;
	void ClearEdits()
	{
		for (short i = 0;i<=4;i++)
		{
			x[i].SetWindowTextW(TEXT(""));
			y[i].SetWindowTextW(TEXT(""));
		}
	}
	CStatic Page_Number,y0, Xa, Ya, eb, ek , er;
	double k=0 , b = 0;
	void double_CString(double data, CStatic& goal)
	{
		CString cst;
		cst.Format(_T("%lf"), data);
		goal.SetWindowTextW(cst);
	}
	void get_outcomes()
	{
		point_2 result = get_average();
		double_CString(result.x, Xa);
		double_CString(result.y, Ya);
		double head = get_regression_1(result);
		point_2 foot = get_regression_2(result);
		//std::cout << head << std::endl << foot.x << std::endl<<foot.y<<std::endl;
		k = head / foot.x;
		b = result.y - k * result.x;
		double_CString(k, ek);
		double_CString(b, eb);
		double_CString(head / sqrt(foot.x * foot.y) , er);
	}
	void Calculater()
	{
		double x=get_position_data(x0);
		double y = k * x + b;
		double_CString(y, y0);
	}
};
