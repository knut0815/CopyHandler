/***************************************************************************
*   Copyright (C) 2001-2008 by Jozef Starosczyk                           *
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
#include "stdafx.h"
#include "ch.h"
#include "resource.h"
#include "StatusDlg.h"
#include "BufferSizeDlg.h"
#include "StringHelpers.h"
#include "StaticEx.h"
#include "Structs.h"
#include "CfgProperties.h"
#include "../libchengine/TTaskManager.h"
#include "../libchengine/TLocalFilesystem.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

bool CStatusDlg::m_bLock=false;

/////////////////////////////////////////////////////////////////////////////
// CStatusDlg dialog

CStatusDlg::CStatusDlg(chengine::TTaskManager* pTasks, CWnd* pParent /*=nullptr*/)
	: ictranslate::CLanguageDialog(IDD_STATUS_DIALOG, pParent, &m_bLock),
	m_pTasks(pTasks),
	m_spTaskMgrStats(new chengine::TTaskManagerStatsSnapshot)
{
	RegisterStaticExControl(AfxGetInstanceHandle());
}

CStatusDlg::~CStatusDlg()
{
}

void CStatusDlg::DoDataExchange(CDataExchange* pDX)
{
	CLanguageDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TASKCOUNT_PROGRESS, m_ctlTaskCountProgress);
	DDX_Control(pDX, IDC_TASKSIZE_PROGRESS, m_ctlTaskSizeProgress);
	DDX_Control(pDX, IDC_CURRENTOBJECT_PROGRESS, m_ctlCurrentObjectProgress);
	DDX_Control(pDX, IDC_SUBTASKCOUNT_PROGRESS, m_ctlSubTaskCountProgress);
	DDX_Control(pDX, IDC_SUBTASKSIZE_PROGRESS, m_ctlSubTaskSizeProgress);
	DDX_Control(pDX, IDC_GLOBAL_PROGRESS, m_ctlProgressAll);
	DDX_Control(pDX, IDC_STATUS_LIST, m_ctlStatusList);
}

BEGIN_MESSAGE_MAP(CStatusDlg,ictranslate::CLanguageDialog)
	//{{AFX_MSG_MAP(CStatusDlg)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_PAUSE_BUTTON, OnPauseButton)
	ON_BN_CLICKED(IDC_CANCEL_BUTTON, OnCancelButton)
	ON_BN_CLICKED(IDC_SET_PRIORITY_BUTTON, OnSetPriorityButton)
	ON_BN_CLICKED(IDC_TASK_ADVANCED_BUTTON, OnTaskAdvancedOptions)
	ON_BN_CLICKED(IDC_SET_BUFFERSIZE_BUTTON, OnSetBuffersizeButton)
	ON_BN_CLICKED(IDC_START_ALL_BUTTON, OnStartAllButton)
	ON_BN_CLICKED(IDC_RESTART_BUTTON, OnRestartButton)
	ON_BN_CLICKED(IDC_DELETE_BUTTON, OnDeleteButton)
	ON_BN_CLICKED(IDC_PAUSE_ALL_BUTTON, OnPauseAllButton)
	ON_BN_CLICKED(IDC_RESTART_ALL_BUTTON, OnRestartAllButton)
	ON_BN_CLICKED(IDC_CANCEL_ALL_BUTTON, OnCancelAllButton)
	ON_BN_CLICKED(IDC_REMOVE_FINISHED_BUTTON, OnRemoveFinishedButton)
	ON_NOTIFY(LVN_KEYDOWN, IDC_STATUS_LIST, OnKeydownStatusList)
	ON_NOTIFY(LVN_CHANGEDSELECTION, IDC_STATUS_LIST, OnSelectionChanged)
	ON_BN_CLICKED(IDC_SHOW_LOG_BUTTON, OnShowLogButton)
	ON_BN_CLICKED(IDC_STICK_BUTTON, OnStickButton)
	ON_BN_CLICKED(IDC_RESUME_BUTTON, OnResumeButton)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CStatusDlg message handlers

BOOL CStatusDlg::OnInitDialog() 
{
	CLanguageDialog::OnInitDialog();

	PrepareResizableControls();

	// set dialog icon
	HICON hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	SetIcon(hIcon, FALSE);

	// get size of list ctrl
	CRect rcList;
	m_ctlStatusList.GetWindowRect(&rcList);
	int iWidth=rcList.Width();

	// set additional styles
	m_ctlStatusList.SetExtendedStyle(m_ctlStatusList.GetExtendedStyle() | LVS_EX_FULLROWSELECT);

	// add columns
	LVCOLUMN lvc;
	lvc.mask=LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
	lvc.fmt=LVCFMT_LEFT;

	lvc.pszText=(PTSTR)GetResManager().LoadString(IDS_COLUMNSTATUS_STRING);
	lvc.cchTextMax = lstrlen(lvc.pszText); 
	lvc.cx = static_cast<int>(0.27*iWidth);
	lvc.iSubItem=-1;
	m_ctlStatusList.InsertColumn(1, &lvc);

	lvc.pszText=(PTSTR)GetResManager().LoadString(IDS_COLUMNSOURCE_STRING);
	lvc.cchTextMax = lstrlen(lvc.pszText); 
	lvc.cx = static_cast<int>(0.3*iWidth);
	lvc.iSubItem=0;
	m_ctlStatusList.InsertColumn(2, &lvc);

	lvc.pszText=(PTSTR)GetResManager().LoadString(IDS_COLUMNDESTINATION_STRING);
	lvc.cchTextMax = lstrlen(lvc.pszText);
	lvc.cx = static_cast<int>(0.27*iWidth);
	lvc.iSubItem=1;
	m_ctlStatusList.InsertColumn(3, &lvc);

	lvc.pszText=(PTSTR)GetResManager().LoadString(IDS_COLUMNPROGRESS_STRING);
	lvc.cchTextMax = lstrlen(lvc.pszText);
	lvc.cx = static_cast<int>(0.15*iWidth);
	lvc.iSubItem=2;
	m_ctlStatusList.InsertColumn(4, &lvc);

	// images
	m_images.Create(16, 16, ILC_COLOR16 | ILC_MASK, 0, 3);
	m_images.Add(AfxGetApp()->LoadIcon(MAKEINTRESOURCE(IDI_WORKING_ICON)));
	m_images.Add(AfxGetApp()->LoadIcon(MAKEINTRESOURCE(IDI_ERROR_ICON)));
	m_images.Add(AfxGetApp()->LoadIcon(MAKEINTRESOURCE(IDI_PAUSED_ICON)));
	m_images.Add(AfxGetApp()->LoadIcon(MAKEINTRESOURCE(IDI_FINISHED_ICON)));
	m_images.Add(AfxGetApp()->LoadIcon(MAKEINTRESOURCE(IDI_CANCELLED_ICON)));
	m_images.Add(AfxGetApp()->LoadIcon(MAKEINTRESOURCE(IDI_WAITING_ICON)));

	m_ctlStatusList.SetImageList(&m_images, LVSIL_SMALL);

	// set fixed progresses ranges
	m_ctlTaskCountProgress.SetRange32(0, 100);
	m_ctlProgressAll.SetRange32(0, 100);

	// change the size of a dialog
	StickDialogToScreenEdge();
//	ApplyButtonsState();
//	EnableControls(false);

	// refresh data
	RefreshStatus();

	// select needed element
	SelectInitialTask();

	// refresh data timer
	SetTimer(777, GetPropValue<PP_STATUSREFRESHINTERVAL>(GetConfig()), nullptr);

	return TRUE;
}

