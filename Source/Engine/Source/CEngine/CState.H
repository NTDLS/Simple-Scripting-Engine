#ifndef _CState_H_
#define _CState_H_
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.h>

#include "../../../../../@Libraries/CMemPool/CMemPool.H"
#include "../../../../../@Libraries/CStack/CStack.H"

#include "CException.H"
#include "CLines.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define MAX_VAR_NAME_SIZE   64
#define MAX_TYPE_NAME_SIZE  64

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CState {

public:
	CException *Error;

	int SetLineIndex(int iNewLineIndex);
	const char *Namespace(void);
	int ScriptIndex(void);

	int ScopeEnterMajor(void);
	int ScopeEnter(void);
	int ScopeDrop(void);

	void CycleScope(void);

	int TryEnter(void);
	int TryDrop(void);
	int CurrentScope();
	int ScopeAtLastTry(void);

	int SoftErrorCountAtLastCatch; //Is this really needed?
	bool ContentTypeWritten;
	int LineIndex;
	int TryScope;
	int LastMajorScope;
	int MinimumScope;	//This is the lowest level (other than global) that the variable function
						// should look for variables. This is to prevent local calling function
						// vars from being available from a called function.

	CState(void *lpEngine);
	~CState();

private:
	CRITICAL_SECTION CS;

	int TryDropScopeEx(void);
	int DropScopeEx(void);
	int Scope;

	CStack *TryScopes;
	CStack *MajorScopes;

	void *pEngine; //Pointer to the Engine Class.
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

