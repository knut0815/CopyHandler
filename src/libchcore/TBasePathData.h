// ============================================================================
//  Copyright (C) 2001-2010 by Jozef Starosczyk
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
/// @file  TBasePathData.h
/// @date  2010/10/13
/// @brief Contains declarations of classes related to keeping additional path data.
// ============================================================================
#ifndef __TBASEPATHDATA_H__
#define __TBASEPATHDATA_H__

#include "libchcore.h"
#include "TPath.h"
#include "TModPathContainer.h"
#include <bitset>
#include "TSharedModificationTracker.h"
#include "TRemovedObjects.h"

BEGIN_CHCORE_NAMESPACE

/////////////////////////////////////////////////////////////////////////////
// TBasePathData
class LIBCHCORE_API TBasePathData
{
private:
	enum EModifications
	{
		eMod_Added,
		eMod_SkipProcessing,
		eMod_DstPath,

		eMod_Last
	};

public:
	TBasePathData();
	TBasePathData(const TBasePathData& rEntry);

	bool GetSkipFurtherProcessing() const;
	void SetSkipFurtherProcessing(bool bSkipFurtherProcessing);

	void SetDestinationPath(const TSmartPath& strPath);
	TSmartPath GetDestinationPath() const;
	bool IsDestinationPathSet() const;

	void Store(const ISerializerContainerPtr& spContainer, size_t stObjectID) const;
	static void InitLoader(const IColumnsDefinitionPtr& spColumnDefs);
	void Load(const ISerializerRowReaderPtr& spRowReader, size_t& stObjectID);

private:
#pragma warning(push)
#pragma warning(disable: 4251)
	typedef std::bitset<eMod_Last> BitSet;
	mutable BitSet m_setModifications;

	TSharedModificationTracker<bool, BitSet, eMod_SkipProcessing> m_bSkipFurtherProcessing;		// specifies if the path should be (or not) processed further
	TSharedModificationTracker<TSmartPath, BitSet, eMod_DstPath> m_pathDst;
#pragma warning(pop)
};

typedef boost::shared_ptr<TBasePathData> TBasePathDataPtr;

//////////////////////////////////////////////////////////////////////////
// TBasePathDataContainer

class LIBCHCORE_API TBasePathDataContainer
{
public:
	// constructors/destructor
	TBasePathDataContainer();
	~TBasePathDataContainer();

	// standard access to data
	bool Exists(size_t stObjectID) const;
	TBasePathDataPtr GetExisting(size_t stObjectID) const;
	TBasePathDataPtr Get(size_t stObjectID);

	void Remove(size_t stObjectID);
	void Clear();

	// inner object read interface (to not create new inner objects when reading non-existent data)
	bool GetSkipFurtherProcessing(size_t stObjectID) const;
	TSmartPath GetDestinationPath(size_t stObjectID) const;
	bool IsDestinationPathSet(size_t stObjectID) const;

	void Store(const ISerializerContainerPtr& spContainer) const;
	void Load(const ISerializerContainerPtr& spContainer);

private:
	TBasePathDataContainer(const TBasePathDataContainer& rSrc);
	TBasePathDataContainer& operator=(const TBasePathDataContainer& rSrc);

protected:
#pragma warning(push)
#pragma warning(disable: 4251)
	typedef std::map<size_t, TBasePathDataPtr> MapEntries;
	MapEntries m_mapEntries;
	mutable TRemovedObjects m_setRemovedObjects;

	mutable boost::shared_mutex m_lock;
#pragma warning(pop)
};

END_CHCORE_NAMESPACE

#endif // __TBASEPATHDATA_H__
