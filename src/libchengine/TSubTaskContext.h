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
/// @file  TSubTaskContext.h
/// @date  2010/09/19
/// @brief Contains declaration of subtask context class.
// ============================================================================
#ifndef __TSUBTASKCONTEXT_H__
#define __TSUBTASKCONTEXT_H__

#include "EOperationTypes.h"
#include "TBasePathData.h"
#include "TFileInfoArray.h"
#include "IFilesystem.h"
#include "../liblogger/TLogFileData.h"

namespace chcore
{
	class TWorkerThreadController;
}

namespace chengine
{
	class TTaskConfigTracker;
	class TConfig;
	class TFileFiltersArray;

	///////////////////////////////////////////////////////////////////////////
	// TSubTaskContext

	class LIBCHENGINE_API TSubTaskContext
	{
	public:
		TSubTaskContext(TConfig& rConfig, const TBasePathDataContainerPtr& spBasePaths,
			const TFileFiltersArray& rFilters,
			TTaskConfigTracker& rCfgTracker, const logger::TLogFileDataPtr& spLogFileData,
		                chcore::TWorkerThreadController& rThreadController, const IFilesystemPtr& spFilesystem);
		TSubTaskContext(const TSubTaskContext& rSrc) = delete;
		~TSubTaskContext();

		TSubTaskContext& operator=(const TSubTaskContext& rSrc) = delete;

		TConfig& GetConfig();
		const TConfig& GetConfig() const;

		EOperationType GetOperationType() const;
		void SetOperationType(EOperationType eOperationType);

		TBasePathDataContainerPtr GetBasePaths() const;

		const TFileFiltersArray& GetFilters() const;
		TFileInfoArray& GetFilesCache();
		const TFileInfoArray& GetFilesCache() const;

		chcore::TSmartPath GetDestinationPath() const;
		void SetDestinationPath(const chcore::TSmartPath& pathDestination);

		TTaskConfigTracker& GetCfgTracker();
		const TTaskConfigTracker& GetCfgTracker() const;

		logger::TLogFileDataPtr GetLogFileData() const;

		chcore::TWorkerThreadController& GetThreadController();
		const chcore::TWorkerThreadController& GetThreadController() const;

		IFilesystemPtr GetLocalFilesystem() const;

	private:
		TConfig& m_rConfig;

		EOperationType m_eOperationType;

		// information about input paths
#pragma warning(push)
#pragma warning(disable: 4251)
		TBasePathDataContainerPtr m_spBasePaths;
#pragma warning(pop)

		const TFileFiltersArray& m_rFilters;

		// data on which to operate
		TFileInfoArray m_tFilesCache;

		chcore::TSmartPath m_pathDestination;

		// configuration changes tracking
		TTaskConfigTracker& m_rCfgTracker;

		// local filesystem access functions
#pragma warning(push)
#pragma warning(disable: 4251)
		IFilesystemPtr m_spFilesystem;
		logger::TLogFileDataPtr m_spLogFileData;
#pragma warning(pop)

		// thread control
		chcore::TWorkerThreadController& m_rThreadController;
	};
}

#endif // __TSUBTASKCONTEXT_H__
