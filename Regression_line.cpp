// Regression_line.cpp: 实现文件
//

#include "pch.h"
#include "综合计算工具.h"
#include "afxdialogex.h"
#include "Regression_line.h"
#include"TIP.h"

// Regression_line 对话框

IMPLEMENT_DYNAMIC(Regression_line, CDialogEx)

Regression_line::Regression_line(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_Regression_line, pParent)
{

}

Regression_line::~Regression_line()
{
}

void Regression_line::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, x[0]);
	DDX_Control(pDX, IDC_EDIT4, x[1]);
	DDX_Control(pDX, IDC_EDIT7, x[2]);
	DDX_Control(pDX, IDC_EDIT10, x[3]);
	DDX_Control(pDX, IDC_EDIT11, x[4]);
	DDX_Control(pDX, IDC_EDIT2, y[0]);
	DDX_Control(pDX, IDC_EDIT3, y[1]);
	DDX_Control(pDX, IDC_EDIT5, y[2]);
	DDX_Control(pDX, IDC_EDIT6, y[3]);
	DDX_Control(pDX, IDC_EDIT8, y[4]);
	DDX_Control(pDX, IDC_STATIC1, Page_Number);
	DDX_Control(pDX, IDC_EDIT9, x0);
	DDX_Control(pDX, IDC_Y0, y0);
	DDX_Control(pDX, IDC_Xa, Xa);
	DDX_Control(pDX, IDC_Ya, Ya);
	DDX_Control(pDX, IDC_eb, eb);
	DDX_Control(pDX, IDC_ek, ek);
	DDX_Control(pDX, IDC_r, er);
}


BEGIN_MESSAGE_MAP(Regression_line, CDialogEx)
	ON_BN_CLICKED(IDOK, &Regression_line::OnBnClickedOk)
	ON_BN_CLICKED(IDC_ADDpage , &Regression_line::OnBnClickedADDpage)
	ON_BN_CLICKED(IDC_Calculate, &Regression_line::OnBnClickedCalculate)
END_MESSAGE_MAP()


// Regression_line 消息处理程序

void Regression_line::OnBnClickedOk()
{
	if (page == 0)
		elementnumber = 0;
	elementnumber = get_points(page) + page * 5;
	if (elementnumber == 0)
	{
		TIP dlg;
		dlg.Tip_m("请先输入！");
	}
	else if (elementnumber == 1)
	{
		TIP dlg;
		dlg.Tip_m("请输入两个及以上点！");
	}
	else
	{
		get_outcomes();
		if (get_position_data(x0) != 0)
			Calculater();
	}
}

void Regression_line::OnBnClickedADDpage()
{
	elementnumber = get_points(page);
	if(page==1)
	{
		TIP dlg;
		dlg.Tip_m("已达到最大！");
	}
	else if (elementnumber < 4)
	{
		TIP dlg;
		dlg.Tip_m("请先填满本页！");
	}
	else
	{
		page = 1;
		Page_Number.SetWindowTextW(TEXT("2"));
		//elementnumber = get_points(0);
		ClearEdits();
	}
}


void Regression_line::OnBnClickedCalculate()
{
	if(elementnumber!=0)
		Calculater();
	else
	{
		TIP dlg;
		dlg.Tip_m("请先写入表达式！");
	}
}
