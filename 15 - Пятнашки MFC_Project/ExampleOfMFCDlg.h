
// ExampleOfMFCDlg.h : header file
//

#pragma once

#include "DieHolder.h"


// CExampleOfMFCDlg dialog
class CExampleOfMFCDlg : public CDialogEx
{
// Construction
public:
	CExampleOfMFCDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_EXAMPLEOFMFC_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	DieHolder m_mapOfTheGame;
	HICON m_hIcon;
	CFont m_font;
	CString sec;
	CString m_LeaderBoardStaticText;
	bool boolResult;
	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnTimer(UINT);
	afx_msg void OnBnClickedButton(UINT anID);
	afx_msg void OnBnClickedRestart();
	afx_msg void OnBnClickedLeaderboardButton();
	void UpdateLeaderBoard();
	afx_msg void OnBnClickedClearboardbutton();
public:
	static UINT m_timeruint;
};
