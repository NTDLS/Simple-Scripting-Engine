#ifndef _CScript_CPP_
#define _CScript_CPP_
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>

#include "CEngine.H"
#include "CScripts.H"

#include "../../../../NSWFL/NSWFL.h"

using namespace NSWFL::String;
using namespace NSWFL::File;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Adds the contents of a file to the scripting engine.
*/
ErrorLevel CScripts::Add(const char *sFileName)
{
	ErrorLevel Result = ERROR_OK;
	int iLOCStart = ((CEngine *)this->pEngine)->Lines->Collection.Count;

	char sFullFilepath[MAX_PATH];

	if (CharIndex(sFileName, ':') != 1)
	{
		if (!_fullpath(sFullFilepath, sFileName, sizeof(sFullFilepath)))
		{
			return this->Error->Hard("Could not determine the absolute file path of [%s]", sFileName);
		}
	}
	else {
		strcpy_s(sFullFilepath, sizeof(sFullFilepath), sFileName);
	}

	//Determine if the file has already been included. Silentely ignore duplicates.
	for (int i = 0; i < this->Collection.Count; i++)
	{
		if (_strcmpi(this->Collection.Items[i].FileName, sFileName) == 0)
		{
			return ERROR_OK;
		}
	}

	this->Collection.Items = (SCRIPTITEM *)
		pMem->ReAllocate(this->Collection.Items, sizeof(SCRIPTITEM), this->Collection.Count + 1);
	if (!this->Collection.Items)
	{
		return this->Error->Hard("Memory allocation error");
	}

	SCRIPTITEM *pItem = &this->Collection.Items[this->Collection.Count++];

	pItem->Namespace = NULL;

	if (sFullFilepath)
	{
		pItem->FileName = (char *) pMem->CloneString(sFullFilepath);
		Result = ((CEngine *)this->pEngine)->Lines->LoadFromScript(this->Collection.Count - 1);

		if (this->Collection.Count == 1)
		{
			char sPathOnly[MAX_PATH];
			if (GetFilePath(sFullFilepath, sPathOnly, sizeof(sPathOnly)))
			{
				SetCurrentDirectory(sPathOnly);
			}
		}
	}

	if (Result == ERROR_OK)
	{
		if (((CEngine*)pEngine)->ValidateCodeBeforeExecution)
		{
			return ((CEngine*)pEngine)->ValidateCode(iLOCStart, ((CEngine *)this->pEngine)->Lines->Collection.Count);
		}
	}

	return Result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CScripts::IsDefined(int iIndex)
{
	return (iIndex >= 0 && iIndex < this->Collection.Count && this->Collection.Items[iIndex].FileName);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SCRIPTITEM *CScripts::GetPointer(int iIndex)
{
	if (this->IsDefined(iIndex))
	{
		return &this->Collection.Items[iIndex];
	}

	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SCRIPTITEM *CScripts::GetNameSpace(const char *sName)
{
	for (int i = 0; i < this->Collection.Count; i++)
	{
		if (this->Collection.Items[i].Namespace && _strcmpi(this->Collection.Items[i].Namespace, sName) == 0)
		{
			return &this->Collection.Items[i];
		}
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SCRIPTITEM *CScripts::GetPartialNameSpace(const char *sName, bool *fullQualified)
{
	size_t length = strlen(sName);

	for (int i = 0; i < this->Collection.Count; i++)
	{
		if (this->Collection.Items[i].Namespace && _strcmpi(this->Collection.Items[i].Namespace, sName) == 0)
		{
			if(fullQualified)
			{
				*fullQualified = true;
			}
			return &this->Collection.Items[i];
		}
		else if (this->Collection.Items[i].Namespace && _strnicmp(this->Collection.Items[i].Namespace, sName, length) == 0)
		{
			if(strlen(this->Collection.Items[i].Namespace) > length && this->Collection.Items[i].Namespace[length] == '.')
			{
				return &this->Collection.Items[i];
			}
		}
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SCRIPTITEM *CScripts::GetPartialNameSpace(const char *sName)
{
	return GetPartialNameSpace(sName, NULL);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CScripts::CScripts(void *lpEngine)
{
	this->pEngine = lpEngine;
	this->Error = ((CEngine *)lpEngine)->Error;

	memset(&this->Collection, 0, sizeof(this->Collection));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CScripts::~CScripts()
{
	if (this->Collection.Count)
	{
		for (int iItem = 0; iItem < this->Collection.Count; iItem++)
		{
			pMem->Free(this->Collection.Items[iItem].FileName);
			if (this->Collection.Items[iItem].Namespace)
			{
				pMem->Free(this->Collection.Items[iItem].Namespace);
			}
		}
		pMem->Free(this->Collection.Items);
	}
	memset(&this->Collection, 0, sizeof(this->Collection));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

