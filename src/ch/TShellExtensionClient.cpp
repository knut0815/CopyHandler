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
#include "stdafx.h"
#include "TShellExtensionClient.h"
#include "objbase.h"
#include "../chext/Logger.h"
#include "../chext/guids.h"
#include "ch.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

TShellExtensionClient::TShellExtensionClient() :
	m_bInitialized(false),
	m_piShellExtControl(nullptr)
{
}

TShellExtensionClient::~TShellExtensionClient()
{
	FreeControlInterface();
	UninitializeCOM();
}

HRESULT TShellExtensionClient::InitializeCOM()
{
	if(m_bInitialized)
		return S_FALSE;

	HRESULT hResult = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	if(SUCCEEDED(hResult))
		m_bInitialized = true;
	else if(hResult == RPC_E_CHANGED_MODE)
		return S_FALSE;

	return hResult;
}

void TShellExtensionClient::UninitializeCOM()
{
	if(m_bInitialized)
	{
		CoUninitialize();
		m_bInitialized = false;
	}
}

bool TShellExtensionClient::DetectRegExe()
{
	const DWORD dwSize = 32768;
	wchar_t szData[dwSize];
	DWORD dwResult = ::GetModuleFileName(nullptr, szData, dwSize);
	if (dwResult == 0)
		return false;
	szData[dwResult] = L'\0';

	std::wstring wstrDir = szData;

	size_t stPos = wstrDir.find_last_of(L'\\');
	if (stPos != std::wstring::npos)
		wstrDir.erase(wstrDir.begin() + stPos + 1, wstrDir.end());

#ifdef _WIN64
	wstrDir += _T("regchext64.exe");
#else
	wstrDir += _T("regchext.exe");
#endif

	m_strRegExe = wstrDir;

	return true;
}

logger::TLoggerPtr& TShellExtensionClient::GetLogger()
{
	if(!m_spLog)
		m_spLog = logger::MakeLogger(GetLogFileData(), L"ShellExtClient");

	return m_spLog;
}

ERegistrationResult TShellExtensionClient::RegisterShellExtDll(long lClientVersion, long& rlExtensionVersion, CString& rstrExtensionStringVersion)
{
	LOG_INFO(GetLogger()) << L"Registering shell extension";

	HRESULT hResult = InitializeCOM();
	if(FAILED(hResult))
	{
		LOG_ERROR(GetLogger()) << L"Failed to initialize COM. Error: " << hResult;
		return eFailure;
	}

	// get rid of the interface, so we can at least try to re-register
	LOG_DEBUG(GetLogger()) << L"Freeing control interface";
	FreeControlInterface();

	LOG_DEBUG(GetLogger()) << L"Detecting regchext binary";
	if(!DetectRegExe())
	{
		LOG_ERROR(GetLogger()) << L"Failed to detect regchext binary";
		return eFailure;
	}

	LOG_DEBUG(GetLogger()) << L"Executing regchext binary";
	// if previous operation failed (ie. vista system) - try running regsvr32 with elevated privileges
	// try with regsvr32
	SHELLEXECUTEINFO sei;
	memset(&sei, 0, sizeof(sei));
	sei.cbSize = sizeof(sei);
	sei.fMask = SEE_MASK_UNICODE | SEE_MASK_NOCLOSEPROCESS;
	sei.lpVerb = _T("runas");
	sei.lpFile = m_strRegExe.c_str();
	sei.lpParameters = _T("");
	sei.nShow = SW_SHOW;

	if(!ShellExecuteEx(&sei))
	{
		DWORD dwLastError = GetLastError();
		LOG_ERROR(GetLogger()) << L"Failed to execute regchext binary. Error: " << dwLastError;
		return eFailure;
	}

	LOG_DEBUG(GetLogger()) << L"Waiting for registration process to finish";
	if(SUCCEEDED(hResult) && WaitForSingleObject(sei.hProcess, 10000) != WAIT_OBJECT_0)
	{
		DWORD dwLastError = GetLastError();
		LOG_ERROR(GetLogger()) << L"Waiting failed. Last error: " << dwLastError;
		CloseHandle(sei.hProcess);

		return eFailure;
	}

	DWORD dwExitCode = 0;
	if (!GetExitCodeProcess(sei.hProcess, &dwExitCode))
	{
		DWORD dwLastError = GetLastError();
		LOG_ERROR(GetLogger()) << L"Failed to retrieve process exit code. Last error: " << dwLastError;
		CloseHandle(sei.hProcess);

		return eFailure;
	}
	CloseHandle(sei.hProcess);

	LOG_INFO(GetLogger()) << L"Registration result: " << dwExitCode;

	ERegistrationResult eResult = (ERegistrationResult)dwExitCode;

	if(eResult == eSuccess || eResult == eSuccessNative)
	{
		// NOTE: we are re-trying to enable the shell extension through our notification interface
		// in case of class-not-registered error because (it seems) system needs some time to process
		// DLL's self registration and usually the first call fails.
		int iTries = 3;
		do
		{
			LOG_DEBUG(GetLogger()) << L"Trying to enable native shell extension";
			hResult = EnableExtensionIfCompatible(lClientVersion, rlExtensionVersion, rstrExtensionStringVersion);
			if(hResult == REGDB_E_CLASSNOTREG)
			{
				LOG_ERROR(GetLogger()) << L"Class CLSID_CShellExtControl still not registered";
				Sleep(500);
			}
		}
		while(--iTries && hResult == REGDB_E_CLASSNOTREG);

		if(FAILED(hResult))
		{
			LOG_INFO(GetLogger()) << L"Shell Extension requires system restart";
			eResult = eSuccessNeedRestart;
		}
	}

	return eResult;
}

