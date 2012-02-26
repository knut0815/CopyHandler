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
/// @file  TSubTaskArray.cpp
/// @date  2011/11/08
/// @brief File contain implementation of a class handling a sequence of subtasks.
// ============================================================================
#include "stdafx.h"
#include "TSubTaskArray.h"
#include "TTaskOperationPlan.h"
#include <boost\smart_ptr\make_shared.hpp>
#include "TSubTaskScanDirectory.h"
#include "TSubTaskCopyMove.h"
#include "TSubTaskDelete.h"
#include "TSubTaskContext.h"
#include "TTaskLocalStats.h"
#include "TSubTaskFastMove.h"
#include "SerializationHelpers.h"
#include "TBinarySerializer.h"
#include "TTaskStatsSnapshot.h"

BEGIN_CHCORE_NAMESPACE

namespace details
{
	///////////////////////////////////////////////////////////////////////////
	// TTaskBasicProgressInfo

	TTaskBasicProgressInfo::TTaskBasicProgressInfo() :
		m_stSubOperationIndex(0)
	{
	}

	TTaskBasicProgressInfo::~TTaskBasicProgressInfo()
	{
	}

	void TTaskBasicProgressInfo::ResetProgress()
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);
		m_stSubOperationIndex = 0;
	}

	void TTaskBasicProgressInfo::SetSubOperationIndex(size_t stSubOperationIndex)
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);
		m_stSubOperationIndex = stSubOperationIndex;
	}

	size_t TTaskBasicProgressInfo::GetSubOperationIndex() const
	{
		boost::shared_lock<boost::shared_mutex> lock(m_lock);
		return m_stSubOperationIndex;
	}

	void TTaskBasicProgressInfo::IncreaseSubOperationIndex()
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);
		++m_stSubOperationIndex;
	}

	void TTaskBasicProgressInfo::Serialize(TReadBinarySerializer& rSerializer)
	{
		using Serializers::Serialize;

		size_t stSubOperationIndex = 0;
		Serialize(rSerializer, stSubOperationIndex);

		boost::unique_lock<boost::shared_mutex> lock(m_lock);

		m_stSubOperationIndex = stSubOperationIndex;
	}

	void TTaskBasicProgressInfo::Serialize(TWriteBinarySerializer& rSerializer) const
	{
		using Serializers::Serialize;

		size_t stSubOperationIndex = 0;
		{
			boost::shared_lock<boost::shared_mutex> lock(m_lock);
			stSubOperationIndex = m_stSubOperationIndex;
		}

		Serialize(rSerializer, stSubOperationIndex);
	}
}

///////////////////////////////////////////////////////////////////////////
// TSubTasksArray

TSubTasksArray::TSubTasksArray(TTaskLocalStatsInfo& rLocalStats) :
	m_pSubTaskContext(NULL),
	m_rLocalStats(rLocalStats)
{
}

TSubTasksArray::TSubTasksArray(const TOperationPlan& rOperationPlan, TSubTaskContext& rSubTaskContext, TTaskLocalStatsInfo& rLocalStats) :
	m_pSubTaskContext(NULL),
	m_rLocalStats(rLocalStats)
{
	Init(rOperationPlan, rSubTaskContext);
}

TSubTasksArray::~TSubTasksArray()
{
}

void TSubTasksArray::Init(const TOperationPlan& rOperationPlan, TSubTaskContext& rSubTaskContext)
{
	m_vSubTasks.clear();
	m_tProgressInfo.ResetProgress();
	m_pSubTaskContext = &rSubTaskContext;

	switch(rOperationPlan.GetOperationType())
	{
	case eOperation_Copy:
		{
			TSubTaskBasePtr spOperation = boost::make_shared<TSubTaskScanDirectories>(boost::ref(rSubTaskContext));
			AddSubTask(spOperation, 5, true);
			spOperation = boost::make_shared<TSubTaskCopyMove>(boost::ref(rSubTaskContext));
			AddSubTask(spOperation, 95, false);

			break;
		}
	case eOperation_Move:
		{
			TSubTaskBasePtr spOperation = boost::make_shared<TSubTaskFastMove>(boost::ref(rSubTaskContext));
			AddSubTask(spOperation, 5, true);
			spOperation = boost::make_shared<TSubTaskScanDirectories>(boost::ref(rSubTaskContext));
			AddSubTask(spOperation, 5, false);
			spOperation = boost::make_shared<TSubTaskCopyMove>(boost::ref(rSubTaskContext));
			AddSubTask(spOperation, 85, false);
			spOperation = boost::make_shared<TSubTaskDelete>(boost::ref(rSubTaskContext));
			AddSubTask(spOperation, 5, false);

			break;
		}
	default:
		THROW_CORE_EXCEPTION(eErr_UndefinedOperation);
	}
}

