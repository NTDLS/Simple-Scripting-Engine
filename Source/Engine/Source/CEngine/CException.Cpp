#ifndef _CException_CPP_
#define _CException_CPP_
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>

#include "CException.H"
#include "CEngine.H"

#include "../../../../Shared/NSWFL/NSWFL.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CException::CException(void *lpEngine)
{
	this->pEngine = lpEngine;
	this->_bAbsorbExceptions = false;
	memset(&this->Collection, 0, sizeof(this->Collection));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CException::~CException()
{
	if (this->Collection.Count)
	{
		for (int iItem = 0; iItem < this->Collection.Count; iItem++)
		{
			pMem->Free(this->Collection.Items[iItem].Message);
		}
		pMem->Free(this->Collection.Items);
	}
	memset(&this->Collection, 0, sizeof(this->Collection));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CException::AbsorbExceptions(void)
{
	this->_iAdsorbedHardCount = 0;
	this->_iAdsorbedSoftCount = 0;
	return this->_bAbsorbExceptions;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CException::AbsorbExceptions(bool bIgnore)
{
	this->_iAdsorbedHardCount = 0;
	this->_iAdsorbedSoftCount = 0;
	this->_bAbsorbExceptions = bIgnore;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const char *CException::Text(int iIndex)
{
	if (iIndex >= 0 && iIndex < this->Collection.Count)
	{
		return this->Collection.Items[iIndex].Message;
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CException::Line(int iIndex)
{
	EXCEPTIONITEM *pException = this->GetPointer(iIndex);

	if (pException && pException->LineIndex >= 0)
	{
		CEngine *pEngine = ((CEngine *)this->pEngine);
		if (pEngine->Lines)
		{
			LINEITEM *pLine = pEngine->Lines->GetPointer(pException->LineIndex);
			if (pLine)
			{
				return pLine->FileLine;
			}
		}
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const char *CException::File(int iIndex)
{
	EXCEPTIONITEM *pException = this->GetPointer(iIndex);

	if (pException && pException->LineIndex >= 0)
	{
		CEngine *pEngine = ((CEngine *)this->pEngine);
		if (pEngine->Lines)
		{
			LINEITEM *pLine = pEngine->Lines->GetPointer(pException->LineIndex);
			if (pLine && pEngine->Scripts)
			{
				SCRIPTITEM *pScript = pEngine->Scripts->GetPointer(pLine->ScriptIndex);
				if (pScript)
				{
					return pScript->FileName;
				}
			}
		}
	}
	return "";
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EXCEPTIONITEM *CException::GetPointer(int iIndex)
{
	if (iIndex >= 0 || iIndex < this->Collection.Count)
	{
		return &this->Collection.Items[iIndex];
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CException::Clear(void)
{
	int iCleared = 0;

	if (this->Collection.Count)
	{
		for (int iItem = 0; iItem < this->Collection.Count; iItem++)
		{
			pMem->Free(this->Collection.Items[iItem].Message);
		}
		pMem->Free(this->Collection.Items);
		iCleared++;
	}
	memset(&this->Collection, 0, sizeof(this->Collection));

	return iCleared;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CException::Throw(void)
{
	return this->Throw(false);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CException::Throw(bool bThrowAll)
{
	CEngine *pEngine = ((CEngine *)this->pEngine);

	int iThrown = 0;

	if (this->Collection.Count)
	{
		for (int iItem = 0; iItem < this->Collection.Count; iItem++)
		{
			EXCEPTIONITEM *Item = &this->Collection.Items[iItem];

			if (Item->Type == ERROR_HARD || bThrowAll)
			{
				iThrown++;
				if (pEngine->Lines)
				{
					LINEITEM *pLine = pEngine->Lines->GetPointer(Item->LineIndex);
					if (pLine && pEngine->Scripts)
					{
						SCRIPTITEM *pScript = pEngine->Scripts->GetPointer(pLine->ScriptIndex);

						if (pScript)
						{
							pEngine->ErrorOut("%d\t%s\t%d\t%s\n",
								iItem + 1, pScript->FileName, pLine->FileLine, Item->Message);
						}
						else {
							pEngine->ErrorOut("%d\t%s\t%d\t%s\n",
								iItem + 1, "", pLine->FileLine, Item->Message);
						}
					}
					else if (pLine) {
						pEngine->ErrorOut("%d\t%s\t%d\t%s\n",
							iItem + 1, "", pLine->FileLine, Item->Message);
					}
					else {
						pEngine->ErrorOut("%d\t%s\t%d\t%s\n",
							iItem + 1, "", 0, Item->Message);
					}
				}
				else {
					pEngine->ErrorOut("%d\t%s\t%d\t%s\n",
						iItem + 1, "", 0, Item->Message);
				}
			}
			pMem->Free(Item->Message);
			memset(&this->Collection.Items[iItem], 0, sizeof(EXCEPTIONITEM));
		}

		pMem->Free(this->Collection.Items);
		this->Collection.Items = NULL;
		this->Collection.Count = 0;
		this->Collection.HardCount = 0;
		this->Collection.SoftCount = 0;
	}

	return iThrown;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CException::AddBase(int iErrorType, const char *sText)
{
/*
#if _DEBUG
	printf("Exception raised: %d\n", ((CEngine *)this->pEngine)->State->CurrentScope());
#endif
*/

	if (this->_bAbsorbExceptions)
	{
		if (iErrorType == ERROR_HARD)
		{
			return ++this->_iAdsorbedHardCount;
		}
		else if (iErrorType == ERROR_SOFT)
		{
			return ++this->_iAdsorbedSoftCount;
		}
	}

	this->Collection.Items = (EXCEPTIONITEM *)
		pMem->ReAllocate(this->Collection.Items, sizeof(EXCEPTIONITEM), (this->Collection.Count + 1));

	if (this->Collection.Items)
	{
		EXCEPTIONITEM *Item = &this->Collection.Items[this->Collection.Count++];
		Item->Message = (char *) pMem->CloneString(sText);
		Item->LineIndex = ((CEngine *)this->pEngine)->State->LineIndex;
		Item->Type = iErrorType;

		if (iErrorType == ERROR_HARD)
		{
			this->Collection.HardCount++;
		}
		else if (iErrorType == ERROR_SOFT)
		{
			this->Collection.SoftCount++;
		}
	}

	return this->Collection.HardCount;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CException::AddBase(int iErrorType, int iLineIndex, const char *sText)
{
	if (this->_bAbsorbExceptions)
	{
		if (iErrorType == ERROR_HARD)
		{
			return ++this->_iAdsorbedHardCount;
		}
		else if (iErrorType == ERROR_SOFT)
		{
			return ++this->_iAdsorbedSoftCount;
		}
	}

	this->Collection.Items = (EXCEPTIONITEM *)
		pMem->ReAllocate(this->Collection.Items, sizeof(EXCEPTIONITEM), (this->Collection.Count + 1));

	if (this->Collection.Items)
	{
		EXCEPTIONITEM *Item = &this->Collection.Items[this->Collection.Count++];
		Item->Message = (char *) pMem->CloneString(sText);
		Item->LineIndex = iLineIndex;
		Item->Type = iErrorType;

		if (iErrorType == ERROR_HARD)
		{
			this->Collection.HardCount++;
		}
		else if (iErrorType == ERROR_SOFT)
		{
			this->Collection.SoftCount++;
		}
	}

	return this->Collection.HardCount;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CException::SoftCount(void)
{
	if (this->_bAbsorbExceptions)
	{
		return this->_iAdsorbedSoftCount;
	}
	return this->Collection.SoftCount;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CException::HardCount(void)
{
	if (this->_bAbsorbExceptions)
	{
		return this->_iAdsorbedHardCount;
	}
	return this->Collection.HardCount;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function will add a hard error, but only if there are no soft errors on the exception stack.
*/
int CException::Inform(const char *sFormat, ...)
{
	if (this->SoftCount() == 0)
	{
		if (this->_bAbsorbExceptions)
		{
			return ++this->_iAdsorbedSoftCount;
		}

		va_list ArgList;
		va_start(ArgList, sFormat);

		int iMemoryRequired = _vscprintf(sFormat, ArgList);

		char *sMsg = (char *) pMem->Allocate(sizeof(char), iMemoryRequired + 1);

		int iMsgLength = _vsprintf_s_l(sMsg, iMemoryRequired + 1, sFormat, NULL, ArgList);
		va_end(ArgList);

		this->AddBase(ERROR_HARD, sMsg);

		pMem->Free(sMsg);
	}

	return this->Collection.HardCount;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CException::Soft(const char *sFormat, ...)
{
	if (this->_bAbsorbExceptions)
	{
		return ++this->_iAdsorbedSoftCount;
	}

	va_list ArgList;
	va_start(ArgList, sFormat);

	int iMemoryRequired = _vscprintf(sFormat, ArgList);

	char *sMsg = (char *) pMem->Allocate(sizeof(char), iMemoryRequired + 1);

	int iMsgLength = _vsprintf_s_l(sMsg, iMemoryRequired + 1, sFormat, NULL, ArgList);
	va_end(ArgList);

	if (this->SoftCount() && ((CEngine*)pEngine)->ThrowSoftExceptions)
	{
		this->AddBase(ERROR_HARD, sMsg);
	}
	else {
		this->AddBase(ERROR_SOFT, sMsg);
	}

	pMem->Free(sMsg);

	return this->Collection.HardCount;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CException::Soft(int iLineIndex, const char *sFormat, ...)
{
	if (this->_bAbsorbExceptions)
	{
		return ++this->_iAdsorbedSoftCount;
	}

	va_list ArgList;
	va_start(ArgList, sFormat);

	int iMemoryRequired = _vscprintf(sFormat, ArgList);

	char *sMsg = (char *) pMem->Allocate(sizeof(char), iMemoryRequired + 1);

	int iMsgLength = _vsprintf_s_l(sMsg, iMemoryRequired + 1, sFormat, NULL, ArgList);
	va_end(ArgList);

	if (this->SoftCount() && ((CEngine*)pEngine)->ThrowSoftExceptions)
	{
		this->AddBase(ERROR_HARD, sMsg);
	}
	else {
		this->AddBase(ERROR_SOFT, sMsg);
	}

	pMem->Free(sMsg);

	return this->Collection.HardCount;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CException::Hard(const char *sFormat, ...)
{
	if (this->_bAbsorbExceptions)
	{
		return ++this->_iAdsorbedHardCount;
	}

	va_list ArgList;
	va_start(ArgList, sFormat);

	int iMemoryRequired = _vscprintf(sFormat, ArgList);

	char *sMsg = (char *) pMem->Allocate(sizeof(char), iMemoryRequired + 1);

	int iMsgLength = _vsprintf_s_l(sMsg, iMemoryRequired + 1, sFormat, NULL, ArgList);
	va_end(ArgList);

	this->AddBase(ERROR_HARD, sMsg);

	pMem->Free(sMsg);

	return this->Collection.HardCount;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CException::LastSoftCollectionIndex(void)
{
	for (int iIndex = this->Collection.Count - 1; iIndex > -1; iIndex--)
	{
		if (this->Collection.Items[iIndex].Type == ERROR_SOFT)
		{
			return iIndex;
		}
	}

	return INVALID_INDEX;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CException::Hard(int iLineIndex, const char *sFormat, ...)
{
	if (this->_bAbsorbExceptions)
	{
		return ++this->_iAdsorbedHardCount;
	}

	va_list ArgList;
	va_start(ArgList, sFormat);

	int iMemoryRequired = _vscprintf(sFormat, ArgList);

	char *sMsg = (char *) pMem->Allocate(sizeof(char), iMemoryRequired + 1);

	int iMsgLength = _vsprintf_s_l(sMsg, iMemoryRequired + 1, sFormat, NULL, ArgList);
	va_end(ArgList);

	this->AddBase(ERROR_HARD, iLineIndex, sMsg);

	pMem->Free(sMsg);

	return this->Collection.HardCount;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
