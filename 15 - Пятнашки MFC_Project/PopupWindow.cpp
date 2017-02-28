/* PopupWindow.cpp - Copyright 2007-2008 Progeny Software Inc. All Rights Reserved. */

// PopupWindow.cpp : implementation file
//

#include "stdafx.h"
#include "genhist.h"
#include "PopupWindow.h"
#include <initializer_list>
#include <algorithm>
#include <functional>
#include "ColumnsCollection.h"
#include "ColumnTypeConformance.h"
#include "LinkWorkAssistant.h"

#include "Gdiplusex.h"
using namespace Gdiplus;

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifndef LINK_COLOR_IN_GRID
#define LINK_COLOR_IN_GRID RGB(40, 60, 160)
#endif

#define MAX_WIDTH(pDC) (int)(pDC->GetDeviceCaps(LOGPIXELSX) * 3.25)
#define MAX_HEIGHT(pDC) (int)(pDC->GetDeviceCaps(LOGPIXELSY) * 3.0)
#define MIN_WIDTH(pDC) (int)(pDC->GetDeviceCaps(LOGPIXELSX) * 1.0)
#define MIN_HEIGHT(pDC) (int)(pDC->GetDeviceCaps(LOGPIXELSY) * 1.0)

namespace
{
	CRect rcEMPTY{-1, -1, -1, -1};
	const TCHAR* const c_newLineString = _T("\r\n");
	const TCHAR bulletP[3] = {183, _T(' '), 0}; // for automatic bulleting
};

/////////////////////////////////////////////////////////////////////////////
// CPopupWindow

CPopupWindow::CPopupWindow() 
	:	m_szClassName(AfxRegisterWndClass(CS_VREDRAW | CS_HREDRAW, 0, 0, 0)),
		m_rctPosition(0, 0, 0, 0),
		m_ptPosition(0, 0)
{
	SetBackgroundColor(RGB(255, 255, 255));
	SetBorderColor(RGB(0, 0, 0));
	SetTextColor(RGB(0, 0, 0));
	m_bDelayPosition = false;
	m_pFont = NULL;
	m_szSourceLink = _T("");
}

CPopupWindow::~CPopupWindow()
{
}


BEGIN_MESSAGE_MAP(CPopupWindow, CWnd)
	//{{AFX_MSG_MAP(CPopupWindow)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CPopupWindow message handlers

/**
 *	Creates a new popup window
 *
 *	@param[in] lpszClassName string class name.  Must be identical to m_szClassName
 *	@param[in] lpszWindowName string window name.
 *	@param[in] dwStyle window style
 *	@param[in] rect output rectangle
 *	@param[in] pParentWnd pointer to parent window
 *	@param[in] nID ID of the new window
 *	@param[in] pContext pointer to application context (Default is NULL)
 *	@return	BOOL TRUE if the window was successfully created.
 *	@author	Chris Payson (CSP)
 *	@since 30 Jan 2007
 */
BOOL CPopupWindow::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext) 
{	
	
	if (_tcscmp(lpszClassName, m_szClassName) != 0)
	{	ASSERT(0);
		TRACE("Invalid class name.  This must be a popup window!\n");
		return (FALSE);
	}

	m_rctPosition = CRect(rect.left, rect.top, rect.left, rect.top);
	m_ptPosition = m_rctPosition.TopLeft();

	return CWnd::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
}

/**
 *	Creates a new PopupWindow
 *
 *	@param[in] lpszWindowName string window name
 *	@param[in] dwStyle window style
 *	@param[in] &rect output rectangle
 *	@param[in] *pParentWnd poitner to parent window
 *	@param[in] nID ID of the new window
 *	@param[in] *pContext pointer to application context.  (Default is NULL)
 *	@return	BOOL TRUE if the window was successfully  created
 *	@author	Chris Payson (CSP)
 *	@since 30 Jan 2007
 */
BOOL CPopupWindow::Create(LPCTSTR lpszWindowName, DWORD dwStyle, const RECT &rect, CWnd *pParentWnd, UINT nID, CCreateContext *pContext)
{	return (Create(m_szClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext));
}


#define N_BORDER 2
#define N_TRIHEIGHT 15.0f
#define N_RADIUS 10
#define N_TRIPADDING N_RADIUS

/**
 *	Handles painting of the background.  This is where most of the
 *	fun will be taking place
 *
 *	@param[in] pDC pointer to output device
 *	@return	BOOL TRUE if we handled the paint, otherwise false
 *	@author	Chris Payson (CSP)
 *	@since 31 Jan 2007
 */