void CStatusDlg::SelectInitialTask()
{
	size_t stIndex = 0;
	bool bSelected = false;
	while(stIndex < m_pTasks->GetSize())
	{
		chengine::TTaskPtr spTask = m_pTasks->GetAt(stIndex);
		if(m_spInitialSelection)
		{
			if(spTask == m_spInitialSelection)
			{
				m_ctlStatusList.SetItemState(boost::numeric_cast<int>(stIndex), LVIS_SELECTED, LVIS_SELECTED);
				bSelected = true;
				break;
			}
		}
		else
		{
			if(spTask->IsRunning())
			{
				m_ctlStatusList.SetItemState(boost::numeric_cast<int>(stIndex), LVIS_SELECTED, LVIS_SELECTED);
				bSelected = true;
				break;
			}
		}

		stIndex++;
	}

	if(!bSelected && m_pTasks->GetSize() > 0)
	{
		stIndex = m_pTasks->GetSize() - 1;
		m_ctlStatusList.SetItemState(boost::numeric_cast<int>(stIndex), LVIS_SELECTED, LVIS_SELECTED);
	}
}

void CStatusDlg::EnableControls(bool bEnable)
{
	// enable/disable controls
	GetDlgItem(IDC_SET_BUFFERSIZE_BUTTON)->EnableWindow(bEnable);
	GetDlgItem(IDC_SET_PRIORITY_BUTTON)->EnableWindow(bEnable);
	GetDlgItem(IDC_TASK_ADVANCED_BUTTON)->EnableWindow(bEnable);

	if (!bEnable)
	{
		GetDlgItem(IDC_TASKID_STATIC)->SetWindowText(GetResManager().LoadString(IDS_EMPTYTASKID_STRING));

		GetDlgItem(IDC_OPERATION_STATIC)->SetWindowText(GetResManager().LoadString(IDS_EMPTYOPERATIONTEXT_STRING));
		GetDlgItem(IDC_SOURCEOBJECT_STATIC)->SetWindowText(GetResManager().LoadString(IDS_EMPTYSOURCETEXT_STRING));
		GetDlgItem(IDC_DESTINATIONOBJECT_STATIC)->SetWindowText(GetResManager().LoadString(IDS_EMPTYDESTINATIONTEXT_STRING));
		GetDlgItem(IDC_BUFFERSIZE_STATIC)->SetWindowText(GetResManager().LoadString(IDS_EMPTYBUFFERSIZETEXT_STRING));
		GetDlgItem(IDC_THREADPRIORITY_STATIC)->SetWindowText(GetResManager().LoadString(IDS_EMPTYPRIORITYTEXT_STRING));
		
		// subtask
		GetDlgItem(IDC_SUBTASKNAME_STATIC)->SetWindowText(GetResManager().LoadString(IDS_EMPTYSUBTASKNAME_STRING));
		GetDlgItem(IDC_SUBTASKPROCESSED_STATIC)->SetWindowText(GetResManager().LoadString(IDS_EMPTYPROCESSEDTEXT_STRING));
		GetDlgItem(IDC_SUBTASKTIME_STATIC)->SetWindowText(GetResManager().LoadString(IDS_EMPTYTIMETEXT_STRING));
		GetDlgItem(IDC_SUBTASKTRANSFER_STATIC)->SetWindowText(GetResManager().LoadString(IDS_EMPTYTRANSFERTEXT_STRING));

		GetDlgItem(IDC_TASKPROCESSED_STATIC)->SetWindowText(GetResManager().LoadString(IDS_EMPTYPROCESSEDTEXT_STRING));
		GetDlgItem(IDC_TASKTRANSFER_STATIC)->SetWindowText(GetResManager().LoadString(IDS_EMPTYTRANSFERTEXT_STRING));
		GetDlgItem(IDC_TASKTIME_STATIC)->SetWindowText(GetResManager().LoadString(IDS_EMPTYTIMETEXT_STRING));

		m_ctlTaskCountProgress.SetPos(0);
		m_ctlTaskSizeProgress.SetPos(0);
		m_ctlCurrentObjectProgress.SetPos(0);
		m_ctlSubTaskCountProgress.SetPos(0);
		m_ctlSubTaskSizeProgress.SetPos(0);
	}
}

void CStatusDlg::OnTimer(UINT_PTR nIDEvent) 
{
	if (nIDEvent == 777)	// refreshing data
	{
		// turn off timer for some time
		KillTimer(777);

		RefreshStatus();

		// reenable
		SetTimer(777, GetPropValue<PP_STATUSREFRESHINTERVAL>(GetConfig()), nullptr);
	}

	CLanguageDialog::OnTimer(nIDEvent);
}

void CStatusDlg::OnSetBuffersizeButton()
{
	chengine::TTaskPtr spTask = GetSelectedItemPointer();
	if(!spTask)
		return;

	int iCurrentBufferIndex = 0;
	chengine::TTaskStatsSnapshotPtr spTaskStats = m_spTaskMgrStats->GetTaskStatsForTaskID(boost::numeric_cast<chengine::taskid_t>(GetSelectedItemSessionUniqueID()));
	if(spTaskStats)
	{
		chengine::TSubTaskStatsSnapshotPtr spSubTaskStats = spTaskStats->GetSubTasksStats().GetCurrentSubTaskSnapshot();
		if(spSubTaskStats)
			iCurrentBufferIndex = spSubTaskStats->GetCurrentBufferIndex();
	}

	chengine::TBufferSizes tBufferSizes;
	spTask->GetBufferSizes(tBufferSizes);

	CBufferSizeDlg dlg(&tBufferSizes, (chengine::TBufferSizes::EBufferType)iCurrentBufferIndex);
	if(dlg.DoModal() == IDOK)
		spTask->SetBufferSizes(dlg.GetBufferSizes());
}

chengine::TTaskPtr CStatusDlg::GetSelectedItemPointer()
{
	// returns ptr to a TTask for a given element in listview
	if(m_ctlStatusList.GetSelectedCount() == 1)
	{
		POSITION pos = m_ctlStatusList.GetFirstSelectedItemPosition();
		int nPos = m_ctlStatusList.GetNextSelectedItem(pos);
		return m_pTasks->GetTaskByTaskID(boost::numeric_cast<chengine::taskid_t>(m_ctlStatusList.GetItemData(nPos)));
	}

	return chengine::TTaskPtr();
}

size_t CStatusDlg::GetSelectedItemSessionUniqueID()
{
	// returns ptr to a TTask for a given element in listview
	if(m_ctlStatusList.GetSelectedCount() == 1)
	{
		POSITION pos = m_ctlStatusList.GetFirstSelectedItemPosition();
		int nPos = m_ctlStatusList.GetNextSelectedItem(pos);
		return m_ctlStatusList.GetItemData(nPos);
	}

	return std::numeric_limits<size_t>::max();
}

void CStatusDlg::StickDialogToScreenEdge()
{
	// get coord of screen and window
	CRect rcScreen, rect;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &rcScreen, 0);
	GetWindowRect(&rect);

	SetWindowPos(nullptr, rcScreen.right-rect.Width(),
		rcScreen.bottom-rect.Height(), rect.Width(), rect.Height(),
		SWP_NOOWNERZORDER | SWP_NOZORDER);
}

