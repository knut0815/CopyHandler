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
#ifndef __TEVENT_H__
#define __TEVENT_H__

#include "libchengine.h"

namespace chengine
{
	class LIBCHENGINE_API TEvent
	{
	public:
		TEvent(bool bManualReset, bool bInitialState);
		TEvent(const TEvent& rSrc) = delete;
		virtual ~TEvent();

		TEvent& operator=(const TEvent& rSrc) = delete;

		HANDLE Get() const { return m_hEvent; }

		void SetEvent(bool bSet);
		void SetEvent();
		void ResetEvent();

		HANDLE Handle() const { return m_hEvent; }

	private:
#ifdef _DEBUG
		bool m_bSignaled = false;
#endif
		HANDLE m_hEvent;
	};
}

#endif