BOOL CPopupWindow::OnEraseBkgnd(CDC* pDC) 
{
	if (m_bDelayPosition)	// If delayed, set position prior to painting
	{
		SetPosition(m_rctPosition);
	}

	CRect rctClip;
	const int nClipResult = pDC->GetClipBox(&rctClip);
	if (nClipResult == NULLREGION || nClipResult == ERROR)
		return (TRUE);

	HRGN hrgn = NULL;
	hrgn = CreateRectRgn(rctClip.left, rctClip.top, rctClip.right, rctClip.bottom);
	Region rgn(hrgn);
	CGdiPlus::SelectClippingRegion(pDC, &rgn);

	// Draw the window as a bubble
	Graphics *pGraphics = new Graphics(pDC->GetSafeHdc());
	pGraphics->SetSmoothingMode(SmoothingModeHighQuality);

	GraphicsPath path;
	CRect rctBubble;
		
	GetClientRect(&rctBubble);								// Get window size

	const int nDeflate = N_BORDER / 2;
	rctBubble.DeflateRect(nDeflate, nDeflate, nDeflate + 1, nDeflate + 1);


	// Convert point to client cords
	CPoint ptTri = m_ptPosition;
	CWnd *pParentWnd = GetParent();
	if (pParentWnd) pParentWnd->ClientToScreen(&ptTri);
	ScreenToClient(&ptTri);

	const bool bTriBottom = ptTri.y > rctBubble.top;
	const bool bTriRight =  ptTri.x > rctBubble.left + rctBubble.Width() / 2;
	
	// Build Triangle
	float nTriLeft = (float)ptTri.x;
	float nTriRight = nTriLeft + N_TRIHEIGHT;
	float nTriTop = 0.0f, nTriBottom = 0.0f;

	if (bTriRight)
	{	nTriRight = nTriLeft - N_TRIHEIGHT;
	}

	m_triangleShift = static_cast<int>(N_BORDER + N_RADIUS);
	if (bTriBottom)
	{
		rctBubble.bottom -= (int)N_TRIHEIGHT;

		nTriTop = rctBubble.bottom - 1.0f;
		nTriBottom = rctBubble.bottom + N_TRIHEIGHT;
	}
	else
	{
		rctBubble.top += (int)N_TRIHEIGHT;

		m_triangleShift += static_cast<int>(N_TRIHEIGHT);

		nTriTop = rctBubble.top + 1.0f;
		nTriBottom = 0.0f;//N_TRIHEIGHT - rctBubble.top;
	}

	// Build Bubble Path
	CGdiPlus::BuildRoundedRectanglePath(path, rctBubble, N_RADIUS);
	AddTriToPath(&path, nTriLeft, nTriTop,	nTriRight, nTriBottom);

	// Draw the outer bubble
	Pen borderPen(Color(255, 
		GetRValue(m_clrBorder),
		GetGValue(m_clrBorder),
		GetBValue(m_clrBorder)),
		N_BORDER);

	pGraphics->DrawPath(&borderPen, &path);


	// Adjust the inner bubble
	rctBubble.DeflateRect(nDeflate, nDeflate, nDeflate, nDeflate);

	if (bTriRight)
	{	nTriLeft -= nDeflate;
		nTriRight += nDeflate;
	}
	else
	{	nTriLeft += nDeflate;
		nTriRight -= nDeflate;
	}

	if (bTriBottom)
	{	nTriTop -= 1.0f;
		nTriBottom -= (float)nDeflate + 1.0f;
	}
	else
	{	nTriTop += 1.0f;
		nTriBottom += (float)nDeflate + 1.0f;
	}

	// Build the inner bubble
	GraphicsPath pathInside;
	CGdiPlus::BuildRoundedRectanglePath(pathInside, rctBubble, N_RADIUS);
	AddTriToPath(&pathInside, nTriLeft, nTriTop, nTriRight, nTriBottom);

	// Now draw inside bubble
	Color backgroundColor(255,
		GetRValue(m_clrBackground),
		GetGValue(m_clrBackground),
		GetBValue(m_clrBackground));

	SolidBrush backgroundBrush(backgroundColor);
	Pen backgroundPen(backgroundColor);

	pGraphics->FillPath(&backgroundBrush, &pathInside);
	pGraphics->DrawPath(&backgroundPen, &pathInside);

	delete pGraphics;

	CGdiPlus::RestoreClippingRegion(pDC);
	DeleteObject(hrgn);


	return (TRUE);
}

#define TEXTOUT_FLAGS DT_EDITCONTROL | DT_LEFT | DT_WORDBREAK