void CStatusDlg::ApplyButtonsState()
{
	// remember ptr to TTask
	chengine::TTaskPtr spSelectedTask = GetSelectedItemPointer();

	// set status of buttons pause/resume/cancel
	if (spSelectedTask != nullptr)
	{
		if(spSelectedTask->GetTaskState() == chengine::eTaskState_LoadError)
		{
			GetDlgItem(IDC_SHOW_LOG_BUTTON)->EnableWindow(true);
			GetDlgItem(IDC_PAUSE_BUTTON)->EnableWindow(false);
			GetDlgItem(IDC_RESUME_BUTTON)->EnableWindow(false);
			GetDlgItem(IDC_RESTART_BUTTON)->EnableWindow(false);
			GetDlgItem(IDC_CANCEL_BUTTON)->EnableWindow(false);
			GetDlgItem(IDC_DELETE_BUTTON)->EnableWindow(true);
		}
		else
		{
			GetDlgItem(IDC_RESTART_BUTTON)->EnableWindow(true);
			GetDlgItem(IDC_SHOW_LOG_BUTTON)->EnableWindow(true);
			GetDlgItem(IDC_DELETE_BUTTON)->EnableWindow(true);

			if (spSelectedTask->GetTaskState() == chengine::eTaskState_Finished || spSelectedTask->GetTaskState() == chengine::eTaskState_Cancelled)
			{
				GetDlgItem(IDC_CANCEL_BUTTON)->EnableWindow(false);
				GetDlgItem(IDC_PAUSE_BUTTON)->EnableWindow(false);
				GetDlgItem(IDC_RESUME_BUTTON)->EnableWindow(false);
			}
			else
			{
				// pause/resume
				if (spSelectedTask->GetTaskState() == chengine::eTaskState_Paused)
				{
					GetDlgItem(IDC_PAUSE_BUTTON)->EnableWindow(false);
					GetDlgItem(IDC_RESUME_BUTTON)->EnableWindow(true);
				}
				else
				{
					GetDlgItem(IDC_PAUSE_BUTTON)->EnableWindow(true);
					if (spSelectedTask->GetTaskState() == chengine::eTaskState_Waiting)
						GetDlgItem(IDC_RESUME_BUTTON)->EnableWindow(true);
					else
						GetDlgItem(IDC_RESUME_BUTTON)->EnableWindow(false);
				}

				GetDlgItem(IDC_CANCEL_BUTTON)->EnableWindow(true);
			}
		}
	}
	else
	{
		GetDlgItem(IDC_SHOW_LOG_BUTTON)->EnableWindow(false);
		GetDlgItem(IDC_PAUSE_BUTTON)->EnableWindow(false);
		GetDlgItem(IDC_RESUME_BUTTON)->EnableWindow(false);
		GetDlgItem(IDC_RESTART_BUTTON)->EnableWindow(false);
		GetDlgItem(IDC_CANCEL_BUTTON)->EnableWindow(false);
		GetDlgItem(IDC_DELETE_BUTTON)->EnableWindow(false);
	}
}

void CStatusDlg::OnSetPriorityButton() 
{
	CMenu menu;
	HMENU hMenu=GetResManager().LoadMenu(MAKEINTRESOURCE(IDR_PRIORITY_MENU));
	if (!menu.Attach(hMenu))
	{
		DestroyMenu(hMenu);
		return;
	}
	
	CMenu* pPopup = menu.GetSubMenu(0);
	ASSERT(pPopup != nullptr);
	if(pPopup)
	{
		// set point in which to set menu
		CRect rect;
		GetDlgItem(IDC_SET_PRIORITY_BUTTON)->GetWindowRect(&rect);

		pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, rect.right+1, rect.top, this);
	}
}

void CStatusDlg::OnTaskAdvancedOptions()
{
	CMenu menu;
	HMENU hMenu = GetResManager().LoadMenu(MAKEINTRESOURCE(IDR_TASK_ADVANCED_MENU));
	if(!menu.Attach(hMenu))
	{
		DestroyMenu(hMenu);
		return;
	}

	CMenu* pPopup = menu.GetSubMenu(0);
	ASSERT(pPopup != nullptr);
	if(pPopup)
	{
		// set point in which to set menu
		CRect rect;
		GetDlgItem(IDC_TASK_ADVANCED_BUTTON)->GetWindowRect(&rect);

		pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, rect.right + 1, rect.top, this);
	}
}

BOOL CStatusDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	if (HIWORD(wParam) == 0)
	{
		if (LOWORD(wParam) >= ID_POPUP_TIME_CRITICAL && LOWORD(wParam) <= ID_POPUP_IDLE)
		{
			// processing priority
			chengine::TTaskPtr spSelectedTask = GetSelectedItemPointer();

			if(spSelectedTask == nullptr)
				return ictranslate::CLanguageDialog::OnCommand(wParam, lParam);
			
			switch (LOWORD(wParam))
			{
			case ID_POPUP_TIME_CRITICAL:
				spSelectedTask->SetPriority(THREAD_PRIORITY_TIME_CRITICAL);
				GetDlgItem(IDC_THREADPRIORITY_STATIC)->SetWindowText(GetResManager().LoadString(IDS_PRIORITY0_STRING+PriorityToIndex(THREAD_PRIORITY_TIME_CRITICAL)));
				break;
			case ID_POPUP_HIGHEST:
				spSelectedTask->SetPriority(THREAD_PRIORITY_HIGHEST);
				GetDlgItem(IDC_THREADPRIORITY_STATIC)->SetWindowText(GetResManager().LoadString(IDS_PRIORITY0_STRING+PriorityToIndex(THREAD_PRIORITY_HIGHEST)));
				break;
			case ID_POPUP_ABOVE_NORMAL:
				spSelectedTask->SetPriority(THREAD_PRIORITY_ABOVE_NORMAL);
				GetDlgItem(IDC_THREADPRIORITY_STATIC)->SetWindowText(GetResManager().LoadString(IDS_PRIORITY0_STRING+PriorityToIndex(THREAD_PRIORITY_ABOVE_NORMAL)));
				break;
			case ID_POPUP_NORMAL:
				spSelectedTask->SetPriority(THREAD_PRIORITY_NORMAL);
				GetDlgItem(IDC_THREADPRIORITY_STATIC)->SetWindowText(GetResManager().LoadString(IDS_PRIORITY0_STRING+PriorityToIndex(THREAD_PRIORITY_NORMAL)));
				break;
			case ID_POPUP_BELOW_NORMAL:
				spSelectedTask->SetPriority(THREAD_PRIORITY_BELOW_NORMAL);
				GetDlgItem(IDC_THREADPRIORITY_STATIC)->SetWindowText(GetResManager().LoadString(IDS_PRIORITY0_STRING+PriorityToIndex(THREAD_PRIORITY_BELOW_NORMAL)));
				break;
			case ID_POPUP_LOWEST:
				spSelectedTask->SetPriority(THREAD_PRIORITY_LOWEST);
				GetDlgItem(IDC_THREADPRIORITY_STATIC)->SetWindowText(GetResManager().LoadString(IDS_PRIORITY0_STRING+PriorityToIndex(THREAD_PRIORITY_LOWEST)));
				break;
			case ID_POPUP_IDLE:
				spSelectedTask->SetPriority(THREAD_PRIORITY_IDLE);
				GetDlgItem(IDC_THREADPRIORITY_STATIC)->SetWindowText(GetResManager().LoadString(IDS_PRIORITY0_STRING+PriorityToIndex(THREAD_PRIORITY_IDLE)));
				break;
			}
		}
		else if(LOWORD(wParam) == ID_POPUP_RESET_APPLY_TO_ALL)
		{
			// processing priority
			chengine::TTaskPtr spSelectedTask = GetSelectedItemPointer();

			if(spSelectedTask == nullptr)
				return ictranslate::CLanguageDialog::OnCommand(wParam, lParam);

			spSelectedTask->RestoreFeedbackDefaults();
		}
	}
	return ictranslate::CLanguageDialog::OnCommand(wParam, lParam);
}

