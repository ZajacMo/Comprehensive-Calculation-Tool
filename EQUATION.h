#pragma once
#include "afxdialogex.h"
#include<string>

// EQUATION 对话框

class EQUATION : public CDialogEx
{
	DECLARE_DYNAMIC(EQUATION)

public:
	EQUATION(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~EQUATION();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_EQUATION };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
private:
	CEdit model[2][3];
	CStatic outcome;
	double get_position_data(CEdit& goal)
	{
		CString   str;
		goal.GetWindowTextW(str);
		std::string temp = (CStringA)str;
		const char* tempchar = temp.data();
		double result = atof(tempchar);
		return result;
	}
	
public:
	double m[3], d;
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	void get_information(short group)
	{
		for (int i = 0; i <= 2; i++)
		{
			m[i] = get_position_data(model[group][i]);
		}
	}
	void judger()
	{
		d = pow(m[1], 2) - 4 * m[0] * m[2];
		if (d < 0)
			outcome.SetWindowTextW(_TEXT("原方程无实根"));
		else if (d == 0)
		{
			CString cst;
			cst.Format(_T("Ⅹ1 = Ⅹ2 = %lf "), -0.5 * m[1] / m[2]);
			outcome.SetWindowTextW(cst);
		}
		else
		{
			CString cst;
			cst.Format(_T("Ⅹ1 = %lf  , Ⅹ2 = %lf "), -0.5 * (m[1] + sqrt(d)) / m[2], -0.5 * (m[1] - sqrt(d)) / m[2]);
			outcome.SetWindowTextW(cst);
		}
	}
	
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedButton3();
	bool enter_test2()
	{
		bool b = 0;
		if (get_position_data(model[1][0]) != 0)
			b = 1;
		return b;
	}
};
