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
// File was originally based on FileInfo.cpp by Antonio Tejada Lacaci.
// Almost everything has changed since then.
#include "stdafx.h"
#include "TFileInfo.h"
#include "../libchcore/TCoreException.h"
#include "../libchcore/ErrorCodes.h"
#include "../libchcore/TFileTime.h"
#include "../libserializer/ISerializerRowData.h"
#include "../libserializer/IColumnsDefinition.h"
#include "../libserializer/ISerializerContainer.h"

using namespace chcore;
using namespace serializer;

namespace chengine
{
	//////////////////////////////////////////////////////////////////////
	// Construction/Destruction
	//////////////////////////////////////////////////////////////////////

	TFileInfo::TFileInfo() :
		m_oidObjectID(0),
		m_pathFile(m_setModifications),
		m_spBasePathData(m_setModifications),
		m_dwAttributes(m_setModifications, 0),
		m_uhFileSize(m_setModifications, 0),
		m_ftCreation(m_setModifications),
		m_ftLastAccess(m_setModifications),
		m_ftLastWrite(m_setModifications),
		m_uiFlags(m_setModifications, 0)
	{
		m_setModifications[eMod_Added] = true;
	}

	TFileInfo::TFileInfo(const TBasePathDataPtr& spBasePathData, const TSmartPath& rpathFile, DWORD dwAttributes, ULONGLONG uhFileSize, const TFileTime& ftCreation, const TFileTime& ftLastAccess, const TFileTime& ftLastWrite, unsigned int uiFlags) :
		m_oidObjectID(0),
		m_pathFile(m_setModifications, rpathFile),
		m_spBasePathData(m_setModifications, spBasePathData),
		m_dwAttributes(m_setModifications, dwAttributes),
		m_uhFileSize(m_setModifications, uhFileSize),
		m_ftCreation(m_setModifications, ftCreation),
		m_ftLastAccess(m_setModifications, ftLastAccess),
		m_ftLastWrite(m_setModifications, ftLastWrite),
		m_uiFlags(m_setModifications, uiFlags)
	{
		if (m_spBasePathData.Get())
			m_pathFile.Modify().MakeRelativePath(m_spBasePathData.Get()->GetSrcPath());
	}

	TFileInfo::~TFileInfo()
	{
	}

	void TFileInfo::Init(const TBasePathDataPtr& spBasePathData, const TSmartPath& rpathFile,
		DWORD dwAttributes, ULONGLONG uhFileSize, const TFileTime& ftCreation, const TFileTime& ftLastAccess, const TFileTime& ftLastWrite,
		unsigned int uiFlags)
	{
		m_pathFile = rpathFile;
		m_spBasePathData = spBasePathData;
		m_dwAttributes = dwAttributes;
		m_uhFileSize = uhFileSize;
		m_ftCreation = ftCreation;
		m_ftLastAccess = ftLastAccess;
		m_ftLastWrite = ftLastWrite;
		m_uiFlags = uiFlags;

		if (m_spBasePathData.Get())
			m_pathFile.Modify().MakeRelativePath(m_spBasePathData.Get()->GetSrcPath());
	}

	void TFileInfo::Init(const TSmartPath& rpathFile, DWORD dwAttributes, ULONGLONG uhFileSize, const TFileTime& ftCreation, const TFileTime& ftLastAccess, const TFileTime& ftLastWrite,
		unsigned int uiFlags)
	{
		m_pathFile = rpathFile;
		m_spBasePathData.Modify().reset();
		m_dwAttributes = dwAttributes;
		m_uhFileSize = uhFileSize;
		m_ftCreation = ftCreation;
		m_ftLastAccess = ftLastAccess;
		m_ftLastWrite = ftLastWrite;
		m_uiFlags = uiFlags;
	}

	void TFileInfo::SetParentObject(const TBasePathDataPtr& spBasePathData)
	{
		// cannot set parent object if there is already one specified
		if (m_spBasePathData.Get())
			throw TCoreException(eErr_InvalidArgument, L"m_spBasePathData", LOCATION);

		m_spBasePathData = spBasePathData;
		if (m_spBasePathData.Get())
			m_pathFile.Modify().MakeRelativePath(m_spBasePathData.Get()->GetSrcPath());
	}