void CStatusDlg::OnPauseButton() 
{
	chengine::TTaskPtr spTask = GetSelectedItemPointer();
	if(spTask)
	{
		TRACE("PauseProcessing call...\n");
		spTask->PauseProcessing();

		RefreshStatus();
	}
}

void CStatusDlg::OnResumeButton() 
{
	chengine::TTaskPtr spTask = GetSelectedItemPointer();
	if(spTask)
	{
		if(spTask->GetTaskState() == chengine::eTaskState_Waiting)
			spTask->SetForceFlag();
		else
			spTask->ResumeProcessing();

		RefreshStatus();
	}
}

void CStatusDlg::OnCancelButton() 
{
	chengine::TTaskPtr spTask = GetSelectedItemPointer();
	if(spTask)
	{
		spTask->CancelProcessing();
		RefreshStatus();
	}
}

void CStatusDlg::OnRestartButton() 
{
	chengine::TTaskPtr spTask = GetSelectedItemPointer();
	if(spTask)
	{
		spTask->RestartProcessing();
		RefreshStatus();
	}
}

void CStatusDlg::OnDeleteButton() 
{
	chengine::TTaskPtr spTask = GetSelectedItemPointer();
	if(spTask)
	{
		chengine::ETaskCurrentState eTaskState = spTask->GetTaskState();
		switch(eTaskState)
		{
		case chengine::eTaskState_Finished:
		case chengine::eTaskState_Cancelled:
		case chengine::eTaskState_LoadError:
			break;	// allow processing as-is

		default:
			// ask to cancel
			if(MsgBox(IDS_CONFIRMCANCEL_STRING, MB_OKCANCEL | MB_ICONQUESTION) == IDOK)
				spTask->CancelProcessing();
			else
				return;
		}

		m_pTasks->RemoveFinished(spTask);
		RefreshStatus();
	}
}

void CStatusDlg::OnPauseAllButton() 
{
	TRACE("Pause All...\n");
	m_pTasks->TasksPauseProcessing();
	RefreshStatus();
}

void CStatusDlg::OnStartAllButton() 
{
	TRACE("Resume Processing...\n");
	m_pTasks->TasksResumeProcessing();
	RefreshStatus();
}

void CStatusDlg::OnRestartAllButton() 
{
	TRACE("Restart Processing...\n");
	m_pTasks->TasksRestartProcessing();	
	RefreshStatus();
}

void CStatusDlg::OnCancelAllButton() 
{
	TRACE("Cancel Processing...\n");
	m_pTasks->TasksCancelProcessing();	
	RefreshStatus();
}

void CStatusDlg::OnRemoveFinishedButton() 
{
	m_pTasks->RemoveAllFinished();
	RefreshStatus();
}

void CStatusDlg::OnKeydownStatusList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_KEYDOWN* pLVKeyDow = (LV_KEYDOWN*)pNMHDR;
	switch (pLVKeyDow->wVKey)
	{
	case VK_DELETE:
		OnDeleteButton();
		break;
	case VK_SPACE:
		{
			chengine::TTaskPtr spTask = GetSelectedItemPointer();
			if (!spTask)
				return;
		
			if(spTask->GetTaskState() == chengine::eTaskState_Paused)
				OnResumeButton();
			else
				OnPauseButton();
			break;
		}
	}

	*pResult = 0;
}

int CStatusDlg::GetImageFromStatus(chengine::ETaskCurrentState eState)
{
	switch(eState)
	{
	case chengine::eTaskState_Cancelled:
		return 4;
	case chengine::eTaskState_Finished:
		return 3;
	case chengine::eTaskState_Waiting:
		return 5;
	case chengine::eTaskState_Paused:
		return 2;
	case chengine::eTaskState_Error:
	case chengine::eTaskState_LoadError:
		return 1;
	default:
		return 0;
	}
}

CString CStatusDlg::FormatTime(unsigned long long timeSeconds)
{
	if(timeSeconds > 30*24*3600)	// more than 30 days
		return L"\u221E";	// infinity character

	long lDays = boost::numeric_cast<long>(timeSeconds/86400);
	timeSeconds %= 86400;
	long lHours = boost::numeric_cast<long>(timeSeconds/3600);
	timeSeconds %= 3600;
	long lMinutes = boost::numeric_cast<long>(timeSeconds/60);
	timeSeconds %= 60;

	CString strResult;
	if(lDays != 0)
		strResult.Format(_T("%02ld:%02ld:%02ld:%02I64u"), lDays, lHours, lMinutes, timeSeconds);
	else
	{
		if (lHours != 0)
			strResult.Format(_T("%02ld:%02ld:%02I64u"), lHours, lMinutes, timeSeconds);
		else
			strResult.Format(_T("%02ld:%02I64u"), lMinutes, timeSeconds);
	}

	return strResult;
}

CString CStatusDlg::FormatTimeMiliseconds(unsigned long long timeMiliSeconds)
{
	unsigned long long timeSeconds = timeMiliSeconds / 1000;
	return FormatTime(timeSeconds);
}

void CStatusDlg::RefreshStatus()
{
	// remember address of a current selection
	size_t stSelectedTaskID = GetSelectedItemSessionUniqueID();

	// get all the stats needed
	m_pTasks->GetStatsSnapshot(m_spTaskMgrStats);

	// get rid of item after the current part
	m_ctlStatusList.LimitItems(boost::numeric_cast<int>(m_spTaskMgrStats->GetTaskStatsCount()));

	// add task info
	for(size_t stIndex = 0; stIndex < m_spTaskMgrStats->GetTaskStatsCount(); ++stIndex)
	{
		chengine::TTaskStatsSnapshotPtr spTaskStats = m_spTaskMgrStats->GetTaskStatsAt(stIndex);
		// set (update/add new) entry in the task list (on the left)
		SetTaskListEntry(stIndex, spTaskStats);

		// right side update
		if(spTaskStats->GetTaskID() == stSelectedTaskID)
			UpdateTaskStatsDetails(spTaskStats);
	}

	// set title
	SetWindowTitle(GetProgressWindowTitleText());

	// refresh overall progress
	m_ctlProgressAll.SetRange(0, 100);
	m_ctlProgressAll.SetPos(boost::numeric_cast<int>(m_spTaskMgrStats->GetCombinedProgress() * 100.0));
	
	// progress - count of processed data/count of data
	CString strTemp;
	strTemp = GetSizeString(m_spTaskMgrStats->GetProcessedSize()) + CString(_T("/"));
	strTemp += GetSizeString(m_spTaskMgrStats->GetTotalSize());
	GetDlgItem(IDC_GLOBALPROCESSED_STATIC)->SetWindowText(strTemp);
	
	// transfer
	CString strSpeed = GetSpeedString(m_spTaskMgrStats->GetSizeSpeed(), m_spTaskMgrStats->GetAvgSizeSpeed(), m_spTaskMgrStats->GetCountSpeed(), m_spTaskMgrStats->GetAvgCountSpeed());
	GetDlgItem(IDC_GLOBALTRANSFER_STATIC)->SetWindowText(strSpeed);

	// if selection's missing - hide controls
	if (m_ctlStatusList.GetSelectedCount() == 0)
		EnableControls(false);
	else
		EnableControls();		// enable controls
	
	// apply state of the resume, cancel, ... buttons
	ApplyButtonsState();

	// update taskbar progress
	UpdateTaskBarProgress();
}

