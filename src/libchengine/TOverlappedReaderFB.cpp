// ============================================================================
//  Copyright (C) 2001-2016 by Jozef Starosczyk
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
#include "stdafx.h"
#include "TOverlappedReaderFB.h"
#include "TFileInfo.h"
#include "TEventGuard.h"
#include "../libchcore/TWorkerThreadController.h"

using namespace chcore;

namespace chengine
{
	TOverlappedReaderFB::TOverlappedReaderFB(const IFilesystemPtr& spFilesystem,
		const IFeedbackHandlerPtr& spFeedbackHandler,
		TWorkerThreadController& rThreadController,
		const TSubTaskStatsInfoPtr& spStats,
		const TFileInfoPtr& spSrcFileInfo,
		const logger::TLogFileDataPtr& spLogFileData,
		const TBufferListPtr& spEmptyBuffers,
		const TOverlappedProcessorRangePtr& spDataRange,
		DWORD dwChunkSize,
		size_t stMaxOtfBuffers, size_t stMaxReadAheadBuffers,
		bool bNoBuffering,
		bool bProtectReadOnlyFiles) :
		m_counterOnTheFly(),
		m_spReader(std::make_shared<TOverlappedReader>(spLogFileData, spEmptyBuffers, spDataRange, dwChunkSize, stMaxOtfBuffers, stMaxReadAheadBuffers, m_counterOnTheFly.GetSharedCount())),
		m_eventReadingFinished(true, false),
		m_eventProcessingFinished(true, false),
		m_eventLocalKill(true, false),
		m_spFilesystem(spFilesystem),
		m_spSrcFileInfo(spSrcFileInfo),
		m_spSrcFile(),
		m_spStats(spStats),
		m_rThreadController(rThreadController),
		m_spLog(logger::MakeLogger(spLogFileData, L"File-Reader"))
	{
		if(!spFeedbackHandler)
			throw TCoreException(eErr_InvalidArgument, L"spFeedbackHandler is NULL", LOCATION);
		if(!spFilesystem)
			throw TCoreException(eErr_InvalidArgument, L"spFilesystem is NULL", LOCATION);
		if(!spStats)
			throw TCoreException(eErr_InvalidArgument, L"spStats is NULL", LOCATION);
		if(!spSrcFileInfo)
			throw TCoreException(eErr_InvalidArgument, L"spSrcFileInfo is NULL", LOCATION);
		if(!spLogFileData)
			throw TCoreException(eErr_InvalidArgument, L"spLogFileData is NULL", LOCATION);
		if(!spEmptyBuffers)
			throw TCoreException(eErr_InvalidArgument, L"spEmptyBuffers is NULL", LOCATION);
		if(!spDataRange)
			throw TCoreException(eErr_InvalidArgument, L"spDataRange is NULL", LOCATION);

		IFilesystemFilePtr fileSrc = m_spFilesystem->CreateFileObject(IFilesystemFile::eMode_Read, m_spSrcFileInfo->GetFullFilePath(), bNoBuffering, bProtectReadOnlyFiles);
		m_spSrcFile = std::make_shared<TFilesystemFileFeedbackWrapper>(fileSrc, spFeedbackHandler, spLogFileData, rThreadController, spFilesystem);
	}

	TOverlappedReaderFB::~TOverlappedReaderFB()
	{
	}

	TSubTaskBase::ESubOperationResult TOverlappedReaderFB::Start()
	{
		TSubTaskBase::ESubOperationResult eResult = UpdateFileStats();
		return eResult;
	}

	TOrderedBufferQueuePtr TOverlappedReaderFB::GetFinishedQueue() const
	{
		return m_spReader->GetFinishedQueue();
	}

	void TOverlappedReaderFB::StartThreaded()
	{
		m_eventProcessingFinished.ResetEvent();
		TEventGuard guardProcessingFinished(m_eventProcessingFinished, true);
		TEvent eventNonSignaled(true, false);

		m_eThreadResult = TSubTaskBase::eSubResult_Continue;

		// read data from file to buffer
		// NOTE: order is critical here:
		// - write finished is first, so that all the data that were already queued to be written, will be written and accounted for (in stats)
		// - kill request is second, so that we can stop processing as soon as all the data is written to destination location;
		//      that also means that we don't want to queue reads or writes anymore - all the data that were read until now, will be lost
		// - write possible - we're prioritizing write queuing here to empty buffers as soon as possible
		// - read possible - lowest priority - if we don't have anything to write or finalize , then read another part of source data
		enum { eKillThread, eLocalKill, eReadFailed, eReadPossible, eDataSourceFinished };

		std::vector<HANDLE> vHandles = {
			m_rThreadController.GetKillThreadHandle(),
			m_eventLocalKill.Handle(),
			m_spReader->GetEventReadFailedHandle(),
			m_spReader->GetEventReadPossibleHandle(),
			m_spReader->GetEventDataSourceFinishedHandle()
		};

		bool bDataSourceFinished = false;

		try
		{
			while(m_eThreadResult == TSubTaskBase::eSubResult_Continue && !bDataSourceFinished)
			{
				DWORD dwResult = WaitForMultipleObjectsEx(boost::numeric_cast<DWORD>(vHandles.size()), vHandles.data(), false, INFINITE, true);
				switch(dwResult)
				{
				case STATUS_USER_APC:
					break;

				case WAIT_OBJECT_0 + eKillThread:
				case WAIT_OBJECT_0 + eLocalKill:
					m_eThreadResult = TSubTaskBase::eSubResult_KillRequest;
					break;

				case WAIT_OBJECT_0 + eReadPossible:
					m_eThreadResult = OnReadPossible();
					break;

				case WAIT_OBJECT_0 + eReadFailed:
					m_eThreadResult = OnReadFailed();
					break;

				case WAIT_OBJECT_0 + eDataSourceFinished:
					bDataSourceFinished = true;
					m_eThreadResult = TSubTaskBase::eSubResult_Continue;
					break;

				default:
					throw TCoreException(eErr_UnhandledCase, L"Unknown result from async waiting function", LOCATION);
				}
			}
		}
		catch(const std::exception&)
		{
			m_eThreadResult = TSubTaskBase::eSubResult_Error;
		}

		m_spSrcFile->CancelIo();

		WaitForOnTheFlyBuffers();
		ClearQueues();

		LOG_DEBUG(m_spLog) << L"Reader stopping processing. Max on-the-fly requests: " << m_counterOnTheFly.GetMaxUsed();

		if(m_eThreadResult == TSubTaskBase::eSubResult_Continue && bDataSourceFinished)
			m_eventReadingFinished.SetEvent();
	}