/**
 *	Handles Painting of the internal window data.  We are
 *	limiting ourselves to painting text here.
 *
 *	@author	Chris Payson (CSP)
 *	@since 31 Jan 2007
 */
void CPopupWindow::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	CString szText = _T("");
	GetWindowText(szText);

	// we will assume that the window has been properly sized.
	CRect rct;
	GetClientRect(&rct);

	// Get the inner text rectangle
	CSize sizeText = GetInnerTextRectSize(rct);
	
	rct.left += N_BORDER + N_RADIUS;
	rct.right = rct.left + sizeText.cx;
	


	// Convert point to client cords
	CPoint ptTri = m_ptPosition;
	CWnd *pParentWnd = GetParent();
	if (pParentWnd) pParentWnd->ClientToScreen(&ptTri);
	ScreenToClient(&ptTri);

	const bool bTriBottom = rct.top < ptTri.y;	// tri on top or bottom?
	
	if (bTriBottom) 
	{
		rct.top += N_BORDER + N_RADIUS;
		rct.bottom = rct.top + sizeText.cy;
	}
	else
	{
		rct.bottom -= N_BORDER + N_RADIUS;
		rct.top = rct.bottom - sizeText.cy;
	}

	std::shared_ptr<CFont> pOldFont(dc.SelectObject(m_pFont), [&dc](CFont* p){ dc.SelectObject(p); });

	COLORREF clrTextOld = dc.SetTextColor(m_clrText);
	COLORREF clrBkOld = dc.SetBkColor(m_clrBackground);

	if (m_HintLines.size() > 0)
	{
		CRect rcFullText(rct); // local variable inside this "if"
		POINT origin{rcFullText.left, rcFullText.top};

		for (auto& data : m_HintLines) // for every link
		{
			auto sText = std::get<0>(data);
			auto bLink = std::get<1>(data).GetLength() > 0;
			const auto bMultiLine = std::get<2>(data);
			CRect rcText = std::get<3>(data); // need to calculate this rect

			rcText.OffsetRect(origin);

			const auto len = sText.GetLength();
			std::vector<TCHAR> v(len + 10); // need at least 3 characters more - do 10
			memset(v.data(), 0, sizeof(TCHAR) * (len + 10));
			_tcsncat(v.data(), sText.GetBuffer(), (len + 9));

			if (rcText.bottom > rcFullText.bottom)
			{
				rcText.bottom = rcFullText.bottom;
			}
			if (rcText.top >= rcText.bottom)
			{
				break; // nothing to draw more (out of window)
			}

			if (bLink)
			{
				dc.SetTextColor(LINK_COLOR_IN_GRID);
			}
			if (bMultiLine) // calculate with height
			{
				dc.DrawTextEx(sText.GetBuffer(), &rcText,
					DT_EDITCONTROL | DT_EXTERNALLEADING | DT_LEFT | DT_NOPREFIX | DT_TOP | DT_WORDBREAK | DT_END_ELLIPSIS,
					nullptr);
			}
			else
			{
				// calculate with ellipsis
				::DrawTextEx(dc.GetSafeHdc(), v.data(), len, &rcText,
					DT_EXTERNALLEADING | DT_END_ELLIPSIS | DT_LEFT | DT_NOPREFIX | DT_SINGLELINE,
					NULL);
			}
			if (bLink)
			{
				dc.SetTextColor(m_clrText);
			}

		} // for (auto& data : m_HintLines) // for every link
	}
	else
	{
		if(m_szSourceLink.GetLength() <= 0)
			dc.DrawText(szText, &rct, TEXTOUT_FLAGS | DT_END_ELLIPSIS);
		else
		{
			int sourceStartPos = szText.Find(_T("Source -"));
			if(sourceStartPos < 0)
			{
				sourceStartPos = szText.Find(_T("SOURCE -"));
			}

			CString strTemp = szText.Mid(0,sourceStartPos);
			dc.DrawText(strTemp, &rct, TEXTOUT_FLAGS | DT_END_ELLIPSIS);
			clrTextOld = dc.SetTextColor(RGB(40,60,160));
			rct.top = rct.bottom - 20;
			strTemp = szText.Mid(sourceStartPos);
			dc.DrawText(strTemp, &rct, TEXTOUT_FLAGS | DT_END_ELLIPSIS);
		}
	}

	dc.SetBkColor(clrBkOld);
	dc.SetTextColor(clrTextOld);

	// Do not call CWnd::OnPaint() for painting messages
}

/**
 *	Sets the background color of the control
 *
 *	@param[in] color color to set window background to
 *	@author	Chris Payson (CSP)
 *	@since 1 Feb 2007
 */
