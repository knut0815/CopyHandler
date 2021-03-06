// ============================================================================
//  Copyright (C) 2001-2016 by Jozef Starosczyk
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
#ifndef __TEVENTCOUNTER_H__
#define __TEVENTCOUNTER_H__

#include "TEvent.h"

namespace chengine
{
	enum class EEventCounterMode
	{
		eSetIfEqual,
		eSetIfNotEqual
	};

	template<class T, EEventCounterMode EventMode, T CompareValue>
	class TEventCounter
	{
	public:
		explicit TEventCounter(T initialValue = 0) :
			m_event(true, false),
			m_spCounter(std::make_shared<TSharedCount<T>>(initialValue)),
			m_tMaxUsed(initialValue)
		{
			UpdateEvent();
		}

		TEventCounter(const TEventCounter& rSrc) = delete;
		TEventCounter& operator=(const TEventCounter& rSrc) = delete;

		void Increase()
		{
			m_spCounter->Increase();
			if(m_spCounter->GetValue() > m_tMaxUsed)
				m_tMaxUsed = m_spCounter->GetValue();
			UpdateEvent();
		}

		void Decrease()
		{
			m_spCounter->Decrease();
			UpdateEvent();
		}

		T GetMaxUsed() const
		{
			return m_tMaxUsed;
		}

		HANDLE GetEventHandle() const
		{
			return m_event.Handle();
		}

		TSharedCountPtr<T> GetSharedCount()
		{
			return m_spCounter;
		}

	private:
		void UpdateEvent()
		{
			bool bIsEqual = (m_spCounter->GetValue() == CompareValue);
			m_event.SetEvent(EventMode == EEventCounterMode::eSetIfEqual ? bIsEqual : !bIsEqual);
		}

	private:
		TEvent m_event;
		TSharedCountPtr<T> m_spCounter;
		T m_tMaxUsed;
	};
}

#endif
