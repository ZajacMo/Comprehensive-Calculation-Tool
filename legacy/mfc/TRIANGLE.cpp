// TRIANGLE.cpp: 实现文件
//

#include "pch.h"
#include "综合计算工具.h"
#include "afxdialogex.h"
#include "TRIANGLE.h"
//#include<io.h>
#include <fcntl.h>

// TRIANGLE 对话框

IMPLEMENT_DYNAMIC(TRIANGLE, CDialogEx)

TRIANGLE::TRIANGLE(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_TRIANGLE, pParent)
{

}

TRIANGLE::~TRIANGLE()
{
}

void TRIANGLE::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, x1);
	DDX_Control(pDX, IDC_EDIT4, y1);
	DDX_Control(pDX, IDC_EDIT7, z1);
	DDX_Control(pDX, IDC_EDIT2, x2);
	DDX_Control(pDX, IDC_EDIT5, y2);
	DDX_Control(pDX, IDC_EDIT8, z2);
	DDX_Control(pDX, IDC_EDIT3, x3);
	DDX_Control(pDX, IDC_EDIT6, y3);
	DDX_Control(pDX, IDC_EDIT9, z3);
	DDX_Control(pDX, IDC_SIZE, SIZE);
	DDX_Control(pDX, IDC_CIRCUNFERENCE, CIRCUMFERENCE);
}


BEGIN_MESSAGE_MAP(TRIANGLE, CDialogEx)
	ON_BN_CLICKED(IDOK, &TRIANGLE::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &TRIANGLE::OnBnClickedCancel)
END_MESSAGE_MAP()


// TRIANGLE 消息处理程序


void TRIANGLE::OnBnClickedOk()
{	
	CString cst;
	//std::cout << "OnBnClickedOk run" << std::endl;
	get_handled_points();
	double_CString(get_size(), SIZE);
	double_CString(get_circumference(), CIRCUMFERENCE);
}


void TRIANGLE::OnBnClickedCancel()
{
	TRIANGLE::OnCancel();
}
