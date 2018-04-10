<%;Preprocessors

#Namespace "Windows.Registry"
#Include "Windows.ss"

%>
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
<%;Definitions

;;; Registry class roots ;;;
#define HKCR						0x80000000 ;HKEY_CLASSES_ROOT
#define HKCU						0x80000001 ;HKEY_CURRENT_USER
#define HKLM						0x80000002 ;HKEY_LOCAL_MACHINE
#define HKUS						0x80000003 ;HKEY_USERS
#define HKPD						0x80000004 ;HKEY_PERFORMANCE_DATA
#define HKPT						0x80000050 ;HKEY_PERFORMANCE_TEXT
#define HKPNLST						0x80000060 ;HKEY_PERFORMANCE_NLSTEXT
#define HKCC						0x80000005 ;HKEY_CURRENT_CONFIG
#define HKDD						0x80000006 ;HKEY_DYN_DATA

;;; Registry permissions / rights ;;;
#define KEY_ENUMERATE_SUB_KEYS		0x8
#define KEY_NOTIFY					0x10
#define KEY_QUERY_VALUE				0x1
#define KEY_SET_VALUE				0x0002
#define KEY_READ					(Windows.STANDARD_RIGHTS_WRITE bOR KEY_QUERY_VALUE bOR KEY_ENUMERATE_SUB_KEYS bOR KEY_NOTIFY) bAND (bNOT Windows.SYNCHRONIZE)

;;; Registry value types ;;;
#define TYPE_BINARY					3 ;A non-text sequence of bytes.
#define TYPE_DWORD					4 ;Same as DWORD_LITTLE_ENDIAN.
#define TYPE_DWORD_BIG_ENDIAN		5 ;A 32-bit integer stored in big-endian format. This is the opposite of the way Intel-based computers normally store numbers -- the byte order is reversed.
#define TYPE_DWORD_LITTLE_ENDIAN	4 ;A 32-bit integer stored in little-endian format. This is the way Intel-based computers store numbers.
#define TYPE_EXPAND_SZ				2 ;A null-terminated string which contains unexpanded environment variables.
#define TYPE_LINK					6 ;A Unicode symbolic link.
#define TYPE_MULTI_SZ				7 ;A series of strings, each separated by a null character and the entire set terminated by a two null characters.
#define TYPE_NONE					0 ;No data type.
#define TYPE_RESOURCE_LIST			8 ;A list of resources in the resource map.
#define TYPE_SZ						1 ;A string terminated by a null character. 

%>
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
<%;External Prototypes

Declare RegCloseKey Lib "advapi32.dll" (ByVal hKey As StrictType.Handle) As StrictType.Long

Declare RegOpenKeyEx Lib "advapi32.dll" Alias "RegOpenKeyExA" _
	(ByVal hKey as StrictType.Handle, _
	ByVal lpSubKey As StrictType.String, _
	ByVal ulOptions As StrictType.Long, _
	ByVal samDesired As StrictType.Long, _
	ByRef phkResult As StrictType.Long) As StrictType.Long
	
Declare RegQueryTextEx Lib "advapi32.dll" Alias "RegQueryValueExA" _
	(ByVal hKey As StrictType.Handle, _
	ByVal lpValueName As StrictType.String, _
	ByVal lpReserved As StrictType.Long, _
	ByRef lpType As StrictType.Long, _
	ByRef lpData As StrictType.String, _
	ByRef lpcbData As StrictType.Long) As StrictType.Long

Declare RegQueryNumericEx Lib "advapi32.dll" Alias "RegQueryValueExA" _
	(ByVal hKey As StrictType.Handle, _
	ByVal lpValueName As StrictType.String, _
	ByVal lpReserved As StrictType.Long, _
	ByRef lpType As StrictType.Long, _        
	ByRef lpData As StrictType.Long, _
	ByRef lpcbData As StrictType.Long) As StrictType.Long

Declare RegCreateKey Lib "advapi32.dll" Alias "RegCreateKeyA" _
	(ByVal hkey As StrictType.Handle, _
	ByVal lpSubKey As StrictType.String, _
	ByRef phkResult As StrictType.Long) As StrictType.Long
	
Declare RegDeleteKey Lib "advapi32.dll" Alias "RegDeleteKeyA" _
	(ByVal hkey As StrictType.Handle, _
	ByVal lpSubKey As StrictType.String) As StrictType.Long
	
Declare RegDeleteValue Lib "advapi32.dll" Alias "RegDeleteValueA" _
	(ByVal hkey As StrictType.Handle, _
	ByVal lpValueName As StrictType.String) As StrictType.Long

Declare RegSetTextEx Lib "advapi32.dll" Alias "RegSetValueExA" _
	(ByVal hkey As StrictType.Handle, _
	ByVal lpValueName As StrictType.String, _
	ByVal Reserved As StrictType.Long, _
	ByVal dwType As StrictType.Long, _
	ByRef lpData As StrictType.String, _
	ByVal cbData As StrictType.Long) As StrictType.Long

