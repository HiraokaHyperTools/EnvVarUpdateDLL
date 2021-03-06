EnvVarUpdateDLL
---------------

This is a DLL version of well known NSIS function [EnvVarUpdate](http://nsis.sourceforge.net/Environmental_Variables:_append,_prepend,_and_remove_entries), workable for NSIS 3.03
 or later.

Using long fixed-length string buffer (32,768 TCHARs) for supporting long PATH environment variable.

## Syntax

```
  EnvVarUpdateDLL::EnvVarUpdate "EnvVarName" "Action" "RegLoc" "PathString"
  Pop "ResultVar"
```

## Parameters

- **ResultVar**
  - Updated environmental variable returned by the function
  
- **EnvVarName**
  - Environmental variable name such as "PATH", "LIB", or "MYVAR"

- **Action**
  - "A" = Append
  - "P" = Prepend
  - "R" = Remove

- **RegLoc**
  - "HKLM" = the "all users" section of the registry
  - "HKCU" = the "current user" section

- **PathString**
  - A pathname or string to add to or remove from the contents of EnvVarName (e.g., "C:\MyApp")

## Examples

### Installer Examples

```nsis
!include "winmessages.nsh"

Section "Add ${APP} to PATH"
  EnvVarUpdateDLL::EnvVarUpdate "PATH" "A" "HKCU" "$INSTDIR"
  Pop $0
  SendMessage ${HWND_BROADCAST} ${WM_WININICHANGE} 0 "STR:Environment" /TIMEOUT=5000
SectionEnd
```