void CPopupWindow::SetBackgroundColor(COLORREF color)
{	m_clrBackground = color;
}

/**
 *	Sets the border color of the control
 *
 *	@param[in] color color to set window border to
 *	@author	Chris Payson (CSP)
 *	@since 1 Feb 2007
 */
void CPopupWindow::SetBorderColor(COLORREF color)
{	m_clrBorder = color;
}

/**
 *	Sets the text color
 *
 *	@param[in] color color to set window text to
 *	@author	Chris Payson (CSP)
 *	@since 1 Feb 2007
 */
void CPopupWindow::SetTextColor(COLORREF color)
{	m_clrText = color;
}

/**
 *	Gets the size of the inner rectangle for text
 *
 *	@param[in] &rct bounding rectangle (including the tail)
 *	@return	CSize resulting inner size
 *	@author	Chris Payson (CSP)
 *	@since 1 Feb 2007
 */
CSize CPopupWindow::GetInnerTextRectSize(const CRect &rct)
{	
	// Note:  If this formula changes, be sure to update GetOuterRectSize
	return CSize(rct.Width() - (N_BORDER + N_RADIUS) * 2, 
		rct.Height() - (N_BORDER + N_RADIUS) * 2 - (int)N_TRIHEIGHT);
}

/**
 *	Queries the window for a window size that will be capable of
 *	displaying the given text.  (There is a limit of course!)
 *
 *	@param[in] pointer to output Device
 *	@param[in] szText text to query window size with
 *	@return	CSize resulting size
 *	@author	Chris Payson (CSP)
 *	@since 1 Feb 2007
 */
CSize CPopupWindow::QueryWindowSize(CDC *pDC, LPCTSTR szText)
{	return (QueryWindowSize(pDC, szText, CRect(0, 0, 9999, 0)));
}

/**
 *	Gets the outer rectangle size based on a rectangle
 *	reserved for text
 *
 *	@param[in] &rct rectangle reserved for text
 *	@return	CSize Size required for window
 *	@author	Chris Payson (CSP)
 *	@since 1 Feb 2007
 */
CSize CPopupWindow::GetOuterRectSize(const CRect &rct)
{
	// Note:  If this formula changes, be sure to update GetInnerTextRectSize
	return CSize(rct.Width() + (N_BORDER + N_RADIUS) * 2,
		rct.Height() + N_BORDER * 3 + N_RADIUS * 2 + (int)N_TRIHEIGHT);

}


