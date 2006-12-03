/************************************************************************
	Copy Handler 1.x - program for copying data in Microsoft Windows
						 systems.
	Copyright (C) 2001-2004 Ixen Gerthannes (copyhandler@o2.pl)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*************************************************************************/
#ifndef __MINIVIEWDLG_H__
#define __MINIVIEWDLG_H__

/////////////////////////////////////////////////////////////////////////////
// CMiniViewDlg dialog
#include "structs.h"
#include "ProgressListBox.h"
#include "resource.h"

#define BTN_COUNT 5

#define MSG_DRAWBUTTON	1
#define MSG_ONCLICK		2

#define WM_MINIVIEWDBLCLK		WM_USER+14

class CMiniViewDlg : public CHLanguageDialog
{
// internal struct
public:
	struct _BTNDATA_
	{
		void (*pfnCallbackFunc)(CMiniViewDlg*, UINT, _BTNDATA_*, CDC*);		// callback - click
		int iPosition;		// button pos counting from right
		bool bPressed;		// is it pressed ?
		bool bEnabled;		// is it enabled ?

		CRect rcButton;		// filled in OnNCPaint
	} m_bdButtons[BTN_COUNT];

// Construction
public:
	CMiniViewDlg(CTaskArray* pArray, bool* pbHide, CWnd* pParent = NULL);   // standard constructor

	void ShowWindow();
	void HideWindow();
	void ResizeDialog();
	friend void OnRestartBtn(CMiniViewDlg* pDlg, UINT uiMsg, CMiniViewDlg::_BTNDATA_* pData, CDC* pDC=NULL);
	friend void OnCancelBtn(CMiniViewDlg* pDlg, UINT uiMsg, CMiniViewDlg::_BTNDATA_* pData, CDC* pDC=NULL);
	friend void OnResume(CMiniViewDlg* pDlg, UINT uiMsg, CMiniViewDlg::_BTNDATA_* pData, CDC* pDC=NULL);
	friend void OnPause(CMiniViewDlg* pDlg, UINT uiMsg, CMiniViewDlg::_BTNDATA_* pData, CDC* pDC=NULL);
	friend void OnCloseBtn(CMiniViewDlg* pDlg, UINT uiMsg, CMiniViewDlg::_BTNDATA_* pData, CDC* pDC);

	void RefreshStatus();
	void RecalcSize(int nHeight, bool bInitial);

	virtual UINT GetLanguageUpdateOptions() { return LDF_NODIALOGSIZE; };
	virtual void OnLanguageChanged(WORD wOld, WORD wNew);

	// from CMainWnd
	CTaskArray *m_pTasks;

	CBrush m_brBackground;
	int m_iLastHeight;
	bool m_bShown;
	_PROGRESSITEM_ item;

	// cache
	TASK_MINI_DISPLAY_DATA dd;
	bool m_bActive;

	// lock
	static bool m_bLock;
	bool *m_pbHide;		// does the big status dialog visible ?

	// in onmousemove points to last pressed button
	int m_iIndex;

// Dialog Data
	//{{AFX_DATA(CMiniViewDlg)
	enum { IDD = IDD_MINIVIEW_DIALOG };
	CProgressListBox	m_ctlStatus;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMiniViewDlg)
	public:
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CMiniViewDlg)
	afx_msg HBRUSH OnCtlColor(CDC*, CWnd*, UINT);
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnSelchangeProgressList();
	afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnNcPaint();
	afx_msg BOOL OnNcActivate(BOOL bActive);
	afx_msg void OnSetfocusProgressList();
	afx_msg void OnSelcancelProgressList();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	afx_msg void OnDblclkProgressList();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif