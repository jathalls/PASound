
// ChildView.h : interface of the CChildView class
//


#pragma once
//#include "PortAudioSound.h"

#define IDT_TIMER_0	(WM_USER+200)

class PortAudioSound;
// CChildView window

class CChildView : public CWnd
{
// Construction
public:
	CChildView();

// Attributes
public:
	PortAudioSound *pPortAudioSound;
// Operations
public:

// Overrides
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// Implementation
public:
	virtual ~CChildView();
	float offset;
	float scale;
	double maxi;
	double mini;

	
	BYTE col;
	int currentColumn;
	int currentRow;

protected:
	bool isDisplaying;
	UINT timerID;

	

	

	// Generated message map functions
protected:
	
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
protected:
//	afx_msg LRESULT WM_USER(WPARAM wParam, LPARAM lParam);
//	afx_msg LRESULT OnUser(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnWMUser(WPARAM wParam, LPARAM lParam);


	afx_msg void OnTimer(UINT TimerVal);
	// no mor messages after OnTimer please
public:
	
};

