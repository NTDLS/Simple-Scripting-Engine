#ifndef _CReqParams_CPP_
#define _CReqParams_CPP_
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>

#include "CEngine.H"
#include "CReqParams.H"

#include "../../../../Shared/NSWFL/NSWFL.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

REQPARAMITEM *CReqParams::First(void)
{
	if (this->Collection.Count > 0)
	{
		return &this->Collection.Items[0];
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

REQPARAMITEM *CReqParams::Last(void)
{
	if (this->Collection.Count > 0)
	{
		return &this->Collection.Items[this->Collection.Count - 1];
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

REQPARAMITEM *CReqParams::GetPointer(int iIndex)
{
	if (this->Collection.Count > 0 && iIndex < this->Collection.Count)
	{
		return &this->Collection.Items[iIndex];
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CReqParams::Add(const char *sName, int iType)
{
	TYPEITEM *pType = ((CEngine*)this->pEngine)->Types->GetPointer(iType);

	return this->Add(sName, iType, STRICT_TYPE_INVALID, pType->IsComplex || pType->IsCustomType, false);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CReqParams::Add(const char *sName, int iType, bool isArray)
{
	TYPEITEM *pType = ((CEngine*)this->pEngine)->Types->GetPointer(iType);

	return this->Add(sName, iType, STRICT_TYPE_INVALID, pType->IsComplex || pType->IsCustomType, isArray);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CReqParams::Add(const char *sName, int iType, int iStrictType, bool bByReference, bool isArray)
{
	this->Collection.Items = (REQPARAMITEM *)
		pMem->ReAllocate(this->Collection.Items, sizeof(REQPARAMITEM), this->Collection.Count + 1);
	if (!this->Collection.Items)
	{
		return this->Error->Hard("Memory allocation error");
	}

	REQPARAMITEM *pItem = &this->Collection.Items[this->Collection.Count++];

	int iLength = (int)strlen(sName);
	pItem->Type = iType;
	pItem->StrictType = iStrictType;
	pItem->ByReference = bByReference;
	pItem->Name = (char *)pMem->CloneString(sName);

	pItem->IsArray = isArray;
	pItem->IsOptional = false;
	pItem->Default.Length = 0;
	pItem->Default.Value = NULL;

	return this->Error->HardCount();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CReqParams::AddOptional(const char *sName, int iType, unsigned int uDefault)
{
	return this->AddOptional(sName, iType, (int)uDefault);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CReqParams::AddOptional(const char *sName, int iType, int iDefault)
{
	char sInt[64];
	_itoa_s(iDefault, sInt, sizeof(sInt), 10);
	return this->AddOptional(sName, iType, sInt);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CReqParams::AddOptional(const char *sName, int iType, const char *sDefault)
{
	if (sDefault)
	{
		return this->AddOptional(sName, iType, STRICT_TYPE_INVALID, false, sDefault, (int)strlen(sDefault));
	}
	else {
		return this->AddOptional(sName, iType, STRICT_TYPE_INVALID, false, NULL, 0);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CReqParams::AddOptional(const char *sName, int iType, int iStrictType, bool bByReference, const char *sDefault, int iDefaultLength)
{
	this->Collection.Items = (REQPARAMITEM *)
		pMem->ReAllocate(this->Collection.Items, sizeof(REQPARAMITEM), this->Collection.Count + 1);
	if (!this->Collection.Items)
	{
		return this->Error->Hard("Memory allocation error");
	}

	REQPARAMITEM *pItem = &this->Collection.Items[this->Collection.Count++];

	int iLength = (int)strlen(sName);
	pItem->Type = iType;
	pItem->StrictType = iStrictType;
	pItem->ByReference = bByReference;
	pItem->Name = (char *)pMem->CloneString(sName);

	pItem->IsOptional = true;
	pItem->Default.Length = iDefaultLength;
	pItem->Default.Value = NULL;

	if (sDefault && pItem->Default.Length > 0)
	{
		pItem->Default.Value = (char *)pMem->Allocate(1, iDefaultLength + 1);
		memcpy_s(pItem->Default.Value, iDefaultLength + 1, sDefault, iDefaultLength);
	}

	return this->Error->HardCount();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CReqParams::Add(CReqParams *pReqParams)
{
	for (int iItem = 0; iItem < pReqParams->Collection.Count; iItem++)
	{
		if (pReqParams->Collection.Items[iItem].IsOptional)
		{
			if (this->AddOptional(pReqParams->Collection.Items[iItem].Name,
				pReqParams->Collection.Items[iItem].Type,
				pReqParams->Collection.Items[iItem].StrictType,
				pReqParams->Collection.Items[iItem].ByReference,
				pReqParams->Collection.Items[iItem].Default.Value,
				pReqParams->Collection.Items[iItem].Default.Length) != ERROR_OK)
			{
				return this->Error->HardCount();
			}
		}
		else {
			if (this->Add(pReqParams->Collection.Items[iItem].Name,
				pReqParams->Collection.Items[iItem].Type,
				pReqParams->Collection.Items[iItem].StrictType,
				pReqParams->Collection.Items[iItem].ByReference,
				pReqParams->Collection.Items[iItem].IsArray) != ERROR_OK)
			{
				return this->Error->HardCount();
			}
		}

		REQPARAMITEM *pParam = &this->Collection.Items[this->Collection.Count - 1];
		if (strcmp(pReqParams->Collection.Items[iItem].Name, PARENT_INDEX_VAR_NAME) == 0)
		{
			pParam->ByReference = true;
		}
	}

	return this->Error->HardCount();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CReqParams::CReqParams(void *lpEngine, CReqParams *pReqParams)
{
	this->pEngine = lpEngine;
	this->Error = ((CEngine *)lpEngine)->Error;

	memset(&this->Collection, 0, sizeof(this->Collection));

	this->Add(pReqParams);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CReqParams::CReqParams(void *lpEngine)
{
	this->pEngine = lpEngine;
	this->Error = ((CEngine *)lpEngine)->Error;

	memset(&this->Collection, 0, sizeof(this->Collection));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CReqParams::~CReqParams()
{
	if (this->Collection.Count)
	{
		for (int iItem = 0; iItem < this->Collection.Count; iItem++)
		{
			if (this->Collection.Items[iItem].Name)
			{
				pMem->Free(this->Collection.Items[iItem].Name);
			}
			if (this->Collection.Items[iItem].Default.Value)
			{
				pMem->Free(this->Collection.Items[iItem].Default.Value);
			}
		}
		pMem->Free(this->Collection.Items);
	}
	memset(&this->Collection, 0, sizeof(this->Collection));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

