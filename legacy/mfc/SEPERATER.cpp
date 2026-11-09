// SEPERATER.cpp: 实现文件
//

#include "pch.h"
#include "综合计算工具.h"
//#include "afxdialogex.h"
#include "SEPERATER.h"
#include<iostream>

CString Seperater_f(int number_int);

// SEPERATER 对话框

IMPLEMENT_DYNAMIC(SEPERATER, CDialogEx)

SEPERATER::SEPERATER(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SEPERATER, pParent)
{

}

SEPERATER::~SEPERATER()
{
}

void SEPERATER::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, INPUT_TEXT);
	//  DDX_Control(pDX, IDC_RESULT , RESULT);
	DDX_Control(pDX, IDC_OUTCOMES, OUTCOME);
}

BEGIN_MESSAGE_MAP(SEPERATER, CDialogEx)
	ON_BN_CLICKED(IDOK, &SEPERATER::OnStnClickedResult)
	ON_BN_CLICKED(IDCANCEL, &SEPERATER::OnBnClickedCancel)
END_MESSAGE_MAP()
// SEPERATER 消息处理程序


void SEPERATER::OnStnClickedResult()
{
	// TODO: 在此添加控件通知处理程序代码
	CString  number_cstring;
	INPUT_TEXT.GetWindowTextW(number_cstring);
	int number_int =_ttoi(number_cstring);
	if(number_int>1&& number_int<1000000000)
		OUTCOME.SetWindowTextW(Seperater_f(number_int));
	else
		OUTCOME.SetWindowTextW(TEXT("请输入2到1000000000的正整数！"));
}

CString Seperater_f(int number_int)
{
	CString outcome;
	char temp[10];
	char answer[100];
	memset(answer,'\0', sizeof(answer));
	int order = 0;
	for (int tester = 2; tester <= number_int; tester++)
	{
		int goal = number_int / tester;
		while (number_int == goal * tester)
		{
			sprintf_s(temp, "%d", tester);
			if (order != 0)
				strcat_s(answer, "*");
			strcat_s(answer, temp);
			order++;
			number_int = goal;
			goal = number_int / tester;
		}
	}
	//std::string a = answer;
	outcome.Format(_T("%s"), (CString)answer);
	
	return outcome;
}

void SEPERATER::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnCancel();
}
