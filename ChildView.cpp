
// ChildView.cpp : implementation of the CChildView class
//

#include "stdafx.h"
#include "PASound.h"
#include "ChildView.h"
#include "PortAudioSound.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CChildView

CChildView::CChildView()
{
}

CChildView::~CChildView()
{
	if (timerID != 0){
		KillTimer(timerID);
		timerID = 0;
	}
}


BEGIN_MESSAGE_MAP(CChildView, CWnd)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
//	ON_MESSAGE(0x0400, &CChildView::WM_USER)
//	ON_MESSAGE(WM_USER, &CChildView::OnUser)
ON_MESSAGE(0x0400, &CChildView::OnWMUser)
ON_WM_KEYUP()
ON_WM_TIMER()
END_MESSAGE_MAP()



// CChildView message handlers

BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
		::LoadCursor(NULL, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW+1), NULL);
	currentColumn = 0;
	currentRow = 0;
	offset = 20000.0;
	scale = 1/650000000.0;
	maxi = 0.0000001;
	mini = 6500000000.0;
	isDisplaying = FALSE;
	return TRUE;
}

void CChildView::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	
	// Do not call CWnd::OnPaint() for painting messages
}



BOOL CChildView::OnEraseBkgnd(CDC* pDC)
{
	/*CRect drawing_Area;
	GetClientRect(&drawing_Area);

	COLORREF backcolor = RGB(0, 0, 0);
	CBrush brush_back_ground(backcolor);

	pDC->FillRect(&drawing_Area, &brush_back_ground);*/

	

	return TRUE;
}


//afx_msg LRESULT CChildView::WM_USER(WPARAM wParam, LPARAM lParam)
//{
//	return 0;
//}




afx_msg LRESULT CChildView::OnWMUser(WPARAM wParam, LPARAM lParam)
{
	if (!isDisplaying){
		isDisplaying = TRUE;
		CDC *pDC = GetDC();
		CRect drawing_Area;
		GetClientRect(drawing_Area);
		if (pPortAudioSound->DrawSpectrogram(pDC, &drawing_Area)){
			//if (TRUE){
			InvalidateRect(drawing_Area, 1);
			UpdateWindow();
		}
		ReleaseDC(pDC);
		isDisplaying = FALSE;
	}

	return 0;
}


void CChildView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default

	CWnd::OnKeyUp(nChar, nRepCnt, nFlags);

	if (nChar == '4'){
		timerID = SetTimer(IDT_TIMER_0, 240000, NULL);
		if (timerID == 0){
			return;
		}
		
		CDC *pDC = GetDC();
		CRect drawing_Area;
		GetClientRect(drawing_Area);
		pDC->TextOutW(10, 280, L"Recording...");

		ReleaseDC(pDC);

		pPortAudioSound->StartRecord();

		
	}
}

void CChildView::OnTimer(UINT TimerVal){
	if (TimerVal == timerID){
		pPortAudioSound->StopRecord();



		KillTimer(TimerVal);

		CDC *pDC = GetDC();
		CRect drawing_Area;
		GetClientRect(drawing_Area);
		pDC->TextOutW(10, 280, L"                    ");

		ReleaseDC(pDC);
	}
}


