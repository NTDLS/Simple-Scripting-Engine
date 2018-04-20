#ifndef _CVars_CPP_
#define _CVars_CPP_
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>

#include "../../../../NSWFL/NSWFL.h"
#include "../../../../CMathParser/CMathParser.H"
#include "../../../../CSQLBinding/CSQLEncapsulation.H"

#include "../Entry.H"

#include "CEngine.H"
#include "CVars.H"
#include "CScripts.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::String;
using namespace NSWFL::Conversion;
using namespace NSWFL::Collections;
using namespace NSWFL::XML;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
	When IsBuiltInClass == true, the variable should be instantiated in [CVars::Add(...)] and freed in [CVars::Free(..)]
*/
int VAR_TYPE_INVALID = INVALID_INDEX;
int VAR_TYPE_STRING = INVALID_INDEX;
int VAR_TYPE_BYTES = INVALID_INDEX;
int VAR_TYPE_NUMERIC = INVALID_INDEX;
int VAR_TYPE_SQLCONNECTION = INVALID_INDEX;
int VAR_TYPE_SQLRECORDSET = INVALID_INDEX;
int VAR_TYPE_FILE = INVALID_INDEX;
int VAR_TYPE_LIST = INVALID_INDEX;
int VAR_TYPE_GENERIC = INVALID_INDEX;
#ifdef COM_PROCEDURE_CALL_IMPLEMENTATION
int VAR_TYPE_COM_OBJECT = INVALID_INDEX;
#endif //COM_PROCEDURE_CALL_IMPLEMENTATION
int VAR_TYPE_SOCKETSERVER = INVALID_INDEX;
int VAR_TYPE_SOCKETCLIENT = INVALID_INDEX;
int VAR_TYPE_XMLREADER = INVALID_INDEX;
int VAR_TYPE_XMLWRITER = INVALID_INDEX;
int VAR_TYPE_EXCEPTION = INVALID_INDEX;
int VAR_TYPE_USERDEFINED = INVALID_INDEX;
int VAR_TYPE_ARRAY = INVALID_INDEX;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const char *sMathConstants[] = {
	"bAND", //Bitwise AND
	"AND", //Logical AND
	"XOR", //Bitwise XOR
	"MOD", //Modulo
	"OR", //Logical OR
	"bOR", //bitwise OR
	"NOT", //Logical NOT
	"bNOT", //Bitwise NOT
	"True", //Logical TRUE
	"False", //Logical FALSE
	NULL
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CVars::IsMathConstant(const char *sText)
{
	for (int i = 0; sMathConstants[i]; i++)
	{
		if (_StrEqlI(sMathConstants[i], sText))
		{
			return true;
		}
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CVars::Count(void)
{
	return this->Collection.Count;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CVars::Add(LINEITEM *pLine)
{
	return this->Add(pLine, NULL);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CVars::CheckVariableName(const char *sVarName, int iLength)
{
	if (iLength >= MAX_VAR_NAME_SIZE)
	{
		return this->Error->Hard("Variable name too long");
	}
	else if (!((sVarName[0] >= 'a' && sVarName[0] <= 'z') || (sVarName[0] >= 'A' && sVarName[0] <= 'Z') || sVarName[0] == '_'))
	{
		return this->Error->Hard("Variable names cannot begin with [%c]", sVarName[0]);
	}
	return this->Error->HardCount();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Returns a friendly value for complex types.
*/
bool CVars::FriendlyValue(VARIABLEITEM *pVar, char *sOutput, int iMaxSz)
{
	int iMaxFriendlySz = FRIENDLY_DEBUG_VAR_SIZE;

	if (((CEngine*)this->pEngine)->Types->GetPointer(pVar->Type)->IsBuiltInClass)
	{
		if (pVar->Type == VAR_TYPE_LIST && pVar->Length > 0)
		{
			VectorBinary* pVect = ((VectorBinary*)pVar->Value);

			StringBuilder string;

			string.AppendF("<Count: %d, Allocated: %d>", pVect->Count(), pVect->Allocated());

			string.Terminate();

			int iFriendlyLength = string.Length > iMaxFriendlySz ? iMaxFriendlySz : string.Length;
			strncpy_s(sOutput, iMaxSz, string.Buffer, iFriendlyLength);
			return true;
		}
		else if (pVar->Type == VAR_TYPE_SQLCONNECTION && pVar->Length > 0)
		{
			CSQL *pSQL = (CSQL *)pVar->Value;

			StringBuilder string;

			string.AppendF("<Connected: %s, String: %s>", pSQL->IsConnected() ? "Yes" : "No", pSQL->ConnectionString());

			string.Terminate();

			int iFriendlyLength = string.Length > iMaxFriendlySz ? iMaxFriendlySz : string.Length;
			strncpy_s(sOutput, iMaxSz, string.Buffer, iFriendlyLength);
			return true;
		}
		else if (pVar->Type == VAR_TYPE_EXCEPTION)
		{
			EXCEPTIONITEM *pExcept = ((EXCEPTIONITEM *)pVar->Value);
			if (pExcept)
			{
				int iLength = (int)strlen(pExcept->Message);
				int iFriendlyLength = iLength > iMaxFriendlySz ? iMaxFriendlySz : iLength;
				strncpy_s(sOutput, iMaxSz, pExcept->Message, iFriendlyLength);
				return true;
			}
		}

		char sLength[64];
		sprintf_s(sOutput, iMaxSz, "<%s byte%s %s>",
			FormatInteger(sLength, sizeof(sLength), pVar->Length),
			pVar->Length == 1 ? "" : "s",
			((CEngine*)this->pEngine)->Types->Name(pVar->Type));
	}
	else {
		int iFriendlyLength = pVar->Length > iMaxFriendlySz ? iMaxFriendlySz : pVar->Length;
		strncpy_s(sOutput, iMaxSz, pVar->Value, iFriendlyLength);
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CVars::Add(LINEITEM *pLine, int *iOutVarIndex)
{
	char sVarName[MAX_VAR_NAME_SIZE];
	char sVarType[MAX_TOKEN_SIZE];
	int iVarType = VAR_TYPE_INVALID;

	ARRAYDEFINITION arrayDefinition;

	char sTok[MAX_TOKEN_SIZE];
	int iTokSz = 0;
	int iRPos = 0;
	bool bIsPreProcessor = false;

	if (pLine->Text[iRPos] == '#')
	{
		bIsPreProcessor = true;
		iRPos++; //Parsing preprocessor, skip the '#';
	}

	//Parse the "Var":
	if ((iTokSz = Token(pLine->Text, pLine->Length, sTok, sizeof(sTok), &iRPos)) <= 0)
	{
		return this->Error->Hard("Invalid token");
	}
	if (!_StrEqlI(sTok, "Var") && !_StrEqlI(sTok, "Define"))
	{
		return this->Error->Hard("Expected: [Var]");
	}

	//Parse the variable name:
	if ((iTokSz = Token(pLine->Text, pLine->Length, sTok, sizeof(sTok), &iRPos)) <= 0)
	{
		return this->Error->Hard("Expected: [Variable Name]");
	}
	if (this->CheckVariableName(sTok, iTokSz))
	{
		return this->Error->HardCount();
	}
	strcpy_s(sVarName, sizeof(sVarName), sTok);

	if (!bIsPreProcessor)
	{
		//Parse the "AS":
		if ((iTokSz = Token(pLine->Text, pLine->Length, sTok, sizeof(sTok), &iRPos)) <= 0)
		{
			return this->Error->Hard("Expected: [as]");
		}
		if (!_StrEqlI(sTok, "AS"))
		{
			return this->Error->Hard("Expected: [as]");
		}

		//Parse the variable type:
		if ((iTokSz = TokenWithDot(pLine->Text, pLine->Length, sVarType, sizeof(sVarType), &iRPos)) <= 0)
		{
			return this->Error->Hard("Expected: [Type Name]");
		}

		if ((iVarType = ((CEngine*)this->pEngine)->Types->GetIndex(sVarType)) == VAR_TYPE_INVALID)
		{
			return this->Error->Hard("Expected: [Valid Type Name]");
		}
	}

	SkipWhiteSpaces(pLine->Text, pLine->Length, &iRPos);

	if (pLine->Text[iRPos] == '[')
	{
		//int iOriginalRPos = iRPos;
		int iBegBrace = 0;
		int iEndBrace = 0;

		ErrorLevel result;

		if ((result = ((CEngine*)this->pEngine)->GetSquareBracketIndexPositions(pLine->Text, pLine->Length, &iRPos, &iBegBrace, &iEndBrace)) != ERROR_OK)
		{
			return result;
		}

		int iPreviousSoftErrors = this->Error->SoftCount();

		arrayDefinition.Type = iVarType;
		arrayDefinition.NumberOfElements = ((CEngine*)this->pEngine)->ParseMathI(pLine->Text + iBegBrace, iEndBrace - iBegBrace);

		if (this->Error->SoftCount() != iPreviousSoftErrors)
		{
			return this->Error->HardCount();
		}

		iVarType = VAR_TYPE_ARRAY;

		SkipWhiteSpaces(pLine->Text, pLine->Length, &iRPos);
	}

	int iIndex = INVALID_INDEX;
	if (this->Add(sVarName, iVarType, &arrayDefinition, &iIndex) != ERROR_OK)
	{
		return this->Error->HardCount();
	}

	if (iOutVarIndex)
	{
		*iOutVarIndex = iIndex;
	}

	if (bIsPreProcessor)
	{
		ErrorLevel Result = this->SetByEval(iIndex, pLine->Text + iRPos, pLine->Length - iRPos);

		//Preprocessor's types must be determined after the fact.
		if (IsNumeric(this->Collection.Items[iIndex].Value, this->Collection.Items[iIndex].Length))
		{
			this->Collection.Items[iIndex].Type = VAR_TYPE_NUMERIC;
		}
		else {
			this->Collection.Items[iIndex].Type = VAR_TYPE_STRING;
		}

		return Result;
	}
	else if (pLine->Text[iRPos] == '=' && iRPos != pLine->Length)
	{
		iRPos++; //Skip the equal sign.
		SkipWhiteSpaces(pLine->Text, pLine->Length, &iRPos);

		return this->SetByEval(iIndex, pLine->Text + iRPos, pLine->Length - iRPos);
	}
	else if (iRPos != pLine->Length) {
		return this->Error->Hard("Expected: [End Of Line] or [Initial Value]");
	}
	else if (iVarType == VAR_TYPE_NUMERIC) {
		return this->Set(iIndex, 0); //Default all numerics to Zero.
	}

	return this->Error->HardCount();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CVars::IsDefined(const char *sName)
{
	return this->IsDefined(this->GetIndex(sName));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CVars::IsDefined(int iIndex)
{
	return (iIndex >= 0 && iIndex < this->Collection.Count && this->Collection.Items[iIndex].Name);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

VARIABLEITEM *CVars::GetPointer(int iIndex)
{
	if (this->IsDefined(iIndex))
	{
		return &this->Collection.Items[iIndex];
	}

	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

VARIABLEITEM *CVars::GetPointer(const char *sName)
{
	if (((CEngine*)this->pEngine)->State->Namespace())
	{
		char sNSVar[MAX_TOKEN_SIZE];
		strcpy_s(sNSVar, sizeof(sNSVar), ((CEngine*)this->pEngine)->State->Namespace());
		strcat_s(sNSVar, sizeof(sNSVar), ".");
		strcat_s(sNSVar, sizeof(sNSVar), sName);

		VARIABLEITEM *pVar = this->GetPointerEx(sNSVar);
		if (pVar)
		{
			return pVar;
		}
	}

	return this->GetPointerEx(sName);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

VARIABLEITEM *CVars::GetPointerEx(const char *sName)
{
	for (int iScope = ((CEngine *)this->pEngine)->State->CurrentScope(); iScope > -1; iScope--)
	{
		if (iScope == ((CEngine *)this->pEngine)->State->MinimumScope)
		{
			iScope = 0; //Global
		}

		//Search for built-in items first, they have priority.
		for (int iItem = 0; iItem < this->_EndOfBuiltInIndex; iItem++)
		{
			if (this->Collection.Items[iItem].Name && this->Collection.Items[iItem].Scope == iScope)
			{
				if (_StrEqlI(this->Collection.Items[iItem].Name, sName))
				{
					return &this->Collection.Items[iItem];
				}
			}
		}

		//Then search for user items in reverse order, because the required object is
		//	almost always one of the most recent added.
		for (int iItem = this->Collection.Count - 1; iItem >= this->_EndOfBuiltInIndex; iItem--)
		{
			if (this->Collection.Items[iItem].Name && this->Collection.Items[iItem].Scope == iScope)
			{
				if (_StrEqlI(this->Collection.Items[iItem].Name, sName))
				{
					return &this->Collection.Items[iItem];
				}
			}
		}
	}

	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

VarIndex CVars::GetIndex(const char *sName)
{
	if (((CEngine*)this->pEngine)->State->Namespace())
	{
		char sNSVar[MAX_TOKEN_SIZE];
		strcpy_s(sNSVar, sizeof(sNSVar), ((CEngine*)this->pEngine)->State->Namespace());
		strcat_s(sNSVar, sizeof(sNSVar), ".");
		strcat_s(sNSVar, sizeof(sNSVar), sName);

		VarIndex index = this->GetIndexBase(sNSVar);
		if (index != INVALID_INDEX)
		{
			return index;
		}
	}

	return this->GetIndexBase(sName);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

VarIndex CVars::GetIndexBase(const char *sName)
{
	for (int iScope = ((CEngine *)this->pEngine)->State->CurrentScope(); iScope > -1; iScope--)
	{
		if (iScope == ((CEngine *)this->pEngine)->State->MinimumScope)
		{
			iScope = 0; //Global
		}

		for (int iItem = 0; iItem < this->Collection.Count; iItem++)
		{
			if (this->Collection.Items[iItem].Scope == iScope)
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
	}

	return INVALID_INDEX;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CVars::Clear(VARIABLEITEM *pItem)
{
	if (!pItem)
	{
		return this->Error->Hard("Undefined variable");
	}

	if (pItem->Value != NULL)
	{
		pMem->Free(pItem->Value);
	}

	pItem->Value = NULL;
	pItem->Length = 0;
	return this->Error->HardCount();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CVars::Clear(int iIndex)
{
	if (!this->IsDefined(iIndex))
	{
		return this->Error->Hard("Undefined variable");
	}
	return this->Clear(&this->Collection.Items[iIndex]);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CVars::Free(VARIABLEITEM *pItem)
{
	return this->Free(pItem, false);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CVars::Free(VARIABLEITEM *pItem, bool bValueOnly)
{
	if (!pItem)
	{
		return this->Error->Hard("Undefined variable");
	}

	if (!bValueOnly)
	{
		if (pItem->ChildVariableIndex != INVALID_INDEX)
		{
			/*
				This is a parent variable in a dependency chain, we need to cleanup all children.
			*/

			//Free the child variable.
			this->Free(pItem->ChildVariableIndex);
		}

		if (pItem->ParentVariableIndex != INVALID_INDEX)
		{
			/*
				This is a child variable in a dependency chain, there is nothing to free.
			*/
			pItem->Value = NULL;
			pItem->Length = 0;

			//Disassociate the parent variable.
			this->Collection.Items[pItem->ParentVariableIndex].ChildVariableIndex = INVALID_INDEX;
		}
	}

	if (pItem->Value)
	{
		TYPEITEM *pType = ((CEngine*)this->pEngine)->Types->GetPointer(pItem->Type);
		if (pType->IsBuiltInClass)
		{
			//Destroy built in classes.
			if (pItem->Type == VAR_TYPE_SQLCONNECTION
				|| pItem->Type == VAR_TYPE_SQLRECORDSET
				|| pItem->Type == VAR_TYPE_SOCKETSERVER
				|| pItem->Type == VAR_TYPE_XMLREADER
				|| pItem->Type == VAR_TYPE_XMLWRITER)
			{
				if (pItem->Type == VAR_TYPE_SOCKETSERVER)
				{
					if (((CSocketServer *)pItem->Value)->IsInitialized())
					{
						if (((CSocketServer *)pItem->Value)->IsActive())
						{
							((CSocketServer *)pItem->Value)->Stop(true);
						}
						((CSocketServer *)pItem->Value)->Destroy();
					}
				}

				delete pItem->Value;
				pItem->Value = NULL;
			}
			else if (pItem->Type == VAR_TYPE_EXCEPTION)
			{
				if (pItem->Value != NULL)
				{
					pMem->Free(((EXCEPTIONITEM *)pItem->Value)->Message);
				}
			}
			else if (pItem->Type == VAR_TYPE_LIST)
			{
				if (pItem->Value != NULL)
				{
					delete (VectorBinary *)pItem->Value;
				}

				pItem->Value = NULL;
				pItem->Length = 0;
			}
			else if (pItem->Type == VAR_TYPE_SOCKETCLIENT)
			{
				//The memory is handled by the owning socket server.

				if (pItem->Value)
				{
					//delete (CSocketClient *)pItem->Value;
				}

				pItem->Value = NULL;
				pItem->Length = 0;
			}
#ifdef COM_PROCEDURE_CALL_IMPLEMENTATION
			if (pItem->Type == VAR_TYPE_COM_OBJECT)
			{
				((IDispatch *)pItem->Value)->Release();
				pItem->Value = NULL;
			}
#endif //COM_PROCEDURE_CALL_IMPLEMENTATION
		}
		else if (pType->IsStrictType)
		{
			CEngine *pEngine = ((CEngine *)this->pEngine);

			//Free RAM for applicable members
			for (int iMemberIndex = 0; iMemberIndex < pType->Members.Count; iMemberIndex++)
			{
				if (pType->Members.Items[iMemberIndex].StrictType == STRICT_TYPE_STRING && pType->Members.Items[iMemberIndex].IsContiguous == false)
				{
					void *pValue = (*(char **)(pItem->Value + pType->Members.Items[iMemberIndex].ByteOffset));
					if (pValue)
					{
						pMem->Free(pValue);
					}
				}
			}
		}

		if (pItem->Value != NULL)
		{
			pMem->Free(pItem->Value);
		}
	}

	if (!bValueOnly)
	{
		if (pItem->Name != NULL)
		{
			pMem->Free(pItem->Name);
		}

		pItem->Name = NULL;
		pItem->Length = 0;
		pItem->Value = NULL;
		pItem->Type = 0;
		pItem->Scope = INVALID_INDEX;
		pItem->ChildVariableIndex = INVALID_INDEX;
		pItem->ParentVariableIndex = INVALID_INDEX;
	}
	else
	{
		pItem->Value = NULL;
		pItem->Length = 0;
	}

	return this->Error->HardCount();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CVars::Free(const char *sName)
{
	VARIABLEITEM *pVar = this->GetPointer(sName);
	if (!pVar)
	{
		return this->Error->Hard("Undefined variable");
	}
	else return this->Free(pVar);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CVars::Free(int iIndex)
{
	if (!this->IsDefined(iIndex))
	{
		return this->Error->Hard("Undefined variable");
	}
	return this->Free(&this->Collection.Items[iIndex]);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
We have to pass the variable by its index because the internal
calls to Evaluate() can reallocate the variable array.
*/

ErrorLevel CVars::SetByEval(int iIndex, const char *sText, int iLength)
{
	return this->SetByEval(iIndex, sText, iLength, -1);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	We have to pass the variable by its index because the internal
		calls to Evaluate() can reallocate the variable array.
*/

ErrorLevel CVars::SetByEval(int iIndex, const char *sText, int iLength, int iArrayIndex)
{
	return this->SetByEval(iIndex, NULL, iArrayIndex, sText, iLength);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
sMemberName is used to set the value of a property within a structure, class or strict type.

Note: We have to pass the variable by its index because the
internal calls to Evaluate() can reallocate the variable array.
*/
ErrorLevel CVars::SetByEval(int iIndex, const char *sMemberName, const char *sText, int iLength)
{
	return SetByEval(iIndex, sMemberName, -1, sText, iLength);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	sMemberName is used to set the value of a property within a structure, class or strict type.

	Note: We have to pass the variable by its index because the
		internal calls to Evaluate() can reallocate the variable array.
*/
ErrorLevel CVars::SetByEval(int iIndex, const char *sMemberName, int iArrayIndex, const char *sText, int iLength)
{
	ErrorLevel Error = ERROR_OK;

	/* Setting of complex variable values is now implemented.
	if(((CEngine*)this->pEngine)->Types->IsReadOnly(this->GetPointer(iIndex)->Type))
	{
		return this->Error->Hard("Variable is readonly");
	}
	*/

	CParams *pParams = NULL;
	Error = ((CEngine *)this->pEngine)->Evaluate(sText, iLength, &pParams);
	if (Error != ERROR_OK)
	{
		if (pParams)
		{
			delete pParams;
		}
		return Error;
	}
	else if (pParams->Collection.Count > 1)
	{
		this->Error->Hard("Too many parameters supplied");
		Error = INVALID_INDEX;
	}
	else if (pParams->Collection.Count < 1)
	{
		if(this->Error->SoftCount() > 0)
		{
			return this->Error->HardCount();
		}

		//this->Error->Hard("No parameters supplied or an error occured");
		Error = INVALID_INDEX;
	}
	else if (this->Set(iIndex, sMemberName, iArrayIndex, pParams) != ERROR_OK)
	{
		Error = INVALID_INDEX;
	}

	delete pParams;
	return Error;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CVars::Set(int iIndex, const char *sValue)
{
	if (((CEngine*)this->pEngine)->Types->IsReadOnly(this->GetPointer(iIndex)->Type))
	{
		return this->Error->Hard("Variable is readonly");
	}

	int iLength = 0;
	if (sValue)
	{
		iLength = (int)strlen(sValue);
	}
	return this->Set(iIndex, sValue, iLength);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CVars::Set(VARIABLEITEM *pItem, const char *sValue)
{
	if (((CEngine*)this->pEngine)->Types->IsReadOnly(pItem->Type))
	{
		return this->Error->Hard("Variable is readonly");
	}

	int iLength = 0;
	if (sValue)
	{
		iLength = (int)strlen(sValue);
	}
	return this->Set(pItem, sValue, iLength);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CVars::Set(int iIndex, const char *sValue, int iLength)
{
	if (((CEngine*)this->pEngine)->Types->IsReadOnly(this->GetPointer(iIndex)->Type))
	{
		return this->Error->Hard("Variable is readonly");
	}
	if (!this->IsDefined(iIndex))
	{
		return this->Error->Hard("Undefined variable");
	}
	return this->Set(&this->Collection.Items[iIndex], sValue, iLength);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CVars::Set(VARIABLEITEM *pItem, const char *sValue, int iLength)
{
	if (((CEngine*)this->pEngine)->Types->IsReadOnly(pItem->Type))
	{
		return this->Error->Hard("Variable is readonly");
	}
	if (!pItem)
	{
		return this->Error->Hard("Undefined variable");
	}

	if (pItem->Type == VAR_TYPE_NUMERIC)
	{
		if (!IsNumeric(sValue, iLength))
		{
			return this->Error->Hard("Failed to convert string to numeric");
		}
	}

	if (pItem->Alloc < iLength + 1)
	{
		pItem->Alloc = iLength + 1;

		if (!(pItem->Value = (char *) pMem->ReAllocate(pItem->Value, sizeof(char), pItem->Alloc)))
		{
			return this->Error->Hard("Memory allocation error");
		}
	}

	pItem->Length = iLength;
	memcpy_s(pItem->Value, pItem->Alloc, sValue, iLength);
	pItem->Value[iLength] = '\0';

	return this->Error->HardCount();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CVars::Append(VARIABLEITEM *pItem, const char *sValue, int iLength)
{
	if (((CEngine*)this->pEngine)->Types->IsReadOnly(pItem->Type))
	{
		return this->Error->Hard("Variable is readonly");
	}
	if (!pItem)
	{
		return this->Error->Hard("Undefined variable");
	}

	if (pItem->Type == VAR_TYPE_NUMERIC)
	{
		if (!IsNumeric(sValue, iLength))
		{
			return this->Error->Hard("Failed to convert string to numeric");
		}
	}

	if (pItem->Alloc < pItem->Length + iLength + 1)
	{
		pItem->Alloc = pItem->Length + iLength + 1;

		if (!(pItem->Value = (char *) pMem->ReAllocate(pItem->Value, sizeof(char), pItem->Alloc)))
		{
			return this->Error->Hard("Memory allocation error");
		}
	}

	memcpy_s(pItem->Value + pItem->Length, iLength + 1, sValue, iLength);
	pItem->Length += iLength;
	pItem->Value[pItem->Length] = '\0';

	return this->Error->HardCount();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CVars::Set(int iIndex, char cValue)
{
	if (((CEngine*)this->pEngine)->Types->IsReadOnly(this->GetPointer(iIndex)->Type))
	{
		return this->Error->Hard("Variable is readonly");
	}

	char sChar[2];
	sprintf_s(sChar, sizeof(sChar), "%c", cValue);
	return this->Set(&this->Collection.Items[iIndex], sChar);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CVars::Set(int iIndex, unsigned int iValue)
{
	if (((CEngine*)this->pEngine)->Types->IsReadOnly(this->GetPointer(iIndex)->Type))
	{
		return this->Error->Hard("Variable is readonly");
	}
	char sInt[64];
	if (_itoa_s(iValue, sInt, sizeof(sInt), 10) == 0)
	{
		return this->Set(&this->Collection.Items[iIndex], sInt);
	}
	else {
		return this->Error->Hard("Numeric conversion error");
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CVars::Set(VARIABLEITEM *pItem, int iValue)
{
	if (((CEngine*)this->pEngine)->Types->IsReadOnly(pItem->Type))
	{
		return this->Error->Hard("Variable is readonly");
	}
	char sInt[64];
	if (_itoa_s(iValue, sInt, sizeof(sInt), 10) == 0)
	{
		return this->Set(pItem, sInt);
	}
	else {
		return this->Error->Hard("Numeric conversion error");
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CVars::Set(int iIndex, int iValue)
{
	if (((CEngine*)this->pEngine)->Types->IsReadOnly(this->GetPointer(iIndex)->Type))
	{
		return this->Error->Hard("Variable is readonly");
	}
	char sInt[64];
	if (_itoa_s(iValue, sInt, sizeof(sInt), 10) == 0)
	{
		return this->Set(&this->Collection.Items[iIndex], sInt);
	}
	else {
		return this->Error->Hard("Numeric conversion error");
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CVars::Set(VARIABLEITEM *pItem, double dValue)
{
	if (((CEngine*)this->pEngine)->Types->IsReadOnly(pItem->Type))
	{
		return this->Error->Hard("Variable is readonly");
	}
	CMathParser MP;
	if (((CEngine*)pEngine)->MathDebugMode)
	{
		MP.DebugMode(((CEngine*)pEngine)->MathDebugMode);
	}
	char sVal[128];
	int iLength = MP.DoubleToChar(dValue, sVal, sizeof(sVal));
	if (iLength >= 0)
	{
		return this->Set(pItem, sVal, iLength);
	}
	else {
		return this->Error->Hard("Numeric conversion error");
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CVars::Set(int iIndex, double dValue)
{
	if (((CEngine*)this->pEngine)->Types->IsReadOnly(this->GetPointer(iIndex)->Type))
	{
		return this->Error->Hard("Variable is readonly");
	}
	CMathParser MP;
	if (((CEngine*)pEngine)->MathDebugMode)
	{
		MP.DebugMode(((CEngine*)pEngine)->MathDebugMode);
	}
	char sVal[128];
	int iLength = MP.DoubleToChar(dValue, sVal, sizeof(sVal));
	if (iLength >= 0)
	{
		return this->Set(&this->Collection.Items[iIndex], sVal, iLength);
	}
	else {
		return this->Error->Hard("Numeric conversion error");
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
sMemberName is used to set the value of a property within a structure, class or strict type.
*/
ErrorLevel CVars::Set(int iIndex, const char *sMemberName, CParams *pParams)
{
	return this->Set(iIndex, sMemberName, -1, pParams);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	sMemberName is used to set the value of a property within a structure, class or strict type.
*/
ErrorLevel CVars::Set(int iIndex, const char *sMemberName, int iArrayIndex, CParams *pParams)
{
	/* Setting of complex variable values is now implemented.
	if(((CEngine*)this->pEngine)->Types->IsReadOnly(this->GetPointer(iIndex)->Type))
	{
		return this->Error->Hard("Variable is readonly");
	}
	*/

	if (!this->IsDefined(iIndex))
	{
		return this->Error->Hard("Undefined variable");
	}
	return this->Set(&this->Collection.Items[iIndex], sMemberName, iArrayIndex, pParams);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CVars::Set(int iIndex, CParams *pParams)
{
	if (((CEngine*)this->pEngine)->Types->IsReadOnly(this->GetPointer(iIndex)->Type))
	{
		return this->Error->Hard("Variable is readonly");
	}
	if (!this->IsDefined(iIndex))
	{
		return this->Error->Hard("Undefined variable");
	}
	return this->Set(&this->Collection.Items[iIndex], NULL, pParams);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CVars::Set(VARIABLEITEM *pItem, CParams *pParams)
{
	return this->Set(pItem, NULL, pParams);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CVars::Set(VARIABLEITEM *pItem, const char *sMemberName, CParams *pParams)
{
	return this->Set(pItem, sMemberName, -1, pParams);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	sMemberName is used to set the value of a property within a structure, class or strict type.
*/
ErrorLevel CVars::Set(VARIABLEITEM *pItem, const char *sMemberName, int iArrayIndex, CParams *pParams)
{
	CEngine *pEngine = ((CEngine*)this->pEngine);

	TYPEITEM *pType = pEngine->Types->GetPointer(pItem->Type);

	/* Setting of complex variable values is now implemented.
	if(((CEngine*)this->pEngine)->Types->IsReadOnly(this->GetPointer(iIndex)->Type))
	{
		return this->Error->Hard("Variable is readonly");
	}
	*/

	if (!pItem)
	{
		return this->Error->Hard("Undefined variable");
	}

	if (pType->IsStrictType)
	{
		if (sMemberName)
		{
			bool bHandled = false;

			MEMBERENDPOINTINFO MemberEndpoint;
			memset(&MemberEndpoint, 0, sizeof(MemberEndpoint));
			int iNonMemberPosition = 0;

			if (pEngine->Types->TraverseMembers(pItem, sMemberName, &MemberEndpoint, &iNonMemberPosition) != ERROR_OK)
			{
				return this->Error->HardCount();
			}

			void *pValue = (pItem->Value + MemberEndpoint.ByteOffset);

			if (MemberEndpoint.Member->StrictType == STRICT_TYPE_UINTEGER16)
			{
				unsigned short Value = (unsigned short)pParams->ToUInt(0);
				memcpy_s(pValue, MemberEndpoint.Member->SizeInBytes, &Value, sizeof(Value));
				bHandled = true;
			}
			else if (MemberEndpoint.Member->StrictType == STRICT_TYPE_UINTEGER32)
			{
				unsigned int Value = (unsigned int)pParams->ToUInt(0);
				memcpy_s(pValue, MemberEndpoint.Member->SizeInBytes, &Value, sizeof(Value));
				bHandled = true;
			}
			else if (MemberEndpoint.Member->StrictType == STRICT_TYPE_UINTEGER64)
			{
				unsigned __int64 Value = (unsigned __int64)pParams->ToUInt(0);
				memcpy_s(pValue, MemberEndpoint.Member->SizeInBytes, &Value, sizeof(Value));
				bHandled = true;
			}
			else if (MemberEndpoint.Member->StrictType == STRICT_TYPE_INTEGER16)
			{
				short Value = (short)pParams->ToInt(0);
				memcpy_s(pValue, MemberEndpoint.Member->SizeInBytes, &Value, sizeof(Value));
				bHandled = true;
			}
			else if (MemberEndpoint.Member->StrictType == STRICT_TYPE_INTEGER32)
			{
				int Value = (int)pParams->ToInt(0);
				memcpy_s(pValue, MemberEndpoint.Member->SizeInBytes, &Value, sizeof(Value));
				bHandled = true;
			}
			else if (MemberEndpoint.Member->StrictType == STRICT_TYPE_INTEGER64)
			{
				__int64 Value = (__int64)pParams->ToInt(0);
				memcpy_s(pValue, MemberEndpoint.Member->SizeInBytes, &Value, sizeof(Value));
				bHandled = true;
			}
			else if (MemberEndpoint.Member->StrictType == STRICT_TYPE_FLOAT)
			{
				float Value = (float)pParams->ToDouble(0);
				memcpy_s(pValue, MemberEndpoint.Member->SizeInBytes, &Value, sizeof(Value));
				bHandled = true;
			}
			else if (MemberEndpoint.Member->StrictType == STRICT_TYPE_DOUBLE)
			{
				double Value = (double)pParams->ToDouble(0);
				memcpy_s(pValue, MemberEndpoint.Member->SizeInBytes, &Value, sizeof(Value));
				bHandled = true;
			}
			else if (MemberEndpoint.Member->StrictType == STRICT_TYPE_STRING)
			{
				PARAMITEM *pParam = pParams->GetPointer(0);

				if (MemberEndpoint.Member->IsContiguous)
				{
					int iLength = pParam->Length;
					if (iLength > MemberEndpoint.Member->SizeInBytes)
					{
						iLength = MemberEndpoint.Member->SizeInBytes;
					}
					if (iLength < MemberEndpoint.Member->SizeInBytes)
					{
						//Of we are not going to be overwriting all of the bytes, then zero it out. Null terminate.
						memset(pValue, 0, MemberEndpoint.Member->SizeInBytes);
					}

					memcpy_s(pValue, MemberEndpoint.Member->SizeInBytes, pParam->Value, iLength);
					((char *)pValue)[iLength] = '\0';
				}
				else {
					int iLength = pParam->Length * sizeof(char);

					if ((*(char **)pValue))
					{
						pMem->Free((*(char **)pValue));
					}

					(*(char **)pValue) = (char *)pMem->Allocate(iLength + 1, 1);
					memcpy_s((*(char **)pValue), iLength, pParam->Value, iLength);
					(*(char **)pValue)[iLength] = '\0';
				}
				bHandled = true;
			}
			else if (MemberEndpoint.Member->StrictType == STRICT_TYPE_BYTE)
			{
				//Byte sequences are NOT resizable and are not NULL terminated.

				PARAMITEM *pParam = pParams->GetPointer(0);

				int iLength = pParam->Length;
				if (iLength > MemberEndpoint.Member->SizeInBytes)
				{
					iLength = MemberEndpoint.Member->SizeInBytes;
				}
				if (iLength < MemberEndpoint.Member->SizeInBytes)
				{
					//If we are not going to be overwriting all of the bytes, then zero it out.
					memset(pValue, 0, MemberEndpoint.Member->SizeInBytes);
				}

				memcpy_s(pValue, MemberEndpoint.Member->SizeInBytes, pParam->Value, iLength);

				bHandled = true;
			}
			else {
				this->Error->Hard("Type of member variable [%s] cannot be accessed directly", sMemberName);
				return NULL;
			}
		}
		else
		{
			PARAMITEM *pParam = pParams->First();

			if (pItem->Alloc == 0)
			{
				pItem->Value = (char *)pMem->ReAllocate(pItem->Value, 1, pItem->Length);
				pItem->Alloc = pItem->Length;
				memset(pItem->Value, 0, pItem->Length);
			}

			int iCopyLength = pItem->Alloc;
			if (pParam->Length < iCopyLength)
			{
				iCopyLength = pParam->Length;
			}

			memcpy_s(pItem->Value, pItem->Alloc, pParam->Value, iCopyLength);

			//return this->Error->Hard("Member not specified for [%s]", pItem->Name);
		}
	}
	else
	{
		PARAMITEM *pParam = pParams->First();
		if (pItem->Type == VAR_TYPE_NUMERIC)
		{
			if (!IsNumeric(pParam->Value, pParam->Length))
			{
				return this->Error->Hard("Failed to convert string to numeric");
			}
		}

		TYPEITEM *pDestinationType = pEngine->Types->GetPointer(pItem->Type);
		if (pDestinationType->Index == VAR_TYPE_BYTES)
		{
			if (iArrayIndex = -1)
			{
				if (pParam->Length >= pItem->Length)
				{
					return this->Error->Hard("Insufficient space to store the requested value");
				}

				memcpy_s(pItem->Value, pItem->Alloc, pParam->Value, pParam->Length);
			}
			else if (iArrayIndex < 0 || iArrayIndex >= pItem->Alloc)
			{
				return this->Error->Hard("Array index is out of bounds");
			}
			else
			{
				((char *)pItem->Value)[iArrayIndex] = pParam->Value[0];
			}
		}
		else if (pDestinationType->Index == VAR_TYPE_ARRAY)
		{
			VARIABLEITEM *pVar = GetArraySubVariable(pItem, iArrayIndex);
			if (pVar != NULL)
			{
				pEngine->Vars->Set(pVar, pParams);
			}
		}
		else if (pDestinationType->IsComplex)
		{
			if (pParam->Value[0] == '@')
			{
				int iSourceVarIndex = pParams->ToVarIndex(0);

				VARIABLEITEM *pSourceVar = &this->Collection.Items[iSourceVarIndex];

				if (pSourceVar->Reserved)
				{
					pItem->Length = pSourceVar->Length;
					pItem->Value = pSourceVar->Value;

					pSourceVar->Value = NULL;
					pSourceVar->Length = 0;
					//Now that we have assumed the pointers of the variable, allow it to be freed.
					pSourceVar->Reserved = false;
					pEngine->Vars->Free(pSourceVar);
				}
				else {
					return this->Error->Hard("A complex return pointer was expected");
				}
			}
			else {
				return this->Error->Hard("A variable pointer was expected");
			}
		}
		else {
			if (pItem->Alloc < pParam->Length + 1)
			{
				pItem->Alloc = pParam->Length + 1;

				if (!(pItem->Value = (char *) pMem->ReAllocate(pItem->Value, sizeof(char), pItem->Alloc)))
				{
					return this->Error->Hard("Memory allocation error");
				}
			}

			pItem->Length = pParam->Length;
			memcpy_s(pItem->Value, pItem->Alloc, pParam->Value, pItem->Length);
		}
	}

	return this->Error->HardCount();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

VARIABLEITEM *CVars::GetArraySubVariable(VARIABLEITEM *pItem, int iArrayIndex)
{
	TYPEITEM *pSubType = ((CEngine*)pEngine)->Types->GetPointer(pItem->ArrayDefinition.Type);

	if (pSubType->IsBuiltIn)
	{
		int iSubVarIndex = (int)((void **)pItem->Value)[iArrayIndex];

		VARIABLEITEM *pVar = NULL;

		if (iSubVarIndex > 0)
		{
			pVar = this->GetPointer(iSubVarIndex);
		}

		if (pVar == NULL)
		{
			int iNewVarIndex = 0;
			this->AddUnique(pItem->ArrayDefinition.Type, &iNewVarIndex);
			pVar = this->GetPointer(iNewVarIndex);
			pVar->ParentVariableIndex = pItem->Index;
			((void **)pItem->Value)[iArrayIndex] = (void *)iNewVarIndex;
		}

		return pVar;
	}

	this->Error->Hard("Array type has not been implemented");

	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CVars::SetPointer(VARIABLEITEM *pVar, void *pObject, int iSize)
{
	VARIABLEITEM *pNode = &this->Collection.Items[pVar->Index];

	//Find the top node in a variable dependency chain.
	while (pNode->ParentVariableIndex != INVALID_INDEX)
	{
		pNode = &this->Collection.Items[pNode->ParentVariableIndex];
	}

	this->Free(pNode, true);
	pNode->Value = (char *)(int)pObject;
	pNode->Length = iSize;

	//Set the values of all children nodes in a variable dependency chain.
	while (pNode->ChildVariableIndex != INVALID_INDEX)
	{
		pNode = &this->Collection.Items[pNode->ChildVariableIndex];
		pNode->Value = (char *)(int)pObject;
		pNode->Length = iSize;
	}

	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CVars::Add(const char *sName, int iValue)
{
	int iIndex = INVALID_INDEX;
	if (this->Add(sName, VAR_TYPE_NUMERIC, &iIndex) != ERROR_OK)
	{
		return this->Error->HardCount();
	}
	return this->Set(iIndex, iValue);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Addes a reference to another variable.
*/
ErrorLevel CVars::AddNewStrongReference(const char *sNewName, int iReferencedIndex)
{
	VARIABLEITEM *pParent = this->GetPointer(iReferencedIndex);
	TYPEITEM *pType = ((CEngine*)this->pEngine)->Types->GetPointer(pParent->Type);

	if (pType->IsComplex)
	{
		int iNewIndex = 0;
		this->Add(sNewName, pParent->Type, &iNewIndex);

		pParent = this->GetPointer(iReferencedIndex); //The address could have changed.

		VARIABLEITEM *pChild = this->GetPointer(iNewIndex);
		pChild->Length = sizeof(void *); //Copying the pointer.
		pChild->Value = (char *)(int)pParent->Value;
		pChild->ArrayDefinition = pParent->ArrayDefinition;

		//Associate the two variables.
		pChild->ParentVariableIndex = pParent->Index;
		pParent->ChildVariableIndex = pChild->Index;
	}
	else {
		return this->Error->Hard("Strong reference to simple types is not implemented");
	}

	return this->Error->HardCount();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CVars::AddUnique(int iType, int *iOutVarIndex)
{
	char sNewVar[MAX_VAR_NAME_SIZE];
	sprintf_s(sNewVar, sizeof(sNewVar), "Tmp_%d_%d_%d", ((CEngine*)this->pEngine)->State->CurrentScope(), rand(), ++this->_UniqueCounter);
	return this->Add(sNewVar, iType, iOutVarIndex);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CVars::AddAndSet(const char *sName, int iType, const char *sValue, int iLength, int iScope)
{
	if (((CEngine*)this->pEngine)->Types->IsReadOnly(iType))
	{
		return this->Error->Hard("Variable is readonly");
	}
	int iIndex = INVALID_INDEX;
	if (this->Add(sName, iType, &iIndex) != ERROR_OK)
	{
		return this->Error->HardCount();
	}

	this->Collection.Items[iIndex].Scope = iScope;

	return this->Set(iIndex, sValue, iLength);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CVars::AddAndSet(const char *sName, int iType, const char *sValue, int iLength)
{
	if (((CEngine*)this->pEngine)->Types->IsReadOnly(iType))
	{
		return this->Error->Hard("Variable is readonly");
	}
	int iIndex = INVALID_INDEX;
	if (this->Add(sName, iType, &iIndex) != ERROR_OK)
	{
		return this->Error->HardCount();
	}
	return this->Set(iIndex, sValue, iLength);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CVars::AddAndSet(const char *sName, const char *sValue)
{
	int iIndex = INVALID_INDEX;
	if (this->Add(sName, VAR_TYPE_STRING, &iIndex) != ERROR_OK)
	{
		return this->Error->HardCount();
	}
	return this->Set(iIndex, sValue);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CVars::AddAndSet(const char *sName, double dValue)
{
	int iIndex = INVALID_INDEX;
	if (this->Add(sName, VAR_TYPE_NUMERIC, &iIndex) != ERROR_OK)
	{
		return this->Error->HardCount();
	}
	return this->Set(iIndex, dValue);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CVars::AddAndSet(const char *sName, int iValue)
{
	int iIndex = INVALID_INDEX;
	if (this->Add(sName, VAR_TYPE_NUMERIC, &iIndex) != ERROR_OK)
	{
		return this->Error->HardCount();
	}
	return this->Set(iIndex, iValue);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

VarIndex CVars::GetFreeIndex(void)
{
	for (int iItem = 0; iItem < this->Collection.Count; iItem++)
	{
		if (!this->Collection.Items[iItem].Name
			&& !this->Collection.Items[iItem].Reserved)
		{
			this->Collection.Items[iItem].Scope = INVALID_INDEX;
			return iItem;
		}
	}

	if (this->Collection.Count == this->Collection.Alloc)
	{
		if (this->Collection.Alloc == 0)
		{
			this->Collection.Alloc = 10;
		}
		else {
			this->Collection.Alloc += this->Collection.Alloc / 2;
		}

		this->Collection.Items = (VARIABLEITEM *)
			pMem->ReAllocate(this->Collection.Items, sizeof(VARIABLEITEM), this->Collection.Alloc);
		if (!this->Collection.Items)
		{
			this->Error->Hard("Memory allocation error");
			return INVALID_INDEX;
		}
	}

	this->Collection.Items[this->Collection.Count].Scope = INVALID_INDEX;

	return this->Collection.Count++;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

VARIABLEITEM *CVars::GetFreePointer(void)
{
	for (int iItem = 0; iItem < this->Collection.Count; iItem++)
	{
		if (!this->Collection.Items[iItem].Name
			&& !this->Collection.Items[iItem].Reserved)
		{
			this->Collection.Items[iItem].Scope = INVALID_INDEX;
			return &this->Collection.Items[iItem];
		}
	}

	if (this->Collection.Count == this->Collection.Alloc)
	{
		if (this->Collection.Alloc == 0)
		{
			this->Collection.Alloc = 10;
		}
		else {
			this->Collection.Alloc += this->Collection.Alloc / 2;
		}

		this->Collection.Items = (VARIABLEITEM *)
			pMem->ReAllocate(this->Collection.Items, sizeof(VARIABLEITEM), this->Collection.Alloc);
		if (!this->Collection.Items)
		{
			this->Error->Hard("Memory allocation error");
			return NULL;
		}
	}

	this->Collection.Items[this->Collection.Count].Scope = INVALID_INDEX;

	return &this->Collection.Items[this->Collection.Count++];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CVars::Add(const char *sName, int iType, int *iOutVarIndex)
{
	return Add(sName, iType, NULL, iOutVarIndex);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CVars::Add(const char *sName, int iType, ARRAYDEFINITION *arrayDefinition, int *iOutVarIndex)
{
	for (int iItem = 0; iItem < this->Collection.Count; iItem++)
	{
		if (this->Collection.Items[iItem].Scope == ((CEngine *)this->pEngine)->State->CurrentScope())
		{
			if (_StrEqlI(this->Collection.Items[iItem].Name, sName))
			{
				return this->Error->Hard("Variable redeclaration");
			}
		}
	}

	int iLength = (int)strlen(sName);

	if (this->CheckVariableName(sName, iLength))
	{
		return this->Error->HardCount();
	}

	int iIndex = this->GetFreeIndex();
	VARIABLEITEM *pItem = &this->Collection.Items[iIndex];
	memset(pItem, 0, sizeof(VARIABLEITEM));

	if (((CEngine*)this->pEngine)->State->Namespace())
	{
		char sNSName[MAX_VAR_NAME_SIZE];
		strcpy_s(sNSName, sizeof(sNSName), ((CEngine*)this->pEngine)->State->Namespace());
		strcat_s(sNSName, sizeof(sNSName), ".");
		strcat_s(sNSName, sizeof(sNSName), sName);
		pItem->Name = (char *)pMem->CloneString(sNSName);
	}
	else {
		pItem->Name = (char *)pMem->CloneString(sName);
	}

	if (!pItem->Name)
	{
		return this->Error->Hard("Memory allocation error");
	}

	pItem->Scope = ((CEngine *)this->pEngine)->State->CurrentScope();
	pItem->Type = iType;
	pItem->Index = iIndex;
	pItem->ChildVariableIndex = INVALID_INDEX;
	pItem->ParentVariableIndex = INVALID_INDEX;
	if (arrayDefinition != NULL)
	{
		memcpy_s(&pItem->ArrayDefinition, sizeof(ARRAYDEFINITION), arrayDefinition, sizeof(ARRAYDEFINITION));
	}
	else
	{
		memset(&pItem->ArrayDefinition, 0, sizeof(ARRAYDEFINITION));
	}

	TYPEITEM *pType = ((CEngine*)this->pEngine)->Types->GetPointer(iType);

	if (iType == VAR_TYPE_ARRAY)
	{
		pItem->Length = sizeof(void *) * pItem->ArrayDefinition.NumberOfElements;
		pItem->Alloc = pItem->Length;
		pItem->Value = (char *)pMem->Allocate(pItem->Alloc, 1);
		memset(pItem->Value, 0, pItem->Length);
	}
	else if (pType->IsStrictType)
	{
		//Allocate enough RAM for the entire collection of variables.
		pItem->Length = pType->Members.SizeInBytes;
		pItem->Value = (char *)pMem->Allocate(pItem->Length, 1);
		memset(pItem->Value, 0, pItem->Length);
	}
	else if (pType->IsBuiltInClass)
	{
		//Initialize built in classes here:
		if (iType == VAR_TYPE_SQLCONNECTION)
		{
			pItem->Length = sizeof(CSQL);
			pItem->Value = (char *) new CSQL;
			memset(pItem->Value, 0, pItem->Length);

			((CSQL *)pItem->Value)->pPublicData = this->pEngine;
		}
		else if (iType == VAR_TYPE_SQLRECORDSET)
		{
			pItem->Length = sizeof(CBoundRecordSet);
			pItem->Value = (char *) new CBoundRecordSet;
			memset(pItem->Value, 0, pItem->Length);

			((CBoundRecordSet *)pItem->Value)->pPublicData = this->pEngine;
		}
		else if (iType == VAR_TYPE_LIST)
		{
			pItem->Length = sizeof(VectorBinary);
			pItem->Value = (char *) new VectorBinary();
			memset(pItem->Value, 0, pItem->Length);
		}
		else if (iType == VAR_TYPE_SOCKETSERVER)
		{
			pItem->Length = sizeof(CSocketServer);
			pItem->Value = (char *) new CSocketServer;
			memset(pItem->Value, 0, pItem->Length);

			((CSocketServer *)pItem->Value)->pUserData = this->pEngine;
		}
		else if (iType == VAR_TYPE_SOCKETCLIENT)
		{
			pItem->Length = sizeof(CSocketClient);
			pItem->Value = (char *) new CSocketClient;
			memset(pItem->Value, 0, pItem->Length);
			((CSocketClient *)pItem->Value)->pUserData = this->pEngine;
		}
		else if (iType == VAR_TYPE_XMLREADER)
		{
			pItem->Length = sizeof(XMLReader);
			pItem->Value = (char *) new XMLReader;
			memset(pItem->Value, 0, pItem->Length);

			((XMLReader *)pItem->Value)->pUserData = this->pEngine;
		}
		else if (iType == VAR_TYPE_XMLWRITER)
		{
			pItem->Length = sizeof(XMLWriter);
			pItem->Value = (char *) new XMLWriter;
			memset(pItem->Value, 0, pItem->Length);

			((XMLWriter *)pItem->Value)->pUserData = this->pEngine;
			((XMLWriter *)pItem->Value)->Initialize();
		}
#ifdef COM_PROCEDURE_CALL_IMPLEMENTATION
		else if (iType == VAR_TYPE_COM_OBJECT)
		{
			return this->Error->Hard("Not implemented");
		}
#endif //COM_PROCEDURE_CALL_IMPLEMENTATION
	}

	if (iOutVarIndex)
	{
		*iOutVarIndex = iIndex;
	}

	return this->Error->HardCount();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CVars::StrictTypeToLoseType(int iStrictType)
{
	if (
		iStrictType == STRICT_TYPE_INTEGER16
		|| iStrictType == STRICT_TYPE_INTEGER32
		|| iStrictType == STRICT_TYPE_INTEGER64
		|| iStrictType == STRICT_TYPE_UINTEGER16
		|| iStrictType == STRICT_TYPE_UINTEGER32
		|| iStrictType == STRICT_TYPE_UINTEGER64
		|| iStrictType == STRICT_TYPE_FLOAT
		|| iStrictType == STRICT_TYPE_DOUBLE

/*
		#define STRICT_TYPE_SQLCONNECTION	12
		#define STRICT_TYPE_SQLRECORDSET	13
		#define STRICT_TYPE_FORM			14
		#define STRICT_TYPE_FILE			15
		#define STRICT_TYPE_GENERIC			16
		#define STRICT_TYPE_SOCKETSERVER	17
		#define STRICT_TYPE_SOCKETCLIENT	18
		#define STRICT_TYPE_EXCEPTION		19
*/
		
		)
	{
		return VAR_TYPE_NUMERIC;
	}
	else if (iStrictType == STRICT_TYPE_STRING
		|| iStrictType == STRICT_TYPE_BYTE)
	{
		return VAR_TYPE_STRING;
	}
	//else if(iStrictType != STRICT_TYPE_INVALID) {
	//	return STRICT_TYPE_GENERIC;
	//}

	return VAR_TYPE_INVALID;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CVars::StrictTypeSize(int iStrictType)
{
	if (iStrictType == STRICT_TYPE_INTEGER16 || iStrictType == STRICT_TYPE_UINTEGER16)
	{
		return sizeof(short);
	}
	else if (iStrictType == STRICT_TYPE_INTEGER32 || iStrictType == STRICT_TYPE_UINTEGER32)
	{
		return sizeof(int);
	}
	else if (iStrictType == STRICT_TYPE_INTEGER64 || iStrictType == STRICT_TYPE_UINTEGER64)
	{
		return sizeof(__int64);
	}
	else if (iStrictType == STRICT_TYPE_FLOAT)
	{
		return sizeof(float);
	}
	else if (iStrictType == STRICT_TYPE_DOUBLE)
	{
		return sizeof(double);
	}
	else if (iStrictType == STRICT_TYPE_STRING)
	{
		return sizeof(INT_PTR); //This is a pointer.
	}
	else if (iStrictType == STRICT_TYPE_BYTE)
	{
		return 1 /* ((n)*ByteCount) */;
	}
	else if (iStrictType == STRICT_TYPE_INVALID)
	{
		return sizeof(INT_PTR); //This is a pointer.
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CVars::TranslateStrictType(const char *sName)
{
	if (_StrEqlI(sName, "StrictType.uShort")
		|| _StrEqlI(sName, "StrictType.uInt16")
		|| _StrEqlI(sName, "StrictType.Boolean"))
	{
		return STRICT_TYPE_UINTEGER16;
	}
	else if (_StrEqlI(sName, "StrictType.uInteger")
		|| _StrEqlI(sName, "StrictType.uLong")
		|| _StrEqlI(sName, "StrictType.uInt")
		|| _StrEqlI(sName, "StrictType.uInt32"))
	{
		return STRICT_TYPE_UINTEGER32;
	}
	else if (_StrEqlI(sName, "StrictType.uInt64"))
	{
		return STRICT_TYPE_UINTEGER64;
	}
	else if (_StrEqlI(sName, "StrictType.Short")
		|| _StrEqlI(sName, "StrictType.Int16")
		|| _StrEqlI(sName, "StrictType.Boolean"))
	{
		return STRICT_TYPE_INTEGER16;
	}
	else if (_StrEqlI(sName, "StrictType.Handle"))
	{
		return STRICT_TYPE_UINTEGER32;
	}
	else if (_StrEqlI(sName, "StrictType.Integer")
		|| _StrEqlI(sName, "StrictType.Long")
		|| _StrEqlI(sName, "StrictType.Int")
		|| _StrEqlI(sName, "StrictType.Int32"))
	{
		return STRICT_TYPE_INTEGER32;
	}
	else if (_StrEqlI(sName, "StrictType.Int64"))
	{
		return STRICT_TYPE_INTEGER64;
	}
	else if (_StrEqlI(sName, "StrictType.Float")
		|| _StrEqlI(sName, "StrictType.Int"))
	{
		return STRICT_TYPE_INTEGER32;
	}
	else if (_StrEqlI(sName, "StrictType.Float"))
	{
		return STRICT_TYPE_FLOAT;
	}
	else if (_StrEqlI(sName, "StrictType.Double"))
	{
		return STRICT_TYPE_DOUBLE;
	}
	else if (_StrEqlI(sName, "StrictType.String"))
	{
		return STRICT_TYPE_STRING;
	}
	else if (_StrEqlI(sName, "StrictType.Byte")
		|| _StrEqlI(sName, "StrictType.Char"))
	{
		return STRICT_TYPE_BYTE;
	}

	TYPEITEM *pType = ((CEngine*)this->pEngine)->Types->GetPointer(sName);
	if (pType && pType->IsStrictType)
	{
		return STRICT_TYPE_GENERIC;
	}

	return STRICT_TYPE_INVALID;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CVars::DropScope(int iScope)
{
	for (int iItem = 0; iItem < this->Collection.Count; iItem++)
	{
		if (this->Collection.Items[iItem].Scope == iScope
			&& !this->Collection.Items[iItem].Reserved)
		{
			if (this->Free(iItem) != ERROR_OK)
			{
				return this->Error->HardCount();
			}
		}
	}
	return this->Error->HardCount();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CVars::AssessContentType(int iIndex)
{
	return this->AssessContentType(this->GetPointer(iIndex));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CVars::AssessContentType(VARIABLEITEM *pItem)
{
	if (!pItem)
	{
		return this->Error->Hard("Undefined variable");
	}

	if (pItem->Type == VAR_TYPE_SQLCONNECTION)
	{
		return STRICT_TYPE_SQLCONNECTION;
	}
	else if (pItem->Type == VAR_TYPE_SQLRECORDSET)
	{
		return STRICT_TYPE_SQLRECORDSET;
	}
	else if (pItem->Type == VAR_TYPE_XMLREADER)
	{
		return STRICT_TYPE_XMLREADER;
	}
	else if (pItem->Type == VAR_TYPE_XMLWRITER)
	{
		return STRICT_TYPE_XMLWRITER;
	}
	else if (pItem->Type == VAR_TYPE_SOCKETSERVER)
	{
		return STRICT_TYPE_SOCKETSERVER;
	}
	else if (pItem->Type == VAR_TYPE_SOCKETCLIENT)
	{
		return STRICT_TYPE_SOCKETCLIENT;
	}
#ifdef COM_PROCEDURE_CALL_IMPLEMENTATION
	else if (pItem->Type == VAR_TYPE_COM_OBJECT)
	{
		return STRICT_TYPE_COM_OBJECT;
	}
#endif //COM_PROCEDURE_CALL_IMPLEMENTATION
	else if (pItem->Type == VAR_TYPE_FILE)
	{
		return STRICT_TYPE_FILE;
	}
	else if (pItem->Type == VAR_TYPE_EXCEPTION)
	{
		return STRICT_TYPE_EXCEPTION;
	}
	else if (pItem->Type == VAR_TYPE_GENERIC)
	{
		return STRICT_TYPE_GENERIC;
	}
	else if (IsNumeric(pItem->Value, pItem->Length))
	{
		if (CharIndex(pItem->Value, pItem->Length, '.') >= 0)
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

CVars::CVars(void *lpEngine)
{
	this->pEngine = lpEngine;
	this->Error = ((CEngine *)lpEngine)->Error;
	this->_UniqueCounter = 0;
	this->_EndOfBuiltInIndex = 0;

	memset(&this->Collection, 0, sizeof(this->Collection));

	this->AddAndSet("NULL", (const char *)NULL);
	this->GetPointer("NULL")->Length = -1;

	this->AddAndSet("CR", "\r"); //Carrage-Return
	this->AddAndSet("LF", "\n"); //Line-Feed
	this->AddAndSet("Tab", "\t"); //Tab
	this->AddAndSet("CRLF", "\r\n"); //Carrage-Return Line-Feed
	this->AddAndSet("bAND", "&"); //Bitwise AND
	this->AddAndSet("AND", "&&"); //Logical AND
	this->AddAndSet("XOR", "^"); //Bitwise XOR
	this->AddAndSet("MOD", "%"); //Modulo
	this->AddAndSet("OR", "||"); //Logical OR
	this->AddAndSet("bOR", "|"); //bitwise OR
	this->AddAndSet("NOT", "!"); //Logical NOT
	this->AddAndSet("bNOT", "~"); //Bitwise NOT
	this->AddAndSet("True", 1); //Logical TRUE
	this->AddAndSet("False", 0); //Logical FALSE

	this->AddAndSet("Engine", VAR_TYPE_GENERIC, (const char *)NULL, 0);
	this->AddAndSet("Date", VAR_TYPE_GENERIC, (const char *)NULL, 0);
	this->AddAndSet("Time", VAR_TYPE_GENERIC, (const char *)NULL, 0);
	this->AddAndSet("Code", VAR_TYPE_GENERIC, (const char *)NULL, 0);
	this->AddAndSet("Env", VAR_TYPE_GENERIC, (const char *)NULL, 0);
	this->AddAndSet("Env.CommandLine", VAR_TYPE_GENERIC, (const char *)NULL, 0);
	this->AddAndSet("Convert", VAR_TYPE_GENERIC, (const char *)NULL, 0);
	this->AddAndSet("Error", VAR_TYPE_GENERIC, (const char *)NULL, 0);
	this->AddAndSet("OS", VAR_TYPE_GENERIC, (const char *)NULL, 0);
	this->AddAndSet("Debug", VAR_TYPE_GENERIC, (const char *)NULL, 0);
	this->AddAndSet("File", VAR_TYPE_GENERIC, (const char *)NULL, 0);
	this->AddAndSet("Web", VAR_TYPE_GENERIC, (const char *)NULL, 0);
	this->AddAndSet("Settings", VAR_TYPE_GENERIC, (const char *)NULL, 0);
	this->AddAndSet("Math", VAR_TYPE_GENERIC, (const char *)NULL, 0);
	this->AddAndSet("Sys", VAR_TYPE_GENERIC, (const char *)NULL, 0);
	this->AddAndSet("Process", VAR_TYPE_GENERIC, (const char *)NULL, 0);
	this->AddAndSet("Query", VAR_TYPE_GENERIC, (const char *)NULL, 0);
	this->AddAndSet("Form", VAR_TYPE_GENERIC, (const char *)NULL, 0);

	this->AddAndSet("Limits", VAR_TYPE_GENERIC, (const char *)NULL, 0);
	this->AddAndSet("Limits.Int32.Min", (double)LONG_MIN);
	this->AddAndSet("Limits.Int32.Max", (double)LONG_MAX);
	this->AddAndSet("Limits.uInt32.Min", (double)0);
	this->AddAndSet("Limits.uInt32.Max", (double)UINT_MAX);
	this->AddAndSet("Limits.Int16.Min", (double)SHRT_MIN);
	this->AddAndSet("Limits.Int16.Max", (double)SHRT_MAX);
	this->AddAndSet("Limits.uInt16.Min", (double)0);
	this->AddAndSet("Limits.uInt16.Max", (double)USHRT_MAX);
	this->AddAndSet("Limits.File.PathLength", (double)MAX_PATH);

	this->AddAndSet("FileAccess", (int)VAR_TYPE_GENERIC, (const char *)NULL, 0);
	this->AddAndSet("FileAccess.Read", (int)GENERIC_READ);
	this->AddAndSet("FileAccess.Write", (int)GENERIC_WRITE);
	this->AddAndSet("FileAccess.ReadWrite", (int)(GENERIC_READ | GENERIC_WRITE));

	this->AddAndSet("CreationDisposition", VAR_TYPE_GENERIC, (const char *)NULL, 0);
	this->AddAndSet("CreationDisposition.CreateNew", (int)CREATE_NEW);
	this->AddAndSet("CreationDisposition.AlwaysCreate", (int)CREATE_ALWAYS);
	this->AddAndSet("CreationDisposition.OpenExisting", (int)OPEN_EXISTING);
	this->AddAndSet("CreationDisposition.AlwaysOpen", (int)OPEN_ALWAYS);
	this->AddAndSet("CreationDisposition.Truncate", (int)TRUNCATE_EXISTING);

	this->AddAndSet("Engine.Version", gsFileVersion);
	this->AddAndSet("Engine.Name", gsTitleCaption);
	this->AddAndSet("Engine.Build", __DATE__);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CVars::~CVars()
{
	if (this->Collection.Count)
	{
		for (int iItem = 0; iItem < this->Collection.Count; iItem++)
		{
			if (this->Collection.Items[iItem].Name)
			{
				this->Free(iItem);
			}
		}
		pMem->Free(this->Collection.Items);
	}
	memset(&this->Collection, 0, sizeof(this->Collection));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
