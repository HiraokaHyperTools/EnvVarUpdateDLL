//! @file NsisString.h
//! @author kenjiuno
//! @date Jun 27 2018

#pragma once

#include <nsis/pluginapi.h> // nsis plugin

#include "FixedLenStr.h"

namespace Utils
{
	//! Light-weight auto string suitable input/output for NSIS (without CRT)
	class NsisString : public FixedLenStr
	{
	public:
		//! ctor
		NsisString() : FixedLenStr(g_stringsize)
		{

		}
	};
}
