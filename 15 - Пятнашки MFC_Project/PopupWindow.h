/* PopupWindow.h - Copyright 2007-2008 Progeny Software Inc. All Rights Reserved. */

#if !defined(AFX_POPUPWINDOW_H__AE3665B7_CB73_4A76_A7B2_C53F0A406533__INCLUDED_)
#define AFX_POPUPWINDOW_H__AE3665B7_CB73_4A76_A7B2_C53F0A406533__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PopupWindow.h : header file
//
#include <memory>
class cEvent;
class Gdiplus::GraphicsPath;
class ColumnsCollection;

/////////////////////////////////////////////////////////////////////////////
// CPopupWindow window

class CPopupWindow : public CWnd
{
// Construction
public:
	CPopupWindow();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPopupWindow)
	public:
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	//}}AFX_VIRTUAL

// Implementation
public:
	void SetFont(CFont *pFont);
	CRect GetPosition() const;
	CSize QueryWindowSize(CDC *pDC, LPCTSTR szText, const CRect &rctReq);
	void SetPosition(CRect rct);
	void SetPosition(const CPoint &pt);
	void SetText(LPCTSTR szText);
	void SetSourceLink(LPCTSTR szText);
	CSize QueryWindowSize(CDC *pDC, LPCTSTR szText);
	void SetTextColor(COLORREF color);
	void SetBorderColor(COLORREF color);
	void SetBackgroundColor(COLORREF color);

	virtual BOOL Create(LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	virtual ~CPopupWindow();

	// Generated message map functions
protected:
	void AddTriToPath(Gdiplus::GraphicsPath *pPath, float left, float top, float right, float bottom);
	CSize GetOuterRectSize(const CRect &rct);
	CSize GetInnerTextRectSize(const CRect &rct);

	CFont *m_pFont;
	CPoint m_ptPosition;
	bool m_bDelayPosition;
	CRect m_rctPosition;
	COLORREF m_clrText;
	COLORREF m_clrBorder;
	COLORREF m_clrBackground;
	CString m_szSourceLink;

	//{{AFX_MSG(CPopupWindow)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	LPCTSTR m_szClassName;

public:
	///@brief we need to transfer into class columns
	///  and events for normal hint displaying
	void SetColumnsColection(ColumnsCollection* const apCC)
	{
		// inline here
		m_pColumnsCollection = apCC;
	}
	///@brief set columns to display
	/// data and time columns will be shifted first in any case
	///  during further processing
	///
	void SetSubDataToDisplay(std::vector<TlmTypes::EVENT_PROPERTY>& atoUse)
	{
		m_arrToDisplay = atoUse;
	}
	/// @brief data from this event will be used as a source for
	///  text forming and as a source of links for hint
	///
	void SetAnEventToTip(std::shared_ptr<cEVENT>& pEvent)
	{
		m_pEvent = pEvent;
	}
protected:

	///@brief detect link position inside m_HintLines
	/// by mouse point
	///  returns m_HintLines.size() + N if could not find link at point
	///
	size_t LinkPosition(POINT const apt) const;

	///@brief we need the whole text for calculating size
	/// of the hint window
	///
	CString CPopupWindow::FormTextAccordingEvent();

	///@brief lines for display in hint
	///  1-st part of the tuple is text
	///  2-nd part - link if exists
	///  3-d part - multiline sign - true, ellipsis - false
	typedef std::tuple<CString, CString, bool, CRect> HLD; // hint line description
	std::vector<HLD> m_HintLines;

	///@brief calculate necessary size for window
	/// also fill CRect inside m_HintLines
	///  szToFill - should contain restrictions by height and width
	/// will contain necessary size for text
	/// pDC should be already with necessary font
	bool CalculateNecessarySize(SIZE& szToFill, CDC* const pDC);

	///@brief here we have array of ids to display in hint
	std::vector<TlmTypes::EVENT_PROPERTY> m_arrToDisplay;

	///@brief copy of an event to display in tip
	/// beware: could be nullptr
	std::shared_ptr<cEVENT> m_pEvent;

	///@brief here we have array of ids to display in hint
	ColumnsCollection* m_pColumnsCollection = nullptr;

	///@brief clicked link position
	/// could be greater then m_HintLines in case of no click at moment
	/// become valid after mouse down
	size_t m_linkPosition = 2134987;

	///@brief we are drawing text not directly inside window rect
	///  if we have triangle at top
	/// we need to shift a little - it's rather for mouse cursor calculation
	/// and for rectangles already calculated without this shift
	int m_triangleShift = 0;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_POPUPWINDOW_H__AE3665B7_CB73_4A76_A7B2_C53F0A406533__INCLUDED_)
