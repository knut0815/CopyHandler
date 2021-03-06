// ============================================================================
//  Copyright (C) 2001-2013 by Jozef Starosczyk
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
#include "stdafx.h"
#include "TTaskInfo.h"
#include "../libchcore/TCoreException.h"
#include "../libserializer/ISerializerContainer.h"
#include "../libserializer/ISerializerRowData.h"

using namespace chcore;
using namespace serializer;

namespace chengine
{
	TTaskInfoEntry::TTaskInfoEntry() :
		m_oidObjectID(0),
		m_pathSerializeLocation(m_setModifications),
		m_pathLogPath(m_setModifications),
		m_iOrder(m_setModifications, 0)
	{
		m_setModifications[eMod_Added] = true;
	}

	TTaskInfoEntry::TTaskInfoEntry(object_id_t oidTaskID, const TSmartPath& pathTask, const TSmartPath& pathLog, int iOrder, const TTaskPtr& spTask) :
		m_oidObjectID(oidTaskID),
		m_pathSerializeLocation(m_setModifications, pathTask),
		m_pathLogPath(m_setModifications, pathLog),
		m_iOrder(m_setModifications, iOrder),
		m_spTask(spTask)
	{
		m_setModifications[eMod_Added] = true;
	}

	TTaskInfoEntry::TTaskInfoEntry(const TTaskInfoEntry& rSrc) :
		m_oidObjectID(rSrc.m_oidObjectID),
		m_setModifications(rSrc.m_setModifications),
		m_pathSerializeLocation(m_setModifications, rSrc.m_pathSerializeLocation),
		m_pathLogPath(m_setModifications, rSrc.m_pathLogPath),
		m_iOrder(m_setModifications, rSrc.m_iOrder),
		m_spTask(rSrc.m_spTask)
	{
	}

	TTaskInfoEntry& TTaskInfoEntry::operator=(const TTaskInfoEntry& rSrc)
	{
		if (this != &rSrc)
		{
			m_oidObjectID = rSrc.m_oidObjectID;
			m_pathSerializeLocation = rSrc.m_pathSerializeLocation;
			m_pathLogPath = rSrc.m_pathLogPath;
			m_iOrder = rSrc.m_iOrder;
			m_spTask = rSrc.m_spTask;
			m_setModifications = rSrc.m_setModifications;
		}

		return *this;
	}

	TSmartPath TTaskInfoEntry::GetTaskSerializeLocation() const
	{
		return m_pathSerializeLocation;
	}

	void TTaskInfoEntry::SetTaskSerializeLocation(const TSmartPath& strTaskPath)
	{
		m_pathSerializeLocation = strTaskPath;
	}

	TSmartPath TTaskInfoEntry::GetTaskLogPath() const
	{
		return m_pathLogPath;
	}

	void TTaskInfoEntry::SetTaskLogPath(const TSmartPath& pathLog)
	{
		m_pathLogPath = pathLog;
	}

	TTaskPtr TTaskInfoEntry::GetTask() const
	{
		return m_spTask;
	}

	void TTaskInfoEntry::SetTask(const TTaskPtr& spTask)
	{
		m_spTask = spTask;
	}

	int TTaskInfoEntry::GetOrder() const
	{
		return m_iOrder;
	}

	void TTaskInfoEntry::SetOrder(int iOrder)
	{
		m_iOrder = iOrder;
	}

	void TTaskInfoEntry::Store(const ISerializerContainerPtr& spContainer) const
	{
		if (!m_setModifications.any())
			return;

		bool bAdded = m_setModifications[eMod_Added];
		ISerializerRowData& rRow = spContainer->GetRow(m_oidObjectID, bAdded);

		if (bAdded || m_setModifications[eMod_TaskPath])
			rRow.SetValue(_T("path"), m_pathSerializeLocation);
		if (bAdded || m_setModifications[eMod_LogPath])
			rRow.SetValue(_T("logpath"), m_pathLogPath);
		if (bAdded || m_setModifications[eMod_Order])
			rRow.SetValue(_T("task_order"), m_iOrder);

		m_setModifications.reset();
	}

	void TTaskInfoEntry::Load(const ISerializerRowReaderPtr& spRowReader)
	{
		spRowReader->GetValue(_T("id"), m_oidObjectID);
		spRowReader->GetValue(_T("path"), m_pathSerializeLocation.Modify());
		spRowReader->GetValue(_T("logpath"), m_pathLogPath.Modify());
		spRowReader->GetValue(_T("task_order"), m_iOrder.Modify());

		m_setModifications.reset();
	}

	void TTaskInfoEntry::InitColumns(IColumnsDefinition& rColumnDefs)
	{
		rColumnDefs.AddColumn(_T("id"), ColumnType<object_id_t>::value);
		rColumnDefs.AddColumn(_T("path"), IColumnsDefinition::eType_path);
		rColumnDefs.AddColumn(_T("logpath"), IColumnsDefinition::eType_path);
		rColumnDefs.AddColumn(_T("task_order"), IColumnsDefinition::eType_int);
	}

