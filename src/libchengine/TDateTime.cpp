// ============================================================================
//  Copyright (C) 2001-2011 by Jozef Starosczyk
//  ixen@copyhandler.com
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU Library General Public License
//  (version 2) as published by the Free Software Foundation;
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU Library General Public
//  License along with this program; if not, write to the
//  Free Software Foundation, Inc.,
//  59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// ============================================================================
/// @file  TDateTime.cpp
/// @date  2011/10/26
/// @brief Contains implementation of date&time handling class.
// ============================================================================
#include "stdafx.h"
#include "TDateTime.h"
#include "../libchcore/TCoreException.h"
#include "../libchcore/TCoreWin32Exception.h"

using namespace chcore;
using namespace string;

namespace chengine
{
	TDateTime::TDateTime() :
		m_tTime(0)
	{
	}

	TDateTime::TDateTime(int iYear, int iMonth, int iDay, int iHour, int iMinute, int iSecond)
	{
		if (iYear < 1900)
			throw TCoreException(eErr_InvalidArgument, L"iYear", LOCATION);

		tm tTime;

		tTime.tm_sec = iSecond;
		tTime.tm_min = iMinute;
		tTime.tm_hour = iHour;
		tTime.tm_mday = iDay;
		tTime.tm_mon = iMonth - 1;
		tTime.tm_year = iYear - 1900;
		tTime.tm_isdst = -1;

		m_tTime = _mktime64(&tTime);
		if (m_tTime == -1)
			throw TCoreWin32Exception(eErr_InvalidArgument, GetLastError(), L"Cannot make time", LOCATION);
	}

	TDateTime::TDateTime(FILETIME ftDateTime)
	{
		operator=(ftDateTime);
	}

	TDateTime::TDateTime(SYSTEMTIME sysDateTime)
	{
		operator=(sysDateTime);
	}

	TDateTime::TDateTime(time_t tDateTime) :
		m_tTime(tDateTime)
	{
	}

	TDateTime& TDateTime::operator=(FILETIME ftDateTime)
	{
		// convert and process as system time
		FILETIME tLocalFileTime;
		if (!FileTimeToLocalFileTime(&ftDateTime, &tLocalFileTime))
			throw TCoreWin32Exception(eErr_InvalidArgument, GetLastError(), L"Cannot convert file time to local file time", LOCATION);

		SYSTEMTIME sysTime;
		if (!FileTimeToSystemTime(&tLocalFileTime, &sysTime))
			throw TCoreWin32Exception(eErr_InvalidArgument, GetLastError(), L"Cannot convert file time to system time", LOCATION);

		return operator=(sysTime);
	}

	TDateTime& TDateTime::operator=(SYSTEMTIME sysDateTime)
	{
		if (sysDateTime.wYear < 1900)
			throw TCoreException(eErr_InvalidArgument, L"sysDateTime.wYear", LOCATION);

		tm tTime;

		tTime.tm_sec = sysDateTime.wSecond;
		tTime.tm_min = sysDateTime.wMinute;
		tTime.tm_hour = sysDateTime.wHour;
		tTime.tm_mday = sysDateTime.wDay;
		tTime.tm_mon = sysDateTime.wMonth - 1;
		tTime.tm_year = sysDateTime.wYear - 1900;
		tTime.tm_isdst = -1;

		m_tTime = _mktime64(&tTime);
		if (m_tTime == -1)
			throw TCoreWin32Exception(eErr_InvalidArgument, GetLastError(), L"Cannot make time", LOCATION);

		return *this;
	}

	TDateTime& TDateTime::operator=(time_t tDateTime)
	{
		m_tTime = tDateTime;
		return *this;
	}

	void TDateTime::Clear()
	{
		m_tTime = 0;
	}

	void TDateTime::SetCurrentDateTime()
	{
		m_tTime = _time64(nullptr);
	}

	void TDateTime::GetAsSystemTime(SYSTEMTIME& tSystemTime) const
	{
		tm tThisTimeInfo;
		errno_t err = _localtime64_s(&tThisTimeInfo, &m_tTime);
		if (err != 0)
			throw TCoreException(eErr_InvalidData, L"Reading localtime failed", LOCATION);

		tSystemTime.wYear = (WORD)(tThisTimeInfo.tm_year + 1900);
		tSystemTime.wMonth = (WORD)(tThisTimeInfo.tm_mon + 1);
		tSystemTime.wDayOfWeek = (WORD)tThisTimeInfo.tm_wday;
		tSystemTime.wDay = (WORD)tThisTimeInfo.tm_mday;
		tSystemTime.wHour = (WORD)tThisTimeInfo.tm_hour;
		tSystemTime.wMinute = (WORD)tThisTimeInfo.tm_min;
		tSystemTime.wSecond = (WORD)tThisTimeInfo.tm_sec;
		tSystemTime.wMilliseconds = 0;
	}

