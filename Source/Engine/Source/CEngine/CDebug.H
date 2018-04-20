#ifndef _CDebug_H_
#define _CDebug_H_
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define DEBUGPIPENAME			"\\\\.\\pipe\\SSE"
#define DEBUGPIPEBUFSZ			1024
#define DEBUGPIPETIMEOUT		100
#define DEBUGMAXINSTANCENAME	255

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CException.H"
#include "CLines.H"

typedef struct _tag_BreakPoint {
	char *FileName;
	int LineIndex; //The index of the file line in the line array.
	int ScriptIndex;
	int ReportedIndex;
	bool Used;
} BREAKPOINT, *LPBREAKPOINT;

typedef struct _tag_BreakPoints {
	BREAKPOINT *Points;
	int Count;
} BREAKPOINTS, *LPBREAKPOINTS;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CDebug {

public:
	CException *Error;

	CDebug(void *lpEngine, const char *sDebugInstanceName);
	CDebug(void *lpEngine);
	~CDebug();

	void StopDebugThreadProc(void);
	void SendImmediateInfo(const char *sValue);
	void SendImmediateWord(const char *sType, const char *sValue);
	void SendSymbolInfo(const char *sName, const char *sType, const char *sValue);
	ErrorLevel AddBreakPoint(const char *sFilename, int iFileLine);
	ErrorLevel RemoveBreakPoint(const char *sFilename, int iFileLine);
	ErrorLevel RemoveAllPoints(void);
	bool IsBreakPoint(int iLineIndex);
	void Break(void);
	ErrorLevel Start(void);
	friend unsigned __stdcall DebugThreadProcEx(LPVOID pVoid);

private:
	void *pEngine; //Pointer to the Engine Class.

	BREAKPOINT *GetFreeBreakPointSlot(void);
	int ReadPipe(char *sBuffer, int iBufferSize);
	int WritePipe(const char *sFormat, ...);
	ErrorLevel DebugThreadProc(void);
	void FreeBreakPoint(BREAKPOINT *pPoint);
	bool PokeNamedPipe(const char *sLocalPipeName, DWORD dwAccessMode);
	void RedirectIO(FILE *hFrom, HANDLE hTo);

	HANDLE hInitialBreakPointsLoaded;
	HANDLE hThread;
	bool bKeepRunning;
	bool bBreakOnNextLine;
	int iBreakAtScope;
	int iBreakAtLessThanScope;
	int iBreakAtLessThanEqualScope;

public:
	bool IsDebuggerAttached;
	bool UseSTDRedirection;
	bool WaitOnIDE;

	HANDLE hReadCmdPipe;
	HANDLE hWriteCmdPipe;
	HANDLE hWriteTxtPipe;
	HANDLE hWriteErrPipe;
	HANDLE hBreakEvent;
	BREAKPOINTS BreakPoints;

	char sInstanceName[DEBUGMAXINSTANCENAME];
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

