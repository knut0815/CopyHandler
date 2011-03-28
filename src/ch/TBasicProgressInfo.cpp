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
/// @file  TBasicProgressInfo.cpp
/// @date  2011/03/28
/// @brief Contains implementation of class recording progress info.
// ============================================================================
#include "stdafx.h"
#include "TBasicProgressInfo.h"


TTaskBasicProgressInfo::TTaskBasicProgressInfo() :
m_stCurrentIndex(0),
m_ullCurrentFileProcessedSize(0),
m_stSubOperationIndex(0)
{
}

TTaskBasicProgressInfo::~TTaskBasicProgressInfo()
{
}

void TTaskBasicProgressInfo::SetCurrentIndex(size_t stIndex)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_stCurrentIndex = stIndex;
	m_ullCurrentFileProcessedSize = 0;
}

void TTaskBasicProgressInfo::IncreaseCurrentIndex()
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	++m_stCurrentIndex;
	m_ullCurrentFileProcessedSize = 0;
}

size_t TTaskBasicProgressInfo::GetCurrentIndex() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_stCurrentIndex;
}

void TTaskBasicProgressInfo::SetCurrentFileProcessedSize(unsigned long long ullSize)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_ullCurrentFileProcessedSize = ullSize;
}

unsigned long long TTaskBasicProgressInfo::GetCurrentFileProcessedSize() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_ullCurrentFileProcessedSize;
}

void TTaskBasicProgressInfo::IncreaseCurrentFileProcessedSize(unsigned long long ullSizeToAdd)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_ullCurrentFileProcessedSize += ullSizeToAdd;
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
