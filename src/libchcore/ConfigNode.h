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
#ifndef __TCONFIGNODE_H__
#define __TCONFIGNODE_H__

#include "TString.h"
#include <boost/variant.hpp>
#include <bitset>
#include "TSharedModificationTracker.h"

BEGIN_CHCORE_NAMESPACE

namespace details
{
	class ConfigNode
	{
	public:
		ConfigNode(size_t stObjectID, const TString& strNodeName, int iOrder, const TString& strValue);

		TString GetNodeName() const;
		int GetOrder() const;

	public:
		enum EModifications
		{
			eMod_None = 0,
			eMod_Added = 1,
			eMod_NodeName,
			eMod_Value,
			eMod_Order,

			eMod_Last
		};

		typedef std::bitset<eMod_Last> Bitset;
		mutable Bitset m_setModifications;

		size_t m_stObjectID;
		TSharedModificationTracker<int, Bitset, eMod_Order> m_iOrder;
		TSharedModificationTracker<TString, Bitset, eMod_NodeName> m_strNodeName;
		TSharedModificationTracker<TString, Bitset, eMod_Value> m_strValue;
	};
}

END_CHCORE_NAMESPACE

#endif
