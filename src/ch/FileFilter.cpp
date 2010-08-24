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
#include "FileInfo.h"
#include "FileFilter.h"

////////////////////////////////////////////////////////////////////////////
bool _tcicmp(TCHAR c1, TCHAR c2)
{
	TCHAR ch1[2]={c1, 0}, ch2[2]={c2, 0};
	return (_tcsicmp(ch1, ch2) == 0);
}

CFileFilter::CFileFilter()
{
	// files mask
	m_bUseMask=false;
	m_astrMask.clear();

	m_bUseExcludeMask=false;
	m_astrExcludeMask.clear();

	// size filtering
	m_bUseSize=false;
	m_iSizeType1=GT;
	m_ullSize1=0;
	m_bUseSize2=false;
	m_iSizeType2=LT;
	m_ullSize2=0;

	// date filtering
	m_bUseDate=false;
	m_iDateType=DATE_CREATED;
	m_iDateType1=GT;
	m_bDate1=false;
	m_tDate1=CTime::GetCurrentTime();
	m_bTime1=false;
	m_tTime1=CTime::GetCurrentTime();

	m_bUseDate2=false;
	m_iDateType2=LT;
	m_bDate2=false;
	m_tDate2=CTime::GetCurrentTime();
	m_bTime2=false;
	m_tTime2=CTime::GetCurrentTime();

	// attribute filtering
	m_bUseAttributes=false;
	m_iArchive=2;
	m_iReadOnly=2;
	m_iHidden=2;
	m_iSystem=2;
	m_iDirectory=2;
}

CFileFilter::CFileFilter(const CFileFilter& rFilter)
{
	*this=rFilter;
}

CFileFilter& CFileFilter::operator=(const CFileFilter& rFilter)
{
	// files mask
	m_bUseMask=rFilter.m_bUseMask;
	m_astrMask = rFilter.m_astrMask;

	m_bUseExcludeMask=rFilter.m_bUseExcludeMask;
	m_astrExcludeMask = rFilter.m_astrExcludeMask;

	// size filtering
	m_bUseSize=rFilter.m_bUseSize;
	m_iSizeType1=rFilter.m_iSizeType1;
	m_ullSize1=rFilter.m_ullSize1;
	m_bUseSize2=rFilter.m_bUseSize2;
	m_iSizeType2=rFilter.m_iSizeType2;
	m_ullSize2=rFilter.m_ullSize2;

	// date filtering
	m_bUseDate=rFilter.m_bUseDate;
	m_iDateType=rFilter.m_iDateType;
	m_iDateType1=rFilter.m_iDateType1;
	m_bDate1=rFilter.m_bDate1;
	m_tDate1=rFilter.m_tDate1;
	m_bTime1=rFilter.m_bTime1;
	m_tTime1=rFilter.m_tTime1;

	m_bUseDate2=rFilter.m_bUseDate2;
	m_iDateType2=rFilter.m_iDateType2;
	m_bDate2=rFilter.m_bDate2;
	m_tDate2=rFilter.m_tDate2;
	m_bTime2=rFilter.m_bTime2;
	m_tTime2=rFilter.m_tTime2;

	// attribute filtering
	m_bUseAttributes=rFilter.m_bUseAttributes;
	m_iArchive=rFilter.m_iArchive;
	m_iReadOnly=rFilter.m_iReadOnly;
	m_iHidden=rFilter.m_iHidden;
	m_iSystem=rFilter.m_iSystem;
	m_iDirectory=rFilter.m_iDirectory;

	return *this;
}

CString& CFileFilter::GetCombinedMask(CString& strMask) const
{
	strMask.Empty();
	if(m_astrMask.size() > 0)
	{
		strMask = m_astrMask.at(0);
		for(size_t stIndex = 1; stIndex < m_astrMask.size(); stIndex++)
			strMask += _T("|") + m_astrMask.at(stIndex);
	}

	return strMask;
}

void CFileFilter::SetCombinedMask(const CString& pMask)
{
	m_astrMask.clear();

	TCHAR *pszData=new TCHAR[pMask.GetLength()+1];
	_tcscpy(pszData, pMask);

	TCHAR *szToken=_tcstok(pszData, _T("|"));
	while (szToken != NULL)
	{
		// add token to a table
		m_astrMask.push_back(szToken);

		// search for next
		szToken=_tcstok(NULL, _T("|"));
	}

	delete [] pszData;
}

CString& CFileFilter::GetCombinedExcludeMask(CString& strMask) const
{
	strMask.Empty();
	if(m_astrExcludeMask.size() > 0)
	{
		strMask = m_astrExcludeMask.at(0);
		for(size_t stIndex = 1; stIndex < m_astrExcludeMask.size(); stIndex++)
			strMask += _T("|") + m_astrExcludeMask.at(stIndex);
	}

	return strMask;
}

