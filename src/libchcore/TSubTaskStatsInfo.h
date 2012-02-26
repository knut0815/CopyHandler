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
/// @file  TSubTaskStatsInfo.h
/// @date  2012/02/22
/// @brief Contains declaration of class responsible for tracking stats for subtasks.
// ============================================================================
#ifndef __TSUBTASKSTATSINFO_H__
#define __TSUBTASKSTATSINFO_H__

#include "libchcore.h"
#include "TString.h"

BEGIN_CHCORE_NAMESPACE

class TSubTaskStatsInfo;
class TSubTaskStatsSnapshot;

// class used to guard scope of the subtask processing (
class TSubTaskProcessingGuard
{
public:
	TSubTaskProcessingGuard(TSubTaskStatsInfo& rStats);
	~TSubTaskProcessingGuard();

private:
	TSubTaskProcessingGuard(const TSubTaskProcessingGuard&);
	TSubTaskProcessingGuard& operator=(const TSubTaskProcessingGuard&);

private:
	TSubTaskStatsInfo& m_rStats;
};

class TSubTaskStatsInfo
{
public:
	TSubTaskStatsInfo();

	void Clear();

	void GetSnapshot(TSubTaskStatsSnapshot& rStatsSnapshot) const;

	void IncreaseProcessedCount(size_t stIncreaseBy);
	void SetProcessedCount(size_t stIndex);

	void SetTotalCount(size_t stCount);

	// size stats
	void IncreaseProcessedSize(unsigned long long ullIncreaseBy);
	void SetProcessedSize(unsigned long long ullProcessedSize);

	void SetTotalSize(unsigned long long ullTotalSize);

	// buffer index
	void SetCurrentBufferIndex(int iCurrentIndex);

	// current path
	void SetCurrentPath(const TString& strPath);

	// time
	void SetTimeElapsed(time_t timeElapsed);

private:
	TSubTaskStatsInfo(const TSubTaskStatsInfo&);
	TSubTaskStatsInfo& operator=(const TSubTaskStatsInfo&);

	// is running?
	void MarkAsRunning();
	void MarkAsNotRunning();

	// time tracking
	void EnableTimeTracking();
	void DisableTimeTracking();
	void UpdateTime(boost::upgrade_lock<boost::shared_mutex>& lock) const;

	// calculates progress in percent
	double CalculateProgressInPercent(boost::upgrade_lock<boost::shared_mutex>& lock) const;

private:
	bool m_bSubTaskIsRunning;

	unsigned long long m_ullTotalSize;
	unsigned long long m_ullProcessedSize;

	size_t m_stTotalCount;
	size_t m_stProcessedCount;

	int m_iCurrentBufferIndex;

	TString m_strCurrentPath;		// currently processed path

	mutable time_t m_timeElapsed;
	mutable time_t m_timeLast;

#pragma warning(push)
#pragma warning(disable: 4251)
	mutable boost::shared_mutex m_lock;
#pragma warning(pop)

	friend class TSubTaskProcessingGuard;
};

END_CHCORE_NAMESPACE

#endif
