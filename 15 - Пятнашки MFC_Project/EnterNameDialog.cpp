// EnterNameDialog.cpp : implementation file
//

#include "stdafx.h"
#include "ExampleOfMFC.h"
#include "ExampleOfMFCDlg.h"
#include "EnterNameDialog.h"
#include "afxdialogex.h"
#include <locale.h>

// EnterNameDialog dialog

IMPLEMENT_DYNAMIC(CEnterNameDialog, CDialogEx)

CEnterNameDialog::CEnterNameDialog(CWnd* pParent /*=NULL*/)
	: CDialogEx(CEnterNameDialog::IDD, pParent)
	, m_EnterName(_T(""))
{
	m_EnterName = "Введите свое имя";
}

CEnterNameDialog::~CEnterNameDialog()
{
}

void CEnterNameDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_EnterName);
	DDX_Control(pDX, IDC_EDIT1, editBoxControl);
}


BEGIN_MESSAGE_MAP(CEnterNameDialog, CDialogEx)
	ON_BN_CLICKED(IDOK, &CEnterNameDialog::OnBnClickedOk)
END_MESSAGE_MAP()


// EnterNameDialog message handlers


void CEnterNameDialog::OnBnClickedOk()
{
	setlocale(LC_ALL, "Russian");
	CStdioFile fileOut;
	if (fileOut.Open(L"15table.txt", CFile::modeReadWrite | CFile::modeCreate | CFile::modeNoTruncate) == FALSE) 
		MessageBox(L"Ошибка сохранения результата", L"Ошибка", MB_OK | MB_ICONINFORMATION);
	UpdateData(TRUE);
	fileOut.SeekToEnd();
	m_EnterName += L"\n";
	fileOut.WriteString(m_EnterName);
	CString timecount;
	timecount.Format(L"%d\n", CExampleOfMFCDlg::m_timeruint);
	fileOut.WriteString(timecount);
	fileOut.Close();
	CDialogEx::OnOK();

}


BOOL CEnterNameDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	m_hIcon = LoadIcon(AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_ICON1));
	SetIcon(m_hIcon, TRUE);			// Set big icon
	editBoxControl.SetLimitText(44);
	// TODO:  Add extra initialization here

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
