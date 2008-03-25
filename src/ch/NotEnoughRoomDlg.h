/***************************************************************************
*   Copyright (C) 2001-2008 by J�zef Starosczyk                           *
*   ixen@copyhandler.com                                                  *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU Library General Public License          *
*   (version 2) as published by the Free Software Foundation;             *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU Library General Public     *
*   License along with this program; if not, write to the                 *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/
#ifndef __NOTENOUGHROOMDLG_H__
#define __NOTENOUGHROOMDLG_H__

/////////////////////////////////////////////////////////////////////////////
// CNotEnoughRoomDlg dialog

class CNotEnoughRoomDlg : public ictranslate::CLanguageDialog
{
// Construction
public:
	CNotEnoughRoomDlg();   // standard constructor

// Dialog Data
	//{{AFX_DATA(CNotEnoughRoomDlg)
	enum { IDD = IDD_FEEDBACK_NOTENOUGHPLACE_DIALOG };
	CListBox	m_ctlFiles;
	//}}AFX_DATA

	CString m_strTitle;
	CStringArray m_strFiles;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNotEnoughRoomDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
public:
	bool m_bEnableTimer;
	int m_iTime;
	int m_iDefaultOption;
	CString	m_strDisk;
	__int64 m_llRequired;

protected:
	void UpdateDialog();
	virtual void OnLanguageChanged();

	// Generated message map functions
	//{{AFX_MSG(CNotEnoughRoomDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnRetryButton();
	afx_msg void OnIgnoreButton();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif
