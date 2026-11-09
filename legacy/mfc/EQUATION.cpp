// EQUATION.cpp: 实现文件
//

#include "pch.h"
#include "综合计算工具.h"
#include "afxdialogex.h"
#include "EQUATION.h"
#include"TIP.h"

// EQUATION 对话框

IMPLEMENT_DYNAMIC(EQUATION, CDialogEx)

EQUATION::EQUATION(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_EQUATION, pParent)
{

}

EQUATION::~EQUATION()
{
}

void EQUATION::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_Model11, model[0][0]);
	DDX_Control(pDX, IDC_Model12, model[0][1]);
	DDX_Control(pDX, IDC_Model13, model[0][2]);
	DDX_Control(pDX, IDC_Model21, model[1][0]);
	DDX_Control(pDX, IDC_Model22, model[1][1]);
	DDX_Control(pDX, IDC_Model23, model[1][2]);
	DDX_Control(pDX, IDC_OUTCOME, outcome);
}


BEGIN_MESSAGE_MAP(EQUATION, CDialogEx)
	
	ON_BN_CLICKED(IDC_BUTTON1, &EQUATION::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &EQUATION::OnBnClickedButton2)
	ON_BN_CLICKED(IDOK, &EQUATION::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON3, &EQUATION::OnBnClickedButton3)
END_MESSAGE_MAP()


// EQUATION 消息处理程序
bool test(double m[3])
{
	bool b = 0;
	if (m[0] == 0 && m[1] == 0 && m[2] == 0)
	{
		TIP dlg;
		dlg.Tip_m("请先输入内容！");
	}
	else if (m[0] == 0)
	{
		TIP dlg;
		dlg.Tip_m("请输入二次函数！");
	}
	else
		b = 1;
	return b;
}

void EQUATION::OnBnClickedButton1()
{
	get_information(0);
	if(test(m)==1)
		judger();
	// TODO: 在此添加控件通知处理程序代码
}


void EQUATION::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码
	get_information(1);
	if(test(m)==1)
	{
		m[2] += m[0] * m[1] * m[1];
		m[1] *= 2 * m[0];
		judger();
	}
}


void EQUATION::OnBnClickedOk()
{
	get_information(0);
	if (m[0] == 0&&m[1]!=0)
	{
		TIP dlg;
		dlg.Tip_m("请输入二次函数！");
	}
	else if (m[0] != 0)
	{
		if(enter_test2()==0)
			judger();
		else
		{
			TIP dlg;
			dlg.Tip_m("请只输入一种类型！");
		}
	}
	else
		OnBnClickedButton2();
}


void EQUATION::OnBnClickedButton3()
{
	OnCancel();
	// TODO: 在此添加控件通知处理程序代码
}
