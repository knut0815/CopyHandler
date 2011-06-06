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
/// @file  TTaskOperationPlan.cpp
/// @date  2010/09/18
/// @brief File contains class handling planning of the entire operation.
// ============================================================================
#ifndef __TTASKOPERATIONPLAN_H__
#define __TTASKOPERATIONPLAN_H__

#include "libchcore.h"
#include <boost\serialization\split_member.hpp>
#include "EOperationTypes.h"

BEGIN_CHCORE_NAMESPACE

enum ESubOperationType
{
	eSubOperation_None,
	eSubOperation_Scanning,
	eSubOperation_Copying,
	eSubOperation_Deleting,

	// add new operation types before this one
	eSubOperation_Max
};

///////////////////////////////////////////////////////////////////////////
// TOperationPlan

// class describes the sub-operations to be performed
class LIBCHCORE_API TOperationPlan
{
public:
	TOperationPlan();
	TOperationPlan(const TOperationPlan& rSrc);
	~TOperationPlan();

	TOperationPlan& operator=(const TOperationPlan& rSrc);

	void SetOperationType(EOperationType eOperation);
	EOperationType GetOperationType() const;

	size_t GetSubOperationsCount() const;
	ESubOperationType GetSubOperationAt(size_t stIndex) const;
	double GetEstimatedTimeAt(size_t stIndex) const;

	template<class Archive>
	void load(Archive& ar, unsigned int /*uiVersion*/);

	template<class Archive>
	void save(Archive& ar, unsigned int /*uiVersion*/) const;

	BOOST_SERIALIZATION_SPLIT_MEMBER();

private:
	EOperationType m_eOperation;
#pragma warning(push)
#pragma warning(disable: 4251)
	std::vector<std::pair<ESubOperationType, double> > m_vSubOperations;	///< Vector of sub-task type and estimated part in the entire task time

	mutable boost::shared_mutex m_lock;
#pragma warning(pop)
};

template<class Archive>
void TOperationPlan::load(Archive& ar, unsigned int /*uiVersion*/)
{
	EOperationType eOperation = eOperation_None;
	ar >> eOperation;
	SetOperationType(eOperation);
}

template<class Archive>
void TOperationPlan::save(Archive& ar, unsigned int /*uiVersion*/) const
{
	ar << GetOperationType();
}

END_CHCORE_NAMESPACE

#endif
