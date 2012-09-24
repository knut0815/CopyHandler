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
/// @file  TDataBuffer.cpp
/// @date  2012/03/04
/// @brief Contains class representing buffer for data.
// ============================================================================
#include "stdafx.h"
#include "TDataBuffer.h"

BEGIN_CHCORE_NAMESPACE

namespace
{
	const size_t c_DefaultBufferSize = 65536;
	const size_t c_DefaultPageSize = 1024*1024;
	const size_t c_DefaultMaxMemory = 1024*1024;
}

namespace details
{
	TVirtualAllocMemoryBlock::TVirtualAllocMemoryBlock(size_t stSize, size_t stChunkSize) :
		m_pMemory(NULL),
		m_stMemorySize(0),
		m_stChunkSize(0)
	{
		AllocBlock(stSize, stChunkSize);
	}

	TVirtualAllocMemoryBlock::~TVirtualAllocMemoryBlock()
	{
		try
		{
			FreeBlock();
		}
		catch(...)
		{
		}
	}

	void TVirtualAllocMemoryBlock::GetFreeChunks(std::list<LPVOID>& rListChunks)
	{
		rListChunks.insert(rListChunks.end(), m_setFreeChunks.begin(), m_setFreeChunks.end());
		m_setFreeChunks.clear();
	}

	void TVirtualAllocMemoryBlock::ReleaseChunks(std::list<LPVOID>& rListChunks)
	{
		std::list<LPVOID>::iterator iterList = rListChunks.begin();
		while(iterList != rListChunks.end())
		{
			if(ReleaseChunk(*iterList))
				iterList = rListChunks.erase(iterList);
			else
				++iterList;
		}
	}

	bool TVirtualAllocMemoryBlock::ReleaseChunk(LPVOID pChunk)
	{
		if(IsValidChunk(pChunk))
		{
			m_setFreeChunks.insert(pChunk);
			return true;
		}
		return false;
	}

	size_t TVirtualAllocMemoryBlock::CountOwnChunks(const std::list<LPVOID>& rListChunks)
	{
		std::set<LPVOID> setChunks;
		for(std::list<LPVOID>::const_iterator iterList = rListChunks.begin(); iterList != rListChunks.end(); ++iterList)
		{
			if(IsValidChunk(*iterList))
				setChunks.insert(*iterList);
		}

		return setChunks.size();
	}

	bool TVirtualAllocMemoryBlock::IsChunkOwner(LPVOID pChunk) const
	{
		return(pChunk >= m_pMemory && pChunk < (BYTE*)m_pMemory + m_stMemorySize);
	}

	bool TVirtualAllocMemoryBlock::AreAllChunksFree() const
	{
		if(m_stChunkSize == 0)
			return true;
		return m_setFreeChunks.size() == m_stMemorySize / m_stChunkSize;
	}

	bool TVirtualAllocMemoryBlock::HasFreeChunks() const
	{
		return !m_setFreeChunks.empty();
	}

	void TVirtualAllocMemoryBlock::AllocBlock(size_t stSize, size_t stChunkSize)
	{
		FreeBlock();

		// allocate
		LPVOID pBuffer = VirtualAlloc(NULL, stSize, MEM_COMMIT, PAGE_READWRITE);
		if(!pBuffer)
			THROW_CORE_EXCEPTION(eErr_CannotAllocateMemory);

		m_pMemory = pBuffer;
		m_stMemorySize = stSize;
		m_stChunkSize = stChunkSize;

		// slice the page to buffers
		size_t stSliceCount = m_stMemorySize / m_stChunkSize;
		for(size_t stIndex = 0; stIndex < stSliceCount; ++stIndex)
		{
			LPVOID pSimpleBuffer = (BYTE*)pBuffer + stIndex * stChunkSize;
			m_setFreeChunks.insert(pSimpleBuffer);
		}
	}

	void TVirtualAllocMemoryBlock::FreeBlock()
	{
		if(m_pMemory)
		{
			VirtualFree(m_pMemory, 0, MEM_RELEASE);
			m_stMemorySize = 0;
			m_stChunkSize = 0;
		}
	}

	bool TVirtualAllocMemoryBlock::IsValidChunk(LPVOID pChunk) const
	{
		if(IsChunkOwner(pChunk))
		{
			bool bValidPtr = (((BYTE*)pChunk - (BYTE*)m_pMemory) % m_stChunkSize) == 0;
			_ASSERTE(bValidPtr);
			return bValidPtr;
		}
		else
			return false;
		
	}
}
///////////////////////////////////////////////////////////////////////////////////
// class TSimpleDataBuffer

