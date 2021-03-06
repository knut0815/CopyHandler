// ============================================================================
//  Copyright (C) 2001-2014 by Jozef Starosczyk
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
#ifndef __TFILETIME_H__
#define __TFILETIME_H__

#include "libchcore.h"

namespace chcore
{
	class LIBCHCORE_API TFileTime
	{
	public:
		TFileTime();
		// cppcheck-suppress noExplicitConstructor
		TFileTime(const FILETIME& rftTime);
		~TFileTime();

		TFileTime& operator=(const FILETIME& rSrc);

		bool operator==(const TFileTime& rSrc) const;
		bool operator!=(const TFileTime& rSrc) const;

		void SetCurrentTime();
		const FILETIME& GetAsFiletime() const;

		void FromUInt64(unsigned long long ullTime);
		unsigned long long ToUInt64() const;

	private:
		FILETIME m_ftTime;
	};

	typedef std::shared_ptr<TFileTime> TFileTimePtr;
}

#endif
