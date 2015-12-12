// ============================================================================
//  Copyright (C) 2001-2011 by Jozef Starosczyk
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
/// @file  TSubTaskArray.h
/// @date  2011/11/08
/// @brief File contain definition of a class handling a sequence of subtasks.
// ============================================================================
#ifndef __TSUBTASKSARRAY_H__
#define __TSUBTASKSARRAY_H__

#include "libchcore.h"
#include <boost/tuple/tuple.hpp>
#include "TSubTaskBase.h"
#include "TTaskLocalStats.h"
#include "TSubTaskArrayStatsSnapshot.h"
#include "TSharedModificationTracker.h"
#include <bitset>
#include <atomic>

namespace chcore
{
	class TOperationPlan;
	class TSubTaskContext;

	///////////////////////////////////////////////////////////////////////////
	// TTaskBasicProgressInfo
	class LIBCHCORE_API TSubTasksArray
	{
	public:
		TSubTasksArray(TSubTaskContext& rSubTaskContext);
		TSubTasksArray(const TOperationPlan& rOperationPlan, TSubTaskContext& rSubTaskContext);
		~TSubTasksArray();

		void Init(const TOperationPlan& rOperationPlan);
		EOperationType GetOperationType() const;

		// Stats handling
		void GetStatsSnapshot(TSubTaskArrayStatsSnapshot& rSnapshot) const;
		void ResetProgressAndStats();

		// progress handling
		void Store(const ISerializerPtr& spSerializer) const;
		void Load(const ISerializerPtr& spSerializer);

		TSubTaskBase::ESubOperationResult Execute(const IFeedbackHandlerPtr& spFeedbackHandler, bool bRunOnlyEstimationSubTasks);

	private:
		TSubTasksArray(const TSubTasksArray& rSrc);
		TSubTasksArray& operator=(const TSubTasksArray& rSrc);

		void AddSubTask(const TSubTaskBasePtr& spOperation, bool bIsPartOfEstimation);
		static TSubTaskBasePtr CreateSubtask(ESubOperationType eType, TSubTaskContext& rContext);

		IColumnsDefinition& InitSubtasksColumns(const ISerializerContainerPtr& spContainer) const;
		IColumnsDefinition& InitSubtasksInfoColumns(const ISerializerContainerPtr& spContainer) const;

	private:
		enum EModifications
		{
			eMod_Added,
			eMod_OperationType,

			// last element
			eMod_Last
		};

		typedef std::bitset<eMod_Last> Bitset;

		TSubTaskContext& m_rSubTaskContext;

#pragma warning(push)
#pragma warning(disable: 4251)
		mutable Bitset m_setModifications;

		TSharedModificationTracker<EOperationType, Bitset, eMod_OperationType> m_eOperationType;

		std::vector<std::pair<TSubTaskBasePtr, bool> > m_vSubTasks;	// pointer to the subtask object / is this the part of estimation?

		mutable std::atomic<object_id_t> m_oidSubOperationIndex;		 // index of sub-operation from TOperationDescription

#pragma warning(pop)

		mutable object_id_t m_oidLastStoredIndex;

		friend class TScopedRunningTimeTracker;
	};
}

#endif