void CStatusDlg::OnSelectionChanged(NMHDR* /*pNMHDR*/, LRESULT* /*pResult*/)
{
	TRACE("Received LVN_CHANGEDSELECTION\n");
	RefreshStatus();
}

void CStatusDlg::OnCancel() 
{
	PostCloseMessage();
	CLanguageDialog::OnCancel();
}

void CStatusDlg::OnShowLogButton() 
{
	// show log
	chengine::TTaskPtr spTask = GetSelectedItemPointer();
	if(!spTask)
		return;

	chcore::TSmartPath logPath = spTask->GetLogPath();
	chengine::TLocalFilesystem localFilesystem(GetLogFileData());
	if(!localFilesystem.PathExist(logPath))
	{
		MsgBox(IDS_LOGFILEEMPTY_STRING, MB_OK | MB_ICONINFORMATION);
		return;
	}

	ULONG_PTR hResult = (ULONG_PTR)ShellExecute(this->m_hWnd, _T("open"), _T("notepad.exe"), logPath.ToString(), nullptr, SW_SHOWNORMAL);
	if(hResult < 32)
	{
		ictranslate::CFormat fmt(GetResManager().LoadString(IDS_SHELLEXECUTEERROR_STRING));
		fmt.SetParam(_T("%errno"), hResult);
		fmt.SetParam(_T("%path"), spTask->GetLogPath().ToString());
		AfxMessageBox(fmt.ToString());
	}
}

LRESULT CStatusDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	if (message == WM_UPDATESTATUS)
	{
		TRACE("Received WM_UPDATESTATUS\n");
		RefreshStatus();
	}
	return ictranslate::CLanguageDialog::WindowProc(message, wParam, lParam);
}

void CStatusDlg::OnStickButton() 
{
	StickDialogToScreenEdge();
}

void CStatusDlg::SetBufferSizesString(unsigned long long ullValue, int iIndex)
{
	CString strResult;
	switch(iIndex)
	{
	case chengine::TBufferSizes::eBuffer_Default:
		strResult = GetResManager().LoadString(IDS_BSDEFAULT_STRING);
		break;
	case chengine::TBufferSizes::eBuffer_OneDisk:
		strResult = GetResManager().LoadString(IDS_BSONEDISK_STRING);
		break;
	case chengine::TBufferSizes::eBuffer_TwoDisks:
		strResult = GetResManager().LoadString(IDS_BSTWODISKS_STRING);
		break;
	case chengine::TBufferSizes::eBuffer_CD:
		strResult = GetResManager().LoadString(IDS_BSCD_STRING);
		break;
	case chengine::TBufferSizes::eBuffer_LAN:
		strResult = GetResManager().LoadString(IDS_BSLAN_STRING);
		break;
	default:
		_ASSERTE(false);
	}

	strResult += GetSizeString(ullValue);

	GetDlgItem(IDC_BUFFERSIZE_STATIC)->SetWindowText(strResult);
}

void CStatusDlg::PostCloseMessage()
{
	GetParent()->PostMessage(WM_STATUSCLOSING);
}

void CStatusDlg::OnLanguageChanged()
{
	// remove all columns
	int iCnt=m_ctlStatusList.GetHeaderCtrl()->GetItemCount();

	// Delete all of the columns.
	for (int i=0;i<iCnt;i++)
		m_ctlStatusList.DeleteColumn(0);

	// get size of list ctrl
	CRect rcList;
	m_ctlStatusList.GetWindowRect(&rcList);
	int iWidth=rcList.Width();

	// refresh the header in a list
	LVCOLUMN lvc;
	lvc.mask=LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
	lvc.fmt=LVCFMT_LEFT;

	lvc.pszText=(PTSTR)GetResManager().LoadString(IDS_COLUMNSTATUS_STRING);
	lvc.cchTextMax = lstrlen(lvc.pszText); 
	lvc.cx = static_cast<int>(0.27*iWidth);
	lvc.iSubItem=-1;
	m_ctlStatusList.InsertColumn(1, &lvc);

	lvc.pszText=(PTSTR)GetResManager().LoadString(IDS_COLUMNSOURCE_STRING);
	lvc.cchTextMax = lstrlen(lvc.pszText); 
	lvc.cx = static_cast<int>(0.3*iWidth);
	lvc.iSubItem=0;
	m_ctlStatusList.InsertColumn(2, &lvc);

	lvc.pszText=(PTSTR)GetResManager().LoadString(IDS_COLUMNDESTINATION_STRING);
	lvc.cchTextMax = lstrlen(lvc.pszText);
	lvc.cx = static_cast<int>(0.27*iWidth);
	lvc.iSubItem=1;
	m_ctlStatusList.InsertColumn(3, &lvc);

	lvc.pszText=(PTSTR)GetResManager().LoadString(IDS_COLUMNPROGRESS_STRING);
	lvc.cchTextMax = lstrlen(lvc.pszText);
	lvc.cx = static_cast<int>(0.15*iWidth);
	lvc.iSubItem=2;
	m_ctlStatusList.InsertColumn(4, &lvc);

	RefreshStatus();
}

