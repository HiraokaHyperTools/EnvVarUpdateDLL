
Name "EnvVarUpdate Plugin Example"
OutFile "Test.exe"
ShowInstDetails show
XPStyle on
Unicode true

;RequestExecutionLevel user
RequestExecutionLevel admin

Section ""
  EnvVarUpdateDLL::EnvVarUpdate "MyPath" "P" "HKLM" "C:\A"
  Pop $0
  DetailPrint $0

  EnvVarUpdateDLL::EnvVarUpdate "MyPath" "A" "HKLM" "C:\B"
  Pop $0
  DetailPrint $0

  EnvVarUpdateDLL::EnvVarUpdate "MyPath" "A" "HKLM" "C:\C"
  Pop $0
  DetailPrint $0

  EnvVarUpdateDLL::EnvVarUpdate "MyPath" "R" "HKLM" "C:\B"
  Pop $0
  DetailPrint $0
SectionEnd

; eof
