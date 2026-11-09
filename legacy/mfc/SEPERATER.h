#pragma once
#include "afxdialogex.h"
#include<string>




class SEPERATER : public CDialogEx
{
	DECLARE_DYNAMIC(SEPERATER)

public:
	SEPERATER(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~SEPERATER();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SEPERATER };
#endif

protected:
//public:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	DECLARE_MESSAGE_MAP()
	
public:
	float size;
	CEdit INPUT_TEXT;
	afx_msg void OnStnClickedResult();
	CStatic OUTCOME;
	afx_msg void OnBnClickedCancel();
};


