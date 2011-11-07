/***************************************************************************
 *   Copyright (C) 2001-2008 by J�zef Starosczyk                           *
 *   ixen@copyhandler.com                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License          *
 *   (version 2) as published by the Free Software Foundation;             *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef __FEEDBACKHANDLERBASE_H__
#define __FEEDBACKHANDLERBASE_H__

#include "libchcore.h"
#include "../libicpf/interface.h"

BEGIN_CHCORE_NAMESPACE

enum EFileError
{
	eDeleteError,		///< Problem occured when tried to delete the fs object
	eSeekError,			///< Problem occured when tried to set file pointer
	eResizeError,		///< Problem occured when tried to change size of the fs object
	eReadError,			///< Problem occured when tried to read data from file
	eWriteError,		///< Problem occured when tried to write data to a file
	eFastMoveError,		///< Problem occured when tried to perform fast move operation (that does not involve copying contents)
	eCreateError		///< Problem occured when tried to create the fs object
};

struct FEEDBACK_ALREADYEXISTS
{
	TFileInfoPtr spSrcFileInfo;
	TFileInfoPtr spDstFileInfo;
};

struct FEEDBACK_FILEERROR
{
	const wchar_t* pszSrcPath;
	const wchar_t* pszDstPath;
	EFileError eFileError;			// error type
	ulong_t ulError;				// system error
};

struct FEEDBACK_NOTENOUGHSPACE
{
	ull_t ullRequiredSize;
	const wchar_t* pszSrcPath;
	const wchar_t* pszDstPath;
};

class IFeedbackHandler : public icpf::IInterface
{
public:
	enum EFeedbackType
	{
		eFT_Unknown = 0,
		// requests for use feedback
		eFT_FileAlreadyExists,
		eFT_FileError,
		eFT_NotEnoughSpace,
		// notifications
		eFT_OperationFinished,	///< Task has finished processing
		eFT_OperationError,		///< Error encountered while processing task
		eFT_LastType
	};

	enum EFeedbackResult
	{
		eResult_Unknown = 0,
		eResult_Overwrite,
		eResult_CopyRest,
		eResult_Skip,
		eResult_Cancel,
		eResult_Pause,
		eResult_Retry,
		eResult_Ignore
	};

public:
	virtual ull_t RequestFeedback(ull_t ullFeedbackID, ptr_t pFeedbackParam) = 0;
};

class IFeedbackHandlerFactory : public icpf::IInterface
{
public:
	virtual IFeedbackHandler* Create() = 0;
};

END_CHCORE_NAMESPACE

#endif