// ============================================================================
/// CStatusDlg::PrepareResizableControls
/// @date 2009/04/18
///
/// @brief     Prepares the resizable controls.
// ============================================================================
void CStatusDlg::PrepareResizableControls()
{
	ClearResizableControls();

	// left part of dialog (task list)
	AddResizableControl(IDC_TASKLIST_LABEL_STATIC, 0, 0, 0.5, 0.0);
	AddResizableControl(IDC_STATUS_LIST, 0, 0, 0.5, 1.0);

	// left part of dialog (buttons under the task list)
	AddResizableControl(IDC_PAUSE_BUTTON, 0, 1.0, 0, 0);
	AddResizableControl(IDC_RESTART_BUTTON, 0, 1.0, 0, 0);
	AddResizableControl(IDC_RESUME_BUTTON, 0, 1.0, 0, 0);
	AddResizableControl(IDC_CANCEL_BUTTON, 0, 1.0, 0, 0);
	AddResizableControl(IDC_DELETE_BUTTON, 0, 1.0, 0, 0);
	AddResizableControl(IDC_PAUSE_ALL_BUTTON, 0, 1.0, 0, 0);
	AddResizableControl(IDC_START_ALL_BUTTON, 0, 1.0, 0, 0);
	AddResizableControl(IDC_CANCEL_ALL_BUTTON, 0, 1.0, 0, 0);
	AddResizableControl(IDC_REMOVE_FINISHED_BUTTON, 0, 1.0, 0, 0);
	AddResizableControl(IDC_RESTART_ALL_BUTTON, 0, 1.0, 0, 0);
	AddResizableControl(IDC_TASK_ADVANCED_BUTTON, 0, 1.0, 0, 0);

	// left part of dialog (global stats)
	AddResizableControl(IDC_GLOBAL_GROUP_STATIC, 0.0, 1.0, 0.5, 0);

	AddResizableControl(IDC_GLOBALPROCESSED_LABEL_STATIC, 0.0, 1.0, 0.0, 0.0);
	AddResizableControl(IDC_GLOBALPROCESSED_STATIC, 0.0, 1.0, 0.5, 0);
	AddResizableControl(IDC_GLOBALTRANSFER_LABEL_STATIC, 0.0, 1.0, 0.0, 0.0);
	AddResizableControl(IDC_GLOBALTRANSFER_STATIC, 0.0, 1.0, 0.5, 0);
	AddResizableControl(IDC_GLOBALPROGRESS_LABEL_STATIC, 0.0, 1.0, 0.0, 0.0);
	AddResizableControl(IDC_GLOBAL_PROGRESS, 0.0, 1.0, 0.5, 0.0);

	// right part of dialog  (task info)
	AddResizableControl(IDC_TASKINFORMATION_GROUP_STATIC, 0.5, 0.0, 0.5, 0);

	// right part of dialog (subsequent entries)
	AddResizableControl(IDC_TASKID_LABEL_STATIC, 0.5, 0.0, 0.0, 0);
	AddResizableControl(IDC_TASKID_STATIC, 0.5, 0.0, 0.5, 0);

	AddResizableControl(IDC_OPERATION_LABEL_STATIC, 0.5, 0.0, 0.0, 0);
	AddResizableControl(IDC_OPERATION_STATIC, 0.5, 0.0, 0.5, 0);

	AddResizableControl(IDC_SOURCEOBJECT_LABEL_STATIC, 0.5, 0.0, 0.0, 0);
	AddResizableControl(IDC_SOURCEOBJECT_STATIC, 0.5, 0.0, 0.5, 0);

	AddResizableControl(IDC_DESTINATIONOBJECT_LABEL_STATIC, 0.5, 0.0, 0.0, 0);
	AddResizableControl(IDC_DESTINATIONOBJECT_STATIC, 0.5, 0.0, 0.5, 0);

	AddResizableControl(IDC_BUFFERSIZE_LABEL_STATIC, 0.5, 0.0, 0.0, 0);
	AddResizableControl(IDC_BUFFERSIZE_STATIC, 0.5, 0.0, 0.5, 0);
	AddResizableControl(IDC_SET_BUFFERSIZE_BUTTON, 1.0, 0.0, 0.0, 0.0);

	AddResizableControl(IDC_THREADPRIORITY_LABEL_STATIC, 0.5, 0.0, 0.0, 0);
	AddResizableControl(IDC_THREADPRIORITY_STATIC, 0.5, 0.0, 0.5, 0);
	AddResizableControl(IDC_SET_PRIORITY_BUTTON, 1.0, 0.0, 0.0, 0.0);

	// right part of the dialog (subtask stats)
	AddResizableControl(IDC_CURRENTPHASE_GROUP_STATIC, 0.5, 0.0, 0.5, 0);

	AddResizableControl(IDC_SUBTASKNAME_LABEL_STATIC, 0.5, 0.0, 0.0, 0);
	AddResizableControl(IDC_SUBTASKNAME_STATIC, 0.5, 0.0, 0.5, 0);

	AddResizableControl(IDC_SUBTASKPROCESSED_LABEL_STATIC, 0.5, 0.0, 0.0, 0);
	AddResizableControl(IDC_SUBTASKPROCESSED_STATIC, 0.5, 0.0, 0.5, 0);

	AddResizableControl(IDC_SUBTASKTIME_LABEL_STATIC, 0.5, 0.0, 0.0, 0);
	AddResizableControl(IDC_SUBTASKTIME_STATIC, 0.5, 0.0, 0.5, 0);

	AddResizableControl(IDC_SUBTASKTRANSFER_LABEL_STATIC, 0.5, 0.0, 0.0, 0);
	AddResizableControl(IDC_SUBTASKTRANSFER_STATIC, 0.5, 0.0, 0.5, 0);

	AddResizableControl(IDC_CURRENTOBJECT_LABEL_STATIC, 0.5, 0.0, 0.0, 0);
	AddResizableControl(IDC_CURRENTOBJECT_PROGRESS, 0.5, 0.0, 0.5, 0);

	AddResizableControl(IDC_SUBTASKCOUNT_LABEL_STATIC, 0.5, 0.0, 0.0, 0);
	AddResizableControl(IDC_SUBTASKCOUNT_PROGRESS, 0.5, 0.0, 0.5, 0);

	AddResizableControl(IDC_SUBTASKSIZE_LABEL_STATIC, 0.5, 0.0, 0.0, 0);
	AddResizableControl(IDC_SUBTASKSIZE_PROGRESS, 0.5, 0.0, 0.5, 0);

	// right part of the dialog (task stats)
	AddResizableControl(IDC_ENTIRETASK_GROUP_STATIC, 0.5, 0.0, 0.5, 0);

	AddResizableControl(IDC_TASKPROCESSED_LABEL_STATIC, 0.5, 0.0, 0.0, 0);
	AddResizableControl(IDC_TASKPROCESSED_STATIC, 0.5, 0.0, 0.5, 0);

	AddResizableControl(IDC_TASKTIME_LABEL_STATIC, 0.5, 0.0, 0.0, 0);
	AddResizableControl(IDC_TASKTIME_STATIC, 0.5, 0.0, 0.5, 0);

	AddResizableControl(IDC_TASKTRANSFER_LABEL_STATIC, 0.5, 0.0, 0.0, 0);
	AddResizableControl(IDC_TASKTRANSFER_STATIC, 0.5, 0.0, 0.5, 0);

	AddResizableControl(IDC_TASKCOUNT_LABEL_STATIC, 0.5, 0.0, 0.0, 0);
	AddResizableControl(IDC_TASKCOUNT_PROGRESS, 0.5, 0.0, 0.5, 0);

	AddResizableControl(IDC_TASKSIZE_LABEL_STATIC, 0.5, 0.0, 0.0, 0);
	AddResizableControl(IDC_TASKSIZE_PROGRESS, 0.5, 0.0, 0.5, 0);

	AddResizableControl(IDC_SHOW_LOG_BUTTON, 1.0, 0.0, 0.0, 0);
	AddResizableControl(IDC_STICK_BUTTON, 1.0, 1.0, 0, 0);

	InitializeResizableControls();
}

