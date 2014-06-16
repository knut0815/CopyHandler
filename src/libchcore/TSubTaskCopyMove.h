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
/// @file  TSubTaskCopyMove.h
/// @date  2010/09/18
/// @brief Contains declarations of classes responsible for copy and move sub-operation.
// ============================================================================
#ifndef __TSUBTASKCOPYMOVE_H__
#define __TSUBTASKCOPYMOVE_H__

#include "libchcore.h"
#include "TSubTaskBase.h"
#include "DataBuffer.h"

BEGIN_CHCORE_NAMESPACE

class TLocalFilesystemFile;
typedef boost::shared_ptr<TFileInfo> TFileInfoPtr;
struct CUSTOM_COPY_PARAMS;
class TReadBinarySerializer;
class TWriteBinarySerializer;

class TDataBufferManager;
class TSimpleDataBuffer;
class TBufferSizes;

namespace details
{
	///////////////////////////////////////////////////////////////////////////
	// TCopyMoveProgressInfo

	class TCopyMoveProgressInfo : public TSubTaskProgressInfo
	{
	public:
		TCopyMoveProgressInfo();
		virtual ~TCopyMoveProgressInfo();

		virtual void ResetProgress();

		// file being processed
		void SetCurrentIndex(size_t stIndex);
		void IncreaseCurrentIndex();
		size_t GetCurrentIndex() const;

		// part of file being processed
		void SetCurrentFileProcessedSize(unsigned long long ullSize);
		unsigned long long GetCurrentFileProcessedSize() const;
		void IncreaseCurrentFileProcessedSize(unsigned long long ullSizeToAdd);
		void DecreaseCurrentFileProcessedSize(unsigned long long ullSizeToSubtract);

		void Store(const ISerializerRowDataPtr& spRowData) const;
		static void InitLoader(IColumnsDefinition& rColumns);
		void Load(const ISerializerRowReaderPtr& spRowReader);
		bool WasSerialized() const;

	private:
		enum EModifications
		{
			eMod_Added,
			eMod_CurrentIndex,
			eMod_CurrentFileProcessedSize,

			// last item
			eMod_Last
		};

		typedef std::bitset<eMod_Last> Bitset;
		mutable Bitset m_setModifications;

		TSharedModificationTracker<size_t, Bitset, eMod_CurrentIndex> m_stCurrentIndex;
		TSharedModificationTracker<unsigned long long, Bitset, eMod_CurrentFileProcessedSize> m_ullCurrentFileProcessedSize;	// count of bytes processed for current file

		mutable boost::shared_mutex m_lock;
	};
}

class LIBCHCORE_API TSubTaskCopyMove : public TSubTaskBase
{
public:
	TSubTaskCopyMove(TSubTaskContext& tSubTaskContext);

	virtual void Reset();

	virtual ESubOperationResult Exec();
	virtual ESubOperationType GetSubOperationType() const { return eSubOperation_Copying; }

	virtual void Store(const ISerializerPtr& spSerializer) const;
	virtual void Load(const ISerializerPtr& spSerializer);

	virtual TSubTaskProgressInfo& GetProgressInfo() { return m_tProgressInfo; }
	virtual void GetStatsSnapshot(TSubTaskStatsSnapshotPtr& rStats) const;

private:
	TBufferSizes::EBufferType GetBufferIndex(const TBufferSizes& rBufferSizes, const TFileInfoPtr& spFileInfo);
	bool AdjustBufferIfNeeded(TDataBufferManager& rBuffer, TBufferSizes& rBufferSizes);

	ESubOperationResult CustomCopyFileFB(CUSTOM_COPY_PARAMS* pData);

	ESubOperationResult OpenSrcAndDstFilesFB(CUSTOM_COPY_PARAMS* pData, TLocalFilesystemFile &fileSrc, TLocalFilesystemFile &fileDst, bool bNoBuffer, bool& bSkip);

	ESubOperationResult OpenSourceFileFB(TLocalFilesystemFile& fileSrc, const TSmartPath& spPathToOpen, bool bNoBuffering);
	ESubOperationResult OpenDestinationFileFB(TLocalFilesystemFile& fileDst, const TSmartPath& pathDstFile, bool bNoBuffering, const TFileInfoPtr& spSrcFileInfo, unsigned long long& ullSeekTo, bool& bFreshlyCreated);
	ESubOperationResult OpenExistingDestinationFileFB(TLocalFilesystemFile& fileDst, const TSmartPath& pathDstFilePath, bool bNoBuffering);

	ESubOperationResult SetFilePointerFB(TLocalFilesystemFile& file, long long llDistance, const TSmartPath& pathFile, bool& bSkip);
	ESubOperationResult SetEndOfFileFB(TLocalFilesystemFile& file, const TSmartPath& pathFile, bool& bSkip);

	ESubOperationResult ReadFileFB(TLocalFilesystemFile& file, chcore::TSimpleDataBuffer& rBuffer, DWORD dwToRead, DWORD& rdwBytesRead, const TSmartPath& pathFile, bool& bSkip);
	ESubOperationResult WriteFileFB(TLocalFilesystemFile& file, chcore::TSimpleDataBuffer& rBuffer, DWORD dwToWrite, DWORD& rdwBytesWritten, const TSmartPath& pathFile, bool& bSkip);
	ESubOperationResult WriteFileExFB(TLocalFilesystemFile& file, chcore::TSimpleDataBuffer& rBuffer, DWORD dwToWrite, DWORD& rdwBytesWritten, const TSmartPath& pathFile, bool& bSkip, bool bNoBuffer);
	ESubOperationResult CreateDirectoryFB(const TSmartPath& pathDirectory);

	ESubOperationResult CheckForFreeSpaceFB();

private:
#pragma warning(push)
#pragma warning(disable: 4251)
	details::TCopyMoveProgressInfo m_tProgressInfo;
	TSubTaskStatsInfo m_tSubTaskStats;
#pragma warning(pop)
};

END_CHCORE_NAMESPACE

#endif
