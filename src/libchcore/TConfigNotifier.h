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
#ifndef __TCONFIGNOTIFIER_H__
#define __TCONFIGNOTIFIER_H__

#include "libchcore.h"
#include "TStringSet.h"

namespace chcore
{
	// class defines configuration change notification record; not to be used outside
	class TConfigNotifier
	{
	public:
		TConfigNotifier(void(*pfnCallback)(const TStringSet&, void*), void* pParam);
		~TConfigNotifier();

		void operator()(const TStringSet& rsetPropNames);

		TConfigNotifier& operator=(const TConfigNotifier& rNotifier);

		bool operator==(const TConfigNotifier& rNotifier) const;

	private:
		void(*m_pfnCallback)(const TStringSet&, void*);
		void* m_pParam;
	};
}

#endif