CString CPopupWindow::FormTextAccordingEvent()
{
	// Steps:
	// 1) extract data and time columns to processing in 1 line
	// 2) form string with dates & times
	// 3) add all strings after that 1 by 1

	m_HintLines.clear();
	m_linkPosition = 1349587; // link position reset to invalid
	CString result;
	if (nullptr == m_pEvent || m_pColumnsCollection == nullptr || m_arrToDisplay.size() <= 0)
	{
		return result;
	}
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// 1) extract data and time columns to processing in 1 line
	struct SDUMMY{
		TlmTypes::EVENT_PROPERTY an_id;
		CString sText; // extracted text wil be here (or empty string if we omit this field)
	} arrV[] =
	{
		{TlmTypes::pSTART_DATE, _T("")},
		{TlmTypes::pSTART_TIME, _T("")},
		{TlmTypes::pEND_DATE, _T("")},
		{TlmTypes::pEND_TIME, _T("")}
	};

	std::vector<TlmTypes::EVENT_PROPERTY> vDateTime;
	std::vector<TlmTypes::EVENT_PROPERTY> arrToDisplay = m_arrToDisplay; // we are using local copy only
	for (auto& typeAndStr: arrV)
	{
		auto it = std::find(arrToDisplay.begin(), arrToDisplay.end(), typeAndStr.an_id);
		if (it != arrToDisplay.end())
		{
			vDateTime.emplace_back(typeAndStr.an_id); // add necessary field
			arrToDisplay.erase(it); // erase from the initial collection
		}
	}
	//=======================================================
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// 2) form string with dates & times
	//
	// 2.1 fill strings into arrV
	// 2.2 form final string
	if (vDateTime.size() > 0)
	{
		// 2.1 fill strings into arrV
		for (auto& an_id : vDateTime)
		{
			std::find_if(std::begin(arrV), std::end(arrV), [&an_id](SDUMMY& p)
			{
				return p.an_id == an_id;
			})->sText = m_pEvent->Extractor(an_id); // set string inside arrV
		}

		// 2.2 form final string
		CString v_dt;
		v_dt.Format(_T("%s %s - %s %s")
			, arrV[0].sText.GetBuffer()
			, arrV[1].sText.GetBuffer()
			, arrV[2].sText.GetBuffer()
			, arrV[3].sText.GetBuffer()
		);
		v_dt.Replace(_T("  "), _T(" ")); // case when no start time or end date
		v_dt.Trim(_T(" -")); // case when to start or end part

		result += v_dt;
		m_HintLines.emplace_back(HLD(v_dt, _T(""), true, rcEMPTY));
	}

	auto fDualNewLine = [this, &result]()->void
	{
		if (m_HintLines.size() != 1) // when we have 1: only date+time inside
		{
			result += c_newLineString; // add new line
			m_HintLines.emplace_back(HLD(_T(""), _T(""), false, rcEMPTY)); // empty string
		}
	};

	//=======================================================
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// 3) add all strings after that 1 by 1
	for (const auto an_id: arrToDisplay)
	{
		auto pColumn =  m_pColumnsCollection->ByType(an_id);
		if (pColumn == nullptr)
		{
			continue;
		}

		auto vConformance = RequestConformance();
		std::shared_ptr<WCTL_EventProperty> vProp;
		if (m_pEvent->m_props.FindProperty(an_id, vProp))
		{
			auto const vType = vConformance->Type(an_id);
			if (vType == WCTL_EventProperty::SUBTYPE::epst_LinkList || vType == WCTL_EventProperty::SUBTYPE::epst_SimpleList)
			{
				std::function<CString(size_t)> fn;
				std::function<size_t()> fnSize;
				std::function<CString(size_t)> fnLink;
				if (vType == WCTL_EventProperty::SUBTYPE::epst_LinkList)
				{
					LinkWorkAssistant vAssistant(vProp);
					fn = [vAssistant](size_t ind)->CString
					{
						return CString(bulletP) + vAssistant[ind]->TextDisplay(); // add bullet here
					};
					fnSize = [vAssistant]()->size_t
					{
						return vAssistant.size();
					};
					fnLink = [vAssistant](size_t ind)->CString
					{
						return vAssistant[ind]->TextLink(); // add bullet here
					};
				}
				if (vType == WCTL_EventProperty::SUBTYPE::epst_SimpleList)
				{
					std::vector<CString> sList;
					vProp->GetValue(sList);

					fn = [sList](size_t ind)->CString
					{
						return CString(bulletP) + sList[ind]; // automatic bulleting
					};
					fnSize = [sList]()->size_t
					{
						return sList.size();
					};
					fnLink = [sList](size_t ind)->CString
					{
						return _T(""); // add bullet here
					};
				}
				// form string according functions
				CString ret;
				auto const nDm = fnSize();
				if (nDm > 0)
				{
					if (result.GetLength() > 0)
					{
						result += c_newLineString;
					}
					fDualNewLine();

					auto xLine = pColumn->GetTitle(); // set title
					result += xLine + c_newLineString;
					m_HintLines.emplace_back(HLD(xLine, _T(""), true, rcEMPTY));
					for (decltype(fnSize()) i = 0; i < nDm; ++i)
					{
						auto xLine = fn(i);
						m_HintLines.emplace_back(HLD(xLine, fnLink(i), false, rcEMPTY));

						ret += xLine + ((i + 1) < nDm ? c_newLineString : _T(""));
					}

					result += ret; // add bulleted list
				}
				continue;

			}
		}// if (m_pEvent->m_props.FindProperty(an_id, vProp)) // only lists processing

		{/// property not founded in link list or in set
		 /// usual text
			std::wstring szTxt;
			m_pEvent->Get(an_id, szTxt);
			if (szTxt.length() > 0)
			{
				if (result.GetLength() > 0)
				{
					result += c_newLineString; // add new line
				}
				fDualNewLine();

				CString xLine = pColumn->GetTitle() + " - " + szTxt.c_str();
				m_HintLines.emplace_back(HLD(xLine, _T(""), true, rcEMPTY));
				result += xLine; // column nanme and the value
			}
		}

	}//for (const auto an_id: arrToDisplay)
	//=======================================================

	m_linkPosition = m_HintLines.size() + 1; // link position reset to invalid
	return result;
}

/**
 *	Sets the window Text and sizes the window to it
 *
 *	@param[in] szText text to set the window to and size to
 *	@author	Chris Payson (CSP)
 *	@since 1 Feb 2007
 */