TSimpleDataBuffer::TSimpleDataBuffer() :
	m_pBuffer(NULL),
	m_pBufferManager(NULL),
	m_stBufferSize(0),
	m_stDataSize(0)
{
}

TSimpleDataBuffer::~TSimpleDataBuffer()
{
	ReleaseBuffer();
}

LPVOID TSimpleDataBuffer::GetBufferPtr()
{
	return m_pBuffer;
}

void TSimpleDataBuffer::ReleaseBuffer()
{
	if(m_pBufferManager && m_pBuffer)
		m_pBufferManager->ReleaseBuffer(*this);
}

void TSimpleDataBuffer::Initialize(TDataBufferManager& rBufferManager, LPVOID pBuffer, size_t stBufferSize)
{
	ReleaseBuffer();

	m_pBufferManager = &rBufferManager;
	m_pBuffer = pBuffer;
	m_stBufferSize = stBufferSize;
}

void TSimpleDataBuffer::SetDataSize(size_t stDataSize)
{
	if(stDataSize > m_stBufferSize)
		THROW_CORE_EXCEPTION(eErr_InvalidArgument);

	m_stDataSize = stDataSize;
}

void TSimpleDataBuffer::CutDataFromBuffer(size_t stCount)
{
	if(stCount >= m_stBufferSize || !m_pBuffer)
		return;	// nothing to do

	memmove(m_pBuffer, (BYTE*)m_pBuffer + stCount, m_stBufferSize - stCount);
}

///////////////////////////////////////////////////////////////////////////////////
// class TDataBufferManager

TDataBufferManager::TDataBufferManager() :
	m_stMaxMemory(0),
	m_stPageSize(0),
	m_stBufferSize(0)
{
}

TDataBufferManager::~TDataBufferManager()
{
	try
	{
		FreeBuffers();
	}
	catch(...)
	{
	}
}

bool TDataBufferManager::CheckBufferConfig(size_t& stMaxMemory, size_t& stPageSize, size_t& stBufferSize)
{
	bool bResult = true;

	// first the user-facing buffer size
	if(stBufferSize == 0)
	{
		stBufferSize = c_DefaultMaxMemory;
		bResult = false;
	}
	else
	{
		size_t stNewSize = RoundUp(stBufferSize, DefaultAllocGranularity);
		if(stBufferSize != stNewSize)
		{
			stBufferSize = stNewSize;
			bResult = false;
		}
	}

	// now the page size
	if(stPageSize == 0)
	{
		stPageSize = std::max(c_DefaultPageSize, RoundUp(c_DefaultPageSize, stBufferSize));
		bResult = false;
	}
	else
	{
		size_t stNewSize = RoundUp(stPageSize, stBufferSize);
		if(stPageSize != stNewSize)
		{
			stPageSize = stNewSize;
			bResult = false;
		}
	}

	if(stMaxMemory == 0)
	{
		stMaxMemory = std::max(c_DefaultMaxMemory, RoundUp(c_DefaultMaxMemory, stPageSize));
		bResult = false;
	}
	else if(stMaxMemory < stPageSize)
	{
		size_t stNewSize = RoundUp(stMaxMemory, stBufferSize);
		if(stNewSize != stMaxMemory)
		{
			bResult = false;
			stMaxMemory = stPageSize;
		}
	}

	return bResult;
}

bool TDataBufferManager::CheckBufferConfig(size_t& stMaxMemory)
{
	size_t stDefaultPageSize = c_DefaultPageSize;
	size_t stDefaultBufferSize = c_DefaultBufferSize;
	return CheckBufferConfig(stMaxMemory, stDefaultPageSize, stDefaultBufferSize);
}

void TDataBufferManager::Initialize(size_t stMaxMemory)
{
	Initialize(stMaxMemory, c_DefaultPageSize, c_DefaultBufferSize);
}

void TDataBufferManager::Initialize(size_t stMaxMemory, size_t stPageSize, size_t stBufferSize)
{
	FreeBuffers();

	// validate input (note that input parameters should already be checked by caller)
	if(!CheckBufferConfig(stMaxMemory, stPageSize, stBufferSize))
		THROW_CORE_EXCEPTION(eErr_InvalidArgument);

	m_stMaxMemory = stMaxMemory;
	m_stPageSize = stPageSize;
	m_stBufferSize = stBufferSize;

	// allocate
	if(!AllocNewPage())
		THROW_CORE_EXCEPTION(eErr_CannotAllocateMemory);
}

