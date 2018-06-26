/**
 * @file EnvVarUpdateDLL.cpp
 * @brief DLL version of EnvVarUpdate
 * @author kenjiuno
 * @date Jun 26 2018
 */

#include <windows.h>
#include <nsis/pluginapi.h> // nsis plugin

HWND g_hwndParent;

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

//! Light-weight auto string suitable input/output for NSIS (without CRT)
class NsisString
{
public:
	//! Allocated buffer at heap
	LPTSTR msgbuf;

public:
	//! ctor
	NsisString()
	{
		msgbuf = (LPTSTR)GlobalAlloc(GPTR, (g_stringsize + 1) * sizeof(TCHAR));
	}

	//! ctor
	NsisString(LPCTSTR source, size_t offset, size_t charaCount)
	{
		msgbuf = (LPTSTR)GlobalAlloc(GPTR, (g_stringsize + 1) * sizeof(TCHAR));

		AssignString(source, offset, charaCount);
	}

	//! Assign from external string
	bool AssignString(const NsisString &source)
	{
		if (msgbuf != nullptr && source.msgbuf != nullptr && source.StringCharaCount() < BufferCharaCount())
		{
			FillZero();
			lstrcpy(msgbuf, source.msgbuf);
			return true;
		}
		return false;
	}

	//! Assign from external string
	bool AssignString(LPCTSTR source, size_t offset, size_t charaCount)
	{
		if (msgbuf != nullptr && charaCount < BufferCharaCount(true))
		{
			FillZero();
			lstrcpyn(msgbuf, source + offset, charaCount + 1);
			return true;
		}
		return false;
	}

	//! Append string if not empty.
	bool AppendStringIfNotEmpty(LPCTSTR text)
	{
		if (StringCharaCount() != 0)
		{
			return AppendString(text);
		}
		return true;
	}

	//! Append single null terminated string.
	bool AppendString(LPCTSTR text)
	{
		if (StringCharaCount() + lstrlen(text) < BufferCharaCount())
		{
			lstrcat(msgbuf, text);
			return true;
		}
		return false;
	}

	//! Fill zero before writing to this string
	void FillZero()
	{
		ZeroFill(msgbuf, BufferBytesLength(true));
	}

	//! Return string length in character. Assume single null terminated.
	size_t StringCharaCount() const
	{
		return (msgbuf == nullptr) ? 0 : lstrlen(msgbuf);
	}

	//! Return string length in bytes. Assume single null terminated.
	size_t StringBytesLength() const
	{
		return sizeof(TCHAR) * StringCharaCount();
	}

	//! Obtain buffer chara count
	size_t BufferCharaCount(bool includeNullTerm = false) const
	{
		return (msgbuf == nullptr) ? 0 : (g_stringsize + (includeNullTerm ? 1 : 0));
	}

