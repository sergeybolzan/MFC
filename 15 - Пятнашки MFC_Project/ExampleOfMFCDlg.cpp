
// ExampleOfMFCDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ExampleOfMFC.h"
#include "ExampleOfMFCDlg.h"
#include "afxdialogex.h"
#include "EnterNameDialog.h"
#include <map>
#include <locale.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CExampleOfMFCDlg dialog



CExampleOfMFCDlg::CExampleOfMFCDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CExampleOfMFCDlg::IDD, pParent)
//	, m_timeruint(0)
, m_LeaderBoardStaticText(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

UINT CExampleOfMFCDlg::m_timeruint = 0;

void CExampleOfMFCDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_TIME, m_timeruint);
	DDX_Text(pDX, IDC_MYSTATIC, m_LeaderBoardStaticText);
}

BEGIN_MESSAGE_MAP(CExampleOfMFCDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_COMMAND_RANGE(IDC_BUTTON_0_0, IDC_BUTTON_3_3, OnBnClickedButton)
	ON_BN_CLICKED(ID_RESTART, &CExampleOfMFCDlg::OnBnClickedRestart)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_LEADERBOARD_BUTTON, &CExampleOfMFCDlg::OnBnClickedLeaderboardButton)
	ON_BN_CLICKED(IDC_CLEARBOARDBUTTON, &CExampleOfMFCDlg::OnBnClickedClearboardbutton)
END_MESSAGE_MAP()


// CExampleOfMFCDlg message handlers

BOOL CExampleOfMFCDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	LOGFONT lf;                        // Used to create the CFont.
	memset(&lf, 0, sizeof(LOGFONT));   // Clear out structure.
	lf.lfHeight = 30;                  // Request a N-pixel-high font
	wcscpy(lf.lfFaceName, L"Tahoma");    //    with face name "Arial".
	m_font.CreateFontIndirect(&lf);    // Create the font.
	// Use the font to paint a control. This code assumes
	// a control named IDC_TEXT1 in the dialog box.
	GetDlgItem(IDC_TIME)->SetFont(&m_font);
	GetDlgItem(IDC_TIME2)->SetFont(&m_font);
	for (auto i = 1000; i <= 1015; i++) 	GetDlgItem(i)->SetFont(&m_font);

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	m_hIcon = LoadIcon(AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_ICON1));
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	UpdateLeaderBoard();
	boolResult = FALSE;
	SetTimer(1, 1000, NULL);
	m_mapOfTheGame.DieBuild(GetSafeHwnd());
	m_mapOfTheGame.RestartGame();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CExampleOfMFCDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CExampleOfMFCDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CExampleOfMFCDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CExampleOfMFCDlg::OnBnClickedButton(UINT anID)
{
	m_mapOfTheGame.DieMove(anID);
	if (m_mapOfTheGame.WinningCheck() == TRUE && boolResult == FALSE)
	{
		boolResult = TRUE;
		CString sMsg;
		sMsg.Format(L"Поздравляем, Вы выйграли!\n\nВаш результат: %d сек\nСохранить результат?", m_timeruint);
		KillTimer(1);
		if (MessageBox(sMsg, L"Победа!", MB_YESNO | MB_ICONINFORMATION) == IDYES)
		{
			CEnterNameDialog dlgEnterName;
			dlgEnterName.DoModal();
			UpdateLeaderBoard();
			UpdateData(FALSE);
		}
	}
}

void CExampleOfMFCDlg::OnBnClickedRestart()
{
	int msgboxID = NULL;
	if (boolResult == FALSE) msgboxID = MessageBox(L"Нажмите \"ДА\", чтобы перезапустить игру или \"НЕТ\" для отмены", L"Перезапустить игру?", MB_YESNO | MB_ICONQUESTION);
	if (msgboxID == IDNO) return;
	m_mapOfTheGame.RestartGame();
	m_timeruint = 0;
	SetTimer(1, 1000, NULL);
	boolResult = FALSE;
}

void CExampleOfMFCDlg::OnTimer(UINT uTime)
{
	m_timeruint++;
	UpdateData(FALSE);
}



void CExampleOfMFCDlg::OnBnClickedLeaderboardButton()
{
	setlocale(LC_ALL, "Russian");// чтобы ReadString читал русские символы
	CStdioFile fileIn;
	if (fileIn.Open(L"15table.txt", CFile::modeRead) == FALSE)
		MessageBox(L"Ошибка открытия таблицы", L"Ошибка", MB_OK | MB_ICONINFORMATION);
	
	std::multimap<int, CString> myMap;
	
	CString nameFromFile, timeFromFile;
	while (fileIn.ReadString(nameFromFile))
	{
		fileIn.ReadString(timeFromFile);
		int timeCount = _tstoi(timeFromFile);
		myMap.insert(std::pair<int, CString>(timeCount, nameFromFile)); //заполняет контейнер map парами время - имя (сортировка происходит автоматически) 
	}
	fileIn.Close();

	CString LeaderBoardString;
	int i = 0;
	for (auto it = myMap.begin(); it != myMap.end(), i <= 10; ++it, i++)
	{
		CString a;
		a.Format(L"%d", it->first);
		LeaderBoardString += it->second + L" - " + a + L" сек\n";
	}
	MessageBox(LeaderBoardString, L"Таблица лидеров", MB_YESNO);
}


void CExampleOfMFCDlg::UpdateLeaderBoard()
{
	setlocale(LC_ALL, "Russian");// чтобы ReadString читал русские символы
	CStdioFile fileIn;
	if (fileIn.Open(L"15table.txt", CFile::modeRead) == FALSE)
		MessageBox(L"Ошибка открытия таблицы", L"Ошибка", MB_OK | MB_ICONINFORMATION);

	std::multimap<int, CString> myMap;

	CString nameFromFile, timeFromFile;
	while (fileIn.ReadString(nameFromFile))
	{
		fileIn.ReadString(timeFromFile);
		int timeCount = _tstoi(timeFromFile);
		myMap.insert(std::pair<int, CString>(timeCount, nameFromFile)); //заполняет контейнер map парами время - имя (сортировка происходит автоматически) 
	}
	fileIn.Close();

	m_LeaderBoardStaticText = L"Таблица лидеров (10 лучших):\n\n";

	if (myMap.size() != NULL)
	{
		int i = 0;
		for (auto it = myMap.begin(); it != myMap.end() && i <= 10; ++it, i++)
		{
			CString a;
			a.Format(L"%d", it->first);
			m_LeaderBoardStaticText += it->second + L" - " + a + L" сек\n";
		}
	}
}


void CExampleOfMFCDlg::OnBnClickedClearboardbutton()
{
	CStdioFile fileIn;
	if (fileIn.Open(L"15table.txt", CFile::modeCreate) == FALSE)
		MessageBox(L"Ошибка стирания таблицы", L"Ошибка", MB_OK | MB_ICONINFORMATION);
	fileIn.Close();
	UpdateLeaderBoard();
	UpdateData(FALSE);
}
