//! @file ZeroFill.h
//! @author kenjiuno
//! @date Jun 27 2018

#pragma once

#include <Windows.h>

namespace Utils
{
	//! ZeroMemory without CRT
	void ZeroFill(void *ptr, size_t cnt)
	{
		LPBYTE fill = reinterpret_cast<LPBYTE>(ptr);
		LPBYTE fillEnd = fill + cnt;
		for (; fill < fillEnd; fill++)
		{
			*fill = 0;
		}
	}
}