	bool TFileInfo::operator==(const TFileInfo& rInfo) const
	{
		return (rInfo.m_dwAttributes == m_dwAttributes &&
			rInfo.m_ftCreation.Get() == m_ftCreation.Get() &&
			rInfo.m_ftLastWrite.Get() == m_ftLastWrite.Get() &&
			rInfo.m_uhFileSize == m_uhFileSize);
	}

	TSmartPath TFileInfo::GetFullFilePath() const
	{
		if (m_spBasePathData.Get())
		{
			TSmartPath pathCombined = m_spBasePathData.Get()->GetSrcPath();
			pathCombined += m_pathFile;
			return pathCombined;
		}

		return m_pathFile;
	}

	object_id_t TFileInfo::GetSrcObjectID() const
	{
		if (m_spBasePathData.Get())
			return m_spBasePathData.Get()->GetObjectID();
		return (object_id_t)-1;
	}

	TBasePathDataPtr TFileInfo::GetBasePathData() const
	{
		return m_spBasePathData;
	}

	void TFileInfo::MarkAsProcessed(bool bProcessed)
	{
		if (bProcessed)
			m_uiFlags.Modify() |= eFlag_Processed;
		else
			m_uiFlags.Modify() &= ~eFlag_Processed;
	}

	bool TFileInfo::IsProcessed() const
	{
		return m_uiFlags & eFlag_Processed;
	}

	bool TFileInfo::IsBasePathProcessed() const
	{
		if(!m_spBasePathData.Get())
			return false;

		return m_spBasePathData.Get()->GetSkipFurtherProcessing();
	}

	ULONGLONG TFileInfo::GetLength64() const
	{
		return m_uhFileSize;
	}

	void TFileInfo::SetLength64(ULONGLONG uhSize)
	{
		m_uhFileSize = uhSize;
	}

	void TFileInfo::SetFileTimes(const TFileTime& rCreation, const TFileTime& rLastAccess, const TFileTime& rLastWrite)
	{
		m_ftCreation = rCreation;
		m_ftLastAccess = rLastAccess;
		m_ftLastWrite = rLastWrite;
	}

	const TSmartPath& TFileInfo::GetFilePath() const
	{
		return m_pathFile;
	}

	void TFileInfo::SetFilePath(const TSmartPath& tPath)
	{
		m_pathFile = tPath;
	}

	const TFileTime& TFileInfo::GetCreationTime() const
	{
		return m_ftCreation;
	}

	const TFileTime& TFileInfo::GetLastAccessTime() const
	{
		return m_ftLastAccess;
	}

	const TFileTime& TFileInfo::GetLastWriteTime() const
	{
		return m_ftLastWrite;
	}

	DWORD TFileInfo::GetAttributes() const
	{
		return m_dwAttributes;
	}

	void TFileInfo::SetAttributes(DWORD dwAttributes)
	{
		m_dwAttributes = dwAttributes;
	}