bool TDataBufferManager::IsInitialized() const
{
	if(m_stPageSize == 0 || m_stMaxMemory == 0 || m_stBufferSize == 0)
		return false;
	return true;
}

bool TDataBufferManager::CheckResizeSize(size_t& stNewMaxSize)
{
	if(m_stPageSize == 0 || m_stMaxMemory == 0 || m_stBufferSize == 0)
	{
		stNewMaxSize = 0;
		return false;
	}

	size_t stPageSize = m_stPageSize;
	size_t stBufferSize = m_stBufferSize;

	bool bRes = CheckBufferConfig(stNewMaxSize, stPageSize, stBufferSize);
	// make sure the page size and buffer size are unchanged after the call
	_ASSERTE(stPageSize == m_stPageSize && stBufferSize == m_stBufferSize);
	if(stPageSize != m_stPageSize || stBufferSize != m_stBufferSize)
		THROW_CORE_EXCEPTION(eErr_InternalProblem);

	return bRes;
}

void TDataBufferManager::ChangeMaxMemorySize(size_t stNewMaxSize)
{
	if(!CheckResizeSize(stNewMaxSize))
		THROW_CORE_EXCEPTION(eErr_InvalidArgument);

	if(stNewMaxSize >= m_stMaxMemory)
		m_stMaxMemory = stNewMaxSize;
	else
	{
		size_t stCurrentMaxPages = m_stMaxMemory / m_stPageSize;
		size_t stNewMaxPages = stNewMaxSize / m_stPageSize;
		size_t stPagesToFree = stCurrentMaxPages - stNewMaxPages;
		size_t stPagesStillUnallocated = stCurrentMaxPages - m_vVirtualAllocBlocks.size();

		// first free the memory that has not been allocated yet
		if(stPagesStillUnallocated != 0 && stPagesToFree != 0)
		{
			size_t stUnallocatedPagesToFree = std::min(stPagesStillUnallocated, stPagesToFree);
			m_stMaxMemory -= stUnallocatedPagesToFree * m_stPageSize;
			stPagesToFree -= stUnallocatedPagesToFree;
		}

		// is there still too much memory that needs to be freed?
		if(stPagesToFree != 0)
		{
			// free pages that are already allocated
			FreeAllocatedPages(stPagesToFree);
		}
	}
}

bool TDataBufferManager::HasFreeBuffer() const
{
	return !m_listUnusedBuffers.empty();
}

bool TDataBufferManager::CanAllocPage() const
{
	if(!IsInitialized())
		return false;

	size_t stMaxPages = m_stMaxMemory / m_stPageSize;
	return m_vVirtualAllocBlocks.size() < stMaxPages;
}

bool TDataBufferManager::AllocNewPage()
{
	if(!CanAllocPage())
		return false;

	if(!m_vAllocBlocksToFree.empty())
	{
		// re-use the already disposed-of alloc block
		for(std::vector<details::TVirtualAllocMemoryBlockPtr>::iterator iterAllocBlock = m_vAllocBlocksToFree.begin(); iterAllocBlock != m_vAllocBlocksToFree.end(); ++iterAllocBlock)
		{
			details::TVirtualAllocMemoryBlockPtr spAllocBlock(*iterAllocBlock);
			if(spAllocBlock->HasFreeChunks())
			{
				m_vVirtualAllocBlocks.push_back(spAllocBlock);
				m_vAllocBlocksToFree.erase(iterAllocBlock);

				spAllocBlock->GetFreeChunks(m_listUnusedBuffers);

				return true;
			}
		}
	}

	// alloc new block if can't re-use the old one
	details::TVirtualAllocMemoryBlockPtr spAllocBlock(new details::TVirtualAllocMemoryBlock(m_stPageSize, m_stBufferSize));
	m_vVirtualAllocBlocks.push_back(spAllocBlock);
	spAllocBlock->GetFreeChunks(m_listUnusedBuffers);

	return true;
}

void TDataBufferManager::FreeAllocatedPages(size_t stPagesCount)
{
	if(stPagesCount == 0)
		return;

	std::vector<std::pair<details::TVirtualAllocMemoryBlockPtr, size_t> > vFreeBuffers;
	for(std::vector<details::TVirtualAllocMemoryBlockPtr>::iterator iterAllocBlock = m_vVirtualAllocBlocks.begin(); iterAllocBlock != m_vVirtualAllocBlocks.end(); ++iterAllocBlock)
	{
		vFreeBuffers.push_back(std::make_pair(*iterAllocBlock, (*iterAllocBlock)->CountOwnChunks(m_listUnusedBuffers)));
	}

	// sort by the count of free blocks
	std::sort(vFreeBuffers.begin(), vFreeBuffers.end(),
		boost::bind(&std::pair<details::TVirtualAllocMemoryBlockPtr, size_t>::second, _1) > boost::bind(&std::pair<details::TVirtualAllocMemoryBlockPtr, size_t>::second, _2));

	// and free pages with the most free blocks inside
	size_t stPagesToProcess = std::min(stPagesCount, vFreeBuffers.size());
	for(size_t stIndex = 0; stIndex < stPagesToProcess; ++stIndex)
	{
		FreePage(vFreeBuffers[stIndex].first);
	}
}

