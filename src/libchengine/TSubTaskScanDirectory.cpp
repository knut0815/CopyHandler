// ============================================================================
//  Copyright (C) 2001-2009 by Jozef Starosczyk
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
/// @file  TSubTaskScanDirectory.cpp
/// @date  2010/09/18
/// @brief Contains implementation of classes related to scan directory subtask.
// ============================================================================
#include "stdafx.h"
#include "TSubTaskScanDirectory.h"
#include "TSubTaskContext.h"
#include "TTaskConfiguration.h"
#include "IFeedbackHandler.h"
#include "TBasePathData.h"
#include "../libchcore/TWorkerThreadController.h"
#include "TTaskLocalStats.h"
#include "TFileInfoArray.h"
#include "TFileInfo.h"
#include "TScopedRunningTimeTracker.h"
#include "TFeedbackHandlerWrapper.h"
#include "TBufferSizes.h"
#include "TFilesystemFeedbackWrapper.h"

using namespace chcore;
using namespace string;
using namespace serializer;

namespace chengine
{
	///////////////////////////////////////////////////////////////////////////////////////////////////
	// class TSubTaskScanDirectories
	TSubTaskScanDirectories::TSubTaskScanDirectories(TSubTaskContext& rContext) :
		TSubTaskBase(rContext),
		m_tSubTaskStats(eSubOperation_Scanning),
		m_spLog(std::make_unique<logger::TLogger>(rContext.GetLogFileData(), L"ST-ScanDirs"))
	{
	}

	TSubTaskScanDirectories::~TSubTaskScanDirectories()
	{
	}

	void TSubTaskScanDirectories::Reset()
	{
		m_tSubTaskStats.Clear();
	}

	void TSubTaskScanDirectories::InitBeforeExec()
	{
		TBasePathDataContainerPtr spBasePaths = GetContext().GetBasePaths();

		file_count_t fcSize = spBasePaths->GetCount();
		file_count_t fcIndex = m_tSubTaskStats.GetCurrentIndex();

		if(fcIndex >= fcSize)
			fcIndex = 0;

		if(fcSize > 0)
		{
			TBasePathDataPtr spBasePath = spBasePaths->GetAt(fcIndex);
			m_tSubTaskStats.SetCurrentPath(spBasePath->GetSrcPath().ToString());
		}
		else
			m_tSubTaskStats.SetCurrentPath(TString());
	}

	TSubTaskScanDirectories::ESubOperationResult TSubTaskScanDirectories::Exec(const IFeedbackHandlerPtr& spFeedback)
	{
		TScopedRunningTimeTracker guard(m_tSubTaskStats);
		TFeedbackHandlerWrapperPtr spFeedbackHandler(std::make_shared<TFeedbackHandlerWrapper>(spFeedback, guard));

		// log
		TFileInfoArray& rFilesCache = GetContext().GetFilesCache();
		TWorkerThreadController& rThreadController = GetContext().GetThreadController();
		TBasePathDataContainerPtr spBasePaths = GetContext().GetBasePaths();
		const TConfig& rConfig = GetContext().GetConfig();
		const TFileFiltersArray& rafFilters = GetContext().GetFilters();
		IFilesystemPtr spFilesystem = GetContext().GetLocalFilesystem();

		TFilesystemFeedbackWrapper tFilesystemFBWrapper(spFeedbackHandler, spFilesystem, GetContext().GetLogFileData(), rThreadController);

		LOG_INFO(m_spLog) << _T("Searching for files...");

		// reset progress
		rFilesCache.SetComplete(false);

		// new stats
		m_tSubTaskStats.SetCurrentBufferIndex(TBufferSizes::eBuffer_Default);
		m_tSubTaskStats.SetTotalCount(spBasePaths->GetCount());
		m_tSubTaskStats.SetProcessedCount(0);
		m_tSubTaskStats.SetTotalSize(0);
		m_tSubTaskStats.SetProcessedSize(0);
		m_tSubTaskStats.SetCurrentPath(TString());

		// delete the content of rFilesCache
		rFilesCache.Clear();

		bool bIgnoreDirs = GetTaskPropValue<eTO_IgnoreDirectories>(rConfig);
		bool bForceDirectories = GetTaskPropValue<eTO_CreateDirectoriesRelativeToRoot>(rConfig);

		// add everything
		TString strFormat;

		file_count_t fcSize = spBasePaths->GetCount();
		// NOTE: in theory, we should resume the scanning, but in practice we are always restarting scanning if interrupted.
		file_count_t fcIndex = 0;		// m_tSubTaskStats.GetCurrentIndex()
		for (; fcIndex < fcSize; fcIndex++)
		{
			TBasePathDataPtr spBasePath = spBasePaths->GetAt(fcIndex);
			TSmartPath pathCurrent = spBasePath->GetSrcPath();

			m_tSubTaskStats.SetCurrentIndex(fcIndex);

			// new stats
			m_tSubTaskStats.SetProcessedCount(fcIndex);
			m_tSubTaskStats.SetCurrentPath(pathCurrent.ToString());

			TFileInfoPtr spFileInfo(std::make_shared<TFileInfo>());

			// check if we want to process this path at all (might be already fast moved)
			if (spBasePath->GetSkipFurtherProcessing())
				continue;

			// try to get some info about the input path; let user know if the path does not exist.
			ESubOperationResult eResult = tFilesystemFBWrapper.GetFileInfoFB(pathCurrent, spFileInfo, spBasePath);
			if (eResult == TSubTaskBase::eSubResult_SkipFile)
				continue;
			if (eResult != TSubTaskBase::eSubResult_Continue)
				return eResult;

			// log
			strFormat = _T("Adding file/folder (clipboard) : %path ...");
			strFormat.Replace(_T("%path"), pathCurrent.ToString());
			LOG_INFO(m_spLog) << strFormat.c_str();

			// add if needed
			if (spFileInfo->IsDirectory())
			{
				// add if folder's aren't ignored
				if (!bIgnoreDirs && !bForceDirectories)
				{
					// add directory info; it is not to be filtered with afFilters
					rFilesCache.Add(spFileInfo);

					// log
					strFormat = _T("Added folder %path");
					strFormat.Replace(_T("%path"), spFileInfo->GetFullFilePath().ToString());
					LOG_INFO(m_spLog) << strFormat.c_str();
				}

				// don't add folder contents when moving inside one disk boundary
				// log
				strFormat = _T("Recursing folder %path");
				strFormat.Replace(_T("%path"), spFileInfo->GetFullFilePath().ToString());
				LOG_INFO(m_spLog) << strFormat.c_str();

				ScanDirectory(spFileInfo->GetFullFilePath(), spBasePath, true, !bIgnoreDirs || bForceDirectories, rafFilters);

				// check for kill need
				if (rThreadController.KillRequested())
				{
					// log
					LOG_INFO(m_spLog) << _T("Kill request while adding data to files array (RecurseDirectories)");
					rFilesCache.Clear();
					return eSubResult_KillRequest;
				}
			}
			else
			{
				// add file info if passes filters
				if (rafFilters.Match(spFileInfo))
					rFilesCache.Add(spFileInfo);

				// log
				strFormat = _T("Added file %path");
				strFormat.Replace(_T("%path"), spFileInfo->GetFullFilePath().ToString());
				LOG_INFO(m_spLog) << strFormat.c_str();
			}
		}

		// update stats
		m_tSubTaskStats.SetCurrentIndex(fcIndex);
		m_tSubTaskStats.SetProcessedCount(fcIndex);
		m_tSubTaskStats.SetCurrentPath(TString());

		rFilesCache.SetComplete(true);

		// log
		LOG_INFO(m_spLog) << _T("Searching for files finished");

		return eSubResult_Continue;
	}

