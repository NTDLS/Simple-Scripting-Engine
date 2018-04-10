<%
#define ERROR_MORE_DATA					234
#define SC_MANAGER_ENUMERATE_SERVICE	0x0004

#define SERVICE_ACTIVE		0x1
#define SERVICE_INACTIVE	0x2
#define SERVICE_STATE_ALL	SERVICE_ACTIVE bOR SERVICE_INACTIVE
   
#define SERVICE_KERNEL_DRIVER		0x1
#define SERVICE_FILE_SYSTEM_DRIVER	0x2
#define SERVICE_ADAPTER				0x4
#define SERVICE_RECOGNIZER_DRIVER	0x8
#define SERVICE_WIN32_OWN_PROCESS	0x10
#define SERVICE_WIN32_SHARE_PROCESS	0x20
#define SERVICE_INTERACTIVE_PROCESS	0x100
#define SERVICE_WIN32				SERVICE_WIN32_OWN_PROCESS bOR SERVICE_WIN32_SHARE_PROCESS

;Service State -- for CurrentState
#define SERVICE_STOPPED				0x00000001
#define SERVICE_START_PENDING		0x00000002
#define SERVICE_STOP_PENDING		0x00000003
#define SERVICE_RUNNING				0x00000004
#define SERVICE_CONTINUE_PENDING	0x00000005
#define SERVICE_PAUSE_PENDING		0x00000006
#define SERVICE_PAUSED				0x00000007

;Controls Accepted  (Bit Mask)
#define SERVICE_ACCEPT_STOP						0x00000001
#define SERVICE_ACCEPT_PAUSE_CONTINUE			0x00000002
#define SERVICE_ACCEPT_SHUTDOWN					0x00000004
#define SERVICE_ACCEPT_PARAMCHANGE				0x00000008
#define SERVICE_ACCEPT_NETBINDCHANGE			0x00000010
#define SERVICE_ACCEPT_HARDWAREPROFILECHANGE	0x00000020
#define SERVICE_ACCEPT_POWEREVENT				0x00000040
#define SERVICE_ACCEPT_SESSIONCHANGE			0x00000080
#define SERVICE_ACCEPT_PRESHUTDOWN				0x00000100
#define SERVICE_ACCEPT_TIMECHANGE				0x00000200
#define SERVICE_ACCEPT_TRIGGEREVENT				0x00000400

;Start Type
#define SERVICE_BOOT_START		0x00000000
#define SERVICE_SYSTEM_START	0x00000001
#define SERVICE_AUTO_START		0x00000002
#define SERVICE_DEMAND_START	0x00000003
#define SERVICE_DISABLED		0x00000004

;Error control type
#define SERVICE_ERROR_IGNORE	0x00000000
#define SERVICE_ERROR_NORMAL	0x00000001
#define SERVICE_ERROR_SEVERE	0x00000002
#define SERVICE_ERROR_CRITICAL	0x00000003

StrictType SERVICE_STATUS
   ServiceType As StrictType.Long
   CurrentState As StrictType.Long
   ControlsAccepted As StrictType.Long
   Win32ExitCode As StrictType.Long
   ServiceSpecificExitCode As StrictType.Long
   CheckPoint As StrictType.Long
   WaitHint As StrictType.Long
End StrictType

StrictType ENUM_SERVICE_STATUS
   pServiceName As StrictType.Long
   pDisplayName As StrictType.Long
   Status As SERVICE_STATUS
End StrictType

StrictType SSE_SERVICE_ENUM
	ServiceName as StrictType.String
	DisplayName as StrictType.String
End StrictType

Declare OpenSCManager Lib "advapi32" Alias "OpenSCManagerA" _
  (ByRef lpMachineName As StrictType.String, _
   ByRef lpDatabaseName As StrictType.String, _
   ByVal dwDesiredAccess As StrictType.Long) As StrictType.Long

Declare CloseServiceHandle Lib "advapi32.dll" _
	(ByVal hSCObject As StrictType.Long) As StrictType.Long
  
Declare EnumServicesStatus Lib "advapi32" Alias "EnumServicesStatusA" _
  (ByVal hSCManager As StrictType.Long, _
   ByVal dwServiceType As StrictType.Long, _
   ByVal dwServiceState As StrictType.Long, _
   ByRef lpServices As ENUM_SERVICE_STATUS, _
   ByVal cbBufSize As StrictType.Long, _
   ByRef pcbBytesNeeded As StrictType.Long, _
   ByRef lpServicesReturned As StrictType.Long, _
   ByRef lpResumeHandle As StrictType.Long) As StrictType.Long   

;Function EnumerateServices(searchPattern as String) as List

var searchPattern as string = "Avid"

	var result as List

	var svcManagerHandle As Numeric = OpenSCManager(@NULL, @NULL, SC_MANAGER_ENUMERATE_SERVICE)

	var serviceStatus as ENUM_SERVICE_STATUS
	var bufferSize as Numeric = Length(serviceStatus) * 100
	var bytesNeeded as Numeric
	var servicesReturned as Numeric
	var resumeHandle as Numeric
	var enumResult as Numeric

	var statusBytes as Bytes
	statusBytes.Allocate(bufferSize)

	While(True)
		enumResult = EnumServicesStatus( _
				svcManagerHandle, _
				SERVICE_WIN32, _
				SERVICE_STATE_ALL, _
				@statusBytes, _
				bufferSize, _
				@bytesNeeded, _
				@servicesReturned, _
				@resumeHandle);
		
		var serviceIndex as Numeric = 0
		For(serviceIndex as Numeric = 0 to servicesReturned)
			serviceStatus = Cast(statusBytes.Offset(Length(serviceStatus) * serviceIndex), ENUM_SERVICE_STATUS)

			var serviceName as string = Convert.StringFromPointer(serviceStatus.pServiceName)

			If(searchPattern.Equals("") or serviceName.IndexOf(searchPattern) >= 0)
				var enumValue as SSE_SERVICE_ENUM
				
				enumValue.ServiceName =  Convert.StringFromPointer(serviceStatus.pServiceName)
				enumValue.DisplayName =  Convert.StringFromPointer(serviceStatus.pDisplayName)

				result.Add(enumValue)
			End If
		Next

		If (enumResult = 0 and resumeHandle = 0)	
			print("Error enummerating the service status.\n")
		Else If (resumeHandle = 0)
			Break
		End If
	WEnd


	;CloseServiceHandle(svcManagerHandle)
;
For(i as Numeric to result.Count())

	;var svc as SSE_SERVICE_ENUM
	
	;svc = result[i]
	
	=Convert.StringFromPointer(result[i])
	
	
	;svc = Cast(result[i], SSE_SERVICE_ENUM)
	Print(svc.ServiceName)
Next

;return(result)
;End Function

%>