	//! Obtain buffer byte size
	size_t BufferBytesLength(bool includeNullTerm = false) const
	{
		return sizeof(TCHAR) * BufferCharaCount(includeNullTerm);
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
	~NsisString()
	{
		if (msgbuf != nullptr)
		{
			GlobalFree(msgbuf);
		}
	}

	//! Get path part from path
	bool GetToken(TCHAR delim, size_t &nextOffset, NsisString &outStr)
	{
		if (msgbuf != nullptr)
		{
			const size_t lastOffset = nextOffset;
			const size_t maxOffset = StringCharaCount();
			if (maxOffset <= nextOffset)
			{
				return false;
			}
			while (true)
			{
				TCHAR chara = msgbuf[nextOffset];
				if (chara == delim || chara == 0 || maxOffset <= nextOffset)
				{
					bool result = outStr.AssignString(msgbuf, lastOffset, nextOffset - lastOffset);
					nextOffset++;
					return result;
				}
				nextOffset++;
			}
		}
		return false;
	}
};

//! getter prototype
typedef bool(*GetRegValue)(LPCTSTR EnvVarName, NsisString &ResultVar);

//! getter error fallback
bool GetNullRegValue(LPCTSTR EnvVarName, NsisString &ResultVar)
{
	return false;
}

//! getter for current user
bool GetHKCURegValue(LPCTSTR EnvVarName, NsisString &ResultVar)
{
	HKEY keyHandle;
	LSTATUS error = RegOpenKeyEx(
		HKEY_CURRENT_USER,
		_T("Environment"),
		0,
		KEY_READ,
		&keyHandle
	);
	if (error == ERROR_SUCCESS)
	{
		DWORD typeReturned;
		DWORD bytesWritten = ResultVar.BufferBytesLength();
		ResultVar.FillZero();
		error = RegQueryValueEx(
			keyHandle,
			EnvVarName,
			NULL,
			&typeReturned,
			reinterpret_cast<LPBYTE>(static_cast<LPTSTR>(ResultVar)),
			&bytesWritten
		);
		if (error == ERROR_SUCCESS || error == ERROR_FILE_NOT_FOUND)
		{
			RegCloseKey(keyHandle);
			return true;
		}

		RegCloseKey(keyHandle);
	}
	return false;
}

//! getter for local machine
bool GetHKLMRegValue(LPCTSTR EnvVarName, NsisString &ResultVar)
{
	HKEY keyHandle;
	LSTATUS error = RegOpenKeyEx(
		HKEY_LOCAL_MACHINE,
		_T("SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment"),
		0,
		KEY_READ,
		&keyHandle
	);
	if (error == ERROR_SUCCESS)
	{
		DWORD typeReturned;
		DWORD bytesWritten = ResultVar.BufferBytesLength();
		ResultVar.FillZero();
		error = RegQueryValueEx(
			keyHandle,
			EnvVarName,
			NULL,
			&typeReturned,
			reinterpret_cast<LPBYTE>(static_cast<LPTSTR>(ResultVar)),
			&bytesWritten
		);
		if (error == ERROR_SUCCESS || error == ERROR_FILE_NOT_FOUND)
		{
			RegCloseKey(keyHandle);
			return true;
		}

		RegCloseKey(keyHandle);
	}
	return false;
}

//! setter prototype
typedef bool(*SetRegValue)(LPCTSTR EnvVarName, const NsisString &NewValue);

//! setter error fallback
bool SetNullRegValue(LPCTSTR EnvVarName, const NsisString &NewValue)
{
	return false;
}

//! setter for current user
bool SetHKCURegValue(LPCTSTR EnvVarName, const NsisString &NewValue)
{
	HKEY keyHandle;
	DWORD disposition;
	LSTATUS error = RegCreateKeyEx(
		HKEY_CURRENT_USER,
		_T("Environment"),
		0,
		NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_WRITE,
		NULL,
		&keyHandle,
		&disposition
	);
	if (error == ERROR_SUCCESS)
	{
		error = RegSetValueEx(
			keyHandle,
			EnvVarName,
			0,
			REG_EXPAND_SZ,
			reinterpret_cast<const BYTE *>(static_cast<LPCTSTR>(NewValue)),
			NewValue.StringBytesLength()
		);

		if (error == ERROR_SUCCESS)
		{
			RegCloseKey(keyHandle);
			return true;
		}

		RegCloseKey(keyHandle);
	}
	return false;
}

//! setter for local machine
bool SetHKLMRegValue(LPCTSTR EnvVarName, const NsisString &NewValue)
{
	HKEY keyHandle;
	DWORD disposition;
	LSTATUS error = RegCreateKeyEx(
		HKEY_LOCAL_MACHINE,
		_T("SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment"),
		0,
		NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_WRITE,
		NULL,
		&keyHandle,
		&disposition
	);
	if (error == ERROR_SUCCESS)
	{
		error = RegSetValueEx(
			keyHandle,
			EnvVarName,
			0,
			REG_EXPAND_SZ,
			reinterpret_cast<const BYTE *>(static_cast<LPCTSTR>(NewValue)),
			NewValue.StringBytesLength()
		);

		if (error == ERROR_SUCCESS)
		{
			RegCloseKey(keyHandle);
			return true;
		}

		RegCloseKey(keyHandle);
	}
	return false;
}

// To work with Unicode version of NSIS, please use TCHAR-type
// functions for accessing the variables and the stack.

extern "C" void __declspec(dllexport) EnvVarUpdate(
	HWND hwndParent,
	int string_size,
	LPTSTR variables,
	stack_t **stacktop,
	extra_parameters *extra,
	...
)
{
	EXDLL_INIT();
	g_hwndParent = hwndParent;


	// note if you want parameters from the stack, pop them off in order.
	// i.e. if you are called via exdll::myFunction file.dat read.txt
	// calling popstring() the first time would give you file.dat,
	// and the second time would give you read.txt. 
	// you should empty the stack of your parameters, and ONLY your
	// parameters.

	// do your stuff here
	{
		NsisString ResultVar;
		NsisString EnvVarName;
		NsisString Action;
		NsisString RegLoc;
		NsisString PathString;

		bool success = false;

		if (true
			&& EnvVarName.Pop()
			&& Action.Pop()
			&& RegLoc.Pop()
			&& PathString.Pop()
			)
		{
			bool Append = Action.CompareToIgnoreCase(_T("A")) == 0;
			bool Prepend = Action.CompareToIgnoreCase(_T("P")) == 0;
			bool Remove = Action.CompareToIgnoreCase(_T("R")) == 0;

			bool HKLM = RegLoc.CompareToIgnoreCase(_T("HKLM")) == 0;
			bool HKCU = RegLoc.CompareToIgnoreCase(_T("HKCU")) == 0;

			GetRegValue getter = GetNullRegValue;
			SetRegValue setter = SetNullRegValue;

			if (HKCU)
			{
				getter = GetHKCURegValue;
				setter = SetHKCURegValue;
			}
			else if (HKLM)
			{
				getter = GetHKLMRegValue;
				setter = SetHKLMRegValue;
			}

			NsisString PathFromReg;
			NsisString NewPathStr;
			if (getter(EnvVarName, PathFromReg))
			{
				if (Append)
				{
					size_t offset = 0;
					NsisString onePath;
					success = true;
					// filter out your path from registry
					while (PathFromReg.GetToken(_T(';'), offset, onePath))
					{
						if (onePath.CompareToIgnoreCase(PathString) != 0)
						{
							success &= NewPathStr.AppendStringIfNotEmpty(_T(";"));
							success &= NewPathStr.AppendString(onePath);
						}
					}
					// now append your path
					success &= NewPathStr.AppendStringIfNotEmpty(_T(";"));
					success &= NewPathStr.AppendString(PathString);
				}
				else if (Prepend)
				{
					size_t offset = 0;
					NsisString onePath;
					success = true;
					// at first prepend your path
					success &= NewPathStr.AppendString(PathString);
					// filter out your path from registry
					while (PathFromReg.GetToken(_T(';'), offset, onePath))
					{
						if (onePath.CompareToIgnoreCase(PathString) != 0)
						{
							success &= NewPathStr.AppendStringIfNotEmpty(_T(";"));
							success &= NewPathStr.AppendString(onePath);
						}
					}
				}
				else if (Remove)
				{
					size_t offset = 0;
					NsisString onePath;
					success = true;
					// omit your path
					while (PathFromReg.GetToken(_T(';'), offset, onePath))
					{
						if (onePath.CompareToIgnoreCase(PathString) != 0)
						{
							success &= NewPathStr.AppendStringIfNotEmpty(_T(";"));
							success &= NewPathStr.AppendString(onePath);
						}
					}
				}

				if (success)
				{
					ResultVar.AssignString(NewPathStr);

					success = setter(EnvVarName, NewPathStr);
				}
			}
		}

		if (!success)
		{
			extra->exec_flags->exec_error++;
		}

		ResultVar.Push();
	}
}