	object_id_t TTaskInfoEntry::GetObjectID() const
	{
		return m_oidObjectID;
	}

	void TTaskInfoEntry::ResetModifications()
	{
		m_setModifications.reset();
	}

	///////////////////////////////////////////////////////////////////////////
	TTaskInfoContainer::TTaskInfoContainer() :
		m_oidLastObjectID(0)
	{
	}

	void TTaskInfoContainer::Add(const TSmartPath& pathTask, const TSmartPath& pathLog, int iOrder, const TTaskPtr& spTask)
	{
		m_vTaskInfos.push_back(TTaskInfoEntry(++m_oidLastObjectID, pathTask, pathLog, iOrder, spTask));
	}

	void TTaskInfoContainer::RemoveAt(size_t stIndex)
	{
		if (stIndex >= m_vTaskInfos.size())
			throw TCoreException(eErr_BoundsExceeded, L"stIndex", LOCATION);

		std::vector<TTaskInfoEntry>::iterator iter = m_vTaskInfos.begin() + stIndex;
		object_id_t oidTaskID = (*iter).GetObjectID();
		m_vTaskInfos.erase(m_vTaskInfos.begin() + stIndex);
		m_setRemovedTasks.Add(oidTaskID);
	}

	void TTaskInfoContainer::Clear()
	{
		for(TTaskInfoEntry& rEntry : m_vTaskInfos)
		{
			m_setRemovedTasks.Add(rEntry.GetObjectID());
		}
		m_vTaskInfos.clear();
	}

	TTaskInfoEntry& TTaskInfoContainer::GetAt(size_t stIndex)
	{
		if (stIndex >= m_vTaskInfos.size())
			throw TCoreException(eErr_BoundsExceeded, L"stIndex", LOCATION);

		return m_vTaskInfos[stIndex];
	}

	const TTaskInfoEntry& TTaskInfoContainer::GetAt(size_t stIndex) const
	{
		if (stIndex >= m_vTaskInfos.size())
			throw TCoreException(eErr_BoundsExceeded, L"stIndex", LOCATION);

		return m_vTaskInfos[stIndex];
	}

	size_t TTaskInfoContainer::GetCount() const
	{
		return m_vTaskInfos.size();
	}

	bool TTaskInfoContainer::GetByTaskID(taskid_t tTaskID, TTaskInfoEntry& rInfo) const
	{
		for (std::vector<TTaskInfoEntry>::const_iterator iter = m_vTaskInfos.begin(); iter != m_vTaskInfos.end(); ++iter)
		{
			if ((*iter).GetObjectID() == tTaskID)
			{
				rInfo = *iter;
				return true;
			}
		}

		return false;
	}

	bool TTaskInfoContainer::IsEmpty() const
	{
		return m_vTaskInfos.empty();
	}

	void TTaskInfoContainer::ClearModifications()
	{
		m_setRemovedTasks.Clear();

		for(TTaskInfoEntry& rEntry : m_vTaskInfos)
		{
			// if marked as added, we don't consider it modified anymore
			rEntry.ResetModifications();
		}
	}

	void TTaskInfoContainer::Store(const ISerializerContainerPtr& spContainer) const
	{
		InitColumns(spContainer);

		spContainer->DeleteRows(m_setRemovedTasks);
		m_setRemovedTasks.Clear();

		for(const TTaskInfoEntry& rEntry : m_vTaskInfos)
		{
			rEntry.Store(spContainer);
		}
	}

	void TTaskInfoContainer::Load(const ISerializerContainerPtr& spContainer)
	{
		InitColumns(spContainer);

		ISerializerRowReaderPtr spRowReader = spContainer->GetRowReader();

		TTaskInfoEntry tEntry;
		while (spRowReader->Next())
		{
			tEntry.Load(spRowReader);

			m_vTaskInfos.push_back(tEntry);
			m_oidLastObjectID = std::max(m_oidLastObjectID, tEntry.GetObjectID());
		}
	}

	TTaskInfoEntry& TTaskInfoContainer::GetAtOid(object_id_t oidObjectID)
	{
		for (std::vector<TTaskInfoEntry>::iterator iter = m_vTaskInfos.begin(); iter != m_vTaskInfos.end(); ++iter)
		{
			if ((*iter).GetObjectID() == oidObjectID)
				return *iter;
		}

		throw TCoreException(eErr_InvalidArgument, L"Object id does not exist", LOCATION);
	}

	void TTaskInfoContainer::InitColumns(const ISerializerContainerPtr& spContainer) const
	{
		IColumnsDefinition& rColumns = spContainer->GetColumnsDefinition();
		if (rColumns.IsEmpty())
			TTaskInfoEntry::InitColumns(rColumns);
	}
}
