#pragma once
#include "afxdialogex.h"

// MENU 对话框

class MENU : public CDialogEx
{
	DECLARE_DYNAMIC(MENU)

public:
	MENU(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~MENU();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MENU };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedSpereterBotton();
	afx_msg void OnBnClickedTriangle();
	afx_msg void OnBnClickedRegression_line();
	afx_msg void OnBnClickedEquation();
	afx_msg void OnBnClickedCancel();
	HICON m_hIcon;
};