void TSubTasksArray::ResetProgressAndStats()
{
	m_tProgressInfo.ResetProgress();
	m_rLocalStats.Clear();

	boost::tuples::tuple<TSubTaskBasePtr, double, bool> tupleRow;
	BOOST_FOREACH(tupleRow, m_vSubTasks)
	{
		if(tupleRow.get<0>() == NULL)
			THROW_CORE_EXCEPTION(eErr_InternalProblem);

		tupleRow.get<0>()->Reset();
	}
}

void TSubTasksArray::SerializeProgress(TReadBinarySerializer& rSerializer)
{
	m_tProgressInfo.Serialize(rSerializer);
	boost::tuples::tuple<TSubTaskBasePtr, double, bool> tupleRow;
	BOOST_FOREACH(tupleRow, m_vSubTasks)
	{
		tupleRow.get<0>()->GetProgressInfo().Serialize(rSerializer);
	}
}

void TSubTasksArray::SerializeProgress(TWriteBinarySerializer& rSerializer) const
{
	m_tProgressInfo.Serialize(rSerializer);
	boost::tuples::tuple<TSubTaskBasePtr, double, bool> tupleRow;
	BOOST_FOREACH(tupleRow, m_vSubTasks)
	{
		tupleRow.get<0>()->GetProgressInfo().Serialize(rSerializer);
	}
}

TSubTaskBase::ESubOperationResult TSubTasksArray::Execute(bool bRunOnlyEstimationSubTasks)
{
	if(!m_pSubTaskContext)
		THROW_CORE_EXCEPTION(eErr_InternalProblem);

	TSubTaskBase::ESubOperationResult eResult = TSubTaskBase::eSubResult_Continue;

	size_t stSubOperationIndex = m_tProgressInfo.GetSubOperationIndex();

	for(; stSubOperationIndex < m_vSubTasks.size() && eResult == TSubTaskBase::eSubResult_Continue; ++stSubOperationIndex)
	{
		boost::tuples::tuple<TSubTaskBasePtr, int, bool>& rCurrentSubTask = m_vSubTasks[stSubOperationIndex];
		TSubTaskBasePtr spCurrentSubTask = rCurrentSubTask.get<0>();

		m_rLocalStats.SetCurrentSubOperationType(spCurrentSubTask->GetSubOperationType());

		// set current sub-operation index to allow resuming
		m_tProgressInfo.SetSubOperationIndex(stSubOperationIndex);

		// if we run in estimation mode only, then stop processing and return to the caller
		if(bRunOnlyEstimationSubTasks && !rCurrentSubTask.get<2>())
		{
			eResult = TSubTaskBase::eSubResult_Continue;
			break;
		}

		eResult = spCurrentSubTask->Exec();
	}

	return eResult;
}

void TSubTasksArray::AddSubTask(const TSubTaskBasePtr& spOperation, int iPercent, bool bIsPartOfEstimation)
{
	m_vSubTasks.push_back(boost::make_tuple(spOperation, iPercent, bIsPartOfEstimation));
}

void TSubTasksArray::GetTaskStats(TTaskStatsSnapshot& rSnapshot) const
{
	rSnapshot.Clear();

	// from local stats
	m_rLocalStats.GetSnapshot(rSnapshot);

	// current task
	size_t stSubOperationIndex = m_tProgressInfo.GetSubOperationIndex();

	const boost::tuples::tuple<TSubTaskBasePtr, int, bool>& rCurrentSubTask = m_vSubTasks[stSubOperationIndex];
	TSubTaskBasePtr spCurrentSubTask = rCurrentSubTask.get<0>();

	spCurrentSubTask->GetStatsSnapshot(rSnapshot.GetCurrentSubTaskStats());

	// progress
	TSubTaskStatsSnapshot tSnapshot;
	double dTotalProgress = 0.0;
	for(stSubOperationIndex = 0; stSubOperationIndex < m_vSubTasks.size(); ++stSubOperationIndex)
	{
		const boost::tuples::tuple<TSubTaskBasePtr, int, bool>& rCurrentSubTask = m_vSubTasks[stSubOperationIndex];
		TSubTaskBasePtr spCurrentSubTask = rCurrentSubTask.get<0>();
		int iSubTaskPercent = rCurrentSubTask.get<1>();

		spCurrentSubTask->GetStatsSnapshot(tSnapshot);

		double dCurrentTaskPercent = tSnapshot.GetProgressInPercent() * iSubTaskPercent / 100.0;
		dTotalProgress += dCurrentTaskPercent;
	}

	rSnapshot.SetTaskProgressInPercent(dTotalProgress);
}

END_CHCORE_NAMESPACE
