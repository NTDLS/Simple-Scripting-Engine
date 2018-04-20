#ifndef _CState_CPP_
#define _CState_CPP_
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>

#include "../../../../NSWFL/NSWFL.h"

#include "CEngine.H"
#include "CState.H"
#include "CScripts.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::Collections;

//#define _DEBUG_SCOPE 1

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CState::CState(void *lpEngine)
{
	this->pEngine = lpEngine;
	this->Error = ((CEngine *)lpEngine)->Error;
	this->TryScopes = new Stack(BEHAVIOR_FILO);
	this->MajorScopes = new Stack(BEHAVIOR_FILO);

	this->LastMajorScope = 0;
	this->SoftErrorCountAtLastCatch = 0;
	this->TryScope = 0;
	this->Scope = 0;		//0=Global
	this->MinimumScope = 0; //0=Global
	this->LineIndex = 0;	//Current line of execution.
	this->ContentTypeWritten = false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CState::TryEnter(void)
{
	this->SoftErrorCountAtLastCatch = 0;
	this->ScopeEnter();

	this->TryScopes->Push(this->Scope);

	return ++this->TryScope;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CState::TryDrop(void)
{
	this->SoftErrorCountAtLastCatch = 0;

	return this->TryDropScopeEx();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CState::TryDropScopeEx(void)
{
	if (this->Scope - 1 < 0)
	{
		__debugbreak();
	}

	this->DropScopeEx();

	int iScope = 0;
	this->TryScopes->Pop(&iScope);

	return --this->TryScope;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CState::ScopeAtLastTry(void)
{
	int iScope = 0;
	if (this->TryScopes->Peek(&iScope))
	{
		return iScope;
	}
	else return INVALID_INDEX;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CState::ScopeEnterMajor(void)
{
	this->MajorScopes->Push(this->LastMajorScope);
	this->LastMajorScope = this->Scope + 1;
	return this->ScopeEnter();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CState::ScopeEnter(void)
{
#if _DEBUG_SCOPE
	int scope = ++this->Scope;
	printf("Enter: %d\n", scope);
	return scope;
#else
	return ++this->Scope;
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CState::CurrentScope()
{
	return this->Scope;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CState::CycleScope(void)
{
	this->ScopeDrop();
	this->ScopeEnter();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CState::ScopeDrop(void)
{
	if (this->Scope - 1 < 0)
	{
		__debugbreak();
	}

	//Because we can exit a function from within a try or catch block, we
	//	will perform cleanup of the catch blocks here when dropping scope.
	if (this->Scope == this->ScopeAtLastTry())
	{
		int iScope = 0;
		this->TryScopes->Pop(&iScope);

		--this->TryScope;
	}

	return this->DropScopeEx();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CState::DropScopeEx(void)
{
	if (this->Scope - 1 < 0)
	{
		__debugbreak();
	}

	if (this->Scope == this->LastMajorScope)
	{
		if (!this->MajorScopes->Pop(&this->LastMajorScope))
		{
			this->LastMajorScope = -1;
		}
	}

	((CEngine *)this->pEngine)->Vars->DropScope(this->Scope);

#if _DEBUG_SCOPE
	int scope = --this->Scope;
	if(scope == 1)
	{
		printf("");
	}
	printf("Drop: %d\n", scope);
	return scope;
#else
	return --this->Scope;
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CState::SetLineIndex(int iNewLineIndex)
{
	int iOldLineIndex = this->LineIndex;
	this->LineIndex = iNewLineIndex;
	return iOldLineIndex;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const char *CState::Namespace(void)
{
	CEngine *pEngine = ((CEngine *)this->pEngine);

	if (pEngine->Lines && pEngine->Lines->Collection.Count > this->LineIndex)
	{
		int iScriptIndex = pEngine->Lines->GetPointer(this->LineIndex)->ScriptIndex;
		return pEngine->Scripts->Collection.Items[iScriptIndex].Namespace;
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CState::ScriptIndex(void)
{
	CEngine *pEngine = ((CEngine *)this->pEngine);
	int iScriptIndex = pEngine->Lines->GetPointer(this->LineIndex)->ScriptIndex;
	return iScriptIndex;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CState::~CState()
{
	this->SoftErrorCountAtLastCatch = 0;
	this->TryScope = 0;
	this->Scope = 0;		//0=Global
	this->LineIndex = 0;	//Current line of execution.
	delete this->TryScopes;
	delete this->MajorScopes;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