ERegistrationResult TShellExtensionClient::UnRegisterShellExtDll()
{
	LOG_INFO(GetLogger()) << L"Unregistering shell extension";

	HRESULT hResult = InitializeCOM();
	if(FAILED(hResult))
	{
		LOG_ERROR(GetLogger()) << L"Failed to initialize COM. Error: " << hResult;
		return eFailure;
	}

	// get rid of the interface, so we can at least try to re-register
	LOG_DEBUG(GetLogger()) << L"Freeing control interface";
	FreeControlInterface();

	LOG_DEBUG(GetLogger()) << L"Detecting regchext binary";
	if(!DetectRegExe())
	{
		LOG_ERROR(GetLogger()) << L"Failed to detect regchext binary";
		return eFailure;
	}

	LOG_DEBUG(GetLogger()) << L"Executing regchext binary";
	// if previous operation failed (ie. vista system) - try running regsvr32 with elevated privileges
	// try with regsvr32
	SHELLEXECUTEINFO sei;
	memset(&sei, 0, sizeof(sei));
	sei.cbSize = sizeof(sei);
	sei.fMask = SEE_MASK_UNICODE | SEE_MASK_NOCLOSEPROCESS;
	sei.lpVerb = _T("runas");
	sei.lpFile = m_strRegExe.c_str();
	sei.lpParameters = _T("/u");
	sei.nShow = SW_SHOW;

	if(!ShellExecuteEx(&sei))
	{
		DWORD dwLastError = GetLastError();
		LOG_ERROR(GetLogger()) << L"Failed to execute regchext binary. Error: " << dwLastError;
		return eFailure;
	}

	LOG_DEBUG(GetLogger()) << L"Waiting for deregistration process to finish";
	if(SUCCEEDED(hResult) && WaitForSingleObject(sei.hProcess, 10000) != WAIT_OBJECT_0)
	{
		DWORD dwLastError = GetLastError();
		LOG_ERROR(GetLogger()) << L"Waiting failed. Last error: " << dwLastError;
		CloseHandle(sei.hProcess);

		return eFailure;
	}

	DWORD dwExitCode = 0;
	if(!GetExitCodeProcess(sei.hProcess, &dwExitCode))
	{
		DWORD dwLastError = GetLastError();
		LOG_ERROR(GetLogger()) << L"Failed to retrieve process exit code. Last error: " << dwLastError;
		CloseHandle(sei.hProcess);

		return eFailure;
	}
	CloseHandle(sei.hProcess);

	LOG_INFO(GetLogger()) << L"Deregistration result: " << dwExitCode;

	return (ERegistrationResult)dwExitCode;
}

HRESULT TShellExtensionClient::EnableExtensionIfCompatible(long lClientVersion, long& rlExtensionVersion, CString& rstrExtensionStringVersion)
{
	rlExtensionVersion = 0;
	rstrExtensionStringVersion.Empty();

	BSTR bstrVersion = nullptr;

	HRESULT hResult = RetrieveControlInterface();
	if(SUCCEEDED(hResult) && !m_piShellExtControl)
		hResult = E_FAIL;
	if(SUCCEEDED(hResult))
		hResult = m_piShellExtControl->GetVersion(&rlExtensionVersion, &bstrVersion);
	if(SUCCEEDED(hResult))
	{
		// enable or disable extension - currently we only support extension from strictly the same version as CH
		bool bVersionMatches = (lClientVersion == rlExtensionVersion);
		hResult = m_piShellExtControl->SetFlags(bVersionMatches ? eShellExt_Enabled : 0, eShellExt_Enabled);
		if(SUCCEEDED(hResult))
			hResult = bVersionMatches ? S_OK : S_FALSE;
	}

	// do not overwrite S_OK/S_FALSE status after this line - it needs to be propagated upwards
	if(bstrVersion)
	{
		rstrExtensionStringVersion = bstrVersion;
		::SysFreeString(bstrVersion);
	}

	return hResult;
}

void TShellExtensionClient::Close()
{
	FreeControlInterface();
}

HRESULT TShellExtensionClient::RetrieveControlInterface()
{
	HRESULT hResult = InitializeCOM();
	if(SUCCEEDED(hResult))
		hResult = CoCreateInstance(CLSID_CShellExtControl, nullptr, CLSCTX_ALL, IID_IShellExtControl, (void**)&m_piShellExtControl);
	if(SUCCEEDED(hResult) && !m_piShellExtControl)
		hResult = E_FAIL;

	return hResult;
}

void TShellExtensionClient::FreeControlInterface()
{
	if(m_piShellExtControl)
	{
		m_piShellExtControl->Release();
		m_piShellExtControl = nullptr;
	}
}