void CFileFilter::SetCombinedExcludeMask(const CString& pMask)
{
	m_astrExcludeMask.clear();

	TCHAR *pszData=new TCHAR[pMask.GetLength()+1];
	_tcscpy(pszData, pMask);

	TCHAR *szToken=_tcstok(pszData, _T("|"));
	while (szToken != NULL)
	{
		// add token
		m_astrExcludeMask.push_back(szToken);

		// find next
		szToken=_tcstok(NULL, _T("|"));
	}

	delete [] pszData;
}

bool CFileFilter::Match(const CFileInfo& rInfo) const
{
	// check by mask
	if(m_bUseMask)
	{
		bool bRes=false;
		for(std::vector<CString>::const_iterator iterMask = m_astrMask.begin(); iterMask != m_astrMask.end(); ++iterMask)
		{
			if(MatchMask(*iterMask, rInfo.GetFileName()))
				bRes = true;
		}
		if(!bRes)
			return false;
	}

	// excluding mask
	if(m_bUseExcludeMask)
	{
		for(std::vector<CString>::const_iterator iterExcludeMask = m_astrExcludeMask.begin(); iterExcludeMask != m_astrExcludeMask.end(); ++iterExcludeMask)
		{
			if(MatchMask(*iterExcludeMask, rInfo.GetFileName()))
				return false;
		}
	}

	// by size
	if (m_bUseSize)
	{
		switch (m_iSizeType1)
		{
		case LT:
			if (m_ullSize1 <= rInfo.GetLength64())
				return false;
			break;
		case LE:
			if (m_ullSize1 < rInfo.GetLength64())
				return false;
			break;
		case EQ:
			if (m_ullSize1 != rInfo.GetLength64())
				return false;
			break;
		case GE:
			if (m_ullSize1 > rInfo.GetLength64())
				return false;
			break;
		case GT:
			if (m_ullSize1 >= rInfo.GetLength64())
				return false;
			break;
		}

		// second part
		if (m_bUseSize2)
		{
			switch (m_iSizeType2)
			{
			case LT:
				if (m_ullSize2 <= rInfo.GetLength64())
					return false;
				break;
			case LE:
				if (m_ullSize2 < rInfo.GetLength64())
					return false;
				break;
			case EQ:
				if (m_ullSize2 != rInfo.GetLength64())
					return false;
				break;
			case GE:
				if (m_ullSize2 > rInfo.GetLength64())
					return false;
				break;
			case GT:
				if (m_ullSize2 >= rInfo.GetLength64())
					return false;
				break;
			}
		}
	}

	// date - get the time from rInfo
	if (m_bUseDate)
	{
		COleDateTime tm;
		switch (m_iDateType)
		{
		case DATE_CREATED:
			tm=rInfo.GetCreationTime();
			break;
		case DATE_MODIFIED:
			tm=rInfo.GetLastWriteTime();
			break;
		case DATE_LASTACCESSED:
			tm=rInfo.GetLastAccessTime();
			break;
		}

		// counting...
		unsigned long ulInfo=0, ulCheck=0;
		if (m_bDate1)
		{
			ulInfo=(tm.GetYear()-1970)*32140800+tm.GetMonth()*2678400+tm.GetDay()*86400;
			ulCheck=(m_tDate1.GetYear()-1970)*32140800+m_tDate1.GetMonth()*2678400+m_tDate1.GetDay()*86400;
		}

		if (m_bTime1)
		{
			ulInfo+=tm.GetHour()*3600+tm.GetMinute()*60+tm.GetSecond();
			ulCheck+=m_tTime1.GetHour()*3600+m_tTime1.GetMinute()*60+m_tTime1.GetSecond();
		}

		// ... and comparing
		switch (m_iDateType1)
		{
		case LT:
			if (ulInfo >= ulCheck)
				return false;
			break;
		case LE:
			if (ulInfo > ulCheck)
				return false;
			break;
		case EQ:
			if (ulInfo != ulCheck)
				return false;
			break;
		case GE:
			if (ulInfo < ulCheck)
				return false;
			break;
		case GT:
			if (ulInfo <= ulCheck)
				return false;
			break;
		}

		if (m_bUseDate2)
		{
			// counting...
			ulInfo=0, ulCheck=0;
			if (m_bDate2)
			{
				ulInfo=(tm.GetYear()-1970)*32140800+tm.GetMonth()*2678400+tm.GetDay()*86400;
				ulCheck=(m_tDate2.GetYear()-1970)*32140800+m_tDate2.GetMonth()*2678400+m_tDate2.GetDay()*86400;
			}

			if (m_bTime2)
			{
				ulInfo+=tm.GetHour()*3600+tm.GetMinute()*60+tm.GetSecond();
				ulCheck+=m_tTime2.GetHour()*3600+m_tTime2.GetMinute()*60+m_tTime2.GetSecond();
			}

			// ... comparing
			switch (m_iDateType2)
			{
			case LT:
				if (ulInfo >= ulCheck)
					return false;
				break;
			case LE:
				if (ulInfo > ulCheck)
					return false;
				break;
			case EQ:
				if (ulInfo != ulCheck)
					return false;
				break;
			case GE:
				if (ulInfo < ulCheck)
					return false;
				break;
			case GT:
				if (ulInfo <= ulCheck)
					return false;
				break;
			}
		}
	} // of m_bUseDate

	// attributes
	if (m_bUseAttributes)
	{
		if ( (m_iArchive == 1 && !rInfo.IsArchived()) || (m_iArchive == 0 && rInfo.IsArchived()))
			return false;
		if ( (m_iReadOnly == 1 && !rInfo.IsReadOnly()) || (m_iReadOnly == 0 && rInfo.IsReadOnly()))
			return false;
		if ( (m_iHidden == 1 && !rInfo.IsHidden()) || (m_iHidden == 0 && rInfo.IsHidden()))
			return false;
		if ( (m_iSystem == 1 && !rInfo.IsSystem()) || (m_iSystem == 0 && rInfo.IsSystem()))
			return false;
		if ( (m_iDirectory == 1 && !rInfo.IsDirectory()) || (m_iDirectory == 0 && rInfo.IsDirectory()))
			return false;
	}

	return true;
}