	void TSubTaskScanDirectories::GetStatsSnapshot(TSubTaskStatsSnapshotPtr& spStats) const
	{
		m_tSubTaskStats.GetSnapshot(spStats);
	}

	int TSubTaskScanDirectories::ScanDirectory(TSmartPath pathDirName, const TBasePathDataPtr& spBasePathData,
		bool bRecurse, bool bIncludeDirs, const TFileFiltersArray& afFilters)
	{
		TFileInfoArray& rFilesCache = GetContext().GetFilesCache();
		TWorkerThreadController& rThreadController = GetContext().GetThreadController();
		TBasePathDataContainerPtr spBasePaths = GetContext().GetBasePaths();
		const IFilesystemPtr& spFilesystem = GetContext().GetLocalFilesystem();

		IFilesystemFindPtr spFinder = spFilesystem->CreateFinderObject(pathDirName, PathFromString(_T("*")));
		TFileInfoPtr spFileInfo(std::make_shared<TFileInfo>());

		while (spFinder->FindNext(spFileInfo))
		{
			if (rThreadController.KillRequested())
				break;

			if (!spFileInfo->IsDirectory())
			{
				if (afFilters.Match(spFileInfo))
				{
					spFileInfo->SetParentObject(spBasePathData);
					rFilesCache.Add(spFileInfo);
					spFileInfo = std::make_shared<TFileInfo>();
				}
			}
			else
			{
				TSmartPath pathCurrent = spFileInfo->GetFullFilePath();
				if (bIncludeDirs)
				{
					spFileInfo->SetParentObject(spBasePathData);
					rFilesCache.Add(spFileInfo);
					spFileInfo = std::make_shared<TFileInfo>();
				}

				if (bRecurse)
					ScanDirectory(pathCurrent, spBasePathData, bRecurse, bIncludeDirs, afFilters);
			}
		}

		return 0;
	}
	void TSubTaskScanDirectories::Store(const ISerializerPtr& spSerializer) const
	{
		ISerializerContainerPtr spContainer = spSerializer->GetContainer(_T("subtask_scan"));

		InitColumns(spContainer);

		ISerializerRowData& rRow = spContainer->GetRow(0, m_tSubTaskStats.WasAdded());

		m_tSubTaskStats.Store(rRow);
	}

	void TSubTaskScanDirectories::Load(const ISerializerPtr& spSerializer)
	{
		ISerializerContainerPtr spContainer = spSerializer->GetContainer(_T("subtask_scan"));

		InitColumns(spContainer);

		ISerializerRowReaderPtr spRowReader = spContainer->GetRowReader();
		if(spRowReader->Next())
			m_tSubTaskStats.Load(spRowReader);
	}

	void TSubTaskScanDirectories::InitColumns(const ISerializerContainerPtr& spContainer) const
	{
		IColumnsDefinition& rColumns = spContainer->GetColumnsDefinition();
		if(rColumns.IsEmpty())
			TSubTaskStatsInfo::InitColumns(rColumns);
	}
}
