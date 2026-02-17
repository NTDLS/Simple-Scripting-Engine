#ifndef _CScriptTypes_CPP_
#define _CScriptTypes_CPP_
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>

#include "../../../../Shared/NSWFL/NSWFL.h"
#include "../../../../Shared/CMathParser/CMathParser.H"

#include "../Entry.H"

#include "CEngine.H"
#include "CScriptTypes.H"
#include "CScripts.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::String;
using namespace NSWFL::Conversion;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CScriptTypes::Count(void)
{
	return this->Collection.Count;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEMEMBERITEM *GetMember(TYPEITEM *pType, int iIndex)
{
	return &pType->Members.Items[iIndex];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEMEMBERITEM *CScriptTypes::GetMember(TYPEITEM *pType, const char *sMemberName)
{
	for (int i = 0; i < pType->Members.Count; i++)
	{
		if (_StrEql(sMemberName, pType->Members.Items[i].Name))
		{
			return &pType->Members.Items[i];
		}
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CScriptTypes::CheckTypeName(const char *sName, int iLength)
{
	if (iLength >= MAX_TYPE_NAME_SIZE)
	{
		return this->Error->Hard("Type name too long");
	}
	else if (!((sName[0] >= 'a' && sName[0] <= 'z') || (sName[0] >= 'A' && sName[0] <= 'Z') || sName[0] == '_'))
	{
		return this->Error->Hard("Type names cannot begin with [%c]", sName[0]);
	}
	return this->Error->HardCount();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEITEM *CScriptTypes::Add(const char *sName)
{
	if (this->IsDefined(sName))
	{
		this->Error->Hard("Type redefiniton [%s]", sName);
		return NULL;
	}

	TYPEITEM *pType = this->GetFreePointer();
	if (pType)
	{
		pType->Name = pMem->CloneString(sName);
		pType->Index = this->Collection.Count - 1;
	}

	return pType;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CScriptTypes::AddStrictType()
{
	CEngine* pEngine = (CEngine*) this->pEngine;

	char sTok[MAX_TOKEN_SIZE];
	char sTypeName[MAX_NAME_SIZE];
	char sMemberName[MAX_VAR_NAME_SIZE];
	int iTokSz = 0;
	int iRPos = 0;
	int iStrictVarType = STRICT_TYPE_INVALID;
	int iLooseType = VAR_TYPE_INVALID;
	int iVarType = VAR_TYPE_INVALID;
	int iSizeInBytes = 0;
	bool bIsContiguous = false;

	LINEITEM *pLine = pEngine->Lines->GetPointer(pEngine->State->LineIndex);

	if (Token(pLine->Text, pLine->Length, sTok, sizeof(sTok), &iRPos) <= 0)
	{
		return this->Error->Hard("Expected: [Type]");
	}

	if (Token(pLine->Text, pLine->Length, sTok, sizeof(sTok), &iRPos) <= 0)
	{
		return this->Error->Hard("Expected: [Type Name]");
	}

	TYPEITEM *pType = this->Add(sTok);
	pType->IsStrictType = true;
	pType->IsCustomType = true;

	for (int iLineIndex = pEngine->State->LineIndex + 1; iLineIndex < pEngine->Lines->Collection.Count; iLineIndex++)
	{
		pLine = pEngine->Lines->GetPointer(iLineIndex);
		if (pLine->CodeBlockLevel > 0)
		{
			bIsContiguous = false;
			iRPos = 0;
			if (Token(pLine->Text, pLine->Length, sMemberName, sizeof(sMemberName), &iRPos) <= 0)
			{
				return this->Error->Hard("Expected: [Type Member Name]");
			}

			if (_StrEqlI(sMemberName, "End"))
			{
				int iTempRPos = iRPos;
				if (Token(pLine->Text, pLine->Length, sTok, sizeof(sTok), &iTempRPos) > 0)
				{
					if (_StrEqlI(sTok, "StrictType"))
					{
						//Were done parsing the type;
						return this->Error->HardCount();
					}
				}
			}

			if (Token(pLine->Text, pLine->Length, sTok, sizeof(sTok), &iRPos) <= 0)
			{
				return this->Error->Hard("Expected: [as]");
			}

			if (TokenWithDot(pLine->Text, pLine->Length, sTypeName, sizeof(sTypeName), &iRPos) <= 0)
			{
				return this->Error->Hard("Expected: [Member Type]");
			}

			if ((iStrictVarType = pEngine->Vars->TranslateStrictType(sTypeName)) == STRICT_TYPE_INVALID)
			{
				return this->Error->Hard("Expected: [Valid Type Name]");
			}

			if (iStrictVarType == STRICT_TYPE_GENERIC)
			{
				int iMemberType = pEngine->Types->GetIndex(sTypeName);
				iLooseType = iMemberType;
				iSizeInBytes = pEngine->Types->GetPointer(iMemberType)->Members.SizeInBytes;
			}
			else {
				iLooseType = pEngine->Vars->StrictTypeToLoseType(iStrictVarType);
				iSizeInBytes = pEngine->Vars->StrictTypeSize(iStrictVarType);
			}

			//Get instance count
			if (pLine->Text[iRPos] == '(')
			{
				//When an instance count is specified, the memory for this member will be allocated as part of
				//	the whole structure - making this member contiguous - and requiring that it's memory not
				//	be freed as a normal pointer member (emulated static allocation).
				bIsContiguous = true;

				iRPos++;

				//Strings are stored as pointers, but if this is an "array of string" then we convery it to (bytes*n).
				if (iStrictVarType == STRICT_TYPE_STRING)
				{
					iSizeInBytes = pEngine->Vars->StrictTypeSize(STRICT_TYPE_BYTE);
				}

				if (Token(pLine->Text, pLine->Length, sTok, sizeof(sTok), &iRPos) <= 0)
				{
					return this->Error->Hard("Expected: [Instance Count]");
				}

				int iInstances = atol(sTok);
				if (iInstances > 0)
				{
					iSizeInBytes *= iInstances;
				}
				else {
					return this->Error->Hard("Expected: [valid Instance Count]");
				}

				if (pLine->Text[iRPos] != ')')
				{
					return this->Error->Hard("Expected: \"(\"");
				}
				iRPos++;
			}

			pType->Members.Items = (TYPEMEMBERITEM *) pMem->ReAllocate(pType->Members.Items, sizeof(TYPEMEMBERITEM), pType->Members.Count + 1);

			TYPEMEMBERITEM *pTypeMember = &pType->Members.Items[pType->Members.Count++];
			pTypeMember->Name = pMem->CloneString(sMemberName);
			pTypeMember->Type = iLooseType;
			pTypeMember->SizeInBytes = iSizeInBytes;
			pTypeMember->ByteOffset = pType->Members.SizeInBytes;
			pTypeMember->StrictType = iStrictVarType;
			pTypeMember->IsContiguous = bIsContiguous;
			pTypeMember->Index = pType->Members.Count - 1;
			pTypeMember->ParentIndex = pType->Index;

			pType->Members.SizeInBytes += pTypeMember->SizeInBytes;

			//printf("Custom Type: [%s] member [%s] is type: [%s] (%d bytes).\n",
			//	pType->Name, sMemberName, this->Name(iVarType), pEngine->Vars->StrictTypeSize(iStrictVarType));
		}
	}

	return this->Error->HardCount();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CScriptTypes::IsDefined(const char *sName)
{
	return this->IsDefined(this->GetIndex(sName));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CScriptTypes::IsDefined(int iIndex)
{
	return (iIndex >= 0 && iIndex < this->Collection.Count && this->Collection.Items[iIndex].Name);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const char *CScriptTypes::Name(int iIndex)
{
	if (this->IsDefined(iIndex))
	{
		return this->Collection.Items[iIndex].Name;
	}

	return this->Collection.Items[0].Name;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEITEM *CScriptTypes::GetPointer(int iIndex)
{
	if (iIndex != VAR_TYPE_INVALID)
	{
		if (this->IsDefined(iIndex))
		{
			return &this->Collection.Items[iIndex];
		}
	}

	return &this->Collection.Items[0];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEITEM *CScriptTypes::GetPointer(const char *sName)
{
	for (int iItem = 0; iItem < this->Collection.Count; iItem++)
	{
		if (this->Collection.Items[iItem].Name)
		{
			if (_StrEqlI(this->Collection.Items[iItem].Name, sName))
			{
				return &this->Collection.Items[iItem];
			}
		}
	}

	return &this->Collection.Items[0];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

VarTypeIndex CScriptTypes::GetIndex(const char *sName)
{
	if (!_StrEqlI(sName, INVALID_TYPE_NAME))
	{
		for (int iItem = 0; iItem < this->Collection.Count; iItem++)
		{
			if (this->Collection.Items[iItem].Name)
			{
				if (_StrEqlI(this->Collection.Items[iItem].Name, sName))
				{
					return iItem;
				}
			}
		}
	}
	return VAR_TYPE_INVALID;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CScriptTypes::Free(TYPEITEM *pItem)
{
	if (!pItem)
	{
		return this->Error->Hard("Undefined Type");
	}

	pMem->Free(pItem->Name);

	if (pItem->Members.Count > 0)
	{
		for (int iMember = 0; iMember < pItem->Members.Count; iMember++)
		{
			pMem->Free(pItem->Members.Items[iMember].Name);
		}
		pMem->Free(pItem->Members.Items);
	}

	return this->Error->HardCount();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

VarTypeIndex CScriptTypes::GetFreeIndex(void)
{
	this->Collection.Items = (TYPEITEM *)
		pMem->ReAllocate(this->Collection.Items, sizeof(TYPEITEM), this->Collection.Count + 1);
	if (!this->Collection.Items)
	{
		this->Error->Hard("Memory allocation error");
		return INVALID_INDEX;
	}

	memset(&this->Collection.Items[this->Collection.Count], 0, sizeof(TYPEITEM));

	return this->Collection.Count++;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEITEM *CScriptTypes::GetFreePointer(void)
{
	this->Collection.Items = (TYPEITEM *)
		pMem->ReAllocate(this->Collection.Items, sizeof(TYPEITEM), this->Collection.Count + 1);
	if (!this->Collection.Items)
	{
		this->Error->Hard("Memory allocation error");
		return NULL;
	}

	memset(&this->Collection.Items[this->Collection.Count], 0, sizeof(TYPEITEM));

	return &this->Collection.Items[this->Collection.Count++];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CScriptTypes::IsReadOnly(const char *sName)
{
	TYPEITEM *pType = this->GetPointer(sName);

	if (pType->IsBuiltInClass)
	{
		//Even built in classes are not read-only in (Code.Scope <= 0).
		//	This is so system level access can write these variables.
		return ((CEngine *)this->pEngine)->State->CurrentScope() > 0;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CScriptTypes::IsReadOnly(int iIndex)
{
	TYPEITEM *pType = this->GetPointer(iIndex);

	if (pType->IsBuiltInClass)
	{
		//Even built in classes are not read-only in (Code.Scope <= 0).
		//	This is so system level access can write these variables.
		return ((CEngine *)this->pEngine)->State->CurrentScope() > 0;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CScriptTypes::CScriptTypes(void *lpEngine)
{
	this->pEngine = lpEngine;
	this->Error = ((CEngine *)lpEngine)->Error;

	memset(&this->Collection, 0, sizeof(this->Collection));

	TYPEITEM *pType = NULL;

	pType = this->Add(INVALID_TYPE_NAME);
	pType->IsBuiltIn = true;
	VAR_TYPE_INVALID = INVALID_INDEX;

	pType = this->Add("String");
	pType->IsBuiltIn = true;
	VAR_TYPE_STRING = this->Collection.Count - 1;

	pType = this->Add("Bytes");
	pType->IsBuiltIn = true;
	VAR_TYPE_BYTES = this->Collection.Count - 1;

	pType = this->Add("Numeric");
	pType->IsBuiltIn = true;
	VAR_TYPE_NUMERIC = this->Collection.Count - 1;

	pType = this->Add("SQL.Connection");
	pType->IsBuiltIn = true;
	pType->IsComplex = true;
	pType->IsBuiltInClass = true;
	VAR_TYPE_SQLCONNECTION = this->Collection.Count - 1;

	pType = this->Add("SQL.RecordSet");
	pType->IsBuiltIn = true;
	pType->IsComplex = true;
	pType->IsBuiltInClass = true;
	VAR_TYPE_SQLRECORDSET = this->Collection.Count - 1;

	pType = this->Add("File");
	pType->IsBuiltIn = true;
	pType->IsComplex = true;
	VAR_TYPE_FILE = this->Collection.Count - 1;

	pType = this->Add("List");
	pType->IsBuiltIn = true;
	pType->IsComplex = true;
	pType->IsBuiltInClass = true;
	VAR_TYPE_LIST = this->Collection.Count - 1;

	pType = this->Add("Generic"); //Used for built-in namespaces/classes (eg: Code., Sys., Env.).
	pType->IsBuiltIn = true;
	pType->IsComplex = false;
	VAR_TYPE_GENERIC = this->Collection.Count - 1;

#ifdef COM_PROCEDURE_CALL_IMPLEMENTATION
	pType = this->Add("COM.Object");
	pType->IsBuiltIn = true;
	pType->IsComplex = false;
	VAR_TYPE_COM_OBJECT = this->Collection.Count - 1;
#endif //COM_PROCEDURE_CALL_IMPLEMENTATION

	pType = this->Add("XML.Reader");
	pType->IsBuiltIn = true;
	pType->IsComplex = true;
	pType->IsBuiltInClass = true;
	VAR_TYPE_XMLREADER = this->Collection.Count - 1;

	pType = this->Add("XML.Writer");
	pType->IsBuiltIn = true;
	pType->IsComplex = true;
	pType->IsBuiltInClass = true;
	VAR_TYPE_XMLWRITER = this->Collection.Count - 1;

	pType = this->Add("Socket.Server");
	pType->IsBuiltIn = true;
	pType->IsComplex = true;
	pType->IsBuiltInClass = true;
	VAR_TYPE_SOCKETSERVER = this->Collection.Count - 1;

	pType = this->Add("Socket.Client");
	pType->IsBuiltIn = true;
	pType->IsComplex = true;
	pType->IsBuiltInClass = true;
	VAR_TYPE_SOCKETCLIENT = this->Collection.Count - 1;

	pType = this->Add("Exception");
	pType->IsBuiltIn = true;
	pType->IsComplex = true;
	pType->IsBuiltInClass = true;
	VAR_TYPE_EXCEPTION = this->Collection.Count - 1;

	pType = this->Add("Array");
	pType->IsBuiltIn = true;
	pType->IsComplex = true;
	VAR_TYPE_ARRAY = this->Collection.Count - 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CScriptTypes::TraverseMembers(VARIABLEITEM *pItem, const char *sMemberLineage, MEMBERENDPOINTINFO *lpMemberEndpoint, int *iNonMemberPosition)
{
	CEngine *pEngine = ((CEngine*)this->pEngine);

	char sTok[MAX_TOKEN_SIZE];
	int iTokSz = 0;
	int iRPos = 0;
	int iMemberLineageLen = strlen(sMemberLineage);

	TYPEITEM *pType = pEngine->Types->GetPointer(pItem->Type);

	if ((iTokSz = GetNextToken(sMemberLineage, iMemberLineageLen, sTok, sizeof(sTok), &iRPos, ".", 1)) > 0)
	{
		iRPos++; //Skip the '.'
		lpMemberEndpoint->Member = pEngine->Types->GetMember(pType, sTok);
	}

	if (!lpMemberEndpoint->Member)
	{
		return this->Error->Hard("Member [%s] not found in type [%s]", sMemberLineage, pType->Name);
	}

	while (true)
	{
		lpMemberEndpoint->ByteOffset += lpMemberEndpoint->Member->ByteOffset;

		if (lpMemberEndpoint->Member)
		{
			if (lpMemberEndpoint->Member->StrictType == STRICT_TYPE_GENERIC)
			{
				pType = pEngine->Types->GetPointer(lpMemberEndpoint->Member->Type);
				if (!pType || !pType->IsStrictType)
				{
					this->Error->Hard("Not implemented");
					return INVALID_INDEX;
				}
			}
			else
			{
				if (iNonMemberPosition)
				{
					*iNonMemberPosition = (iMemberLineageLen - (iRPos - 1));
				}
				return ERROR_OK; //No need to drill further, we've found a tangible value.
			}
		}
		else {
			return this->Error->Hard("Not implemented");
		}

		if ((iTokSz = GetNextToken(sMemberLineage, iMemberLineageLen, sTok, sizeof(sTok), &iRPos, ".", 1)) > 0)
		{
			iRPos++; //Skip the '.'
			lpMemberEndpoint->Member = pEngine->Types->GetMember(pType, sTok);

			if (!lpMemberEndpoint->Member)
			{
				return this->Error->Hard("Member [%s] not found in type [%s]", sMemberLineage, pType->Name);
			}
		}
		else {
			return this->Error->Hard("Expected: A valid member token");
		}
	}

	return this->Error->Hard("Malformed member lineage");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CScriptTypes::~CScriptTypes()
{
	if (this->Collection.Count)
	{
		for (int iItem = 0; iItem < this->Collection.Count; iItem++)
		{
			this->Free(&this->Collection.Items[iItem]);
		}
		pMem->Free(this->Collection.Items);
	}
	memset(&this->Collection, 0, sizeof(this->Collection));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