void CPopupWindow::SetText(LPCTSTR szText)
{
	ASSERT(GetSafeHwnd());
	auto sText = FormTextAccordingEvent();
	if (sText.GetLength() <= 0)
	{
		sText = szText;
	}
	SetWindowText(sText);

	CDC *pDC = GetDC();
	CSize sizeWnd = QueryWindowSize(pDC, szText);
	ReleaseDC(pDC);

	SetWindowPos(NULL, 0, 0, sizeWnd.cx, sizeWnd.cy, 
		SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOACTIVATE | SWP_NOCOPYBITS);

	// optimize placement based on new size!
	SetPosition(m_ptPosition);
}

/**
 *	Sets the output position of the window.  This point
 *	represents the point where the tail will extend from in
 *	screen coordinates.
 *	The rest of the window will be rotated around it.
 *
 *	@param[in] &pt point where bubble will extend from
 *	@author	Chris Payson (CSP)
 *	@since 1 Feb 2007
 */
void CPopupWindow::SetPosition(const CPoint &pt)
{	SetPosition(CRect(pt, pt));
}

/**
 *	Adds a triangle to the path
 *
 *	@param[out] *pPath pointer to path where triangle will be added
 *	@param[in] left left most value of triangle
 *	@param[in] top top most value of triangle
 *	@param[in] right right most value of triangle 
 *	@param[in] bottom bottom most value of triangle
 *	@author	Chris Payson (CSP)
 *	@since 1 Feb 2007
 */
void CPopupWindow::AddTriToPath(GraphicsPath *pPath, float left, float top, float right, float bottom)
{
	pPath->AddLine(left, top, left, bottom);
	pPath->AddLine(left, bottom, right, top);
	pPath->AddLine(right, top, left, top);
}

/**
 *	Sets the output positon of the window.  The rectangle passed in
 *	is not the size of the window, but a region where the bubble cannot
 *	be placed.  The Bubble is placed along the outside of this region.
 *
 *	@param[in] &rct rectangle that outlines the bubble start point.
 *	This should be in screen coordinates
 *	@author	Chris Payson (CSP)
 *	@since 2 Feb 2007
 */
void CPopupWindow::SetPosition(CRect rct)
{
	m_rctPosition = rct;
	
	CRect rctOuterWnd(0, 0, 0, 0), rctWnd(0, 0, 0, 0);
	GetWindowRect(&rctWnd);
		
	CWnd *pWndParent = GetParent();
	if (pWndParent)
	{	pWndParent->GetClientRect(&rctOuterWnd);
		pWndParent->ScreenToClient(rct);
	}
	else
	{
		rctOuterWnd.right = GetSystemMetrics(SM_CXSCREEN);
		rctOuterWnd.bottom = GetSystemMetrics(SM_CYSCREEN);
	}

	// 1 Feb 2007 CSP If we don't have a proper width, we'll
	// delay positioning of the window until the first paint
	if (rctOuterWnd.Size().cx <= 0
	|| rctOuterWnd.Size().cy <= 0)
	{
		m_bDelayPosition = true;
		return;
	}

	m_bDelayPosition = false;

	// we now have a rectangle in parent client coords.  Ideally, the
	// text will popup above and to the right...
	rctWnd.OffsetRect(-rctWnd.left, -rctWnd.top);

	const int nRightArea = rctOuterWnd.right - rct.right;
	const int nLeftArea = rct.left - rctOuterWnd.left;
	const int nTopArea = rct.top - rctOuterWnd.top;
	const int nBottomArea = rctOuterWnd.bottom - rct.bottom;

	CDC *pDC = GetDC();
	const int nMinWidth = MIN_WIDTH(pDC);
	const int nMinHeight = MIN_HEIGHT(pDC);
	ReleaseDC(pDC);


	int nAvailHeight = 0, nAvailWidth = 0;

	// Place horizontally in the area with the most space, giving preference 
	// to the right hand side.
	// 2 Apr 2007 CSP BUG#2359 Ensure that there is room for the bubble
	if (nRightArea >= nLeftArea || nRightArea >= rctWnd.Width())
	{	nAvailWidth = nRightArea;
		m_ptPosition.x = rct.right;
		
		rctWnd.OffsetRect(m_ptPosition.x - N_TRIPADDING, 0);
	}
	else
	{
		nAvailWidth = nLeftArea;
		m_ptPosition.x = rct.left;
		
		rctWnd.OffsetRect(m_ptPosition.x + N_TRIPADDING - rctWnd.Width(), 0);
	}



	// Place vertically in the area where we have the most space.
	// 2 Apr 2007 CSP BUG#2359 Ensure that there is room for the bubble
	if (nTopArea >= nBottomArea || nTopArea >= rctWnd.Height())
	{
		nAvailHeight = nTopArea;
		m_ptPosition.y = rct.top;

		rctWnd.OffsetRect(0, m_ptPosition.y - rctWnd.Height());
	}
	else
	{
		nAvailHeight = nBottomArea;
		m_ptPosition.y = rct.bottom;
		
		rctWnd.OffsetRect(0, m_ptPosition.y);
	}
	

	// if the available width is less than whole, we need
	// to move the window
	if (nAvailWidth < rctWnd.Width())
	{	
		if (rctWnd.left < rctOuterWnd.left)
		{
			rctWnd.OffsetRect(rctOuterWnd.left - rctWnd.left, 0);
		}
		else if (rctWnd.right > rctOuterWnd.right)
		{
			rctWnd.OffsetRect(rctOuterWnd.right - rctWnd.right, 0);
		}

		nAvailWidth = rctOuterWnd.Width();
	}

	// move the window and shrink it if necessary
	SetWindowPos(NULL, rctWnd.left, rctWnd.top, 
		__min(rctWnd.Width(), nAvailWidth),
		__min(rctWnd.Height(), nAvailHeight), 
		SWP_NOOWNERZORDER | SWP_NOACTIVATE | SWP_NOCOPYBITS);
}

