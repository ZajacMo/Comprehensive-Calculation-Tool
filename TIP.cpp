// TIP.cpp: 实现文件
//

#include "pch.h"
#include "综合计算工具.h"
#include "afxdialogex.h"
#include "TIP.h"


// TIP 对话框

IMPLEMENT_DYNAMIC(TIP, CDialogEx)

TIP::TIP(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_TIP, pParent)
{

}

TIP::~TIP()
{
}

void TIP::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX , IDC_STATIC01 ,  tipinformation);
	settext();
}


BEGIN_MESSAGE_MAP(TIP, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON1, &TIP::OnBnClickedButton1)
	ON_STN_CLICKED(IDC_STATIC01, &TIP::OnStnClickedStatic01)
END_MESSAGE_MAP()


// TIP 消息处理程序

void TIP::OnBnClickedButton1()
{
	OnOK();
}


void TIP::OnStnClickedStatic01()
{
	// TODO: 在此添加控件通知处理程序代码
}
