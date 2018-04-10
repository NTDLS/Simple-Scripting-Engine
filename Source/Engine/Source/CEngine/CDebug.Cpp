#ifndef _CDebug_CPP_
#define _CDebug_CPP_
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>
#include <Process.H>
#include <fcntl.h>
#include "IO.H"
#include "../../../../../@Libraries/NSWFL/NSWFL.H"

#include "CEngine.H"
#include "CDebug.H"
#include "CScripts.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::String;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CDebug::WritePipe(const char *sFormat, ...)
{
	va_list ArgList;
	va_start(ArgList, sFormat);

	int iMemoryRequired = _vscprintf(sFormat, ArgList);

	char *sBuf = (char *)calloc(sizeof(char), iMemoryRequired + 1);

	int iSz = _vsprintf_s_l(sBuf, iMemoryRequired + 1, sFormat, NULL, ArgList);
	va_end(ArgList);

	DWORD dwBytesWritten = 0;
	if (!WriteFile(this->hWriteCmdPipe, sBuf, iSz, &dwBytesWritten, NULL))
	{
		dwBytesWritten = 0;
	}

	FlushFileBuffers(this->hWriteCmdPipe);

	free(sBuf);

	return (int)dwBytesWritten;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CDebug::ReadPipe(char *sBuffer, int iBufferSize)
{
	DWORD dwBytesRead = 0;
	DWORD dwLastError = 0;

	do
	{
		if (ReadFile(this->hReadCmdPipe, sBuffer, iBufferSize - 1, &dwBytesRead, NULL))
		{
			sBuffer[dwBytesRead] = '\0';
			return (int)dwBytesRead;
		}
		else {
			dwLastError = GetLastError(); // This pipe is non-blocking.
		}
	} while (dwLastError == ERROR_NO_DATA);

	return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CDebug::SendSymbolInfo(const char *sName, const char *sType, const char *sValue)
{
	this->WritePipe("::SymbolInfo~|%s|%s|%s", sName, sType, sValue);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CDebug::SendImmediateInfo(const char *sValue)
{
	this->WritePipe("::ImmediateInfo~|%s", sValue);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CDebug::SendImmediateWord(const char *sType, const char *sValue)
{
	this->WritePipe("::ImmediateWord~|%s|%s", sType, sValue);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CDebug::RedirectIO(FILE *hFrom, HANDLE hNamedPipe)
{
	if (this->UseSTDRedirection)
	{
		int fd = _open_osfhandle((intptr_t)hNamedPipe, _O_WRONLY | _O_TEXT);
		_dup2(fd, _fileno(hFrom));
		setvbuf(hFrom, NULL, _IONBF, 0);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CDebug::DebugThreadProc(void)
{
	DWORD dwAvailable = 0;

	CEngine *pEngine = (CEngine *)this->pEngine;

	bool bConnected = false;
	bool bWasPreviouslyConnected = false;

	while (this->bKeepRunning)
	{
		if (!bConnected)
		{
			this->IsDebuggerAttached = false;

			if (bWasPreviouslyConnected)
			{
				if (this->UseSTDRedirection)
				{
					//We were connected and now we're not. So we have to re-redirect the stdout.
					*stdout = *pEngine->OriginalSTDOut;

					DisconnectNamedPipe(this->hReadCmdPipe);
					DisconnectNamedPipe(this->hWriteCmdPipe);
					DisconnectNamedPipe(this->hWriteTxtPipe);
					DisconnectNamedPipe(this->hWriteErrPipe);
				}

				this->RemoveAllPoints();
			}

			bWasPreviouslyConnected = false;

			while (this->bKeepRunning && !bConnected)
			{
				bConnected = ConnectNamedPipe(this->hReadCmdPipe, NULL) ? true : (GetLastError() == ERROR_PIPE_CONNECTED);
				if (bConnected && this->bKeepRunning)
				{
					bConnected = ConnectNamedPipe(this->hWriteCmdPipe, NULL) ? true : (GetLastError() == ERROR_PIPE_CONNECTED);
					if (bConnected && this->bKeepRunning)
					{
						bConnected = ConnectNamedPipe(this->hWriteTxtPipe, NULL) ? true : (GetLastError() == ERROR_PIPE_CONNECTED);
						if (bConnected && this->bKeepRunning)
						{
							bConnected = ConnectNamedPipe(this->hWriteErrPipe, NULL) ? true : (GetLastError() == ERROR_PIPE_CONNECTED);
						}
					}
				}
				Sleep(100);
			}

			if (!bConnected || !this->bKeepRunning)
			{
				continue;
			}

			this->IsDebuggerAttached = true;

			if (this->UseSTDRedirection)
			{
				//Redirect unbuffered stdOUT to the text pipe.
				RedirectIO(stdout, this->hWriteTxtPipe);
				RedirectIO(stderr, this->hWriteTxtPipe);
			}
		}

		bWasPreviouslyConnected = true;

		bConnected = (PeekNamedPipe(this->hReadCmdPipe, NULL, 0, NULL, &dwAvailable, &dwAvailable) > 0);
		if (dwAvailable)
		{
			char sBuffer[DEBUGPIPEBUFSZ + 1];
			int iBytesRead = 0;
			if (ReadPipe(sBuffer, sizeof(sBuffer)))
			{
				//------------------------------------------------------------------------------------------------------
				//Initial load of break points.
				if (_strcmpi(sBuffer, "::BreakPoints:Begin~") == 0)
				{
					while ((iBytesRead = ReadPipe(sBuffer, sizeof(sBuffer))) > 0)
					{
						if (_strcmpi(sBuffer, "::BreakPoints:End~") == 0)
						{
							if (this->WaitOnIDE && this->hInitialBreakPointsLoaded)
							{
								//Do not want to continue untill the initial breakpoints are loaded.
								SetEvent(this->hInitialBreakPointsLoaded);
							}
							break;
						}

						char **sArray = Split(sBuffer, "|");
						if (sArray && UBound(sArray) == 3 && _strcmpi(sArray[0], "::BreakPoint~") == 0)
						{
							this->AddBreakPoint(sArray[1], atoi(sArray[2]));
						}
						FreeArray(sArray);
					}
					if (iBytesRead < 0)
					{
						continue;
					}
				}
				//------------------------------------------------------------------------------------------------------
				//List all loaded files.
				else if (_strnicmp(sBuffer, "::BeginFileList~", 16) == 0)
				{
					this->WritePipe("::BeginFileList~");

					for (int iScript = 0; iScript < pEngine->Scripts->Collection.Count; iScript++)
					{
						SCRIPTITEM *pScript = pEngine->Scripts->GetPointer(iScript);
						this->WritePipe("::FileList~|%s", pScript->FileName);
					}

					this->WritePipe("::EndFileList~");
				}
				//------------------------------------------------------------------------------------------------------
				//List all non-system local variables.
				else if (_strnicmp(sBuffer, "::ListLocalVariables~", 21) == 0)
				{
					for (int iVar = pEngine->Vars->_EndOfBuiltInIndex; iVar < pEngine->Vars->Count(); iVar++)
					{
						if (pEngine->Vars->IsDefined(iVar))
						{
							VARIABLEITEM *pVar = pEngine->Vars->GetPointer(iVar);
							if (pVar->Scope >= pEngine->State->LastMajorScope)
							{
								char sFriendlyText[FRIENDLY_DEBUG_VAR_SIZE + 1];
								if (pEngine->Vars->FriendlyValue(pVar, sFriendlyText, sizeof(sFriendlyText)))
								{
									this->WritePipe("::UpdateLocalVariable~|%s|%d|%s|%s",
										pVar->Name, pVar->Scope, pEngine->Types->Name(pVar->Type), sFriendlyText);
								}
							}
						}
					}

					this->WritePipe("::EndOfLocalVariable~");
				}
				//------------------------------------------------------------------------------------------------------
				//List all non-system variables
				else if (_strnicmp(sBuffer, "::ListVariables~", 16) == 0)
				{
					this->WritePipe("::BeginListVariables~");

					CStringBuilder string;

					for (int iVar = pEngine->Vars->_EndOfBuiltInIndex; iVar < pEngine->Vars->Count(); iVar++)
					{
						if (pEngine->Vars->IsDefined(iVar))
						{
							VARIABLEITEM *pVar = pEngine->Vars->GetPointer(iVar);
							string.AppendF("%s|", pVar->Name);
						}

						if (string.Length > 500) //Stream the data over in roughly 500 bytes chunks.
						{
							this->WritePipe("::ListVariables~|%s", string.Buffer);
							string.Clear();
						}
					}

					this->WritePipe("::ListVariables~|%s", string.Buffer);
					this->WritePipe("::EndListVariables~");
				}
				//------------------------------------------------------------------------------------------------------
				//Add a breakpoint
				else if (_strnicmp(sBuffer, "::BreakPoint~", 13) == 0)
				{
					char **sArray = Split(sBuffer + 14, "|");
					if (sArray && UBound(sArray) == 2)
					{
						this->AddBreakPoint(sArray[0], atoi(sArray[1]));
					}
					FreeArray(sArray);
				}
				//------------------------------------------------------------------------------------------------------
				//Remove a breakpoint
				else if (_strnicmp(sBuffer, "::DeleteBreakPoint~", 19) == 0)
				{
					char **sArray = Split(sBuffer + 20, "|");
					if (sArray && UBound(sArray) == 2)
					{
						this->RemoveBreakPoint(sArray[0], atoi(sArray[1]));
					}
					FreeArray(sArray);
				}
				//------------------------------------------------------------------------------------------------------
				else if (_strnicmp(sBuffer, "::QuickWatch~|", 14) == 0)
				{
					char *sVarName = sBuffer + 14;
					VARIABLEITEM *pVar = pEngine->Vars->GetPointer(sVarName);
					if (pVar)
					{
						char sFriendlyText[QUICKWATCH_DEBUG_VAR_SIZE + 1];
						if (pEngine->Vars->FriendlyValue(pVar, sFriendlyText, sizeof(sFriendlyText)))
						{
							this->WritePipe("::QuickWatch~|%s|%s|%s",
								sVarName, pEngine->Types->Name(pVar->Type), sFriendlyText);
						}
					}
					else {
						//Unknown symbol.
					}
				}
				//------------------------------------------------------------------------------------------------------
				else if (_strnicmp(sBuffer, "::ToolTipSymbolInfo~|", 21) == 0)
				{
					char *sVarName = sBuffer + 21;
					VARIABLEITEM *pVar = pEngine->Vars->GetPointer(sVarName);
					if (pVar)
					{
						char sFriendlyText[FRIENDLY_DEBUG_VAR_SIZE + 1];
						if (pEngine->Vars->FriendlyValue(pVar, sFriendlyText, sizeof(sFriendlyText)))
						{
							this->WritePipe("::ToolTipSymbolInfo~|%s|%s|%s",
								sVarName, pEngine->Types->Name(pVar->Type), sFriendlyText);
						}
					}
					else {
						//Unknown symbol.
					}
				}
				//------------------------------------------------------------------------------------------------------
				else if (_strnicmp(sBuffer, "::ImmediateAutoList~|", 21) == 0)
				{
					char *sWord = sBuffer + 21;
					int iWordLen = (int)strlen(sWord);

					this->WritePipe("::ImmediateAutoListBegin~");

					for (int i = 0; i < pEngine->Vars->Count(); i++)
					{
						VARIABLEITEM *pVar = pEngine->Vars->GetPointer(i);
						if (pVar)
						{
							if (_strnicmp(pVar->Name, sWord, iWordLen) == 0)
							{
								if (pEngine->Types->GetPointer(pVar->Type)->IsComplex)
								{
									SendImmediateWord("GenericVar", pVar->Name);
								}
								else {
									SendImmediateWord("Var", pVar->Name);
								}
							}
						}
					}

					for (int i = 0; i < pEngine->Procs->Count(); i++)
					{
						PROCITEM *pProc = pEngine->Procs->GetPointer(i);
						if (pProc)
						{
							if (_strnicmp(pProc->Name, sWord, iWordLen) == 0)
							{
								if (pProc->Type == PROC_TYPE_SCRIPT)
								{
									SendImmediateWord("ScriptProc", pProc->Name);
								}
								else if (pProc->Type == PROC_TYPE_SYSTEM)
								{
									SendImmediateWord("SysProc", pProc->Name);
								}
							}
						}
					}

					this->WritePipe("::ImmediateAutoListEnd~");
				}
				//------------------------------------------------------------------------------------------------------
				else if (_strnicmp(sBuffer, "::Immediate~|", 13) == 0)
				{
					char *sCodeText = sBuffer + 13;

					VARIABLEITEM *pVar = pEngine->Vars->GetPointer(sCodeText);
					if (pVar)
					{
						char sFriendlyText[FRIENDLY_DEBUG_VAR_SIZE + 1];
						if (pEngine->Vars->FriendlyValue(pVar, sFriendlyText, sizeof(sFriendlyText)))
						{
							SendImmediateInfo(sFriendlyText);
						}
					}
					else {
						CParams *pParam = NULL;
						pEngine->Error->AbsorbExceptions(true);
						pEngine->Evaluate(sCodeText, (int)strlen(sCodeText), &pParam);
						pEngine->Error->AbsorbExceptions(false);
						if (pParam)
						{
							if (pParam->Collection.Count == 1)
							{
								this->SendImmediateInfo(pParam->ToString(0));
							}
							else {
								this->SendImmediateInfo("<done>");
							}
							delete pParam;
						}
						else {
							this->SendImmediateInfo("<done>");
						}
					}
				}
				//------------------------------------------------------------------------------------------------------
				else if (_strnicmp(sBuffer, "::SymbolInfo~|", 14) == 0)
				{
					char *sCodeText = sBuffer + 14;

					VARIABLEITEM *pVar = pEngine->Vars->GetPointer(sCodeText);
					if (pVar)
					{
						int iFriendlyLength = pVar->Length > 100 ? 100 : pVar->Length;
						char sFriendlyText[101];
						memcpy(sFriendlyText, pVar->Value, iFriendlyLength);
						sFriendlyText[iFriendlyLength] = '\0';

						SendSymbolInfo(sCodeText, pEngine->Types->Name(pVar->Type), sFriendlyText);
					}
					else {
						CParams *pParam = NULL;

						pEngine->Error->AbsorbExceptions(true);
						pEngine->Evaluate(sCodeText, (int)strlen(sCodeText), &pParam);
						pEngine->Error->AbsorbExceptions(false);

						if (pParam)
						{
							if (pParam->Collection.Count == 1)
							{
								this->SendSymbolInfo(sCodeText, "<dynamic>", pParam->ToString(0));
							}
							delete pParam;
						}
						else {
							this->SendSymbolInfo(sCodeText, "<dynamic>", "<exception>");
						}
					}
				}
				//------------------------------------------------------------------------------------------------------
				else if (_strnicmp(sBuffer, "::Pause~", 8) == 0)
				{
					this->bBreakOnNextLine = true;
				}
				//------------------------------------------------------------------------------------------------------
				else if (_strnicmp(sBuffer, "::Continue~", 11) == 0)
				{
					SetEvent(this->hBreakEvent);
				}
				//------------------------------------------------------------------------------------------------------
				else if (_strnicmp(sBuffer, "::StepOver~", 11) == 0)
				{
					this->iBreakAtLessThanEqualScope = pEngine->State->CurrentScope();
					SetEvent(this->hBreakEvent);
				}
				//------------------------------------------------------------------------------------------------------
				else if (_strnicmp(sBuffer, "::StepInto~", 11) == 0)
				{
					this->bBreakOnNextLine = true;
					SetEvent(this->hBreakEvent);
				}
				//------------------------------------------------------------------------------------------------------
				else if (_strnicmp(sBuffer, "::StepOut~", 10) == 0)
				{
					if (pEngine->State->CurrentScope() > 0)
					{
						this->iBreakAtLessThanScope = pEngine->State->CurrentScope();
					}
					SetEvent(this->hBreakEvent);
				}
				//------------------------------------------------------------------------------------------------------
				else {
					if (this->WaitOnIDE && this->hInitialBreakPointsLoaded)
					{
						SetEvent(this->hInitialBreakPointsLoaded);
					}

					MessageBox(NULL, sBuffer, "", 0);
					return this->Error->Hard("Invalid debugger command: [%s]", sBuffer);
				}
			}
		}
		Sleep(1);
	}

	if (this->WaitOnIDE && this->hInitialBreakPointsLoaded)
	{
		SetEvent(this->hInitialBreakPointsLoaded);
	}

	return this->Error->HardCount();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned __stdcall DebugThreadProcEx(LPVOID pVoid)
{
	CDebug *pDebug = (CDebug *)pVoid;

	pDebug->DebugThreadProc();
	_endthreadex(0);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CDebug::Start(void)
{
	this->iBreakAtScope = INVALID_INDEX;
	this->iBreakAtLessThanEqualScope = INVALID_INDEX;
	this->iBreakAtLessThanScope = INVALID_INDEX;

	char sPipeName[MAX_PATH];
	bool bConnected = false;

	if (this->bKeepRunning)
	{
		return this->Error->HardCount();
	}

	sprintf_s(sPipeName, sizeof(sPipeName), "%s_RCMD_%s", DEBUGPIPENAME, this->sInstanceName);
	this->hReadCmdPipe = CreateNamedPipe(
		sPipeName,
		PIPE_ACCESS_INBOUND, // read/write access 
		PIPE_TYPE_MESSAGE | // message type pipe 
		PIPE_READMODE_MESSAGE | // message-read mode
		//FILE_FLAG_WRITE_THROUGH | //Do not return until data is receive.
		PIPE_WAIT, // non-blocking mode 
		PIPE_UNLIMITED_INSTANCES, // max. instances 
		DEBUGPIPEBUFSZ, // output buffer size 
		DEBUGPIPEBUFSZ, // input buffer size 
		DEBUGPIPETIMEOUT, // client time-out 
		NULL
		); // no security attribute 
	if (this->hReadCmdPipe == INVALID_HANDLE_VALUE)
	{
		return this->Error->Hard("Failed to create debugger named pipe (Read)");
	}

	sprintf_s(sPipeName, sizeof(sPipeName), "%s_WCMD_%s", DEBUGPIPENAME, this->sInstanceName);
	this->hWriteCmdPipe = CreateNamedPipe(
		sPipeName,
		PIPE_ACCESS_OUTBOUND | FILE_FLAG_WRITE_THROUGH, PIPE_TYPE_MESSAGE |
		PIPE_WAIT, // non-blocking mode 
		PIPE_UNLIMITED_INSTANCES, // max. instances 
		DEBUGPIPEBUFSZ, // output buffer size 
		DEBUGPIPEBUFSZ, // input buffer size 
		DEBUGPIPETIMEOUT, // client time-out 
		NULL
		); // no security attribute 
	if (this->hWriteCmdPipe == INVALID_HANDLE_VALUE)
	{
		return this->Error->Hard("Failed to create debugger named pipe (Write)");
	}

	sprintf_s(sPipeName, sizeof(sPipeName), "%s_WTXT_%s", DEBUGPIPENAME, this->sInstanceName);
	this->hWriteTxtPipe = CreateNamedPipe(
		sPipeName,
		PIPE_ACCESS_OUTBOUND | FILE_FLAG_WRITE_THROUGH, PIPE_TYPE_MESSAGE |
		PIPE_WAIT, // non-blocking mode 
		PIPE_UNLIMITED_INSTANCES, // max. instances 
		DEBUGPIPEBUFSZ, // output buffer size 
		DEBUGPIPEBUFSZ, // input buffer size 
		DEBUGPIPETIMEOUT, // client time-out 
		NULL
		); // no security attribute 
	if (this->hWriteCmdPipe == INVALID_HANDLE_VALUE)
	{
		return this->Error->Hard("Failed to create debugger named pipe (Write)");
	}

	sprintf_s(sPipeName, sizeof(sPipeName), "%s_WERR_%s", DEBUGPIPENAME, this->sInstanceName);
	this->hWriteErrPipe = CreateNamedPipe(
		sPipeName,
		PIPE_ACCESS_OUTBOUND | FILE_FLAG_WRITE_THROUGH, PIPE_TYPE_MESSAGE |
		PIPE_WAIT, // non-blocking mode 
		PIPE_UNLIMITED_INSTANCES, // max. instances 
		DEBUGPIPEBUFSZ, // output buffer size 
		DEBUGPIPEBUFSZ, // input buffer size 
		DEBUGPIPETIMEOUT, // client time-out 
		NULL
		); // no security attribute 
	if (this->hWriteCmdPipe == INVALID_HANDLE_VALUE)
	{
		return this->Error->Hard("Failed to create debugger named pipe (Write)");
	}

	if (this->WaitOnIDE)
	{
		char sEvent[1024];
		sprintf_s(sEvent, sizeof(sEvent), "Global\\SSThreadStartedEvent%d", GetCurrentProcessId());
		this->hInitialBreakPointsLoaded = CreateEvent(NULL, FALSE, FALSE, sEvent);
	}
	else {
		this->hInitialBreakPointsLoaded = NULL;
	}

	if ((this->hThread = (HANDLE)_beginthreadex(NULL, 0, DebugThreadProcEx, (LPVOID) this, CREATE_SUSPENDED, NULL)) == NULL)
	{
		return this->Error->Hard("Failed to create debug thread");
	}

	this->bKeepRunning = true;

	ResumeThread(this->hThread);

	if (this->WaitOnIDE)
	{
		if (WaitForSingleObject(this->hInitialBreakPointsLoaded, 5000) == WAIT_TIMEOUT)
		{
			return this->Error->Hard("Failed to load initial breakpoints");
		}
		CloseHandle(hInitialBreakPointsLoaded);
		hInitialBreakPointsLoaded = NULL;
	}

	return this->Error->HardCount();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CDebug::StopDebugThreadProc(void)
{
	DWORD dwExitCode = 0;
	DWORD dwWait = 1000;

	while (GetExitCodeThread(this->hThread, &dwExitCode))
	{
		if (dwExitCode != STILL_ACTIVE)
		{
			CloseHandle(this->hThread);
		}
		if (dwWait-- == 0)
		{
			TerminateThread(this->hThread, 0);
		}
		Sleep(1);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BREAKPOINT *CDebug::GetFreeBreakPointSlot(void)
{
	for (int iSlot = 0; iSlot < this->BreakPoints.Count; iSlot++)
	{
		if (!this->BreakPoints.Points[iSlot].Used)
		{
			return &this->BreakPoints.Points[iSlot];
		}
	}

	if (!(this->BreakPoints.Points = (BREAKPOINT *)
		realloc(this->BreakPoints.Points, sizeof(BREAKPOINT) * (this->BreakPoints.Count + 1))))
	{
		this->Error->Hard("Failed to allocate additional memory for break point");
		return NULL;
	}

	memset(&this->BreakPoints.Points[this->BreakPoints.Count], 0, sizeof(BREAKPOINT));

	return &this->BreakPoints.Points[this->BreakPoints.Count++];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CDebug::AddBreakPoint(const char *sFilename, int iFileLine)
{
	CLines *pLines = ((CEngine *)this->pEngine)->Lines;
	CScripts *pScripts = ((CEngine *)this->pEngine)->Scripts;

	for (int iScriptIndex = 0; iScriptIndex < pScripts->Collection.Count; iScriptIndex++)
	{
		if (_strcmpi(pScripts->Collection.Items[iScriptIndex].FileName, sFilename) == 0)
		{
			for (int iLineIndex = 0; iLineIndex < pLines->Collection.Count; iLineIndex++)
			{
				if (pLines->Collection.Items[iLineIndex].ScriptIndex == iScriptIndex
					&& pLines->Collection.Items[iLineIndex].FileLine >= iFileLine)
				{
					BREAKPOINT *pPoint = this->GetFreeBreakPointSlot();

					if (pPoint)
					{
						pPoint->Used = true;
						pPoint->ReportedIndex = iFileLine;
						pPoint->LineIndex = iLineIndex;
						pPoint->FileName = _strdup(sFilename);
						pPoint->ScriptIndex = iScriptIndex;
					}
					return this->Error->HardCount();
				}
			} //Find line loop.
			return this->Error->HardCount();
		}
	} //Find script loop.

	//We couldn't find the specified file, its probably just an unrelated file that's open in the IDE.
	return this->Error->HardCount();
	//return this->Error->Hard("Could not find script file for new breakpoint");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CDebug::FreeBreakPoint(BREAKPOINT *pPoint)
{
	if (pPoint->FileName)
	{
		free(pPoint->FileName);
	}
	memset(pPoint, 0, sizeof(BREAKPOINT));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CDebug::RemoveAllPoints(void)
{
	for (int iSlot = 0; iSlot < this->BreakPoints.Count; iSlot++)
	{
		if (this->BreakPoints.Points[iSlot].Used)
		{
			FreeBreakPoint(&this->BreakPoints.Points[iSlot]);
		}
	}

	return this->Error->HardCount();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CDebug::RemoveBreakPoint(const char *sFilename, int iFileLine)
{
	for (int iSlot = 0; iSlot < this->BreakPoints.Count; iSlot++)
	{
		if (this->BreakPoints.Points[iSlot].Used)
		{
			if (_strcmpi(this->BreakPoints.Points[iSlot].FileName, sFilename) == 0)
			{
				if (this->BreakPoints.Points[iSlot].ReportedIndex == iFileLine)
				{
					FreeBreakPoint(&this->BreakPoints.Points[iSlot]);
					return this->Error->HardCount();
				}
			}
		}
	}

	return this->Error->HardCount();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CDebug::IsBreakPoint(int iLineIndex)
{
	if (!this->IsDebuggerAttached)
	{
		return false;
	}

	if (this->bBreakOnNextLine)
	{
		this->bBreakOnNextLine = false;
		return true;
	}

	if (this->iBreakAtScope != INVALID_INDEX)
	{
		if (((CEngine *)this->pEngine)->State->CurrentScope() == this->iBreakAtScope)
		{
			this->iBreakAtScope = INVALID_INDEX;
			return true;
		}
	}

	if (this->iBreakAtLessThanEqualScope != INVALID_INDEX)
	{
		if (((CEngine *)this->pEngine)->State->CurrentScope() <= this->iBreakAtLessThanEqualScope)
		{
			this->iBreakAtLessThanEqualScope = INVALID_INDEX;
			return true;
		}
	}

	if (this->iBreakAtLessThanScope != INVALID_INDEX)
	{
		if (((CEngine *)this->pEngine)->State->CurrentScope() < this->iBreakAtLessThanScope)
		{
			this->iBreakAtLessThanScope = INVALID_INDEX;
			return true;
		}
	}

	for (int iSlot = 0; iSlot < this->BreakPoints.Count; iSlot++)
	{
		if (this->BreakPoints.Points[iSlot].Used && this->BreakPoints.Points[iSlot].LineIndex == iLineIndex)
		{
			return true;
		}
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CDebug::Break(void)
{
	CEngine *pEngine = ((CEngine*)this->pEngine);

	LINEITEM *pLine = pEngine->Lines->GetPointer(pEngine->State->LineIndex);
	if (pLine && pEngine->Scripts)
	{
		SCRIPTITEM *pScript = pEngine->Scripts->GetPointer(pLine->ScriptIndex);
		this->WritePipe("::BreakPointHit~|%s|%d|%s", pScript->FileName, pLine->FileLine, pLine->Text);
	}

	while (this->IsDebuggerAttached && WaitForSingleObject(this->hBreakEvent, 1000) == WAIT_TIMEOUT)
	{
		Sleep(1);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CDebug::CDebug(void *lpEngine)
{
	this->pEngine = lpEngine;
	this->Error = ((CEngine *)lpEngine)->Error;
	this->bKeepRunning = false;
	this->hReadCmdPipe = false;
	this->hWriteCmdPipe = false;
	this->iBreakAtScope = INVALID_INDEX;
	this->IsDebuggerAttached = false;
	this->bBreakOnNextLine = false;
	this->WaitOnIDE = false;
	this->UseSTDRedirection = true;

	//char sGUID[64];
	//NewGUID(sGUID, sizeof(sGUID));
	sprintf_s(this->sInstanceName, sizeof(this->sInstanceName), "%d", GetCurrentProcessId());

	memset(&this->BreakPoints, 0, sizeof(this->BreakPoints));

	char sEvent[1024];
	sprintf_s(sEvent, sizeof(sEvent), "Global\\SSBreakPoint%d", GetCurrentProcessId());
	if (!(this->hBreakEvent = CreateEvent(NULL, FALSE, FALSE, sEvent)))
	{
		this->Error->Hard("Failed to create breakpoint event");
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CDebug::CDebug(void *lpEngine, const char *sDebugInstanceName)
{
	this->pEngine = lpEngine;
	this->Error = ((CEngine *)lpEngine)->Error;
	this->bKeepRunning = false;
	this->hReadCmdPipe = false;
	this->hWriteCmdPipe = false;
	this->iBreakAtScope = INVALID_INDEX;
	this->bBreakOnNextLine = false;
	this->WaitOnIDE = true;
	this->IsDebuggerAttached = false;
	this->UseSTDRedirection = true;

	strcpy_s(this->sInstanceName, sizeof(this->sInstanceName), sDebugInstanceName);

	memset(&this->BreakPoints, 0, sizeof(this->BreakPoints));

	char sEvent[1024];
	sprintf_s(sEvent, sizeof(sEvent), "Global\\SSBreakPoint%d", GetCurrentProcessId());
	if (!(this->hBreakEvent = CreateEvent(NULL, FALSE, FALSE, sEvent)))
	{
		this->Error->Hard("Failed to create breakpoint event");
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function is intended to open a local application pipe to force
		any blocking attempts to accept a connection to move on.
*/
bool CDebug::PokeNamedPipe(const char *sLocalPipeName, DWORD dwAccessMode)
{
	HANDLE hFile = NULL;
	char sPipeName[DEBUGMAXINSTANCENAME * 2];
	sprintf_s(sPipeName, sizeof(sPipeName), "%s%s%s", DEBUGPIPENAME, sLocalPipeName, this->sInstanceName);
	if ((hFile = CreateFile(sPipeName, dwAccessMode, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFile);
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CDebug::~CDebug()
{
	if (this->bKeepRunning)
	{
		this->bKeepRunning = false;

		FlushFileBuffers(this->hReadCmdPipe);
		FlushFileBuffers(this->hWriteCmdPipe);
		FlushFileBuffers(this->hWriteTxtPipe);
		FlushFileBuffers(this->hWriteErrPipe);

		this->PokeNamedPipe("_RCMD_", GENERIC_WRITE);
		this->PokeNamedPipe("_WCMD_", GENERIC_READ);
		this->PokeNamedPipe("_WTXT_", GENERIC_READ);
		this->PokeNamedPipe("_WERR_", GENERIC_READ);

		DisconnectNamedPipe(this->hReadCmdPipe);
		DisconnectNamedPipe(this->hWriteCmdPipe);
		DisconnectNamedPipe(this->hWriteTxtPipe);
		DisconnectNamedPipe(this->hWriteErrPipe);

		CloseHandle(this->hReadCmdPipe);
		CloseHandle(this->hWriteCmdPipe);
		CloseHandle(this->hWriteTxtPipe);
		CloseHandle(this->hWriteErrPipe);

		this->StopDebugThreadProc();

		if (this->BreakPoints.Points && this->BreakPoints.Count > 0)
		{
			for (int iSlot = 0; iSlot < this->BreakPoints.Count; iSlot++)
			{
				if (this->BreakPoints.Points[iSlot].Used)
				{
					FreeBreakPoint(&this->BreakPoints.Points[iSlot]);
				}
			}

			free(this->BreakPoints.Points);
		}

		if (this->hBreakEvent)
		{
			CloseHandle(this->hBreakEvent);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
