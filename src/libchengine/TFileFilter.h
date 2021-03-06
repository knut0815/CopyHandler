/***************************************************************************
 *   Copyright (C) 2001-2008 by Jozef Starosczyk                           *
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
#ifndef __TFILEFILTER_H__
#define __TFILEFILTER_H__

#include <atltime.h>
#include "TDateTime.h"
#include <bitset>
#include "../libstring/TStringPatternArray.h"
#include "../libserializer/TSharedModificationTracker.h"

namespace chengine
{
	class TConfig;
	class TFileInfo;
	typedef std::shared_ptr<TFileInfo> TFileInfoPtr;

	class LIBCHENGINE_API TFileFilter
	{
	public:
		enum ESizeCompareType
		{
			eSizeCmp_Less = 0,
			eSizeCmp_LessOrEqual = 1,
			eSizeCmp_Equal = 2,
			eSizeCmp_GreaterOrEqual = 3,
			eSizeCmp_Greater = 4
		};

		enum EDateCompareType
		{
			eDateCmp_Less = 0,
			eDateCmp_LessOrEqual = 1,
			eDateCmp_Equal = 2,
			eDateCmp_GreaterOrEqual = 3,
			eDateCmp_Greater = 4
		};

		enum EDateType
		{
			eDateType_Created = 0,
			eDateType_Modified = 1,
			eDateType_LastAccessed = 2
		};

	public:
		TFileFilter();
		TFileFilter(const TFileFilter& rFilter);
		TFileFilter& operator=(const TFileFilter& rFilter);

		bool operator==(const TFileFilter& rSrc) const;
		bool operator!=(const TFileFilter& rSrc) const;

		void SetData(const TFileFilter& rSrc);

		// matching
		bool Match(const TFileInfoPtr& spInfo) const;

		// serialization
		void StoreInConfig(TConfig& rConfig) const;
		void ReadFromConfig(const TConfig& rConfig);

		void Store(const serializer::ISerializerContainerPtr& spContainer) const;
		void Load(const serializer::ISerializerRowReaderPtr& spRowReader);
		static void InitColumns(serializer::IColumnsDefinition& rColumns);

		// other
		serializer::object_id_t GetObjectID() const;
		void SetObjectID(serializer::object_id_t oidObjectID);
		void ResetModifications();

		// atrributes access
		bool GetUseMask() const;
		void SetUseMask(bool bUseMask);

		string::TString GetCombinedMask() const;
		void SetCombinedMask(const string::TString& pMask);

		bool GetUseExcludeMask() const;
		void SetUseExcludeMask(bool bUseExcludeMask);

		string::TString GetCombinedExcludeMask() const;
		void SetCombinedExcludeMask(const string::TString& pMask);

		bool GetUseSize1() const;
		void SetUseSize1(bool bUseSize1);

		ESizeCompareType GetSizeType1() const;
		void SetSizeType1(ESizeCompareType eSizeType1);

		unsigned long long GetSize1() const;
		void SetSize1(unsigned long long ullSize1);

		bool GetUseSize2() const;
		void SetUseSize2(bool bUseSize2);

		ESizeCompareType GetSizeType2() const;
		void SetSizeType2(ESizeCompareType eSizeType2);

		unsigned long long GetSize2() const;
		void SetSize2(unsigned long long ullSize2);

		// dates
		TFileFilter::EDateType GetDateType() const;
		void SetDateType(TFileFilter::EDateType eDateType);

		// date 1
		bool GetUseDateTime1() const;
		void SetUseDateTime1(bool bUseDateTime1);

		TFileFilter::EDateCompareType GetDateCmpType1() const;
		void SetDateCmpType1(TFileFilter::EDateCompareType eCmpType1);

		bool GetUseDate1() const;
		void SetUseDate1(bool tDate1);

		bool GetUseTime1() const;
		void SetUseTime1(bool tTime1);

		const TDateTime& GetDateTime1() const;
		void SetDateTime1(const TDateTime& tDateTime1);

		// date 2
		bool GetUseDateTime2() const;
		void SetUseDateTime2(bool bUseDateTime2);

		TFileFilter::EDateCompareType GetDateCmpType2() const;
		void SetDateCmpType2(TFileFilter::EDateCompareType eCmpType2);

		bool GetUseDate2() const;
		void SetUseDate2(bool tDate2);

		bool GetUseTime2() const;
		void SetUseTime2(bool tTime2);

		const TDateTime& GetDateTime2() const;
		void SetDateTime2(const TDateTime& tDateTime2);

		// attributes
		bool GetUseAttributes() const;
		void SetUseAttributes(bool bUseAttributes);

		int GetArchive() const;
		void SetArchive(int iArchive);

		int GetReadOnly() const;
		void SetReadOnly(int iReadOnly);

		int GetHidden() const;
		void SetHidden(int iHidden);

		int GetSystem() const { return m_iSystem; }
		void SetSystem(int iSystem) { m_iSystem = iSystem; }

		int GetDirectory() const { return m_iDirectory; }
		void SetDirectory(int iDirectory) { m_iDirectory = iDirectory; }

	private:
		enum EModifications
		{
			eMod_Added,
			eMod_UseMask,
			eMod_Mask,
			eMod_UseExcludeMask,
			eMod_ExcludeMask,
			eMod_UseSize1,
			eMod_SizeCmpType1,
			eMod_Size1,
			eMod_UseSize2,
			eMod_SizeCmpType2,
			eMod_Size2,
			eMod_DateType,
			eMod_UseDateTime1,
			eMod_DateCmpType1,
			eMod_UseDate1,
			eMod_UseTime1,
			eMod_DateTime1,
			eMod_UseDateTime2,
			eMod_DateCmpType2,
			eMod_UseDate2,
			eMod_UseTime2,
			eMod_DateTime2,
			eMod_UseAttributes,
			eMod_AttrArchive,
			eMod_AttrReadOnly,
			eMod_AttrHidden,
			eMod_AttrSystem,
			eMod_AttrDirectory,

			eMod_Last
		};

		// object identification
		serializer::object_id_t m_oidObjectID;

		// modification management
#pragma warning(push)
#pragma warning(disable: 4251)
		typedef std::bitset<eMod_Last> Bitset;
		mutable Bitset m_setModifications;

		// files mask
		serializer::TSharedModificationTracker<bool, Bitset, eMod_UseMask> m_bUseMask;
		serializer::TSharedModificationTracker<string::TStringPatternArray, Bitset, eMod_Mask> m_astrMask;

		// files mask-
		serializer::TSharedModificationTracker<bool, Bitset, eMod_UseExcludeMask> m_bUseExcludeMask;
		serializer::TSharedModificationTracker<string::TStringPatternArray, Bitset, eMod_ExcludeMask> m_astrExcludeMask;

		// size filtering
		serializer::TSharedModificationTracker<bool, Bitset, eMod_UseSize1> m_bUseSize1;
		serializer::TSharedModificationTracker<ESizeCompareType, Bitset, eMod_SizeCmpType1> m_eSizeCmpType1;
		serializer::TSharedModificationTracker<unsigned long long, Bitset, eMod_Size1> m_ullSize1;

		serializer::TSharedModificationTracker<bool, Bitset, eMod_UseSize2> m_bUseSize2;
		serializer::TSharedModificationTracker<ESizeCompareType, Bitset, eMod_SizeCmpType2> m_eSizeCmpType2;
		serializer::TSharedModificationTracker<unsigned long long, Bitset, eMod_Size2> m_ullSize2;

		// date filtering
		serializer::TSharedModificationTracker<EDateType, Bitset, eMod_DateType> m_eDateType;	// created/last modified/last accessed

		serializer::TSharedModificationTracker<bool, Bitset, eMod_UseDateTime1> m_bUseDateTime1;

		serializer::TSharedModificationTracker<EDateCompareType, Bitset, eMod_DateCmpType1> m_eDateCmpType1;	// before/after
		serializer::TSharedModificationTracker<bool, Bitset, eMod_UseDate1> m_bUseDate1;
		serializer::TSharedModificationTracker<bool, Bitset, eMod_UseTime1> m_bUseTime1;
		serializer::TSharedModificationTracker<TDateTime, Bitset, eMod_DateTime1> m_tDateTime1;

		serializer::TSharedModificationTracker<bool, Bitset, eMod_UseDateTime2> m_bUseDateTime2;

		serializer::TSharedModificationTracker<EDateCompareType, Bitset, eMod_DateCmpType2> m_eDateCmpType2;
		serializer::TSharedModificationTracker<bool, Bitset, eMod_UseDate2> m_bUseDate2;
		serializer::TSharedModificationTracker<bool, Bitset, eMod_UseTime2> m_bUseTime2;
		serializer::TSharedModificationTracker<TDateTime, Bitset, eMod_DateTime2> m_tDateTime2;

		// attribute filtering
		serializer::TSharedModificationTracker<bool, Bitset, eMod_UseAttributes> m_bUseAttributes;
		serializer::TSharedModificationTracker<int, Bitset, eMod_AttrArchive> m_iArchive;
		serializer::TSharedModificationTracker<int, Bitset, eMod_AttrReadOnly> m_iReadOnly;
		serializer::TSharedModificationTracker<int, Bitset, eMod_AttrHidden> m_iHidden;
		serializer::TSharedModificationTracker<int, Bitset, eMod_AttrSystem> m_iSystem;
		serializer::TSharedModificationTracker<int, Bitset, eMod_AttrDirectory> m_iDirectory;
#pragma warning(pop)
	};
}

#endif
