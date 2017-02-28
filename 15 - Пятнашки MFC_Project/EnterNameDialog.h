#pragma once
#include "afxwin.h"


// EnterNameDialog dialog

class CEnterNameDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CEnterNameDialog)

public:
	CEnterNameDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CEnterNameDialog();

// Dialog Data
	enum { IDD = IDD_ENTERNAME_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	CString m_EnterName;
	HICON m_hIcon;
	CEdit editBoxControl;

	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();

public:
};
