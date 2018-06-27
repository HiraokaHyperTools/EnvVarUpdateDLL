//! @file LongString.h
//! @author kenjiuno
//! @date Jun 27 2018

#pragma once

#include "FixedLenStr.h"

namespace Utils
{
	//! A 32K length string.
	/*!
		@remarks
		How long LongString is?
		@li The input/output limitation of <a href="https://msdn.microsoft.com/en-us/library/windows/desktop/ms724265(v=vs.85).aspx">ExpandEnvironmentStrings</a> is good example.
		@li MSDN says: "buffers is limited to 32K."
	*/
	class LongString : public FixedLenStr
	{
	public:
		//! ctor
		LongString() : FixedLenStr(32768)
		{

		}
	};
};
