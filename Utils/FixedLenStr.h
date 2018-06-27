//! @file FixedLenStr.h
//! @author kenjiuno
//! @date Jun 27 2018

#pragma once

#include "ZeroFill.h"

namespace Utils
{
	//! A fixed length string suitable for NSIS input/output string
	/*!
		@remarks Represents a single null terminated string.
	 */
	class FixedLenStr
	{
	public:
		//! Allocated buffer at heap, having null barrier char at end.
		LPTSTR msgbuf;

		//! max position in TCHAR count (excluding one null barrier char)
		size_t maxPos;

	protected:
		//! ctor with 
		FixedLenStr(size_t maxCharCount) : maxPos(0)
		{
			msgbuf = (LPTSTR)GlobalAlloc(GPTR, (maxCharCount + 1) * sizeof(TCHAR));

			if (msgbuf != nullptr)
			{
				maxPos = maxCharCount;
			}
		}

	public:
		//! Assign from external string
		bool AssignString(const FixedLenStr &source)
		{
			if (true
				&& msgbuf != nullptr
				&& source.msgbuf != nullptr
				&& source.StringCharCount() < BufferCharCount()
				)
			{
				Clear();
				lstrcpy(msgbuf, source.msgbuf);
				return true;
			}
			return false;
		}

		//! Assign from external string
		//! @param charCount -1 is invalid.
		bool AssignString(LPCTSTR source, size_t offset, size_t charCount)
		{
			if (msgbuf != nullptr && charCount < BufferCharCount(true))
			{
				Clear();
				lstrcpyn(msgbuf, source + offset, charCount + 1);
				return true;
			}
			return false;
		}

		//! Append string if not empty.
		bool AppendStringIfNotEmpty(LPCTSTR text)
		{
			if (StringCharCount() != 0)
			{
				return AppendString(text);
			}
			return true;
		}

		//! Append single null terminated string.
		bool AppendString(LPCTSTR text)
		{
			const size_t textLen = lstrlen(text);
			LPTSTR appendAt = msgbuf + lstrlen(msgbuf);
			LPTSTR endAt = msgbuf + maxPos;
			if (appendAt + textLen < endAt)
			{
				lstrcpy(appendAt, text);
				return true;
			}
			return false;
		}

		//! Clear string buffer.
		void Clear()
		{
			ZeroFill(msgbuf, BufferBytesLength(true));
		}

		//! Return written string length in TCHAR count.
		size_t StringCharCount() const
		{
			return (msgbuf == nullptr) ? 0 : lstrlen(msgbuf);
		}

		//! Return written string length in bytes.
		size_t StringBytesLength() const
		{
			return sizeof(TCHAR) * StringCharCount();
		}

		//! Obtain buffer size in TCHAR count.
		size_t BufferCharCount(bool includeNullBarrier = false) const
		{
			return (msgbuf == nullptr) ? 0 : (maxPos + (includeNullBarrier ? 1 : 0));
		}

		//! Obtain buffer byte size
		size_t BufferBytesLength(bool includeNullBarrier = false) const
		{
			return sizeof(TCHAR) * BufferCharCount(includeNullBarrier);
		}

		//! NSIS pushstring
		void Push()
		{
			pushstring(msgbuf);
		}

		//! NSIS popstring
		bool Pop()
		{
			if (msgbuf != nullptr)
			{
				if (popstring(msgbuf) == 0)
				{
					return true;
				}
			}
			return false;
		}

		//! LPTSTR cast for NSIS support functions.
		operator LPTSTR()
		{
			return msgbuf;
		}

		//! LPCTSTR cast for NSIS support functions.
		operator LPCTSTR() const
		{
			return msgbuf;
		}

		//! _tcscmpi compatible one
		int CompareToIgnoreCase(LPCTSTR psz) const
		{
			return lstrcmpi(msgbuf, psz);
		}

		//! dtor
		~FixedLenStr()
		{
			if (msgbuf != nullptr)
			{
				GlobalFree(msgbuf);
			}
		}

		//! Split path to tokens separated by delimiter.
		/*!
			@param nextPos 0 for initial start.
		 */
		bool GetToken(TCHAR delim, size_t &nextPos, FixedLenStr &outStr)
		{
			if (msgbuf != nullptr)
			{
				const size_t maxLen = lstrlen(msgbuf);
				const size_t lastPos = nextPos;
				if (maxLen <= nextPos)
				{
					return false;
				}
				while (true)
				{
					TCHAR oneChar = msgbuf[nextPos];
					if (oneChar == delim || oneChar == 0 || maxLen < nextPos)
					{
						bool result = outStr.AssignString(msgbuf, lastPos, nextPos - lastPos);
						nextPos++;
						return result;
					}
					nextPos++;
				}
			}
			return false;
		}
	};
}
