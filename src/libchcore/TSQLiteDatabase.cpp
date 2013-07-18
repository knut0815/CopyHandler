// ============================================================================
//  Copyright (C) 2001-2013 by Jozef Starosczyk
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
#include "stdafx.h"
#include "TSQLiteDatabase.h"
#include "sqlite3/sqlite3.h"
#include "ErrorCodes.h"
#include "TSQLiteException.h"

BEGIN_CHCORE_NAMESPACE

namespace sqlite
{
	TSQLiteDatabase::TSQLiteDatabase(PCTSTR pszFilename) :
		m_pDBHandle(NULL),
		m_bInTransaction(false)
	{
		int iResult = sqlite3_open16(pszFilename, &m_pDBHandle);
		if(iResult != SQLITE_OK)
		{
			const wchar_t* pszMsg = (const wchar_t*)sqlite3_errmsg16(m_pDBHandle);
			THROW_SQLITE_EXCEPTION(eErr_SQLiteCannotOpenDatabase, iResult, pszMsg);
		}
	}

	TSQLiteDatabase::~TSQLiteDatabase()
	{
		int iResult = sqlite3_close_v2(m_pDBHandle);	// handles properly the NULL DB Handle
		_ASSERTE(iResult == SQLITE_OK);
	}

	HANDLE TSQLiteDatabase::GetHandle()
	{
		return m_pDBHandle;
	}

	bool TSQLiteDatabase::GetInTransaction() const
	{
		return m_bInTransaction;
	}

	void TSQLiteDatabase::SetInTransaction(bool bInTransaction)
	{
		m_bInTransaction = bInTransaction;
	}
}

END_CHCORE_NAMESPACE
