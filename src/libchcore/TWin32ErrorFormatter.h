// ============================================================================
//  Copyright (C) 2001-2015 by Jozef Starosczyk
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
#ifndef __TWIN32ERRORFORMATTER_H__
#define __TWIN32ERRORFORMATTER_H__

#include "../libstring/TString.h"
#include "libchcore.h"

namespace chcore
{
	class LIBCHCORE_API TWin32ErrorFormatter
	{
	public:
		static string::TString FormatWin32ErrorCode(DWORD dwErrorCode, bool bUseNumberFallback);
		static string::TString FormatWin32ErrorCodeWithFallback(DWORD dwErrorCode, const wchar_t* pszModuleName, bool bUseNumberFallback);

	private:
		static string::TString FormatWin32ErrorCodeWithModule(DWORD dwErrorCode, HMODULE hModule, bool bUseNumberFallback);
	};
}

#endif
