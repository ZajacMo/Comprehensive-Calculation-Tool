// MENU.cpp: 实现文件
//

#include "pch.h"
#include "综合计算工具.h"
#include "afxdialogex.h"
#include "MENU.h"
//#include"TIP.h"
#include"SEPERATER.h"
#include"TRIANGLE.h"
#include"Regression_line.h"
#include"EQUATION.h"

// MENU 对话框

IMPLEMENT_DYNAMIC(MENU, CDialogEx)

MENU::MENU(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MENU, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);
}

MENU::~MENU()
{
}

void MENU::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}



BEGIN_MESSAGE_MAP(MENU, CDialogEx)
	ON_BN_CLICKED(IDC_SPERETER_BOTTON, &MENU::OnBnClickedSpereterBotton)
	ON_BN_CLICKED(IDC_TRIANGLE, &MENU::OnBnClickedTriangle)
	ON_BN_CLICKED(IDC_Regression_line, &MENU::OnBnClickedRegression_line)
	ON_BN_CLICKED(IDC_EQUATION, &MENU::OnBnClickedEquation)
	ON_BN_CLICKED(IDCANCEL, &MENU::OnBnClickedCancel)
END_MESSAGE_MAP()


// MENU 消息处理程序



void MENU::OnBnClickedSpereterBotton()
{
	SEPERATER seperater;
	seperater.DoModal();
}


void MENU::OnBnClickedTriangle()
{
	TRIANGLE dlg;
	dlg.DoModal();
}


void MENU::OnBnClickedRegression_line()
{
	//Tip_m("请小心");
	Regression_line dlg;
	dlg.DoModal();
}


void MENU::OnBnClickedEquation()
{
	EQUATION dlg;
	dlg.DoModal();
}


void MENU::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnCancel();
}