	bool TFileInfo::IsDirectory() const
	{
		return (m_dwAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
	}

	bool TFileInfo::IsArchived() const
	{
		return (m_dwAttributes & FILE_ATTRIBUTE_ARCHIVE) != 0;
	}

	bool TFileInfo::IsReadOnly() const
	{
		return (m_dwAttributes & FILE_ATTRIBUTE_READONLY) != 0;
	}

	bool TFileInfo::IsCompressed() const
	{
		return (m_dwAttributes & FILE_ATTRIBUTE_COMPRESSED) != 0;
	}

	bool TFileInfo::IsSystem() const
	{
		return (m_dwAttributes & FILE_ATTRIBUTE_SYSTEM) != 0;
	}

	bool TFileInfo::IsHidden() const
	{
		return (m_dwAttributes & FILE_ATTRIBUTE_HIDDEN) != 0;
	}

	bool TFileInfo::IsTemporary() const
	{
		return (m_dwAttributes & FILE_ATTRIBUTE_TEMPORARY) != 0;
	}

	bool TFileInfo::IsNormal() const
	{
		return m_dwAttributes == 0;
	}

	void TFileInfo::Store(const ISerializerContainerPtr& spContainer) const
	{
		if (m_setModifications.any())
		{
			ISerializerRowData& rRow = spContainer->GetRow(m_oidObjectID, m_setModifications[eMod_Added]);

			if (m_setModifications[eMod_Path])
				rRow.SetValue(_T("rel_path"), m_pathFile);
			if (m_setModifications[eMod_BasePath])
				rRow.SetValue(_T("base_path_id"), m_spBasePathData.Get()->GetObjectID());
			if (m_setModifications[eMod_Attributes])
				rRow.SetValue(_T("attr"), m_dwAttributes);
			if (m_setModifications[eMod_FileSize])
				rRow.SetValue(_T("size"), m_uhFileSize);
			if (m_setModifications[eMod_TimeCreated])
				rRow.SetValue(_T("time_created"), m_ftCreation.Get().ToUInt64());
			if (m_setModifications[eMod_TimeLastWrite])
				rRow.SetValue(_T("time_last_write"), m_ftLastWrite.Get().ToUInt64());
			if (m_setModifications[eMod_TimeLastAccess])
				rRow.SetValue(_T("time_last_access"), m_ftLastAccess.Get().ToUInt64());
			if (m_setModifications[eMod_Flags])
				rRow.SetValue(_T("flags"), m_uiFlags);

			m_setModifications.reset();
		}
	}

	void TFileInfo::InitColumns(IColumnsDefinition& rColumns)
	{
		rColumns.AddColumn(_T("id"), ColumnType<object_id_t>::value);
		rColumns.AddColumn(_T("rel_path"), IColumnsDefinition::eType_path);
		rColumns.AddColumn(_T("base_path_id"), ColumnType<object_id_t>::value);
		rColumns.AddColumn(_T("attr"), IColumnsDefinition::eType_ulong);
		rColumns.AddColumn(_T("size"), IColumnsDefinition::eType_ulonglong);
		rColumns.AddColumn(_T("time_created"), IColumnsDefinition::eType_ulonglong);
		rColumns.AddColumn(_T("time_last_write"), IColumnsDefinition::eType_ulonglong);
		rColumns.AddColumn(_T("time_last_access"), IColumnsDefinition::eType_ulonglong);
		rColumns.AddColumn(_T("flags"), IColumnsDefinition::eType_uint);
	}

	void TFileInfo::Load(const ISerializerRowReaderPtr& spRowReader, const TBasePathDataContainerPtr& spSrcContainer)
	{
		size_t stBaseObjectID = 0;
		unsigned long long ullTime = 0;
		spRowReader->GetValue(_T("id"), m_oidObjectID);
		spRowReader->GetValue(_T("rel_path"), m_pathFile.Modify());
		spRowReader->GetValue(_T("base_path_id"), stBaseObjectID);
		spRowReader->GetValue(_T("attr"), m_dwAttributes.Modify());
		spRowReader->GetValue(_T("size"), m_uhFileSize.Modify());

		spRowReader->GetValue(_T("time_created"), ullTime);
		m_ftCreation.Modify().FromUInt64(ullTime);

		spRowReader->GetValue(_T("time_last_write"), ullTime);
		m_ftLastWrite.Modify().FromUInt64(ullTime);

		spRowReader->GetValue(_T("time_last_access"), ullTime);
		m_ftLastAccess.Modify().FromUInt64(ullTime);

		spRowReader->GetValue(_T("flags"), m_uiFlags.Modify());

		m_spBasePathData = spSrcContainer->FindByID(stBaseObjectID);

		m_setModifications.reset();
	}

	object_id_t TFileInfo::GetObjectID() const
	{
		return m_oidObjectID;
	}

	void TFileInfo::SetObjectID(object_id_t oidObjectID)
	{
		m_oidObjectID = oidObjectID;
	}
}