	TSubTaskBase::ESubOperationResult TOverlappedReaderFB::StopThreaded()
	{
		m_eventLocalKill.SetEvent();

		DWORD dwResult = WaitForSingleObjectEx(m_eventProcessingFinished.Handle(), INFINITE, FALSE);
		if(dwResult != WAIT_OBJECT_0)
			throw TCoreException(eErr_InternalProblem, L"Failed to wait writer processing to finish", LOCATION);

		return m_eThreadResult;
	}

	void TOverlappedReaderFB::WaitForOnTheFlyBuffers()
	{
		bool bStop = false;
		do
		{
			DWORD dwResult = WaitForSingleObjectEx(m_counterOnTheFly.GetEventHandle(), INFINITE, TRUE);
			switch(dwResult)
			{
			case STATUS_USER_APC:
				break;

			case WAIT_OBJECT_0:
				bStop = true;
				break;

			default:
				throw TCoreException(eErr_UnhandledCase, L"Unknown result from async waiting function", LOCATION);
			}
		}
		while(!bStop);
	}

	void TOverlappedReaderFB::ClearQueues()
	{
		m_spReader->ClearBuffers();
	}

	TSubTaskBase::ESubOperationResult TOverlappedReaderFB::UpdateFileStats()
	{
		// update the source file size (it might differ from the time this file was originally scanned).
		// NOTE: this kind of update could be also done when copying chunks of data beyond the original end-of-file,
		//       but it would require frequent total size updates and thus - serializations).
		// NOTE2: the by-chunk corrections of stats are still applied when copying to ensure even further size
		//        matching; this update however still allows for better serialization management.
		file_size_t fsOldSize = m_spSrcFileInfo->GetLength64();
		file_size_t fsNewSize = 0;

		TSubTaskBase::ESubOperationResult eResult = m_spSrcFile->GetFileSize(fsNewSize);
		if(eResult != TSubTaskBase::eSubResult_Continue)
			return eResult;

		if(fsNewSize != fsOldSize)
		{
			m_spStats->AdjustTotalSize(fsOldSize, fsNewSize);
			m_spSrcFileInfo->SetLength64(fsNewSize);
		}

		return eResult;
	}

	HANDLE TOverlappedReaderFB::GetEventReadingFinishedHandle() const
	{
		return m_eventReadingFinished.Handle();
	}

	HANDLE TOverlappedReaderFB::GetEventProcessingFinishedHandle() const
	{
		return m_eventProcessingFinished.Handle();
	}

	void TOverlappedReaderFB::QueueProcessedBuffer(TOverlappedDataBuffer* pBuffer)
	{
		if(!pBuffer)
			throw TCoreException(eErr_InvalidArgument, L"pBuffer is NULL", LOCATION);

		if(pBuffer->HasError())
			m_spReader->AddFailedReadBuffer(pBuffer);
		else
			m_spReader->AddFinishedReadBuffer(pBuffer);

		m_counterOnTheFly.Decrease();
	}

	TSubTaskBase::ESubOperationResult TOverlappedReaderFB::OnReadPossible()
	{
		TOverlappedDataBuffer* pBuffer = m_spReader->GetEmptyBuffer();
		if(!pBuffer)
			return TSubTaskBase::eSubResult_Continue;

		m_counterOnTheFly.Increase();

		pBuffer->SetParam(this);
		TSubTaskBase::ESubOperationResult eResult = m_spSrcFile->ReadFileFB(*pBuffer);
		if(eResult != TSubTaskBase::eSubResult_Continue)
		{
			m_spReader->AddEmptyBuffer(pBuffer);
			m_counterOnTheFly.Decrease();
		}

		return eResult;
	}

	TSubTaskBase::ESubOperationResult TOverlappedReaderFB::OnReadFailed()
	{
		TOverlappedDataBuffer* pBuffer = m_spReader->GetFailedReadBuffer();
		if(!pBuffer)
			throw TCoreException(eErr_InternalProblem, L"Cannot retrieve failed read buffer", LOCATION);

		// read error encountered - handle it
		TSubTaskBase::ESubOperationResult eResult = m_spSrcFile->HandleReadError(*pBuffer);
		if(eResult == TSubTaskBase::eSubResult_Retry)
		{
			m_spSrcFile->Close();
			m_spReader->AddRetryBuffer(pBuffer);
			eResult = TSubTaskBase::eSubResult_Continue;
		}
		else if(eResult != TSubTaskBase::eSubResult_Continue)
			m_spReader->AddEmptyBuffer(pBuffer);

		return eResult;
	}
}