// function expects arrays to be sorted
void TDataBufferManager::FreePage(const details::TVirtualAllocMemoryBlockPtr& spAllocBlock)
{
	spAllocBlock->ReleaseChunks(m_listUnusedBuffers);
	if(spAllocBlock->AreAllChunksFree())
	{
		std::vector<details::TVirtualAllocMemoryBlockPtr>::iterator iterAllocBlock = std::find(m_vVirtualAllocBlocks.begin(), m_vVirtualAllocBlocks.end(), spAllocBlock);
		if(iterAllocBlock == m_vVirtualAllocBlocks.end())
			THROW_CORE_EXCEPTION(eErr_InternalProblem);
		m_vVirtualAllocBlocks.erase(iterAllocBlock);
	}
	else
	{
		m_vAllocBlocksToFree.push_back(spAllocBlock);
	}
}

bool TDataBufferManager::GetFreeBuffer(TSimpleDataBuffer& rSimpleBuffer)
{
	if(m_listUnusedBuffers.empty())
	{
		// try to alloc new page; we won't get one if max memory would be exceeded or allocation failed
		// this one also populates the buffers list
		if(CanAllocPage())
		{
			if(!AllocNewPage())
				THROW_CORE_EXCEPTION(eErr_CannotAllocateMemory);
		}
	}

	if(!m_listUnusedBuffers.empty())
	{
		LPVOID pBuffer = m_listUnusedBuffers.front();
		m_listUnusedBuffers.pop_front();
		rSimpleBuffer.Initialize(*this, pBuffer, m_stBufferSize);
		return true;
	}

	return false;
}

void TDataBufferManager::ReleaseBuffer(TSimpleDataBuffer& rSimpleBuffer)
{
	if(rSimpleBuffer.m_pBuffer)
	{
		if(m_vAllocBlocksToFree.empty())
			m_listUnusedBuffers.push_back(rSimpleBuffer.m_pBuffer);
		else
		{
			for(std::vector<details::TVirtualAllocMemoryBlockPtr>::iterator iterAllocBlock = m_vAllocBlocksToFree.begin(); iterAllocBlock != m_vAllocBlocksToFree.end(); ++iterAllocBlock)
			{
				const details::TVirtualAllocMemoryBlockPtr& spAllocBlock = (*iterAllocBlock);
				if(spAllocBlock->IsChunkOwner(rSimpleBuffer.m_pBuffer))
				{
					spAllocBlock->ReleaseChunk(rSimpleBuffer.m_pBuffer);
					if(spAllocBlock->AreAllChunksFree())
					{
						m_vAllocBlocksToFree.erase(iterAllocBlock);
						break;
					}
				}
			}
		}
	}
}

void TDataBufferManager::FreeBuffers()
{
	for(std::vector<details::TVirtualAllocMemoryBlockPtr>::iterator iterAllocBlock = m_vVirtualAllocBlocks.begin(); iterAllocBlock != m_vVirtualAllocBlocks.end(); ++iterAllocBlock)
	{
		(*iterAllocBlock)->ReleaseChunks(m_listUnusedBuffers);
		_ASSERTE((*iterAllocBlock)->AreAllChunksFree());	// without throwing on this condition, because there might be a situation that
															// some hanged thread did not release the buffer
	}

	_ASSERTE(m_vAllocBlocksToFree.empty());	// virtual alloc blocks to free should be empty at this point (because all the
											// buffers should be returned to the pool)
	_ASSERTE(m_listUnusedBuffers.empty());	// and all buffers should be returned to the pool by the caller
	if(!m_listUnusedBuffers.empty())
		THROW_CORE_EXCEPTION(eErr_InternalProblem);

	m_vVirtualAllocBlocks.clear();
	m_vAllocBlocksToFree.clear();
	//m_listUnusedBuffers.clear();

	m_stBufferSize = 0;
	m_stPageSize = 0;
	m_stMaxMemory = 0;
}

END_CHCORE_NAMESPACE
