
Name "EnvVarUpdate Plugin Example"
OutFile "Test.exe"
ShowInstDetails show
XPStyle on
Unicode false

RequestExecutionLevel user
;RequestExecutionLevel admin

Section ""
  EnvVarUpdateDLL::EnvVarUpdate "MyPath" "P" "HKCU" "C:\A"
  Pop $0
  DetailPrint $0

  EnvVarUpdateDLL::EnvVarUpdate "MyPath" "A" "HKCU" "C:\B"
  Pop $0
  DetailPrint $0

  EnvVarUpdateDLL::EnvVarUpdate "MyPath" "A" "HKCU" "C:\C"
  Pop $0
  DetailPrint $0

  EnvVarUpdateDLL::EnvVarUpdate "MyPath" "R" "HKCU" "C:\B"
  Pop $0
  DetailPrint $0
SectionEnd

; eof