bool CFileFilter::MatchMask(LPCTSTR lpszMask, LPCTSTR lpszString) const
{
	bool bMatch = 1;

	//iterate and delete '?' and '*' one by one
	while(*lpszMask != _T('\0') && bMatch && *lpszString != _T('\0'))
	{
		if (*lpszMask == _T('?')) lpszString++;
		else if (*lpszMask == _T('*'))
		{
			bMatch = Scan(lpszMask, lpszString);
			lpszMask--;
		}
		else
		{
			bMatch = _tcicmp(*lpszMask, *lpszString);
			lpszString++;
		}
		lpszMask++;
	}
	while (*lpszMask == _T('*') && bMatch) lpszMask++;

	return bMatch && *lpszString == _T('\0') && *lpszMask == _T('\0');
}

// scan '?' and '*'
bool CFileFilter::Scan(LPCTSTR& lpszMask, LPCTSTR& lpszString) const
{
	// remove the '?' and '*'
	for(lpszMask++; *lpszString != _T('\0') && (*lpszMask == _T('?') || *lpszMask == _T('*')); lpszMask++)
		if (*lpszMask == _T('?')) lpszString++;
	while ( *lpszMask == _T('*')) lpszMask++;

	// if lpszString is empty and lpszMask has more characters or,
	// lpszMask is empty, return 
	if (*lpszString == _T('\0') && *lpszMask != _T('\0')) return false;
	if (*lpszString == _T('\0') && *lpszMask == _T('\0')) return true; 
	// else search substring
	else
	{
		LPCTSTR wdsCopy = lpszMask;
		LPCTSTR lpszStringCopy = lpszString;
		bool bMatch = true;
		do 
		{
			if (!MatchMask(lpszMask, lpszString)) lpszStringCopy++;
			lpszMask = wdsCopy;
			lpszString = lpszStringCopy;
			while (!(_tcicmp(*lpszMask, *lpszString)) && (*lpszString != '\0')) lpszString++;
			wdsCopy = lpszMask;
			lpszStringCopy = lpszString;
		}
		while ((*lpszString != _T('\0')) ? !MatchMask(lpszMask, lpszString) : (bMatch = false) != false);

		if (*lpszString == _T('\0') && *lpszMask == _T('\0')) return true;

		return bMatch;
	}
}

CFiltersArray& CFiltersArray::operator=(const CFiltersArray& rSrc)
{
	if(this != &rSrc)
	{
		m_vFilters = rSrc.m_vFilters;
	}

	return *this;
}

bool CFiltersArray::Match(const CFileInfo& rInfo) const
{
	if(m_vFilters.empty())
		return true;

	// if only one of the filters matches - return true
	for(std::vector<CFileFilter>::const_iterator iterFilter = m_vFilters.begin(); iterFilter != m_vFilters.end(); iterFilter++)
	{
		if((*iterFilter).Match(rInfo))
			return true;
	}

	return false;
}

bool CFiltersArray::IsEmpty() const
{
	return m_vFilters.empty();
}

void CFiltersArray::Add(const CFileFilter& rFilter)
{
	m_vFilters.push_back(rFilter);
}

bool CFiltersArray::SetAt(size_t stIndex, const CFileFilter& rNewFilter)
{
	BOOST_ASSERT(stIndex < m_vFilters.size());
	if(stIndex < m_vFilters.size())
	{
		CFileFilter& rFilter = m_vFilters.at(stIndex);
		rFilter = rNewFilter;
		return true;
	}
	else
		return false;
}

const CFileFilter* CFiltersArray::GetAt(size_t stIndex) const
{
	BOOST_ASSERT(stIndex < m_vFilters.size());
	if(stIndex < m_vFilters.size())
		return &m_vFilters.at(stIndex);
	else
		return NULL;
}

bool CFiltersArray::RemoveAt(size_t stIndex)
{
	BOOST_ASSERT(stIndex < m_vFilters.size());
	if(stIndex < m_vFilters.size())
	{
		m_vFilters.erase(m_vFilters.begin() + stIndex);
		return true;
	}
	else
		return false;
}

size_t CFiltersArray::GetSize() const
{
	return m_vFilters.size();
}
