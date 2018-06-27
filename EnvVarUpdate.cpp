//! @file EnvVarUpdateDLL.cpp
//! @brief DLL version of EnvVarUpdate
//! @author kenjiuno
//! @date Jun 26 2018

#include <windows.h>
#include <nsis/pluginapi.h> // nsis plugin

#include "Utils/NsisString.h"
#include "Utils/LongString.h"

using namespace Utils;

HWND g_hwndParent;

//! getter prototype
typedef bool(*GetRegValue)(LPCTSTR EnvVarName, FixedLenStr &ResultVar);

//! getter error fallback
bool GetNullRegValue(LPCTSTR EnvVarName, FixedLenStr &ResultVar)
{
	return false;
}

//! generic getter
bool GetRegValueFrom(HKEY baseKey, LPCTSTR keyName, LPCTSTR valueName, FixedLenStr &ResultVar)
{
	HKEY keyHandle;
	LSTATUS error = RegOpenKeyEx(
		baseKey,
		keyName,
		0,
		KEY_READ,
		&keyHandle
	);
	if (error == ERROR_SUCCESS)
	{
		DWORD typeReturned;
		DWORD bytesWritten = ResultVar.BufferBytesLength();
		ResultVar.Clear();
		error = RegQueryValueEx(
			keyHandle,
			valueName,
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

//! getter for current user
bool GetHKCURegValue(LPCTSTR EnvVarName, FixedLenStr &ResultVar)
{
	return GetRegValueFrom(
		HKEY_CURRENT_USER, 
		_T("Environment"), 
		EnvVarName, 
		ResultVar
	);
}

//! getter for local machine
bool GetHKLMRegValue(LPCTSTR EnvVarName, FixedLenStr &ResultVar)
{
	return GetRegValueFrom(
		HKEY_LOCAL_MACHINE, 
		_T("SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment"), 
		EnvVarName, 
		ResultVar
	);
}

//! setter prototype
typedef bool(*SetRegValue)(LPCTSTR EnvVarName, const FixedLenStr &NewValue);

//! setter error fallback
bool SetNullRegValue(LPCTSTR EnvVarName, const FixedLenStr &NewValue)
{
	return false;
}

//! setter for current user
bool SetHKCURegValue(LPCTSTR EnvVarName, const FixedLenStr &NewValue)
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
bool SetHKLMRegValue(LPCTSTR EnvVarName, const FixedLenStr &NewValue)
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

			LongString PathFromReg;
			LongString NewPathStr;
			if (getter(EnvVarName, PathFromReg))
			{
				if (Append)
				{
					size_t offset = 0;
					LongString onePath;
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
					LongString onePath;
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
					LongString onePath;
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
