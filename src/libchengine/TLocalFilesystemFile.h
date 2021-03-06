// ============================================================================
//  Copyright (C) 2001-2015 by Jozef Starosczyk
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
#ifndef __TLOCALFILESYSTEMFILE_H__
#define __TLOCALFILESYSTEMFILE_H__

#include "TOverlappedDataBuffer.h"
#include "IFilesystemFile.h"
#include "../liblogger/TLogger.h"

namespace chengine
{
	class LIBCHENGINE_API TLocalFilesystemFile : public IFilesystemFile
	{
	public:
		virtual ~TLocalFilesystemFile();

		void Truncate(file_size_t fsNewSize) override;

		void ReadFile(TOverlappedDataBuffer& rBuffer) override;
		void WriteFile(TOverlappedDataBuffer& rBuffer) override;
		void FinalizeFile(TOverlappedDataBuffer& rBuffer) override;

		void CancelIo() override;

		bool IsOpen() const override;
		bool IsFreshlyCreated() override;

		file_size_t GetFileSize() override;
		void GetFileInfo(TFileInfo& tFileInfo) override;

		chcore::TSmartPath GetFilePath() const override;

		void Close() override;
		file_size_t GetSeekPositionForResume(file_size_t fsLastAvailablePosition) override;

		void SetBasicInfo(DWORD dwAttributes, const chcore::TFileTime& ftCreationTime, const chcore::TFileTime& ftLastAccessTime, const chcore::TFileTime& ftLastWriteTime) override;

	private:
		TLocalFilesystemFile(EOpenMode eMode, const chcore::TSmartPath& pathFile, bool bNoBuffering, bool bProtectReadOnlyFiles, const logger::TLogFileDataPtr& spLogFileData);

		void EnsureOpen();

		void OpenFileForReading();
		void OpenFileForWriting();

		DWORD GetFlagsAndAttributes(bool bNoBuffering) const;

		void InternalClose();

		std::wstring GetFileInfoForLog(bool bNoBuffering) const;

	private:
		chcore::TSmartPath m_pathFile;
		HANDLE m_hFile = INVALID_HANDLE_VALUE;
		EOpenMode m_eMode = eMode_Read;
		bool m_bProtectReadOnlyFiles = false;
		bool m_bNoBuffering = false;

		bool m_bFreshlyCreated = false;

#pragma warning(push)
#pragma warning(disable: 4251)
		logger::TLoggerPtr m_spLog;
#pragma warning(pop)

		friend class TLocalFilesystem;
	};
}

#endif
