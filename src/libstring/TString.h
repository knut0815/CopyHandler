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
/// @file  TString.h
/// @date  2011/06/05
/// @brief Contains declaration of TString class.
// ============================================================================
#ifndef __TSTRING_H__
#define __TSTRING_H__

#include "libstring.h"
#include <ostream>

namespace string
{
	class TStringArray;

	///////////////////////////////////////////////////////////////
	// TString manipulation class
	/** \brief String manipulation class
	 *
	 *  Class allows user to manipulate TString objects (either standard ANSI char_t*
	 *  based strings or UNICODE ones - wchar_t related) with simple functions and
	 *  operators.
	 */
	class LIBSTRING_API TString
	{
	public:
		TString();
		// cppcheck-suppress noExplicitConstructor
		TString(const wchar_t* pszStr);
		TString(const wchar_t* pszStart, const wchar_t* pszEnd, size_t stMaxStringSize = DefaultMaxStringSize);
		TString(const wchar_t* pszStart, size_t stCount);

		TString(const TString& str);
		TString(TString&& str) noexcept;

		~TString();

		// assignment
		TString& operator=(const TString& src);
		TString& operator=(TString&& src) noexcept;

		const TString& operator=(const wchar_t* pszSrc);

		TString operator+(const TString& src) const;
		const TString& operator+=(const TString& src);

		TString operator+(const wchar_t* pszSrc) const;
		const TString& operator+=(const wchar_t* pszSrc);

		bool operator<(const wchar_t* psz) const { return Compare(psz) < 0; }
		bool operator<=(const wchar_t* psz) const { return Compare(psz) <= 0; }
		bool operator==(const wchar_t* psz) const { return Compare(psz) == 0; }
		bool operator>=(const wchar_t* psz) const { return Compare(psz) >= 0; }
		bool operator>(const wchar_t* psz) const { return Compare(psz) > 0; }
		bool operator!=(const wchar_t* psz) const { return Compare(psz) != 0; }

		bool operator<(const TString& str) const { return Compare(str) < 0; }
		bool operator<=(const TString& str) const { return Compare(str) <= 0; }
		bool operator==(const TString& str) const { return Compare(str) == 0; }
		bool operator>=(const TString& str) const { return Compare(str) >= 0; }
		bool operator>(const TString& str) const { return Compare(str) >= 0; }
		bool operator!=(const TString& str) const { return Compare(str) != 0; }

		// appends the given TString to this
		void Append(const wchar_t* pszSrc);		///< Appends an unicode TString to the TString object
		void Append(const TString& src);			///< Appends a TString object to another TString object

		TString Left(size_t tLen) const;		///< Returns TString with the Left part of a source TString
		TString Right(size_t tLen) const;		///< Returns TString with the Right part of a source TString
		TString Mid(size_t tStart, size_t tLen = (size_t)-1) const;	///< Returns TString with the middle part of a source TString
		TString MidRange(size_t tStart, size_t stAfterEndPos) const;	///< Returns TString with the middle part of a source TString

		void LeftSelf(size_t tLen);	///< Makes this TString it's Left part
		void RightSelf(size_t tLen);	///< Makes this TString it's Right part
		void MidSelf(size_t tStart, size_t tLen = (size_t)-1);	///< Makes this TString it's middle part

		void TrimRightSelf(const wchar_t* pszElements);

		bool Delete(size_t stIndex, size_t stCount);

		void Split(const wchar_t* pszSeparators, TStringArray& rStrings) const;

		// compare operations
		int Compare(const wchar_t* psz) const;	///< Comparison of this TString object with a given unicode TString
		int Compare(const TString& str) const;	///< Comparison of this TString object with another TString object

		int CompareNoCase(const wchar_t* psz) const;	///< Comparison (case insensitive) of this TString object with a given unicode TString
		int CompareNoCase(const TString& str) const;	///< Comparison (case insensitive) of this TString object with another TString object

		bool StartsWith(const wchar_t* pszText) const;
		bool StartsWithNoCase(const wchar_t* pszText) const;

		bool EndsWith(const wchar_t* pszText) const;
		bool EndsWithNoCase(const wchar_t* pszText) const;

		size_t FindFirstOf(const wchar_t* pszChars, size_t stStartFromPos = 0) const;
		size_t FindLastOf(const wchar_t* pszChars) const;

		size_t Find(const wchar_t* pszText, size_t stStartPos = 0);
		void Replace(const wchar_t* pszWhat, const wchar_t* pszWithWhat);

		bool GetAt(size_t tPos, wchar_t& wch) const;						///< Gets a character at a specified position
		wchar_t GetAt(size_t tPos) const;

		const wchar_t* c_str() const;

		wchar_t* GetBuffer(size_t tMinSize);		///< Gives user access to the unicode internal buffer
		void ReleaseBuffer();						///< Releases the buffer get from get_bufferx functions
		void ReleaseBufferSetLength(size_t tSize);

		size_t GetLength() const;	///< Returns the GetLength of this TString in chars

		void Clear();			///< Clear the contents of the TString object

		bool IsEmpty() const;	///< Returns true if the TString is empty

	protected:
		void SetString(const wchar_t* pszStart, size_t stCount);
		void SetString(const wchar_t* pszString);
		void Reserve(size_t stLen);

	protected:
		wchar_t* m_pszData;			// contains the real string inside
		size_t m_stBufferSize;		// allocated string buffer size

	public:
		static const size_t npos = (size_t)-1;
		static const size_t DefaultMaxStringSize = 65536;
	};

	inline std::wostream& operator<<(std::wostream& os, const TString& rString)
	{
		return os << std::wstring(rString.c_str());
	}
}

LIBSTRING_API string::TString operator+(const wchar_t* pszString, const string::TString& str);

#endif
