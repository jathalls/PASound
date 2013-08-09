
// ChildView.h : interface of the CChildView class
//


#pragma once
#include "PortAudioSound.h"


// CChildView window

class CChildView : public CWnd
{
// Construction
public:
	CChildView();

// Attributes
public:

// Operations
public:

// Overrides
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// Implementation
public:
	virtual ~CChildView();
	PortAudioSound *pPortAudioSound;
	BYTE col;

	// Generated message map functions
protected:
	
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	
protected:
//	afx_msg LRESULT WM_USER(WPARAM wParam, LPARAM lParam);
//	afx_msg LRESULT OnUser(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnWMUser(WPARAM wParam, LPARAM lParam);
};