CString CStatusDlg::GetStatusString(const chengine::TTaskStatsSnapshotPtr& spTaskStats)
{
	CString strStatusText;
	// status string
	// first
	switch(spTaskStats->GetTaskState())
	{
	case chengine::eTaskState_Error:
		{
			strStatusText = GetResManager().LoadString(IDS_STATUS_ERROR_STRING);
			strStatusText += _T("/");
			break;
		}
	case chengine::eTaskState_LoadError:
		{
			strStatusText = GetResManager().LoadString(IDS_STATUS_LOADERROR_STRING);
			strStatusText += _T("/");
			break;
		}
	case chengine::eTaskState_Paused:
		{
			strStatusText = GetResManager().LoadString(IDS_STATUS_PAUSED_STRING);
			strStatusText += _T("/");
			break;
		}
	case chengine::eTaskState_Finished:
		{
			strStatusText = GetResManager().LoadString(IDS_STATUS_FINISHED_STRING);
			strStatusText += _T("/");
			break;
		}
	case chengine::eTaskState_Waiting:
		{
			strStatusText = GetResManager().LoadString(IDS_STATUS_WAITING_STRING);
			strStatusText += _T("/");
			break;
		}
	case chengine::eTaskState_Cancelled:
		{
			strStatusText = GetResManager().LoadString(IDS_STATUS_CANCELLED_STRING);
			strStatusText += _T("/");
			break;
		}
	case chengine::eTaskState_None:
		{
			strStatusText = GetResManager().LoadString(IDS_STATUS_INITIALIZING_STRING);
			strStatusText += _T("/");
			break;
		}
	case chengine::eTaskState_Processing:
		break;
	default:
		BOOST_ASSERT(false);		// not implemented state
	}

	// second part
	chengine::ESubOperationType eSubOperationType = chengine::eSubOperation_None;
	chengine::TSubTaskStatsSnapshotPtr spSubtaskStats = spTaskStats->GetSubTasksStats().GetCurrentSubTaskSnapshot();
	if(spSubtaskStats)
		eSubOperationType = spSubtaskStats->GetSubOperationType();

	if(eSubOperationType == chengine::eSubOperation_Deleting)
		strStatusText += GetResManager().LoadString(IDS_STATUS_DELETING_STRING);
	else if(eSubOperationType == chengine::eSubOperation_Scanning)
		strStatusText += GetResManager().LoadString(IDS_STATUS_SEARCHING_STRING);
	else if(eSubOperationType == chengine::eSubOperation_FastMove)
		strStatusText += GetResManager().LoadString(IDS_STATUS_FASTMOVE_STRING);
	else if(spTaskStats->GetOperationType() == chengine::eOperation_Copy)
		strStatusText += GetResManager().LoadString(IDS_STATUS_COPYING_STRING);
	else if(spTaskStats->GetOperationType() == chengine::eOperation_Move)
		strStatusText += GetResManager().LoadString(IDS_STATUS_MOVING_STRING);
	else
		strStatusText += GetResManager().LoadString(IDS_STATUS_UNKNOWN_STRING);

	if(!spTaskStats->GetFilters().IsEmpty())
		strStatusText += GetResManager().LoadString(IDS_FILTERING_STRING);

	// third part
	if(spTaskStats->GetIgnoreDirectories())
	{
		strStatusText += _T("/");
		strStatusText += GetResManager().LoadString(IDS_STATUS_ONLY_FILES_STRING);
	}
	if(spTaskStats->GetCreateEmptyFiles())
	{
		strStatusText += _T("/");
		strStatusText += GetResManager().LoadString(IDS_STATUS_WITHOUT_CONTENTS_STRING);
	}

	return strStatusText;
}

CString CStatusDlg::GetSubtaskName(chengine::ESubOperationType eSubtask) const
{
	if(eSubtask == chengine::eSubOperation_Deleting)
		return GetResManager().LoadString(IDS_STATUS_DELETING_STRING);
	if(eSubtask == chengine::eSubOperation_Scanning)
		return GetResManager().LoadString(IDS_STATUS_SEARCHING_STRING);
	if(eSubtask == chengine::eSubOperation_FastMove)
		return GetResManager().LoadString(IDS_STATUS_FASTMOVE_STRING);
	if(eSubtask == chengine::eSubOperation_Copying)
		return GetResManager().LoadString(IDS_STATUS_COPYING_STRING);

	return GetResManager().LoadString(IDS_STATUS_UNKNOWN_STRING);
}

void CStatusDlg::SetTaskListEntry(size_t stPos, const chengine::TTaskStatsSnapshotPtr& spTaskStats)
{
	// index subitem
	CString strStatusText = GetStatusString(spTaskStats);
	LVITEM lvi;
	lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
	lvi.iItem = boost::numeric_cast<int>(stPos);
	lvi.iSubItem = 0;
	lvi.pszText = (PTSTR)(PCTSTR)strStatusText;
	lvi.cchTextMax = lstrlen(lvi.pszText);
	lvi.lParam = boost::numeric_cast<LPARAM>(spTaskStats->GetTaskID());
	lvi.iImage = GetImageFromStatus(spTaskStats->GetTaskState());
	if(boost::numeric_cast<int>(stPos) < m_ctlStatusList.GetItemCount())
		m_ctlStatusList.SetItem(&lvi);
	else
		m_ctlStatusList.InsertItem(&lvi);

	string::TString strCurrentPath = spTaskStats->GetSourcePath();

	// input file
	lvi.mask=LVIF_TEXT;
	lvi.iSubItem = 1;
	if(strCurrentPath.IsEmpty())
		strCurrentPath = GetResManager().LoadString(IDS_NONEINPUTFILE_STRING);

	lvi.pszText = (PTSTR)strCurrentPath.c_str();
	lvi.cchTextMax = lstrlen(lvi.pszText);
	m_ctlStatusList.SetItem(&lvi);

	// destination path
	lvi.iSubItem = 2;
	string::TString strDestinationPath = spTaskStats->GetDestinationPath();
	lvi.pszText = (PTSTR)strDestinationPath.c_str();
	lvi.cchTextMax = lstrlen(lvi.pszText);
	m_ctlStatusList.SetItem(&lvi);

	// insert dest subitem
	lvi.iSubItem=3;

	CString strFmt;
	strFmt.Format(_T("%.1f %%"), spTaskStats->GetCombinedProgress() * 100.0);

	lvi.pszText = (PTSTR)(PCTSTR)strFmt;
	lvi.cchTextMax = lstrlen(lvi.pszText);
	m_ctlStatusList.SetItem(&lvi);
}

CString CStatusDlg::GetProcessedText(unsigned long long ullProcessedCount, unsigned long long ullTotalCount, unsigned long long ullProcessedSize, unsigned long long ullTotalSize)
{
	CString strProcessedText;
	strProcessedText.Format(_T("%I64u/%I64u ("), ullProcessedCount, ullTotalCount);

	strProcessedText += GetSizeString(ullProcessedSize) + _T("/");
	strProcessedText += GetSizeString(ullTotalSize) + _T(")");

	return strProcessedText;
}

CString CStatusDlg::GetSpeedString(double dSizeSpeed, double dAvgSizeSpeed, double dCountSpeed, double dAvgCountSpeed) const
{
	CString strSpeedText = GetSizeString(dSizeSpeed);	// last avg
	CString strAvgSpeedText = GetSizeString(dAvgSizeSpeed);	// last avg

	CString strAvgWord = GetResManager().LoadString(IDS_AVERAGEWORD_STRING);

	// avg transfer
	CString strFmt;
	strFmt.Format(_T("%s/s (%s%s/s); %.0f/s (%s%.0f/s)"), (PCTSTR)strSpeedText, (PCTSTR)strAvgWord, (PCTSTR)strAvgSpeedText,
		dCountSpeed, (PCTSTR)strAvgWord, dAvgCountSpeed);

	return strFmt;
}