bool CPopupWindow::CalculateNecessarySize(SIZE& szToFill, CDC* const pDC)
{
	const SIZE szRestrictions(szToFill);
	if (m_HintLines.size() <= 0)
	{
		return false;
	}

	szToFill.cx = 0; // need to fill these fields
	szToFill.cy = 0;

	CPoint initial{0,0};
	int width = 10;

	for (auto& data: m_HintLines) // for every link
	{
		auto sText = std::get<0>(data);
		const auto bMultiLine = std::get<2>(data);
		CRect& rcText = std::get<3>(data); // need to calculate this rect

		rcText.left = initial.x;
		rcText.top = initial.y;
		if (sText.GetLength() <= 0)
		{
			sText = bulletP;// at least something to draw
		}

		RECT rcRez{0, 0, szRestrictions.cx, -1};
		if (bMultiLine) // calculate with height
		{
			pDC->DrawTextEx(sText.GetBuffer(), &rcRez,
				DT_EDITCONTROL | DT_CALCRECT | DT_EXTERNALLEADING | DT_LEFT | DT_NOPREFIX | DT_TOP | DT_WORDBREAK,
				nullptr);
		}
		else
		{
			// calculate with ellipsis
			const auto len = sText.GetLength();
			std::vector<TCHAR> v(len + 10); // need at least 3 characters more - do 10
			memset(v.data(), 0, sizeof(TCHAR) * (len + 10));
			_tcsncat(v.data(), sText.GetBuffer(), (len + 9));

			::DrawTextEx(pDC->GetSafeHdc(), v.data(), len, &rcRez,
				DT_EXTERNALLEADING | DT_END_ELLIPSIS | DT_LEFT | DT_NOPREFIX | DT_SINGLELINE | DT_CALCRECT,
				NULL);
		}

		if (width < rcRez.right - rcRez.left)
		{
			width = rcRez.right - rcRez.left;
		}
		if (width > szRestrictions.cx)
		{
			width > szRestrictions.cx;
		}
		rcText.right = rcText.left + width; // adjust target rectangle
		rcText.bottom = rcText.top + rcRez.bottom - rcRez.top; // adjust target rectangle

		initial.y = rcText.bottom; // shift to the next item

	}
	szToFill.cx = width;
	szToFill.cy = initial.y;
	if (szToFill.cy > szRestrictions.cy)
	{
		szToFill.cy = szRestrictions.cy;
	}

	return true;
}

/**
 *	Queries the window size for the text
 *
 *	@param[in] *pDC pointer to output DC that will be used to query 
 *	the size from
 *	@param[in] szText Text to be wrapped to the window
 *	@param[in] &rctReq requested rectangle for the window
 *	@return	CSize resulting window size
 *	@author	Chris Payson (CSP)
 *	@since 5 Feb 2007
 */
