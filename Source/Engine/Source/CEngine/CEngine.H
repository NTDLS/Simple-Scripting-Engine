#ifndef _CEngine_H_
#define _CEngine_H_
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CException.H"
#include "CVars.H"
#include "CScripts.H"
#include "CState.H"
#include "CParams.H"
#include "CProcs.H"
#include "CLines.H"
#include "CDebug.H"
#include "CScriptTypes.H"
#include "Inlines.H"
#include "../../../../NSWFL/NSWFL.h"

int SQLErrorHandler(void *pCaller, const char *sSource, const char *sErrorMsg, const int iErrorNumber);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define Token(sBuf, iBufSz, sOutToken, iMaxOutSize, piRPos) \
	GetNextToken(sBuf, iBufSz, sOutToken, iMaxOutSize, piRPos, "() +-/\\*%<>=!|&{}[]^.\t\r\n,\"\'", 27)

#define TokenWithDot(sBuf, iBufSz, sOutToken, iMaxOutSize, piRPos) \
	GetNextToken(sBuf, iBufSz, sOutToken, iMaxOutSize, piRPos, "() +-/\\*%<>=!|&{}[]^\t\r\n,\"\'", 26)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define INVALID_INDEX			-1
#define MAX_TOKEN_SIZE			1024
#define MAX_NAME_SIZE			64 //Procedure, Variable
//#define ENGINE_DEBUG_FILE_NAME	"C:\\SSEDebugFile.txt"

#ifdef ENGINE_DEBUG_FILE_NAME
#define WriteDebugLog WriteDebugLog_Internal
#else
#define WriteDebugLog(x, ...)
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CEngine {

public:
	NSWFL::Collections::Stack *CommandLine;
	CException *Error;
	CScripts *Scripts;
	CVars *Vars;
	CState *State;
	CProcs *Procs;
	CLines *Lines;
	CDebug *Debug;
	CScriptTypes *Types;

	FILE *OriginalSTDOut;

	ErrorLevel ExecuteLineOfCode(LINEITEM *pLine, CParams *pReturn);
	ErrorLevel Process(void);
	ErrorLevel Process(CParams *pReturn);
	ErrorLevel Evaluate(const char *sText, int iLength, CParams **returnValues);
	ErrorLevel GetParenIndexPositions(const char *sText, int iLength, int *piRPos, int *piBegPos, int *piEndPos);
	ErrorLevel GetSquareBracketIndexPositions(const char *sText, int iLength, int *piRPos, int *piBegPos, int *piEndPos);

	ErrorLevel ParseMath(const char *sText, int iLength, int *piRPos, NSWFL::String::StringBuilder *pBuilder, NSWFL::String::StringBuilder *pResult);
	ErrorLevel ParseMath(const char *sText, int iLength, int *piRPos, NSWFL::String::StringBuilder *pResult);
	double ParseMathD(const char *sText, int iLength);
	int ParseMathI(const char *sText, int iLength);

	ErrorLevel InitializeCGIEnvironment(void);
	ErrorLevel InitializeCGIFormVariables(void);
	ErrorLevel ParseCGIPostData(const char *sNameSpace, const char *sData, int iDataLength);

	ErrorLevel SkipFunctionBlock(void);
	ErrorLevel SkipWhileBlock(void);
	ErrorLevel SkipIfBlock(bool bFullSkip);
	ErrorLevel SkipStrictTypeBlock(void);
	ErrorLevel SkipForBlock(void);
	ErrorLevel SkipToCatchBlock(void);
	ErrorLevel SkipCatchBlock(void);

	ErrorLevel ParseIfLogic(LINEITEM *pLine);
	ErrorLevel ParseWhileLogic(LINEITEM *pLine);
	ErrorLevel DoWhileLoop(CParams *pReturn);

	char GetEscapedCharacter(char cCharacter);

	ErrorLevel DoForLoop(CParams *pReturn);
	ErrorLevel ForLoopSpin(CParams *pReturn, LINEITEM *pLine, int iTOIndex, int iVarIndex, int iLogicLine);

	ErrorLevel ValidateCode(void);
	ErrorLevel ValidateCode(int iLOCStart, int LOCEnd);
	ErrorLevel IntegrateWithDebugger(const char *sDebugInstanceName);
	ErrorLevel IntegrateWithDebugger(void);
	ErrorLevel ParsePreprocessor(LINEITEM *pLine);

	int ErrorOut(const char *sFormat, ...);

	CEngine(void);
	~CEngine();

	bool GetLastSystemError(char *sOutErrorMessage, int iMaxOutErrorMessage);
	bool GetLastSystemError(HRESULT hResult, char *sOutErrorMessage, int iMaxOutErrorMessage);

	bool FormVariablesLoaded;
	bool ValidateCodeBeforeExecution;
	bool ThrowSoftExceptions;
	bool MathDebugMode;

	bool AreStringsEqual(char *sStr1, int iLength1, char *sStr2, int iLength2);
	bool AreStringsEqualI(char *sStr1, int iLength1, char *sStr2, int iLength2);

#ifdef ENGINE_DEBUG_FILE_NAME
FILE *EngineDebugFile;
void WriteDebugLog_Internal(const char *sFormat, ...);
#endif

private:
	bool IsMathChar(const char cChar);
	bool IsIdentifierChar(const char cChar);
	bool IsNumeric(char cIn);
	NSWFL::String::StringBuilder *EvaluateString(const char *sText, int iLength, int *piRPos);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