void CStatusDlg::UpdateTaskStatsDetails(const chengine::TTaskStatsSnapshotPtr& spTaskStats)
{
	unsigned long long timeTotalEstimated = 0;
	unsigned long long timeElapsed = 0;
	unsigned long long timeRemaining = 0;

	chengine::TSubTaskStatsSnapshotPtr spSubTaskStats = spTaskStats->GetSubTasksStats().GetCurrentSubTaskSnapshot();
	if(spSubTaskStats)
	{
		// text progress
		CString strProcessedText = GetProcessedText(spSubTaskStats->GetProcessedCount(), spSubTaskStats->GetTotalCount(), spSubTaskStats->GetProcessedSize(), spSubTaskStats->GetTotalSize());
		GetDlgItem(IDC_SUBTASKPROCESSED_STATIC)->SetWindowText(strProcessedText);

		// progress bars
		m_ctlCurrentObjectProgress.SetProgress(spSubTaskStats->GetCurrentItemProcessedSize(), spSubTaskStats->GetCurrentItemTotalSize());
		m_ctlSubTaskCountProgress.SetProgress(spSubTaskStats->GetProcessedCount(), spSubTaskStats->GetTotalCount());
		m_ctlSubTaskSizeProgress.SetProgress(spSubTaskStats->GetProcessedSize(), spSubTaskStats->GetTotalSize());

		// time information
		timeTotalEstimated = spSubTaskStats->GetEstimatedTotalTime();
		timeElapsed = spSubTaskStats->GetTimeElapsed();
		timeRemaining = timeTotalEstimated - timeElapsed;

		CString strTime1 = FormatTimeMiliseconds(timeElapsed);
		CString strTime2 = FormatTimeMiliseconds(timeTotalEstimated);
		CString strTime3 = FormatTimeMiliseconds(timeRemaining);

		CString strTime;
		strTime.Format(_T("%s / %s (%s)"), (PCTSTR)strTime1, (PCTSTR)strTime2, (PCTSTR)strTime3);
		GetDlgItem(IDC_SUBTASKTIME_STATIC)->SetWindowText(strTime);

		// speed information
		CString strSpeed = GetSpeedString(spSubTaskStats->GetSizeSpeed(), spSubTaskStats->GetAvgSizeSpeed(), spSubTaskStats->GetCountSpeed(), spSubTaskStats->GetAvgCountSpeed());
		GetDlgItem(IDC_SUBTASKTRANSFER_STATIC)->SetWindowText(strSpeed);

		// subtask name
		chengine::ESubOperationType eSubOperationType = spSubTaskStats->GetSubOperationType();
		CString strSubtaskName = GetSubtaskName(eSubOperationType);
		GetDlgItem(IDC_SUBTASKNAME_STATIC)->SetWindowText(strSubtaskName);

		// current path
		string::TString strPath = spSubTaskStats->GetCurrentPath();
		if(strPath.IsEmpty())
			strPath = GetResManager().LoadString(IDS_NONEINPUTFILE_STRING);

		SetBufferSizesString(spTaskStats->GetCurrentBufferSize(), spSubTaskStats->GetCurrentBufferIndex());
	}
	else
	{
		m_ctlCurrentObjectProgress.SetProgress(0, 100);
		m_ctlSubTaskCountProgress.SetProgress(0, 100);
		m_ctlSubTaskSizeProgress.SetProgress(0, 100);

		GetDlgItem(IDC_SUBTASKNAME_STATIC)->SetWindowText(GetResManager().LoadString(IDS_EMPTYSUBTASKNAME_STRING));
		GetDlgItem(IDC_SUBTASKPROCESSED_STATIC)->SetWindowText(GetResManager().LoadString(IDS_EMPTYPROCESSEDTEXT_STRING));
		GetDlgItem(IDC_SUBTASKTIME_STATIC)->SetWindowText(GetResManager().LoadString(IDS_EMPTYTIMETEXT_STRING));
		GetDlgItem(IDC_SUBTASKTRANSFER_STATIC)->SetWindowText(GetResManager().LoadString(IDS_EMPTYTRANSFERTEXT_STRING));
		GetDlgItem(IDC_BUFFERSIZE_STATIC)->SetWindowText(GetResManager().LoadString(IDS_EMPTYBUFFERSIZETEXT_STRING));
	}

	//////////////////////////////////////////////////////
	// data that can be changed by a thread
	CString strStatusText = GetStatusString(spTaskStats);
	GetDlgItem(IDC_OPERATION_STATIC)->SetWindowText(strStatusText);	// operation

	CString strSrcPath = spTaskStats->GetSourcePath().c_str();
	if(strSrcPath.IsEmpty())
		strSrcPath = GetResManager().LoadString(IDS_EMPTYSOURCETEXT_STRING);

	GetDlgItem(IDC_SOURCEOBJECT_STATIC)->SetWindowText(spTaskStats->GetSourcePath().c_str());	// src object

	// count of processed data/overall count of data
	CString strProcessedText = GetProcessedText(spTaskStats->GetProcessedCount(), spTaskStats->GetTotalCount(),
		spTaskStats->GetProcessedSize(), spTaskStats->GetTotalSize());
	GetDlgItem(IDC_TASKPROCESSED_STATIC)->SetWindowText(strProcessedText);

	// transfer
	CString strTaskSpeed = GetSpeedString(spTaskStats->GetSizeSpeed(), spTaskStats->GetAvgSizeSpeed(), spTaskStats->GetCountSpeed(), spTaskStats->GetAvgCountSpeed());
	GetDlgItem(IDC_TASKTRANSFER_STATIC)->SetWindowText(strTaskSpeed);

	// elapsed time / estimated total time (estimated time left)
	timeTotalEstimated = spTaskStats->GetEstimatedTotalTime();
	timeElapsed = spTaskStats->GetTimeElapsed();
	timeRemaining = timeTotalEstimated - timeElapsed;

	CString strTime1 = FormatTimeMiliseconds(timeElapsed);
	CString strTime2 = FormatTimeMiliseconds(timeTotalEstimated);
	CString strTime3 = FormatTimeMiliseconds(timeRemaining);

	CString strTime;
	strTime.Format(_T("%s / %s (%s)"), (PCTSTR)strTime1, (PCTSTR)strTime2, (PCTSTR)strTime3);
	GetDlgItem(IDC_TASKTIME_STATIC)->SetWindowText(strTime);

	// set progress
	m_ctlTaskCountProgress.SetProgress(spTaskStats->GetProcessedCount(), spTaskStats->GetTotalCount());
	m_ctlTaskSizeProgress.SetProgress(spTaskStats->GetProcessedSize(), spTaskStats->GetTotalSize());

	GetDlgItem(IDC_DESTINATIONOBJECT_STATIC)->SetWindowText(spTaskStats->GetDestinationPath().c_str());
	GetDlgItem(IDC_THREADPRIORITY_STATIC)->SetWindowText(GetResManager().LoadString(IDS_PRIORITY0_STRING + PriorityToIndex(spTaskStats->GetThreadPriority())));
	GetDlgItem(IDC_TASKID_STATIC)->SetWindowText(spTaskStats->GetTaskName().c_str());
}

void CStatusDlg::SetWindowTitle(PCTSTR pszText)
{
	CString strCurrentTitle;
	GetWindowText(strCurrentTitle);
	if(strCurrentTitle != CString(pszText))
		SetWindowText(pszText);
}

CString CStatusDlg::GetProgressWindowTitleText() const
{
	CString strTitleText;

	if(m_spTaskMgrStats->GetTaskStatsCount() != 0)
		strTitleText.Format(_T("%s [%.1f %%]"), GetResManager().LoadString(IDS_STATUSTITLE_STRING), m_spTaskMgrStats->GetCombinedProgress() * 100.0);
	else
		strTitleText = GetResManager().LoadString(IDS_STATUSTITLE_STRING);

	return strTitleText;
}

void CStatusDlg::UpdateTaskBarProgress() const
{
	if(m_spTaskMgrStats->GetRunningTasks() != 0)
	{
		unsigned long long ullProgress = (unsigned long long)(m_spTaskMgrStats->GetCombinedProgress() * 100.0);

		m_taskBarProgress.SetState(m_hWnd, TBPF_NORMAL);
		m_taskBarProgress.SetPosition(m_hWnd, ullProgress, 100);
	}
	else
		m_taskBarProgress.SetState(m_hWnd, TBPF_NOPROGRESS);
}
