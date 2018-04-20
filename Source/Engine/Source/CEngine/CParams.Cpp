#ifndef _CParam_CPP_
#define _CParam_CPP_
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>

#include "CEngine.H"
#include "CParams.H"

#include "../../../../NSWFL/NSWFL.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::String;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CParams::Add(PARAMITEM *pParam)
{
	if (this->Type != VAR_TYPE_INVALID && this->EnforceTypes)
	{
		if (this->Type == VAR_TYPE_NUMERIC && !IsNumeric(pParam->Value, pParam->Length))
		{
			return this->Error->Hard("Numeric conversion error");
		}
	}

	this->Collection.Items = (PARAMITEM *)
		pMem->ReAllocate(this->Collection.Items, sizeof(PARAMITEM), this->Collection.Count + 1);
	if (!this->Collection.Items)
	{
		return this->Error->Hard("Memory allocation error");
	}

	PARAMITEM *pItem = &this->Collection.Items[this->Collection.Count++];

	pItem->Length = pParam->Length;
	if (pParam->Value)
	{
		if (!(pItem->Value = (char *) pMem->Allocate(sizeof(char), pParam->Length + 1)))
		{
			this->Error->Hard("Memory allocation error");
		}
		memcpy_s(pItem->Value, pParam->Length + 1, pParam->Value, pParam->Length);
	}
	else {
		pItem->Value = NULL;
	}

	return ERROR_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CParams::Add(double dValue, int iDecimalPlaces)
{
	if (this->Type != VAR_TYPE_INVALID && this->EnforceTypes)
	{
		if (this->Type != VAR_TYPE_NUMERIC && this->Type != VAR_TYPE_STRING)
		{
			return this->Error->Hard("Numeric conversion error");
		}
	}

	char sDbl[512];
	if (_gcvt_s(sDbl, sizeof(sDbl), dValue, iDecimalPlaces))
	{
		return this->Error->Hard("Numeric conversion error");
	}

	return this->Add(sDbl);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CParams::Add(double dValue)
{
	if (this->Type != VAR_TYPE_INVALID && this->EnforceTypes)
	{
		if (this->Type != VAR_TYPE_NUMERIC && this->Type != VAR_TYPE_STRING)
		{
			return this->Error->Hard("Numeric conversion error");
		}
	}

	char sDbl[256];
	if (_gcvt_s(sDbl, sizeof(sDbl), dValue, 16))
	{
		return this->Error->Hard("Numeric conversion error");
	}

	int iLength = (int)strlen(sDbl);

	if (sDbl[iLength - 1] == '.')
	{
		//%%__|JMP 12/22/2010
		//	This was set to remove the decimal if there were only zeros after it.
		//	It now reduces it down to one zero. 1.0000 becomes 1.0
		sDbl[iLength++] = '0';
		sDbl[iLength] = '\0';
	}

	return this->Add(sDbl, iLength);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CParams::Add(unsigned int iValue)
{
	if (this->Type != VAR_TYPE_INVALID && this->EnforceTypes)
	{
		if (this->Type != VAR_TYPE_NUMERIC && this->Type != VAR_TYPE_STRING)
		{
			return this->Error->Hard("Numeric conversion error");
		}
	}

	char sInt[64];
	if (_ultoa_s(iValue, sInt, sizeof(sInt), 10) != 0)
	{
		return this->Error->Hard("Numeric conversion error");
	}

	return this->Add(sInt);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CParams::AddIndex(int iObjectIndex, int iMemberIndex)
{
	if (this->Type != VAR_TYPE_INVALID && this->EnforceTypes)
	{
		if (this->Type != VAR_TYPE_NUMERIC && this->Type != VAR_TYPE_STRING)
		{
			return this->Error->Hard("Numeric conversion error");
		}
	}

	char sInt[65];
	if (_itoa_s(iObjectIndex, sInt + 1, sizeof(sInt) - 1, 10) != 0)
	{
		return this->Error->Hard("Numeric conversion error");
	}
	sInt[0] = '@';

	if (iMemberIndex >= 0)
	{
		int iLength = strlen(sInt);

		sInt[iLength++] = '.'; //Variable index and menber index are seperated by a '.' because it alows the overall value to remain numeric.

		if (_itoa_s(iMemberIndex, sInt + iLength, sizeof(sInt) - iLength, 10) != 0)
		{
			return this->Error->Hard("Numeric conversion error");
		}
	}

	return this->Add(sInt);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CParams::AddIndex(int iObjectIndex)
{
	if (this->Type != VAR_TYPE_INVALID && this->EnforceTypes)
	{
		if (this->Type != VAR_TYPE_NUMERIC && this->Type != VAR_TYPE_STRING)
		{
			return this->Error->Hard("Numeric conversion error");
		}
	}

	char sInt[65];
	if (_itoa_s(iObjectIndex, sInt + 1, sizeof(sInt) - 1, 10) != 0)
	{
		return this->Error->Hard("Numeric conversion error");
	}

	sInt[0] = '@';

	return this->Add(sInt);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CParams::Add(int iValue)
{
	if (this->Type != VAR_TYPE_INVALID && this->EnforceTypes)
	{
		if (this->Type != VAR_TYPE_NUMERIC && this->Type != VAR_TYPE_STRING)
		{
			return this->Error->Hard("Numeric conversion error");
		}
	}

	char sInt[64];
	if (_itoa_s(iValue, sInt, sizeof(sInt), 10) != 0)
	{
		return this->Error->Hard("Numeric conversion error");
	}

	return this->Add(sInt);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CParams::Add(const char *sText)
{
	/*
	if(this->Type != VAR_TYPE_INVALID && this->EnforceTypes)
	{
		//We do not check this value for type appropriateness because it is checked in all called functions.
	}
	*/

	if (sText)
	{
		return this->Add(sText, (int)strlen(sText));
	}
	else {
		return this->Add(sText, 0);
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PARAMITEM *CParams::Add(void)
{
	/*
	if(this->Type != VAR_TYPE_INVALID && this->EnforceTypes)
	{
		//We do not check this value for type appropriateness.
	}
	*/

	this->Collection.Items = (PARAMITEM *)
		pMem->ReAllocate(this->Collection.Items, sizeof(PARAMITEM), this->Collection.Count + 1);
	if (!this->Collection.Items)
	{
		this->Error->Hard("Memory allocation error");
		return NULL;
	}

	PARAMITEM *pItem = &this->Collection.Items[this->Collection.Count++];

	pItem->Length = 0;
	pItem->Value = NULL;

	return pItem;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CParams::Add(const char *sText, int iLength)
{
	if (this->Type != VAR_TYPE_INVALID && this->EnforceTypes)
	{
		if (this->Type != VAR_TYPE_STRING)
		{
			return this->Error->Hard("Numeric conversion error");
		}
	}

	this->Collection.Items = (PARAMITEM *)
		pMem->ReAllocate(this->Collection.Items, sizeof(PARAMITEM), this->Collection.Count + 1);
	if (!this->Collection.Items)
	{
		return this->Error->Hard("Memory allocation error");
	}

	PARAMITEM *pItem = &this->Collection.Items[this->Collection.Count++];

	pItem->Length = iLength;
	if (sText)
	{
		if (!(pItem->Value = (char *) pMem->Allocate(sizeof(char), iLength + 1)))
		{
			this->Error->Hard("Memory allocation error");
		}
		memcpy_s(pItem->Value, iLength + 1, sText, iLength);
	}
	else {
		pItem->Value = NULL;
	}

	return this->Error->HardCount();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char *CParams::ToString(int iIndex)
{
	if (this->Collection.Count > iIndex)
	{
		return this->Collection.Items[iIndex].Value;
	}
	else {
		this->Error->Hard("Invalid variable");
		return NULL;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

double CParams::ToDouble(int iIndex)
{
	if (this->Collection.Count > iIndex)
	{
		return atof(this->Collection.Items[iIndex].Value);
	}
	else {
		this->Error->Hard("Invalid variable");
		return NULL;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned int CParams::ToUInt(int iIndex)
{
	if (this->Collection.Count > iIndex)
	{
		return (unsigned int)strtoul(this->Collection.Items[iIndex].Value, NULL, 0);
	}
	else {
		this->Error->Hard("Invalid variable");
		return NULL;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CParams::ToBool(int iIndex)
{
	if (this->Collection.Count > iIndex)
	{
		return atoi(this->Collection.Items[iIndex].Value) != 0;
	}
	else {
		this->Error->Hard("Invalid variable");
		return NULL;
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

size_t CParams::ToIntPtr(int iIndex)
{
	if (this->Collection.Count > iIndex)
	{
		return (size_t)_atoi64(this->Collection.Items[iIndex].Value);
	}
	else {
		this->Error->Hard("Invalid variable");
		return NULL;
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CParams::ToInt(int iIndex)
{
	if (this->Collection.Count > iIndex)
	{
		return atoi(this->Collection.Items[iIndex].Value);
	}
	else {
		this->Error->Hard("Invalid variable");
		return NULL;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CParams::AssessContentType(int iIndex)
{
	return this->AssessContentType(this->GetPointer(iIndex));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CParams::AssessContentType(PARAMITEM *pParam)
{
	if (!pParam)
	{
		return this->Error->Hard("Undefined parameter");
	}

	if (IsNumeric(pParam->Value, pParam->Length))
	{
		if (CharIndex(pParam->Value, pParam->Length, '.') >= 0)
		{
			return STRICT_TYPE_DOUBLE;
		}
		return STRICT_TYPE_INTEGER32;
	}
	else {
		return STRICT_TYPE_STRING;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PARAMITEM *CParams::First(void)
{
	if (this->Collection.Count >= 1)
	{
		return &this->Collection.Items[0];
	}
	else {
		return NULL;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PARAMITEM *CParams::Second(void)
{
	if (this->Collection.Count >= 2)
	{
		return &this->Collection.Items[1];
	}
	else {
		return NULL;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PARAMITEM *CParams::Third(void)
{
	if (this->Collection.Count >= 3)
	{
		return &this->Collection.Items[2];
	}
	else {
		return NULL;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CParams::Clear(void)
{
	this->Destroy();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CParams::IsDefined(int iIndex)
{
	return (iIndex >= 0 && iIndex < this->Collection.Count);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PARAMITEM *CParams::GetPointer(int iIndex)
{
	if (this->IsDefined(iIndex))
	{
		return &this->Collection.Items[iIndex];
	}

	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CParams::CParams(void *lpEngine, int iRequiredType)
{
	this->pEngine = lpEngine;
	this->Error = ((CEngine *)lpEngine)->Error;
	this->Type = iRequiredType;
	this->EnforceTypes = false;

	memset(&this->Collection, 0, sizeof(this->Collection));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CParams::CParams(void *lpEngine)
{
	this->pEngine = lpEngine;
	this->Error = ((CEngine *)lpEngine)->Error;
	this->Type = VAR_TYPE_INVALID;
	this->EnforceTypes = false;

	memset(&this->Collection, 0, sizeof(this->Collection));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CParams::Destroy(void)
{
	if (this->Collection.Count)
	{
		for (int iItem = 0; iItem < this->Collection.Count; iItem++)
		{
			if (this->Collection.Items[iItem].Value)
			{
				pMem->Free(this->Collection.Items[iItem].Value);
			}
		}
		pMem->Free(this->Collection.Items);
	}
	memset(&this->Collection, 0, sizeof(this->Collection));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CParams::~CParams()
{
	this->Destroy();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Returns: Numeric index of procedure or variable.
*/

int CParams::ToObjectIndex(int iIndex)
{
	return this->ToObjectIndex(iIndex, NULL);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Returns: Numeric index of procedure or variable.
*/

int CParams::ToObjectIndex(int iIndex, int *iMemberIndex)
{
	if (this->Collection.Items[iIndex].Value[0] == '@')
	{
		int iDecimalPosition = CharIndex(this->Collection.Items[iIndex].Value, this->Collection.Items[iIndex].Length, '.');
		if (iDecimalPosition >= 0)
		{
			this->Collection.Items[iIndex].Value[iDecimalPosition] = '\0';
		}

		int iObjectIndex = atol(this->Collection.Items[iIndex].Value + 1);
		if (iDecimalPosition >= 0)
		{
			if (iMemberIndex != NULL)
			{
				*iMemberIndex = atol(this->Collection.Items[iIndex].Value + iDecimalPosition + 1);
			}
			else {
				this->Error->Hard("Member not requested or feature not implemented");
			}
		}
		else {
			if (iMemberIndex != NULL)
			{
				*iMemberIndex = -1;
			}
		}

		if (iDecimalPosition >= 0)
		{
			this->Collection.Items[iIndex].Value[iDecimalPosition] = '.';
		}

		return iObjectIndex;
	}
	else {
		//return atol(this->Collection.Items[iIndex].Value);
		return INVALID_INDEX;
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Returns: Numeric index of root-variable.
*/
int CParams::ToVarIndex(int iIndex)
{
	int iVarIndex = this->ToObjectIndex(iIndex);
	if (iVarIndex != INVALID_INDEX)
	{
		VARIABLEITEM *pVar = NULL;
		TYPEITEM *pType = NULL;
		bool bParsingForRoot = false;

		do {
			/*
			if(iVarIndex == 58)
			{
				printf("dbg");
			}
			*/
			pVar = ((CEngine*)this->pEngine)->Vars->GetPointer(iVarIndex);
			/*
			if(pVar == NULL)
			{
				printf("dbg");
			}
			*/
			pType = ((CEngine*)this->pEngine)->Types->GetPointer(pVar->Type);

			bParsingForRoot = false;
			if (pType->IsComplex && pVar->Length > 0 && pVar->Value[0] == '@')
			{
				iVarIndex = atol(pVar->Value + 1);
				bParsingForRoot = true;
			}

		} while (bParsingForRoot);

		return iVarIndex;
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Returns: [PROCITEM *]
*/
void *CParams::ToProcIndexPointer(int iIndex)
{
	int iProcIndex = this->ToObjectIndex(iIndex);
	if (iProcIndex != INVALID_INDEX)
	{
		return ((CEngine*)this->pEngine)->Procs->GetPointer(iProcIndex);
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Returns: [VARIABLEITEM *]
*/
void *CParams::ToVarIndexPointer(int iIndex)
{
	int iVarIndex = this->ToObjectIndex(iIndex);
	if (iVarIndex != INVALID_INDEX)
	{
		return ((CEngine*)this->pEngine)->Vars->GetPointer(iVarIndex);
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

