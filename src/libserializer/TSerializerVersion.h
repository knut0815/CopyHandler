// ============================================================================
//  Copyright (C) 2001-2012 by Jozef Starosczyk
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
#ifndef __TSERIALIZERVERSION_H__
#define __TSERIALIZERVERSION_H__

#include "TSQLiteDatabase.h"
#include "libserializer.h"

namespace serializer
{
	class LIBSERIALIZER_API TSerializerVersion
	{
	public:
		explicit TSerializerVersion(const sqlite::TSQLiteDatabasePtr& spDatabase);
		~TSerializerVersion();

		int GetVersion();
		void SetVersion(int iNewVersion);

	protected:
		void Setup();

	private:
#pragma warning(push)
#pragma warning(disable: 4251)
		sqlite::TSQLiteDatabasePtr m_spDatabase;
#pragma warning(pop)
		bool m_bSetupExecuted;
	};
}

#endif
