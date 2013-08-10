
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
	
	CDC *pDC = GetDC();
	
	if (pPortAudioSound->ReadIndex != pPortAudioSound->WriteIndex){

		CRect drawing_Area;
		GetClientRect(drawing_Area);
		CString str;
		

		


		int readIndex = pPortAudioSound->ReadIndex;
		while (pPortAudioSound->ppPowSpect[readIndex][0] != 0.0){
			//display
			//BYTE grey = (BYTE) pPortAudioSound->ppPowSpect[readIndex][10];
			//COLORREF bcolor = RGB(col,col,col);
			
			//str.Format(L"Display %d 10=%f\n", (int) col, pPortAudioSound->ppPowSpect[readIndex][10]);
			//OutputDebugString(str);
			//col = ++col % 255;
			//CBrush brushbg(bcolor);
			//pDC->FillRect(&drawing_Area, &brushbg);
			

			for (int x = 0; x < 256;x++){
				if (!isnan(pPortAudioSound->ppPowSpect[readIndex][256 - x])){
					//if (pPortAudioSound->ppPowSpect[readIndex][256 - x] > max)max = pPortAudioSound->ppPowSpect[readIndex][256 - x];
					//if (pPortAudioSound->ppPowSpect[readIndex][256 - x] < min)min = pPortAudioSound->ppPowSpect[readIndex][256 - x];
					maxi = max(pPortAudioSound->ppPowSpect[readIndex][256 - x], maxi);
					mini = min(pPortAudioSound->ppPowSpect[readIndex][256 - x], mini);
				}
				BYTE shade = 256-(BYTE) (((pPortAudioSound->ppPowSpect[readIndex][256 - x]-mini)/maxi)*255.0) ;
				
				COLORREF bcolor = RGB(shade, shade, shade);
				pDC->SetPixel(currentColumn, x+(currentRow*300), bcolor);
			}
			currentColumn = ++currentColumn;
			if (currentColumn >= drawing_Area.Width()){
				currentColumn = 0;
				currentRow = ++currentRow % 2;
			}
/*			BYTE shaded = (BYTE) (((pPortAudioSound->ppPowSpect[readIndex][256 - 128]-mini)/maxi)*255.0);
			str.Format(L"at %d  from %f-%f/%f=%f shade=%d\n",currentColumn,pPortAudioSound->ppPowSpect[readIndex][256-128],maxi,mini,
				(pPortAudioSound->ppPowSpect[readIndex][256-128]-mini)/maxi,shaded);
			OutputDebugString(str);*/

			pPortAudioSound->ppPowSpect[readIndex][0] = 0.0;
			readIndex = ++readIndex%pPortAudioSound->numBuffers;
			readIndex = pPortAudioSound->ReadIndex;
			if (readIndex == pPortAudioSound->WriteIndex) break;
		}
		pPortAudioSound->ReadIndex = readIndex;
		InvalidateRect(drawing_Area, 1);
		UpdateWindow();
	}

	return 0;
}
