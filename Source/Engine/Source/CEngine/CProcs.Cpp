#ifndef _CProcs_CPP_
#define _CProcs_CPP_
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define _CRT_SECURE_NO_WARNINGS

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>
#include <Math.H>
#include <Process.h>
#include <Psapi.H>

#include "../../../../NSWFL/NSWFL.h"
#include "../../../../CMathParser/CMathParser.H"

#include "CEngine.H"
#include "CProcs.H"
#include "CScripts.H"
#include "Helpers.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::String;
using namespace NSWFL::Conversion;
using namespace NSWFL::System;
using namespace NSWFL::File;
using namespace NSWFL::Math;
using namespace NSWFL::Hashing;
using namespace NSWFL::Collections;
using namespace NSWFL::XML;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CProcs::Add(const char *sName, int iType, int iReturnType, CReqParams *pReqParams)
{
	if (this->GetPointer(sName))
	{
		return this->Error->Hard("Multiple procedure definitions");
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

		this->Collection.Items = (PROCITEM *)
			pMem->ReAllocate(this->Collection.Items, sizeof(PROCITEM), this->Collection.Alloc);
		if (!this->Collection.Items)
		{
			return this->Error->Hard("Memory allocation error");
		}
	}

	PROCITEM *pItem = &this->Collection.Items[this->Collection.Count++];
	memset(pItem, 0, sizeof(PROCITEM));
	pItem->Type = iType;
	pItem->ReturnType = iReturnType;
	pItem->Name = (char *) pMem->CloneString(sName);

#ifdef EXTERNAL_PROCEDURE_CALL_IMPLEMENTATION
	pItem->Library = NULL;
	pItem->Alias = NULL;
#endif //EXTERNAL_PROCEDURE_CALL_IMPLEMENTATION

	if (pReqParams)
	{
		pItem->ReqParams = new CReqParams(this->pEngine, pReqParams);
	}
	else {
		pItem->ReqParams = NULL;
	}

	return this->Error->HardCount();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef EXTERNAL_PROCEDURE_CALL_IMPLEMENTATION
ErrorLevel CProcs::AddExternal(LINEITEM *pLine)
{
	CEngine *pEngine = (CEngine *)this->pEngine;

	char sProcName[MAX_NAME_SIZE];
	char sVarName[MAX_NAME_SIZE];
	char sTypeName[MAX_NAME_SIZE];
	char sTok[MAX_TOKEN_SIZE];
	char sLibrary[MAX_TOKEN_SIZE];
	char sAlias[MAX_TOKEN_SIZE];
	char sCall[MAX_TOKEN_SIZE];

	int iTokSz = 0;
	int iWPos = 0;
	int iRPos = 0;
	int iStrictVarType = STRICT_TYPE_INVALID;
	int iStrictReturnType = STRICT_TYPE_INVALID;

	memset(sAlias, 0, sizeof(sAlias));
	memset(sLibrary, 0, sizeof(sLibrary));
	memset(sTok, 0, sizeof(sTok));
	memset(sVarName, 0, sizeof(sVarName));
	memset(sProcName, 0, sizeof(sProcName));
	memset(sCall, 0, sizeof(sCall));

	//Parse "Declare"
	if (Token(pLine->Text, pLine->Length, sTok, sizeof(sTok), &iRPos) <= 0)
	{
		return this->Error->Hard("Expected: [Declare]");
	}
	else if (!_StrEqlI(sTok, "Declare"))
	{
		return this->Error->Hard("Expected: [Declare]");
	}
	SkipWhiteSpaces(pLine->Text, pLine->Length, &iRPos);

	//Parse "Function Name"
	if (Token(pLine->Text, pLine->Length, sProcName, sizeof(sProcName), &iRPos) <= 0)
	{
		return this->Error->Hard("Expected: [Function Name]");
	}
	SkipWhiteSpaces(pLine->Text, pLine->Length, &iRPos);

	while (true)
	{
		//Parse "Flag"
		if (Token(pLine->Text, pLine->Length, sTok, sizeof(sTok), &iRPos) <= 0)
		{
			if (strlen(sLibrary) == 0)
			{
				return this->Error->Hard("[Lib] attribute is required");
			}
			else {
				break;
			}
		}
		SkipWhiteSpaces(pLine->Text, pLine->Length, &iRPos);

		if (_StrEqlI(sTok, "Lib"))
		{
			//Parse "Library Name"
			if (pLine->Text[iRPos] == '\"' && iRPos < pLine->Length)
			{
				iWPos = 0;
				iRPos++; //Skip the '"'.
				while (pLine->Text[iRPos] != '\"')
				{
					sLibrary[iWPos++] = pLine->Text[iRPos++];
				}
				sLibrary[iWPos] = '\0';
				iRPos++; //Skip the '"'.
			}
			else {
				return this->Error->Hard("Expected: [\"]");

			}
		}
		else if (_StrEqlI(sTok, "Alias"))
		{
			//Parse "Alias Name"
			if (pLine->Text[iRPos] == '\"')
			{
				iWPos = 0;
				iRPos++; //Skip the '"'.
				while (pLine->Text[iRPos] != '\"' && iRPos < pLine->Length)
				{
					sAlias[iWPos++] = pLine->Text[iRPos++];
				}
				sAlias[iWPos] = '\0';
				iRPos++; //Skip the '"'.
			}
			else {
				return this->Error->Hard("Expected: [\"]");
			}
		}
		else if (_StrEqlI(sTok, "Type"))
		{
			//Parse "Calling Convention"
			if (pLine->Text[iRPos] == '\"')
			{
				iWPos = 0;
				iRPos++; //Skip the '"'.
				while (pLine->Text[iRPos] != '\"' && iRPos < pLine->Length)
				{
					sCall[iWPos++] = pLine->Text[iRPos++];
				}
				sCall[iWPos] = '\0';
				iRPos++; //Skip the '"'.
			}
			else {
				return this->Error->Hard("Expected: [\"]");
			}
		}
		else {
			return this->Error->Hard("Expected: [Lib], [Alias] or [Type], found [%s]", sTok);
		}

		SkipWhiteSpaces(pLine->Text, pLine->Length, &iRPos);
	}

	if (strlen(sCall) == 0)
	{
		strcpy_s(sCall, sizeof(sCall), "StdCall");
	}
	if (strlen(sAlias) == 0)
	{
		strcpy_s(sAlias, sizeof(sAlias), sProcName);
	}

	//Start parsing the parameters:
	CReqParams ReqParam(pEngine);

	int iBegBrace = 0;
	int iEndBrace = 0;

	if (pLine->Text[iRPos] != '(')
	{
		return this->Error->Hard("Expected: [(]");
	}

	if (pEngine->GetParenIndexPositions(pLine->Text, pLine->Length, &iRPos, &iBegBrace, &iEndBrace) != ERROR_OK)
	{
		return this->Error->HardCount();
	}

	if ((iEndBrace - iBegBrace) < 0)
	{
		return this->Error->Hard("Parenthesis required for procedure declaration");
	}

	for (iRPos = iBegBrace; iRPos < iEndBrace; iRPos++)
	{
		bool bByReference = false;
		bool bPassTypeSpecified = false;

		if (Token(pLine->Text, pLine->Length, sVarName, sizeof(sVarName), &iRPos) <= 0)
		{
			return this->Error->Hard("Expected: [Variable Name]");
		}

		if (_StrEqlI(sVarName, "ByVal") || _StrEqlI(sVarName, "ByRef"))
		{
			bByReference = _StrEqlI(sVarName, "ByRef");
			bPassTypeSpecified = true;

			if (Token(pLine->Text, pLine->Length, sVarName, sizeof(sVarName), &iRPos) <= 0)
			{
				return this->Error->Hard("Expected: [Variable Name]");
			}
		}

		if (Token(pLine->Text, pLine->Length, sTok, sizeof(sTok), &iRPos) <= 0)
		{
			return this->Error->Hard("Expected: [as]");
		}

		if (!_StrEqlI(sTok, "as"))
		{
			return this->Error->Hard("Expected: [as]");
		}

		if (TokenWithDot(pLine->Text, pLine->Length, sTypeName, sizeof(sTypeName), &iRPos) <= 0)
		{
			return this->Error->Hard("Expected: [Type Name]");
		}

		int iVarType = pEngine->Types->GetIndex(sTypeName);
		if (iVarType == VAR_TYPE_INVALID)
		{
			if ((iStrictVarType = pEngine->Vars->TranslateStrictType(sTypeName)) == STRICT_TYPE_INVALID)
			{
				return this->Error->Hard("Expected: [Valid Type Name]");
			}

			iVarType = pEngine->Vars->StrictTypeToLoseType(iStrictVarType);
		}
		else {
			TYPEITEM *pType = pEngine->Types->GetPointer(iVarType);
			if (!pType->IsStrictType)
			{
				return this->Error->Hard("Expected: [Strict Type] or [Custom Strict Type]");
			}

			iStrictVarType = STRICT_TYPE_GENERIC;
		}

		SkipWhiteSpaces(pLine->Text, pLine->Length, &iRPos);

		if (ReqParam.Add(sVarName, iVarType) != ERROR_OK)
		{
			return this->Error->HardCount();
		}
		REQPARAMITEM *pReqParam = ReqParam.Last();

		pReqParam->ByReference = bByReference;
		pReqParam->StrictType = iStrictVarType;
	}

	SkipWhiteSpaces(pLine->Text, pLine->Length, &iRPos);

	if (ReqParam.Collection.Count == 0)
	{
		if (pLine->Text[iRPos++] != ')')
		{
			return this->Error->Hard("Expected: [)]");
		}

		SkipWhiteSpaces(pLine->Text, pLine->Length, &iRPos);
	}

	if (Token(pLine->Text, pLine->Length, sTok, sizeof(sTok), &iRPos) > 0)
	{
		if (!_StrEqlI(sTok, "as"))
		{
			return this->Error->Hard("Expected: [as]");
		}

		if (TokenWithDot(pLine->Text, pLine->Length, sTypeName, sizeof(sTypeName), &iRPos) <= 0)
		{
			return this->Error->Hard("Expected: [Return type name]");
		}

		if ((iStrictReturnType = pEngine->Vars->TranslateStrictType(sTypeName)) == STRICT_TYPE_INVALID)
		{
			return this->Error->Hard("Expected: [Valid return type name]");
		}
	}

	int iReturnType = pEngine->Vars->StrictTypeToLoseType(iStrictReturnType);

	if(pEngine->Scripts->Collection.Items[pLine->ScriptIndex].Namespace != NULL)
	{
		strcpy_s(sTok, sizeof(sTok), sProcName);

		strcpy_s(sProcName, sizeof(sProcName), pEngine->Scripts->Collection.Items[pLine->ScriptIndex].Namespace);
		strcat_s(sProcName, sizeof(sProcName), ".");
		strcat_s(sProcName, sizeof(sProcName), sTok);
	}

	if (this->Add(sProcName, PROC_TYPE_EXTERNAL, iReturnType, &ReqParam) != ERROR_OK)
	{
		return this->Error->HardCount();
	}

	PROCITEM *pProc = this->GetPointer(sProcName);
	pProc->Library = (char *) pMem->CloneString(sLibrary);
	pProc->Alias = (char *) pMem->CloneString(sAlias);
	pProc->LineBegin = pLine->LineIndex;
	pProc->CallingConvention = TranslateCallingConvention(sCall);
	pProc->AssessedReturnType = iStrictReturnType;

	if (pProc->CallingConvention == EXTERNAL_TYPE_INVALID)
	{
		return this->Error->Hard("Expected: A valid calling convention name [StdCall], [CDecl], [FastCall]");
	}

	return this->Error->HardCount();
}
#endif //EXTERNAL_PROCEDURE_CALL_IMPLEMENTATION

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef EXTERNAL_PROCEDURE_CALL_IMPLEMENTATION
int CProcs::TranslateCallingConvention(const char *sName)
{
	if (_StrEqlI(sName, "StdCall"))
	{
		return EXTERNAL_TYPE_STDCALL;
	}
	else if (_StrEqlI(sName, "CDECL"))
	{
		return EXTERNAL_TYPE_CDECL;
	}
	else if (_StrEqlI(sName, "FastCall"))
	{
		return EXTERNAL_TYPE_FASTCALL;
	}
	return EXTERNAL_TYPE_INVALID;
}
#endif //EXTERNAL_PROCEDURE_CALL_IMPLEMENTATION

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CProcs::AddScript(LINEITEM *pLine)
{
	CEngine *pEngine = (CEngine *)this->pEngine;

	char sProcName[MAX_NAME_SIZE];
	char sVarType[MAX_TOKEN_SIZE];
	char sVarName[MAX_NAME_SIZE];
	char sTok[MAX_TOKEN_SIZE];

	int iTokSz = 0;
	int iRPos = 0;
	int iVarType = VAR_TYPE_INVALID;
	int iReturnType = VAR_TYPE_INVALID;

	if (Token(pLine->Text, pLine->Length, sTok, sizeof(sTok), &iRPos) <= 0)
	{
		return this->Error->Hard("Expected: [Function]");
	}
	else if (!_StrEqlI(sTok, "Function"))
	{
		return this->Error->Hard("Expected: [Function]");
	}
	SkipWhiteSpaces(pLine->Text, pLine->Length, &iRPos);

	//Get function name.
	if (TokenWithDot(pLine->Text, pLine->Length, sTok, sizeof(sTok), &iRPos) <= 0)
	{
		return this->Error->Hard("Expected: [Function Name]");
	}
	SkipWhiteSpaces(pLine->Text, pLine->Length, &iRPos);

	if (((CEngine*)this->pEngine)->State->Namespace())
	{
		strcpy_s(sProcName, sizeof(sProcName), ((CEngine*)this->pEngine)->State->Namespace());
		strcat_s(sProcName, sizeof(sProcName), ".");
		strcat_s(sProcName, sizeof(sProcName), sTok);
	}
	else {
		strcpy_s(sProcName, sizeof(sProcName), sTok);
	}

	//Start parsing the parameters:
	CReqParams ReqParam(pEngine);

	int iBegBrace = 0;
	int iEndBrace = 0;

	if (pLine->Text[iRPos] != '(')
	{
		return this->Error->Hard("Expected: [(]");
	}

	if (pEngine->GetParenIndexPositions(pLine->Text, pLine->Length, &iRPos, &iBegBrace, &iEndBrace) != ERROR_OK)
	{
		return this->Error->HardCount();
	}

	if ((iEndBrace - iBegBrace) < 0)
	{
		return this->Error->Hard("Parenthesis required for procedure declaration");
	}

	for (iRPos = iBegBrace; iRPos < iEndBrace; iRPos++)
	{
		bool isArray = false;

		if (Token(pLine->Text, pLine->Length, sVarName, sizeof(sVarName), &iRPos) <= 0)
		{
			return this->Error->Hard("Expected: [Variable Name]");
		}

		if (Token(pLine->Text, pLine->Length, sTok, sizeof(sTok), &iRPos) <= 0)
		{
			return this->Error->Hard("Expected: [as]");
		}

		if (TokenWithDot(pLine->Text, pLine->Length, sVarType, sizeof(sVarType), &iRPos) <= 0)
		{
			return this->Error->Hard("Expected: [Type Name]");
		}

		if ((iVarType = pEngine->Types->GetIndex(sVarType)) == VAR_TYPE_INVALID)
		{
			return this->Error->Hard("Expected: [Valid Type Name]");
		}

		SkipWhiteSpaces(pLine->Text, pLine->Length, &iRPos);

		if (pLine->Text[iRPos] == '[')
		{
			iRPos++; //Skip open bracket;
			SkipWhiteSpaces(pLine->Text, pLine->Length, &iRPos);
			if (pLine->Text[iRPos] == ']')
			{
				iRPos++; //Skip open bracket;

				isArray = true;
			}
			else
			{
				return this->Error->Hard("Bracket micmatch when parsing array parameter");
			}
		}

		SkipWhiteSpaces(pLine->Text, pLine->Length, &iRPos);

		if (pLine->Text[iRPos] == '=')
		{
			return this->Error->Hard("Optional parameters are not implemented");
		}

		if (pLine->Text[iRPos] != ',' && pLine->Text[iRPos] != ')')
		{
			return this->Error->Hard("Unexpected character when parsing parameter \"%c\"", pLine->Text[iRPos]);
		}

		if (ReqParam.Add(sVarName, iVarType, isArray) != ERROR_OK)
		{
			return this->Error->HardCount();
		}
	}

	SkipWhiteSpaces(pLine->Text, pLine->Length, &iRPos);

	if (ReqParam.Collection.Count == 0)
	{
		if (pLine->Text[iRPos++] != ')')
		{
			return this->Error->Hard("Expected: [)]");
		}

		SkipWhiteSpaces(pLine->Text, pLine->Length, &iRPos);
	}

	if (Token(pLine->Text, pLine->Length, sTok, sizeof(sTok), &iRPos) > 0)
	{
		if (!_StrEqlI(sTok, "as"))
		{
			return this->Error->Hard("Expected: [as]");
		}

		char sTypeName[MAX_NAME_SIZE];
		if (TokenWithDot(pLine->Text, pLine->Length, sTypeName, sizeof(sTypeName), &iRPos) <= 0)
		{
			return this->Error->Hard("Expected: [Return type name]");
		}

		if ((iReturnType = pEngine->Types->GetIndex(sTypeName)) == VAR_TYPE_INVALID)
		{
			return this->Error->Hard("Expected: [Valid return type name]");
		}
	}

	if (this->Add(sProcName, PROC_TYPE_SCRIPT, iReturnType, &ReqParam) != ERROR_OK)
	{
		return this->Error->HardCount();
	}

	this->GetPointer(sProcName)->LineBegin = pLine->LineIndex;

	return this->Error->HardCount();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CProcs::AddSystem(const char *sName, int iReturnType, CReqParams *pReqParams)
{
	return this->Add(sName, PROC_TYPE_SYSTEM, iReturnType, pReqParams);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char *CProcs::SQLBuffer(int *iOutAllocatedSize)
{
	if (!this->sSQLBuffer)
	{
		this->iAllocatedSize = 8000 + 1;
		if (!(this->sSQLBuffer = (char *) pMem->Allocate(sizeof(char), this->iAllocatedSize)))
		{
			this->Error->Soft("Failed to allocate memory for SQL return values");
			return NULL;
		}
	}

	*iOutAllocatedSize = this->iAllocatedSize;

	return this->sSQLBuffer;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CProcs::CProcs(void *lpEngine)
{
	this->pEngine = lpEngine;
	this->Error = ((CEngine *)lpEngine)->Error;
	this->sSQLBuffer = NULL;
	this->iAllocatedSize = 0;
	this->_EndOfBuiltInIndex = 0;

	memset(&this->Collection, 0, sizeof(this->Collection));

	CReqParams *pReqParams;
	CReqParams *pNoParams = new CReqParams(this->pEngine);

	CReqParams *pIndexParamOnly = new CReqParams(this->pEngine);
	pIndexParamOnly->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.

	//-----( Console Output )--------------------------------------------------
	this->AddSystem("Print", VAR_TYPE_NUMERIC, NULL);
	this->AddSystem("Prints", VAR_TYPE_NUMERIC, NULL);
	this->AddSystem("BinPrint", VAR_TYPE_INVALID, NULL);
	this->AddSystem("Cls", VAR_TYPE_INVALID, pNoParams);

	//-----( Core )------------------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add("ReturnValue", VAR_TYPE_NUMERIC);
	this->AddSystem("Return", VAR_TYPE_STRING, NULL);
	delete pReqParams;

	//-----( Convert )------------------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC);
	pReqParams->Add("Pointer", VAR_TYPE_NUMERIC);
	this->AddSystem("Convert.LengthFromPointer", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC);
	pReqParams->Add("Pointer", VAR_TYPE_NUMERIC);
	this->AddSystem("Convert.StringFromPointer", VAR_TYPE_STRING, pReqParams);
	delete pReqParams;

	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add("Value", VAR_TYPE_GENERIC);
	pReqParams->Add("Type", VAR_TYPE_NUMERIC);
	this->AddSystem("Cast", VAR_TYPE_GENERIC, pReqParams);
	delete pReqParams;

	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->AddOptional("ExitCode", VAR_TYPE_NUMERIC, 0);
	this->AddSystem("Exit", VAR_TYPE_INVALID, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add("Text", VAR_TYPE_STRING);
	this->AddSystem("DataLen", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add("Length", VAR_TYPE_NUMERIC);
	this->AddSystem("Space", VAR_TYPE_STRING, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add("Logic", VAR_TYPE_NUMERIC);
	pReqParams->Add("ValueIfTrue", VAR_TYPE_STRING);
	pReqParams->Add("ValueIfFalse", VAR_TYPE_STRING);
	this->AddSystem("IIF", VAR_TYPE_STRING, pReqParams);
	delete pReqParams;
	//------------------------------------------------

	//-----( Env )------------------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC);
	pReqParams->Add("Index", VAR_TYPE_NUMERIC);
	this->AddSystem("Env.CommandLine.Get", VAR_TYPE_INVALID, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC);
	this->AddSystem("Env.CommandLine.Count", VAR_TYPE_INVALID, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC);
	pReqParams->Add("Name", VAR_TYPE_STRING);
	this->AddSystem("Env.Get", VAR_TYPE_STRING, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC);
	pReqParams->Add("Name", VAR_TYPE_STRING);
	pReqParams->AddOptional("Value", VAR_TYPE_STRING, "");
	this->AddSystem("Env.Set", VAR_TYPE_INVALID, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC);
	pReqParams->Add("Name", VAR_TYPE_STRING);
	this->AddSystem("Env.Index", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC);
	pReqParams->Add("Index", VAR_TYPE_NUMERIC);
	this->AddSystem("Env.Name", VAR_TYPE_STRING, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC);
	pReqParams->Add("Index", VAR_TYPE_NUMERIC);
	this->AddSystem("Env.Value", VAR_TYPE_STRING, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	this->AddSystem("Env.Count", VAR_TYPE_NUMERIC, pIndexParamOnly);
	//------------------------------------------------

	//-----( Process )------------------------------------------------------------
	this->AddSystem("Process.ModuleName", VAR_TYPE_STRING, pIndexParamOnly);
	this->AddSystem("Process.PageFaultCount", VAR_TYPE_NUMERIC, pIndexParamOnly);
	this->AddSystem("Process.PagefileUsage", VAR_TYPE_NUMERIC, pIndexParamOnly);
	this->AddSystem("Process.PeakPagefileUsage", VAR_TYPE_NUMERIC, pIndexParamOnly);
	this->AddSystem("Process.PeakWorkingSetSize", VAR_TYPE_NUMERIC, pIndexParamOnly);
	this->AddSystem("Process.WorkingSetSize", VAR_TYPE_NUMERIC, pIndexParamOnly);
	this->AddSystem("Process.Id", VAR_TYPE_NUMERIC, pIndexParamOnly);
	this->AddSystem("Process.Username", VAR_TYPE_STRING, pIndexParamOnly);
	//------------------------------------------------

	//-----( Sys )------------------------------------------------------------
	this->AddSystem("Sys.TickCount", VAR_TYPE_NUMERIC, pIndexParamOnly);
	this->AddSystem("Sys.MachineName", VAR_TYPE_STRING, pIndexParamOnly);
	this->AddSystem("Sys.OSVersion", VAR_TYPE_STRING, pIndexParamOnly);
	this->AddSystem("Sys.Memory.Load", VAR_TYPE_NUMERIC, pIndexParamOnly);
	this->AddSystem("Sys.Memory.TotalPhysical", VAR_TYPE_NUMERIC, pIndexParamOnly);
	this->AddSystem("Sys.Memory.AvailablePhysical", VAR_TYPE_NUMERIC, pIndexParamOnly);
	this->AddSystem("Sys.Memory.TotalPageFile", VAR_TYPE_NUMERIC, pIndexParamOnly);
	this->AddSystem("Sys.Memory.AvailablePageFile", VAR_TYPE_NUMERIC, pIndexParamOnly);
	this->AddSystem("Sys.Memory.TotalVirtual", VAR_TYPE_NUMERIC, pIndexParamOnly);
	this->AddSystem("Sys.Memory.AvailableVirtual", VAR_TYPE_NUMERIC, pIndexParamOnly);
	//------------------------------------------------

	//-----( Numeric )----------------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("Value", VAR_TYPE_NUMERIC);
	this->AddSystem("Numeric.Equals", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("Value", VAR_TYPE_NUMERIC);
	this->AddSystem("Numeric.GreaterThan", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("Value", VAR_TYPE_NUMERIC);
	this->AddSystem("Numeric.LessThan", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("Value", VAR_TYPE_NUMERIC);
	this->AddSystem("Numeric.NotEquals", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	this->AddSystem("Numeric.Length", VAR_TYPE_NUMERIC, pIndexParamOnly);
	this->AddSystem("Numeric.ToInteger", VAR_TYPE_NUMERIC, pIndexParamOnly);
	this->AddSystem("Numeric.ToDouble", VAR_TYPE_NUMERIC, pIndexParamOnly);
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC);
	pReqParams->AddOptional("DecimalPlaces", VAR_TYPE_NUMERIC, -1);
	this->AddSystem("Numeric.ToString", VAR_TYPE_STRING, pReqParams);
	delete pReqParams;
	//------------------------------------------------

	//-----( List )----------------------------------------------------------
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("Value", VAR_TYPE_STRING);
	this->AddSystem("List.Add", VAR_TYPE_STRING, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	this->AddSystem("List.Count", VAR_TYPE_NUMERIC, pIndexParamOnly);
	//------------------------------------------------
	this->AddSystem("List.Allocated", VAR_TYPE_NUMERIC, pIndexParamOnly);
	//------------------------------------------------
	this->AddSystem("List.Clear", VAR_TYPE_INVALID, pIndexParamOnly);
	//------------------------------------------------
	this->AddSystem("List.Compact", VAR_TYPE_INVALID, pIndexParamOnly);
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("Value", VAR_TYPE_STRING);
	this->AddSystem("List.Contains", VAR_TYPE_STRING, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("Value", VAR_TYPE_STRING);
	this->AddSystem("List.DeleteOne", VAR_TYPE_STRING, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("Value", VAR_TYPE_STRING);
	this->AddSystem("List.DeleteAll", VAR_TYPE_STRING, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("Index", VAR_TYPE_NUMERIC);
	this->AddSystem("List.DeleteAt", VAR_TYPE_STRING, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("Value", VAR_TYPE_STRING);
	this->AddSystem("List.IndexOf", VAR_TYPE_STRING, pReqParams);
	delete pReqParams;
	//------------------------------------------------


	//-----( Bytes )----------------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("Count", VAR_TYPE_NUMERIC);
	this->AddSystem("Bytes.Allocate", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("Count", VAR_TYPE_NUMERIC);
	this->AddSystem("Bytes.Offset", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->AddOptional("Value", VAR_TYPE_NUMERIC, 0);
	this->AddSystem("Bytes.Initialize", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	this->AddSystem("Bytes.Free", VAR_TYPE_INVALID, pReqParams);
	delete pReqParams;
	//------------------------------------------------


	//-----( String )----------------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add("Value", VAR_TYPE_NUMERIC);
	pReqParams->AddOptional("DecimalPlaces", VAR_TYPE_NUMERIC, 0);
	this->AddSystem("FormatNumeric", VAR_TYPE_STRING, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add("Value", VAR_TYPE_NUMERIC);
	pReqParams->AddOptional("DecimalPlaces", VAR_TYPE_NUMERIC, 0);
	this->AddSystem("FormatSize", VAR_TYPE_STRING, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add("Text", VAR_TYPE_STRING);
	this->AddSystem("ToInteger", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add("Text", VAR_TYPE_STRING);
	this->AddSystem("ToDouble", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add("Text", VAR_TYPE_STRING);
	pReqParams->AddOptional("Length", VAR_TYPE_NUMERIC, -1);
	this->AddSystem("Trim", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add("OriginalString", VAR_TYPE_STRING);
	pReqParams->Add("StringToReplace", VAR_TYPE_STRING);
	pReqParams->Add("With", VAR_TYPE_STRING);
	pReqParams->AddOptional("CaseSensitive", VAR_TYPE_NUMERIC, true);
	this->AddSystem("Replace", VAR_TYPE_STRING, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->AddOptional("Length", VAR_TYPE_NUMERIC, -1);
	this->AddSystem("String.Trim", VAR_TYPE_STRING, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("StringToReplace", VAR_TYPE_STRING);
	pReqParams->Add("With", VAR_TYPE_STRING);
	pReqParams->AddOptional("CaseSensitive", VAR_TYPE_NUMERIC, true);
	this->AddSystem("String.Replace", VAR_TYPE_STRING, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("StringToFind", VAR_TYPE_STRING);
	pReqParams->AddOptional("CaseSensitive", VAR_TYPE_NUMERIC, true);
	pReqParams->AddOptional("StartPosition", VAR_TYPE_NUMERIC, (int)0);
	this->AddSystem("String.IndexOf", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add("StringToSearch", VAR_TYPE_STRING);
	pReqParams->Add("StringToFind", VAR_TYPE_STRING);
	pReqParams->AddOptional("CaseSensitive", VAR_TYPE_NUMERIC, true);
	pReqParams->AddOptional("StartPosition", VAR_TYPE_NUMERIC, (int)0);
	this->AddSystem("IndexOf", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add("Text", VAR_TYPE_STRING);
	this->AddSystem("Length", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add("Text", VAR_TYPE_STRING);
	pReqParams->Add("StartIndex", VAR_TYPE_NUMERIC);
	pReqParams->AddOptional("Length", VAR_TYPE_NUMERIC, (const char *)NULL);
	this->AddSystem("SubString", VAR_TYPE_STRING, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add("Text", VAR_TYPE_STRING);
	this->AddSystem("ToUpper", VAR_TYPE_STRING, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add("String1", VAR_TYPE_STRING);
	pReqParams->Add("String2", VAR_TYPE_STRING);
	this->AddSystem("Equals", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add("String1", VAR_TYPE_STRING);
	pReqParams->Add("String2", VAR_TYPE_STRING);
	this->AddSystem("NotEquals", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add("Text", VAR_TYPE_STRING);
	this->AddSystem("ToLower", VAR_TYPE_STRING, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("String", VAR_TYPE_STRING);
	pReqParams->AddOptional("CaseSensitive", VAR_TYPE_NUMERIC, true);
	this->AddSystem("String.Equals", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("String", VAR_TYPE_STRING);
	pReqParams->AddOptional("CaseSensitive", VAR_TYPE_NUMERIC, true);
	this->AddSystem("String.GreaterThan", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("String", VAR_TYPE_STRING);
	pReqParams->AddOptional("CaseSensitive", VAR_TYPE_NUMERIC, true);
	this->AddSystem("String.LessThan", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("String", VAR_TYPE_STRING);
	pReqParams->AddOptional("CaseSensitive", VAR_TYPE_NUMERIC, true);
	this->AddSystem("String.NotEquals", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	this->AddSystem("String.ToUpper", VAR_TYPE_STRING, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	this->AddSystem("String.ToLower", VAR_TYPE_STRING, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("StartIndex", VAR_TYPE_NUMERIC);
	pReqParams->AddOptional("Length", VAR_TYPE_NUMERIC, (const char *)NULL);
	this->AddSystem("String.SubString", VAR_TYPE_STRING, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add("Text", VAR_TYPE_STRING);
	this->AddSystem("ASCII", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add("CharCode", VAR_TYPE_NUMERIC);
	this->AddSystem("Char", VAR_TYPE_STRING, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add("Text", VAR_TYPE_STRING);
	this->AddSystem("Reverse", VAR_TYPE_STRING, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	this->AddSystem("String.Reverse", VAR_TYPE_STRING, pIndexParamOnly);
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("String", VAR_TYPE_STRING);
	this->AddSystem("String.Append", VAR_TYPE_STRING, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add("Text", VAR_TYPE_STRING);
	this->AddSystem("Base64Encode", VAR_TYPE_STRING, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add("Text", VAR_TYPE_STRING);
	this->AddSystem("Base64Decode", VAR_TYPE_STRING, pReqParams);
	delete pReqParams;
	//------------------------------------------------

	//-----( Threading )-------------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add("MilliSeconds", VAR_TYPE_NUMERIC);
	this->AddSystem("Sleep", VAR_TYPE_INVALID, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	this->AddSystem("Pause", VAR_TYPE_INVALID, pNoParams);

	//-----( Date / Time )-----------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->AddOptional("FormatText", VAR_TYPE_STRING, (const char *)NULL);
	this->AddSystem("Date.Now", VAR_TYPE_STRING, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->AddOptional("FormatText", VAR_TYPE_STRING, (const char *)NULL);
	this->AddSystem("Time.Now", VAR_TYPE_STRING, pReqParams);
	delete pReqParams;

	//-----( Math )------------------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("X", VAR_TYPE_NUMERIC);
	pReqParams->Add("Y", VAR_TYPE_NUMERIC);
	this->AddSystem("Math.Pow", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("Expression", VAR_TYPE_NUMERIC);
	pReqParams->AddOptional("Decimals", VAR_TYPE_NUMERIC, 0);
	this->AddSystem("Math.Round", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("Expression", VAR_TYPE_NUMERIC);
	this->AddSystem("Math.SmartRound", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("Expression", VAR_TYPE_STRING);
	this->AddSystem("Math.ParseDoubleExpression", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("Expression", VAR_TYPE_STRING);
	this->AddSystem("Math.ParseIntegerExpression", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("DebugMode", VAR_TYPE_NUMERIC);
	this->AddSystem("Math.DebugMode", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("Value", VAR_TYPE_NUMERIC);
	this->AddSystem("Math.Abs", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("Value", VAR_TYPE_NUMERIC);
	this->AddSystem("Math.Sin", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("Value", VAR_TYPE_NUMERIC);
	this->AddSystem("Math.Cos", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("Value", VAR_TYPE_NUMERIC);
	this->AddSystem("Math.Tan", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("Value", VAR_TYPE_NUMERIC);
	this->AddSystem("Math.Atan", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->AddOptional("Min", VAR_TYPE_NUMERIC, (const char *)NULL);
	pReqParams->AddOptional("Max", VAR_TYPE_NUMERIC, (const char *)NULL);
	this->AddSystem("Math.Random", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->AddOptional("Seed", VAR_TYPE_NUMERIC, (const char *)NULL);
	this->AddSystem("Math.SetRandom", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;

	//-----( Exceptions )-------------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("ErrorMessage", VAR_TYPE_STRING);
	this->AddSystem("Error.Throw", VAR_TYPE_INVALID, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	this->AddSystem("Error.UnWind", VAR_TYPE_NUMERIC, pIndexParamOnly);
	this->AddSystem("Error.Count", VAR_TYPE_NUMERIC, pIndexParamOnly);
	this->AddSystem("Error.Clear", VAR_TYPE_NUMERIC, pIndexParamOnly);
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->AddOptional("Index", VAR_TYPE_NUMERIC, 0);
	this->AddSystem("Error.Text", VAR_TYPE_STRING, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->AddOptional("Index", VAR_TYPE_NUMERIC, 0);
	this->AddSystem("Error.File", VAR_TYPE_STRING, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->AddOptional("Index", VAR_TYPE_NUMERIC, 0);
	this->AddSystem("Error.Line", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	this->AddSystem("Exception.GetText", VAR_TYPE_STRING, pIndexParamOnly);
	this->AddSystem("Exception.GetLine", VAR_TYPE_NUMERIC, pIndexParamOnly);
	this->AddSystem("Exception.GetFile", VAR_TYPE_STRING, pIndexParamOnly);
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("Text", VAR_TYPE_STRING);
	this->AddSystem("Exception.SetText", VAR_TYPE_INVALID, pReqParams);
	delete pReqParams;

	//-----( File )-----------------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("FullPath", VAR_TYPE_STRING);
	this->AddSystem("File.Drive", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("FullPath", VAR_TYPE_STRING);
	this->AddSystem("File.Path", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("FullPath", VAR_TYPE_STRING);
	this->AddSystem("File.Name", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("FullPath", VAR_TYPE_STRING);
	this->AddSystem("File.NameOnly", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("FullPath", VAR_TYPE_STRING);
	this->AddSystem("File.Extension", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("FileName", VAR_TYPE_STRING);
	this->AddSystem("File.Exists", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("FileName", VAR_TYPE_STRING);
	this->AddSystem("File.CanRead", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("FileName", VAR_TYPE_STRING);
	this->AddSystem("File.CanWrite", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("FileName", VAR_TYPE_STRING);
	this->AddSystem("File.CanReadWrite", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("OldFileName", VAR_TYPE_STRING);
	pReqParams->Add("NewFileName", VAR_TYPE_STRING);
	this->AddSystem("File.Rename", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("OldFileName", VAR_TYPE_STRING);
	pReqParams->Add("NewFileName", VAR_TYPE_STRING);
	this->AddSystem("File.Move", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("OldFileName", VAR_TYPE_STRING);
	pReqParams->Add("NewFileName", VAR_TYPE_STRING);
	pReqParams->AddOptional("FailIfExists", VAR_TYPE_NUMERIC, (int)1);
	this->AddSystem("File.Copy", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("FileName", VAR_TYPE_STRING);
	this->AddSystem("File.Delete", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("FileName", VAR_TYPE_STRING);
	pReqParams->AddOptional("OpenMode", VAR_TYPE_NUMERIC, (int)-1);
	pReqParams->AddOptional("CreationDisposition", VAR_TYPE_NUMERIC, (int)-1);
	this->AddSystem("File.Open", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	this->AddSystem("File.Close", VAR_TYPE_INVALID, pIndexParamOnly);
	this->AddSystem("File.Handle", VAR_TYPE_NUMERIC, pIndexParamOnly);
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->AddOptional("NumberOfBytes", VAR_TYPE_NUMERIC, (int)-1);
	this->AddSystem("File.Read", VAR_TYPE_STRING, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("Data", VAR_TYPE_STRING);
	pReqParams->AddOptional("NumberOfBytes", VAR_TYPE_NUMERIC, -1);
	this->AddSystem("File.Write", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->AddOptional("Distance", VAR_TYPE_NUMERIC, 0);
	this->AddSystem("File.Seek", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->AddOptional("Distance", VAR_TYPE_NUMERIC, 0);
	this->AddSystem("File.Seek.FromBeginning", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->AddOptional("Distance", VAR_TYPE_NUMERIC, 0);
	this->AddSystem("File.Seek.FromEnd", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->AddOptional("FileName", VAR_TYPE_STRING, "");
	this->AddSystem("File.Size", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------

	//-----( Web )-------------------------------------------------------------
	this->AddSystem("Web.InitializeCGI", VAR_TYPE_INVALID, pIndexParamOnly);
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("Text", VAR_TYPE_STRING);
	pReqParams->AddOptional("Length", VAR_TYPE_NUMERIC, (int)-1);
	this->AddSystem("Web.URLEncode", VAR_TYPE_STRING, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("Text", VAR_TYPE_STRING);
	this->AddSystem("Web.URLDecode", VAR_TYPE_STRING, pReqParams);
	delete pReqParams;

	//-----( State )-----------------------------------------------------------
	this->AddSystem("Code.Scope", VAR_TYPE_NUMERIC, pIndexParamOnly);
	this->AddSystem("Code.Scope.Enter", VAR_TYPE_INVALID, pIndexParamOnly);
	this->AddSystem("Code.Scope.Exit", VAR_TYPE_INVALID, pIndexParamOnly);
	this->AddSystem("Code.Line", VAR_TYPE_NUMERIC, pIndexParamOnly);
	this->AddSystem("Code.File", VAR_TYPE_STRING, pIndexParamOnly);
	this->AddSystem("Code.Namespace", VAR_TYPE_STRING, pIndexParamOnly);
	//------------------------------------------------
	this->AddSystem("String.ToDouble", VAR_TYPE_NUMERIC, pIndexParamOnly);
	this->AddSystem("String.ToInteger", VAR_TYPE_NUMERIC, pIndexParamOnly);
	this->AddSystem("String.Length", VAR_TYPE_NUMERIC, pIndexParamOnly);
	this->AddSystem("String.Free", VAR_TYPE_INVALID, pIndexParamOnly);
	this->AddSystem("Code.Variables.Free", VAR_TYPE_INVALID, pIndexParamOnly);
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("VariableName", VAR_TYPE_STRING);
	this->AddSystem("Code.Variables.IsDefined", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("FunctionName", VAR_TYPE_STRING);
	this->AddSystem("Code.Functions.IsDefined", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("DynamicExpression", VAR_TYPE_STRING);
	this->AddSystem("Code.Functions.Call", VAR_TYPE_STRING, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("VariableName", VAR_TYPE_STRING);
	this->AddSystem("Code.Variables.Undefine", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("VariableName", VAR_TYPE_STRING);
	pReqParams->Add("TypeName", VAR_TYPE_STRING);
	pReqParams->AddOptional("Expression", VAR_TYPE_STRING, "");
	pReqParams->AddOptional("Scope", VAR_TYPE_NUMERIC, -1);
	this->AddSystem("Code.Variables.Define", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("CodeText", VAR_TYPE_STRING);
	this->AddSystem("Code.Inject", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("Filename", VAR_TYPE_STRING);
	this->AddSystem("Code.Append", VAR_TYPE_INVALID, pReqParams);
	delete pReqParams;
	//------------------------------------------------

	//-----( Debug.IsAttached )--------------------------------------------------
	this->AddSystem("Debug.IsAttached", VAR_TYPE_NUMERIC, pIndexParamOnly);

	//-----( Debug.Evaluate )--------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("CodeText", VAR_TYPE_STRING);
	this->AddSystem("Debug.Evaluate", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;

	//-----( Debug Break )--------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->AddOptional("BreakCondition", VAR_TYPE_NUMERIC, 1);
	this->AddSystem("Debug.Break", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;

	//-----( SQL Server )-----------------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("ConnectionString", VAR_TYPE_STRING);
	this->AddSystem("SQL.Connection.Connect", VAR_TYPE_INVALID, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->AddOptional("Name", VAR_TYPE_STRING, (const char *)NULL);
	this->AddSystem("SQL.Connection.Transaction.Begin", VAR_TYPE_INVALID, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->AddOptional("Name", VAR_TYPE_STRING, (const char *)NULL);
	this->AddSystem("SQL.Connection.Transaction.Commit", VAR_TYPE_INVALID, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->AddOptional("Name", VAR_TYPE_STRING, (const char *)NULL);
	this->AddSystem("SQL.Connection.Transaction.Rollback", VAR_TYPE_INVALID, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	this->AddSystem("SQL.Connection.Transaction.Depth", VAR_TYPE_NUMERIC, pIndexParamOnly);
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("SQLText", VAR_TYPE_STRING);
	this->AddSystem("SQL.Connection.Value", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("SQLText", VAR_TYPE_STRING);
	pReqParams->AddOptional("RecordSet", VAR_TYPE_SQLRECORDSET, INVALID_INDEX);
	this->AddSystem("SQL.Connection.Execute", VAR_TYPE_INVALID, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("NameOrIndex", VAR_TYPE_STRING);
	this->AddSystem("SQL.RecordSet.Value", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("ColumnIndex", VAR_TYPE_NUMERIC);
	this->AddSystem("SQL.RecordSet.Columns.Name", VAR_TYPE_STRING, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("ColumnName", VAR_TYPE_STRING);
	this->AddSystem("SQL.RecordSet.Columns.Index", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	this->AddSystem("SQL.RecordSet.Columns.Count", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	this->AddSystem("SQL.RecordSet.Fetch", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	this->AddSystem("SQL.RecordSet.Close", VAR_TYPE_INVALID, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	this->AddSystem("SQL.Connection.Close", VAR_TYPE_INVALID, pReqParams);
	delete pReqParams;

	//-----( Form )-----------------------------------------------------------
	this->AddSystem("Form.Value.Count", VAR_TYPE_NUMERIC, pIndexParamOnly);
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("FieldName", VAR_TYPE_STRING);
	this->AddSystem("Form.Value.IsDefined", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("FieldName", VAR_TYPE_STRING);
	pReqParams->AddOptional("FieldValue", VAR_TYPE_STRING, "");
	this->AddSystem("Form.Value.Define", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("FieldIndex", VAR_TYPE_NUMERIC);
	this->AddSystem("Form.Value.ByIndex", VAR_TYPE_STRING, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("FieldName", VAR_TYPE_STRING);
	this->AddSystem("Form.Value.Index", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("FieldName", VAR_TYPE_STRING);
	this->AddSystem("Form.Value.ByName", VAR_TYPE_STRING, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("FieldIndex", VAR_TYPE_NUMERIC);
	this->AddSystem("Form.Value.Name", VAR_TYPE_STRING, pReqParams);
	delete pReqParams;
	//------------------------------------------------

	//-----( Query )-----------------------------------------------------------
	this->AddSystem("Query.Value.Count", VAR_TYPE_NUMERIC, pIndexParamOnly);
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("FieldName", VAR_TYPE_STRING);
	this->AddSystem("Query.Value.IsDefined", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("FieldName", VAR_TYPE_STRING);
	pReqParams->AddOptional("FieldValue", VAR_TYPE_STRING, "");
	this->AddSystem("Query.Value.Define", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("FieldIndex", VAR_TYPE_NUMERIC);
	this->AddSystem("Query.Value.ByIndex", VAR_TYPE_STRING, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("FieldName", VAR_TYPE_STRING);
	this->AddSystem("Query.Value.Index", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("FieldName", VAR_TYPE_STRING);
	this->AddSystem("Query.Value.ByName", VAR_TYPE_STRING, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("FieldIndex", VAR_TYPE_NUMERIC);
	this->AddSystem("Query.Value.Name", VAR_TYPE_STRING, pReqParams);
	delete pReqParams;
	//------------------------------------------------


	//-----( Misc. )-----------------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add("Text", VAR_TYPE_STRING);
	pReqParams->AddOptional("Length", VAR_TYPE_NUMERIC, (int)-1);
	this->AddSystem("SHA1", VAR_TYPE_STRING, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add("Text", VAR_TYPE_STRING);
	pReqParams->AddOptional("Length", VAR_TYPE_NUMERIC, (int)-1);
	this->AddSystem("Checksum", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;

	//-----( Settings )-----------------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->AddOptional("Value", VAR_TYPE_NUMERIC, (int)-1);
	this->AddSystem("Settings.ThrowSoftExceptions", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;

	//-----( XML[Writer] )-----------------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("FileName", VAR_TYPE_STRING);
	this->AddSystem("XML.Writer.Save", VAR_TYPE_INVALID, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	this->AddSystem("XML.Writer.Text", VAR_TYPE_STRING, pIndexParamOnly);
	//------------------------------------------------
	this->AddSystem("XML.Writer.Length", VAR_TYPE_NUMERIC, pIndexParamOnly);
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("Name", VAR_TYPE_STRING);
	pReqParams->Add("Value", VAR_TYPE_STRING);
	this->AddSystem("XML.Writer.Add", VAR_TYPE_INVALID, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("XMLWriter", VAR_TYPE_XMLWRITER);
	this->AddSystem("XML.Writer.AddXML", VAR_TYPE_INVALID, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("Name", VAR_TYPE_STRING);
	this->AddSystem("XML.Writer.SetName", VAR_TYPE_INVALID, pReqParams);
	delete pReqParams;
	//------------------------------------------------

	//-----( XML[Reader] )-----------------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("FileName", VAR_TYPE_STRING);
	this->AddSystem("XML.Reader.Open", VAR_TYPE_INVALID, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("Enable", VAR_TYPE_NUMERIC);
	this->AddSystem("XML.Reader.ProgressiveScan", VAR_TYPE_INVALID, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	this->AddSystem("XML.Reader.Close", VAR_TYPE_NUMERIC, pIndexParamOnly);
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("TagName", VAR_TYPE_STRING);
	pReqParams->AddOptional("DefaultValue", VAR_TYPE_STRING, "");
	this->AddSystem("XML.Reader.ToString", VAR_TYPE_STRING, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("TagName", VAR_TYPE_STRING);
	pReqParams->AddOptional("DefaultValue", VAR_TYPE_NUMERIC, 0);
	this->AddSystem("XML.Reader.ToInteger", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("TagName", VAR_TYPE_STRING);
	pReqParams->AddOptional("DefaultValue", VAR_TYPE_NUMERIC, 0);
	this->AddSystem("XML.Reader.ToDouble", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("TagName", VAR_TYPE_STRING);
	pReqParams->Add("Reader", VAR_TYPE_XMLREADER);
	this->AddSystem("XML.Reader.ToReader", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	this->AddSystem("XML.Reader.Text", VAR_TYPE_STRING, pIndexParamOnly);
	//------------------------------------------------
	this->AddSystem("XML.Reader.Length", VAR_TYPE_NUMERIC, pIndexParamOnly);
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("FileName", VAR_TYPE_STRING);
	this->AddSystem("XML.Reader.ToFile", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->AddOptional("Position", VAR_TYPE_NUMERIC, -1);
	this->AddSystem("XML.Reader.ScanPosition", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("XMLText", VAR_TYPE_STRING);
	this->AddSystem("XML.Reader.Parse", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("TagName", VAR_TYPE_STRING);
	this->AddSystem("XML.Reader.AttributeLength", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------

	//-----( Sockets[Server] )-----------------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->AddOptional("MaxConnections", VAR_TYPE_NUMERIC, (int)10);
	pReqParams->AddOptional("ListenPort", VAR_TYPE_NUMERIC, (int)0);
	this->AddSystem("Socket.Server.Start", VAR_TYPE_INVALID, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	this->AddSystem("Socket.Server.Stop", VAR_TYPE_INVALID, pIndexParamOnly);
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("Host", VAR_TYPE_STRING);
	pReqParams->Add("Port", VAR_TYPE_NUMERIC);
	pReqParams->Add("OutputClient", VAR_TYPE_SOCKETCLIENT);
	this->AddSystem("Socket.Server.Connect", VAR_TYPE_INVALID, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	this->AddSystem("Socket.Server.Connections", VAR_TYPE_NUMERIC, pIndexParamOnly);
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("OutputClient", VAR_TYPE_SOCKETCLIENT);
	pReqParams->AddOptional("PeerIndex", VAR_TYPE_NUMERIC, (int)-1);
	this->AddSystem("Socket.Server.Connection", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------

	//-----( Sockets[Client] )-----------------------------------------------------------
	this->AddSystem("Socket.Client.WasConnect", VAR_TYPE_NUMERIC, pIndexParamOnly);
	this->AddSystem("Socket.Client.IsDisconnectSet", VAR_TYPE_NUMERIC, pIndexParamOnly);
	this->AddSystem("Socket.Client.PeerAddress", VAR_TYPE_STRING, pIndexParamOnly);
	this->AddSystem("Socket.Client.PeerID", VAR_TYPE_NUMERIC, pIndexParamOnly);
	this->AddSystem("Socket.Client.PeerPort", VAR_TYPE_NUMERIC, pIndexParamOnly);
	this->AddSystem("Socket.Client.IsConnected", VAR_TYPE_NUMERIC, pIndexParamOnly);
	this->AddSystem("Socket.Client.IsSendPending", VAR_TYPE_NUMERIC, pIndexParamOnly);
	this->AddSystem("Socket.Client.IsRecvPending", VAR_TYPE_NUMERIC, pIndexParamOnly);
	this->AddSystem("Socket.Client.Recv", VAR_TYPE_STRING, pIndexParamOnly);
	this->AddSystem("Socket.Client.Disconnect", VAR_TYPE_INVALID, pIndexParamOnly);
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("Data", VAR_TYPE_STRING);
	this->AddSystem("Socket.Client.Send", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	this->AddSystem("Socket.Client.TryRecv", VAR_TYPE_STRING, pIndexParamOnly);
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("Data", VAR_TYPE_STRING);
	this->AddSystem("Socket.Client.TrySend", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------

	//-----( Documentation )-----------------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add("OutputPath", VAR_TYPE_STRING);
	this->AddSystem("GenerateDocumentation", VAR_TYPE_INVALID, pReqParams);
	delete pReqParams;
	//------------------------------------------------
	//-----( Special Debugging )-----------------------------------------------------------
	this->AddSystem("Debug.Dump.Memory", VAR_TYPE_INVALID, pIndexParamOnly);
	this->AddSystem("Debug.Dump.Vars", VAR_TYPE_INVALID, pIndexParamOnly);
	this->AddSystem("Debug.Dump.Procs", VAR_TYPE_INVALID, pIndexParamOnly);
	this->AddSystem("Debug.Dump.Scripts", VAR_TYPE_INVALID, pIndexParamOnly);
	this->AddSystem("Debug.Dump.Lines", VAR_TYPE_INVALID, pIndexParamOnly);
	//------------------------------------------------

	//-----( COM)-----------------------------------------------------------
	this->AddSystem("GUID", VAR_TYPE_STRING, pNoParams);

#ifdef COM_PROCEDURE_CALL_IMPLEMENTATION
	//------------------------------------------------
	pReqParams = new CReqParams(this->pEngine);
	pReqParams->Add(PARENT_INDEX_VAR_NAME, VAR_TYPE_NUMERIC); //Variable index.
	pReqParams->Add("ClassName", VAR_TYPE_STRING);
	this->AddSystem("COM.Object.CreateObject", VAR_TYPE_NUMERIC, pReqParams);
	delete pReqParams;
	//------------------------------------------------
#endif //COM_PROCEDURE_CALL_IMPLEMENTATION

	delete pNoParams;
	delete pIndexParamOnly;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
ErrorLevel CProcs::GenerateDocumentation(const char *sOutputPath)
{
	CEngine *pEngine = (CEngine *) this->pEngine;

	const char *paramColor = "#000088";
	const char *identifierColor = "0000EE";
	const char *typeColor = "MediumVioletRed";

	Stack indexAdded;

	for (int iProc = 0; iProc < this->Collection.Count; iProc++)
	{
		FILE *hFile = NULL;

		char sFileName[MAX_PATH];
		sprintf_s(sFileName, "%s\\%s.htm", sOutputPath, this->Collection.Items[iProc].Name);
		CorrectForwardPath(sFileName, sizeof(sFileName));
		CreateFolderStructure(sOutputPath);

		if (fopen_s(&hFile, sFileName, "wb") == 0)
		{
			char sNameSpace[1024];
			ReplaceStrings(this->Collection.Items[iProc].Name, ".", " :: ", sNameSpace, sizeof(sNameSpace));

			indexAdded.Push(this->Collection.Items[iProc].Name);

			fprintf(hFile, "<html>\r\n");
			fprintf(hFile, "<head>\r\n");
			fprintf(hFile, "<title>%s</title>\r\n", sNameSpace);
			fprintf(hFile, "</head>\r\n\r\n");
			fprintf(hFile, "<body Background=\"/Images/BGCode.gif\">\r\n");

			fprintf(hFile, "<font size=\"5\">%s</font><br />\r\n", sNameSpace);
			fprintf(hFile, "<hr /><br /><br />\r\n\r\n");

			fprintf(hFile, "<b>Description:</b>\r\n");
			fprintf(hFile, "<blockquote>\r\n");
			fprintf(hFile, "<!-- ##BEGIN_DESCRIPTION## -->\r\n");
			fprintf(hFile, "Description_Text\r\n");
			fprintf(hFile, "<!-- ##END_DESCRIPTION## -->\r\n");
			fprintf(hFile, "</blockquote>\r\n\r\n");

			fprintf(hFile, "<b>Syntax:</b>\r\n");
			fprintf(hFile, "<blockquote>\r\n");

			fprintf(hFile, "<font color=\"%s\">%s</font>", identifierColor, this->Collection.Items[iProc].Name);
			fprintf(hFile, "(");

			StringBuilder html;

			if (this->Collection.Items[iProc].ReqParams)
			{
				if (this->Collection.Items[iProc].ReqParams->Collection.Count > 0)
				{
					html.AppendF("<ul>");
					for (int iParam = 0; iParam < this->Collection.Items[iProc].ReqParams->Collection.Count; iParam++)
					{
						REQPARAMITEM *pParam = &this->Collection.Items[iProc].ReqParams->Collection.Items[iParam];

						if (strcmp(pParam->Name, PARENT_INDEX_VAR_NAME) != 0)
						{
							html.AppendF("<li><font color=\"%s\">%s</font> as <font color=\"%s\">%s</font><ul><li>",
								paramColor, pParam->Name, typeColor, pEngine->Types->Name(pParam->Type));
							html.AppendF("\r\nDescription_Text\r\n");
							html.AppendF("</ul>");

							fprintf(hFile, "<font color=\"%s\">%s</font>", paramColor, pParam->Name);

							fprintf(hFile, " as <font color=\"%s\">%s</font>", typeColor, pEngine->Types->Name(pParam->Type));

							if (pParam->IsOptional)
							{
								fprintf(hFile, " = ");
								if (pParam->Default.Length > 0)
								{
									if (pParam->Type == VAR_TYPE_STRING)
									{
										fprintf(hFile, "<font color=\"%s\">\"%s\"</font>", "Red", pParam->Default.Value);
									}
									else {
										fprintf(hFile, "<font color=\"%s\">%s</font>", "Blue", pParam->Default.Value);
									}
								}
								else {
									fprintf(hFile, "NULL");
								}
							}

							if (iParam != this->Collection.Items[iProc].ReqParams->Collection.Count - 1)
							{
								fprintf(hFile, ", ");
							}
						}
					}
					html.AppendF("</ul>");
				}
			}
			else {
				fprintf(hFile, "..."); //infinite params.
			}
			fprintf(hFile, ")");

			if (this->Collection.Items[iProc].ReturnType != INVALID_INDEX)
			{
				fprintf(hFile, " as <font color=\"%s\">%s</font> ", typeColor, pEngine->Types->Name(this->Collection.Items[iProc].ReturnType));
			}

			fprintf(hFile, "</blockquote>\r\n\r\n");

			fprintf(hFile, "<b>Parameters:</b>\r\n");
			fprintf(hFile, "<blockquote>\r\n");
			if (html.Length > 0)
			{
				fprintf(hFile, "%s\r\n", html.Buffer);
			}
			else {
				if (this->Collection.Items[iProc].ReqParams)
				{
					fprintf(hFile, "n/a");
				}
				else {
					fprintf(hFile, "<i>Infinite parameters of any type.</i>");
				}
			}
			fprintf(hFile, "</blockquote>\r\n\r\n");

			fprintf(hFile, "<b>Example:</b>\r\n");
			fprintf(hFile, "<blockquote>\r\n");
			fprintf(hFile, "Description_Text\r\n");
			fprintf(hFile, "</blockquote>\r\n\r\n");

			fprintf(hFile, "</body>\r\n");
			fprintf(hFile, "</html>\r\n");

			fclose(hFile);
		}
	}

	//---------------------------------------------------------------------------------------------------------------------------------------------------

	{ //Write index.
		FILE *hFile = NULL;

		char sFileName[MAX_PATH];
		sprintf_s(sFileName, "%s\\Index.hhk", sOutputPath);
		CorrectForwardPath(sFileName, sizeof(sFileName));
		CreateFolderStructure(sOutputPath);

		if (fopen_s(&hFile, sFileName, "wb") == 0)
		{
			fprintf(hFile, "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML//EN\">\r\n");
			fprintf(hFile, "<HTML>\r\n");
			fprintf(hFile, "<HEAD>\r\n");
			fprintf(hFile, "<meta name=\"GENERATOR\" content=\"Microsoft&reg; HTML Help Workshop 4.1\">\r\n");
			fprintf(hFile, "<!-- Sitemap 1.0 -->\r\n");
			fprintf(hFile, "</HEAD><BODY>\r\n");
			fprintf(hFile, "<UL>\r\n");

			for (int iOuterProc = 0; iOuterProc < this->Collection.Count; iOuterProc++)
			{
				char sTok[255];
				int iRPos = 0;
				int iTokSz = 0;

				fprintf(hFile, "<LI> <OBJECT type=\"text/sitemap\">\r\n");
				fprintf(hFile, "<param name=\"Name\" value=\"%s\">\r\n", this->Collection.Items[iOuterProc].Name);
				fprintf(hFile, "<param name=\"Local\" value=\"Script\\%s.htm\">\r\n", this->Collection.Items[iOuterProc].Name);
				fprintf(hFile, "</OBJECT>\r\n");

				while ((iTokSz = GetNextToken(this->Collection.Items[iOuterProc].Name, strlen(this->Collection.Items[iOuterProc].Name), sTok, sizeof(sTok), &iRPos, ".", 1)))
				{
					iRPos++;

					_STACKITEM *stackItem;

					bool bAlreadyUsed = false;

					for (int index = 0; (stackItem = indexAdded.Peek(index)); index++)
					{
						if (_strcmpi((char *)stackItem->Value, sTok) == 0)
						{
							bAlreadyUsed = true;
							break;
						}
					}

					if (bAlreadyUsed)
					{
						continue;
					}

					fprintf(hFile, "<LI> <OBJECT type=\"text/sitemap\">\r\n");
					fprintf(hFile, "<param name=\"Name\" value=\"%s\">\r\n", sTok);

					indexAdded.Push(sTok);

					for (int iInnerProc = 0; iInnerProc < this->Collection.Count; iInnerProc++)
					{
						if (InStrI(sTok, this->Collection.Items[iInnerProc].Name) >= 0)
						{
							fprintf(hFile, "<param name=\"Name\" value=\"%s\">\r\n", this->Collection.Items[iInnerProc].Name);
							fprintf(hFile, "<param name=\"Local\" value=\"Script\\%s.htm\">\r\n", this->Collection.Items[iInnerProc].Name);
						}
					}

					fprintf(hFile, "</OBJECT>\r\n");
				}
			}

			fprintf(hFile, "</UL>\r\n");
			fprintf(hFile, "</BODY></HTML>\r\n");

			fclose(hFile);
		}
	}

	//---------------------------------------------------------------------------------------------------------------------------------------------------

	{ //Write types file.
		Stack addedKeywords;
		FILE *hFile = NULL;

		char sFileName[MAX_PATH];
		sprintf_s(sFileName, "%s\\@Types.txt", sOutputPath);
		CorrectForwardPath(sFileName, sizeof(sFileName));
		CreateFolderStructure(sOutputPath);

		if (fopen_s(&hFile, sFileName, "wb") == 0)
		{
			for(int i = 0; i < pEngine->Types->Count(); i++)
			{
				TYPEITEM *pType = pEngine->Types->GetPointer(i);

				fprintf(hFile, "%s ", pType->Name);
			}
		}
		fclose(hFile);
	}

	{ //Write keyword file.
		VectorBinary addedKeywords;
		FILE *hFile = NULL;

		char sFileName[MAX_PATH];
		sprintf_s(sFileName, "%s\\@Keywords.txt", sOutputPath);
		CorrectForwardPath(sFileName, sizeof(sFileName));
		CreateFolderStructure(sOutputPath);

		if (fopen_s(&hFile, sFileName, "wb") == 0)
		{
			for (int i = 0; i < this->Collection.Count; i++)
			{
				fprintf(hFile, "%s ", this->Collection.Items[i].Name);
				addedKeywords.Add(this->Collection.Items[i].Name);

				char sTok[255];
				int iRPos = 0;
				int iTokSz = 0;

				while ((iTokSz = GetNextToken(this->Collection.Items[i].Name, strlen(this->Collection.Items[i].Name), sTok, sizeof(sTok), &iRPos, ".", 1)))
				{
					if(this->Collection.Items[i].Name[iRPos] == '.')
					{
						iRPos++;

						char typelessKeyword[MAX_TOKEN_SIZE];
						strcpy_s(typelessKeyword, sizeof(typelessKeyword), this->Collection.Items[i].Name);

						for(int t = 0; t < pEngine->Types->Count(); t++)
						{
							TYPEITEM *pType = pEngine->Types->GetPointer(t);

							char sType[MAX_TOKEN_SIZE];
							sprintf_s(sType, sizeof(sType), "%s.", pType->Name);

							char sOutKeyword[MAX_TOKEN_SIZE];
							ReplaceStrings(typelessKeyword, sType, "", sOutKeyword, sizeof(sOutKeyword));
							strcpy_s(typelessKeyword, sizeof(typelessKeyword), sOutKeyword);
						}

						if(_strcmpi(typelessKeyword, this->Collection.Items[i].Name) != 0)
						{
							if (addedKeywords.Contiains(typelessKeyword))
							{
								continue;
							}

							addedKeywords.Add(typelessKeyword);

							fprintf(hFile, "%s ", typelessKeyword);
						}
					}

					if(pEngine->Types->IsDefined(sTok) == false)
					{
						if (addedKeywords.Contiains(sTok))
						{
							continue;
						}

						addedKeywords.Add(sTok);

						fprintf(hFile, "%s ", sTok);
					}
				}
			}

			fclose(hFile);
		}
	}


	return ERROR_OK;
}
#endif _DEBUG

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CProcs::~CProcs()
{
	if (this->Collection.Count)
	{
		for (int iItem = 0; iItem < this->Collection.Count; iItem++)
		{
			if (this->Collection.Items[iItem].ReqParams)
			{
				delete this->Collection.Items[iItem].ReqParams;
			}

			if (this->Collection.Items[iItem].Name)
			{
				pMem->Free(this->Collection.Items[iItem].Name);
			}

#ifdef EXTERNAL_PROCEDURE_CALL_IMPLEMENTATION
			//if(this->Collection.Items[iItem].Type == PROC_TYPE_EXTERNAL)
			{
				if (this->Collection.Items[iItem].Alias)
				{
					pMem->Free(this->Collection.Items[iItem].Alias);
				}
				if (this->Collection.Items[iItem].Library)
				{
					pMem->Free(this->Collection.Items[iItem].Library);
				}
			}
			this->Collection.Items[iItem].CallingConvention = EXTERNAL_TYPE_INVALID;
#endif //EXTERNAL_PROCEDURE_CALL_IMPLEMENTATION

			this->Collection.Items[iItem].Type = PROC_TYPE_INVALID;
			this->Collection.Items[iItem].Name = NULL;
		}
		pMem->Free(this->Collection.Items);
	}
	memset(&this->Collection, 0, sizeof(this->Collection));

	if (this->sSQLBuffer)
	{
		pMem->Free(this->sSQLBuffer);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CProcs::IsDefined(const char *sName)
{
	return this->IsDefined(this->GetIndex(sName));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CProcs::IsDefined(int iIndex)
{
	return (iIndex >= 0 && iIndex < this->Collection.Count && this->Collection.Items[iIndex].Name);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CProcs::Count(void)
{
	return this->Collection.Count;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PROCITEM *CProcs::GetPointer(int iIndex)
{
	if (this->IsDefined(iIndex))
	{
		return &this->Collection.Items[iIndex];
	}

	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PROCITEM *CProcs::GetPointer(const char *sName)
{
	//Search for built-in items first, they have priority.
	for (int iItem = 0; iItem < this->_EndOfBuiltInIndex; iItem++)
	{
		if (_StrEqlI(this->Collection.Items[iItem].Name, sName))
		{
			return &this->Collection.Items[iItem];
		}
	}

	//Then search for user items in reverse order, because the required object is
	//	almost always one of the most recent added.
	for (int iItem = this->Collection.Count - 1; iItem >= this->_EndOfBuiltInIndex; iItem--)
	{
		if (_StrEqlI(this->Collection.Items[iItem].Name, sName))
		{
			return &this->Collection.Items[iItem];
		}
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ProcIndex CProcs::GetIndex(const char *sName)
{
	for (int iItem = 0; iItem < this->Collection.Count; iItem++)
	{
		if (_StrEqlI(this->Collection.Items[iItem].Name, sName))
		{
			return iItem;
		}
	}
	return INVALID_INDEX;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Parses a function call in the given text. Executes it recursively and returns a value.
*/
ErrorLevel CProcs::ExecuteByEval(const char *sText, int iLength, CParams *pAdditionalParams, CParams *pReturn)
{
	char sProcName[MAX_TOKEN_SIZE];
	int iTokSz = 0;
	int iRPos = 0;

	int iIndex = INVALID_INDEX;

	if (TokenWithDot(sText, iLength, sProcName, sizeof(sProcName), &iRPos) <= 0)
	{
		return this->Error->Hard("Invalid token");
	}

	if ((iIndex = this->GetIndex(sProcName)) == INVALID_INDEX)
	{
		return this->Error->Hard("Undefined identifier: [%s]", sProcName);
	}

	int iBegBrace = 0;
	int iEndBrace = 0;

	if (((CEngine*)this->pEngine)->GetParenIndexPositions(sText, iLength, &iRPos, &iBegBrace, &iEndBrace) != ERROR_OK)
	{
		return this->Error->HardCount();
	}

	if ((iEndBrace - iBegBrace) < 0)
	{
		return this->Error->Hard("Parenthesis required for procedure call");
	}

	CParams *pParams = new CParams(this->pEngine);
	CParams *pUserParams = NULL;
	if (iEndBrace - iBegBrace > 0)
	{
		ErrorLevel result = ((CEngine*)this->pEngine)->Evaluate(sText + iBegBrace, (iEndBrace - iBegBrace), &pUserParams);
		if (result != ERROR_OK)
		{
			if (pUserParams)
			{
				delete pUserParams;
			}
			return result;
		}
	}

	//The addition of recursive properties blows this check out of the water...
	//if(iEndBrace != iLength - 1)
	//{
		//return this->Error->Hard("Parenthesis mismatch");
	//}

	//Add any parameters that were passed into this function for inclusion before adding the parsed params.
	if (pAdditionalParams)
	{
		for (int iAddParam = 0; iAddParam < pAdditionalParams->Collection.Count; iAddParam++)
		{
			pParams->Add(&pAdditionalParams->Collection.Items[iAddParam]);
		}
	}

	//Add any parsed parameters
	if (pUserParams)
	{
		for (int iAddParam = 0; iAddParam < pUserParams->Collection.Count; iAddParam++)
		{
			pParams->Add(&pUserParams->Collection.Items[iAddParam]);
		}
		delete pUserParams;
	}

	PROCITEM *pProc = this->GetPointer(iIndex);

	int iSepcifiedParams = 0;
	if (pParams)
	{
		iSepcifiedParams = pParams->Collection.Count;
	}

	if (pProc->ReqParams)
	{
		//Fill in optional parameters.
		if (iSepcifiedParams != pProc->ReqParams->Collection.Count)
		{
			if (!pParams)
			{
				pParams = new CParams(this->pEngine);
			}

			while (iSepcifiedParams < pProc->ReqParams->Collection.Count)
			{
				if (pProc->ReqParams->Collection.Items[iSepcifiedParams].IsOptional)
				{
					pParams->Add(pProc->ReqParams->Collection.Items[iSepcifiedParams].Default.Value,
						pProc->ReqParams->Collection.Items[iSepcifiedParams].Default.Length);
					iSepcifiedParams++;
				}
				else {
					delete pParams;

					return this->Error->Hard("Parameter [%s] of [%s] is not optional",
						pProc->ReqParams->Collection.Items[iSepcifiedParams].Name,
						pProc->Name);
				}
			}
		}
	}

	ErrorLevel Error = this->Execute(iIndex, pParams, pReturn);

	if (pParams)
	{
		delete pParams;
	}

	return Error;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function will execute a given function and return its value.
*/
ErrorLevel CProcs::Execute(int iIndex, CParams *pParams, CParams *pReturn)
{
	CEngine *pEngine = ((CEngine *)this->pEngine);

	//Save the scope before the function call.
	int iScope = pEngine->State->CurrentScope();
	//int iTryScope = ((CEngine *)this->pEngine)->State->TryScope;

	//Enter a new scope for the function call.
	pEngine->State->ScopeEnterMajor();

	int iPreviousMinimumScope = pEngine->State->MinimumScope;
	pEngine->State->MinimumScope = iScope;

	ErrorLevel iErrorLevel = ExecuteInternal(iIndex, pParams, pReturn);

	//If the proc call was a "script proc" then we need to roll back all scopes that were
	//	entered because the function could have called Code.Scope.Enter() or could have returned
	//	from inside an IF, WHILE, FOR or similar scope.
	if (this->Collection.Items[iIndex].Type == PROC_TYPE_SCRIPT)
	{
		//while(pEngine->State->TryScope != iTryScope)
		//{
		//	pEngine->State->TryDrop();
		//}

		while (pEngine->State->CurrentScope() != iScope)
		{
			pEngine->State->ScopeDrop();
		}
	}
	else {
		//If calling a non "script proc" then we only drop the scope that was
		//	created for this function because the multiple scopes could
		//	be intentional (e.g. a call to Code.Scope.Enter()).
		pEngine->State->ScopeDrop();
	}

	pEngine->State->MinimumScope = iPreviousMinimumScope;

	return iErrorLevel;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This is the internal execution function, it should not be called outside of Execute() above.
*/
ErrorLevel CProcs::ExecuteInternal(int iIndex, CParams *pParams, CParams *pReturn)
{
	PROCITEM *pProc = &this->Collection.Items[iIndex];
	CEngine *pEngine = ((CEngine *)this->pEngine);

	if (pProc->ReqParams)
	{
		if (!pParams && pProc->ReqParams->Collection.Count != 0)
		{
			return this->Error->Hard("Wrong number of parameters passed to [%s]", pProc->Name);
		}
		if (pParams)
		{
			//Param count check.
			if (pParams->Collection.Count != pProc->ReqParams->Collection.Count)
			{
				return this->Error->Hard("Wrong number of parameters passed to [%s]", pProc->Name);
			}
		}

		for (int i = 0; i < pProc->ReqParams->Collection.Count; i++)
		{
			if (pEngine->Types->GetPointer(pProc->ReqParams->Collection.Items[i].Type)->IsComplex)
			{
				VARIABLEITEM *pVar = (VARIABLEITEM *)pParams->ToVarIndexPointer(i);
				if (pVar)
				{
					if (pVar->Type != pProc->ReqParams->Collection.Items[i].Type)
					{
						return this->Error->Hard("%s [%s] of function [%s] is of the incorrect type",
							(pProc->ReqParams->Collection.Items[i].ByReference ? "Reference parameter" : "Parameter"),
							pProc->ReqParams->Collection.Items[i].Name, pProc->Name);
					}
				}
				else {
					PARAMITEM *pParam = pParams->GetPointer(i);

					if (pParam->Value && pParam->Length != 0) //If Not NULL
					{
						if (pParams->ToInt(i) == INVALID_INDEX && pProc->ReqParams->Collection.Items[i].IsOptional)
						{
							//Optional parameter passes "invalid index". This is ok.
						}
						else {
							return this->Error->Hard("%s [%s] of function [%s] is of the incorrect type",
								(pProc->ReqParams->Collection.Items[i].ByReference ? "Reference parameter" : "Parameter"),
								pProc->ReqParams->Collection.Items[i].Name, pProc->Name);
						}
					}
				}
			}
			else if (pProc->ReqParams->Collection.Items[i].Type == VAR_TYPE_NUMERIC)
			{
				PARAMITEM *pParam = pParams->GetPointer(i);
				if (pParam->Value && pParam->Length != 0) //Not NULL
				{
					if (/*strcmp(pProc->ReqParams->Collection.Items[i].Name, PARENT_INDEX_VAR_NAME) == 0 ||*/ pProc->ReqParams->Collection.Items[i].ByReference == true)
					{
						if (pParam->Value[0] != '@' || !IsNumeric(pParam->Value + 1, pParam->Length - 1))
						{
							return this->Error->Hard("%s [%s] of function [%s] is of the incorrect type",
								(pProc->ReqParams->Collection.Items[i].ByReference ? "Reference parameter" : "Parameter"),
								pProc->ReqParams->Collection.Items[i].Name, pProc->Name);
						}
					}
					else {
						if (!IsNumeric(pParam->Value, pParam->Length))
						{
							return this->Error->Hard("%s [%s] of function [%s] is of the incorrect type",
								(pProc->ReqParams->Collection.Items[i].ByReference ? "Reference parameter" : "Parameter"),
								pProc->ReqParams->Collection.Items[i].Name, pProc->Name);
						}
					}
				}
			}
		}
	}

	if (pProc->Type == PROC_TYPE_SYSTEM)
	{
		//*********************************************************************************************************************
		//*********************************************************************************************************************
		//*********************************************************************************************************************
		if (_strnicmp(pProc->Name, "math.", 5) == 0)
		{
			//-----------------------------------------------------------------------------------------------------------------
			if (_StrEqlI(pProc->Name, "Math.SmartRound"))
			{
				CMathParser parser;
				char sValue[255];

				int iLength = parser.SmartRound(pParams->ToDouble(1), sValue, sizeof(sValue));
				if(iLength <= 0)
				{
					return this->Error->Soft("Mathematical rounding error");
				}

				return pReturn->Add(sValue, iLength);
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Math.Round"))
			{
				char sValue[255];

				int iRoundedLength = Round(pParams->ToDouble(1), pParams->ToInt(2), sValue, sizeof(sValue));

				return pReturn->Add(sValue, iRoundedLength);
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Math.ParseIntegerExpression"))
			{
				CMathParser parser;

				PARAMITEM *pExpression = pParams->GetPointer(1);

				int iResult = 0;

				if(parser.Calculate(pExpression->Value, pExpression->Length, &iResult) != CMathParser::ResultOk)
				{
					CMathParser::MATHERRORINFO *pMathError = parser.LastError();

					if(pMathError != NULL && pMathError->Text != NULL)
					{
						return this->Error->Soft("An error occured parsing the mathematical expression: %s", pMathError->Text);
					}
					else return this->Error->Soft("An error occured parsing the mathematical expression");
				}

				return pReturn->Add(iResult);
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Math.ParseDoubleExpression"))
			{
				CMathParser parser;

				PARAMITEM *pExpression = pParams->GetPointer(1);

				double dResult = 0;

				if(parser.Calculate(pExpression->Value, pExpression->Length, &dResult) != CMathParser::ResultOk)
				{
					CMathParser::MATHERRORINFO *pMathError = parser.LastError();

					if(pMathError != NULL && pMathError->Text != NULL)
					{
						return this->Error->Soft("An error occured parsing the mathematical expression: %s", pMathError->Text);
					}
					else return this->Error->Soft("An error occured parsing the mathematical expression");
				}

				return pReturn->Add(dResult);
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Math.Random"))
			{
				PARAMITEM *pMin = pParams->GetPointer(1);
				if (pMin && pMin->Value)
				{
					PARAMITEM *pMax = pParams->GetPointer(2);

					int iMin = atol(pMin->Value);
					int iMax = 0;

					if (pMax && pMax->Value)
					{
						iMax = atol(pMax->Value);
					}
					else {
						iMax = 2097150;
					}

					return pReturn->Add(RangeValue(rand(), iMin, iMax));
				}
				return pReturn->Add(rand());
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Math.SetRandom"))
			{

				if (pParams->First()->Value)
				{
					srand((int)pParams->ToInt(1));
				}
				srand(GetTickCount());
				return ERROR_OK;
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Math.Cos"))
			{
				return pReturn->Add(cos(pParams->ToDouble(1)));
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Math.Sin"))
			{
				return pReturn->Add(sin(pParams->ToDouble(1)));
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Math.Tan"))
			{
				return pReturn->Add(tan(pParams->ToDouble(1)));
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Math.Atan"))
			{
				return pReturn->Add(atan(pParams->ToDouble(1)));
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Math.Abs"))
			{
				return pReturn->Add((double)abs((long long)pParams->ToDouble(1)));
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Math.Pow"))
			{
				return pReturn->Add(pow(pParams->ToDouble(1), pParams->ToDouble(2)));
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Math.DebugMode"))
			{
				ErrorLevel Result = pReturn->Add(pEngine->MathDebugMode);
				pEngine->MathDebugMode = pParams->ToBool(1);
				return Result;
			}
			//-----------------------------------------------------------------------------------------------------------------
		}
		//*********************************************************************************************************************
		//*********************************************************************************************************************
		//*********************************************************************************************************************
		else if (_strnicmp(pProc->Name, "numeric.", 8) == 0)
		{
			//-----------------------------------------------------------------------------------------------------------------
			if (_StrEqlI(pProc->Name, "Numeric.Equals"))
			{
				VARIABLEITEM *pBaseVariale = (VARIABLEITEM *)pParams->ToVarIndexPointer(0);
				return pReturn->Add(atof(pBaseVariale->Value) == pParams->ToDouble(1));
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Numeric.GreaterThan"))
			{
				VARIABLEITEM *pBaseVariale = (VARIABLEITEM *)pParams->ToVarIndexPointer(0);
				return pReturn->Add(atof(pBaseVariale->Value) > pParams->ToDouble(1));
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Numeric.LessThan"))
			{
				VARIABLEITEM *pBaseVariale = (VARIABLEITEM *)pParams->ToVarIndexPointer(0);
				return pReturn->Add(atof(pBaseVariale->Value) < pParams->ToDouble(1));
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Numeric.NotEquals"))
			{
				VARIABLEITEM *pBaseVariale = (VARIABLEITEM *)pParams->ToVarIndexPointer(0);
				return pReturn->Add(atof(pBaseVariale->Value) != pParams->ToDouble(1));
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Numeric.ToInteger"))
			{
				int iVarIndex = pParams->ToVarIndex(0);
				VARIABLEITEM *pVar = ((CEngine *)this->pEngine)->Vars->GetPointer(iVarIndex);
				return pReturn->Add(atol(pVar->Value));
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Numeric.ToDouble"))
			{
				int iVarIndex = pParams->ToVarIndex(0);
				VARIABLEITEM *pVar = ((CEngine *)this->pEngine)->Vars->GetPointer(iVarIndex);
				return pReturn->Add(atof(pVar->Value));
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Numeric.ToString"))
			{
				int iVarIndex = pParams->ToVarIndex(0);
				int iPrecision = (int)pParams->ToInt(1);
				VARIABLEITEM *pVar = ((CEngine *)this->pEngine)->Vars->GetPointer(iVarIndex);

				if (iPrecision < 0)
				{
					return pReturn->Add((const char *)pVar->Value, pVar->Length);
				}
				else {
					char sNumber[128];
					FormatDouble(sNumber, sizeof(sNumber), atof(pVar->Value), iPrecision);
					return pReturn->Add(sNumber);
				}
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Numeric.Length"))
			{
				int iVarIndex = pParams->ToVarIndex(0);
				VARIABLEITEM *pVar = ((CEngine *)this->pEngine)->Vars->GetPointer(iVarIndex);
				return pReturn->Add(pVar->Length);
			}
		}
		//*********************************************************************************************************************
		//*********************************************************************************************************************
		//*********************************************************************************************************************
		else if (_strnicmp(pProc->Name, "list.", 5) == 0)
		{
			//-----------------------------------------------------------------------------------------------------------------
			if (_StrEqlI(pProc->Name, "List.Add"))
			{
				VectorBinary *pVector = ((VectorBinary *)((VARIABLEITEM *)pParams->ToVarIndexPointer(0))->Value);
				PARAMITEM *pValue = pParams->GetPointer(1);
				pVector->Add(pValue->Value, pValue->Length);
				return ERROR_OK;
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "List.Count"))
			{
				VectorBinary *pVector = ((VectorBinary *)((VARIABLEITEM *)pParams->ToVarIndexPointer(0))->Value);
				return pReturn->Add(pVector->Count());
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "List.Allocated"))
			{
				VectorBinary *pVector = ((VectorBinary *)((VARIABLEITEM *)pParams->ToVarIndexPointer(0))->Value);
				return pReturn->Add(pVector->Allocated());
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "List.Clear"))
			{
				VectorBinary *pVector = ((VectorBinary *)((VARIABLEITEM *)pParams->ToVarIndexPointer(0))->Value);
				pVector->Clear();
				return ERROR_OK;
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "List.Compact"))
			{
				VectorBinary *pVector = ((VectorBinary *)((VARIABLEITEM *)pParams->ToVarIndexPointer(0))->Value);
				pVector->Compact();
				return ERROR_OK;
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "List.Contains"))
			{
				VectorBinary *pVector = ((VectorBinary *)((VARIABLEITEM *)pParams->ToVarIndexPointer(0))->Value);
				PARAMITEM *pValue = pParams->GetPointer(1);
				return pReturn->Add(pVector->Contiains(pValue->Value));
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "List.DeleteOne"))
			{
				VectorBinary *pVector = ((VectorBinary *)((VARIABLEITEM *)pParams->ToVarIndexPointer(0))->Value);
				PARAMITEM *pValue = pParams->GetPointer(1);
				return pReturn->Add(pVector->DeleteFirstValue(pValue->Value));
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "List.DeleteAll"))
			{
				VectorBinary *pVector = ((VectorBinary *)((VARIABLEITEM *)pParams->ToVarIndexPointer(0))->Value);
				PARAMITEM *pValue = pParams->GetPointer(1);
				return pReturn->Add(pVector->DeleteAllValues(pValue->Value));
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "List.DeleteAt"))
			{
				VectorBinary *pVector = ((VectorBinary *)((VARIABLEITEM *)pParams->ToVarIndexPointer(0))->Value);
				int vectorIndex = pParams->ToInt(1);
				return pReturn->Add(pVector->DeleteAtIndex(vectorIndex));
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "List.IndexOf"))
			{
				VectorBinary *pVector = ((VectorBinary *)((VARIABLEITEM *)pParams->ToVarIndexPointer(0))->Value);
				PARAMITEM *pValue = pParams->GetPointer(1);
				return pReturn->Add(pVector->IndexOfValue(pValue->Value));
			}
		}
		//*********************************************************************************************************************
		//*********************************************************************************************************************
		//*********************************************************************************************************************
		else if (_strnicmp(pProc->Name, "Bytes.", 6) == 0)
		{
			int iMemberIndex = 0;
			int iBaseVarIndex = pParams->ToObjectIndex(0, &iMemberIndex);
			VARIABLEITEM *pBaseVar = ((CEngine *)this->pEngine)->Vars->GetPointer(iBaseVarIndex);

			//-----------------------------------------------------------------------------------------------------------------
			if (_StrEqlI(pProc->Name, "Bytes.Offset"))
			{
				int iByteCount = pParams->ToInt(1);

				if (iByteCount < 0 || iByteCount >= pBaseVar->Alloc || pBaseVar->Value == NULL)
				{
					return this->Error->Hard("Array index is out of bounds");
				}

				return pReturn->Add(pBaseVar->Value + iByteCount, pBaseVar->Alloc - iByteCount);
			}
			//-----------------------------------------------------------------------------------------------------------------
			if (_StrEqlI(pProc->Name, "Bytes.Initialize"))
			{
				int iValue = pParams->ToInt(1);

				memset(pBaseVar->Value, iValue, pBaseVar->Alloc);

				return pReturn->Add(pBaseVar->Value != NULL);
			}
			//-----------------------------------------------------------------------------------------------------------------
			if (_StrEqlI(pProc->Name, "Bytes.Allocate"))
			{
				int iByteCount = pParams->ToInt(1);

				pBaseVar->Value = (char *)pMem->ReAllocate(pBaseVar->Value, 1, iByteCount);
				pBaseVar->Length = iByteCount;
				pBaseVar->Alloc = iByteCount;

				memset(pBaseVar->Value, 0, pBaseVar->Alloc);


				return pReturn->Add(pBaseVar->Value != NULL);
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Bytes.Free"))
			{
				if (pBaseVar->Value != NULL)
				{
					pMem->Free(pBaseVar->Value);
				}

				pBaseVar->Value = NULL;
				pBaseVar->Length = 0;
				pBaseVar->Alloc = 0;

				return this->Error->HardCount();
			}
			//-----------------------------------------------------------------------------------------------------------------

		}
		//*********************************************************************************************************************
		//*********************************************************************************************************************
		//*********************************************************************************************************************
		else if (_strnicmp(pProc->Name, "string.", 7) == 0)
		{
			int iMemberIndex = 0;
			int iBaseVarIndex = pParams->ToObjectIndex(0, &iMemberIndex);
			VARIABLEITEM *pBaseVar = ((CEngine *)this->pEngine)->Vars->GetPointer(iBaseVarIndex);

			//-----------------------------------------------------------------------------------------------------------------
			if (_StrEqlI(pProc->Name, "String.Equals"))
			{
				PARAMITEM *pParam = pParams->GetPointer(1);

				if (iMemberIndex >= 0)
				{
					TYPEITEM *pType = pEngine->Types->GetPointer(pBaseVar->Type);
					if (pType->IsStrictType)
					{
						const char *sBaseValue = pBaseVar->Value + pType->Members.Items[iMemberIndex].ByteOffset;

						if (pParams->ToBool(2))
						{
							return pReturn->Add(StrEql(sBaseValue, strlen(sBaseValue), pParam->Value, pParam->Length));
						}
						else {
							return pReturn->Add(StrEqlI(sBaseValue, strlen(sBaseValue), pParam->Value, pParam->Length));
						}
					}
					else {
						return this->Error->Hard("Not implemented");
					}
				}
				else {
					if (pParams->ToBool(2))
					{
						return pReturn->Add(StrEql(pBaseVar->Value, pBaseVar->Length, pParam->Value, pParam->Length));
					}
					else {
						return pReturn->Add(StrEqlI(pBaseVar->Value, pBaseVar->Length, pParam->Value, pParam->Length));
					}
				}
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "String.Trim"))
			{
				StringBuilder builder(pBaseVar->Value, pBaseVar->Length);

				int iLength = pParams->ToInt(1);
				if (iLength < 0)
				{
					iLength = builder.Length;
				}

				builder.Trim();
				return pReturn->Add(builder.Buffer, iLength);
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "String.GreaterThan"))
			{
				if (pParams->ToBool(2))
				{
					return pReturn->Add(strcmp(pBaseVar->Value, pParams->ToString(1)) > 0);
				}
				else {
					return pReturn->Add(_strcmpi(pBaseVar->Value, pParams->ToString(1)) > 0);
				}
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "String.LessThan"))
			{
				if (pParams->ToBool(2))
				{
					return pReturn->Add(strcmp(pBaseVar->Value, pParams->ToString(1)) < 0);
				}
				else {
					return pReturn->Add(_strcmpi(pBaseVar->Value, pParams->ToString(1)) < 0);
				}
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "String.NotEquals"))
			{
				PARAMITEM *pParam = pParams->GetPointer(1);
				if (pParams->ToBool(2))
				{
					return pReturn->Add(!StrEql(pBaseVar->Value, pBaseVar->Length, pParam->Value, pParam->Length));
				}
				else {
					return pReturn->Add(!StrEqlI(pBaseVar->Value, pBaseVar->Length, pParam->Value, pParam->Length));
				}
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "String.ToInteger"))
			{
				int iVarIndex = pParams->ToVarIndex(0);
				VARIABLEITEM *pVar = ((CEngine *)this->pEngine)->Vars->GetPointer(iVarIndex);
				return pReturn->Add(iFormattedValue(pVar->Value));
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "String.ToDouble"))
			{
				int iVarIndex = pParams->ToVarIndex(0);
				VARIABLEITEM *pVar = ((CEngine *)this->pEngine)->Vars->GetPointer(iVarIndex);
				return pReturn->Add(dFormattedValue(pVar->Value));
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "String.Length"))
			{
				if (iMemberIndex >= 0)
				{
					TYPEITEM *pType = pEngine->Types->GetPointer(pBaseVar->Type);
					if (pType->IsStrictType)
					{
						const char *sBaseValue = pBaseVar->Value + pType->Members.Items[iMemberIndex].ByteOffset;
						return pReturn->Add((unsigned int)strlen(sBaseValue));
					}
					else {
						return this->Error->Hard("Not implemented");
					}
				}
				else return pReturn->Add(pBaseVar->Length);
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "String.ToUpper"))
			{
				VARIABLEITEM *pString = (VARIABLEITEM *)pParams->ToVarIndexPointer(0);
				if (pReturn->Add(pString->Value, pString->Length) == ERROR_OK)
				{
					PARAMITEM *pItem = pReturn->First();
					UCase(pItem->Value, pItem->Length);
				}
				return this->Error->HardCount();
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "String.ToLower"))
			{
				VARIABLEITEM *pString = (VARIABLEITEM *)pParams->ToVarIndexPointer(0);
				if (pReturn->Add(pString->Value, pString->Length) == ERROR_OK)
				{
					PARAMITEM *pItem = pReturn->First();
					LCase(pItem->Value, pItem->Length);
				}
				return this->Error->HardCount();

			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "String.Append"))
			{
				VARIABLEITEM *pString = (VARIABLEITEM *)pParams->ToVarIndexPointer(0);
				PARAMITEM *pAppend = pParams->GetPointer(1);

				return pEngine->Vars->Append(pString, pAppend->Value, pAppend->Length);
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "String.SubString"))
			{
				VARIABLEITEM *pString = (VARIABLEITEM *)pParams->ToVarIndexPointer(0);

				int iStartIndex = atol(pParams->GetPointer(1)->Value);
				int iLength = 0;

				if (iStartIndex < 0)
				{
					return this->Error->Soft("SubString starting index must be equal or greater than zero");
				}

				if (iStartIndex > pString->Length)
				{
					return this->Error->Soft("SubString starting index must be equal or less than %d", pString->Length);
				}

				PARAMITEM *pLength = pParams->GetPointer(2);
				if (pLength->Value)
				{
					iLength = atol(pLength->Value);

					if (iLength > pString->Length - iStartIndex)
					{
						return this->Error->Soft("SubString length must be equal or less than %d", pString->Length - iStartIndex);
					}
				}
				else {
					iLength = pString->Length - iStartIndex;
				}

				if (iLength < 0)
				{
					return this->Error->Soft("SubString length index must be equal or greater than zero");
				}

				return pReturn->Add(pString->Value + iStartIndex, iLength);
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "String.Reverse"))
			{
				VARIABLEITEM *pString = (VARIABLEITEM *)pParams->ToVarIndexPointer(0);
				ReverseString(pString->Value, pString->Length);
				return pReturn->Add(pString->Value, pString->Length);
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "String.Free"))
			{
				int iVarIndex = pParams->ToVarIndex(0);
				return ((CEngine *)this->pEngine)->Vars->Free(iVarIndex);
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "String.Replace"))
			{
				VARIABLEITEM *pString = (VARIABLEITEM *)pParams->ToVarIndexPointer(0);
				PARAMITEM *pToReplace = pParams->GetPointer(1);
				PARAMITEM *pReplaceWith = pParams->GetPointer(2);

				StringBuilder str(pString->Value, pString->Length);

				str.Replace(pToReplace->Value, pToReplace->Length, pReplaceWith->Value, pReplaceWith->Length, pParams->ToBool(3));

				str.Terminate();

				return pReturn->Add(str.Buffer, str.Length);
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "String.IndexOf"))
			{
				PARAMITEM *pStringToFind = pParams->GetPointer(1);
				int iStartPosition = (int)pParams->ToInt(3);

				if (iMemberIndex >= 0)
				{
					TYPEITEM *pType = pEngine->Types->GetPointer(pBaseVar->Type);
					if (pType->IsStrictType)
					{
						const char *sBaseValue = pBaseVar->Value + pType->Members.Items[iMemberIndex].ByteOffset;

						if (pParams->ToBool(2))
						{
							return pReturn->Add(InStr(pStringToFind->Value, pStringToFind->Length, sBaseValue, strlen(sBaseValue), iStartPosition));
						}
						else {
							return pReturn->Add(InStrI(pStringToFind->Value, pStringToFind->Length, sBaseValue, strlen(sBaseValue), iStartPosition));
						}
					}
					else {
						return this->Error->Hard("Not implemented");
					}
				}
				else {
					if (pParams->ToBool(2))
					{
						return pReturn->Add(InStr(pStringToFind->Value, pStringToFind->Length, pBaseVar->Value, pBaseVar->Length, iStartPosition));
					}
					else {
						return pReturn->Add(InStrI(pStringToFind->Value, pStringToFind->Length, pBaseVar->Value, pBaseVar->Length, iStartPosition));
					}
				}
			}
		}
		//*********************************************************************************************************************
		//*********************************************************************************************************************
		//*********************************************************************************************************************
		else if (_strnicmp(pProc->Name, "web.", 4) == 0)
		{
			//-----------------------------------------------------------------------------------------------------------------
			if (_StrEqlI(pProc->Name, "Web.URLEncode"))
			{
				const char *sString = pParams->ToString(1);
				int iLength = (int)pParams->ToInt(2);
				if (iLength < 0)
				{
					iLength = (int)strlen(sString);
				}

				ErrorLevel Result;

				char *sEncoded = (char *) pMem->Allocate(sizeof(char), (iLength * 3 + 1));
				if (URLEncode(sString, iLength, sEncoded, (iLength * 3 + 1)) >= 0)
				{
					Result = pReturn->Add(sEncoded);
				}
				else {
					Result = this->Error->Hard("Insufficient memory allocated for URLEncode");

				}
				pMem->Free(sEncoded);

				return Result;
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Web.URLDecode"))
			{
				char *sString = pParams->ToString(1);

				int iLength = URLDecode(sString);

				return pReturn->Add(sString, iLength);
			}
			//-----------------------------------------------------------------------------------------------------------------
			//This function creates new variables out of any CGI form input.
			else if (_StrEqlI(pProc->Name, "Web.InitializeCGI"))
			{
				if (((CEngine *)this->pEngine)->InitializeCGIEnvironment() != ERROR_OK)
				{
					return this->Error->HardCount();
				}
				if (((CEngine *)this->pEngine)->InitializeCGIFormVariables() != ERROR_OK)
				{
					return this->Error->HardCount();
				}
				return ERROR_OK;
			}
		}
		//*********************************************************************************************************************
		//*********************************************************************************************************************
		//*********************************************************************************************************************
		else if (_strnicmp(pProc->Name, "form.", 5) == 0)
		{
			//-----------------------------------------------------------------------------------------------------------------
			if (_StrEqlI(pProc->Name, "Form.Value.Count"))
			{
				int iVarCount = 0;

				for (int i = 0; i < pEngine->Vars->Count(); i++)
				{
					VARIABLEITEM *pVar = pEngine->Vars->GetPointer(i);
					if (pVar)
					{
						if (_strnicmp(pVar->Name, "Form.", 5) == 0)
						{
							iVarCount++;
						}
					}
				}
				return pReturn->Add(iVarCount);
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Form.Value.Define"))
			{
				char sFieldName[MAX_VAR_NAME_SIZE];
				sprintf_s(sFieldName, sizeof(sFieldName), "Form.%s", pParams->ToString(1));
				if (pEngine->Vars->AddAndSet(sFieldName, pParams->ToString(2)) == ERROR_OK)
				{
					pEngine->Vars->GetPointer(sFieldName)->Scope = 0;
					return ERROR_OK;
				}
				else return this->Error->HardCount();
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Form.Value.IsDefined"))
			{
				char sFieldName[MAX_VAR_NAME_SIZE];
				sprintf_s(sFieldName, sizeof(sFieldName), "Form.%s", pParams->ToString(1));
				return pReturn->Add(pEngine->Vars->IsDefined(sFieldName));
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Form.Value.ByIndex"))
			{
				int iFieldIndex = (int)pParams->ToInt(1);
				int iVarCount = 0;

				for (int i = 0; i < pEngine->Vars->Count(); i++)
				{
					VARIABLEITEM *pVar = pEngine->Vars->GetPointer(i);
					if (pVar)
					{
						if (_strnicmp(pVar->Name, "Form.", 5) == 0)
						{
							if (iVarCount == iFieldIndex)
							{
								return pReturn->Add(pVar->Value, pVar->Length);
							}
							iVarCount++;
						}
					}
				}

				return pReturn->Add("");
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Form.Value.Index"))
			{
				char sFieldName[MAX_VAR_NAME_SIZE];
				sprintf_s(sFieldName, sizeof(sFieldName), "Form.%s", pParams->ToString(1));

				int iVarCount = 0;

				for (int i = 0; i < pEngine->Vars->Count(); i++)
				{
					VARIABLEITEM *pVar = pEngine->Vars->GetPointer(i);
					if (pVar)
					{
						if (_strnicmp(pVar->Name, "Form.", 5) == 0)
						{
							if (_StrEqlI(pVar->Name, sFieldName))
							{
								return pReturn->Add(iVarCount);
							}
							iVarCount++;
						}

					}
				}

				return pReturn->Add(-1);
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Form.Value.ByName"))
			{
				char sFieldName[MAX_VAR_NAME_SIZE];
				sprintf_s(sFieldName, sizeof(sFieldName), "Form.%s", pParams->ToString(1));

				for (int i = 0; i < pEngine->Vars->Count(); i++)
				{
					VARIABLEITEM *pVar = pEngine->Vars->GetPointer(i);
					if (pVar)
					{
						if (_StrEqlI(pVar->Name, sFieldName))
						{
							return pReturn->Add(pVar->Value, pVar->Length);
						}
					}
				}

				return pReturn->Add("");
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Form.Value.Name"))
			{
				int iFieldIndex = (int)pParams->ToInt(1);

				int iVarCount = 0;

				for (int i = 0; i < pEngine->Vars->Count(); i++)
				{
					VARIABLEITEM *pVar = pEngine->Vars->GetPointer(i);
					if (pVar)
					{
						if (_strnicmp(pVar->Name, "Form.", 5) == 0)
						{
							if (iVarCount == iFieldIndex)
							{
								return pReturn->Add(pVar->Name + 5);
							}
							iVarCount++;
						}
					}
				}

				return pReturn->Add("");
			}
			//-----------------------------------------------------------------------------------------------------------------
		}
		//*********************************************************************************************************************
		//*********************************************************************************************************************
		//*********************************************************************************************************************
		else if (_strnicmp(pProc->Name, "Query.", 6) == 0)
		{
			//-----------------------------------------------------------------------------------------------------------------
			if (_StrEqlI(pProc->Name, "Query.Value.Count"))
			{
				int iVarCount = 0;

				for (int i = 0; i < pEngine->Vars->Count(); i++)
				{
					VARIABLEITEM *pVar = pEngine->Vars->GetPointer(i);
					if (pVar)
					{
						if (_strnicmp(pVar->Name, "Query.", 6) == 0)
						{
							iVarCount++;
						}
					}
				}
				return pReturn->Add(iVarCount);
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Query.Value.Define"))
			{
				char sFieldName[MAX_VAR_NAME_SIZE];
				sprintf_s(sFieldName, sizeof(sFieldName), "Query.%s", pParams->ToString(1));
				if (pEngine->Vars->AddAndSet(sFieldName, pParams->ToString(2)) == ERROR_OK)
				{
					pEngine->Vars->GetPointer(sFieldName)->Scope = 0;
					return ERROR_OK;
				}
				return this->Error->HardCount();
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Query.Value.IsDefined"))
			{
				char sFieldName[MAX_VAR_NAME_SIZE];
				sprintf_s(sFieldName, sizeof(sFieldName), "Query.%s", pParams->ToString(1));
				return pReturn->Add(pEngine->Vars->IsDefined(sFieldName));
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Query.Value.ByIndex"))
			{
				int iFieldIndex = (int)pParams->ToInt(1);
				int iVarCount = 0;

				for (int i = 0; i < pEngine->Vars->Count(); i++)
				{
					VARIABLEITEM *pVar = pEngine->Vars->GetPointer(i);
					if (pVar)
					{
						if (_strnicmp(pVar->Name, "Query.", 6) == 0)
						{
							if (iVarCount == iFieldIndex)
							{
								return pReturn->Add(pVar->Value, pVar->Length);
							}
							iVarCount++;
						}
					}
				}

				return pReturn->Add("");
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Query.Value.Index"))
			{
				char sFieldName[MAX_VAR_NAME_SIZE];
				sprintf_s(sFieldName, sizeof(sFieldName), "Query.%s", pParams->ToString(1));

				int iVarCount = 0;

				for (int i = 0; i < pEngine->Vars->Count(); i++)
				{
					VARIABLEITEM *pVar = pEngine->Vars->GetPointer(i);
					if (pVar)
					{
						if (_strnicmp(pVar->Name, "Query.", 6) == 0)
						{
							if (_StrEqlI(pVar->Name, sFieldName))
							{
								return pReturn->Add(iVarCount);
							}
							iVarCount++;
						}

					}
				}

				return pReturn->Add(-1);
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Query.Value.ByName"))
			{
				char sFieldName[MAX_VAR_NAME_SIZE];
				sprintf_s(sFieldName, sizeof(sFieldName), "Query.%s", pParams->ToString(1));

				for (int i = 0; i < pEngine->Vars->Count(); i++)
				{
					VARIABLEITEM *pVar = pEngine->Vars->GetPointer(i);
					if (pVar)
					{
						if (_StrEqlI(pVar->Name, sFieldName))
						{
							return pReturn->Add(pVar->Value, pVar->Length);
						}
					}
				}

				return pReturn->Add("");
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Query.Value.Name"))
			{
				int iFieldIndex = (int)pParams->ToInt(1);

				int iVarCount = 0;

				for (int i = 0; i < pEngine->Vars->Count(); i++)
				{
					VARIABLEITEM *pVar = pEngine->Vars->GetPointer(i);
					if (pVar)
					{
						if (_strnicmp(pVar->Name, "Query.", 6) == 0)
						{
							if (iVarCount == iFieldIndex)
							{
								return pReturn->Add(pVar->Name + 6);
							}
							iVarCount++;
						}
					}
				}

				return pReturn->Add("");
			}
			//-----------------------------------------------------------------------------------------------------------------
		}
		//*********************************************************************************************************************
		//*********************************************************************************************************************
		//*********************************************************************************************************************
		else if (_strnicmp(pProc->Name, "Convert.", 4) == 0)
		{
			//-----------------------------------------------------------------------------------------------------------------
			if (_StrEqlI(pProc->Name, "Convert.LengthFromPointer"))
			{
				size_t pPointer = (int)pParams->ToIntPtr(1);

				return pReturn->Add((unsigned int)strlen((char *)pPointer));
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Convert.StringFromPointer"))
			{
				size_t pPointer = (int)pParams->ToIntPtr(1);

				return pReturn->Add((char *)pPointer);
			}
			//-----------------------------------------------------------------------------------------------------------------
		}


		//*********************************************************************************************************************
		//*********************************************************************************************************************
		//*********************************************************************************************************************
		else if (_strnicmp(pProc->Name, "env.", 4) == 0)
		{
			//-----------------------------------------------------------------------------------------------------------------
			if (_StrEqlI(pProc->Name, "Env.CommandLine.Count"))
			{
				return pReturn->Add((unsigned int)pEngine->CommandLine->StackSize());
			}
			//-----------------------------------------------------------------------------------------------------------------
			if (_StrEqlI(pProc->Name, "Env.CommandLine.Get"))
			{
				int iIndex = pParams->ToInt(1);

				_STACKITEM *pItem = pEngine->CommandLine->Peek(iIndex);

				if (pItem) {
					return pReturn->Add((const char *)pItem->Value, pItem->Size);
				}

				return this->Error->Soft("Invalid command line index");
			}
			//-----------------------------------------------------------------------------------------------------------------
			if (_StrEqlI(pProc->Name, "Env.Get"))
			{
				char *envValue = getenv(pParams->ToString(1));
				if (envValue)
				{
					return pReturn->Add(envValue);
				}
				else {
					return pReturn->Add("");
				}
			}
			//-----------------------------------------------------------------------------------------------------------------
			if (_StrEqlI(pProc->Name, "Env.Index"))
			{
				char *sRequestedName = pParams->ToString(1);
				int iIndex = 0;
				char *next = GetEnvironmentStrings();

				while (*next)
				{
					int iLength = strlen(next);
					for (int iEqual = 0; iEqual < iLength; iEqual++)
					{
						if (next[iEqual] == '=')
						{
							if (iEqual > 0)
							{
								next[iEqual] = '\0';

								if (_StrEqlI(next, sRequestedName))
								{
									return pReturn->Add(iIndex);
								}
								iIndex++;
							}

							break;
						}
					}

					while (*next)
					{
						next++; // next points to the null at the end.
					}

					next++; // advance one more to get to the next string.
				}

				return pReturn->Add("");
			}
			//-----------------------------------------------------------------------------------------------------------------
			if (_StrEqlI(pProc->Name, "Env.Name"))
			{
				int iRequestedIndex = (int)pParams->ToInt(1);
				int iIndex = 0;
				char *next = GetEnvironmentStrings();

				while (*next)
				{
					int iLength = strlen(next);
					for (int iEqual = 0; iEqual < iLength; iEqual++)
					{
						if (next[iEqual] == '=')
						{
							if (iEqual > 0)
							{
								if (iIndex == iRequestedIndex)
								{
									next[iEqual] = '\0';
									return pReturn->Add(next, iEqual);
								}
								iIndex++;
							}

							break;
						}
					}

					while (*next)
					{
						next++; // next points to the null at the end.
					}

					next++; // advance one more to get to the next string.
				}

				return pReturn->Add("");
			}
			//-----------------------------------------------------------------------------------------------------------------
			if (_StrEqlI(pProc->Name, "Env.Value"))
			{
				int iRequestedIndex = (int)pParams->ToInt(1);
				int iIndex = 0;
				char *next = GetEnvironmentStrings();

				while (*next)
				{
					int iLength = strlen(next);
					for (int iEqual = 0; iEqual < iLength; iEqual++)
					{
						if (next[iEqual] == '=')
						{
							if (iEqual > 0)
							{
								if (iIndex == iRequestedIndex)
								{
									next[iEqual] = '\0';
									return pReturn->Add(next + iEqual + 1, iLength - (iEqual + 1));
								}
								iIndex++;
							}

							break;
						}
					}

					while (*next)
					{
						next++; // next points to the null at the end.
					}

					next++; // advance one more to get to the next string.
				}

				return pReturn->Add("");
			}
			//-----------------------------------------------------------------------------------------------------------------
			if (_StrEqlI(pProc->Name, "Env.Count"))
			{
				int iIndex = 0;
				char *next = GetEnvironmentStrings();

				while (*next)
				{
					int iLength = strlen(next);
					for (int iEqual = 0; iEqual < iLength; iEqual++)
					{
						if (next[iEqual] == '=')
						{
							if (iEqual > 0)
							{
								iIndex++;
							}

							break;
						}
					}

					while (*next)
					{
						next++; // next points to the null at the end.
					}

					next++; // advance one more to get to the next string.
				}

				return pReturn->Add(iIndex);
			}
			//-----------------------------------------------------------------------------------------------------------------
			if (_StrEqlI(pProc->Name, "Env.Set"))
			{
				_putenv_s(pParams->ToString(1), pParams->ToString(2));
				return this->Error->HardCount();
			}
			//-----------------------------------------------------------------------------------------------------------------
		}
		//*********************************************************************************************************************
		//*********************************************************************************************************************
		//*********************************************************************************************************************
		else if (_strnicmp(pProc->Name, "Sys.", 4) == 0)
		{
			//-----------------------------------------------------------------------------------------------------------------
			if (_StrEqlI(pProc->Name, "Sys.MachineName"))
			{
				char sTemp[256];
				if (!Get_MachineName(sTemp, sizeof(sTemp)))
				{
					this->Error->Soft("Failed to obtain Machine Name");
				}
				return pReturn->Add(sTemp);
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Sys.OSVersion"))
			{
				char sTemp[256];
				if (!Get_OsVersion(sTemp, sizeof(sTemp)))
				{
					this->Error->Soft("Failed to obtain OS version");
				}
				return pReturn->Add(sTemp);
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Sys.TickCount"))
			{
				return pReturn->Add((unsigned int)GetTickCount());
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Sys.Memory.Load"))
			{
				MEMORYSTATUSEX ms;
				memset(&ms, 0, sizeof(ms));
				ms.dwLength = sizeof(ms);
				GlobalMemoryStatusEx(&ms);
				return pReturn->Add((double)ms.dwMemoryLoad);
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Sys.Memory.TotalPhysical"))
			{
				MEMORYSTATUSEX ms;
				memset(&ms, 0, sizeof(ms));
				ms.dwLength = sizeof(ms);
				GlobalMemoryStatusEx(&ms);
				return pReturn->Add((double)ms.ullTotalPhys);
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Sys.Memory.AvailablePhysical"))
			{
				MEMORYSTATUSEX ms;
				memset(&ms, 0, sizeof(ms));
				ms.dwLength = sizeof(ms);
				GlobalMemoryStatusEx(&ms);
				return pReturn->Add((double)ms.ullAvailPhys);
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Sys.Memory.TotalVirtual"))
			{
				MEMORYSTATUSEX ms;
				memset(&ms, 0, sizeof(ms));
				ms.dwLength = sizeof(ms);
				GlobalMemoryStatusEx(&ms);
				return pReturn->Add((double)ms.ullTotalVirtual);
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Sys.Memory.AvailableVirtual"))
			{
				MEMORYSTATUSEX ms;
				memset(&ms, 0, sizeof(ms));
				ms.dwLength = sizeof(ms);
				GlobalMemoryStatusEx(&ms);
				return pReturn->Add((double)ms.ullAvailVirtual);
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Sys.Memory.TotalPageFile"))
			{
				MEMORYSTATUSEX ms;
				memset(&ms, 0, sizeof(ms));
				ms.dwLength = sizeof(ms);
				GlobalMemoryStatusEx(&ms);
				return pReturn->Add((double)ms.ullTotalPageFile);
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Sys.Memory.AvailablePageFile"))
			{
				MEMORYSTATUSEX ms;
				memset(&ms, 0, sizeof(ms));
				ms.dwLength = sizeof(ms);
				GlobalMemoryStatusEx(&ms);
				return pReturn->Add((double)ms.ullAvailPageFile);
			}
		}
		//*********************************************************************************************************************
		//*********************************************************************************************************************
		//*********************************************************************************************************************
		else if (_strnicmp(pProc->Name, "Process.", 8) == 0)
		{
			if (_StrEqlI(pProc->Name, "Process.ModuleName"))
			{
				char sTemp[MAX_PATH];
				if (GetModuleFileName(NULL, sTemp, sizeof(sTemp)) == 0)
				{
					this->Error->Soft("Failed to obtain the module file name");
				}
				return pReturn->Add(sTemp);
			}
			else if (_StrEqlI(pProc->Name, "Process.Username"))
			{
				char sTemp[256];
				if (!Get_UserName(sTemp, sizeof(sTemp)))
				{
					this->Error->Soft("Failed to obtain Username");
				}
				return pReturn->Add(sTemp);
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Process.Id"))
			{
				return pReturn->Add((unsigned int)GetCurrentProcessId());
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Process.PageFaultCount"))
			{
				HANDLE hProcess = GetCurrentProcess();
				PROCESS_MEMORY_COUNTERS pmi;

				memset(&pmi, 0, sizeof(pmi));
				if (GetProcessMemoryInfo(hProcess, &pmi, sizeof(pmi)))
				{
					return pReturn->Add((unsigned int)pmi.PageFaultCount);
				}
				return this->Error->Hard("Failed to query process memory info");
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Process.PagefileUsage"))
			{
				HANDLE hProcess = GetCurrentProcess();
				PROCESS_MEMORY_COUNTERS pmi;

				memset(&pmi, 0, sizeof(pmi));
				if (GetProcessMemoryInfo(hProcess, &pmi, sizeof(pmi)))
				{
					return pReturn->Add((unsigned int)pmi.PagefileUsage);
				}
				return this->Error->Hard("Failed to query process memory info");
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Process.PeakPagefileUsage"))
			{
				HANDLE hProcess = GetCurrentProcess();
				PROCESS_MEMORY_COUNTERS pmi;

				memset(&pmi, 0, sizeof(pmi));
				if (GetProcessMemoryInfo(hProcess, &pmi, sizeof(pmi)))
				{
					return pReturn->Add((unsigned int)pmi.PeakPagefileUsage);
				}
				return this->Error->Hard("Failed to query process memory info");
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Process.PeakWorkingSetSize"))
			{
				HANDLE hProcess = GetCurrentProcess();
				PROCESS_MEMORY_COUNTERS pmi;

				memset(&pmi, 0, sizeof(pmi));
				if (GetProcessMemoryInfo(hProcess, &pmi, sizeof(pmi)))
				{
					return pReturn->Add((unsigned int)pmi.PeakWorkingSetSize);
				}
				return this->Error->Hard("Failed to query process memory info");
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Process.WorkingSetSize"))
			{
				HANDLE hProcess = GetCurrentProcess();
				PROCESS_MEMORY_COUNTERS pmi;

				memset(&pmi, 0, sizeof(pmi));
				if (GetProcessMemoryInfo(hProcess, &pmi, sizeof(pmi)))
				{
					return pReturn->Add((unsigned int)pmi.WorkingSetSize);
				}
				return this->Error->Hard("Failed to query process memory info");
			}

			//-----------------------------------------------------------------------------------------------------------------
		}
		//*********************************************************************************************************************
		//*********************************************************************************************************************
		//*********************************************************************************************************************
		else if (_strnicmp(pProc->Name, "Debug.", 6) == 0)
		{
			//-----------------------------------------------------------------------------------------------------------------
			if (_StrEqlI(pProc->Name, "Debug.Dump.Scripts"))
			{
				char *sOutputFile = pParams->ToString(0);

				for(int i = 0; i < pEngine->Scripts->Collection.Count; i++)
				{
					SCRIPTITEM *sScript = &pEngine->Scripts->Collection.Items[i];
					printf("[%d]: Namespace: [%s], Filename: [%s]\n",
							i,
							sScript->Namespace,
							sScript->FileName);
				}

				return this->Error->HardCount();
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Debug.Dump.Procs"))
			{
				char *sOutputFile = pParams->ToString(0);

				for(int i = 0; i < pEngine->Procs->Count(); i++)
				{
					PROCITEM *pProc = pEngine->Procs->GetPointer(i);
					printf("[%d]: Type: %d, ReturnType: %d, Params: %d, LineBegin: %d, AssessedReturnType: %d, Library: %s, Alias: [%s], Name: [%s]\n",
							i,
							pProc->Type,
							pProc->ReturnType,
							pProc->ReqParams != NULL ? pProc->ReqParams->Collection.Count : 0,
							pProc->LineBegin,
							pProc->AssessedReturnType,
							pProc->Library,
							pProc->Alias,
							pProc->Name);
				}

				return this->Error->HardCount();
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Debug.Dump.Vars"))
			{
				char *sOutputFile = pParams->ToString(0);

				for(int i = 0; i < pEngine->Vars->Count(); i++)
				{
					VARIABLEITEM *pVar = pEngine->Vars->GetPointer(i);
					if (pVar == NULL)
					{
						printf("[%d]: NULL\r\n", i);
					}
					else
					{
						printf("[%d]: Scope: %d, Type: %d, Reserved: %d, Length: %d, Child: %d, Parent: %d, Name: [%s]\n",
							pVar->Index,
							pVar->Scope,
							pVar->Type,
							pVar->Reserved,
							pVar->Length,
							pVar->ChildVariableIndex,
							pVar->ParentVariableIndex,
							pVar->Name);
					}
				}

				return this->Error->HardCount();
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Debug.Dump.Lines"))
			{
				char *sOutputFile = pParams->ToString(0);

				for(int i = 0; i < pEngine->Lines->Collection.Count; i++)
				{
					printf("Code: %d, Inline,: %d, Text: [%s]\n",
							pEngine->Lines->Collection.Items[i].CodeBlockLevel,
							pEngine->Lines->Collection.Items[i].IsInline,
							pEngine->Lines->Collection.Items[i].Text);
				}

				return this->Error->HardCount();
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Debug.Dump.Memory"))
			{
				char *sOutputFile = pParams->ToString(0);


				printf("Slots:\n");
				printf("    Allocated: %d\n", pMem->Items.iAllocated);
				printf("    Used: %d\n", pMem->Items.iUsedCount);
				printf("    Increment: %d\n", pMem->Items.iAllocIncrement);
#ifdef _DEBUG
				printf("    Reserved Bytes: %d\n", pMem->Items.iReservedBytes);
#endif

				printf("Map:\n");
				for(size_t i = 0; i < pMem->Items.iAllocated; i++)
				{
					if(pMem->Items.Array[i].iAddress != 0)
					{
#ifdef _DEBUG
						printf("    Slot[%d] { address: 0x%x, bytes: %d, file: %s, line: %d\n",
							i,
							pMem->Items.Array[i].iAddress,
							pMem->Items.Array[i].iSize,
							pMem->Items.Array[i].sCodeFile,
							pMem->Items.Array[i].iLineOfCode);
#else
						printf("    Slot[%d] { address: 0x%x\n",
							i,
							pMem->Items.Array[i].iAddress);
#endif
					}
					else {
						printf("    Slot[%d], {free. 0 bytes.}\n", i);
					}
				}

				return this->Error->HardCount();
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Debug.IsAttached"))
			{
				return pReturn->Add(pEngine->Debug && pEngine->Debug->IsDebuggerAttached);
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Debug.Evaluate"))
			{
				if (pEngine->Debug && pEngine->Debug->IsDebuggerAttached)
				{
					int iPreviousMinimumScope = pEngine->State->MinimumScope;
					pEngine->State->MinimumScope = 0;

					char *sCodeText = pParams->ToString(1);
					VARIABLEITEM *pVar = pEngine->Vars->GetPointer(sCodeText);
					if (pVar)
					{
						int iFriendlyLength = pVar->Length > 100 ? 100 : pVar->Length;
						char sFriendlyText[101];
						memcpy(sFriendlyText, pVar->Value, iFriendlyLength);
						sFriendlyText[iFriendlyLength] = '\0';

						pEngine->Debug->SendSymbolInfo(sCodeText, pEngine->Types->Name(pVar->Type), sFriendlyText);
					}
					else {
						this->Error->AbsorbExceptions(true);
						CParams *pParam = NULL;
						pEngine->Evaluate(sCodeText, (int)strlen(sCodeText), &pParam);
						this->Error->AbsorbExceptions(false);
						if (pParam)
						{
							if (pParam->Collection.Count == 1)
							{
								pEngine->Debug->SendSymbolInfo(sCodeText, "<dynamic>", pParam->ToString(0));
							}
							delete pParam;
						}
					}

					pEngine->State->MinimumScope = iPreviousMinimumScope;
				}

				return this->Error->HardCount();
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Debug.Break"))
			{
				if (pEngine->Debug && pEngine->Debug->IsDebuggerAttached)
				{
					if (pParams->ToInt(1) != 0)
					{
						pEngine->Debug->Break();
					}
				}
				return this->Error->HardCount();
			}
			//-----------------------------------------------------------------------------------------------------------------
		}
		//*********************************************************************************************************************
		//*********************************************************************************************************************
		//*********************************************************************************************************************
		else if (_strnicmp(pProc->Name, "settings.", 9) == 0)
		{
			//-----------------------------------------------------------------------------------------------------------------
			if (_StrEqlI(pProc->Name, "Settings.ThrowSoftExceptions"))
			{
				ErrorLevel Result = pReturn->Add(pEngine->ThrowSoftExceptions);
				pEngine->ThrowSoftExceptions = pParams->ToBool(1);
				return Result;
			}
			//-----------------------------------------------------------------------------------------------------------------
		}
		//*********************************************************************************************************************
		//*********************************************************************************************************************
		//*********************************************************************************************************************
		else if (_strnicmp(pProc->Name, "exception.", 10) == 0)
		{
			//-----------------------------------------------------------------------------------------------------------------
			if (_StrEqlI(pProc->Name, "Exception.GetText"))
			{
				VARIABLEITEM *pVar = (VARIABLEITEM *)pParams->ToVarIndexPointer(0);

				if (pVar)
				{
					EXCEPTIONITEM *pExcept = ((EXCEPTIONITEM *)pVar->Value);
					if (pExcept)
					{
						return pReturn->Add(pExcept->Message);
					}
				}
				return this->Error->Hard("The exception is empty");
			}
			//-----------------------------------------------------------------------------------------------------------------
			if (_StrEqlI(pProc->Name, "Exception.SetText"))
			{
				VARIABLEITEM *pVar = (VARIABLEITEM *)pParams->ToVarIndexPointer(0);
				PARAMITEM *paramItem = pParams->Second();

				if (pVar)
				{
					EXCEPTIONITEM *pExcept = ((EXCEPTIONITEM *)pVar->Value);
					if (!pExcept)
					{
						pExcept = (EXCEPTIONITEM *) pMem->Allocate(sizeof(EXCEPTIONITEM), 1);
						pVar->Length = sizeof(EXCEPTIONITEM);
					}

					if (pExcept->Message)
					{
						pMem->Free(pExcept->Message);
					}

					pExcept->Message = pMem->CloneStringN(paramItem->Value, paramItem->Length);

					pVar->Value = (char *)pExcept;
					return this->Error->HardCount();
				}
				return this->Error->Hard("The exception is empty");
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Exception.GetFile"))
			{
				VARIABLEITEM *pVar = (VARIABLEITEM *)pParams->ToVarIndexPointer(0);

				if (pVar)
				{
					EXCEPTIONITEM *pExcept = ((EXCEPTIONITEM *)pVar->Value);
					if (pExcept)
					{
						if (pEngine->Lines)
						{
							LINEITEM *pLine = pEngine->Lines->GetPointer(pExcept->LineIndex);
							if (pLine && pEngine->Scripts)
							{
								SCRIPTITEM *pScript = pEngine->Scripts->GetPointer(pLine->ScriptIndex);

								if (pScript)
								{
									return pReturn->Add(pScript->FileName);
								}
							}
						}
						return pReturn->Add("");
					}
				}
				return this->Error->Hard("The exception is empty");
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Exception.GetLine"))
			{
				VARIABLEITEM *pVar = (VARIABLEITEM *)pParams->ToVarIndexPointer(0);

				if (pVar)
				{
					EXCEPTIONITEM *pExcept = ((EXCEPTIONITEM *)pVar->Value);
					if (pExcept)
					{
						if (pEngine->Lines)
						{
							LINEITEM *pLine = pEngine->Lines->GetPointer(pExcept->LineIndex);
							if (pLine)
							{
								return pReturn->Add(pLine->FileLine);
							}
						}

						return pReturn->Add((int)0);
					}
				}
				return this->Error->Hard("The exception is empty");
			}

			//-----------------------------------------------------------------------------------------------------------------
		}
		//*********************************************************************************************************************
		//*********************************************************************************************************************
		//*********************************************************************************************************************
		else if (_strnicmp(pProc->Name, "error.", 6) == 0)
		{
			//-----------------------------------------------------------------------------------------------------------------
			if (_StrEqlI(pProc->Name, "Error.Text"))
			{
				return pReturn->Add(this->Error->Text((int)pParams->ToInt(1)));
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Error.File"))
			{
				return pReturn->Add(this->Error->File((int)pParams->ToInt(1)));
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Error.Line"))
			{
				return pReturn->Add(this->Error->Line((int)pParams->ToInt(1)));
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Error.Throw"))
			{
				return this->Error->Soft(pParams->ToString(1));
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Error.Count"))
			{
				return pReturn->Add(this->Error->SoftCount());
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Error.UnWind"))
			{
				return pReturn->Add(this->Error->Throw(true));
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Error.Clear"))
			{
				return pReturn->Add(this->Error->Clear());
			}
			//-----------------------------------------------------------------------------------------------------------------
		}
		//*********************************************************************************************************************
		//*********************************************************************************************************************
		//*********************************************************************************************************************
		else if (_strnicmp(pProc->Name, "code.", 5) == 0)
		{
			//-----------------------------------------------------------------------------------------------------------------
			if (_StrEqlI(pProc->Name, "Code.Inject"))
			{
				//pParams->Collection.Items[0].Value; //VariableIndex
				PARAMITEM *pParam = &pParams->Collection.Items[1];
				CEngine *pEngine = ((CEngine *)this->pEngine);
				int iLineIndex = pEngine->State->LineIndex;

				char **sLines = Split(pParam->Value, pParam->Length, "\n");

				while (*sLines)
				{
					int iLength = CleanWhiteSpaces(*sLines);

					if (pEngine->Lines->Add("", 0, *sLines, iLength,
						pEngine->Lines->GetPointer(iLineIndex)->ScriptIndex,
						pEngine->Lines->GetPointer(iLineIndex)->FileLine - 1, 1, false, false) != ERROR_OK)
					{
						break;
					}

					*sLines++;
				}

				return pReturn->Error->HardCount();
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Code.Append"))
			{
				char sFilename[MAX_PATH];

				strcpy_s(sFilename, sizeof(sFilename), pParams->ToString(1));
				CorrectReversePath(sFilename, sizeof(sFilename));
				if (CharIndex(sFilename, '\\') < 0)
				{
					char sCurrentScriptPath[MAX_PATH];

					LINEITEM *pLine = &pEngine->Lines->Collection.Items[pEngine->State->LineIndex];
					SCRIPTITEM *pScript = &pEngine->Scripts->Collection.Items[pLine->ScriptIndex];

					if (!GetFilePath(pScript->FileName, sCurrentScriptPath, sizeof(sCurrentScriptPath)))
					{
						return this->Error->Hard("Failed to parse current script file path");
					}

					AppendPaths(sFilename, sizeof(sFilename), sCurrentScriptPath, pParams->ToString(1));
				}

				return pEngine->Scripts->AddFile(sFilename);
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Code.Variables.Free"))
			{
				int iVarIndex = pParams->ToVarIndex(1);
				return ((CEngine *)this->pEngine)->Vars->Free(iVarIndex);
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Code.Variables.IsDefined"))
			{
				PARAMITEM *pParam = &pParams->Collection.Items[1];
				CEngine *pEngine = ((CEngine *)this->pEngine);

				return pReturn->Add(pEngine->Vars->IsDefined(pParam->Value));
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Code.Functions.Call"))
			{
				const char *sExpression = pParams->ToString(1);
				if (this->ExecuteByEval(sExpression, strlen(sExpression), NULL, pReturn) == ERROR_OK)
				{
					return this->Error->HardCount();
				}

				return  this->Error->Hard("Failed to execute dynamic procedure expression");
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Code.Functions.IsDefined"))
			{
				PARAMITEM *pParam = &pParams->Collection.Items[1];
				CEngine *pEngine = ((CEngine *)this->pEngine);

				return pReturn->Add(pEngine->Procs->IsDefined(pParam->Value));
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Code.Variables.Undefine"))
			{
				PARAMITEM *pParam = &pParams->Collection.Items[1];
				CEngine *pEngine = ((CEngine *)this->pEngine);

				if (pEngine->Vars->IsDefined(pParam->Value))
				{
					pEngine->Vars->Free(pParam->Value);
				}
				else {
					this->Error->Soft("Undefined variable");
				}
				return this->Error->HardCount();
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Code.Variables.Define"))
			{
				PARAMITEM *pVariableName = &pParams->Collection.Items[1];
				PARAMITEM *pType = &pParams->Collection.Items[2];
				PARAMITEM *pExpression = &pParams->Collection.Items[3];
				int iScope = (int)pParams->ToInt(4);
				CEngine *pEngine = ((CEngine *)this->pEngine);

				if (!pEngine->Vars->IsDefined(pVariableName->Value))
				{
					int iType = pEngine->Types->GetIndex(pType->Value);
					if (iType == VAR_TYPE_INVALID)
					{
						return this->Error->Soft("Expected: [Valid Type Name]");
					}

					if (iScope < 0)
					{
						iScope = pEngine->State->CurrentScope() - 1; //Place variable in the current scope.
					}

					CParams *pValue = NULL;
					ErrorLevel errorLevel = pEngine->Evaluate(pExpression->Value, pExpression->Length, &pValue);
					if (errorLevel != ERROR_OK)
					{
						delete pValue;
						return errorLevel;
					}
					PARAMITEM *pValueItem = pValue->First();
					errorLevel = pEngine->Vars->AddAndSet(pVariableName->Value, iType, pValueItem->Value, pValueItem->Length, iScope);
					delete pValue;
					return errorLevel;
				}
				else {
					this->Error->Soft("Variable redeclaration");
				}
				return this->Error->HardCount();
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Code.Scope"))
			{
				return pReturn->Add(((CEngine *)this->pEngine)->State->CurrentScope() - 1);
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Code.Scope.Enter"))
			{
				((CEngine *)this->pEngine)->State->ScopeEnter();
				return ERROR_OK;
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Code.Scope.Exit"))
			{
				((CEngine *)this->pEngine)->State->ScopeDrop();
				return ERROR_OK;
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Code.Line"))
			{
				int iLineIndex = ((CEngine *)this->pEngine)->State->LineIndex;
				return pReturn->Add(((CEngine *)this->pEngine)->Lines->GetPointer(iLineIndex)->FileLine);
			}
			else if (_StrEqlI(pProc->Name, "Code.File"))
			{
				int iLineIndex = pEngine->State->LineIndex;
				int iScriptIndex = pEngine->Lines->GetPointer(iLineIndex)->ScriptIndex;
				return pReturn->Add(pEngine->Scripts->GetPointer(iScriptIndex)->FileName);
			}
			else if (_StrEqlI(pProc->Name, "Code.Namespace"))
			{
				int iLineIndex = pEngine->State->LineIndex;
				int iScriptIndex = pEngine->Lines->GetPointer(iLineIndex)->ScriptIndex;
				if (pEngine->Scripts->GetPointer(iScriptIndex)->Namespace)
				{
					return pReturn->Add(pEngine->Scripts->GetPointer(iScriptIndex)->Namespace);
				}
				else {
					return pReturn->Add("");
				}
			}
			//-----------------------------------------------------------------------------------------------------------------
		}
		//*********************************************************************************************************************
		//*********************************************************************************************************************
		//*********************************************************************************************************************
		else if (_strnicmp(pProc->Name, "file.", 5) == 0)
		{
			//-----------------------------------------------------------------------------------------------------------------
			if (_StrEqlI(pProc->Name, "File.Size"))
			{
				VARIABLEITEM *pVar = (VARIABLEITEM *)pParams->ToVarIndexPointer(0);
				PARAMITEM *pFileName = pParams->GetPointer(1);

				if (pFileName->Length > 0)
				{
					__int64 i64FileSize = 0;
					if (FileSize((const char *)pFileName->Value, &i64FileSize))
					{
						return pReturn->Add((double)i64FileSize);
					}
				}
				else {
					HANDLE hFile = NULL;
					if (!pVar->Length || !(*(HANDLE *)pVar->Value))
					{
						this->Error->Soft("Invalid file handle");
						return pReturn->Add(0);
					}
					else {
						hFile = (*(HANDLE *)pVar->Value);
					}

					__int64 i64FileSize = 0;

					if (FileSize(hFile, &i64FileSize))
					{
						return pReturn->Add((double)i64FileSize);
					}
				}

				this->Error->Soft("Failed to retreve file size");
				return pReturn->Add(0);
			}
			//-----------------------------------------------------------------------------------------------------------------
			//Determines if the specififed file exists
			else if (_StrEqlI(pProc->Name, "File.Exists"))
			{
				VARIABLEITEM *pVar = (VARIABLEITEM *)pParams->ToVarIndexPointer(0);
				PARAMITEM *pFileName = pParams->GetPointer(1);
				return pReturn->Add((int)FileAccess((const char *)pFileName->Value, FExist));
			}
			//-----------------------------------------------------------------------------------------------------------------
			//Determines if the current logged in user has read access to the specififed file.
			else if (_StrEqlI(pProc->Name, "File.CanRead"))
			{
				VARIABLEITEM *pVar = (VARIABLEITEM *)pParams->ToVarIndexPointer(0);
				PARAMITEM *pFileName = pParams->GetPointer(1);
				return pReturn->Add((int)FileAccess((const char *)pFileName->Value, FRead));
			}
			//Determines if the current logged in user has write access to the specififed file.
			else if (_StrEqlI(pProc->Name, "File.CanWrite"))
			{
				VARIABLEITEM *pVar = (VARIABLEITEM *)pParams->ToVarIndexPointer(0);
				PARAMITEM *pFileName = pParams->GetPointer(1);
				return pReturn->Add((int)FileAccess((const char *)pFileName->Value, FWrite));
			}
			//Determines if the current logged in user has read/write access to the specififed file.
			else if (_StrEqlI(pProc->Name, "File.CanReadWrite"))
			{
				VARIABLEITEM *pVar = (VARIABLEITEM *)pParams->ToVarIndexPointer(0);
				PARAMITEM *pFileName = pParams->GetPointer(1);
				return pReturn->Add((int)FileAccess((const char *)pFileName->Value, FReadWrite));
			}
			//-----------------------------------------------------------------------------------------------------------------
			//Moves a file from one location to another.
			else if (_StrEqlI(pProc->Name, "File.Rename") || _StrEqlI(pProc->Name, "File.Move"))
			{
				if (MoveFile(pParams->ToString(1), pParams->ToString(2)) == FALSE)
				{
					char sError[5120];
					if (pEngine->GetLastSystemError(sError, sizeof(sError)))
					{
						this->Error->Soft("Failed to move file: [%s]->[%s]", pParams->ToString(1), sError);
					}
					else {
						this->Error->Soft("Failed to move file: [%s]", pParams->ToString(1));
					}
					return pReturn->Add(false);
				}
				else {
					return pReturn->Add(true);
				}
				return pReturn->Add(false);
			}
			//-----------------------------------------------------------------------------------------------------------------
			//Copies a file from one location to another.
			else if (_StrEqlI(pProc->Name, "File.Copy"))
			{
				if (CopyFile(pParams->ToString(1), pParams->ToString(2), pParams->ToBool(3)) == FALSE)
				{
					char sError[5120];
					if (pEngine->GetLastSystemError(sError, sizeof(sError)))
					{
						this->Error->Soft("Failed to copy file: [%s]->[%s]", pParams->ToString(1), sError);
					}
					else {
						this->Error->Soft("Failed to copy file: [%s]", pParams->ToString(1));
					}
					return pReturn->Add(false);
				}
				else {
					return pReturn->Add(true);
				}
				return pReturn->Add(false);
			}
			//-----------------------------------------------------------------------------------------------------------------
			//Deletes a file from the filesystem.
			else if (_StrEqlI(pProc->Name, "File.Delete"))
			{
				if (DeleteFile(pParams->ToString(1)) == FALSE)
				{
					char sError[5120];
					if (pEngine->GetLastSystemError(sError, sizeof(sError)))
					{
						this->Error->Soft("Failed to delete file: [%s]->[%s]", pParams->ToString(1), sError);
					}
					else {
						this->Error->Soft("Failed to delete file: [%s]", pParams->ToString(1));
					}
					return pReturn->Add(false);
				}
				else {
					return pReturn->Add(true);
				}
				return pReturn->Add(false);
			}
			//-----------------------------------------------------------------------------------------------------------------
			//Opens a new file for read or write access.
			else if (_StrEqlI(pProc->Name, "File.Open"))
			{
				int iVarIndex = pParams->ToVarIndex(0);

				DWORD dwReadWrite = (DWORD)pParams->ToUInt(2);
				DWORD dwCreationDisposition = (DWORD)pParams->ToUInt(3);

				if (dwReadWrite == -1) //Default mode is READ.
				{
					dwReadWrite = GENERIC_READ;
				}

				//Figure out the default creation disposition based on the read mode.
				if (dwCreationDisposition == -1)
				{
					if (dwReadWrite &GENERIC_WRITE)
					{
						dwCreationDisposition = OPEN_ALWAYS;
					}
					else {
						dwCreationDisposition = OPEN_EXISTING;
					}
				}

				HANDLE hFile = CreateFile(pParams->ToString(1), dwReadWrite,
					0, NULL, dwCreationDisposition, FILE_ATTRIBUTE_NORMAL, NULL);

				if (hFile == INVALID_HANDLE_VALUE)
				{
					char sError[5120];
					if (pEngine->GetLastSystemError(sError, sizeof(sError)))
					{
						this->Error->Soft("Failed to open file: [%s]->[%s]", pParams->ToString(1), sError);
					}
					else {
						this->Error->Soft("Failed to open file: [%s]", pParams->ToString(1));
					}
					return pReturn->Add(false);
				}
				else {

					pEngine->Vars->Set(iVarIndex, hFile);
					return pReturn->Add(true);
				}
				return pReturn->Add(false);
			}
			//-----------------------------------------------------------------------------------------------------------------
			//Opens a new file for read or write access.
			else if (_StrEqlI(pProc->Name, "File.Close"))
			{
				bool bResult = false;
				int iVarIndex = pParams->ToVarIndex(0);

				HANDLE hFile = pEngine->Vars->GetHandle(iVarIndex);

				if (hFile == NULL || hFile == INVALID_HANDLE_VALUE)
				{
					this->Error->Soft("Invalid file handle");
					return pReturn->Add(false);
				}

				if (!(bResult = (CloseHandle(hFile) > 0)))
				{
					char sError[5120];
					if (pEngine->GetLastSystemError(sError, sizeof(sError)))
					{
						this->Error->Soft("Close handle failed: [%s]", sError);
					}
					else {
						this->Error->Soft("Close handle failed");
					}
				}

				pEngine->Vars->Set(iVarIndex, (char *)NULL);

				return pReturn->Add(bResult);
			}
			//-----------------------------------------------------------------------------------------------------------------
			//Opens a new file for read or write access.
			else if (_StrEqlI(pProc->Name, "File.Handle"))
			{
				HANDLE hFile = pEngine->Vars->GetHandle(pParams->ToVarIndex(0));

				if (hFile == NULL || hFile == INVALID_HANDLE_VALUE)
				{
					this->Error->Soft("Invalid file handle");
					return pReturn->Add(0);
				}

				return pReturn->Add((unsigned int)hFile);
			}
			//-----------------------------------------------------------------------------------------------------------------
			//Reads from an open file.
			else if (_StrEqlI(pProc->Name, "File.Read"))
			{
				int iVarIndex = pParams->ToVarIndex(0);
				HANDLE hFile = pEngine->Vars->GetHandle(iVarIndex);

				if (hFile == NULL || hFile == INVALID_HANDLE_VALUE)
				{
					this->Error->Soft("Invalid file handle");
					return pReturn->Add(0);
				}

				DWORD dwBytesRead = 0;
				int iBytesToRead = (int)pParams->ToInt(1);
				if (iBytesToRead < 0)
				{
					iBytesToRead = GetFileSize(hFile, (DWORD *)&iBytesToRead);
				}

				char *sBuffer = (char *)pMem->Allocate(sizeof(char), iBytesToRead);
				if (!sBuffer)
				{
					return this->Error->Hard("Memory allocation error");
				}

				if (!ReadFile(hFile, sBuffer, iBytesToRead, &dwBytesRead, NULL))
				{
					char sError[5120];
					if (pEngine->GetLastSystemError(sError, sizeof(sError)))
					{
						this->Error->Soft("Write failed: [%s]", sError);
					}
					else {
						this->Error->Soft("Write file failed");
					}
				}

				ErrorLevel Result = pReturn->Add(sBuffer, (unsigned int)dwBytesRead);
				pMem->Free(sBuffer);
				return Result;
			}
			//-----------------------------------------------------------------------------------------------------------------
			//Writes from an open file.
			else if (_StrEqlI(pProc->Name, "File.Write"))
			{
				int iVarIndex = pParams->ToVarIndex(0);
				HANDLE hFile = pEngine->Vars->GetHandle(iVarIndex);

				if (hFile == NULL || hFile == INVALID_HANDLE_VALUE)
				{
					this->Error->Soft("Invalid file handle");
					return pReturn->Add(0);
				}

				DWORD dwBytesWritten = 0;
				int iBytesToWrite = (int)pParams->ToInt(2);
				if (iBytesToWrite < 0)
				{
					iBytesToWrite = pParams->Collection.Items[1].Length;
				}

				if (!WriteFile(hFile, pParams->ToString(1), iBytesToWrite, &dwBytesWritten, NULL))
				{
					char sError[5120];
					if (pEngine->GetLastSystemError(sError, sizeof(sError)))
					{
						this->Error->Soft("Write failed: [%s]", sError);
					}
					else {
						this->Error->Soft("Write file failed");
					}
				}

				return pReturn->Add((unsigned int)dwBytesWritten);
			}
			//-----------------------------------------------------------------------------------------------------------------
			//Sets the file pointer position from the current file position.
			else if (_StrEqlI(pProc->Name, "File.Seek"))
			{
				int iVarIndex = pParams->ToVarIndex(0);
				HANDLE hFile = pEngine->Vars->GetHandle(iVarIndex);

				if (hFile == NULL || hFile == INVALID_HANDLE_VALUE)
				{
					this->Error->Soft("Invalid file handle");
					return pReturn->Add(0);
				}

				return pReturn->Add((int)FileSeek(hFile, pParams->ToInt(1), FILE_CURRENT));
			}
			//-----------------------------------------------------------------------------------------------------------------
			//Sets the file pointer position from the beginning of the file.
			else if (_StrEqlI(pProc->Name, "File.Seek.FromBeginning"))
			{
				int iVarIndex = pParams->ToVarIndex(0);
				HANDLE hFile = pEngine->Vars->GetHandle(iVarIndex);

				if (hFile == NULL || hFile == INVALID_HANDLE_VALUE)
				{
					this->Error->Soft("Invalid file handle");
					return pReturn->Add(0);
				}

				return pReturn->Add((int)FileSeek(hFile, pParams->ToInt(1), FILE_BEGIN));
			}
			//-----------------------------------------------------------------------------------------------------------------
			//Sets the file pointer position from the end of the file.
			else if (_StrEqlI(pProc->Name, "File.Seek.FromEnd"))
			{
				int iVarIndex = pParams->ToVarIndex(0);
				HANDLE hFile = pEngine->Vars->GetHandle(iVarIndex);

				if (hFile == NULL || hFile == INVALID_HANDLE_VALUE)
				{
					this->Error->Soft("Invalid file handle");
					return pReturn->Add(0);
				}

				return pReturn->Add((int)FileSeek(hFile, pParams->ToInt(1), FILE_END));
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "File.Drive"))
			{
				char sFilePart[MAX_PATH];
				if (GetFileDrive(pParams->ToString(1), sFilePart, sizeof(sFilePart)))
				{
					return pReturn->Add(sFilePart);
				}
				else {
					return this->Error->Hard("GetFileDrive failed.");
				}
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "File.Path"))
			{
				char sFilePart[MAX_PATH];
				if (GetFilePath(pParams->ToString(1), sFilePart, sizeof(sFilePart)))
				{
					return pReturn->Add(sFilePart);
				}
				else {
					return this->Error->Hard("GetFilePath failed.");
				}
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "File.Name"))
			{
				char sFilePart[MAX_PATH];
				if (GetFileName(pParams->ToString(1), sFilePart, sizeof(sFilePart)))
				{
					return pReturn->Add(sFilePart);
				}
				else {
					return this->Error->Hard("GetFileName failed.");
				}
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "File.NameOnly"))
			{
				char sFilePart[MAX_PATH];
				if (GetFileNameOnly(pParams->ToString(1), sFilePart, sizeof(sFilePart)))
				{
					return pReturn->Add(sFilePart);
				}
				else {
					return this->Error->Hard("GetFileNameOnly failed.");
				}
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "File.Extension"))
			{
				char sFilePart[MAX_PATH];
				if (GetFileExtension(pParams->ToString(1), sFilePart, sizeof(sFilePart)))
				{
					return pReturn->Add(sFilePart);
				}
				else {
					return this->Error->Hard("GetFileExtension failed.");
				}
			}
		}
		//*********************************************************************************************************************
		//*********************************************************************************************************************
		//*********************************************************************************************************************
		else if (_strnicmp(pProc->Name, "sql.", 4) == 0)
		{
			//-----------------------------------------------------------------------------------------------------------------
			if (_StrEqlI(pProc->Name, "SQL.Connection.Connect"))
			{
				CEngine *pEngine((CEngine *)this->pEngine);

				int iVarIndex = pParams->ToVarIndex(0);
				char *sConnectionString = pParams->Collection.Items[1].Value;
				CSQL *pSQL = (CSQL *)pEngine->Vars->GetPointer(iVarIndex)->Value;

				bool bResult = pSQL->Connect(sConnectionString, SQLErrorHandler);
			
				if (!bResult || !pSQL->IsConnected())
				{
					return this->Error->Inform("Failed to connect to the SQL Server.");
				}

				return this->Error->HardCount();
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "SQL.Connection.Close"))
			{
				ErrorLevel Result = ERROR_OK;

				CEngine *pEngine((CEngine *)this->pEngine);
				int iVarIndex = pParams->ToVarIndex(0);
				char *sSQL = pParams->Collection.Items[1].Value;
				CSQL *pSQL = (CSQL *)pEngine->Vars->GetPointer(iVarIndex)->Value;

				if (!pSQL->IsConnected())
				{
					return this->Error->Soft("the SQL Server connection is not open.");
				}

				return this->Error->HardCount();
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "SQL.Connection.Transaction.Begin"))
			{
				CEngine *pEngine((CEngine *)this->pEngine);
				int iVarIndex = pParams->ToVarIndex(0);
				CSQL *pSQL = (CSQL *)pEngine->Vars->GetPointer(iVarIndex)->Value;
				const char *sTransactionName = pParams->ToString(1);

				if (sTransactionName)
				{
					pSQL->BeginTransaction(sTransactionName);
				}
				else {
					pSQL->BeginTransaction();
				}

				return this->Error->HardCount();
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "SQL.Connection.Transaction.Commit"))
			{
				CEngine *pEngine((CEngine *)this->pEngine);
				int iVarIndex = pParams->ToVarIndex(0);
				CSQL *pSQL = (CSQL *)pEngine->Vars->GetPointer(iVarIndex)->Value;
				const char *sTransactionName = pParams->ToString(1);

				if (sTransactionName)
				{
					pSQL->CommitTransaction(sTransactionName);
				}
				else {
					pSQL->CommitTransaction();
				}

				return this->Error->HardCount();
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "SQL.Connection.Transaction.Rollback"))
			{
				CEngine *pEngine((CEngine *)this->pEngine);
				int iVarIndex = pParams->ToVarIndex(0);
				CSQL *pSQL = (CSQL *)pEngine->Vars->GetPointer(iVarIndex)->Value;
				const char *sTransactionName = pParams->ToString(1);

				if (sTransactionName)
				{
					pSQL->RollbackTransaction(sTransactionName);
				}
				else {
					pSQL->RollbackTransaction();
				}

				return this->Error->HardCount();
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "SQL.Connection.Transaction.Depth"))
			{
				CEngine *pEngine((CEngine *)this->pEngine);
				int iVarIndex = pParams->ToVarIndex(0);
				CSQL *pSQL = (CSQL *)pEngine->Vars->GetPointer(iVarIndex)->Value;

				return pReturn->Add(pSQL->TransactionDepth());
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "SQL.Connection.Value"))
			{
				ErrorLevel Result = ERROR_OK;

				CEngine *pEngine((CEngine *)this->pEngine);
				int iVarIndex = pParams->ToVarIndex(0);
				char *sSQL = pParams->Collection.Items[1].Value;
				CSQL *pSQL = (CSQL *)pEngine->Vars->GetPointer(iVarIndex)->Value;

				CRecordSet rsTemp;
				rsTemp.pPublicData = pEngine;
				if (pSQL->Execute(sSQL, &rsTemp))
				{
					rsTemp.ThrowErrors(true);

					if (rsTemp.Fetch())
					{
						int iMaxSize = 0;
						char *sBuffer = this->SQLBuffer(&iMaxSize);
						if (sBuffer)
						{
							SQLLEN iOutSz = 0;
							if (rsTemp.sColumnEx(1, sBuffer, (SQLINTEGER)iMaxSize, &iOutSz))
							{
								Result = pReturn->Add(sBuffer, (int)iOutSz);
							}
						}
					}
				}
				rsTemp.Close();

				return Result;
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "SQL.Connection.Execute"))
			{
				ErrorLevel Result = ERROR_OK;

				CEngine *pEngine((CEngine *)this->pEngine);
				int iVarIndex = pParams->ToVarIndex(0);
				char *sSQL = pParams->Collection.Items[1].Value;
				CSQL *pSQL = (CSQL *)pEngine->Vars->GetPointer(iVarIndex)->Value;

				VARIABLEITEM *pRSVar = (VARIABLEITEM *)pParams->ToVarIndexPointer(2);
				if (pRSVar)
				{
					CBoundRecordSet *pRS = (CBoundRecordSet *)pRSVar->Value;
					pRS->ThrowErrors(true);

					if (!pSQL->Execute(sSQL, pRS))
					{
						this->Error->Inform("Failed to execute SQL statement");
					}
				}
				else
				{
					pSQL->ThrowErrors(true);

					if (!pSQL->Execute(sSQL))
					{
						this->Error->Inform("Failed to execute SQL statement");
					}
				}
				return this->Error->HardCount();
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "SQL.RecordSet.Columns.Count"))
			{
				ErrorLevel Result = ERROR_OK;

				CEngine *pEngine((CEngine *)this->pEngine);

				CBoundRecordSet *pRS = (CBoundRecordSet *)pEngine->Vars->GetPointer(pParams->ToVarIndex(0))->Value;
				if (!pRS)
				{
					return this->Error->Soft("RecordSet is not set");
				}

				return pReturn->Add(pRS->Columns.Count);
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "SQL.RecordSet.Columns.Name"))
			{
				ErrorLevel Result = ERROR_OK;

				CEngine *pEngine((CEngine *)this->pEngine);

				CBoundRecordSet *pRS = (CBoundRecordSet *)pEngine->Vars->GetPointer(pParams->ToVarIndex(0))->Value;
				if (!pRS)
				{
					return this->Error->Soft("RecordSet is not set");
				}

				char *sColumnIndex = pParams->Collection.Items[1].Value;
				int iColumnIndex = -1;

				if (IsNumeric(sColumnIndex) && CharIndex(sColumnIndex, '.') < 0)
				{
					iColumnIndex = atol(sColumnIndex);

					if (iColumnIndex < 0 || iColumnIndex >= pRS->Columns.Count)
					{
						return this->Error->Soft("Invalid column index: [%d]", iColumnIndex);
					}
				}
				else {
					if (iColumnIndex < 0 || iColumnIndex >= pRS->Columns.Count)
					{
						return this->Error->Soft("Invalid column index: [%s]", sColumnIndex);
					}
				}

				return pReturn->Add(pRS->Columns.Column[iColumnIndex].Name);
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "SQL.RecordSet.Columns.Index"))
			{
				ErrorLevel Result = ERROR_OK;

				CEngine *pEngine((CEngine *)this->pEngine);

				CBoundRecordSet *pRS = (CBoundRecordSet *)pEngine->Vars->GetPointer(pParams->ToVarIndex(0))->Value;
				if (!pRS)
				{
					return this->Error->Soft("RecordSet is not set");
				}

				char *sColumnName = pParams->ToString(1);
				int iColumnIndex = pRS->GetColumnIndex(sColumnName);

				if (iColumnIndex < 0 || iColumnIndex >= pRS->Columns.Count)
				{
					return this->Error->Soft("Invalid column name: [%s]", sColumnName);
				}

				return pReturn->Add(iColumnIndex);
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "SQL.RecordSet.Value"))
			{
				ErrorLevel Result = ERROR_OK;

				CEngine *pEngine((CEngine *)this->pEngine);
				int iVarIndex = pParams->ToVarIndex(0);

				CBoundRecordSet *pRS = (CBoundRecordSet *)pEngine->Vars->GetPointer(iVarIndex)->Value;
				if (!pRS)
				{
					return this->Error->Soft("RecordSet is not set");
				}

				char *sColumnName = pParams->Collection.Items[1].Value;
				int iColumnIndex = -1;

				if (IsNumeric(sColumnName) && CharIndex(sColumnName, '.') < 0)
				{
					iColumnIndex = atol(sColumnName);
					if (iColumnIndex < 0 || iColumnIndex >= pRS->Columns.Count)
					{
						return this->Error->Soft("Invalid column index: [%d]", iColumnIndex);
					}
				}
				else {
					iColumnIndex = pRS->GetColumnIndex(sColumnName);
				}

				if (iColumnIndex < 0 || iColumnIndex >= pRS->Columns.Count)
				{
					return this->Error->Soft("Invalid column name: [%s]", sColumnName);
				}

				CSQLValue *pValue = pRS->Values(iColumnIndex);
				if (pValue)
				{
					switch (pRS->GetDefaultConversion(pRS->Columns.Column[iColumnIndex].DataType))
					{
					case CTypes::Bit:
						Result = pReturn->Add(pValue->ToBoolean());
						break;
					case CTypes::Char:
						Result = pReturn->Add(pValue->ToString(), (int)pValue->Size());
						break;
					case CTypes::Double:
						Result = pReturn->Add(pValue->ToDouble());
						break;
					case CTypes::Float:
						Result = pReturn->Add(pValue->ToFloat());
						break;
					case CTypes::SBigInt:
						Result = this->Error->Hard("64bit integers have not been implemented");
						//Result = pReturn->Add(pValue->ToI64S);
						break;
					case CTypes::SLong:
						Result = pReturn->Add(pValue->ToIntegerS());
						break;
					case CTypes::SShort:
						Result = pReturn->Add(pValue->ToShortS());
						break;
					}
					delete pValue;
				}
				return Result;
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "SQL.RecordSet.Fetch"))
			{
				ErrorLevel Result = ERROR_OK;

				CEngine *pEngine((CEngine *)this->pEngine);
				int iVarIndex = pParams->ToVarIndex(0);

				CBoundRecordSet *pRS = (CBoundRecordSet *)pEngine->Vars->GetPointer(iVarIndex)->Value;
				if (!pRS)
				{
					return this->Error->Soft("RecordSet is not set");
				}

				return pReturn->Add((int)pRS->Fetch());
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "SQL.RecordSet.Close"))
			{
				ErrorLevel Result = ERROR_OK;

				CEngine *pEngine((CEngine *)this->pEngine);
				int iVarIndex = pParams->ToVarIndex(0);

				CBoundRecordSet *pRS = (CBoundRecordSet *)pEngine->Vars->GetPointer(iVarIndex)->Value;
				if (!pRS)
				{
					return this->Error->Soft("RecordSet is not set");
				}

				if (!pRS->Close())
				{
					this->Error->Soft("Failed to close the RecordSet.");
				}

				return this->Error->HardCount();
			}
			//-----------------------------------------------------------------------------------------------------------------
		}
		//*********************************************************************************************************************
		//*********************************************************************************************************************
		//*********************************************************************************************************************
		else if (_strnicmp(pProc->Name, "XML.", 4) == 0)
		{
			//-----------------------------------------------------------------------------------------------------------------
			if (_StrEqlI(pProc->Name, "XML.Reader.Open"))
			{
				ErrorLevel Result = ERROR_OK;

				CEngine *pEngine((CEngine *)this->pEngine);

				int iVarIndex = pParams->ToVarIndex(0);
				char *sFileName = pParams->ToString(1);

				XMLReader *pPointer = (XMLReader *)pEngine->Vars->GetPointer(iVarIndex)->Value;
				if(pPointer->FromFile(sFileName) == false)
				{
					return this->Error->Soft("Failed to open the XML file [%s]", sFileName);
				}

				return this->Error->HardCount();
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "XML.Reader.Close"))
			{
				ErrorLevel Result = ERROR_OK;

				CEngine *pEngine((CEngine *)this->pEngine);

				int iVarIndex = pParams->ToVarIndex(0);

				XMLReader *pPointer = (XMLReader *)pEngine->Vars->GetPointer(iVarIndex)->Value;
				pPointer->Destroy();

				return this->Error->HardCount();
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "XML.Reader.Parse"))
			{
				ErrorLevel Result = ERROR_OK;

				CEngine *pEngine((CEngine *)this->pEngine);

				int iVarIndex = pParams->ToVarIndex(0);
				char *sXML = pParams->ToString(1);

				XMLReader *pPointer = (XMLReader *)pEngine->Vars->GetPointer(iVarIndex)->Value;

				return pReturn->Add(pPointer->Initialize(sXML));
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "XML.Reader.ScanPosition"))
			{
				ErrorLevel Result = ERROR_OK;

				CEngine *pEngine((CEngine *)this->pEngine);

				int iVarIndex = pParams->ToVarIndex(0);
				int iNewPosition = pParams->ToInt(1);

				XMLReader *pPointer = (XMLReader *)pEngine->Vars->GetPointer(iVarIndex)->Value;

				if(iNewPosition >= 0)
				{
					return pReturn->Add(pPointer->ScanPosition(iNewPosition));
				}
				else {
					return pReturn->Add(pPointer->ScanPosition());
				}
			}
			//-----------------------------------------------------------------------------------------------------------------
			else  if (_StrEqlI(pProc->Name, "XML.Reader.Length"))
			{
				ErrorLevel Result = ERROR_OK;
				CEngine *pEngine((CEngine *)this->pEngine);
				int iVarIndex = pParams->ToVarIndex(0);
				XMLReader *pPointer = (XMLReader *)pEngine->Vars->GetPointer(iVarIndex)->Value;
				return pReturn->Add(pPointer->Length());
			}
			//-----------------------------------------------------------------------------------------------------------------
			else  if (_StrEqlI(pProc->Name, "XML.Reader.Text"))
			{
				ErrorLevel Result = ERROR_OK;
				CEngine *pEngine((CEngine *)this->pEngine);
				int iVarIndex = pParams->ToVarIndex(0);
				XMLReader *pPointer = (XMLReader *)pEngine->Vars->GetPointer(iVarIndex)->Value;
				return pReturn->Add(pPointer->Text());
			}
			//-----------------------------------------------------------------------------------------------------------------
			else  if (_StrEqlI(pProc->Name, "XML.Reader.ToFile"))
			{
				ErrorLevel Result = ERROR_OK;
				CEngine *pEngine((CEngine *)this->pEngine);
				int iVarIndex = pParams->ToVarIndex(0);
				char *sFileName = pParams->ToString(1);
				XMLReader *pPointer = (XMLReader *)pEngine->Vars->GetPointer(iVarIndex)->Value;
				return pReturn->Add(pPointer->ToFile(sFileName));
			}
			//-----------------------------------------------------------------------------------------------------------------
			else  if (_StrEqlI(pProc->Name, "XML.Reader.ToReader"))
			{
				ErrorLevel Result = ERROR_OK;

				CEngine *pEngine((CEngine *)this->pEngine);

				int iVarIndex = pParams->ToVarIndex(0);
				char *sTag = pParams->ToString(1);
				VARIABLEITEM *xmlReaderVariable = (VARIABLEITEM *)pParams->ToVarIndexPointer(2);
				XMLReader *xmlReader = ((XMLReader *)xmlReaderVariable->Value);
				StringBuilder resultingString;
				XMLReader *pPointer = (XMLReader *)pEngine->Vars->GetPointer(iVarIndex)->Value;
				return pReturn->Add(pPointer->ToReader(sTag, xmlReader));
			}
			//-----------------------------------------------------------------------------------------------------------------
			else  if (_StrEqlI(pProc->Name, "XML.Reader.ToInteger"))
			{
				ErrorLevel Result = ERROR_OK;

				CEngine *pEngine((CEngine *)this->pEngine);

				int iVarIndex = pParams->ToVarIndex(0);
				char *sTag = pParams->ToString(1);
				int iDefault = pParams->ToInt(2);

				XMLReader *pPointer = (XMLReader *)pEngine->Vars->GetPointer(iVarIndex)->Value;
				return pReturn->Add(pPointer->ToInteger(sTag, iDefault));
			}
			//-----------------------------------------------------------------------------------------------------------------
			else  if (_StrEqlI(pProc->Name, "XML.Reader.ToDouble"))
			{
				ErrorLevel Result = ERROR_OK;

				CEngine *pEngine((CEngine *)this->pEngine);

				int iVarIndex = pParams->ToVarIndex(0);
				char *sTag = pParams->ToString(1);
				double dDefault = pParams->ToDouble(2);

				XMLReader *pPointer = (XMLReader *)pEngine->Vars->GetPointer(iVarIndex)->Value;
				return pReturn->Add(pPointer->ToDouble(sTag, dDefault));
			}
			//-----------------------------------------------------------------------------------------------------------------
			else  if (_StrEqlI(pProc->Name, "XML.Reader.ToString"))
			{
				ErrorLevel Result = ERROR_OK;

				CEngine *pEngine((CEngine *)this->pEngine);

				int iVarIndex = pParams->ToVarIndex(0);
				char *sTag = pParams->ToString(1);
				char *sDefault = pParams->ToString(2);

				StringBuilder resultingString;

				XMLReader *pPointer = (XMLReader *)pEngine->Vars->GetPointer(iVarIndex)->Value;
				pPointer->ToString(sTag, &resultingString, sDefault);

				ErrorLevel result = pReturn->Add(resultingString.Buffer, resultingString.Length);

				resultingString.Destroy();

				return result;
			}
			//-----------------------------------------------------------------------------------------------------------------
			else  if (_StrEqlI(pProc->Name, "XML.Reader.AttributeLength"))
			{
				ErrorLevel Result = ERROR_OK;

				CEngine *pEngine((CEngine *)this->pEngine);

				int iVarIndex = pParams->ToVarIndex(0);
				char *sTag = pParams->ToString(1);

				XMLReader *pPointer = (XMLReader *)pEngine->Vars->GetPointer(iVarIndex)->Value;

				return pReturn->Add(pPointer->AttributeLength(sTag));
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "XML.Reader.ProgressiveScan"))
			{
				ErrorLevel Result = ERROR_OK;

				CEngine *pEngine((CEngine *)this->pEngine);

				int iVarIndex = pParams->ToVarIndex(0);
				bool bEnable = pParams->ToBool(1);

				XMLReader *pPointer = (XMLReader *)pEngine->Vars->GetPointer(iVarIndex)->Value;
				pPointer->ProgressiveScan(bEnable);
				return this->Error->HardCount();
			}

			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "XML.Writer.SetName"))
			{
				ErrorLevel Result = ERROR_OK;

				CEngine *pEngine((CEngine *)this->pEngine);

				int iVarIndex = pParams->ToVarIndex(0);
				char *sName = pParams->ToString(1);

				XMLWriter *pPointer = (XMLWriter *)pEngine->Vars->GetPointer(iVarIndex)->Value;
				if(pPointer->ChangeName(sName) == false)
				{
					return this->Error->Soft("Failed to add XML attribute [%s]", sName);
				}

				return this->Error->HardCount();
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "XML.Writer.Length"))
			{
				ErrorLevel Result = ERROR_OK;

				CEngine *pEngine((CEngine *)this->pEngine);

				int iVarIndex = pParams->ToVarIndex(0);

				XMLWriter *pPointer = (XMLWriter *)pEngine->Vars->GetPointer(iVarIndex)->Value;

				ErrorLevel result = pReturn->Add(pPointer->Length());

				return this->Error->HardCount();
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "XML.Writer.Text"))
			{
				ErrorLevel Result = ERROR_OK;

				CEngine *pEngine((CEngine *)this->pEngine);

				int iVarIndex = pParams->ToVarIndex(0);

				XMLWriter *pPointer = (XMLWriter *)pEngine->Vars->GetPointer(iVarIndex)->Value;

				int iOutSize = 0;
				char *sText = pPointer->Text(&iOutSize);

				ErrorLevel result = pReturn->Add(sText, iOutSize);

				pMem->Free(sText);

				return result;
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "XML.Writer.AddXML"))
			{
				ErrorLevel Result = ERROR_OK;

				CEngine *pEngine((CEngine *)this->pEngine);

				int iVarIndex = pParams->ToVarIndex(0);
				VARIABLEITEM *pVar = (VARIABLEITEM * )pParams->ToVarIndexPointer(1);

				XMLWriter *pAddPointer = (XMLWriter *) pVar->Value;
				XMLWriter *pPointer = (XMLWriter *)pEngine->Vars->GetPointer(iVarIndex)->Value;

				if(pPointer->Add(pAddPointer) == false)
				{
					return this->Error->Soft("Failed to add XML attribute(s)");
				}

				return this->Error->HardCount();
			}
			//---------------------- -------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "XML.Writer.Add"))
			{
				ErrorLevel Result = ERROR_OK;

				CEngine *pEngine((CEngine *)this->pEngine);

				int iVarIndex = pParams->ToVarIndex(0);
				char *sName = pParams->ToString(1);
				char *sValue = pParams->ToString(2);

				XMLWriter *pPointer = (XMLWriter *)pEngine->Vars->GetPointer(iVarIndex)->Value;
				if(pPointer->Add(sName, sValue) == false)
				{
					return this->Error->Soft("Failed to add XML attribute [%s]", sName);
				}

				return this->Error->HardCount();
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "XML.Writer.Save"))
			{
				ErrorLevel Result = ERROR_OK;

				CEngine *pEngine((CEngine *)this->pEngine);

				int iVarIndex = pParams->ToVarIndex(0);
				char *sFileName = pParams->ToString(1);

				XMLWriter *pPointer = (XMLWriter *)pEngine->Vars->GetPointer(iVarIndex)->Value;
				if(pPointer->ToFile(sFileName) == false)
				{
					return this->Error->Soft("Failed to save XML file [%s]", sFileName);
				}

				return this->Error->HardCount();
			}
		}
		//*********************************************************************************************************************
		//*********************************************************************************************************************
		//*********************************************************************************************************************
		else if (_strnicmp(pProc->Name, "socket.", 7) == 0)
		{
			//-----------------------------------------------------------------------------------------------------------------
			if (_StrEqlI(pProc->Name, "Socket.Server.Stop"))
			{
				ErrorLevel Result = ERROR_OK;

				CEngine *pEngine((CEngine *)this->pEngine);

				int iVarIndex = pParams->ToVarIndex(0);

				CSocketServer *pServer = (CSocketServer *)pEngine->Vars->GetPointer(iVarIndex)->Value;

				pServer->Stop(true);

				return this->Error->HardCount();
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Socket.Server.Start"))
			{
				CEngine *pEngine((CEngine *)this->pEngine);

				int iVarIndex = pParams->ToVarIndex(0);
				int iMaxConnections = (int)pParams->ToInt(1);
				int iPort = (int)pParams->ToInt(2);

				CSocketServer *pServer = (CSocketServer *)pEngine->Vars->GetPointer(iVarIndex)->Value;

				if (!pServer->IsInitialized())
				{
					if (!pServer->Initialize(iPort, iMaxConnections, &SS_SocketOnError))
					{
						return this->Error->HardCount(); //Errors are supplied via SS_SocketOnError
					}

					if (pServer)
					{
						pServer->PacketMode(false);
					}
				}

				pServer->Start();

				return this->Error->HardCount();
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Socket.Server.Connect"))
			{
				ErrorLevel Result = ERROR_OK;

				CEngine *pEngine((CEngine *)this->pEngine);

				int iVarIndex = pParams->ToVarIndex(0);
				const char *sHost = pParams->ToString(1);
				int iPort = (int)pParams->ToInt(2);

				VARIABLEITEM *pClientVar = (VARIABLEITEM *)pParams->ToVarIndexPointer(3);
				CSocketServer *pServer = (CSocketServer *)pEngine->Vars->GetPointer(iVarIndex)->Value;

				CSocketClient *pClient = NULL;
				bool bResult = pServer->Connect(sHost, iPort, &pClient);

				if (bResult)
				{
					pClient->Reserved(true);
					pEngine->Vars->SetPointer(pClientVar, pClient, sizeof(CSocketClient));
					((CSocketClient *)pClientVar->Value)->pUserData = pEngine;
				}
				else {
					this->Error->Inform("Failed to connect to the remote host");
				}

				return this->Error->HardCount();
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Socket.Server.Connections"))
			{
				ErrorLevel Result = ERROR_OK;

				CEngine *pEngine((CEngine *)this->pEngine);

				int iVarIndex = pParams->ToVarIndex(0);

				CSocketServer *pServer = (CSocketServer *)pEngine->Vars->GetPointer(iVarIndex)->Value;

				return pReturn->Add(pServer->CurrentConnections());
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Socket.Server.Connection"))
			{
				ErrorLevel Result = ERROR_OK;

				CEngine *pEngine((CEngine *)this->pEngine);

				int iVarIndex = pParams->ToVarIndex(0);
				VARIABLEITEM *pClientVar = (VARIABLEITEM *)pParams->ToVarIndexPointer(1);
				int iPeerIndex = (int)pParams->ToInt(2);

				CSocketServer *pServer = (CSocketServer *)pEngine->Vars->GetPointer(iVarIndex)->Value;
				CSocketClient *pClient = NULL;

				if (iPeerIndex >= 0)
				{
					pClient = pServer->Client(iPeerIndex);
				}
				else {
					pClient = pServer->GetConnectedClient(); //Enummerate
				}

				if (pClient)
				{
					pEngine->Vars->SetPointer(pClientVar, pClient, sizeof(CSocketClient));
					((CSocketClient *)pClientVar->Value)->pUserData = pEngine;
				}

				return pReturn->Add((unsigned int)pClient != NULL);
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Socket.Client.Disconnect"))
			{
				ErrorLevel Result = ERROR_OK;

				CEngine *pEngine((CEngine *)this->pEngine);

				CSocketClient *pClient = (CSocketClient *)pEngine->Vars->GetPointer(pParams->ToVarIndex(0))->Value;

				pClient->WaitOnDataToBeSent();
				pClient->Reserved(false);
				pClient->Disconnect();

				return this->Error->HardCount();
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Socket.Client.Send"))
			{
				ErrorLevel Result = ERROR_OK;

				CEngine *pEngine((CEngine *)this->pEngine);

				CSocketClient *pClient = (CSocketClient *)pEngine->Vars->GetPointer(pParams->ToVarIndex(0))->Value;

				PARAMITEM *pData = pParams->GetPointer(1);

				return pReturn->Add(pClient->SetNextSendData(pData->Value, pData->Length));
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Socket.Client.Recv"))
			{
				ErrorLevel Result = ERROR_OK;

				CEngine *pEngine((CEngine *)this->pEngine);

				CSocketClient *pClient = (CSocketClient *)pEngine->Vars->GetPointer(pParams->ToVarIndex(0))->Value;

				BASICHUNK chunk;
				memset(&chunk, 0, sizeof(chunk));

				if (pClient->GetNextClientData(&chunk))
				{
					Result = pReturn->Add((char *)chunk.pBuffer, chunk.iLength);
					pClient->Free(&chunk);
				}
				else {
					Result = pReturn->Add("");
				}

				return Result;
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Socket.Client.TrySend"))
			{
				ErrorLevel Result = ERROR_OK;

				CEngine *pEngine((CEngine *)this->pEngine);

				CSocketClient *pClient = (CSocketClient *)pEngine->Vars->GetPointer(pParams->ToVarIndex(0))->Value;

				PARAMITEM *pData = pParams->GetPointer(1);

				return pReturn->Add(pClient->SetSendData(pData->Value, pData->Length));
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Socket.Client.TryRecv"))
			{
				ErrorLevel Result = ERROR_OK;

				CEngine *pEngine((CEngine *)this->pEngine);

				CSocketClient *pClient = (CSocketClient *)pEngine->Vars->GetPointer(pParams->ToVarIndex(0))->Value;

				BASICHUNK chunk;
				memset(&chunk, 0, sizeof(chunk));

				if (pClient->GetClientData(&chunk))
				{
					Result = pReturn->Add((char *)chunk.pBuffer, chunk.iLength);
					pClient->Free(&chunk);
				}
				else {
					Result = pReturn->Add("");
				}

				return Result;
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Socket.Client.IsSendPending"))
			{
				ErrorLevel Result = ERROR_OK;

				CEngine *pEngine((CEngine *)this->pEngine);

				CSocketClient *pClient = (CSocketClient *)pEngine->Vars->GetPointer(pParams->ToVarIndex(0))->Value;

				return pReturn->Add(pClient->IsSendDataWaiting() && pClient->IsConnected(true) && !pClient->IsDisconnectSet());
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Socket.Client.IsRecvPending"))
			{
				ErrorLevel Result = ERROR_OK;

				CEngine *pEngine((CEngine *)this->pEngine);

				CSocketClient *pClient = (CSocketClient *)pEngine->Vars->GetPointer(pParams->ToVarIndex(0))->Value;

				return pReturn->Add(pClient->IsRecvDataWaiting());
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Socket.Client.IsConnected"))
			{
				ErrorLevel Result = ERROR_OK;

				CEngine *pEngine((CEngine *)this->pEngine);

				CSocketClient *pClient = (CSocketClient *)pEngine->Vars->GetPointer(pParams->ToVarIndex(0))->Value;

				return pReturn->Add(pClient->IsConnected() && !pClient->IsDisconnectSet());
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Socket.Client.WasConnect"))
			{
				ErrorLevel Result = ERROR_OK;

				CEngine *pEngine((CEngine *)this->pEngine);

				CSocketClient *pClient = (CSocketClient *)pEngine->Vars->GetPointer(pParams->ToVarIndex(0))->Value;

				return pReturn->Add(pClient->WasConnect());
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Socket.Client.IsDisconnectSet"))
			{
				ErrorLevel Result = ERROR_OK;

				CEngine *pEngine((CEngine *)this->pEngine);

				CSocketClient *pClient = (CSocketClient *)pEngine->Vars->GetPointer(pParams->ToVarIndex(0))->Value;

				return pReturn->Add(pClient->IsDisconnectSet());
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Socket.Client.PeerAddress"))
			{
				ErrorLevel Result = ERROR_OK;

				CEngine *pEngine((CEngine *)this->pEngine);

				CSocketClient *pClient = (CSocketClient *)pEngine->Vars->GetPointer(pParams->ToVarIndex(0))->Value;

				char sPeerAddress[255];
				int iLength = pClient->PeerAddress(sPeerAddress, sizeof(sPeerAddress));

				return pReturn->Add(sPeerAddress, iLength);
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Socket.Client.PeerID"))
			{
				ErrorLevel Result = ERROR_OK;

				CEngine *pEngine((CEngine *)this->pEngine);

				CSocketClient *pClient = (CSocketClient *)pEngine->Vars->GetPointer(pParams->ToVarIndex(0))->Value;

				return pReturn->Add(pClient->PeerID());
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Socket.Client.PeerPort"))
			{
				ErrorLevel Result = ERROR_OK;

				CEngine *pEngine((CEngine *)this->pEngine);

				CSocketClient *pClient = (CSocketClient *)pEngine->Vars->GetPointer((int)pParams->ToInt(0))->Value;

				return pReturn->Add(pClient->PeerPort());
			}
			//-----------------------------------------------------------------------------------------------------------------
		}
		//*********************************************************************************************************************
		//*********************************************************************************************************************
		//*********************************************************************************************************************
		else if (_strnicmp(pProc->Name, "date.", 5) == 0)
		{
			//-----------------------------------------------------------------------------------------------------------------
			if (_StrEqlI(pProc->Name, "Date.Now"))
			{
				PARAMITEM *pParam = &pParams->Collection.Items[1];

				char sDate[64];

				SYSTEMTIME ST;
				memset(&ST, 0, sizeof(ST));
				GetLocalTime(&ST);

				GetDateFormat(LOCALE_USER_DEFAULT, NULL, &ST, pParam->Value, sDate, sizeof(sDate));

				return pReturn->Add(sDate);
			}
			//-----------------------------------------------------------------------------------------------------------------
		}
		//*********************************************************************************************************************
		//*********************************************************************************************************************
		//*********************************************************************************************************************
		else if (_strnicmp(pProc->Name, "time.", 5) == 0)
		{
			//-----------------------------------------------------------------------------------------------------------------
			if (_StrEqlI(pProc->Name, "Time.Now"))
			{
				PARAMITEM *pParam = &pParams->Collection.Items[1];

				char sTime[64];

				SYSTEMTIME ST;
				memset(&ST, 0, sizeof(ST));
				GetLocalTime(&ST);

				GetTimeFormat(LOCALE_USER_DEFAULT, NULL, &ST, pParam->Value, sTime, sizeof(sTime));

				return pReturn->Add(sTime);
			}
			//-----------------------------------------------------------------------------------------------------------------
		}
		else {
			if (_StrEqlI(pProc->Name, "Return"))
			{
				if (pReturn && pReturn->Type != VAR_TYPE_INVALID)
				{
					if (pParams && pParams->Collection.Count > 0)
					{
						PARAMITEM *pParam = pParams->First();
						if (pParam)
						{
							if (pEngine->Types->GetPointer(pReturn->Type)->IsComplex)
							{
								int iComplexVarIndex = pParams->ToVarIndex(0);
								VARIABLEITEM *pComplexVar = pEngine->Vars->GetPointer(iComplexVarIndex);

								if (pReturn->Type != pComplexVar->Type)
								{
									TYPEITEM *expectedType = pEngine->Types->GetPointer(pReturn->Type);
									TYPEITEM *foundType = pEngine->Types->GetPointer(pComplexVar->Type);

									return this->Error->Hard("Incorrect return type. Found [%s], expected [%s].", foundType->Name, expectedType->Name);
								}

								//We want to make sure this variable doesnt get freed when it drops scope so that we can copy the pointers.
								pComplexVar->Reserved = true;
							}

							return pReturn->Add(pParam);
						}
					}
					return this->Error->HardCount();
				}
				else {
					return this->Error->Hard("Function cannot return a value");
				}
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Length"))
			{
				return pReturn->Add(pParams->First()->Length);
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Trim"))
			{
				StringBuilder builder(pParams->First()->Value, pParams->First()->Length);

				int iLength = pParams->ToInt(1);
				if (iLength < 0)
				{
					iLength = builder.Length;
				}

				builder.Trim();
				return pReturn->Add(builder.Buffer, iLength);
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "ToDouble"))
			{
				return pReturn->Add(dFormattedValue(pParams->Collection.Items[0].Value));
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "ToInteger"))
			{
				return pReturn->Add(iFormattedValue(pParams->Collection.Items[0].Value));
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "ASCII"))
			{
				return pReturn->Add((unsigned int)pParams->First()->Value[0]);
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "ToUpper"))
			{
				PARAMITEM *param = pParams->First();
				return pReturn->Add(UCase(param->Value, param->Length));
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "ToLower"))
			{
				PARAMITEM *param = pParams->First();
				return pReturn->Add(LCase(param->Value, param->Length));
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Char"))
			{
				unsigned char sChar[2];

				sChar[0] = (unsigned char)(int)pParams->ToInt(0);

				return pReturn->Add((char *)sChar, 1);
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Equals"))
			{
				bool bResult = false;

				if (pParams->Length(0) == pParams->Length(1))
				{
					bResult = strncmp(pParams->ToString(0), pParams->ToString(1), pParams->Length(0)) == 0;
				}

				return pReturn->Add(bResult);
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "NotEquals"))
			{
				bool bResult = false;

				if (pParams->Length(0) == pParams->Length(1))
				{
					bResult = strncmp(pParams->ToString(0), pParams->ToString(1), pParams->Length(0)) == 0;
				}

				return pReturn->Add(!bResult);
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "SubString"))
			{
				PARAMITEM *pString = pParams->First();

				int iStartIndex = atol(pParams->GetPointer(1)->Value);
				int iLength = 0;

				if (iStartIndex < 0)
				{
					return this->Error->Soft("SubString starting index must be equal or greater than zero");
				}

				if (iStartIndex > pString->Length)
				{
					return this->Error->Soft("SubString starting index must be equal or less than %d", pString->Length);
				}

				PARAMITEM *pLength = pParams->GetPointer(2);
				if (pLength->Value)
				{
					iLength = atol(pLength->Value);

					if (iLength > pString->Length - iStartIndex)
					{
						return this->Error->Soft("SubString length must be equal or less than %d", pString->Length - iStartIndex);
					}
				}
				else {
					iLength = pString->Length - iStartIndex;
				}

				if (iLength < 0)
				{
					return this->Error->Soft("SubString length index must be equal or greater than zero");
				}

				return pReturn->Add(pString->Value + iStartIndex, iLength);
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Base64Encode"))
			{
				PARAMITEM *pString = pParams->First();
				int iRequiredLength = abs(Base64Encode((unsigned char *)pString->Value, pString->Length, NULL, 0)) + 100;
				unsigned char *sTemp = (unsigned char *) pMem->Allocate(iRequiredLength, sizeof(unsigned char));
				int ilength = Base64Encode((unsigned char *)pString->Value, pString->Length, sTemp, iRequiredLength);
				ErrorLevel result = pReturn->Add((char *)sTemp, ilength);
				pMem->Free(sTemp);
				return result;
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Base64Decode"))
			{
				PARAMITEM *pString = pParams->First();
				unsigned char *sTemp = (unsigned char *) pMem->Allocate(pString->Length + 1, sizeof(unsigned char));
				int ilength = Base64Decode((unsigned char *)pString->Value, pString->Length, sTemp, pString->Length + 1);
				if(ilength < 0)
				{
					//pReturn->Add("", 0);
					pMem->Free(sTemp);
					return this->Error->Soft("The input string is not in the correct format.");
				}
				else 
				{
					ErrorLevel result = pReturn->Add((char *)sTemp, ilength);
					pMem->Free(sTemp);
					return result;
				}
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Reverse"))
			{
				PARAMITEM *pString = pParams->First();
				ReverseString(pString->Value, pString->Length);
				return pReturn->Add(pString->Value, pString->Length);
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Space"))
			{
				PARAMITEM *pParam = pParams->First();
				int iLength = atoi(pParam->Value);
				return pReturn->Add(Space(iLength), iLength);
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "DataLen"))
			{
				int iValType = pParams->AssessContentType(0);

				if (iValType == STRICT_TYPE_UINTEGER16)
				{
					return pReturn->Add((int)sizeof(unsigned short));
				}
				if (iValType == STRICT_TYPE_UINTEGER32)
				{
					return pReturn->Add((int)sizeof(unsigned int));
				}
				if (iValType == STRICT_TYPE_UINTEGER64)
				{
					return pReturn->Add((int)sizeof(unsigned __int64));
				}
				else if (iValType == STRICT_TYPE_INTEGER16)
				{
					return pReturn->Add((int)sizeof(short));
				}
				if (iValType == STRICT_TYPE_INTEGER32)
				{
					return pReturn->Add((int)sizeof(int));
				}
				if (iValType == STRICT_TYPE_INTEGER64)
				{
					return pReturn->Add((int)sizeof(__int64));
				}
				else if (iValType == STRICT_TYPE_FLOAT)
				{
					return pReturn->Add((int)sizeof(float));
				}
				else if (iValType == STRICT_TYPE_DOUBLE)
				{
					return pReturn->Add((int)sizeof(double));
				}
				else if (iValType == STRICT_TYPE_STRING)
				{
					return pReturn->Add(pParams->First()->Length);
				}
				else {
					this->Error->Soft("DataLen cannot be used on this datatype");
				}
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "SHA1"))
			{
				const char *sString = pParams->ToString(0);
				int iLength = (int)pParams->ToInt(1);
				if (iLength < 0)
				{
					iLength = (int)strlen(sString);
				}

				char sOut[128];
				if (!SimpleSHA1(sString, iLength, sOut, sizeof(sOut)))
				{
					this->Error->Soft("Failed to generate SHA1 hash");
				}

				return pReturn->Add(sOut);
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Checksum"))
			{
				const char *sString = pParams->ToString(0);
				int iLength = (int)pParams->ToInt(1);
				if (iLength < 0)
				{
					iLength = pParams->Length(0);
				}

				CRC32 crc32;

				unsigned int checksum = crc32.FullCRC((const unsigned char*)sString, iLength);

				return pReturn->Add(checksum);
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "GUID"))
			{
				char sGUID[33];
				if (!NewGUID(sGUID, sizeof(sGUID)))
				{
					this->Error->Soft("Failed to generate GUID");
				}
				return pReturn->Add(sGUID);
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "IIF"))
			{
				double logicValue = pParams->ToDouble(0);
				PARAMITEM *ifTrue = pParams->GetPointer(1);
				PARAMITEM *ifFalse = pParams->GetPointer(2);

				if(logicValue != 0)
				{
					return pReturn->Add(ifTrue);
				}
				else
				{
					return pReturn->Add(ifFalse);
				}
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Replace"))
			{
				PARAMITEM *pString = pParams->GetPointer(0);
				PARAMITEM *pToReplace = pParams->GetPointer(1);
				PARAMITEM *pReplaceWith = pParams->GetPointer(2);

				StringBuilder str(pString->Value, pString->Length);

				str.Replace(pToReplace->Value, pToReplace->Length, pReplaceWith->Value, pReplaceWith->Length, pParams->ToBool(3));

				str.Terminate();

				return pReturn->Add(str.Buffer, str.Length);
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "IndexOf"))
			{
				PARAMITEM *pStringToSearch = pParams->GetPointer(0);
				PARAMITEM *pStringToFind = pParams->GetPointer(1);
				int iStartPosition = (int)pParams->ToInt(3);

				if (pParams->ToBool(2))
				{
					return pReturn->Add(InStr(pStringToFind->Value, pStringToFind->Length, pStringToSearch->Value, pStringToSearch->Length, iStartPosition));
				}
				else {
					return pReturn->Add(InStrI(pStringToFind->Value, pStringToFind->Length, pStringToSearch->Value, pStringToSearch->Length, iStartPosition));
				}
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "FormatNumeric"))
			{
				char sNumber[128];
				FormatDouble(sNumber, sizeof(sNumber), pParams->ToDouble(0), (int)pParams->ToInt(1));
				return pReturn->Add(sNumber);
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Cast"))
			{
				PARAMITEM *pValue = pParams->First();
				int iType = pParams->ToInt(1);

				PARAMITEM pReturnValue;

				pReturnValue.Length = pValue->Length;
				pReturnValue.Value = (char *)pMem->Allocate(pReturnValue.Length, 1);

				int iCopyLength = pReturnValue.Length;
				if (pValue->Length < iCopyLength)
				{
					iCopyLength = pValue->Length;
				}

				memcpy_s(pReturnValue.Value, pReturnValue.Length, pValue->Value, iCopyLength);

				/*
				if (iCopyLength != pReturnValue.Length)
				{
					//TODO: Pad the remainder with 0s.
				}
				*/

				ErrorLevel result = pReturn->Add(&pReturnValue);

				pReturn->Type = iType;
				pReturn->EnforceTypes = true; //TODO: This is not being enforced.

				pMem->Free(pReturnValue.Value);

				return result;
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "FormatSize"))
			{
				char sNumber[128];
				if (FileSizeFriendly(pParams->ToDouble(0), (int)pParams->ToInt(1), sNumber, sizeof(sNumber)))
				{
					return pReturn->Add(sNumber);
				}
				return this->Error->Soft("Failed to format file size");
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "BinPrint"))
			{
				for (int iParam = 0; iParam < pParams->Collection.Count; iParam++)
				{
					if (pParams->Collection.Count > 1)
					{
						printf("[%d/%d] ", iParam + 1, pParams->Collection.Count);
					}
					printf("{\n");
					for (int iIndex = 0; iIndex < pParams->Collection.Items[iParam].Length; iIndex++)
					{
						printf("\t[%d]: 0x%x\n", iIndex, pParams->Collection.Items[iParam].Value[iIndex]);
					}
					printf("}\n");
				}
				return this->Error->HardCount();
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Print") || _StrEqlI(pProc->Name, "Prints"))
			{
				int iPrinted = 0;
				for (int iParam = 0; iParam < pParams->Collection.Count; iParam++)
				{
					iPrinted += printf("%s", pParams->Collection.Items[iParam].Value);
				}
				if (_StrEqlI(pProc->Name, "Print"))
				{
					iPrinted += printf("\n");
				}
				return pReturn->Add(iPrinted);
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Cls"))
			{
				system("cls");
				return this->Error->HardCount();
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Sleep"))
			{
				int iSleepTime = (int)pParams->ToInt(0);
				if (iSleepTime < 0)
				{
					return this->Error->Soft("Sleep time must be greater than zero");
				}
				Sleep(iSleepTime);
				return this->Error->HardCount();
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Pause"))
			{
				system("pause");
				return this->Error->HardCount();
			}
			//-----------------------------------------------------------------------------------------------------------------
			else if (_StrEqlI(pProc->Name, "Exit"))
			{
				exit((int)pParams->ToInt(0));
				return this->Error->HardCount();
			}
			//-----------------------------------------------------------------------------------------------------------------
#ifdef _DEBUG
			else if (_StrEqlI(pProc->Name, "GenerateDocumentation"))
			{
				return this->GenerateDocumentation(pParams->ToString(0));
			}
#endif //_DEBUG
		}
		//-----------------------------------------------------------------------------------------------------------------
#ifdef COM_PROCEDURE_CALL_IMPLEMENTATION
		//-----------------------------------------------------------------------------------------------------------------
		//Instantiates a COM object, very much so like Classic ASP.
	else if (_StrEqlI(pProc->Name, "COM.Object.CreateObject"))
	{
		HRESULT hResult;

		CLSID ClassID;
		//IClassFactory *pClf;
		//void *pVdb;

		if (!(hResult = SUCCEEDED(CoInitialize(NULL))))
		{
			return 1;
		}

		WCHAR sClassLibrary[255];
		size_t Converted = 0;
		mbstowcs_s(&Converted, sClassLibrary,
			sizeof(sClassLibrary) / sizeof(WCHAR),
			pParams->ToString(1), pParams->GetPointer(1)->Length);

		if (SUCCEEDED(hResult = CLSIDFromProgID(sClassLibrary, &ClassID)))
		{
			IDispatch *pDispatch;

			if (SUCCEEDED(hResult = CoCreateInstance(ClassID, NULL, CLSCTX_INPROC_SERVER, IID_IDispatch, (void **)&pDispatch)))
			{
				CEngine *pEngine((CEngine *)this->pEngine);
				int iVarIndex = pParams->ToVarIndex(0);
				VARIABLEITEM *pVar = pEngine->Vars->GetPointer(iVarIndex);
				if (pVar)
				{
					pVar->Value = (char *)pDispatch;
					pVar->Length = sizeof(IDispatch);

					return this->Error->HardCount();
				}
			}
		}

		return this->Error->Hard("CreateObject failed");
	}
#endif //COM_PROCEDURE_CALL_IMPLEMENTATION
	//-----------------------------------------------------------------------------------------------------------------

	return this->Error->Hard("Invalid system procedure [%s]", pProc->Name);
	}
	//*********************************************************************************************************************
#ifdef COM_PROCEDURE_CALL_IMPLEMENTATION
	else if (pProc->Type == PROC_TYPE_COM)
	{
		//-----------------------------------------------------------------------------------------------------------------
		if (_strnicmp(pProc->Name, "COM.Object.", 11) == 0)
		{
			return this->Error->Hard("Not implemented");

			int iVarIndex = pParams->ToVarIndex(0);
			VARIABLEITEM *pVar = pEngine->Vars->GetPointer(iVarIndex);

			IDispatch *pDispatch = (IDispatch *)pVar->Value;

			char *sMethod = pProc->Name + 11; //Strip off the "COM.Object." header.
			char **sMethods = Split(sMethod, "."); //Split the Method name, could be a subclass.

			for (int iIndex = 0; sMethod = sMethods[iIndex++];)
			{
				HRESULT hResult;

				int iMethodNameLength = strlen(sMethod);
				OLECHAR arryMember[255];
				memset(arryMember, 0, sizeof(arryMember));
				OLECHAR *sMember = arryMember;
				size_t Converted = 0;
				mbstowcs_s(&Converted, sMember,
					iMethodNameLength + 1,
					sMethod, iMethodNameLength);

				DISPID idFileExists;

				if (SUCCEEDED(hResult = pDispatch->GetIDsOfNames(IID_NULL, (OLECHAR **)&sMember, 1, LOCALE_SYSTEM_DEFAULT, &idFileExists)))
				{
					unsigned int puArgErr = 0;

					VARIANT VarResult;
					EXCEPINFO pExcepInfo;

					VariantInit(&VarResult);
					VariantInit(&VarResult);

					DISPPARAMS pParams;
					memset(&pParams, 0, sizeof(DISPPARAMS));
					pParams.cArgs = 1;

					VARIANT Arguments[1];
					VariantInit(&Arguments[0]);

					pParams.rgvarg = Arguments;
					pParams.cNamedArgs = 0;
					pParams.rgvarg[0].vt = VT_BSTR;
					pParams.rgvarg[0].bstrVal = SysAllocString(L"C:\\Test.txt");

					hResult = pDispatch->Invoke(
						idFileExists,
						IID_NULL,
						LOCALE_SYSTEM_DEFAULT,
						DISPATCH_METHOD,
						&pParams,
						&VarResult,
						&pExcepInfo,
						&puArgErr
						);

					SysFreeString(pParams.rgvarg[0].bstrVal);

					printf("File Exists? %d\n", abs(VarResult.boolVal));
				}
				else {
					//__debugbreak();
					char sOutErrorMessage[1024];
					if (pEngine->GetLastSystemError(hResult, sOutErrorMessage, sizeof(sOutErrorMessage)))
					{
						return this->Error->Soft(sOutErrorMessage);
					}
					else {
						return this->Error->Soft("Unknown error");
					}
				}
			}

			FreeArray(sMethods);

			return this->Error->HardCount();
		}
		//-----------------------------------------------------------------------------------------------------------------
		else {
			return this->Error->Hard("Invalid COM procedure [%s]", pProc->Name);
		}
	}
#endif //COM_PROCEDURE_CALL_IMPLEMENTATION
	//*********************************************************************************************************************
	else if (pProc->Type == PROC_TYPE_SCRIPT)
	{
		CEngine *pEngine = ((CEngine *)this->pEngine);
		int iVarIndex = INVALID_INDEX;

		int iReturnLineIndex;

		if (pEngine->Debug && pEngine->Debug->IsDebuggerAttached)
		{
			if (pEngine->Debug->IsBreakPoint(pProc->LineBegin))
			{
				iReturnLineIndex = pEngine->State->SetLineIndex(pProc->LineBegin);
				pEngine->Debug->Break();
				pEngine->State->SetLineIndex(pProc->LineBegin + 1);
			}
			else {
				iReturnLineIndex = pEngine->State->SetLineIndex(pProc->LineBegin + 1);
			}
		}
		else {
			iReturnLineIndex = pEngine->State->SetLineIndex(pProc->LineBegin + 1);
		}

		if (pParams)
		{
			for (int iParam = 0; iParam < pParams->Collection.Count; iParam++)
			{
				REQPARAMITEM *pReqParam = &pProc->ReqParams->Collection.Items[iParam];
				PARAMITEM *pParam = &pParams->Collection.Items[iParam];

				if (pEngine->Types->GetPointer(pReqParam->Type)->IsBuiltInClass || pReqParam->IsArray)
				{
					int iVarIndex = pParams->ToVarIndex(iParam);
					VARIABLEITEM *pVar = pEngine->Vars->GetPointer(iVarIndex);

					pEngine->Vars->AddNewStrongReference(pReqParam->Name, pParams->ToVarIndex(iParam));
				}
				else
				{
					if (pEngine->Vars->AddVariableParameterReference(pReqParam, pParam) != ERROR_OK)
					{
						pEngine->State->SetLineIndex(iReturnLineIndex);
						return this->Error->HardCount();
					}
				}
			}
		}

		ErrorLevel error = pEngine->Process(pReturn);
		if (error > ERROR_OK)
		{
			return error;
		}

		pEngine->State->SetLineIndex(iReturnLineIndex);

		return this->Error->HardCount();
	}
	//*********************************************************************************************************************
#ifdef EXTERNAL_PROCEDURE_CALL_IMPLEMENTATION
	else if (pProc->Type == PROC_TYPE_EXTERNAL)
	{
		ErrorLevel Result = ERROR_OK;
		void *pReturnValue = NULL;

		void **pvParams = NULL;
		int *piReferenceIndexes = NULL; //Holds the index of the original "by reference" variables.

		if (pParams->Collection.Count > 0)
		{
			pvParams = (void **) pMem->Allocate(pParams->Collection.Count, sizeof(void *));
			if (!pvParams)
			{
				return this->Error->Hard("Memory allocation error");
			}

			for (int iParam = 0; iParam < pParams->Collection.Count; iParam++)
			{
				REQPARAMITEM *pReqParam = &pProc->ReqParams->Collection.Items[iParam];

				if (pReqParam->StrictType == STRICT_TYPE_UINTEGER16)
				{
					if (pReqParam->ByReference)
					{
						VARIABLEITEM *pVar = (VARIABLEITEM *)pParams->ToVarIndexPointer(iParam);

						unsigned short iValue = (unsigned short)atoi(pVar->Value);
						pvParams[iParam] = (void *) pMem->Allocate(sizeof(iValue), 1);
						memcpy_s(pvParams[iParam], sizeof(unsigned short), &iValue, sizeof(iValue));
					}
					else {
						pvParams[iParam] = (void *)pParams->ToUInt(iParam);
					}
				}
				else if (pReqParam->StrictType == STRICT_TYPE_UINTEGER32)
				{
					if (pReqParam->ByReference)
					{
						VARIABLEITEM *pVar = (VARIABLEITEM *)pParams->ToVarIndexPointer(iParam);

						unsigned int iValue = (unsigned int)atoi(pVar->Value);
						pvParams[iParam] = (void *) pMem->Allocate(sizeof(iValue), 1);
						memcpy_s(pvParams[iParam], sizeof(unsigned int), &iValue, sizeof(iValue));
					}
					else {
						pvParams[iParam] = (void *)pParams->ToUInt(iParam);
					}
				}
				else if (pReqParam->StrictType == STRICT_TYPE_UINTEGER64)
				{
					if (pReqParam->ByReference)
					{
						VARIABLEITEM *pVar = (VARIABLEITEM *)pParams->ToVarIndexPointer(iParam);

						unsigned __int64 iValue = (unsigned __int64)atoi(pVar->Value);
						pvParams[iParam] = (void *) pMem->Allocate(sizeof(iValue), 1);
						memcpy_s(pvParams[iParam], sizeof(unsigned __int64), &iValue, sizeof(iValue));
					}
					else {
						pvParams[iParam] = (void *)pParams->ToUInt(iParam);
					}
				}

				else if (pReqParam->StrictType == STRICT_TYPE_INTEGER16)
				{
					if (pReqParam->ByReference)
					{
						VARIABLEITEM *pVar = (VARIABLEITEM *)pParams->ToVarIndexPointer(iParam);

						short iValue = (short)atoi(pVar->Value);
						pvParams[iParam] = (void *) pMem->Allocate(sizeof(iValue), 1);
						memcpy_s(pvParams[iParam], sizeof(short), &iValue, sizeof(iValue));
					}
					else {
						pvParams[iParam] = (void *)pParams->ToInt(iParam);
					}
				}
				else if (pReqParam->StrictType == STRICT_TYPE_INTEGER32)
				{
					if (pReqParam->ByReference)
					{
						VARIABLEITEM *pVar = (VARIABLEITEM *)pParams->ToVarIndexPointer(iParam);

						int iValue = (int)atoi(pVar->Value);
						pvParams[iParam] = (void *) pMem->Allocate(sizeof(iValue), 1);
						memcpy_s(pvParams[iParam], sizeof(int), &iValue, sizeof(iValue));
					}
					else {
						pvParams[iParam] = (void *)pParams->ToInt(iParam);
					}
				}
				else if (pReqParam->StrictType == STRICT_TYPE_INTEGER64)
				{
					if (pReqParam->ByReference)
					{
						VARIABLEITEM *pVar = (VARIABLEITEM *)pParams->ToVarIndexPointer(iParam);

						__int64 iValue = (__int64)atoi(pVar->Value);
						pvParams[iParam] = (void *) pMem->Allocate(sizeof(iValue), 1);
						memcpy_s(pvParams[iParam], sizeof(__int64), &iValue, sizeof(iValue));
					}
					else {
						pvParams[iParam] = (void *)pParams->ToInt(iParam);
					}
				}
				else if (pReqParam->StrictType == STRICT_TYPE_FLOAT)
				{
					return this->Error->Hard("Type not implemented for External Procedures");
					//float fValue = (float) pParams->ToDouble(iParam);
					//pvParams[iParam] = (void *) pMem->Allocate(sizeof(fValue), 1);
					//memcpy_s(pvParams[iParam], sizeof(float), &fValue, sizeof(fValue));
				}
				else if (pReqParam->StrictType == STRICT_TYPE_DOUBLE)
				{
					return this->Error->Hard("Type not implemented for External Procedures");
					//double dValue = (double) pParams->ToDouble(iParam);
					//pvParams[iParam] = (void *) pMem->Allocate(sizeof(dValue), 1);
					//memcpy_s(pvParams[iParam], sizeof(double), &dValue, sizeof(dValue));
				}
				else if (pReqParam->StrictType == STRICT_TYPE_STRING)
				{
					PARAMITEM *pParam = pParams->GetPointer(iParam);

					if (pReqParam->ByReference && pParam->Value[0] == '@')
					{
						VARIABLEITEM *pVar = (VARIABLEITEM *)pParams->ToVarIndexPointer(iParam);
						if (pVar)
						{
							if (pVar->Length > 0)
							{
								pvParams[iParam] = (void *) pMem->Allocate(sizeof(char), pVar->Length + 1);
								memcpy_s(pvParams[iParam], pVar->Length + 1, pVar->Value, pVar->Length);
							}
							else {
								pvParams[iParam] = NULL; //Set value to NULL;
							}
						}
						else {
							Result = this->Error->Hard("Invalid pointer for variable [%d]", pReqParam->Name);
							break;
						}
					}
					else {
						//if(pParam->Value[0] == '@')
						//{
						//	This error is mostly correct, but it could just be a string which begins with an @ symbol.
						//	Result = this->Error->Hard("Value [%d] must be passed by reference", pReqParam->Name);
						//	break;
						//}

						pvParams[iParam] = (void *) pMem->Allocate(sizeof(char), pParam->Length + 1);
						memcpy_s(pvParams[iParam], pParam->Length + 1, pParam->Value, pParam->Length);
					}
				}
				else if (pReqParam->StrictType == STRICT_TYPE_GENERIC)
				{
					TYPEITEM *pType = pEngine->Types->GetPointer(pReqParam->Type);

					if (pType->IsStrictType)
					{
						if (!pReqParam->ByReference)
						{
							Result = this->Error->Hard("Custom strict types [%s] must be passed by reference", pType->Name);
							break;
						}

						VARIABLEITEM *pVar = (VARIABLEITEM *)pParams->ToVarIndexPointer(iParam);
						pvParams[iParam] = pVar->Value;
					}
					else {
						Result = this->Error->Hard("Expected: Variable of [Strict Type] or [Custom Strict Type]");
						break;
					}
				}
				else {
					Result = this->Error->Hard("Type not implemented for External Procedures");
				}
			}
		}

		if (Result == ERROR_OK)
		{
			this->CallExternalProc(pProc, pParams, pvParams, &pReturnValue);

			if (pReturn && pReturn->Type != STRICT_TYPE_INVALID)
			{
				if (pProc->AssessedReturnType == STRICT_TYPE_UINTEGER16)
				{
					//This is an INT for a reason: because we want to store singed or unsigned shorts.
					pReturn->Add((unsigned int)pReturnValue);
				}
				else if (pProc->AssessedReturnType == STRICT_TYPE_UINTEGER32)
				{
					pReturn->Add((unsigned int)pReturnValue);
				}
				else if (pProc->AssessedReturnType == STRICT_TYPE_UINTEGER64)
				{
					Result = this->Error->Soft("Return type not implemented for External Procedures");
				}
				else if (pProc->AssessedReturnType == STRICT_TYPE_INTEGER16)
				{
					//This is an INT for a reason: because we want to store singed or unsigned shorts.
					pReturn->Add((int)pReturnValue);
				}
				else if (pProc->AssessedReturnType == STRICT_TYPE_INTEGER32)
				{
					pReturn->Add((int)pReturnValue);
				}
				else if (pProc->AssessedReturnType == STRICT_TYPE_INTEGER64)
				{
					Result = this->Error->Soft("Return type not implemented for External Procedures");
				}
				else if (pProc->AssessedReturnType == STRICT_TYPE_FLOAT)
				{
					Result = this->Error->Soft("Return type not implemented for External Procedures");
				}
				else if (pProc->AssessedReturnType == STRICT_TYPE_DOUBLE)
				{
					Result = this->Error->Soft("Return type not implemented for External Procedures");
				}
				else if (pProc->AssessedReturnType == STRICT_TYPE_STRING)
				{
					pReturn->Add((char *)pReturnValue);
				}
				else {
					Result = this->Error->Soft("Return type not implemented for External Procedures");
				}
			}
		}

		//Free parameter memory and copy-back reference varaible values.
		if (pParams->Collection.Count > 0)
		{
			for (int iParam = 0; iParam < pParams->Collection.Count; iParam++)
			{
				int iVarIndex;
				VARIABLEITEM *pVar = NULL;

				if (Result == ERROR_OK)
				{
					if (pProc->ReqParams->Collection.Items[iParam].ByReference || pProc->ReqParams->Collection.Items[iParam].StrictType == STRICT_TYPE_STRING)
					{
						//If passed by reference, set the original variable's value.
						if (pProc->ReqParams->Collection.Items[iParam].ByReference && pParams->Collection.Items[iParam].Value[0] == '@')
						{
							iVarIndex = pParams->ToVarIndex(iParam);
							pVar = pEngine->Vars->GetPointer(iVarIndex);

							switch (pProc->ReqParams->Collection.Items[iParam].StrictType)
							{
							case STRICT_TYPE_GENERIC:
								break; //Generic (custom defined) strict types are passed directly as pointers.
							case STRICT_TYPE_UINTEGER16:
								Result = pEngine->Vars->Set(iVarIndex, (unsigned short)*((unsigned short *)pvParams[iParam]));
								break;
							case STRICT_TYPE_UINTEGER32:
								Result = pEngine->Vars->Set(iVarIndex, (unsigned int)*((unsigned int *)pvParams[iParam]));
								break;
								//case STRICT_TYPE_UINTEGER64:
								//	Result = pEngine->Vars->Set(iVarIndex, (unsigned __int64)pvParams[iParam]);
								//	break;
							case STRICT_TYPE_INTEGER16:
								Result = pEngine->Vars->Set(iVarIndex, (short)*((short *)pvParams[iParam]));
								break;
							case STRICT_TYPE_INTEGER32:
								Result = pEngine->Vars->Set(iVarIndex, (int)*((int *)pvParams[iParam]));
								break;
								//case STRICT_TYPE_INTEGER64:
								//	Result = pEngine->Vars->Set(iVarIndex, (__int64)pvParams[iParam]);
								//	break;
							case STRICT_TYPE_FLOAT:
								Result = pEngine->Vars->Set(iVarIndex, (float)*((float *)pvParams[iParam]));
								break;
							case STRICT_TYPE_DOUBLE:
								Result = pEngine->Vars->Set(iVarIndex, (double)*((double *)pvParams[iParam]));
								break;
							case STRICT_TYPE_STRING:
								Result = pEngine->Vars->Set(iVarIndex, (char *)pvParams[iParam]);
								break;
							default:
								Result = this->Error->Hard("By reference type not implemented for External Procedures");
								break;
							};
						}
					}

					//pEngine->Vars->Set(iVarIndex, (char *)pvParams[iParam]);
					if (pvParams[iParam] &&
						(
							(
								pProc->ReqParams->Collection.Items[iParam].ByReference
								&& pProc->ReqParams->Collection.Items[iParam].StrictType != STRICT_TYPE_GENERIC
								)
							|| pProc->ReqParams->Collection.Items[iParam].StrictType == STRICT_TYPE_STRING)
						)
					{
						pMem->Free(pvParams[iParam]);
						pvParams[iParam] = NULL;
					}
				}
			}

			pMem->Free(pvParams);
		}

		return this->Error->HardCount();
	}
#endif //EXTERNAL_PROCEDURE_CALL_IMPLEMENTATION
	//*********************************************************************************************************************
	else {
		return this->Error->Hard("Invalid procedure type [%d]", pProc->Type);
	}
	//*********************************************************************************************************************

	return this->Error->Hard("Unknown procedure error [%s]", pProc->Name);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef EXTERNAL_PROCEDURE_CALL_IMPLEMENTATION
	ErrorLevel CProcs::CallExternalProc(PROCITEM *pProc, CParams *pParams, void **pvParams, void **pReturnValue)
	{
		CEngine *pEngine = ((CEngine *)this->pEngine);
		ErrorLevel Result = ERROR_OK;
		HINSTANCE hLibrary = NULL;

		if ((hLibrary = LoadLibrary((LPCTSTR)pProc->Library)) == NULL)
		{
			return this->Error->Soft("Could not load external API library");
		}

		if (pProc->CallingConvention == EXTERNAL_TYPE_STDCALL)
		{
			if (pParams->Collection.Count == 0)
			{
				StdProc0 Proc;
				if ((Proc = (StdProc0)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc();
			}
			else if (pParams->Collection.Count == 1)
			{
				StdProc1 Proc;
				if ((Proc = (StdProc1)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0]);
			}
			else if (pParams->Collection.Count == 2)
			{
				StdProc2 Proc;
				if ((Proc = (StdProc2)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0], pvParams[1]);
			}
			else if (pParams->Collection.Count == 3)
			{
				StdProc3 Proc;
				if ((Proc = (StdProc3)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0], pvParams[1], pvParams[2]);
			}
			else if (pParams->Collection.Count == 4)
			{
				StdProc4 Proc;
				if ((Proc = (StdProc4)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0], pvParams[1], pvParams[2], pvParams[3]);
			}
			else if (pParams->Collection.Count == 5)
			{
				StdProc5 Proc;
				if ((Proc = (StdProc5)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0], pvParams[1], pvParams[2], pvParams[3], pvParams[4]);
			}
			else if (pParams->Collection.Count == 6)
			{
				StdProc6 Proc;
				if ((Proc = (StdProc6)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0], pvParams[1], pvParams[2], pvParams[3], pvParams[4],
					pvParams[5]);
			}
			else if (pParams->Collection.Count == 7)
			{
				//"C:\\Test.txt", FileAccess.Write, 0, NULL, CREATE_ALWAYS, 0, NULL

				StdProc7 Proc;
				if ((Proc = (StdProc7)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0], pvParams[1], pvParams[2], pvParams[3], pvParams[4],
					pvParams[5], pvParams[6]);
			}
			else if (pParams->Collection.Count == 8)
			{
				StdProc8 Proc;
				if ((Proc = (StdProc8)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0], pvParams[1], pvParams[2], pvParams[3], pvParams[4],
					pvParams[5], pvParams[6], pvParams[7]);
			}
			else if (pParams->Collection.Count == 9)
			{
				StdProc9 Proc;
				if ((Proc = (StdProc9)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0], pvParams[1], pvParams[2], pvParams[3], pvParams[4],
					pvParams[5], pvParams[6], pvParams[7], pvParams[8]);
			}
			else if (pParams->Collection.Count == 10)
			{
				StdProc10 Proc;
				if ((Proc = (StdProc10)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0], pvParams[1], pvParams[2], pvParams[3], pvParams[4],
					pvParams[5], pvParams[6], pvParams[7], pvParams[8], pvParams[9]);
			}
			else if (pParams->Collection.Count == 11)
			{
				StdProc11 Proc;
				if ((Proc = (StdProc11)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0], pvParams[1], pvParams[2], pvParams[3], pvParams[4],
					pvParams[5], pvParams[6], pvParams[7], pvParams[8], pvParams[9], pvParams[10]);
			}
			else if (pParams->Collection.Count == 12)
			{
				StdProc12 Proc;
				if ((Proc = (StdProc12)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0], pvParams[1], pvParams[2], pvParams[3], pvParams[4],
					pvParams[5], pvParams[6], pvParams[7], pvParams[8], pvParams[9], pvParams[10], pvParams[11]);
			}
			else if (pParams->Collection.Count == 13)
			{
				StdProc13 Proc;
				if ((Proc = (StdProc13)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0], pvParams[1], pvParams[2], pvParams[3], pvParams[4],
					pvParams[5], pvParams[6], pvParams[7], pvParams[8], pvParams[9], pvParams[10], pvParams[11],
					pvParams[12]);
			}
			else if (pParams->Collection.Count == 14)
			{
				StdProc14 Proc;
				if ((Proc = (StdProc14)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0], pvParams[1], pvParams[2], pvParams[3], pvParams[4],
					pvParams[5], pvParams[6], pvParams[7], pvParams[8], pvParams[9], pvParams[10], pvParams[11],
					pvParams[12], pvParams[13]);
			}
			else if (pParams->Collection.Count == 15)
			{
				StdProc15 Proc;
				if ((Proc = (StdProc15)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0], pvParams[1], pvParams[2], pvParams[3], pvParams[4],
					pvParams[5], pvParams[6], pvParams[7], pvParams[8], pvParams[9], pvParams[10], pvParams[11],
					pvParams[12], pvParams[13], pvParams[14]);
			}
			else if (pParams->Collection.Count == 16)
			{
				StdProc16 Proc;
				if ((Proc = (StdProc16)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0], pvParams[1], pvParams[2], pvParams[3], pvParams[4],
					pvParams[5], pvParams[6], pvParams[7], pvParams[8], pvParams[9], pvParams[10], pvParams[11],
					pvParams[12], pvParams[13], pvParams[14], pvParams[15]);
			}
			else if (pParams->Collection.Count == 17)
			{
				StdProc17 Proc;
				if ((Proc = (StdProc17)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0], pvParams[1], pvParams[2], pvParams[3], pvParams[4],
					pvParams[5], pvParams[6], pvParams[7], pvParams[8], pvParams[9], pvParams[10], pvParams[11],
					pvParams[12], pvParams[13], pvParams[14], pvParams[15], pvParams[16]);
			}
			else if (pParams->Collection.Count == 18)
			{
				StdProc18 Proc;
				if ((Proc = (StdProc18)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0], pvParams[1], pvParams[2], pvParams[3], pvParams[4],
					pvParams[5], pvParams[6], pvParams[7], pvParams[8], pvParams[9], pvParams[10], pvParams[11],
					pvParams[12], pvParams[13], pvParams[14], pvParams[15], pvParams[16], pvParams[17]);
			}
			else if (pParams->Collection.Count == 19)
			{
				StdProc19 Proc;
				if ((Proc = (StdProc19)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0], pvParams[1], pvParams[2], pvParams[3], pvParams[4],
					pvParams[5], pvParams[6], pvParams[7], pvParams[8], pvParams[9], pvParams[10], pvParams[11],
					pvParams[12], pvParams[13], pvParams[14], pvParams[15], pvParams[16], pvParams[17], pvParams[18]);
			}
			else if (pParams->Collection.Count == 20)
			{
				StdProc20 Proc;
				if ((Proc = (StdProc20)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0], pvParams[1], pvParams[2], pvParams[3], pvParams[4],
					pvParams[5], pvParams[6], pvParams[7], pvParams[8], pvParams[9], pvParams[10], pvParams[11],
					pvParams[12], pvParams[13], pvParams[14], pvParams[15], pvParams[16], pvParams[17], pvParams[18],
					pvParams[19]);
			}
			else if (pParams->Collection.Count == 21)
			{
				StdProc21 Proc;
				if ((Proc = (StdProc21)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0], pvParams[1], pvParams[2], pvParams[3], pvParams[4],
					pvParams[5], pvParams[6], pvParams[7], pvParams[8], pvParams[9], pvParams[10], pvParams[11],
					pvParams[12], pvParams[13], pvParams[14], pvParams[15], pvParams[16], pvParams[17], pvParams[18],
					pvParams[19], pvParams[20]);
			}
			else if (pParams->Collection.Count == 22)
			{
				StdProc22 Proc;
				if ((Proc = (StdProc22)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0], pvParams[1], pvParams[2], pvParams[3], pvParams[4],
					pvParams[5], pvParams[6], pvParams[7], pvParams[8], pvParams[9], pvParams[10], pvParams[11],
					pvParams[12], pvParams[13], pvParams[14], pvParams[15], pvParams[16], pvParams[17], pvParams[18],
					pvParams[19], pvParams[20], pvParams[21]);
			}
			else if (pParams->Collection.Count == 23)
			{
				StdProc23 Proc;
				if ((Proc = (StdProc23)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0], pvParams[1], pvParams[2], pvParams[3], pvParams[4],
					pvParams[5], pvParams[6], pvParams[7], pvParams[8], pvParams[9], pvParams[10], pvParams[11],
					pvParams[12], pvParams[13], pvParams[14], pvParams[15], pvParams[16], pvParams[17], pvParams[18],
					pvParams[19], pvParams[20], pvParams[21], pvParams[22]);
			}
			else if (pParams->Collection.Count == 24)
			{
				StdProc24 Proc;
				if ((Proc = (StdProc24)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0], pvParams[1], pvParams[2], pvParams[3], pvParams[4],
					pvParams[5], pvParams[6], pvParams[7], pvParams[8], pvParams[9], pvParams[10], pvParams[11],
					pvParams[12], pvParams[13], pvParams[14], pvParams[15], pvParams[16], pvParams[17], pvParams[18],
					pvParams[19], pvParams[20], pvParams[21], pvParams[22], pvParams[23]);
			}
			else if (pParams->Collection.Count == 25)
			{
				StdProc25 Proc;
				if ((Proc = (StdProc25)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0], pvParams[1], pvParams[2], pvParams[3], pvParams[4],
					pvParams[5], pvParams[6], pvParams[7], pvParams[8], pvParams[9], pvParams[10], pvParams[11],
					pvParams[12], pvParams[13], pvParams[14], pvParams[15], pvParams[16], pvParams[17], pvParams[18],
					pvParams[19], pvParams[20], pvParams[21], pvParams[22], pvParams[23], pvParams[24]);
			}
			else {
				Result = this->Error->Soft(">25 parameters not Implemented");
			}
		}
		//----------------------------------------------------------------------------------------------------------------
		else if (pProc->CallingConvention == EXTERNAL_TYPE_CDECL)
		{
			if (pParams->Collection.Count == 0)
			{
				CdeclProc0 Proc;
				if ((Proc = (CdeclProc0)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc();
			}
			else if (pParams->Collection.Count == 1)
			{
				CdeclProc1 Proc;
				if ((Proc = (CdeclProc1)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0]);
			}
			else if (pParams->Collection.Count == 2)
			{
				CdeclProc2 Proc;
				if ((Proc = (CdeclProc2)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0], pvParams[1]);
			}
			else if (pParams->Collection.Count == 3)
			{
				CdeclProc3 Proc;
				if ((Proc = (CdeclProc3)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0], pvParams[1], pvParams[2]);
			}
			else if (pParams->Collection.Count == 4)
			{
				CdeclProc4 Proc;
				if ((Proc = (CdeclProc4)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0], pvParams[1], pvParams[2], pvParams[3]);
			}
			else if (pParams->Collection.Count == 5)
			{
				CdeclProc5 Proc;
				if ((Proc = (CdeclProc5)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0], pvParams[1], pvParams[2], pvParams[3], pvParams[4]);
			}
			else if (pParams->Collection.Count == 6)
			{
				CdeclProc6 Proc;
				if ((Proc = (CdeclProc6)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0], pvParams[1], pvParams[2], pvParams[3], pvParams[4],
					pvParams[5]);
			}
			else if (pParams->Collection.Count == 7)
			{
				CdeclProc7 Proc;
				if ((Proc = (CdeclProc7)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0], pvParams[1], pvParams[2], pvParams[3], pvParams[4],
					pvParams[5], pvParams[6]);
			}
			else if (pParams->Collection.Count == 8)
			{
				CdeclProc8 Proc;
				if ((Proc = (CdeclProc8)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0], pvParams[1], pvParams[2], pvParams[3], pvParams[4],
					pvParams[5], pvParams[6], pvParams[7]);
			}
			else if (pParams->Collection.Count == 9)
			{
				CdeclProc9 Proc;
				if ((Proc = (CdeclProc9)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0], pvParams[1], pvParams[2], pvParams[3], pvParams[4],
					pvParams[5], pvParams[6], pvParams[7], pvParams[8]);
			}
			else if (pParams->Collection.Count == 10)
			{
				CdeclProc10 Proc;
				if ((Proc = (CdeclProc10)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0], pvParams[1], pvParams[2], pvParams[3], pvParams[4],
					pvParams[5], pvParams[6], pvParams[7], pvParams[8], pvParams[9]);
			}
			else if (pParams->Collection.Count == 11)
			{
				CdeclProc11 Proc;
				if ((Proc = (CdeclProc11)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0], pvParams[1], pvParams[2], pvParams[3], pvParams[4],
					pvParams[5], pvParams[6], pvParams[7], pvParams[8], pvParams[9], pvParams[10]);
			}
			else if (pParams->Collection.Count == 12)
			{
				CdeclProc12 Proc;
				if ((Proc = (CdeclProc12)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0], pvParams[1], pvParams[2], pvParams[3], pvParams[4],
					pvParams[5], pvParams[6], pvParams[7], pvParams[8], pvParams[9], pvParams[10], pvParams[11]);
			}
			else if (pParams->Collection.Count == 13)
			{
				CdeclProc13 Proc;
				if ((Proc = (CdeclProc13)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0], pvParams[1], pvParams[2], pvParams[3], pvParams[4],
					pvParams[5], pvParams[6], pvParams[7], pvParams[8], pvParams[9], pvParams[10], pvParams[11],
					pvParams[12]);
			}
			else if (pParams->Collection.Count == 14)
			{
				CdeclProc14 Proc;
				if ((Proc = (CdeclProc14)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0], pvParams[1], pvParams[2], pvParams[3], pvParams[4],
					pvParams[5], pvParams[6], pvParams[7], pvParams[8], pvParams[9], pvParams[10], pvParams[11],
					pvParams[12], pvParams[13]);
			}
			else if (pParams->Collection.Count == 15)
			{
				CdeclProc15 Proc;
				if ((Proc = (CdeclProc15)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0], pvParams[1], pvParams[2], pvParams[3], pvParams[4],
					pvParams[5], pvParams[6], pvParams[7], pvParams[8], pvParams[9], pvParams[10], pvParams[11],
					pvParams[12], pvParams[13], pvParams[14]);
			}
			else if (pParams->Collection.Count == 16)
			{
				CdeclProc16 Proc;
				if ((Proc = (CdeclProc16)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0], pvParams[1], pvParams[2], pvParams[3], pvParams[4],
					pvParams[5], pvParams[6], pvParams[7], pvParams[8], pvParams[9], pvParams[10], pvParams[11],
					pvParams[12], pvParams[13], pvParams[14], pvParams[15]);
			}
			else if (pParams->Collection.Count == 17)
			{
				CdeclProc17 Proc;
				if ((Proc = (CdeclProc17)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0], pvParams[1], pvParams[2], pvParams[3], pvParams[4],
					pvParams[5], pvParams[6], pvParams[7], pvParams[8], pvParams[9], pvParams[10], pvParams[11],
					pvParams[12], pvParams[13], pvParams[14], pvParams[15], pvParams[16]);
			}
			else if (pParams->Collection.Count == 18)
			{
				CdeclProc18 Proc;
				if ((Proc = (CdeclProc18)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0], pvParams[1], pvParams[2], pvParams[3], pvParams[4],
					pvParams[5], pvParams[6], pvParams[7], pvParams[8], pvParams[9], pvParams[10], pvParams[11],
					pvParams[12], pvParams[13], pvParams[14], pvParams[15], pvParams[16], pvParams[17]);
			}
			else if (pParams->Collection.Count == 19)
			{
				CdeclProc19 Proc;
				if ((Proc = (CdeclProc19)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0], pvParams[1], pvParams[2], pvParams[3], pvParams[4],
					pvParams[5], pvParams[6], pvParams[7], pvParams[8], pvParams[9], pvParams[10], pvParams[11],
					pvParams[12], pvParams[13], pvParams[14], pvParams[15], pvParams[16], pvParams[17], pvParams[18]);
			}
			else if (pParams->Collection.Count == 20)
			{
				CdeclProc20 Proc;
				if ((Proc = (CdeclProc20)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0], pvParams[1], pvParams[2], pvParams[3], pvParams[4],
					pvParams[5], pvParams[6], pvParams[7], pvParams[8], pvParams[9], pvParams[10], pvParams[11],
					pvParams[12], pvParams[13], pvParams[14], pvParams[15], pvParams[16], pvParams[17], pvParams[18],
					pvParams[19]);
			}
			else if (pParams->Collection.Count == 21)
			{
				CdeclProc21 Proc;
				if ((Proc = (CdeclProc21)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0], pvParams[1], pvParams[2], pvParams[3], pvParams[4],
					pvParams[5], pvParams[6], pvParams[7], pvParams[8], pvParams[9], pvParams[10], pvParams[11],
					pvParams[12], pvParams[13], pvParams[14], pvParams[15], pvParams[16], pvParams[17], pvParams[18],
					pvParams[19], pvParams[20]);
			}
			else if (pParams->Collection.Count == 22)
			{
				CdeclProc22 Proc;
				if ((Proc = (CdeclProc22)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0], pvParams[1], pvParams[2], pvParams[3], pvParams[4],
					pvParams[5], pvParams[6], pvParams[7], pvParams[8], pvParams[9], pvParams[10], pvParams[11],
					pvParams[12], pvParams[13], pvParams[14], pvParams[15], pvParams[16], pvParams[17], pvParams[18],
					pvParams[19], pvParams[20], pvParams[21]);
			}
			else if (pParams->Collection.Count == 23)
			{
				CdeclProc23 Proc;
				if ((Proc = (CdeclProc23)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0], pvParams[1], pvParams[2], pvParams[3], pvParams[4],
					pvParams[5], pvParams[6], pvParams[7], pvParams[8], pvParams[9], pvParams[10], pvParams[11],
					pvParams[12], pvParams[13], pvParams[14], pvParams[15], pvParams[16], pvParams[17], pvParams[18],
					pvParams[19], pvParams[20], pvParams[21], pvParams[22]);
			}
			else if (pParams->Collection.Count == 24)
			{
				CdeclProc24 Proc;
				if ((Proc = (CdeclProc24)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0], pvParams[1], pvParams[2], pvParams[3], pvParams[4],
					pvParams[5], pvParams[6], pvParams[7], pvParams[8], pvParams[9], pvParams[10], pvParams[11],
					pvParams[12], pvParams[13], pvParams[14], pvParams[15], pvParams[16], pvParams[17], pvParams[18],
					pvParams[19], pvParams[20], pvParams[21], pvParams[22], pvParams[23]);
			}
			else if (pParams->Collection.Count == 25)
			{
				CdeclProc25 Proc;
				if ((Proc = (CdeclProc25)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0], pvParams[1], pvParams[2], pvParams[3], pvParams[4],
					pvParams[5], pvParams[6], pvParams[7], pvParams[8], pvParams[9], pvParams[10], pvParams[11],
					pvParams[12], pvParams[13], pvParams[14], pvParams[15], pvParams[16], pvParams[17], pvParams[18],
					pvParams[19], pvParams[20], pvParams[21], pvParams[22], pvParams[23], pvParams[24]);
			}
			else {
				Result = this->Error->Hard(">25 parameters not Implemented");
			}
		}
		//----------------------------------------------------------------------------------------------------------------
		else if (pProc->CallingConvention == EXTERNAL_TYPE_FASTCALL)
		{
			if (pParams->Collection.Count == 0)
			{
				FastProc0 Proc;
				if ((Proc = (FastProc0)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc();
			}
			else if (pParams->Collection.Count == 1)
			{
				FastProc1 Proc;
				if ((Proc = (FastProc1)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0]);
			}
			else if (pParams->Collection.Count == 2)
			{
				FastProc2 Proc;
				if ((Proc = (FastProc2)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0], pvParams[1]);
			}
			else if (pParams->Collection.Count == 3)
			{
				FastProc3 Proc;
				if ((Proc = (FastProc3)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0], pvParams[1], pvParams[2]);
			}
			else if (pParams->Collection.Count == 4)
			{
				FastProc4 Proc;
				if ((Proc = (FastProc4)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0], pvParams[1], pvParams[2], pvParams[3]);
			}
			else if (pParams->Collection.Count == 5)
			{
				FastProc5 Proc;
				if ((Proc = (FastProc5)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0], pvParams[1], pvParams[2], pvParams[3], pvParams[4]);
			}
			else if (pParams->Collection.Count == 6)
			{
				FastProc6 Proc;
				if ((Proc = (FastProc6)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0], pvParams[1], pvParams[2], pvParams[3], pvParams[4],
					pvParams[5]);
			}
			else if (pParams->Collection.Count == 7)
			{
				FastProc7 Proc;
				if ((Proc = (FastProc7)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0], pvParams[1], pvParams[2], pvParams[3], pvParams[4],
					pvParams[5], pvParams[6]);
			}
			else if (pParams->Collection.Count == 8)
			{
				FastProc8 Proc;
				if ((Proc = (FastProc8)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0], pvParams[1], pvParams[2], pvParams[3], pvParams[4],
					pvParams[5], pvParams[6], pvParams[7]);
			}
			else if (pParams->Collection.Count == 9)
			{
				FastProc9 Proc;
				if ((Proc = (FastProc9)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0], pvParams[1], pvParams[2], pvParams[3], pvParams[4],
					pvParams[5], pvParams[6], pvParams[7], pvParams[8]);
			}
			else if (pParams->Collection.Count == 10)
			{
				FastProc10 Proc;
				if ((Proc = (FastProc10)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0], pvParams[1], pvParams[2], pvParams[3], pvParams[4],
					pvParams[5], pvParams[6], pvParams[7], pvParams[8], pvParams[9]);
			}
			else if (pParams->Collection.Count == 11)
			{
				FastProc11 Proc;
				if ((Proc = (FastProc11)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0], pvParams[1], pvParams[2], pvParams[3], pvParams[4],
					pvParams[5], pvParams[6], pvParams[7], pvParams[8], pvParams[9], pvParams[10]);
			}
			else if (pParams->Collection.Count == 12)
			{
				FastProc12 Proc;
				if ((Proc = (FastProc12)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0], pvParams[1], pvParams[2], pvParams[3], pvParams[4],
					pvParams[5], pvParams[6], pvParams[7], pvParams[8], pvParams[9], pvParams[10], pvParams[11]);
			}
			else if (pParams->Collection.Count == 13)
			{
				FastProc13 Proc;
				if ((Proc = (FastProc13)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0], pvParams[1], pvParams[2], pvParams[3], pvParams[4],
					pvParams[5], pvParams[6], pvParams[7], pvParams[8], pvParams[9], pvParams[10], pvParams[11],
					pvParams[12]);
			}
			else if (pParams->Collection.Count == 14)
			{
				FastProc14 Proc;
				if ((Proc = (FastProc14)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0], pvParams[1], pvParams[2], pvParams[3], pvParams[4],
					pvParams[5], pvParams[6], pvParams[7], pvParams[8], pvParams[9], pvParams[10], pvParams[11],
					pvParams[12], pvParams[13]);
			}
			else if (pParams->Collection.Count == 15)
			{
				FastProc15 Proc;
				if ((Proc = (FastProc15)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0], pvParams[1], pvParams[2], pvParams[3], pvParams[4],
					pvParams[5], pvParams[6], pvParams[7], pvParams[8], pvParams[9], pvParams[10], pvParams[11],
					pvParams[12], pvParams[13], pvParams[14]);
			}
			else if (pParams->Collection.Count == 16)
			{
				FastProc16 Proc;
				if ((Proc = (FastProc16)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0], pvParams[1], pvParams[2], pvParams[3], pvParams[4],
					pvParams[5], pvParams[6], pvParams[7], pvParams[8], pvParams[9], pvParams[10], pvParams[11],
					pvParams[12], pvParams[13], pvParams[14], pvParams[15]);
			}
			else if (pParams->Collection.Count == 17)
			{
				FastProc17 Proc;
				if ((Proc = (FastProc17)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0], pvParams[1], pvParams[2], pvParams[3], pvParams[4],
					pvParams[5], pvParams[6], pvParams[7], pvParams[8], pvParams[9], pvParams[10], pvParams[11],
					pvParams[12], pvParams[13], pvParams[14], pvParams[15], pvParams[16]);
			}
			else if (pParams->Collection.Count == 18)
			{
				FastProc18 Proc;
				if ((Proc = (FastProc18)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0], pvParams[1], pvParams[2], pvParams[3], pvParams[4],
					pvParams[5], pvParams[6], pvParams[7], pvParams[8], pvParams[9], pvParams[10], pvParams[11],
					pvParams[12], pvParams[13], pvParams[14], pvParams[15], pvParams[16], pvParams[17]);
			}
			else if (pParams->Collection.Count == 19)
			{
				FastProc19 Proc;
				if ((Proc = (FastProc19)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0], pvParams[1], pvParams[2], pvParams[3], pvParams[4],
					pvParams[5], pvParams[6], pvParams[7], pvParams[8], pvParams[9], pvParams[10], pvParams[11],
					pvParams[12], pvParams[13], pvParams[14], pvParams[15], pvParams[16], pvParams[17], pvParams[18]);
			}
			else if (pParams->Collection.Count == 20)
			{
				FastProc20 Proc;
				if ((Proc = (FastProc20)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0], pvParams[1], pvParams[2], pvParams[3], pvParams[4],
					pvParams[5], pvParams[6], pvParams[7], pvParams[8], pvParams[9], pvParams[10], pvParams[11],
					pvParams[12], pvParams[13], pvParams[14], pvParams[15], pvParams[16], pvParams[17], pvParams[18],
					pvParams[19]);
			}
			else if (pParams->Collection.Count == 21)
			{
				FastProc21 Proc;
				if ((Proc = (FastProc21)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0], pvParams[1], pvParams[2], pvParams[3], pvParams[4],
					pvParams[5], pvParams[6], pvParams[7], pvParams[8], pvParams[9], pvParams[10], pvParams[11],
					pvParams[12], pvParams[13], pvParams[14], pvParams[15], pvParams[16], pvParams[17], pvParams[18],
					pvParams[19], pvParams[20]);
			}
			else if (pParams->Collection.Count == 22)
			{
				FastProc22 Proc;
				if ((Proc = (FastProc22)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0], pvParams[1], pvParams[2], pvParams[3], pvParams[4],
					pvParams[5], pvParams[6], pvParams[7], pvParams[8], pvParams[9], pvParams[10], pvParams[11],
					pvParams[12], pvParams[13], pvParams[14], pvParams[15], pvParams[16], pvParams[17], pvParams[18],
					pvParams[19], pvParams[20], pvParams[21]);
			}
			else if (pParams->Collection.Count == 23)
			{
				FastProc23 Proc;
				if ((Proc = (FastProc23)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0], pvParams[1], pvParams[2], pvParams[3], pvParams[4],
					pvParams[5], pvParams[6], pvParams[7], pvParams[8], pvParams[9], pvParams[10], pvParams[11],
					pvParams[12], pvParams[13], pvParams[14], pvParams[15], pvParams[16], pvParams[17], pvParams[18],
					pvParams[19], pvParams[20], pvParams[21], pvParams[22]);
			}
			else if (pParams->Collection.Count == 24)
			{
				FastProc24 Proc;
				if ((Proc = (FastProc24)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0], pvParams[1], pvParams[2], pvParams[3], pvParams[4],
					pvParams[5], pvParams[6], pvParams[7], pvParams[8], pvParams[9], pvParams[10], pvParams[11],
					pvParams[12], pvParams[13], pvParams[14], pvParams[15], pvParams[16], pvParams[17], pvParams[18],
					pvParams[19], pvParams[20], pvParams[21], pvParams[22], pvParams[23]);
			}
			else if (pParams->Collection.Count == 25)
			{
				FastProc25 Proc;
				if ((Proc = (FastProc25)GetProcAddress(hLibrary, pProc->Alias)) == NULL)
					Result = this->Error->Soft("Failed to locate %s in %s", pProc->Alias, pProc->Name);
				else *pReturnValue = (void *)Proc(pvParams[0], pvParams[1], pvParams[2], pvParams[3], pvParams[4],
					pvParams[5], pvParams[6], pvParams[7], pvParams[8], pvParams[9], pvParams[10], pvParams[11],
					pvParams[12], pvParams[13], pvParams[14], pvParams[15], pvParams[16], pvParams[17], pvParams[18],
					pvParams[19], pvParams[20], pvParams[21], pvParams[22], pvParams[23], pvParams[24]);
			}
			else {
				Result = this->Error->Hard(">25 parameters not Implemented");
			}
		}

		FreeLibrary(hLibrary);

		return Result;
	}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