Declare RegSetNumericEx Lib "advapi32.dll" Alias "RegSetValueExA" _
	(ByVal hkey As StrictType.Handle, _
	ByVal lpValueName As StrictType.String, _
	ByVal Reserved As StrictType.Long, _
	ByVal dwType As StrictType.Long, _
	ByRef lpData As StrictType.Long, _
	ByVal cbData As StrictType.Long) As StrictType.Long
	
	
%>
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
<%;Abstractions

Function CreateKey(rootKey as Numeric, subKey as String) as Numeric

	var regKeyHandle as numeric
	
	;2147483648
	;2147483650
	
	var regResult as numeric = Windows.Registry.RegCreateKey(rootKey, subKey, @regKeyHandle)
	
	If(regKeyHandle != 0)
		Windows.Registry.RegCloseKey(regKeyHandle)
	End If
	
	Return (regResult = Windows.ERROR_SUCCESS)
	
End Function

Function GetString(rootKey as Numeric, subKey as String, valueName as String) as String

	Var regResult as Numeric
	Var regKeyHandle as Numeric

	If(Windows.Registry.RegOpenKeyEx(rootKey, subKey, NULL, KEY_READ, @regKeyHandle) != Windows.ERROR_SUCCESS)
		Error.Throw("Failed to open registry key.")
	End If
	
	var regValue as String = Space(1000)
	var regValueSize as Numeric = regValue.Length()
	var regType as Numeric = TYPE_SZ

	regResult = Windows.Registry.RegQueryTextEx(regKeyHandle, valueName, NULL, @regType, @regValue, @regValueSize)
	Windows.Registry.RegCloseKey(regKeyHandle)

	If(regResult != Windows.ERROR_SUCCESS)
		Error.Throw("Failed to retrieve registry value, error " & regResult & ".")
	End If

	Return(regValue)
End Function

Function SetString(rootKey as Numeric, subKey as String, valueName as String, valueText as String) as Numeric

	Var regResult as Numeric
	Var regKeyHandle as Numeric

	If(Windows.Registry.RegOpenKeyEx(rootKey, subKey, NULL, KEY_SET_VALUE, @regKeyHandle) != Windows.ERROR_SUCCESS)
		Error.Throw("Failed to open registry key.")
	End If
	
	var regType as Numeric = TYPE_SZ

	regResult = Windows.Registry.RegSetTextEx(regKeyHandle, valueName, NULL, regType, valueText, valueText.Length())
	Windows.Registry.RegCloseKey(regKeyHandle)

	If(regResult != Windows.ERROR_SUCCESS)
		Error.Throw("Failed to set registry value, error " & regResult & ".")
	End If

	Return (regResult = Windows.ERROR_SUCCESS)
End Function

Function GetDWORD(rootKey as Numeric, subKey as String, valueName as String) as Numeric

	Var regResult as Numeric
	Var regKeyHandle as Numeric

	If(Windows.Registry.RegOpenKeyEx(rootKey, subKey, NULL, KEY_READ, @regKeyHandle) != Windows.ERROR_SUCCESS)
		Error.Throw("Failed to open registry key.")
	End If
	
	var regValue as Numeric
	var regValueSize as Numeric = DataLen(regValue)
	var regType as Numeric = TYPE_DWORD

	regResult = Windows.Registry.RegQueryNumericEx(regKeyHandle, valueName, NULL, @regType, @regValue, @regValueSize)

	If(regResult != Windows.ERROR_SUCCESS)
		Error.Throw("Failed to set registry value, error " & regResult & ".")
	End If

	Return(regValue)
End Function

Function SetDWORD(rootKey as Numeric, subKey as String, valueName as String, valueNumber as Numeric) as Numeric

	Var regResult as Numeric
	Var regKeyHandle as Numeric

	If(Windows.Registry.RegOpenKeyEx(rootKey, subKey, NULL, KEY_SET_VALUE, @regKeyHandle) != Windows.ERROR_SUCCESS)
		Error.Throw("Failed to open registry key.")
	End If
	
	var regType as Numeric = TYPE_DWORD

	regResult = Windows.Registry.RegSetNumericEx(regKeyHandle, valueName, NULL, regType, @valueNumber, DataLen(valueNumber))
	Windows.Registry.RegCloseKey(regKeyHandle)

	If(regResult != Windows.ERROR_SUCCESS)
		Error.Throw("Failed to set registry value, error " & regResult & ".")
	End If

	Return (regResult = Windows.ERROR_SUCCESS)
End Function

%>
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
<%;Example
;
;Print("CPU: " & Windows.Registry.GetString(Windows.Registry.HKLM, _
;	"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", _
;	"ProcessorNameString"))
;
%>
