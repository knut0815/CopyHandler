/***************************************************************************
 *   Copyright (C) 2004-2006 by J�zef Starosczyk                           *
 *   ixen@draknet.sytes.net                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
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
/** \file dmutex.h
 *  \brief Contains mutex class for thread safe access with debugging capabilities.
 *  \see The mutex class.
 */
#ifndef __DMUTEX_H__
#define __DMUTEX_H__

#include "libicpf.h"
#include "gen_types.h"
#include "dumpctx.h"
#include "mutex.h"

BEGIN_ICPF_NAMESPACE

/** \brief Class provides the locking and unlocking capabilities for use with threads.
 *
 *  Class is a simple wrapper over the system related thread locking functions. In linux
 *  those functions are pthread_mutex_* and in windoze the functions related to CRITICAL_SECTION
 *  structure.
 *  This class is very similar to the mutex class, with the difference that it allows logging
 *  of the locking/unlocking allowing easier debugging of the mutexes. Interface is almost
 *  out-of-the-box replaceable with standard mutex class.
 */
class LIBICPF_API d_mutex : public mutex
{
public:
/** \name Construction/destruction */
/**@{*/
	d_mutex(dumpctx* pctx);							///< Constructs an unnamed mutex
	d_mutex(const char_t* pszStr, dumpctx* pctx);	///< Constructs a named mutex
	virtual ~d_mutex();								///< Standard destructor
/**@}*/
	
	// standard locking
/** \name Locking/unlocking */
/**@{*/
	void lock(const char_t* pszFile, ulong_t ulLine, const char_t* pszFunction);		///< Locking with logging
	void unlock(const char_t* pszFile, ulong_t ulLine, const char_t* pszFunction);		///< Unlocking with logging
/**@}*/

private:
	char* m_pszName;		///< Name of the mutex
	dumpctx* m_pContext;	///< Dump context that will receive informations about locking/unlocking
	ulong_t m_ulLockCount;	///< Current lock count
};

END_ICPF_NAMESPACE

#endif
