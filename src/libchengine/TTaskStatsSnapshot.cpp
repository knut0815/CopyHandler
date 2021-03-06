// ============================================================================
//  Copyright (C) 2001-2012 by Jozef Starosczyk
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
/// @file  TTaskStatsSnapshot.cpp
/// @date  2012/02/26
/// @brief Contains class responsible for holding task stats.
// ============================================================================
#include "stdafx.h"
#include "TTaskStatsSnapshot.h"
#include "EngineConstants.h"
#include "../libchcore/MathFunctions.h"

using namespace chcore;
using namespace string;

namespace chengine
{
	////////////////////////////////////////////////////////////////////////////////
	// TTaskStatsSnapshot members

	TTaskStatsSnapshot::TTaskStatsSnapshot() :
		m_tSubTasksStats(),
		m_tTaskID(0),
		m_bTaskIsRunning(false),
		m_ullTimeElapsed(0),
		m_iThreadPriority(0),
		m_strDestinationPath(),
		m_filters(),
		m_eTaskState(eTaskState_None),
		m_strTaskID(),
		m_eOperationType(eOperation_None),
		m_bIgnoreDirectories(false),
		m_bCreateEmptyFiles(false),
		m_ullCurrentBufferSize(0),
		m_uiBufferCount(0),
		m_bCacheFilled(false),
		m_ullProcessedCount(0),
		m_ullTotalCount(0),
		m_ullProcessedSize(0),
		m_ullTotalSize(0),
		m_dTaskCountSpeed(0.0),
		m_dTaskSizeSpeed(0.0),
		m_dCombinedProgress(0.0)
	{
	}

	void TTaskStatsSnapshot::Clear()
	{
		m_tSubTasksStats.Clear();
		m_bTaskIsRunning = false;
		m_ullTimeElapsed = 0;
		m_iThreadPriority = 0;
		m_strDestinationPath.Clear();
		m_filters.Clear();
		m_eTaskState = eTaskState_None;
		m_strTaskID.Clear();
		m_eOperationType = eOperation_None;
		m_bIgnoreDirectories = false;
		m_bCreateEmptyFiles = false;
		m_ullCurrentBufferSize = 0;
		m_bCacheFilled = false;
		m_ullProcessedCount = 0;
		m_ullTotalCount = 0;
		m_ullProcessedSize = 0;
		m_ullTotalSize = 0;
		m_dTaskCountSpeed = 0.0;
		m_dTaskSizeSpeed = 0.0;
		m_dCombinedProgress = 0.0;
		m_uiBufferCount = 0;
	}

	void TTaskStatsSnapshot::CalculateProgressAndSpeeds() const
	{
		m_bCacheFilled = false;
		m_ullProcessedCount = 0;
		m_ullTotalCount = 0;
		m_ullProcessedSize = 0;
		m_ullTotalSize = 0;
		m_dTaskCountSpeed = 0.0;
		m_dTaskSizeSpeed = 0.0;
		m_dCombinedProgress = 0.0;

		size_t stCount = m_tSubTasksStats.GetSubTaskSnapshotCount();
		for (size_t stIndex = 0; stIndex < stCount; ++stIndex)
		{
			TSubTaskStatsSnapshotPtr spSubtaskStats = m_tSubTasksStats.GetSubTaskSnapshotAt(stIndex);

			m_ullProcessedCount += spSubtaskStats->GetProcessedCount();
			m_ullTotalCount += spSubtaskStats->GetTotalCount();

			m_ullProcessedSize += spSubtaskStats->GetProcessedSize();
			m_ullTotalSize += spSubtaskStats->GetTotalSize();

			if(IsTaskRunning())
			{
				m_dTaskCountSpeed += spSubtaskStats->GetCountSpeed();
				m_dTaskSizeSpeed += spSubtaskStats->GetSizeSpeed();
			}
		}

		// we're treating each of the items as 4096 bytes object to process
		// to have some balance between items' count and items' size in
		// progress information
		unsigned long long ullProcessed = AssumedFileMinDataSize * m_ullProcessedCount + m_ullProcessedSize;
		unsigned long long ullTotal = AssumedFileMinDataSize * m_ullTotalCount + m_ullTotalSize;

		if (ullTotal != 0)
			m_dCombinedProgress = Math::Div64(ullProcessed, ullTotal);

		m_bCacheFilled = true;
	}

	unsigned long long TTaskStatsSnapshot::GetProcessedCount() const
	{
		if (!m_bCacheFilled)
			CalculateProgressAndSpeeds();

		return m_ullProcessedCount;
	}

	unsigned long long TTaskStatsSnapshot::GetTotalCount() const
	{
		if (!m_bCacheFilled)
			CalculateProgressAndSpeeds();

		return m_ullTotalCount;
	}

	unsigned long long TTaskStatsSnapshot::GetProcessedSize() const
	{
		if (!m_bCacheFilled)
			CalculateProgressAndSpeeds();

		return m_ullProcessedSize;
	}

	unsigned long long TTaskStatsSnapshot::GetTotalSize() const
	{
		if (!m_bCacheFilled)
			CalculateProgressAndSpeeds();

		return m_ullTotalSize;
	}

	double TTaskStatsSnapshot::GetCountSpeed() const
	{
		if (!m_bCacheFilled)
			CalculateProgressAndSpeeds();

		return m_dTaskCountSpeed;
	}

	double TTaskStatsSnapshot::GetSizeSpeed() const
	{
		if (!m_bCacheFilled)
			CalculateProgressAndSpeeds();

		return m_dTaskSizeSpeed;
	}

	double TTaskStatsSnapshot::GetCombinedProgress() const
	{
		if (!m_bCacheFilled)
			CalculateProgressAndSpeeds();

		return m_dCombinedProgress;
	}

	TString TTaskStatsSnapshot::GetSourcePath() const
	{
		return m_strSrcPath;
	}

	void TTaskStatsSnapshot::SetSourcePath(const TString& pathSrc)
	{
		m_strSrcPath = pathSrc;
	}

	double TTaskStatsSnapshot::GetAvgCountSpeed() const
	{
		if (!m_bCacheFilled)
			CalculateProgressAndSpeeds();

		if (m_ullTimeElapsed)
			return Math::Div64(m_ullProcessedCount, m_ullTimeElapsed / 1000.0);

		return 0.0;
	}

	double TTaskStatsSnapshot::GetAvgSizeSpeed() const
	{
		if (!m_bCacheFilled)
			CalculateProgressAndSpeeds();

		if (m_ullTimeElapsed)
			return Math::Div64(m_ullProcessedSize, m_ullTimeElapsed / 1000.0);

		return 0.0;
	}

	unsigned long long TTaskStatsSnapshot::GetEstimatedTotalTime() const
	{
		if (!m_bCacheFilled)
			CalculateProgressAndSpeeds();

		if (m_dCombinedProgress == 0.0)
			return std::numeric_limits<unsigned long long>::max();

		return (unsigned long long)(m_ullTimeElapsed / m_dCombinedProgress);
	}
}
