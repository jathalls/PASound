
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
}


BEGIN_MESSAGE_MAP(CChildView, CWnd)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
//	ON_MESSAGE(0x0400, &CChildView::WM_USER)
//	ON_MESSAGE(WM_USER, &CChildView::OnUser)
ON_MESSAGE(0x0400, &CChildView::OnWMUser)
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

/*
afx_msg LRESULT CChildView::OnUser(WPARAM wParam, LPARAM lParam)
{
	CDC *pDC = GetDC();
	if (pPAS->ReadIndex != pPAS->WriteIndex){

		CRect drawing_Area;
		
		
		int readIndex = pPAS->ReadIndex;
		while (pPAS->ppPowSpect[readIndex][0] != 0.0){
			//display
			BYTE grey = (BYTE) pPAS->ppPowSpect[readIndex][10];
			COLORREF bcolor = RGB(grey, grey, grey);
			CBrush brushbg(bcolor);
			pDC->FillRect(&drawing_Area, &brushbg);
			pPAS->ppPowSpect[readIndex][0] = 0.0;
			readIndex= ++readIndex%pPAS->numBuffers;
			readIndex = pPAS->ReadIndex;
			if (readIndex == pPAS->WriteIndex) break;
		}
		pPAS->ReadIndex = readIndex;
	}

	return 0;
}*/


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