CSize CPopupWindow::QueryWindowSize(CDC *pDC, LPCTSTR szText, const CRect &rctReq)
{
	// 5 Feb 2007 CSP Limit the window width to the size of the parent window
	CRect rctWnd(0, 0, 0, 0);
	CWnd *pParent = GetParent();

	if (pParent)
	{
		pParent->GetClientRect(&rctWnd);
	}
	else
	{
		rctWnd.right = pDC->GetDeviceCaps(HORZRES);
		rctWnd.bottom = pDC->GetDeviceCaps(VERTRES);
	}
	
	// and limit the window to 3.25 inches wide and 3 inches tall.
	const int nMaxTextWidth = __min(MAX_WIDTH(pDC), rctWnd.Width());
	const int nMaxTextHeight = __min(MAX_HEIGHT(pDC), rctWnd.Height());

	// Ideally, we're looking to work with a rectangle 
	// that is 3.25 inches wide, but no more.
	CRect rct = rctReq;
	rct.right = rct.left + __min(rct.Width(), nMaxTextWidth);

	std::shared_ptr<CFont> pOldFont(pDC->SelectObject(m_pFont), [pDC](CFont* p){ pDC->SelectObject(p); });

	SIZE toCalculate{MAX_WIDTH(pDC), MAX_HEIGHT(pDC)};
	if (CalculateNecessarySize(toCalculate, pDC))
	{
		rct.right = rct.left + toCalculate.cx;
		rct.bottom = rct.top + toCalculate.cy;
	}
	else
	{
		const int nHeight = pDC->DrawText(szText, &rct, TEXTOUT_FLAGS | DT_CALCRECT);

		rct.right = rct.left + __min(rct.Width(), nMaxTextWidth);
		rct.bottom = rct.top + __min(nHeight, nMaxTextHeight);
	}

	return (GetOuterRectSize(rct));
}

/**
 *	Gets the rectangle used to set the position of the rectangle
 *
 *	@return	CRect rectangle that was used to set the position of the rectangle
 *	@author	Chris Payson (CSP)
 *	@since 26 Feb 2007
 */
CRect CPopupWindow::GetPosition() const
{	return (m_rctPosition);
}

/**
 *	Set the font for the popup window
 *
 *	@param[in] *pFont pointer to font.  Caller is responsible for
 *	fonts lifetime and scope.
 *	@author	Chris Payson (CSP)
 *	@since 16 Mar 2007
 */
void CPopupWindow::SetFont(CFont *pFont)
{
	m_pFont = pFont;
}

void CPopupWindow::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (m_HintLines.size() > 0)
	{
		const auto nLnk = m_HintLines.size();
		auto linkPos = LinkPosition(point);
		const bool bInLink(linkPos < nLnk);

		m_linkPosition = bInLink ? linkPos : nLnk + 1; // save mouse down pos
	}
}

void CPopupWindow::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if (m_HintLines.size() > 0)
	{
		const auto nLnk = m_HintLines.size();
		auto linkPos = LinkPosition(point);
		const bool bInLink(linkPos < nLnk);
		if (bInLink && linkPos == m_linkPosition)
		{
			const auto& data = m_HintLines[linkPos];
			auto sLink = std::get<1>(data);
			if (sLink.GetLength() > 0)
			{
				::ShellExecute(NULL, _T("open"), sLink.GetBuffer(), NULL, NULL, SW_SHOWNORMAL);
			}
		}
		m_linkPosition = nLnk + 1; // link click invalidated
	}
	else
	{
		if(wcslen(m_szSourceLink)>0)
		{
			::ShellExecute(NULL, _T("open"), m_szSourceLink, NULL, NULL, SW_SHOWNORMAL);
		}
	}

	CWnd::OnLButtonUp(nFlags, point);
}

void CPopupWindow::SetSourceLink(LPCTSTR szText)
{
	m_szSourceLink = szText;
	m_szSourceLink.Remove('"');
    m_szSourceLink.Replace(_T("&amp;"), _T("&"));
    m_szSourceLink.Replace(_T("&quot;"), _T("\""));
}

void CPopupWindow::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_HintLines.size() > 0)
	{
		const auto nLnk = m_HintLines.size();
		auto linkPos = LinkPosition(point);
		const bool bInLink(linkPos < nLnk);

		if (bInLink)
		{
			::SetCursor(::LoadCursor(NULL, IDC_HAND));
		}else
		{
			::SetCursor(::LoadCursor(NULL, IDC_ARROW));
			if (m_linkPosition < nLnk)
			{
				m_linkPosition = nLnk + 1; // link click moved out (in case if it was btnDown)
			}
		}


	}

	CWnd::OnMouseMove(nFlags, point);
}

size_t CPopupWindow::LinkPosition(POINT const apt) const
{
	size_t ret = m_HintLines.size() + 1;
	size_t index = 0;
	for (const auto& data : m_HintLines) // for every link
	{
		const auto bLink = std::get<1>(data).GetLength() > 0;
		if (bLink)
		{
			CRect rcText = std::get<3>(data);
			rcText.OffsetRect(0, m_triangleShift);
			if (rcText.PtInRect(apt))
			{
				ret = index;
				break;
			};
		}
		++index;
	} // for (auto& data : m_HintLines) // for every link
	return ret;
}