	TString TDateTime::Format(bool bUseDate, bool bUseTime) const
	{
		if (!bUseDate && !bUseTime)
			return TString();

		TString strTmp;
		const size_t stMaxBufSize = 1024;
		wchar_t* pszBuffer = strTmp.GetBuffer(stMaxBufSize);

		PCTSTR pszFmt = nullptr;
		if (bUseDate && bUseTime)
			pszFmt = _T("%x %X");
		else if (bUseDate)
			pszFmt = _T("%x");
		else if (bUseTime)
			pszFmt = _T("%X");

		tm tThisTimeInfo;
		errno_t err = _localtime64_s(&tThisTimeInfo, &m_tTime);
		if (err != 0)
			throw TCoreException(eErr_InvalidData, L"Reading local time failed", LOCATION);

		if (!_tcsftime(pszBuffer, stMaxBufSize, pszFmt, &tThisTimeInfo))
			throw TCoreException(eErr_InvalidData, L"Time formatting failed", LOCATION);

		strTmp.ReleaseBuffer();
		return strTmp;
	}

	time_t TDateTime::Compare(const TDateTime& rOtherDateTime, bool bCompareDate, bool bCompareTime) const
	{
		if (!bCompareDate && !bCompareTime)
			return 0;

		tm tThisTimeInfo;
		tm tOtherTimeInfo;
		errno_t err = _localtime64_s(&tThisTimeInfo, &m_tTime);
		if (err != 0)
			throw TCoreException(eErr_InvalidData, L"Reading local time failed", LOCATION);
		err = _localtime64_s(&tOtherTimeInfo, &rOtherDateTime.m_tTime);
		if (err != 0)
			throw TCoreException(eErr_InvalidData, L"Reading local time failed", LOCATION);

		time_t tDiffDateTime = 0;
		if (bCompareDate)
		{
			time_t tThisCompoundDate = (tThisTimeInfo.tm_year - 1900) * 32140800 + tThisTimeInfo.tm_mon * 2678400 + tThisTimeInfo.tm_mday * 86400;
			time_t tOtherCompoundDate = (tOtherTimeInfo.tm_year - 1900) * 32140800 + tOtherTimeInfo.tm_mon * 2678400 + tOtherTimeInfo.tm_mday * 86400;

			// <0 means that this date is less than other date, 0 means they are equal, >0 means that other date is less than this date
			tDiffDateTime = tOtherCompoundDate - tThisCompoundDate;

			// at this point we can return only if this date differs from other date; if they are equal, process time comparison if needed
			if (tDiffDateTime != 0)
				return tDiffDateTime;
		}

		if (bCompareTime)
		{
			time_t tThisCompoundTime = tThisTimeInfo.tm_hour * 3600 + tThisTimeInfo.tm_min * 60 + tThisTimeInfo.tm_sec;
			time_t tOtherCompoundTime = tOtherTimeInfo.tm_hour * 3600 + tOtherTimeInfo.tm_min * 60 + tOtherTimeInfo.tm_sec;

			tDiffDateTime = tOtherCompoundTime - tThisCompoundTime;
		}

		return tDiffDateTime;
	}

	void TDateTime::StoreInConfig(TConfig& rConfig, PCTSTR pszNodeName) const
	{
		rConfig.SetValue(pszNodeName, m_tTime);
	}

	bool TDateTime::ReadFromConfig(const TConfig& rConfig, PCTSTR pszNodeName)
	{
		return rConfig.GetValue(pszNodeName, m_tTime);
	}

	time_t TDateTime::GetAsTimeT() const
	{
		return m_tTime;
	}

	bool TDateTime::operator==(const TDateTime& rSrc) const
	{
		return m_tTime == rSrc.m_tTime;
	}

	bool TDateTime::operator!=(const TDateTime& rSrc) const
	{
		return m_tTime != rSrc.m_tTime;
	}
}
