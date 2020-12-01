#ifndef _CEngine_CPP_
#define _CEngine_CPP_
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define _CRT_SECURE_NO_WARNINGS //FIXFIX: Replace getenv() with secure counterpart.

#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>
#include <Limits.H>

#include "CEngine.H"
#include "../../../../NSWFL/NSWFL.h"
#include "../../../../CMathParser/CMathParser.H"
#include "../../../../CSQLBinding/CSQLEncapsulation.H"

#ifdef _USE_GLOBAL_MEMPOOL
NSWFL::Memory::MemoryPool *pMem = new NSWFL::Memory::MemoryPool; //pMem must be defined and initalized elsewhere.
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::String;
using namespace NSWFL::Conversion;
using namespace NSWFL::File;
using namespace NSWFL::Registry;
using namespace NSWFL::Collections;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int SQLErrorHandler(void *pCaller, const char *sSource, const char *sErrorMsg, const int iErrorNumber)
{
	if (_StrEqlI(sSource, "CSQL"))
	{
		CException *pException = (CException *)((CEngine *)((CSQL *)pCaller)->pPublicData)->Error;
		pException->Soft("%s error %d->[%s]", sSource, iErrorNumber, sErrorMsg);
	}
	else if (_StrEqlI(sSource, "CBoundRecordSet"))
	{
		CException *pException = (CException *)((CEngine *)((CBoundRecordSet *)pCaller)->pPublicData)->Error;
		pException->Soft("%s error %d->[%s]", sSource, iErrorNumber, sErrorMsg);
	}
	else if (_StrEqlI(sSource, "CRecordSet"))
	{
		CException *pException = (CException *)((CEngine *)((CRecordSet *)pCaller)->pPublicData)->Error;
		pException->Soft("%s error %d->[%s]", sSource, iErrorNumber, sErrorMsg);
	}

	//MessageBox(GetActiveWindow(), sErrorMsg, sSource, MB_ICONERROR);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CEngine::IsMathChar(const char cChar)
{
	if (cChar == '(' || cChar == ')'
		|| cChar == '+' || cChar == '-'
		|| cChar == '*' || cChar == '/'
		|| cChar == '!' || cChar == '='
		|| cChar == '<' || cChar == '>'
		|| cChar == '<' || cChar == '>'
		|| cChar == '%' || cChar == '^')
	{
		return true;
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Returns true if the passed character is 0-9.
bool CEngine::IsNumeric(char cIn)
{
	return (cIn >= 48 && cIn <= 57) || cIn == '.';
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CEngine::IsIdentifierChar(const char cChar)
{
	return ((cChar >= 'A' && cChar <= 'Z')
		|| (cChar >= 'a' && cChar <= 'z')
		|| cChar == '_' || this->IsNumeric(cChar));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CEngine::ParseMathI(const char *sText, int iLength)
{
	StringBuilder Result;
	int iRPos = 0;
	if (this->ParseMath(sText, iLength, &iRPos, &Result) == ERROR_OK)
	{
		return atol(Result.Buffer);
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

double CEngine::ParseMathD(const char *sText, int iLength)
{
	StringBuilder Result;
	int iRPos = 0;
	if (this->ParseMath(sText, iLength, &iRPos, &Result) == ERROR_OK)
	{
		return atof(Result.Buffer);
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CEngine::ParseMath(const char *sText, int iLength, int *piRPos, StringBuilder *pResult)
{
	StringBuilder Builder;
	return this->ParseMath(sText, iLength, piRPos, &Builder, pResult);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CEngine::ParseMath(const char *sText, int iLength, int *piRPos, StringBuilder *pBuilder, StringBuilder *pResult)
{
	char sTok[MAX_TOKEN_SIZE];
	char sNSBuf[MAX_TOKEN_SIZE];
	int iTokSz = 0;

	int iRPos = (int)*piRPos;
	int iParenNestLevel = 0;
	int iLastOperatorPosition = 0;

	double dSignCheck = atof(pBuilder->Buffer);

	bool bDoubleMath = false;
	bool bPreIncrement = false;
	bool bPreDecrement = false;
	bool bUnsignedMath = (dSignCheck > INT_MAX || dSignCheck < INT_MIN);

	while (true)
	{
		if (sText[iRPos] == '&' || sText[iRPos] == ',' || iRPos == iLength)
		{
			if (bPreIncrement || bPreDecrement)
			{
				return this->Error->Hard("Failed to parse inline increment or decrement");
			}

			CMathParser MP;

			if (MathDebugMode)
			{
				MP.DebugMode(MathDebugMode);
			}

			if (bDoubleMath || CharIndex(pBuilder->Buffer, pBuilder->Length, '.') >= 0)
			{
				double dResult = 0;
				int iOffset = 0;
				if (pBuilder->Buffer[0] == '@')
				{
					iOffset++;
				}
				if (MP.Calculate(pBuilder->Buffer + iOffset, pBuilder->Length - iOffset, &dResult) != CMathParser::ResultOk)
				{
					return this->Error->Hard("Failed to calculate double expression: %s", MP.LastError()->Text);
				}
				if (pBuilder->Buffer[0] == '@')
				{
					pResult->Append(pBuilder->Buffer[0]);
				}
				pResult->Append(dResult, 8);
			}
			else {
				int iOffset = 0;
				if (pBuilder->Buffer[0] == '@')
				{
					iOffset++;
				}

				if (pBuilder->Buffer[0] == '@')
				{
					pResult->Append(pBuilder->Buffer[0]);
				}

				if(bUnsignedMath)
				{
					unsigned int iResult = 0;
					if (MP.Calculate(pBuilder->Buffer + iOffset, pBuilder->Length - iOffset, &iResult) != CMathParser::ResultOk)
					{
						return this->Error->Hard("Failed to calculate long expression: %s", MP.LastError()->Text);
					}
					pResult->Append(iResult);
				}
				else
				{
					int iResult = 0;
					if (MP.Calculate(pBuilder->Buffer + iOffset, pBuilder->Length - iOffset, &iResult) != CMathParser::ResultOk)
					{
						return this->Error->Hard("Failed to calculate long expression: %s", MP.LastError()->Text);
					}
					pResult->Append(iResult);
				}
			}

			if (sText[iRPos] == '&')
			{
				iRPos++; //Skip the ampersand. The next result will be appended.
			}

			*piRPos = (int)iRPos;

			return this->Error->HardCount();
		}
		else if (this->IsNumeric(sText[iRPos]) || this->IsMathChar(sText[iRPos]))
		{
			if (bPreIncrement || bPreDecrement)
			{
				return this->Error->Hard("Failed to parse inline increment or decrement");
			}

			if (sText[iRPos] == '+' && iRPos + 1 < iLength && sText[iRPos + 1] == '+')
			{
				iRPos += 2; //Skip "++"
				bPreIncrement = true;
			}
			else if (sText[iRPos] == '-' && iRPos + 1 < iLength && sText[iRPos + 1] == '-')
			{
				iRPos += 2; //Skip "--"
				bPreDecrement = true;
			}
			else {
				if (sText[iRPos] == '0' && iLength > iRPos && sText[iRPos + 1] == 'x')
				{
					bUnsignedMath = true;

					if ((iTokSz = Token(sText, iLength, sTok, sizeof(sTok), &iRPos)) <= 0)
					{
						return this->Error->Hard("Expected: [Hex Value]");
					}

					pBuilder->Append(HexToDecU(sTok));

					continue;
				}

				if (sText[iRPos] == '.')
				{
					bool bIsProperty = false;
					if (iLength > iRPos)
					{
						if (!this->IsNumeric(sText[iRPos + 1]))
						{
							//return this->Error->Hard("Not implemented: [Numeric Properties]");
							//-------------------------- (Parse numeric property (Begin))------------------------------------------------------
							bool bHandled = false;
							int iVarIndex = 0;
							int iProcIndex = 0;
							bool bReplaceValue = (pBuilder->Length - iLastOperatorPosition == 0);

							bIsProperty = true;

							this->Vars->AddUnique(VAR_TYPE_NUMERIC, &iVarIndex);

							if (bReplaceValue)
							{
								StringBuilder prePropBuilder;
								int prePropRPos = 0;

								ParseMath(pBuilder->Buffer, pBuilder->Length, &prePropRPos, &prePropBuilder, pResult);
								this->Vars->Set(iVarIndex, pResult->Buffer, pResult->Length);

								pResult->Clear();
								pBuilder->Clear();
							}
							else {
								this->Vars->Set(iVarIndex, pBuilder->Buffer + iLastOperatorPosition, pBuilder->Length - iLastOperatorPosition);
							}

							StringBuilder Property("Numeric");

							if (TokenWithDot(sText, iLength, sTok, sizeof(sTok), &iRPos) > 0)
							{
								Property.Append(sTok);
							}

							if ((iProcIndex = this->Procs->GetIndex(Property.Buffer)) != INVALID_INDEX)
							{
								int iStartPos = iRPos;
								int iBegBrace = 0;
								int iEndBrace = 0;

								if (this->GetParenIndexPositions(sText, iLength, &iRPos, &iBegBrace, &iEndBrace) != ERROR_OK)
								{
									return this->Error->HardCount();
								}

								if ((iEndBrace - iBegBrace) < 0)
								{
									return this->Error->Hard("Parenthesis required for procedure call");
								}

								CParams Additional(this);
								Additional.AddIndex(iVarIndex);

								Property.Append(sText + iStartPos, (iEndBrace - iStartPos) + 1);

								PROCITEM *pProc = this->Procs->GetPointer(iProcIndex);
								CParams Return(this, pProc->ReturnType);
								//We only enforce conversion with script procedures.
								Return.EnforceTypes = (pProc->Type == PROC_TYPE_SCRIPT);

								if (this->Procs->ExecuteByEval(Property.Buffer, Property.Length, &Additional, &Return) != ERROR_OK)
								{
									return this->Error->HardCount();
								}

								if (Return.Collection.Count <= 0)
								{
									return this->Error->Inform("No value returned");
								}
								else if (Return.Collection.Count > 1)
								{
									return this->Error->Hard("Array value returned which is not implemented");
								}

								if (bReplaceValue)
								{
									iLastOperatorPosition = 0;
								}
								else {
									pBuilder->Terminate(iLastOperatorPosition);
								}

								pBuilder->Append(Return.First()->Value, Return.First()->Length);

								if(!bUnsignedMath)
								{
									dSignCheck = atof(Return.First()->Value);
									bUnsignedMath = (dSignCheck > INT_MAX || dSignCheck < INT_MIN);
								}
								bHandled = true;
							}
							else {
								return this->Error->Hard("Unknown variable property");
							}

							Property.Destroy();

							this->Vars->Free(iVarIndex); //This would be cleaned up by the drop in scope if it were not cleaned up early here.

							if (!bHandled)
							{
								return this->Error->Hard("Unhandled numeric property");
							}
							else {
								continue;
							}
							//-------------------------- (Parse numeric property (End))------------------------------------------------------
						}
					}
					else {
						return this->Error->Hard("Unexpected: [.]");
					}

					if (!bIsProperty)
					{
						bDoubleMath = true;
					}
				}
				else if (sText[iRPos] == '(')
				{
					iParenNestLevel++;
				}
				else if (sText[iRPos] == ')')
				{
					iParenNestLevel--;
				}

				if (iParenNestLevel < 0)
				{
					return this->Error->Hard("Parenthesis mismatch");
				}

				if (this->IsMathChar(sText[iRPos]))
				{
					iLastOperatorPosition = pBuilder->Length + 1;
				}

				pBuilder->Append(sText[iRPos++]);
			}
		}
		//----------------------------------------------------------------------------------------------------------------------
		else if (sText[iRPos] == '@')
		{
			if (bPreIncrement || bPreDecrement)
			{
				return this->Error->Hard("Failed to parse inline increment or decrement");
			}

			pBuilder->Append(sText[iRPos]);
			iRPos++; //Skip the "@"

			SkipWhiteSpaces(sText, iLength, &iRPos);

			if ((iTokSz = Token(sText, iLength, sTok, sizeof(sTok), &iRPos)) <= 0)
			{
				return this->Error->Hard("Expected: [Identifier]");
			}

			int iVarIndex = this->Vars->GetIndex(sTok);
			if (iVarIndex != INVALID_INDEX)
			{
				pBuilder->Append(iVarIndex);
			}
			else {
				int iProcIndex = this->Procs->GetIndex(sTok);
				if (iProcIndex != INVALID_INDEX)
				{
					pBuilder->Append(iProcIndex);
				}
				else {
					return this->Error->Hard("Undefined variable");
				}
			}
		}
		//----------------------------------------------------------------------------------------------------------------------
		else if (sText[iRPos] == '\"')
		{
			StringBuilder *pString = EvaluateString(sText, iLength, &iRPos);
			if (this->Error->HardCount())
			{
				return this->Error->HardCount();
			}

			if (pString)
			{
				pBuilder->Append(pString);
				delete pString;
			}
		}
		//----------------------------------------------------------------------------------------------------------------------
		else if (this->IsIdentifierChar(sText[iRPos]) && !this->IsNumeric(sText[iRPos]))
		{
			int iStartPos = iRPos;

			if ((iTokSz = Token(sText, iLength, sTok, sizeof(sTok), &iRPos)) <= 0)
			{
				return this->Error->Hard("Expected: [Identifier]");
			}

			strcpy_s(sNSBuf, sizeof(sNSBuf), sTok);
			while (sText[iRPos] == '.' && this->Scripts->GetPartialNameSpace(sNSBuf))
			{
				iRPos++;

				if ((iTokSz = Token(sText, iLength, sTok, sizeof(sTok), &iRPos)) > 0)
				{
					strcat_s(sNSBuf, sizeof(sNSBuf), ".");
					strcat_s(sNSBuf, sizeof(sNSBuf), sTok);
				}

				if (sText[iRPos] != '.')
				{
					strcpy_s(sTok, sizeof(sTok), sNSBuf);
					break;
				}
			}

			int iProcIndex = INVALID_INDEX;

			int iVarIndex = this->Vars->GetIndex(sTok);
			if (iVarIndex != INVALID_INDEX)
			{
				bool bHandled = false;

				VARIABLEITEM *pVar = this->Vars->GetPointer(iVarIndex);

				bool bPostIncrement = false;
				bool bPostDecrement = false;

				if (bPreIncrement)
				{
					VARIABLEITEM *pVar = this->Vars->GetPointer(iVarIndex);
					if (CharIndex(pVar->Value, pVar->Length, '.') >= 0)
					{
						this->Vars->Set(pVar, atof(pVar->Value) + 1);
					}
					else {
						this->Vars->Set(pVar, atol(pVar->Value) + 1);
					}
					bPreIncrement = false;
				}
				else if (bPreDecrement)
				{
					VARIABLEITEM *pVar = this->Vars->GetPointer(iVarIndex);
					if (CharIndex(pVar->Value, pVar->Length, '.') >= 0)
					{
						this->Vars->Set(pVar, atof(pVar->Value) - 1);
					}
					else {
						this->Vars->Set(pVar, atol(pVar->Value) - 1);
					}
					bPreDecrement = false;
				}

				if (iRPos + 2 <= iLength)
				{
					if (_strnicmp(sText + iRPos, "++", 2) == 0)
					{
						bPostIncrement = true;
						iRPos += 2; //Skip the "++"
					}
					else if (_strnicmp(sText + iRPos, "--", 2) == 0)
					{
						bPostDecrement = true;
						iRPos += 2; //Skip the "--"
					}
				}

				if ((sText + iRPos)[0] == '.')
				{
					int iProcIndex = 0;
					//Parse and handle properties/namespaces (there are three places where this is done).
					StringBuilder Property;

					if (pVar->Type == VAR_TYPE_GENERIC)
					{
						Property.Append(pVar->Name);
					}
					else {
						Property.Append(this->Types->Name(pVar->Type));
					}

					int iParentVariableNameEndPos = Property.Length;
					if (TokenWithDot(sText, iLength, sTok, sizeof(sTok), &iRPos) > 0)
					{
						Property.Append(sTok);
					}

					TYPEITEM *pType = this->Types->GetPointer(pVar->Type);

					//////-------------------------------------------------------------
					char *sMemberName = Property.Buffer + iParentVariableNameEndPos + 1;
					MEMBERENDPOINTINFO MemberEndpoint;
					int iNonMemberPosition = 0;

					if (pType->IsStrictType)
					{
						memset(&MemberEndpoint, 0, sizeof(MemberEndpoint));
						if (this->Types->TraverseMembers(pVar, sMemberName, &MemberEndpoint, &iNonMemberPosition) != ERROR_OK)
						{
							return this->Error->HardCount();
						}

						if (iNonMemberPosition > 0)
						{
							Property.Set(this->Types->Name(MemberEndpoint.Member->Type));
							Property.Append(sMemberName + (strlen(sMemberName) - iNonMemberPosition));
						}
					}

					//////-------------------------------------------------------------

					if (pType->IsStrictType && iNonMemberPosition == 0)
					{
						/*
						char *sMemberName = Property.Buffer + iParentVariableNameEndPos + 1;
						MEMBERENDPOINTINFO MemberEndpoint;
						memset(&MemberEndpoint, 0, sizeof(MemberEndpoint));
						int iNonMemberPosition = 0;

						if((result = this->Types->TraverseMembers(pVar, sMemberName, &MemberEndpoint, &iNonMemberPosition)) != ERROR_OK)
						{
							return result;
						}
						*/

						void *pValue = (pVar->Value + MemberEndpoint.ByteOffset);

						if (MemberEndpoint.Member->StrictType == STRICT_TYPE_INTEGER16)
						{
							pBuilder->Append((short)(*(short*)(pValue)));
						}
						else if (MemberEndpoint.Member->StrictType == STRICT_TYPE_UINTEGER16)
						{
							pBuilder->Append((unsigned short)(*(unsigned short*)(pValue)));
						}
						else if (MemberEndpoint.Member->StrictType == STRICT_TYPE_INTEGER32)
						{
							pBuilder->Append((int)(*(int*)(pValue)));
						}
						else if (MemberEndpoint.Member->StrictType == STRICT_TYPE_UINTEGER32)
						{
							pBuilder->Append((unsigned int)(*(unsigned int*)(pValue)));
						}
						else if (MemberEndpoint.Member->StrictType == STRICT_TYPE_INTEGER64)
						{
							pBuilder->Append((__int64)(*(__int64*)(pValue)));
						}
						else if (MemberEndpoint.Member->StrictType == STRICT_TYPE_UINTEGER64)
						{
							pBuilder->Append((unsigned __int64)(*(unsigned __int64*)(pValue)));
						}
						else if (MemberEndpoint.Member->StrictType == STRICT_TYPE_FLOAT)
						{
							pBuilder->Append((float)(*(float*)(pValue)));
						}
						else if (MemberEndpoint.Member->StrictType == STRICT_TYPE_DOUBLE)
						{
							pBuilder->Append((double)(*(double*)(pValue)));
						}
						else if (MemberEndpoint.Member->StrictType == STRICT_TYPE_STRING)
						{
							if (MemberEndpoint.Member->IsContiguous)
							{
								pBuilder->Append((char*)pValue);
							}
							else {
								pBuilder->Append((*(char**)pValue));
							}
						}
						else if (MemberEndpoint.Member->StrictType == STRICT_TYPE_BYTE)
						{
							pBuilder->Append((const char*)pValue, MemberEndpoint.Member->SizeInBytes);
						}
						else {
							return this->Error->Hard("Type of member variable [%s] cannot be accessed directly", sMemberName);
						}

						//NOT IMPLEMENTED! (Does it need to be?)
						//if(!bUnsignedMath)
						//{
						//dSignCheck = atof(Return.First()->Value);
						//bUnsignedMath = (dSignCheck > INT_MAX || dSignCheck < INT_MIN);
						//}

						bHandled = true;
					}
					else if ((iProcIndex = this->Procs->GetIndex(Property.Buffer)) != INVALID_INDEX)
					{
						iStartPos = iRPos;

						int iBegBrace = 0;
						int iEndBrace = 0;

						if (this->GetParenIndexPositions(sText, iLength, &iRPos, &iBegBrace, &iEndBrace) != ERROR_OK)
						{
							return this->Error->HardCount();
						}

						if ((iEndBrace - iBegBrace) < 0)
						{
							return this->Error->Hard("Parenthesis required for procedure call");
						}

						CParams Additional(this);
						if (pType->IsStrictType)
						{
							Additional.AddIndex(iVarIndex, MemberEndpoint.Member->Index);
						}
						else {
							Additional.AddIndex(iVarIndex);
						}

						Property.Append(sText + iStartPos, (iEndBrace - iStartPos) + 1);

						PROCITEM *pProc = this->Procs->GetPointer(iProcIndex);
						CParams Return(this, pProc->ReturnType);
						//We only enforce conversion with script procedures.
						Return.EnforceTypes = (pProc->Type == PROC_TYPE_SCRIPT);

						if (this->Procs->ExecuteByEval(Property.Buffer, Property.Length, &Additional, &Return) != ERROR_OK)
						{
							return this->Error->HardCount();
						}

						if (Return.Collection.Count <= 0)
						{
							return this->Error->Inform("No value returned");
						}
						else if (Return.Collection.Count > 1)
						{
							return this->Error->Hard("Array value returned which is not implemented");
						}

						if (sText[iRPos] == '.')
						{
							while (sText[iRPos] == '.')
							{
								Property.Clear();

								PROCITEM *pProc = this->Procs->GetPointer(iProcIndex);

								//If adding other generic types, search for this:
								if (pProc->ReturnType == VAR_TYPE_GENERIC)
								{
									Property.Append(pProc->Name); //FIXFIX: This needs some major thinking over!
								}
								else {
									Property.Append(this->Types->Name(pProc->ReturnType));
								}

								if (TokenWithDot(sText, iLength, sTok, sizeof(sTok), &iRPos) > 0)
								{
									Property.Append(sTok);
								}

								if ((iProcIndex = this->Procs->GetIndex(Property.Buffer)) != INVALID_INDEX)
								{
									iStartPos = iRPos;

									this->Vars->AddUnique(pProc->ReturnType, &iVarIndex);
									this->Vars->Set(iVarIndex, Return.First()->Value, Return.First()->Length);

									//--------------------------------------------------------------------------------------

									if (this->GetParenIndexPositions(sText, iLength, &iRPos, &iBegBrace, &iEndBrace) != ERROR_OK)
									{
										return this->Error->HardCount();
									}

									if ((iEndBrace - iBegBrace) < 0)
									{
										return this->Error->Hard("Parenthesis required for procedure call");
									}

									CParams Additional(this);
									Additional.AddIndex(iVarIndex);

									Property.Append(sText + iStartPos, (iEndBrace - iStartPos) + 1);

									Return.Clear();

									if (this->Procs->ExecuteByEval(Property.Buffer, Property.Length, &Additional, &Return) != ERROR_OK)
									{
										return this->Error->HardCount();
									}

									this->Vars->Free(iVarIndex); //This would be cleaned up by the drop in scope if it were not cleaned up early here.

									if (Return.Collection.Count <= 0)
									{
										return this->Error->Inform("No value returned");
									}
									else if (Return.Collection.Count > 1)
									{
										return this->Error->Hard("Array value returned which is not implemented");
									}

									//--------------------------------------------------------------------------------------
								}
								else {
									return this->Error->Hard("Unknown return variable property");
								}
							}

							pBuilder->Append(Return.First()->Value, Return.First()->Length);
							if(!bUnsignedMath)
							{
								dSignCheck = atof(Return.First()->Value);
								bUnsignedMath = (dSignCheck > INT_MAX || dSignCheck < INT_MIN);
							}
							bHandled = true;
						}
						else {
							pBuilder->Append(Return.First()->Value, Return.First()->Length);
							if(!bUnsignedMath)
							{
								dSignCheck = atof(Return.First()->Value);
								bUnsignedMath = (dSignCheck > INT_MAX || dSignCheck < INT_MIN);
							}
							bHandled = true;
						}
					}
					else if ((iVarIndex = this->Vars->GetIndex(Property.Buffer)) != INVALID_INDEX)
					{
						VARIABLEITEM *pPropVar = this->Vars->GetPointer(iVarIndex);
						pBuilder->Append(pPropVar->Value, pPropVar->Length);
						if(!bUnsignedMath)
						{
							dSignCheck = atof(pPropVar->Value);
							bUnsignedMath = (dSignCheck > INT_MAX || dSignCheck < INT_MIN);
						}
						bHandled = true;
					}
					else {
						return this->Error->Hard("Unknown variable property");
					}

					Property.Destroy();
				}

				if (!bHandled)
				{
					pBuilder->Append(pVar->Value, pVar->Length);
					if(!bUnsignedMath)
					{
						dSignCheck = atof(pVar->Value);
						bUnsignedMath = (dSignCheck > INT_MAX || dSignCheck < INT_MIN);
					}
				}

				if (bPostIncrement)
				{
					VARIABLEITEM *pVar = this->Vars->GetPointer(iVarIndex);
					if (CharIndex(pVar->Value, pVar->Length, '.') >= 0)
					{
						this->Vars->Set(pVar, atof(pVar->Value) + 1);
					}
					else {
						this->Vars->Set(pVar, atol(pVar->Value) + 1);
					}
				}
				else if (bPostDecrement)
				{
					VARIABLEITEM *pVar = this->Vars->GetPointer(iVarIndex);
					if (CharIndex(pVar->Value, pVar->Length, '.') >= 0)
					{
						this->Vars->Set(pVar, atof(pVar->Value) - 1);
					}
					else {
						this->Vars->Set(pVar, atol(pVar->Value) - 1);
					}
				}
			}
			else if ((iProcIndex = this->Procs->GetIndex(sTok)) != INVALID_INDEX)
			{
				int iBegBrace = 0;
				int iEndBrace = 0;

				if (this->GetParenIndexPositions(sText, iLength, &iRPos, &iBegBrace, &iEndBrace) != ERROR_OK)
				{
					return this->Error->HardCount();
				}

				if ((iEndBrace - iBegBrace) < 0)
				{
					this->Error->Hard("Parenthesis required for procedure call");
					return this->Error->HardCount();
				}

				PROCITEM *pProc = this->Procs->GetPointer(iProcIndex);
				CParams Return(this, pProc->ReturnType);
				//We only enforce conversion with script procedures.
				Return.EnforceTypes = (pProc->Type == PROC_TYPE_SCRIPT);

				if (this->Procs->ExecuteByEval(sText + iStartPos, (iEndBrace - iStartPos) + 1, NULL, &Return) != ERROR_OK)
				{
					return this->Error->HardCount();
				}

				if (Return.Collection.Count <= 0)
				{
					return this->Error->Inform("No value returned");
				}
				else if (Return.Collection.Count > 1)
				{
					return this->Error->Hard("Array value returned which is not implemented");
				}

				pBuilder->Append(Return.First()->Value, Return.First()->Length);
			}
			else {
				return this->Error->Hard("Undeclared identifier: [%s]", sTok);
			}
			//Make sure there is no code after the last brace.
			//if(pLine->Text[pLine->Length - 1] != ')')
			//{
				//return this->Error->Hard("Code after closing bracket in function call");
			//}

			//CParams Params(this);
			//return this->Procs->ExecuteByEval(pLine->Text, pLine->Length, NULL, &Params);
		}
		else if (IsWhiteSpace(sText[iRPos]))
		{
			SkipWhiteSpaces(sText, iLength, &iRPos);
		}
		else {
			return this->Error->Hard("Unexpected: [%c]", sText[iRPos]);
		}
	}

	return this->Error->Hard("Failed to parse mathematical expression");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

StringBuilder *CEngine::EvaluateString(const char *sText, int iLength, int *piRPos)
{
	char sTok[MAX_TOKEN_SIZE];
	int iTokSz = 0;

	int iRPos = (int)*piRPos;

	StringBuilder *pParam = new StringBuilder();

	bool noEscaping = false;
	bool interpolation = false;

	while(sText[iRPos] == ':' || sText[iRPos] == '$')
	{
		if(sText[iRPos] == ':')
		{
			noEscaping = true;
			iRPos++;
		}

		if(sText[iRPos] == '$')
		{
			interpolation = true;
			iRPos++;
		}
	}

	int iStartPos = iRPos++; //Skip the opening double quote.

	while (true)
	{
		if (iRPos == iLength && sText[iRPos] != '\"')
		{
			this->Error->Hard("Expected: [\"]");
			delete pParam;
			return NULL;
		}
		else if (sText[iRPos] == '$' && sText[iRPos + 1] == '{' && interpolation == true)
		{
			iRPos += 2; //Skip the '${';

			if (TokenWithDot(sText, iLength, sTok, sizeof(sTok), &iRPos) > 0)
			{
				VARIABLEITEM *pInterpolateVar = this->Vars->GetPointer(sTok);
				if(pInterpolateVar == NULL)
				{
					this->Error->Hard("undefined variable found in string interpolation [%s]", sTok);
					return NULL;			
				}

				pParam->Append(pInterpolateVar->Value, pInterpolateVar->Length);

			}
			else {
				this->Error->Hard("Failed to parse string interpolation tag");
				return NULL;			
			}

			if(sText[iRPos] != '}')
			{
				this->Error->Hard("Failed to parse string interpolation tag");
				return NULL;			
			}

			iRPos++; //Skip the '}';

			continue;
		}
		else if (sText[iRPos] == '\\' && noEscaping == false)
		{
			iRPos++; //Skip the '\';
			char Next = this->GetEscapedCharacter(sText[iRPos]);

			pParam->Append(Next);
			iRPos++; //Skip the next character.
			continue;
		}
		else if (sText[iRPos] == '\"')
		{
			iRPos++; //Skip the closing double quote.
			SkipWhiteSpaces(sText, iLength, &iRPos);

			//Parse math
			if (iRPos < iLength && sText[iRPos] == '=')
			{
				int iPreviousSoftErrors = this->Error->SoftCount();

				StringBuilder result;

				iRPos = iStartPos;
				pParam->Clear();
				if (ParseMath(sText, iLength, &iRPos, pParam, &result) != ERROR_OK)
				{
					delete pParam;
					return NULL;
				}

				if (this->Error->SoftCount() != iPreviousSoftErrors)
				{
					delete pParam;
					return NULL;
				}

				pParam->Clear();
				pParam->Append(&result);
			}
			//*******************************************************************
			else if (sText[iRPos] == '.')
			{
				StringBuilder Property;
				CParams Return(this);

				int iVarIndex = INVALID_INDEX;
				int iProcIndex = INVALID_INDEX;
				int iBegBrace = 0;
				int iEndBrace = 0;

				//Literal string properties will include the entire parameter value built to this point.
				Return.Add(pParam->Buffer, pParam->Length);
				pParam->Clear();

				while (sText[iRPos] == '.')
				{
					Property.Clear();
					Property.Append(this->Types->Name(VAR_TYPE_STRING));

					if (TokenWithDot(sText, iLength, sTok, sizeof(sTok), &iRPos) > 0)
					{
						Property.Append(sTok);
					}

					if ((iProcIndex = this->Procs->GetIndex(Property.Buffer)) != INVALID_INDEX)
					{
						iStartPos = iRPos;

						PROCITEM *pProc = this->Procs->GetPointer(iProcIndex);
						Return.Type = pProc->ReturnType;
						//We only enforce conversion with script procedures.
						Return.EnforceTypes = (pProc->Type == PROC_TYPE_SCRIPT);


						this->Vars->AddUnique(VAR_TYPE_STRING, &iVarIndex);
						this->Vars->Set(iVarIndex, Return.First()->Value, Return.First()->Length);

						//--------------------------------------------------------------------------------------

						if (this->GetParenIndexPositions(sText, iLength, &iRPos, &iBegBrace, &iEndBrace) != ERROR_OK)
						{
							return NULL;
						}

						if ((iEndBrace - iBegBrace) < 0)
						{
							this->Error->Hard("Parenthesis required for procedure call");
							return NULL;
						}

						CParams Additional(this);
						Additional.AddIndex(iVarIndex);

						Property.Append(sText + iStartPos, (iEndBrace - iStartPos) + 1);

						Return.Clear();

						if (this->Procs->ExecuteByEval(Property.Buffer, Property.Length, &Additional, &Return) != ERROR_OK)
						{
							return NULL;
						}

						this->Vars->Free(iVarIndex); //This would be cleaned up by the drop in scope if it were not cleaned up early here.

						if (Return.Collection.Count <= 0)
						{
							this->Error->Inform("No value returned");
							return NULL;
						}
						else if (Return.Collection.Count > 1)
						{
							this->Error->Hard("Array value returned which is not implemented");
							return NULL;
						}

						//--------------------------------------------------------------------------------------
					}
					else {
						this->Error->Hard("Unknown return variable property");
						return NULL;
					}
				}

				pParam->Append(Return.First()->Value, Return.First()->Length);

				SkipWhiteSpaces(sText, iLength, &iRPos);
			}

			//*******************************************************************
			break;
		}
		else {
			pParam->Append(sText[iRPos++]);
		}
	}

	*piRPos = iRPos;

	return pParam;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Evaluates a line of code and returns a collection of parameters CParams.Collection.Items[].
	This is the core of the engine, everything is tokenized through here.
*/
ErrorLevel CEngine::Evaluate(const char *sText, int iLength, CParams **returnValues)
{
	(*returnValues) = new CParams(this);

	char sTok[MAX_TOKEN_SIZE];
	char sNSBuf[MAX_TOKEN_SIZE];
	int iTokSz = 0;

	StringBuilder currentParam; //This will contain the actual text of the current parameter.
	StringBuilder finalParam;

	bool bPreIncrement = false;
	bool bPreDecrement = false;
	int iLastCompletedSegmentPos = 0;

	ErrorLevel result = ERROR_OK;

	for (int iRPos = 0;;)
	{
		SkipWhiteSpaces(sText, iLength, &iRPos);

		//----------------------------------------------------------------------------------------------------------------------
		if (sText[iRPos] == ',' || iRPos == iLength)
		{

			iLastCompletedSegmentPos = iRPos;
			if (iRPos != iLength)
			{
				iLastCompletedSegmentPos++; //Skip the comma.
			}

			if (bPreIncrement || bPreDecrement)
			{
				delete (*returnValues); (*returnValues) = NULL;
				return this->Error->Hard("Failed to parse inline increment or decrement");
			}

			finalParam.Append(&currentParam);
			currentParam.Clear();

			(*returnValues)->Add(finalParam.Buffer, finalParam.Length);
			finalParam.Clear();

			//End of parameter.
			if (iRPos == iLength)
			{
				if (this->Error->HardCount())
				{
					delete (*returnValues); (*returnValues) = NULL;
					return this->Error->HardCount();
				}

				//This is where the parsing ends!!!
				return ERROR_OK; //End of parsing.
			}
			else {
				iRPos++;
			}
		}
		//----------------------------------------------------------------------------------------------------------------------
		else if (sText[iRPos] == '\"' || sText[iRPos] == '$' || sText[iRPos] == ':')
		{
			if (bPreIncrement || bPreDecrement)
			{
				delete (*returnValues); (*returnValues) = NULL;
				return this->Error->Hard("Failed to parse inline increment or decrement");
			}

			StringBuilder *pString = EvaluateString(sText, iLength, &iRPos);
			if (this->Error->HardCount())
			{
				return this->Error->HardCount();
			}

			if (pString)
			{
				currentParam.Append(pString);
				delete pString;
			}
		}
		//----------------------------------------------------------------------------------------------------------------------
		else if (sText[iRPos] == '@' || this->IsNumeric(sText[iRPos]) || this->IsMathChar(sText[iRPos]))
		{
			if (bPreIncrement || bPreDecrement)
			{
				delete (*returnValues); (*returnValues) = NULL;
				return this->Error->Hard("Failed to parse inline increment or decrement");
			}

			if (sText[iRPos] == '+' && iRPos + 1 < iLength && sText[iRPos + 1] == '+')
			{
				iRPos += 2; //Skip "++"
				bPreIncrement = true;
			}
			else if (sText[iRPos] == '-' && iRPos + 1 < iLength && sText[iRPos + 1] == '-')
			{
				iRPos += 2; //Skip "--"
				bPreDecrement = true;
			}
			else {
				int iPreviousSoftErrors = this->Error->SoftCount();

				//Parse math.
				StringBuilder Param;
				if ((result = ParseMath(sText, iLength, &iRPos, &currentParam, &Param)) != ERROR_OK)
				{
					delete (*returnValues); (*returnValues) = NULL;
					return result;
				}

				currentParam.Clear();
				currentParam.Append(&Param);

				if (this->Error->SoftCount() != iPreviousSoftErrors || this->Error->HardCount())
				{
					delete (*returnValues); (*returnValues) = NULL;
					return this->Error->HardCount();
				}
			}
		}
		//----------------------------------------------------------------------------------------------------------------------
		else if (this->IsIdentifierChar(sText[iRPos]) && !this->IsNumeric(sText[iRPos]))
		{
			//Parse variables.
			StringBuilder Param;
			int iStartPos = iRPos;

			//Parse function calls and variable names.
			if ((iTokSz = Token(sText, iLength, sTok, sizeof(sTok), &iRPos)) <= 0)
			{
				delete (*returnValues); (*returnValues) = NULL;
				return this->Error->Hard("Expected: [Identifier]");
			}

			strcpy_s(sNSBuf, sizeof(sNSBuf), sTok);
			while (sText[iRPos] == '.' && this->Scripts->GetPartialNameSpace(sNSBuf))
			{
				iRPos++;

				if ((iTokSz = Token(sText, iLength, sTok, sizeof(sTok), &iRPos)) > 0)
				{
					strcat_s(sNSBuf, sizeof(sNSBuf), ".");
					strcat_s(sNSBuf, sizeof(sNSBuf), sTok);
				}

				if (sText[iRPos] != '.')
				{
					strcpy_s(sTok, sizeof(sTok), sNSBuf);
					break;
				}
			}

			if (this->Vars->IsMathConstant(sTok))
			{
				iRPos = iLastCompletedSegmentPos;
				int iPreviousSoftErrors = this->Error->SoftCount();
				currentParam.Clear();

				//Parse math.
				StringBuilder Param;
				if ((result = ParseMath(sText, iLength, &iRPos, &Param, &currentParam)) != ERROR_OK)
				{
					delete (*returnValues); (*returnValues) = NULL;
					result;
				}

				if (this->Error->SoftCount() != iPreviousSoftErrors)
				{
					delete (*returnValues); (*returnValues) = NULL;
					return this->Error->HardCount();
				}
			}
			else {
				int iVarIndex = INVALID_INDEX;
				int iTypeIndex = INVALID_INDEX;
				int iProcIndex = INVALID_INDEX;
				VARIABLEITEM *pVar = NULL;
				bool bHandled = false;

				if ((iVarIndex = this->Vars->GetIndex(sTok)) == INVALID_INDEX)
				{
					if((iProcIndex = this->Procs->GetIndex(sTok)) == INVALID_INDEX)
					{
						iTypeIndex = this->Types->GetIndex(sTok);
					}
				}
				else
				{
					pVar = this->Vars->GetPointer(iVarIndex);
				}

				if (iTypeIndex != INVALID_INDEX)
				{
					Param.Append(iTypeIndex);
					bHandled = true;
				}
				else if (pVar)
				{
					bool bPostIncrement = false;
					bool bPostDecrement = false;

					if (bPreIncrement)
					{
						VARIABLEITEM *pVar = this->Vars->GetPointer(iVarIndex);
						if (CharIndex(pVar->Value, pVar->Length, '.') >= 0)
						{
							this->Vars->Set(pVar, atof(pVar->Value) + 1);
						}
						else {
							this->Vars->Set(pVar, atol(pVar->Value) + 1);
						}
						bPreIncrement = false;
					}
					else if (bPreDecrement)
					{
						VARIABLEITEM *pVar = this->Vars->GetPointer(iVarIndex);
						if (CharIndex(pVar->Value, pVar->Length, '.') >= 0)
						{
							this->Vars->Set(pVar, atof(pVar->Value) - 1);
						}
						else {
							this->Vars->Set(pVar, atol(pVar->Value) - 1);
						}
						bPreDecrement = false;
					}

					if (iRPos + 2 <= iLength)
					{
						if ((sText + iRPos)[0] == '[')
						{
							if (pVar->Type == VAR_TYPE_ARRAY)
							{
								int iBegBrace = 0;
								int iEndBrace = 0;

								if ((result = this->GetSquareBracketIndexPositions(sText, iLength, &iRPos, &iBegBrace, &iEndBrace)) != ERROR_OK)
								{
									delete (*returnValues); (*returnValues) = NULL;
									return result;
								}

								int iPreviousSoftErrors = this->Error->SoftCount();

								int iArrayIndex = this->ParseMathI(sText + iBegBrace, iEndBrace - iBegBrace);
								if (this->Error->SoftCount() != iPreviousSoftErrors)
								{
									delete (*returnValues); (*returnValues) = NULL;
									return this->Error->HardCount();
								}

								TYPEITEM *pSubType = this->Types->GetPointer(pVar->ArrayDefinition.Type);

								if (pSubType->IsBuiltIn)
								{
									int iSubVarIndex = (int)((void **)pVar->Value)[iArrayIndex];

									VARIABLEITEM *pSubVar = NULL;

									if (iSubVarIndex > 0)
									{
										pSubVar = this->Vars->GetPointer(iSubVarIndex);
									}

									this->Vars->AddUnique(pSubType->Index, &iVarIndex); //Create temp variable.

									if (pSubVar != NULL)
									{
										this->Vars->Set(iVarIndex, pSubVar->Value, pSubVar->Length); //Set temp variable.
									}

									pVar = this->Vars->GetPointer(iVarIndex); //Save pointer to temp variable.
								}
								else
								{
									return this->Error->Hard("Array type has not been implemented");
								}

							}
							else if (pVar->Type == VAR_TYPE_LIST)
							{
								//int iOriginalRPos = iRPos;
								int iBegBrace = 0;
								int iEndBrace = 0;

								if ((result = this->GetSquareBracketIndexPositions(sText, iLength, &iRPos, &iBegBrace, &iEndBrace)) != ERROR_OK)
								{
									delete (*returnValues); (*returnValues) = NULL;
									return result;
								}

								VectorBinary* pVect = ((VectorBinary*)pVar->Value);

								int iPreviousSoftErrors = this->Error->SoftCount();

								int iIndex = this->ParseMathI(sText + iBegBrace, iEndBrace - iBegBrace);
								if (this->Error->SoftCount() != iPreviousSoftErrors)
								{
									delete (*returnValues); (*returnValues) = NULL;
									return this->Error->HardCount();
								}

								if (iIndex < 0 || iIndex >= pVect->Count())
								{
									delete (*returnValues); (*returnValues) = NULL;
									return this->Error->Hard("Index is out of bounds (%d of %d)", iIndex, pVect->Count() - 1);
								}

								auto pVecVal = pVect->GetValueAtIndex(iIndex);

								this->Vars->AddUnique(VAR_TYPE_STRING, &iVarIndex); //Create temp variable.
								this->Vars->Set(iVarIndex, pVecVal->Value, pVecVal->Length); //Set temp variable.
								pVar = this->Vars->GetPointer(iVarIndex); //Save pointer to temp variable.
							}
							else if (pVar->Type == VAR_TYPE_STRING)
							{
								//int iOriginalRPos = iRPos;
								int iBegBrace = 0;
								int iEndBrace = 0;

								if ((result = this->GetSquareBracketIndexPositions(sText, iLength, &iRPos, &iBegBrace, &iEndBrace)) != ERROR_OK)
								{
									delete (*returnValues); (*returnValues) = NULL;
									return result;
								}

								int iPreviousSoftErrors = this->Error->SoftCount();

								int iIndex = this->ParseMathI(sText + iBegBrace, iEndBrace - iBegBrace);
								if (this->Error->SoftCount() != iPreviousSoftErrors)
								{
									delete (*returnValues); (*returnValues) = NULL;
									return this->Error->HardCount();
								}

								if (iIndex < 0 || iIndex >= pVar->Length)
								{
									delete (*returnValues); (*returnValues) = NULL;
									return this->Error->Hard("Index is out of bounds (%d of %d)", iIndex, pVar->Length - 1);
								}

								char cCharacter = pVar->Value[iIndex]; //Save the character from the old variable
								this->Vars->AddUnique(pVar->Type, &iVarIndex); //Create temp variable.
								this->Vars->Set(iVarIndex, cCharacter); //Set temp variable.
								pVar = this->Vars->GetPointer(iVarIndex); //Save pointer to temp variable.
							}
							else
							{
								delete (*returnValues); (*returnValues) = NULL;
								return this->Error->Hard("Cannot perform indexing on non-array types");
							}
						}
						else {
							if (_strnicmp(sText + iRPos, "++", 2) == 0)
							{
								bPostIncrement = true;
								iRPos += 2; //Skip the "++"
							}
							else if (_strnicmp(sText + iRPos, "--", 2) == 0)
							{
								bPostDecrement = true;
								iRPos += 2; //Skip the "--"
							}
						}

						if ((sText + iRPos)[0] == '.')
						{
							//Parse and handle properties/namespaces (there are three places where this is done).
							StringBuilder Property;

							//If adding other generic types, search for this:
							if (pVar->Type == VAR_TYPE_GENERIC)
							{
								Property.Append(pVar->Name);
							}
							else {
								Property.Append(this->Types->Name(pVar->Type));
							}

							int iParentVariableNameEndPos = Property.Length;
							if (TokenWithDot(sText, iLength, sTok, sizeof(sTok), &iRPos) > 0)
							{
								Property.Append(sTok);
							}

							TYPEITEM *pType = this->Types->GetPointer(pVar->Type);

							//////-------------------------------------------------------------
							char *sMemberName = Property.Buffer + iParentVariableNameEndPos + 1;
							MEMBERENDPOINTINFO MemberEndpoint;
							int iNonMemberPosition = 0;

							if (pType->IsStrictType)
							{
								memset(&MemberEndpoint, 0, sizeof(MemberEndpoint));
								if (this->Types->TraverseMembers(pVar, sMemberName, &MemberEndpoint, &iNonMemberPosition) != ERROR_OK)
								{
									return this->Error->HardCount();
								}

								if (iNonMemberPosition > 0)
								{
									Property.Set(this->Types->Name(MemberEndpoint.Member->Type));
									Property.Append(sMemberName + (strlen(sMemberName) - iNonMemberPosition));
								}
							}

							//////-------------------------------------------------------------

							if (pType->IsStrictType && iNonMemberPosition == 0)
							{
								/*
								char *sMemberName = Property.Buffer + iParentVariableNameEndPos + 1;
								MEMBERENDPOINTINFO MemberEndpoint;
								memset(&MemberEndpoint, 0, sizeof(MemberEndpoint));
								int iNonMemberPosition = 0;

								if((result = this->Types->TraverseMembers(pVar, sMemberName, &MemberEndpoint, &iNonMemberPosition)) != ERROR_OK)
								{
									return result;
								}
								*/

								void *pValue = (pVar->Value + MemberEndpoint.ByteOffset);

								if (MemberEndpoint.Member->StrictType == STRICT_TYPE_INTEGER16)
								{
									Param.Append((short)(*(short*)(pValue)));
								}
								else if (MemberEndpoint.Member->StrictType == STRICT_TYPE_UINTEGER16)
								{
									Param.Append((unsigned short)(*(unsigned short*)(pValue)));
								}
								else if (MemberEndpoint.Member->StrictType == STRICT_TYPE_INTEGER32)
								{
									Param.Append((int)(*(int*)(pValue)));
								}
								else if (MemberEndpoint.Member->StrictType == STRICT_TYPE_UINTEGER32)
								{
									Param.Append((unsigned int)(*(unsigned int*)(pValue)));
								}
								else if (MemberEndpoint.Member->StrictType == STRICT_TYPE_INTEGER64)
								{
									Param.Append((__int64)(*(__int64*)(pValue)));
								}
								else if (MemberEndpoint.Member->StrictType == STRICT_TYPE_UINTEGER64)
								{
									Param.Append((unsigned __int64)(*(unsigned __int64*)(pValue)));
								}
								else if (MemberEndpoint.Member->StrictType == STRICT_TYPE_FLOAT)
								{
									Param.Append((float)(*(float*)(pValue)));
								}
								else if (MemberEndpoint.Member->StrictType == STRICT_TYPE_DOUBLE)
								{
									Param.Append((double)(*(double*)(pValue)));
								}
								else if (MemberEndpoint.Member->StrictType == STRICT_TYPE_STRING)
								{
									if (MemberEndpoint.Member->IsContiguous)
									{
										Param.Append((char*)pValue);
									}
									else {
										Param.Append((*(char**)pValue));
									}
								}
								else if (MemberEndpoint.Member->StrictType == STRICT_TYPE_BYTE)
								{
									Param.Append((const char*)pValue, MemberEndpoint.Member->SizeInBytes);
								}
								else {
									return this->Error->Hard("Type of member variable [%s] cannot be accessed directly", sMemberName);
								}

								bHandled = true;
							}
							else if ((iProcIndex = this->Procs->GetIndex(Property.Buffer)) != INVALID_INDEX)
							{
								iStartPos = iRPos;

								int iBegBrace = 0;
								int iEndBrace = 0;

								if ((result = this->GetParenIndexPositions(sText, iLength, &iRPos, &iBegBrace, &iEndBrace)) != ERROR_OK)
								{
									return result;
								}

								if ((iEndBrace - iBegBrace) < 0)
								{
									return this->Error->Hard("Parenthesis required for procedure call");
								}

								CParams Additional(this);
								if (pType->IsStrictType)
								{
									Additional.AddIndex(iVarIndex, MemberEndpoint.Member->Index);
								}
								else {
									Additional.AddIndex(iVarIndex);
								}

								Property.Append(sText + iStartPos, (iEndBrace - iStartPos) + 1);

								PROCITEM *pProc = this->Procs->GetPointer(iProcIndex);
								CParams Return(this, pProc->ReturnType);
								//We only enforce conversion with script procedures.
								Return.EnforceTypes = (pProc->Type == PROC_TYPE_SCRIPT);

								if ((result = this->Procs->ExecuteByEval(Property.Buffer, Property.Length, &Additional, &Return)) != ERROR_OK)
								{
									return result;
								}

								if (Return.Collection.Count <= 0)
								{
									return this->Error->Inform("No value returned");
								}
								else if (Return.Collection.Count > 1)
								{
									return this->Error->Hard("Array value returned which is not implemented");
								}

								if (sText[iRPos] == '.')
								{
									while (sText[iRPos] == '.')
									{
										Property.Clear();

										PROCITEM *pProc = this->Procs->GetPointer(iProcIndex);

										//If adding other generic types, search for this:
										if (pProc->ReturnType == VAR_TYPE_GENERIC)
										{
											Property.Append(pProc->Name); //FIXFIX: This needs some major thinking over!
										}
										else {
											Property.Append(this->Types->Name(pProc->ReturnType));
										}

										if (TokenWithDot(sText, iLength, sTok, sizeof(sTok), &iRPos) > 0)
										{
											Property.Append(sTok);
										}

										if ((iProcIndex = this->Procs->GetIndex(Property.Buffer)) != INVALID_INDEX)
										{
											iStartPos = iRPos;

											this->Vars->AddUnique(pProc->ReturnType, &iVarIndex);
											this->Vars->Set(iVarIndex, Return.First()->Value, Return.First()->Length);

											//--------------------------------------------------------------------------------------

											if ((result = this->GetParenIndexPositions(sText, iLength, &iRPos, &iBegBrace, &iEndBrace)) != ERROR_OK)
											{
												return result;
											}

											if ((iEndBrace - iBegBrace) < 0)
											{
												return this->Error->Hard("Parenthesis required for procedure call");
											}

											CParams Additional(this);
											Additional.AddIndex(iVarIndex);

											Property.Append(sText + iStartPos, (iEndBrace - iStartPos) + 1);

											Return.Clear();

											if ((result = this->Procs->ExecuteByEval(Property.Buffer, Property.Length, &Additional, &Return)) != ERROR_OK)
											{
												return result;
											}

											this->Vars->Free(iVarIndex); //This would be cleaned up by the drop in scope if it were not cleaned up early here.

											if (Return.Collection.Count <= 0)
											{
												return this->Error->Inform("No value returned");
											}
											else if (Return.Collection.Count > 1)
											{
												return this->Error->Hard("Array value returned which is not implemented");
											}

											//--------------------------------------------------------------------------------------
										}
										else {
											return this->Error->Hard("Unknown return variable property");
										}
									}

									Param.Append(Return.First()->Value, Return.First()->Length);
									bHandled = true;
								}
								else {
									Param.Append(Return.First()->Value, Return.First()->Length);
									bHandled = true;
								}
							}
							else if ((iVarIndex = this->Vars->GetIndex(Property.Buffer)) != INVALID_INDEX)
							{
								pVar = this->Vars->GetPointer(iVarIndex);
								Param.Append(pVar->Value, pVar->Length);
								bHandled = true;
							}
							else {
								return this->Error->Hard("Unknown variable property");
							}

							Property.Destroy();
						}
					}

					if (!bHandled)
					{
						if (this->Types->GetPointer(pVar->Type)->IsBuiltInClass || pVar->Type == VAR_TYPE_ARRAY)
						{
							//Class types must be passesd by reference.
							Param.Append("@");
							Param.Append(iVarIndex);
						}
						else {
							Param.Append(pVar->Value, pVar->Length);
						}
					}

					if (bPostIncrement)
					{
						VARIABLEITEM *pVar = this->Vars->GetPointer(iVarIndex);
						if (CharIndex(pVar->Value, pVar->Length, '.') >= 0)
						{
							this->Vars->Set(pVar, atof(pVar->Value) + 1);
						}
						else {
							this->Vars->Set(pVar, atol(pVar->Value) + 1);
						}
					}
					else if (bPostDecrement)
					{
						VARIABLEITEM *pVar = this->Vars->GetPointer(iVarIndex);
						if (CharIndex(pVar->Value, pVar->Length, '.') >= 0)
						{
							this->Vars->Set(pVar, atof(pVar->Value) - 1);
						}
						else {
							this->Vars->Set(pVar, atol(pVar->Value) - 1);
						}
					}
				}
				else if (iProcIndex != INVALID_INDEX)
				{
					int iBegBrace = 0;
					int iEndBrace = 0;

					if ((result = this->GetParenIndexPositions(sText, iLength, &iRPos, &iBegBrace, &iEndBrace)) != ERROR_OK)
					{
						return result;
					}

					if ((iEndBrace - iBegBrace) < 0)
					{
						return this->Error->Hard("Parenthesis required for procedure call");
					}

					PROCITEM *pProc = this->Procs->GetPointer(iProcIndex);
					CParams Return(this, pProc->ReturnType);
					//We only enforce conversion with script procedures.
					Return.EnforceTypes = (pProc->Type == PROC_TYPE_SCRIPT);

					if ((result = this->Procs->ExecuteByEval(sText + iStartPos, (iEndBrace - iStartPos) + 1, NULL, &Return)) != ERROR_OK)
					{
						return result;
					}

					if (pProc->ReturnType != VAR_TYPE_INVALID)
					{
						if (Return.Collection.Count <= 0)
						{
							return this->Error->Inform("No value returned");
						}
						else if (Return.Collection.Count > 1)
						{
							return this->Error->Hard("Array value returned which is not implemented");
						}
						//****************

						if (sText[iRPos] == '.')
						{
							StringBuilder Property;

							while (sText[iRPos] == '.')
							{
								Property.Clear();

								PROCITEM *pProc = this->Procs->GetPointer(iProcIndex);

								//If adding other generic types, search for this:
								if (pProc->ReturnType == VAR_TYPE_GENERIC)
								{
									Property.Append(pProc->Name); //FIXFIX: This needs some major thinking over!
								}
								else {
									Property.Append(this->Types->Name(pProc->ReturnType));
								}

								if (TokenWithDot(sText, iLength, sTok, sizeof(sTok), &iRPos) > 0)
								{
									Property.Append(sTok);
								}

								if ((iProcIndex = this->Procs->GetIndex(Property.Buffer)) != INVALID_INDEX)
								{
									iStartPos = iRPos;

									this->Vars->AddUnique(pProc->ReturnType, &iVarIndex);
									this->Vars->Set(iVarIndex, Return.First()->Value, Return.First()->Length);

									//--------------------------------------------------------------------------------------

									if ((result = this->GetParenIndexPositions(sText, iLength, &iRPos, &iBegBrace, &iEndBrace)) != ERROR_OK)
									{
										return result;
									}

									if ((iEndBrace - iBegBrace) < 0)
									{
										return this->Error->Hard("Parenthesis required for procedure call");
									}

									CParams Additional(this);
									Additional.AddIndex(iVarIndex);

									Property.Append(sText + iStartPos, (iEndBrace - iStartPos) + 1);

									Return.Clear();

									if ((result = this->Procs->ExecuteByEval(Property.Buffer, Property.Length, &Additional, &Return)) != ERROR_OK)
									{
										return result;
									}

									this->Vars->Free(iVarIndex); //This would be cleaned up by the drop in scope if it were not cleaned up early here.

									if (Return.Collection.Count <= 0)
									{
										return this->Error->Inform("No value returned");
									}
									else if (Return.Collection.Count > 1)
									{
										return this->Error->Hard("Array value returned which is not implemented");
									}

									//--------------------------------------------------------------------------------------
								}
								else {
									return this->Error->Hard("Unknown return variable property");
								}
							}

							Param.Append(Return.First()->Value, Return.First()->Length);
							bHandled = true;
						}
						else {
							Param.Append(Return.First()->Value, Return.First()->Length);
							bHandled = true;
						}
					}

					//****************
				}
				else {
					delete (*returnValues); (*returnValues) = NULL;
					return this->Error->Hard("Undefined identifier: [%s]", sTok);
				}

				SkipWhiteSpaces(sText, iLength, &iRPos);

				//Parse any trailing math.
				if (iRPos < iLength && this->IsMathChar(sText[iRPos]))
				{
					int iPreviousSoftErrors = this->Error->SoftCount();

					if ((result = ParseMath(sText, iLength, &iRPos, &Param, &currentParam)) != ERROR_OK)
					{
						delete (*returnValues); (*returnValues) = NULL;
						return result;
					}

					if (this->Error->SoftCount() != iPreviousSoftErrors)
					{
						delete (*returnValues); (*returnValues) = NULL;
						return this->Error->HardCount();
					}
				}
				else {
					currentParam.Append(&Param);
				}
			}

			SkipWhiteSpaces(sText, iLength, &iRPos);
		}
		else if (sText[iRPos] == '&')
		{
			if (bPreIncrement || bPreDecrement)
			{
				delete (*returnValues); (*returnValues) = NULL;
				return this->Error->Hard("Failed to parse inline increment or decrement");
			}

			finalParam.Append(&currentParam);
			currentParam.Clear();
			iLastCompletedSegmentPos = iRPos + 1; //Skip the '&'

			iRPos++; //Skip the ampersand. The next result will be appended.
		}
		//----------------------------------------------------------------------------------------------------------------------
		else {
			delete (*returnValues); (*returnValues) = NULL;
			return this->Error->Hard("General syntax error");
		}
		//----------------------------------------------------------------------------------------------------------------------

		currentParam.Terminate();
	}

	delete (*returnValues); (*returnValues) = NULL;
	return this->Error->Hard("General parser error");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CEngine::SkipFunctionBlock(void)
{
	char sTok[MAX_TOKEN_SIZE];
	int iTokSz = 0;
	int iProcEntryCount = 0;

	int iStartingLine = this->State->LineIndex;

	for (;this->State->LineIndex < this->Lines->Collection.Count; this->State->LineIndex++)
	{
		LINEITEM *pLine = this->Lines->GetPointer(this->State->LineIndex);

		if (pLine->CodeBlockLevel > 0)
		{
			int iRPos = 0;

			if (Token(pLine->Text, pLine->Length, sTok, sizeof(sTok), &iRPos) > 0)
			{
				if (_StrEqlI(sTok, "Function"))
				{
					if (iProcEntryCount++ > 0)
					{
						return this->Error->Hard("Nested functions are not allowed");
					}
				}
				else if (_StrEqlI(sTok, "End"))
				{
					if (Token(pLine->Text, pLine->Length, sTok, sizeof(sTok), &iRPos) > 0)
					{
						if (_StrEqlI(sTok, "Function"))
						{
							return this->Error->HardCount();
						}
					}
				}
			}
		}
	}

	return this->Error->Hard(iStartingLine, "Expected: [End Function]");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CEngine::ParseIfLogic(LINEITEM *pLine)
{
	char sTok[MAX_TOKEN_SIZE];
	int iTokSz = 0;
	int iRPos = 0;

	if (Token(pLine->Text, pLine->Length, sTok, sizeof(sTok), &iRPos) <= 0)
	{
		return this->Error->Hard("Expected: [If]");
	}

	if (_StrEqlI(sTok, "Else"))
	{
		if (Token(pLine->Text, pLine->Length, sTok, sizeof(sTok), &iRPos) <= 0)
		{
			return this->Error->Hard("Expected: [If]");
		}
	}

	if (!_StrEqlI(sTok, "If"))
	{
		return this->Error->Hard("Expected: [If]");
	}

	SkipWhiteSpaces(pLine->Text, pLine->Length, &iRPos);

	if (pLine->Text[iRPos] != '(')
	{
		return this->Error->Hard("Expected: [(]");
	}
	else if (pLine->Text[pLine->Length - 1] != ')')
	{
		return this->Error->Hard("Expected: [)]");
	}

	CParams *pParams = NULL;
	ErrorLevel result = this->Evaluate(pLine->Text + iRPos, pLine->Length - iRPos, &pParams);
	if (result != ERROR_OK)
	{
		if (pParams)
		{
			delete pParams;
		}

		if (result > 0) //Hard error.
		{
			return result;
		}
		else {
			return CONDITION_SKIP;
		}
	}

	//We have to check for (pParams != null) because the conditional calls may have raised a soft exception.
	if (pParams)
	{
		result = CONDITION_FALSE;
		if (atof(pParams->Collection.Items[0].Value) != 0)
		{
			result = CONDITION_TRUE;
		}

		delete pParams;
	}
	else {
		return CONDITION_SKIP; //What else can we do?
	}

	return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CEngine::SkipStrictTypeBlock()
{
	char sTok[MAX_TOKEN_SIZE];
	int iTokSz = 0;
	int iRPos = 0;
	int iStartingLine = this->State->LineIndex;

	LINEITEM *pLine = this->Lines->GetPointer(this->State->LineIndex);

	if (Token(pLine->Text, pLine->Length, sTok, sizeof(sTok), &iRPos) <= 0)
	{
		return this->Error->Hard("Expected: [StrictType]");
	}

	this->State->LineIndex++;

	for (;this->State->LineIndex < this->Lines->Collection.Count; this->State->LineIndex++)
	{
		pLine = this->Lines->GetPointer(this->State->LineIndex);
		if (pLine->CodeBlockLevel > 0)
		{
			iRPos = 0;
			if (Token(pLine->Text, pLine->Length, sTok, sizeof(sTok), &iRPos) <= 0)
			{
				return this->Error->Hard("Expected: [Type]");
			}

			if (_StrEqlI(sTok, "End"))
			{
				if (Token(pLine->Text, pLine->Length, sTok, sizeof(sTok), &iRPos) > 0)
				{
					if (_StrEqlI(sTok, "StrictType"))
					{
						return this->Error->HardCount();
					}
				}
			}
		}
	}

	return this->Error->Hard(iStartingLine, "[Type] terminator not found");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CEngine::SkipIfBlock(bool bFullSkip)
{
	char sTok[MAX_TOKEN_SIZE];
	int iTokSz = 0;
	int iNestLevel = 0;
	int iRPos = 0;
	int iStartingLine = this->State->LineIndex;

	LINEITEM *pLine = this->Lines->GetPointer(this->State->LineIndex);

	if (Token(pLine->Text, pLine->Length, sTok, sizeof(sTok), &iRPos) > 0)
	{
		if (_StrEqlI(sTok, "Else"))
		{
			this->State->LineIndex++; //Skip the "Else"
			iNestLevel++; //Else statements are already in scope.
		}
	}

	for (;this->State->LineIndex < this->Lines->Collection.Count; this->State->LineIndex++)
	{
		pLine = this->Lines->GetPointer(this->State->LineIndex);
		if (pLine->CodeBlockLevel > 0)
		{
			iRPos = 0;

			if (Token(pLine->Text, pLine->Length, sTok, sizeof(sTok), &iRPos) > 0)
			{
				if (_StrEqlI(sTok, "If"))
				{
					iNestLevel++;
				}
				else if (_StrEqlI(sTok, "End"))
				{
					if (Token(pLine->Text, pLine->Length, sTok, sizeof(sTok), &iRPos) > 0)
					{
						if (_StrEqlI(sTok, "If"))
						{
							iNestLevel--;

							if (iNestLevel == 0)
							{
								this->State->ScopeDrop();
								return this->Error->HardCount();
							}
						}
					}
				}
				else if (!bFullSkip && _StrEqlI(sTok, "Else"))
				{
					if (iNestLevel == 1)
					{
						if (Token(pLine->Text, pLine->Length, sTok, sizeof(sTok), &iRPos) > 0)
						{
							if (_StrEqlI(sTok, "If")) //Else If
							{
								ErrorLevel Result = this->ParseIfLogic(pLine);

								if (Result == CONDITION_TRUE)
								{
									return this->Error->HardCount();
								}
								else if (Result == CONDITION_FALSE) {
									continue;
								}
							}
							else {
								this->Error->Hard("Unexpected: [%s]", sTok);
							}
						}

						return this->Error->HardCount();
					}
				}
			}
		}
	}

	return this->Error->Hard(iStartingLine, "[If] terminator not found");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CEngine::ParseWhileLogic(LINEITEM *pLine)
{
	char sTok[MAX_TOKEN_SIZE];
	int iTokSz = 0;
	int iRPos = 0;

	if (Token(pLine->Text, pLine->Length, sTok, sizeof(sTok), &iRPos) <= 0)
	{
		return this->Error->Hard("Expected: [While]");
	}

	if (!_StrEqlI(sTok, "While"))
	{
		return this->Error->Hard("Expected: [While]");
	}

	SkipWhiteSpaces(pLine->Text, pLine->Length, &iRPos);

	if (pLine->Text[iRPos] != '(')
	{
		return this->Error->Hard("Expected: [(]");
	}
	else if (pLine->Text[pLine->Length - 1] != ')')
	{
		return this->Error->Hard("Expected: [)]");
	}

	CParams *pParams = NULL;
	ErrorLevel Error = this->Evaluate(pLine->Text + iRPos, pLine->Length - iRPos, &pParams);
	if (Error != ERROR_OK)
	{
		if (pParams)
		{
			delete pParams;
		}
		return Error;
	}

	Error = CONDITION_FALSE;
	if (atof(pParams->Collection.Items[0].Value) > 0)
	{
		Error = CONDITION_TRUE;
	}

	delete pParams;

	return Error;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CEngine::SkipForBlock(void)
{
	char sTok[MAX_TOKEN_SIZE];
	int iTokSz = 0;
	int iNestLevel = 0;
	int iRPos = 0;
	int iStartingLine = this->State->LineIndex;

	LINEITEM *pLine = this->Lines->GetPointer(this->State->LineIndex);

	for (;this->State->LineIndex < this->Lines->Collection.Count; this->State->LineIndex++)
	{
		pLine = this->Lines->GetPointer(this->State->LineIndex);
		if (pLine->CodeBlockLevel > 0)
		{
			iRPos = 0;

			if (Token(pLine->Text, pLine->Length, sTok, sizeof(sTok), &iRPos) > 0)
			{
				if (_StrEqlI(sTok, "For"))
				{
					iNestLevel++;
				}
				else if (_StrEqlI(sTok, "Next"))
				{
					iNestLevel--;

					if (iNestLevel == 0)
					{
						return this->Error->HardCount();
					}
				}
			}
		}
	}

	return this->Error->Hard(iStartingLine, "[For] terminator not found");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CEngine::SkipWhileBlock(void)
{
	char sTok[MAX_TOKEN_SIZE];
	int iTokSz = 0;
	int iNestLevel = 0;
	int iRPos = 0;
	int iStartingLine = this->State->LineIndex;

	LINEITEM *pLine = this->Lines->GetPointer(this->State->LineIndex);

	for (;this->State->LineIndex < this->Lines->Collection.Count; this->State->LineIndex++)
	{
		pLine = this->Lines->GetPointer(this->State->LineIndex);
		if (pLine->CodeBlockLevel > 0)
		{
			iRPos = 0;

			if (Token(pLine->Text, pLine->Length, sTok, sizeof(sTok), &iRPos) > 0)
			{
				if (_StrEqlI(sTok, "While"))
				{
					iNestLevel++;
				}
				else if (_StrEqlI(sTok, "WEnd"))
				{
					iNestLevel--;

					if (iNestLevel == 0)
					{
						return this->Error->HardCount();
					}
				}
			}
		}
	}

	return this->Error->Hard(iStartingLine, "[If] terminator not found");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CEngine::Process(void)
{
	return this->Process(NULL);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// <summary>
/// Skips a single line of execution. Useful in manual coding mode when a line can't compile.
/// </summary>
/// <param name=""></param>
/// <returns></returns>
void CEngine::AdvanceLine(void)
{
	if (this->State->LineIndex < this->Lines->Collection.Count)
	{
		this->State->LineIndex++;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CEngine::Process(CParams *pReturn)
{
	for (;this->State->LineIndex < this->Lines->Collection.Count; this->State->LineIndex++)
	{
		LINEITEM *pLine = this->Lines->GetPointer(this->State->LineIndex);

		ErrorLevel iError = ERROR_OK;

		if (!this->Error->SoftCount() || !this->ThrowSoftExceptions)
		{
			if (pLine->CodeBlockLevel == 0)
			{
				if (pLine->IsInline)
				{
					printf("%s", pLine->Text);
				}
				else {
					printf("%s\n", pLine->Text);
				}
			}
			else {
				iError = this->ExecuteLineOfCode(pLine, pReturn);
				if (iError == EXECUTE_LOOP_END)
				{
					return EXECUTE_LOOP_END;
				}
				else if (iError == EXECUTE_RETURN)
				{
					return EXECUTE_RETURN; //this->Error->HardCount();
				}
				else if (iError != ERROR_OK)
				{
					return iError;
				}
			}
		}
		else
		{
			if (this->Error->SoftCount() > this->State->SoftErrorCountAtLastCatch)
			{
				if (this->State->TryScope && this->State->CurrentScope() == this->State->ScopeAtLastTry())
				{
					if ((iError = this->SkipToCatchBlock()) != ERROR_OK)
					{
						return iError;
					}
				}
				else if (!this->State->TryScope)
				{
					return this->Error->Throw(true);
				}
			}
		}
	}

	return this->Error->HardCount();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Note: (piEndPos - piBegPos) = -2 [means no square brackets]
*/
ErrorLevel CEngine::GetSquareBracketIndexPositions(const char *sText, int iLength, int *piRPos, int *piBegPos, int *piEndPos)
{
	//Get the beginning and ending position of square brackets.

	int iBraces = 0;
	int iRPos = (int)*piRPos;
	bool bInQuotes = false;

	if (piBegPos)
	{
		*piBegPos = (int)*piRPos;
		*piBegPos = ((int)(*piBegPos) + 1);
	}

	for (; iRPos < iLength; iRPos++)
	{
		if (sText[iRPos] == '\\')
		{
			iRPos++; //No need to parse escape sequence, we are only calculating.
			continue;
		}
		else if (sText[iRPos] == '\"') {
			bInQuotes = !bInQuotes;
		}
		else if (!bInQuotes && sText[iRPos] == '[')
		{
			iBraces++;
		}
		else if (!bInQuotes && sText[iRPos] == ']')
		{
			iBraces--;
		}

		if (iBraces == 0)
		{
			break;
		}
	}

	if (iRPos < iLength && sText[iRPos] == ']')
	{
		iRPos++;
	}

	if (iBraces != 0)
	{
		return this->Error->Hard("Square bracket mismatch");
	}

	if (piEndPos)
	{
		*piEndPos = (int)iRPos - 1;
	}
	if (piRPos)
	{
		*piRPos = (int)iRPos;
	}

	return this->Error->HardCount();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
Note: (piEndPos - piBegPos) = -2 [means no Parenthesis]
*/
ErrorLevel CEngine::GetParenIndexPositions(const char *sText, int iLength, int *piRPos, int *piBegPos, int *piEndPos)
{
	//Get the beginning and ending position of this procs parameters.

	int iBraces = 0;
	int iRPos = (int)*piRPos;
	bool bInQuotes = false;

	if (piBegPos)
	{
		*piBegPos = (int)*piRPos;
		*piBegPos = ((int)(*piBegPos) + 1);
	}

	for (; iRPos < iLength; iRPos++)
	{
		if (sText[iRPos] == '\\')
		{
			iRPos++; //No need to parse escape sequence, we are only calculating.
			continue;
		}
		else if (sText[iRPos] == '\"') {
			bInQuotes = !bInQuotes;
		}
		else if (!bInQuotes && sText[iRPos] == '(')
		{
			iBraces++;
		}
		else if (!bInQuotes && sText[iRPos] == ')')
		{
			iBraces--;
		}

		if (iBraces == 0)
		{
			break;
		}
	}

	if (iRPos < iLength && sText[iRPos] == ')')
	{
		iRPos++;
	}

	if (iBraces != 0)
	{
		return this->Error->Hard("Parenthesis mismatch");
	}

	if (piEndPos)
	{
		*piEndPos = (int)iRPos - 1;
	}
	if (piRPos)
	{
		*piRPos = (int)iRPos;
	}

	return this->Error->HardCount();
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CEngine::DoWhileLoop(CParams *pReturn)
{
	ErrorLevel Result = ERROR_OK;
	int iLogicLine = this->State->LineIndex;

	while (this->ParseWhileLogic(this->Lines->GetPointer(iLogicLine)) == CONDITION_TRUE)
	{
		this->State->LineIndex++; //Skip logic line.

		this->State->ScopeEnter();
		ErrorLevel Result = Process(pReturn);
		this->State->ScopeDrop();

		if (Result == EXECUTE_LOOP_END || Result == EXECUTE_LOOP_CONTINUE)
		{
			this->State->LineIndex = iLogicLine;
		}
		else if (Result == EXECUTE_LOOP_BREAK)
		{
			this->State->LineIndex = iLogicLine;
			return this->SkipWhileBlock();
		}
		else if (Result != ERROR_OK)
		{
			return Result;
		}
	}

	return this->SkipWhileBlock();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CEngine::DoForLoop(CParams *pReturn)
{
	ErrorLevel Result = ERROR_OK;
	int iLogicLine = this->State->LineIndex;

	char sTok[MAX_TOKEN_SIZE];
	char sVarName[MAX_VAR_NAME_SIZE];
	int iTokSz = 0;
	int iRPos = 0;
	int iVarIndex = INVALID_INDEX;
	bool bFoundTO = false;
	bool bLoopDeclaredVariable = false;
	int iTOIndex = 0;

	LINEITEM *pLine = this->Lines->GetPointer(iLogicLine);

	if (Token(pLine->Text, pLine->Length, sTok, sizeof(sTok), &iRPos) <= 0)
	{
		return this->Error->Hard("Expected: [For]");
	}

	if (!_StrEqlI(sTok, "For"))
	{
		return this->Error->Hard("Expected: [For]");
	}

	SkipWhiteSpaces(pLine->Text, pLine->Length, &iRPos);

	if (pLine->Text[iRPos] != '(')
	{
		return this->Error->Hard("Expected: [(]");
	}
	else if (pLine->Text[pLine->Length - 1] != ')')
	{
		return this->Error->Hard("Expected: [)]");
	}

	iRPos++; //Skip the '('

	SkipWhiteSpaces(pLine->Text, pLine->Length, &iRPos);

	//Get the variable name.
	if (Token(pLine->Text, pLine->Length, sVarName, sizeof(sVarName), &iRPos) <= 0)
	{
		return this->Error->Hard("Expected: [Variable Name]");
	}

	if (Token(pLine->Text, pLine->Length, sTok, sizeof(sTok), &iRPos) > 0)
	{
		if (_strcmpi(sTok, "as") == 0)
		{
			//Add new variable.
			char sVarType[MAX_TOKEN_SIZE];
			int iVarType = VAR_TYPE_INVALID;

			if (TokenWithDot(pLine->Text, pLine->Length, sVarType, sizeof(sVarType), &iRPos) <= 0)
			{
				return this->Error->Hard("Expected: [Type Name]");
			}

			if ((iVarType = this->Types->GetIndex(sVarType)) == VAR_TYPE_INVALID)
			{
				return this->Error->Hard("Expected: [Valid Type Name]");
			}

			if (this->Vars->Add(sVarName, iVarType, &iVarIndex) != 0)
			{
				return this->Error->HardCount();
			}

			bLoopDeclaredVariable = true;

			if (Token(pLine->Text, pLine->Length, sTok, sizeof(sTok), &iRPos) > 0)
			{
				if (_strcmpi(sTok, "to") == 0)
				{
					this->Vars->Set(iVarIndex, 0);

					iTOIndex = iRPos - 2;
					bFoundTO = true;
				}
				else {
					return this->Error->Hard("Expected: [to]");
				}
			}
		}
		else if (_strcmpi(sTok, "to") == 0)
		{
			//No need to set the variable.
			if ((iVarIndex = this->Vars->GetIndex(sVarName)) == INVALID_INDEX)
			{
				return this->Error->Hard("[%s] is undefined", sTok);
			}
			iTOIndex = iRPos - 2;
			bFoundTO = true;
		}
		else {
			return this->Error->Hard("Expected: [as], [to] or [=]");
		}
	}
	else {
		iVarIndex = this->Vars->GetIndex(sVarName);
	}

	if (!bFoundTO) //Find the index of the " TO ".
	{
		int iLength = 0;
		bool bInQuotes = false;
		bool bLastWasWhiteSpace = false;

		for (iTOIndex = iRPos; iTOIndex < pLine->Length - 3; iTOIndex++)
		{
			if (pLine->Text[iTOIndex] == '\\')
			{
				iTOIndex++; //No need to parse the escape sequence, we are searching for a non related string.
				bLastWasWhiteSpace = false;
				continue;
			}
			else if (pLine->Text[iTOIndex] == '\"') {
				bInQuotes = !bInQuotes;
			}

			if (!bInQuotes)
			{
				if (bLastWasWhiteSpace)
				{
					if (IsWhiteSpace(pLine->Text[iTOIndex + 2]) && _strnicmp(pLine->Text + iTOIndex, "to", 2) == 0)
					{
						//iTOIndex++;
						bFoundTO = true;
						break;
					}
				}

				bLastWasWhiteSpace = IsWhiteSpace(pLine->Text[iTOIndex]);
			}
		}

		if (!bFoundTO)
		{
			return this->Error->Hard("Expected: [to]");
		}
	}

	if (pLine->Text[iRPos] == '=')
	{
		iRPos++; //Skip the '='.

		SkipWhiteSpaces(pLine->Text, pLine->Length, &iRPos);

		if (this->Vars->SetByEval(iVarIndex, pLine->Text + iRPos, (iTOIndex - iRPos)) != ERROR_OK)
		{
			return this->Error->HardCount();
		}
	}

	if (bLoopDeclaredVariable)
	{
		this->Vars->GetPointer(iVarIndex)->Reserved = true;
	}

	ErrorLevel errorLevel = ForLoopSpin(pReturn, pLine, iTOIndex, iVarIndex, iLogicLine);

	if (bLoopDeclaredVariable)
	{
		this->Vars->GetPointer(iVarIndex)->Reserved = false;
	}

	if (this->SkipForBlock() != ERROR_OK)
	{
		return this->Error->HardCount();
	}

	return errorLevel;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CEngine::ForLoopSpin(CParams *pReturn, LINEITEM *pLine, int iTOIndex, int iVarIndex, int iLogicLine)
{
	CParams *pValue = NULL;
	double sToValue = 0;
	double dValue = atof(this->Vars->GetPointer(iVarIndex)->Value);
	double dStepBy = 0;

	int iStepRPos = 0;
	int iStepBeginPos = 0;
	int iStepEndPos = 0;

	bool bStepPositive = true;
	bool bForceDouble = false;

	ErrorLevel Result = ERROR_OK;

	while (true)
	{
		//Unfortunatly, the line address can change if the loop is injecting code.
		pLine = this->Lines->GetPointer(iLogicLine);

		//this->State->ScopeEnter();
		if ((Result = this->Evaluate(pLine->Text + iTOIndex + 2, pLine->Length - (iTOIndex + 3), &pValue)) != 0)
		{
			if (pValue)
			{
				delete pValue;
			}
			return Result;
		}

		sToValue = atof(pValue->First()->Value);
		delete pValue;

		if (dStepBy == 0)
		{
			if ((bStepPositive = dValue < sToValue))
			{
				dStepBy = 1;
			}
			else
			{
				dStepBy = -1;
			}
		}

		if ((bStepPositive && dValue < sToValue) || (!bStepPositive && dValue > sToValue))
		{
			this->State->LineIndex++; //Skip logic line.

			Result = Process(pReturn);

			if (Result == EXECUTE_LOOP_END || Result == EXECUTE_LOOP_CONTINUE)
			{
				if (this->Lines->Collection.Items[this->State->LineIndex].Length > 4)
				{
					if (iStepRPos == 0)
					{
						iStepRPos = 4; //Skip the "next".

						Result = GetParenIndexPositions(
							this->Lines->Collection.Items[this->State->LineIndex].Text,
							this->Lines->Collection.Items[this->State->LineIndex].Length,
							&iStepRPos, &iStepBeginPos, &iStepEndPos);
						if (Result != ERROR_OK)
						{
							return Result;
						}
					}

					if ((Result = this->Evaluate(this->Lines->Collection.Items[this->State->LineIndex].Text + iStepBeginPos, iStepEndPos - iStepBeginPos, &pValue)) != ERROR_OK)
					{
						if (pValue)
						{
							delete pValue;
						}
						return Result;
					}

					PARAMITEM *pStepBy = pValue->First();
					bForceDouble = (bForceDouble || (CharIndex(pStepBy->Value, pStepBy->Length, '.') >= 0));
					dStepBy = atof(pStepBy->Value);
					delete pValue;
				}

				this->State->LineIndex = iLogicLine;
				if (this->Debug)
				{
					if (this->Debug->IsBreakPoint(iLogicLine))
					{
						this->Debug->Break();
					}
				}
			}
			else if (Result == EXECUTE_LOOP_BREAK)
			{
				this->State->LineIndex = iLogicLine;
				return this->Error->HardCount();
			}
			else if (Result != ERROR_OK)
			{
				this->State->LineIndex = iLogicLine;
				return Result;
			}
		}
		else {
			break;
		}

		this->State->CycleScope();

		VARIABLEITEM *pVar = this->Vars->GetPointer(iVarIndex);
		bForceDouble = (bForceDouble || (CharIndex(pVar->Value, pVar->Length, '.') >= 0));

		dValue = atof(this->Vars->GetPointer(iVarIndex)->Value) + dStepBy;

		if (bForceDouble)
		{
			this->Vars->Set(pVar, dValue);
		}
		else {
			this->Vars->Set(pVar, (int)dValue);
		}
	}

	return this->Error->HardCount();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CEngine::SkipToCatchBlock(void)
{
	char sTok[MAX_TOKEN_SIZE];
	int iTokSz = 0;
	int iNestLevel = 0;
	int iRPos = 0;
	int iStartingLine = this->State->LineIndex;
	bool bDropped = false;

	LINEITEM *pLine = this->Lines->GetPointer(this->State->LineIndex);

	for (;this->State->LineIndex < this->Lines->Collection.Count; this->State->LineIndex++)
	{
		pLine = this->Lines->GetPointer(this->State->LineIndex);
		if (pLine->CodeBlockLevel > 0)
		{
			iRPos = 0;

			if (Token(pLine->Text, pLine->Length, sTok, sizeof(sTok), &iRPos) > 0)
			{
				if (_StrEqlI(sTok, "Try"))
				{
					iNestLevel++;
				}
				else if (_StrEqlI(sTok, "End"))
				{
					if (Token(pLine->Text, pLine->Length, sTok, sizeof(sTok), &iRPos) > 0)
					{
						if (_StrEqlI(sTok, "Try"))
						{
							iNestLevel--;
						}
					}
				}
				else if (_StrEqlI(sTok, "Catch"))
				{
					if (iNestLevel == 0)
					{
						this->State->TryDrop();
						this->State->TryEnter();

						int iLastSoftIndex = this->Error->LastSoftCollectionIndex();
						if (iLastSoftIndex == INVALID_INDEX)
						{
							return this->Error->Hard("No error is currently on the stack");
						}

						char sVarName[MAX_NAME_SIZE];

						if (pLine->Text[iRPos] == '(')
						{
							iRPos++; //Skip '('

							if (Token(pLine->Text, pLine->Length, sVarName, sizeof(sVarName), &iRPos) < 0)
							{
								return this->Error->Hard("Expected: [Variable Name]");
							}

							if (pLine->Text[iRPos] != ')')
							{
								return this->Error->Hard("Expected: \")\"");
							}

							iRPos++; //Skip ')'

							int iExceptionVarIndex = INVALID_INDEX;
							if (this->Vars->Add(sVarName, VAR_TYPE_EXCEPTION, &iExceptionVarIndex) != ERROR_OK)
							{
								return this->Error->HardCount();
							}

							VARIABLEITEM *pExVar = this->Vars->GetPointer(iExceptionVarIndex);
							if (pExVar)
							{
								EXCEPTIONITEM *pException = this->Error->GetPointer(iLastSoftIndex);

								pExVar->Length = sizeof(EXCEPTIONITEM);
								pExVar->Value = (char *) pMem->Allocate(sizeof(EXCEPTIONITEM), 1);
								memcpy_s(pExVar->Value, pExVar->Length, pException, pExVar->Length);

								int iMsgLength = (int)strlen(pException->Message);
								((EXCEPTIONITEM *)pExVar->Value)->Message = (char *) pMem->CloneString(pException->Message);
							}
							else {
								return this->Error->HardCount();
							}
						}

						this->Error->Clear();

						this->State->SoftErrorCountAtLastCatch = this->Error->SoftCount();
						return this->Error->HardCount();
					}
				}
			}
		}
	}

	return this->Error->Hard(iStartingLine, "[Try] terminator not found");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CEngine::SkipCatchBlock(void)
{
	char sTok[MAX_TOKEN_SIZE];
	int iTokSz = 0;
	int iNestLevel = 1;
	int iRPos = 0;
	int iStartingLine = this->State->LineIndex;

	LINEITEM *pLine = this->Lines->GetPointer(this->State->LineIndex);

	for (;this->State->LineIndex < this->Lines->Collection.Count; this->State->LineIndex++)
	{
		pLine = this->Lines->GetPointer(this->State->LineIndex);
		if (pLine->CodeBlockLevel > 0)
		{
			iRPos = 0;

			if (Token(pLine->Text, pLine->Length, sTok, sizeof(sTok), &iRPos) > 0)
			{
				if (_StrEqlI(sTok, "Try"))
				{
					iNestLevel++;
				}
				else if (_StrEqlI(sTok, "End"))
				{
					if (Token(pLine->Text, pLine->Length, sTok, sizeof(sTok), &iRPos) > 0)
					{
						if (_StrEqlI(sTok, "Try"))
						{
							iNestLevel--;

							if (iNestLevel == 0)
							{
								this->State->TryDrop();
								return this->Error->HardCount();
							}
						}
					}
				}
			}
		}
	}

	return this->Error->Hard(iStartingLine, "[Try] terminator not found");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CEngine::ParsePreprocessor(LINEITEM *pLine)
{
	char sTok[MAX_TOKEN_SIZE];
	int iTokSz = 0;
	int iRPos = 0;
	int iPosAfterHash = 0;

	if (pLine->Text[iRPos] == '#')
	{
		iPosAfterHash = ++iRPos; //Skip the '#';

		if (Token(pLine->Text, pLine->Length, sTok, sizeof(sTok), &iRPos) <= 0)
		{
			return this->Error->Hard("Invalid token");
		}
		else {
			if (_StrEqlI(sTok, "Define"))
			{
				return this->Vars->Add(pLine);
			}
			else if (_StrEqlI(sTok, "ContentType"))
			{
				CParams *pParam = NULL;
				this->Evaluate(pLine->Text + iRPos, pLine->Length - iRPos, &pParam);
				if (pParam)
				{
					if (pParam->Collection.Count == 1)
					{
						if (this->State->ContentTypeWritten)
						{
							delete pParam;
							return this->Error->Hard("Content-type was already specified");
						}
						this->State->ContentTypeWritten = true;
						printf("Content-Type: %s\n\n", pParam->ToString(0));
					}
					delete pParam;
				}

				return this->Error->HardCount();
			}
			else if (_StrEqlI(sTok, "Namespace"))
			{
				SCRIPTITEM *pScript = &this->Scripts->Collection.Items[pLine->ScriptIndex];

				if (pScript->Namespace)
				{
					return this->Error->Hard("A namespace has already been defined for this script [%s].", pScript->Namespace);
				}

				char sNamespace[MAX_PATH];

				CParams *pParam = NULL;
				ErrorLevel result = this->Evaluate(pLine->Text + iRPos, pLine->Length - iRPos, &pParam);
				if (pParam)
				{
					if (pParam->Collection.Count == 1)
					{
						strcpy_s(sNamespace, sizeof(sNamespace), pParam->ToString(0));
						pScript->Namespace = pMem->CloneString(pParam->ToString(0));
					}
					delete pParam;
				}

				return result;
			}
			else if (_StrEqlI(sTok, "Include"))
			{
				CParams *pParam = NULL;
				ErrorLevel result = this->Evaluate(pLine->Text + iRPos, pLine->Length - iRPos, &pParam);
				if (pParam)
				{
					if (pParam->Collection.Count == 1)
					{
						char sLocalFile[MAX_PATH];
						char sLibraryFile[MAX_PATH];

						//Do a bunch of magic to find the file in relation to the current script.
						strcpy_s(sLocalFile, sizeof(sLocalFile), pParam->ToString(0));

						if (FileAccess(sLocalFile, FExist))
						{
							this->Scripts->AddFile(sLocalFile);
						}
						else {
							CorrectReversePath(sLocalFile, sizeof(sLocalFile));
							if (true/*CharIndex(sLocalFile, '\\') < 0*/)
							{
								char sCurrentScriptPath[MAX_PATH];

								LINEITEM *pLine = &this->Lines->Collection.Items[this->State->LineIndex];
								SCRIPTITEM *pScript = &this->Scripts->Collection.Items[pLine->ScriptIndex];

								if (!GetFilePath(pScript->FileName, sCurrentScriptPath, sizeof(sCurrentScriptPath)))
								{
									delete pParam;
									return this->Error->Hard("Failed to parse current script file path");
								}

								AppendPaths(sLocalFile, sizeof(sLocalFile), sCurrentScriptPath, pParam->ToString(0));
							}

							//If the include file was found, include it - if it wasnt, then check the standard library folder.
							if (FileAccess(sLocalFile, FExist))
							{
								this->Scripts->AddFile(sLocalFile);
							}
							else {
								char sLibraryPath[MAX_PATH];
								DWORD dwLibraryPathSz = sizeof(sLibraryPath);

								if (Get_StringRegistryValue(HKEY_LOCAL_MACHINE, "SOFTWARE\\NetworkDLS\\Simple Scripting Engine", "Path", sLibraryPath, dwLibraryPathSz))
								{
									strcat_s(sLibraryPath, sizeof(sLibraryPath), "\\Library");

									AppendPaths(sLibraryFile, sizeof(sLibraryFile), sLibraryPath, pParam->ToString(0));

									if (FileAccess(sLibraryFile, FExist))
									{
										this->Scripts->AddFile(sLibraryFile);
									}
									else {
										this->Error->Soft("File not found: [%s]", sLocalFile);
										this->Error->Soft("File not found: [%s]", sLibraryFile);
									}
								}
								else {
									this->Error->Soft("File not found: [%s]", sLocalFile);
								}
							}
						}
					}
					delete pParam;
				}

				return result;
			}
			else if (_StrEqlI(sTok, "Undef"))
			{
				if ((iTokSz = Token(pLine->Text, pLine->Length, sTok, sizeof(sTok), &iRPos)) <= 0)
				{
					return this->Error->Hard("Expected: [Variable Name]");
				}

				return this->Vars->Free(sTok);
			}
			else if (_StrEqlI(sTok, "Debug"))
			{
				this->IntegrateWithDebugger();
				return this->Error->HardCount();
			}
			else {
				return this->Error->Hard("Unknown preprocessor: [%s]", sTok);
			}
		}
	}
	else {
		return this->Error->Hard("Expected: #");
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CEngine::ExecuteLineOfCode(LINEITEM *pLine, CParams *pReturn)
{
	char sNSBuf[MAX_TOKEN_SIZE];
	char sTok[MAX_TOKEN_SIZE];
	int iTokSz = 0;
	int iRPos = 0;

	int iIndex = INVALID_INDEX;
	int iArrayIndex = -1; //For array items.

	if (Token(pLine->Text, pLine->Length, sTok, sizeof(sTok), &iRPos) <= 0)
	{
		return this->Error->Hard("Invalid token");
	}
	else if (_StrEqlI(sTok, "Function"))
	{
		return this->SkipFunctionBlock();
	}

	strcpy_s(sNSBuf, sizeof(sNSBuf), sTok);
	while (pLine->Text[iRPos] == '.' && this->Scripts->GetPartialNameSpace(sNSBuf))
	{
		iRPos++;

		if ((iTokSz = Token(pLine->Text, pLine->Length, sTok, sizeof(sTok), &iRPos)) > 0)
		{
			strcat_s(sNSBuf, sizeof(sNSBuf), ".");
			strcat_s(sNSBuf, sizeof(sNSBuf), sTok);
		}

		if (pLine->Text[iRPos] != '.')
		{
			strcpy_s(sTok, sizeof(sTok), sNSBuf);
			break;
		}
	}

	if (this->Debug)
	{
		if (this->Debug->IsBreakPoint(pLine->LineIndex))
		{
			this->Debug->Break();
		}
	}

	if (pLine->Text[0] == '#')
	{
		return ERROR_OK;
	}
	else if (_StrEqlI(sTok, "End"))
	{
		if (Token(pLine->Text, pLine->Length, sTok, sizeof(sTok), &iRPos) > 0)
		{
			if (_StrEqlI(sTok, "Function"))
			{
				if (this->State->CurrentScope() == 0)
				{
					return this->Error->Hard("Scope mismatch: [Fucntion]");
				}
				return EXECUTE_RETURN;
			}
			else if (_StrEqlI(sTok, "If"))
			{
				if (this->State->CurrentScope() == 0)
				{
					return this->Error->Hard("Scope mismatch: [If]");
				}
				this->State->ScopeDrop();
				return ERROR_OK;
			}
			else if (_StrEqlI(sTok, "Try"))
			{
				if (this->State->CurrentScope() == 0 || this->State->TryScope == 0)
				{
					return this->Error->Hard("Scope mismatch: [Try]");
				}
				this->Error->Clear();
				this->State->TryDrop();
				return ERROR_OK;
			}
		}
		return this->Error->Hard("Unexpected: [End]");
	}
	else if (_StrEqlI(sTok, "WEnd") || _StrEqlI(sTok, "Next"))
	{
		return EXECUTE_LOOP_END;
	}
	else if (_StrEqlI(sTok, "While"))
	{
		return this->DoWhileLoop(pReturn);
	}
	else if (_StrEqlI(sTok, "For"))
	{
		this->State->ScopeEnter();
		ErrorLevel Error = this->DoForLoop(pReturn);
		this->State->ScopeDrop();
		return Error;
	}
	else if (_StrEqlI(sTok, "Break"))
	{
		return EXECUTE_LOOP_BREAK;
	}
	else if (_StrEqlI(sTok, "Continue"))
	{
		return EXECUTE_LOOP_CONTINUE;
	}
	else if (_StrEqlI(sTok, "Try"))
	{
		this->Error->Clear();
		this->State->TryEnter();
		return ERROR_OK;
	}
	else if (_StrEqlI(sTok, "Catch"))
	{
		if (this->State->CurrentScope() == 0 || this->State->TryScope == 0)
		{
			return this->Error->Hard("Scope mismatch: [Catch]");
		}
		return this->SkipCatchBlock();
	}
	else if (_StrEqlI(sTok, "Return"))
	{
		if (this->Procs->ExecuteByEval(pLine->Text, pLine->Length, NULL, pReturn) == ERROR_OK)
		{
			return EXECUTE_RETURN;
		}
		return this->Error->HardCount();
	}
	else if (_StrEqlI(sTok, "If"))
	{
		this->State->ScopeEnter();

		ErrorLevel Result = this->ParseIfLogic(pLine);

		if (Result == CONDITION_TRUE)
		{
			return ERROR_OK;
		}
		else if (Result == CONDITION_FALSE) {
			Result = this->SkipIfBlock(false);
		}
		else if (Result == CONDITION_SKIP) {
			Result = this->SkipIfBlock(true);
		}

		return Result;
	}
	else if (_StrEqlI(sTok, "Else"))
	{
		return this->SkipIfBlock(true);
	}
	else if (_StrEqlI(sTok, "Var"))
	{
		return this->Vars->Add(pLine);
	}
	else if (_StrEqlI(sTok, "Declare"))
	{
#ifdef EXTERNAL_PROCEDURE_CALL_IMPLEMENTATION
		return this->Procs->AddExternal(pLine);
#else //EXTERNAL_PROCEDURE_CALL_IMPLEMENTATION
		this->Error->Hard("External function declarations are not implemented");
#endif //EXTERNAL_PROCEDURE_CALL_IMPLEMENTATION
	}
	else if (_StrEqlI(sTok, "StrictType"))
	{
		if (this->Types->AddStrictType() == ERROR_OK)
		{
			return this->SkipStrictTypeBlock();
		}
		return this->Error->HardCount();
	}
	else if ((iIndex = this->Vars->GetIndex(sTok)) != INVALID_INDEX)
	{
		int iParentVariableNameEndPos = 0;
		StringBuilder MemberName;

		if (pLine->Text[iRPos] == '.')
		{
			strcpy_s(sNSBuf, sizeof(sNSBuf), sTok);
			while (pLine->Text[iRPos] == '.' && this->Scripts->GetPartialNameSpace(sNSBuf))
			{
				iRPos++;

				if ((iTokSz = Token(pLine->Text, pLine->Length, sTok, sizeof(sTok), &iRPos)) > 0)
				{
					strcat_s(sNSBuf, sizeof(sNSBuf), ".");
					strcat_s(sNSBuf, sizeof(sNSBuf), sTok);
				}

				if (pLine->Text[iRPos] != '.')
				{
					strcpy_s(sTok, sizeof(sTok), sNSBuf);
					break;
				}
			}

			//Parse and handle properties/namespaces (there are three places where this is done).
			StringBuilder Property;

			int iVarIndex = INVALID_INDEX;
			int iProcIndex = INVALID_INDEX;
			VARIABLEITEM *pVar = this->Vars->GetPointer(iIndex);

			//If adding other generic types, search for this:
			if (pVar->Type == VAR_TYPE_GENERIC)
			{
				Property.Append(pVar->Name);
			}
			else {
				Property.Append(this->Types->Name(pVar->Type));
			}

			iParentVariableNameEndPos = Property.Length;

			if (TokenWithDot(pLine->Text, pLine->Length, sTok, sizeof(sTok), &iRPos) > 0)
			{
				Property.Append(sTok);
			}

			iProcIndex = this->Procs->GetIndex(Property.Buffer);

			TYPEITEM *pType = this->Types->GetPointer(pVar->Type);
			if (pType->IsStrictType)
			{
				MemberName.Append((char*)(Property.Buffer + iParentVariableNameEndPos + 1));
				//This will be handled by code later in the process.
			}
#ifdef COM_PROCEDURE_CALL_IMPLEMENTATION
			else if (pVar->Type == VAR_TYPE_COM_OBJECT)
			{
				if (iProcIndex == INVALID_INDEX)
				{
					if (this->Procs->Add(Property.Buffer, PROC_TYPE_COM, VAR_TYPE_INVALID, NULL) == ERROR_OK)
					{
						iProcIndex = this->Procs->GetIndex(Property.Buffer);
					}
					else {
						return this->Error->HardCount();
					}
				}
			}
#endif //COM_PROCEDURE_CALL_IMPLEMENTATION
			else if (iProcIndex != INVALID_INDEX)
			{
				int iStartPos = iRPos;

				int iBegBrace = 0;
				int iEndBrace = 0;

				if (this->GetParenIndexPositions(pLine->Text, pLine->Length, &iRPos, &iBegBrace, &iEndBrace) != ERROR_OK)
				{
					return NULL;
				}

				if ((iEndBrace - iBegBrace) < 0)
				{
					this->Error->Hard("Parenthesis required for procedure call");
					return NULL;
				}

				Property.Append(pLine->Text + iStartPos, (iEndBrace - iStartPos) + 1);

				CParams Additional(this);
				Additional.AddIndex(iIndex);

				PROCITEM *pProc = this->Procs->GetPointer(iProcIndex);
				CParams Return(this, pProc->ReturnType);
				//We only enforce conversion with script procedures.
				Return.EnforceTypes = (pProc->Type == PROC_TYPE_SCRIPT);

				ErrorLevel Result = this->Procs->ExecuteByEval(Property.Buffer, Property.Length, &Additional, &Return);

				if (Return.Collection.Count > 1)
				{
					return this->Error->Hard("Array value returned which is not implemented");
				}
				return Result;
			}
			else if ((iVarIndex = this->Vars->GetIndex(Property.Buffer)) != INVALID_INDEX)
			{
				iIndex = iVarIndex;
			}
			else {
				this->Error->Hard("Unknown variable property");
				return NULL;
			}
		}

		if (pLine->Text[iRPos] == '[')
		{
			int iBegBrace = 0;
			int iEndBrace = 0;

			ErrorLevel result;

			if ((result = this->GetSquareBracketIndexPositions(pLine->Text, pLine->Length, &iRPos, &iBegBrace, &iEndBrace)) != ERROR_OK)
			{
				return result;
			}

			int iPreviousSoftErrors = this->Error->SoftCount();

			iArrayIndex = this->ParseMathI(pLine->Text + iBegBrace, iEndBrace - iBegBrace);
			if (this->Error->SoftCount() != iPreviousSoftErrors)
			{
				return this->Error->HardCount();
			}

			VARIABLEITEM *pParent = this->Vars->GetPointer(iIndex);

			if (pParent->Type == VAR_TYPE_BYTES)
			{
				if (iArrayIndex < 0 || iArrayIndex >= pParent->Alloc)
				{
					this->Error->Hard("Array index is out of bounds");
					return NULL;
				}
			}
			else if (pParent->Type == VAR_TYPE_ARRAY)
			{
				if (iArrayIndex < 0 || iArrayIndex >= pParent->ArrayDefinition.NumberOfElements)
				{
					this->Error->Hard("Array index is out of bounds");
					return NULL;
				}
			}
			else
			{
				this->Error->Hard("Variable does not support \"set\" indexing");
			}

			SkipWhiteSpaces(pLine->Text, pLine->Length, &iRPos);
		}

		if (_StrEql(pLine->Text + iRPos, "++"))
		{
			VARIABLEITEM *pVar = this->Vars->GetPointer(iIndex);

			if (pVar->Type == VAR_TYPE_ARRAY && iArrayIndex >= 0)
			{
				pVar = Vars->GetArraySubVariable(pVar, iArrayIndex);
			}

			if (pVar->Type != VAR_TYPE_NUMERIC)
			{
				return this->Error->Hard("[++] is only compatible with numeric types.");
			}

			return this->Vars->Set(pVar->Index, atof(pVar->Value) + 1);
		}
		else if (_StrEql(pLine->Text + iRPos, "--"))
		{
			VARIABLEITEM *pVar = this->Vars->GetPointer(iIndex);

			if (pVar->Type == VAR_TYPE_ARRAY && iArrayIndex >= 0)
			{
				pVar = Vars->GetArraySubVariable(pVar, iArrayIndex);
			}

			if (pVar->Type != VAR_TYPE_NUMERIC)
			{
				return this->Error->Hard("[--] is only compatible with numeric types.");
			}

			return this->Vars->Set(pVar->Index, atof(pVar->Value) - 1);
		}
		else if (strncmp(pLine->Text + iRPos, "+=", 2) == 0)
		{
			iRPos += 2; //Skip "+="
			CParams *pReturn = NULL;
			ErrorLevel Error = this->Evaluate(pLine->Text + iRPos, pLine->Length - iRPos, &pReturn);
			if (Error != ERROR_OK)
			{
				if (pReturn)
				{
					delete pReturn;
				}
				return Error;
			}

			VARIABLEITEM *pVar = this->Vars->GetPointer(iIndex);

			if (pVar->Type == VAR_TYPE_ARRAY && iArrayIndex >= 0)
			{
				pVar = Vars->GetArraySubVariable(pVar, iArrayIndex);
			}

			if (pVar->Type == VAR_TYPE_STRING)
			{
				PARAMITEM *pParamItem = pReturn->First();
				this->Vars->Append(pVar, pParamItem->Value, pParamItem->Length);
			}
			else if (pVar->Type == VAR_TYPE_NUMERIC)
			{
				Error = this->Vars->Set(pVar->Index, atof(pVar->Value) + atof(pReturn->First()->Value));
			}
			else {
				Error = this->Error->Hard("[+=] is only compatible with numeric and string types.");
			}

			delete pReturn;
			return Error;
		}
		else if (strncmp(pLine->Text + iRPos, "-=", 2) == 0)
		{
			iRPos += 2; //Skip "-="
			CParams *pReturn = NULL;
			ErrorLevel Error = this->Evaluate(pLine->Text + iRPos, pLine->Length - iRPos, &pReturn);
			if (Error != ERROR_OK)
			{
				if (pReturn)
				{
					delete pReturn;
				}
				return Error;
			}

			VARIABLEITEM *pVar = this->Vars->GetPointer(iIndex);

			if (pVar->Type == VAR_TYPE_ARRAY && iArrayIndex >= 0)
			{
				pVar = Vars->GetArraySubVariable(pVar, iArrayIndex);
			}

			if (pVar->Type == VAR_TYPE_NUMERIC)
			{
				Error = this->Vars->Set(pVar->Index, atof(pVar->Value) - atof(pReturn->First()->Value));
			}
			else {
				Error = this->Error->Hard("[-=] is only compatible with numeric types.");
			}

			delete pReturn;
			return Error;
		}
		else if (pLine->Text[iRPos] == '=')
		{
			if (_strnicmp("serviceStatus", pLine->Text, 13) != 0)
			{
				if (_strnicmp("enumValue", pLine->Text, 9) != 0)
				{
					if (_strnicmp("enumResult", pLine->Text, 10) != 0)
					{
						printf("");
					}
				}
			}

			iRPos++;
			SkipWhiteSpaces(pLine->Text, pLine->Length, &iRPos);

			if (MemberName.Length > 0)
			{
				return this->Vars->SetByEval(iIndex, MemberName.Buffer, pLine->Text + iRPos, pLine->Length - iRPos);
			}
			else
			{
				return this->Vars->SetByEval(iIndex, pLine->Text + iRPos, pLine->Length - iRPos, iArrayIndex);
			}
		}

		return this->Error->Hard("Unknown variable property");
	}
	else if ((iIndex = this->Procs->GetIndex(sTok)) != INVALID_INDEX)
	{
		SkipWhiteSpaces(pLine->Text, pLine->Length, &iRPos);

		if (pLine->Text[iRPos] != '(')
		{
			return this->Error->Hard("Expected: [(]");
		}
		else if (pLine->Text[pLine->Length - 1] != ')')
		{
			return this->Error->Hard("Expected: [)]");
		}

		PROCITEM *pProc = this->Procs->GetPointer(iIndex);
		CParams lpReturn(this, pProc->ReturnType);
		//We only enforce conversion with script procedures.
		lpReturn.EnforceTypes = (pProc->Type == PROC_TYPE_SCRIPT);

		ErrorLevel result = this->Procs->ExecuteByEval(pLine->Text, pLine->Length, NULL, &lpReturn);


		//Borrowed from ErrorLevel CVars::Set(VARIABLEITEM *pItem, const char *sMemberName, int iArrayIndex, CParams *pParams):
		//Functions that return complex types will have the return variable "reserved", but if we are here, that means that the user
		//	isn't using the return variable - so its going to be leaked if we dont clean it up.
		if (lpReturn.Collection.Count > 0)
		{
			TYPEITEM *pDestinationType = this->Types->GetPointer(lpReturn.Type);
			if (pDestinationType->IsComplex)
			{
				if (lpReturn.First()->Value[0] == '@')
				{
					int iSourceVarIndex = lpReturn.ToVarIndex(0);

					VARIABLEITEM *pSourceVar = this->Vars->GetPointer(iSourceVarIndex);

					if (pSourceVar->Reserved)
					{
						//Now that we have assumed the pointers of the variable, allow it to be freed.
						pSourceVar->Reserved = false;
						this->Vars->Free(pSourceVar);
					}
				}
			}
		}

		return result;

	}

	return this->Error->Hard("[%s] is undefined", sTok);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CEngine::ParseCGIPostData(const char *sNameSpace, const char *sData, int iDataLength)
{
	char *sLocalData = (char *)calloc(sizeof(char), iDataLength + 1);
	if (!sLocalData)
	{
		return this->Error->Hard("CGI memory allocation error for %d bytes", iDataLength);
	}
	memcpy_s(sLocalData, iDataLength + 1, sData, iDataLength);
	sLocalData[iDataLength] = '\0';

	ReplaceCharacter(sLocalData, iDataLength, '+', ' ');

	int iItems = 0;
	char **sItems = Split(sLocalData, iDataLength, &iItems, "&", 1);

	char sVarName[MAX_NAME_SIZE];

	for (int i = 0; i < iItems; i++)
	{
		int iEqualPosition = IndexOf(sItems[i], '=');
		if (iEqualPosition >= 0)
		{
			sItems[i][iEqualPosition] = '\0';

			char *sKey = sItems[i];
			char *sValue = sItems[i] + iEqualPosition + 1;

			URLDecode(sValue);

			sprintf_s(sVarName, sizeof(sVarName), "%s.%s", sNameSpace, sKey);
			this->Vars->AddAndSet(sVarName, VAR_TYPE_STRING, sValue, (int)strlen(sValue));

			//printf("<b>%s</b>=%s<br />", sKey, sValue);
		}
		else {
			char *sKey = sItems[i];
			char *sValue = "";

			sprintf_s(sVarName, sizeof(sVarName), "%s.%s", sNameSpace, sKey);
			this->Vars->AddAndSet(sVarName, VAR_TYPE_STRING, sValue, (int)strlen(sValue));
			//printf("<b>%s</b><br />", sItems[i]);
		}
	}

	FreeArray(sItems, iItems);

	free(sLocalData);
	return ERROR_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CEngine::InitializeCGIFormVariables(void)
{
	ErrorLevel Result = ERROR_OK;

	if (!this->FormVariablesLoaded)
	{
		this->FormVariablesLoaded = true;

		char *sRequestMethod = getenv("REQUEST_METHOD");

		if (_strcmpi(sRequestMethod, "GET") == 0)
		{
			char *sQueryString = getenv("QUERY_STRING");
			ParseCGIPostData("Query", sQueryString, strlen(sQueryString));
		}
		else if (_strcmpi(sRequestMethod, "POST") == 0)
		{
			unsigned int iContentLength = atoi(getenv("CONTENT_LENGTH"));
			if (iContentLength > 0)
			{
				char *sPostData = (char *)calloc(sizeof(char), iContentLength + 1);
				if (sPostData)
				{
					if ((fread(sPostData, sizeof(char), iContentLength, stdin)) == iContentLength)
					{
						sPostData[iContentLength] = '\0';
						ParseCGIPostData("Form", sPostData, iContentLength);
						free(sPostData);
					}
				}
			}
		}
	}

	return Result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CEngine::InitializeCGIEnvironment(void)
{


	if (getenv("GATEWAY_INTERFACE"))
	{
		//if(!this->State->ContentTypeWritten)
		//{
		//	printf("")
		//}
		this->InitializeCGIFormVariables();
	}

	return this->Error->HardCount();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CEngine::CEngine(void)
{
	srand(GetTickCount());

#ifdef ENGINE_DEBUG_FILE_NAME
fopen_s(&this->EngineDebugFile, ENGINE_DEBUG_FILE_NAME, "wb");
#endif

	this->ValidateCodeBeforeExecution = true;
	this->FormVariablesLoaded = false;
	this->ThrowSoftExceptions = true;
	this->MathDebugMode = false;
	this->Error = NULL;
	this->Scripts = NULL;
	this->Vars = NULL;
	this->Procs = NULL;
	this->Lines = NULL;
	this->Debug = NULL;
	this->State = NULL;
	this->Types = NULL;
	this->CommandLine = NULL;

	this->OriginalSTDOut = stdout;

	this->Error = new CException(this);
	if (this->Error->HardCount())
	{
		return;
	}

	this->State = new CState(this);
	if (this->Error->HardCount())
	{
		return;
	}

	this->Types = new CScriptTypes(this);
	if (this->Error->HardCount())
	{
		return;
	}

	this->Procs = new CProcs(this);
	if (this->Error->HardCount())
	{
		return;
	}

	this->Vars = new CVars(this);
	if (this->Error->HardCount())
	{
		return;
	}

	this->Lines = new CLines(this);
	if (this->Error->HardCount())
	{
		return;
	}

	this->Scripts = new CScripts(this);
	if (this->Error->HardCount())
	{
		return;
	}

	this->CommandLine = new Stack(BEHAVIOR_FILO);

	this->InitializeCGIEnvironment();

	this->Vars->_EndOfBuiltInIndex = this->Vars->Count();
	this->Procs->_EndOfBuiltInIndex = this->Procs->Count();

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CEngine::~CEngine()
{
	if (this->Error) this->Error->Throw();
	if (this->Scripts) delete this->Scripts;
	if (this->Lines) delete this->Lines;
	if (this->Vars) delete this->Vars;
	if (this->Types) delete this->Types;
	if (this->Procs) delete this->Procs;
	if (this->State) delete this->State;
	if (this->Error) delete this->Error;
	if (this->Debug) delete this->Debug;
	if (this->CommandLine) delete this->CommandLine;

#ifdef ENGINE_DEBUG_FILE_NAME
	if(this->EngineDebugFile != NULL)
	{
		fclose(this->EngineDebugFile);
	}
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Retreives a textual error message from a threads GetLastError() call.
	Returns TRUE if error information was received.
*/
bool CEngine::GetLastSystemError(HRESULT hResult, char *sOutErrorMessage, int iMaxOutErrorMessage)
{
	char *sErrorMessage = NULL;

	if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, hResult, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&sErrorMessage, 0, NULL))
	{
		Trim(sErrorMessage);
		strcpy_s(sOutErrorMessage, iMaxOutErrorMessage, sErrorMessage);
		LocalFree(sErrorMessage);
		return true;
	}

	strcpy_s(sOutErrorMessage, iMaxOutErrorMessage, "Unknown error");
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Retreives a textual error message from a threads GetLastError() call.
	Returns TRUE if error information was received.
*/
bool CEngine::GetLastSystemError(char *sOutErrorMessage, int iMaxOutErrorMessage)
{
	DWORD dwLastError = GetLastError();
	if (dwLastError)
	{
		char *sErrorMessage = NULL;

		if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			NULL, dwLastError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&sErrorMessage, 0, NULL))
		{
			Trim(sErrorMessage);
			strcpy_s(sOutErrorMessage, iMaxOutErrorMessage, sErrorMessage);
			LocalFree(sErrorMessage);
			return true;
		}

		strcpy_s(sOutErrorMessage, iMaxOutErrorMessage, "Unknown error");
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CEngine::ValidateCode(void)
{
	return this->ValidateCode(0, this->Lines->Collection.Count);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Ensures that scopes are properly aligned.

	Currently handles:
		IF / Else / End IF
		Try /Catch / End Try
		Function / End Function
		While / WEnd
		For / Next
*/
ErrorLevel CEngine::ValidateCode(int iLOCStart, int LOCEnd)
{
	char *sConstTok = NULL;

	char sTok[MAX_TOKEN_SIZE];
	int iTokSz = 0;
	int iRPos = 0;
	int iScope = 0;

#define CHECK_TYPE_IF		1
#define CHECK_TYPE_WHILE	2
#define CHECK_TYPE_TRY		3
#define CHECK_TYPE_FOR		4
#define CHECK_TYPE_FUNCTION	5

	typedef struct _tag_CheckStackItem {
		short Type;
		int Scope;
	} CHECKSTACKITEM, *LPCHECKSTACKITEM;

	Stack stack(BEHAVIOR_FILO);

	CHECKSTACKITEM CTI;
	memset(&CTI, 0, sizeof(CTI));

	Stack debugLineNumbers(BEHAVIOR_FILO);

	int iStartingLine = this->State->LineIndex;

	debugLineNumbers.Push(this->State->LineIndex);

	for (this->State->LineIndex = iLOCStart; this->State->LineIndex < LOCEnd; this->State->LineIndex++)
	{
		LINEITEM *pLine = this->Lines->GetPointer(this->State->LineIndex);

		if (pLine->CodeBlockLevel > 0)
		{
			iTokSz = 0;
			iRPos = 0;

			if ((iTokSz = Token(pLine->Text, pLine->Length, sTok, sizeof(sTok), &iRPos)) <= 0)
			{
				return this->Error->Hard(pLine->LineIndex, "Validation: Expected: A valid token");
			}

			if (_StrEqlI(sTok, "If"))
			{
				debugLineNumbers.Push(this->State->LineIndex);
				CTI.Type = CHECK_TYPE_IF;
				CTI.Scope = iScope;
				stack.Push(&CTI, sizeof(CTI));

				iScope++;
			}
			else if (_StrEqlI(pLine->Text, "Else"))
			{
				if (stack.Peek(&CTI))
				{
					if (CTI.Scope != iScope - 1 || CTI.Type != CHECK_TYPE_IF)
					{
						int exceptionLine = 0;
						debugLineNumbers.Pop(&exceptionLine);
						return this->Error->Hard(exceptionLine, "Validation: Code nesting error: [%s]", pLine->Text);
					}
				}
			}
			else if (_StrEqlI(pLine->Text, "End If"))
			{
				iScope--;

				if (stack.Pop(&CTI))
				{
					if (CTI.Scope != iScope || CTI.Type != CHECK_TYPE_IF)
					{
						int exceptionLine = 0;
						debugLineNumbers.Pop(&exceptionLine);
						return this->Error->Hard(exceptionLine, "Validation: Code nesting error: [%s]", pLine->Text);
					}
				}
				debugLineNumbers.Pop();
			}
			else if (_StrEqlI(sTok, "While"))
			{
				debugLineNumbers.Push(this->State->LineIndex);
				CTI.Type = CHECK_TYPE_WHILE;
				CTI.Scope = iScope;
				stack.Push(&CTI, sizeof(CTI));

				iScope++;
			}
			else if (_StrEqlI(pLine->Text, "WEnd"))
			{
				iScope--;

				if (stack.Pop(&CTI))
				{
					if (CTI.Scope != iScope || CTI.Type != CHECK_TYPE_WHILE)
					{
						int exceptionLine = 0;
						debugLineNumbers.Pop(&exceptionLine);
						return this->Error->Hard(exceptionLine, "Validation: Code nesting error: [%s]", pLine->Text);
					}
				}
				debugLineNumbers.Pop();
			}
			else if (_StrEqlI(sTok, "Try"))
			{
				debugLineNumbers.Push(this->State->LineIndex);
				CTI.Type = CHECK_TYPE_TRY;
				CTI.Scope = iScope;
				stack.Push(&CTI, sizeof(CTI));

				iScope++;
			}
			else if (_StrEqlI(pLine->Text, "Catch"))
			{
				if (stack.Peek(&CTI))
				{
					if (CTI.Scope != iScope - 1 || CTI.Type != CHECK_TYPE_TRY)
					{
						int exceptionLine = 0;
						debugLineNumbers.Pop(&exceptionLine);
						return this->Error->Hard(exceptionLine, "Validation: Code nesting error: [%s]", pLine->Text);
					}
				}
			}
			else if (_StrEqlI(pLine->Text, "End Try"))
			{
				iScope--;

				if (stack.Pop(&CTI))
				{
					if (CTI.Scope != iScope || CTI.Type != CHECK_TYPE_TRY)
					{
						int exceptionLine = 0;
						debugLineNumbers.Pop(&exceptionLine);
						return this->Error->Hard(exceptionLine, "Validation: Code nesting error: [%s]", pLine->Text);
					}
				}
				debugLineNumbers.Pop();
			}
			else if (_StrEqlI(sTok, "Function"))
			{
				debugLineNumbers.Push(this->State->LineIndex);
				CTI.Type = CHECK_TYPE_FUNCTION;
				CTI.Scope = iScope;
				stack.Push(&CTI, sizeof(CTI));

				iScope++;
			}
			else if (_StrEqlI(pLine->Text, "End Function"))
			{
				iScope--;

				if (stack.Pop(&CTI))
				{
					if (CTI.Scope != iScope || CTI.Type != CHECK_TYPE_FUNCTION)
					{
						int exceptionLine = 0;
						debugLineNumbers.Pop(&exceptionLine);
						return this->Error->Hard(exceptionLine, "Validation: Code nesting error: [%s]", pLine->Text);
					}
				}
				debugLineNumbers.Pop();
			}
			else if (_StrEqlI(sTok, "For"))
			{
				debugLineNumbers.Push(this->State->LineIndex);
				CTI.Type = CHECK_TYPE_FOR;
				CTI.Scope = iScope;
				stack.Push(&CTI, sizeof(CTI));

				iScope++;
			}
			else if (_StrEqlI(sTok, "Next"))
			{
				iScope--;

				if (stack.Pop(&CTI))
				{
					if (CTI.Scope != iScope || CTI.Type != CHECK_TYPE_FOR)
					{
						int exceptionLine = 0;
						debugLineNumbers.Pop(&exceptionLine);
						return this->Error->Hard(exceptionLine, "Validation: Code nesting error: [%s]", pLine->Text);
					}
				}
				debugLineNumbers.Pop();
			}
		}
		else if (pLine->CodeBlockLevel < 0)
		{
			int exceptionLine = 0;
			debugLineNumbers.Pop(&exceptionLine);
			return this->Error->Hard(exceptionLine, "Validation: Code nesting level fell below zero");
		}

		if (iScope < 0)
		{
			int exceptionLine = 0;
			debugLineNumbers.Pop(&exceptionLine);
			return this->Error->Hard(this->State->LineIndex, "Validation: Code scope fell below zero");
		}
	}

	if (iScope != 0)
	{
		int exceptionLine = 0;
		debugLineNumbers.Pop(&exceptionLine);
		return this->Error->Hard(exceptionLine, "Validation: Code scope mismatch");
	}

	this->State->LineIndex = iStartingLine;

	return this->Error->HardCount();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function is called when the script is executed (but not simply attached) from
		within the IDE because when the script is executed form with the IDE, the scripts
		execution must remain suspended until all break points are loaded.
*/
ErrorLevel CEngine::IntegrateWithDebugger(const char *sDebugInstanceName)
{
	if (!this->Debug)
	{
		this->Debug = new CDebug(this, sDebugInstanceName);
	}
	return this->Error->HardCount();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CEngine::IntegrateWithDebugger(void)
{
	if (!this->Debug)
	{
		this->Debug = new CDebug(this);
	}
	return this->Error->HardCount();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char CEngine::GetEscapedCharacter(char cCharacter)
{
	switch (tolower(cCharacter))
	{
	case 'r':
		return '\r';
	case 'n':
		return '\n';
	case '0':
		return '\0';
	case '\"':
		return '\"';
	case '\\':
		return '\\';
	case 'a':
		return '\a';
	case 'b':
		return '\b';
	case 'f':
		return '\f';
	case 't':
		return '\t';
	case 'v':
		return '\v';
	case '\'':
		return '\'';
	}

	this->Error->Hard("Invalid escape sequence: [%c]", cCharacter);

	return cCharacter;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CEngine::AreStringsEqual(char *sStr1, int iLength1, char *sStr2, int iLength2)
{
	if (iLength1 != iLength2)
	{
		return false;
	}
	for (int i = 0; i < iLength1; i++)
	{
		if (sStr1[i] != sStr2[i])
		{
			return false;
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CEngine::AreStringsEqualI(char *sStr1, int iLength1, char *sStr2, int iLength2)
{
	if (iLength1 != iLength2)
	{
		return false;
	}
	for (int i = 0; i < iLength1; i++)
	{
		if (tolower(sStr1[i]) != tolower(sStr2[i]))
		{
			return false;
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CEngine::ErrorOut(const char *sFormat, ...)
{
	va_list ArgList;
	va_start(ArgList, sFormat);

	int iMemoryRequired = _vscprintf(sFormat, ArgList);

#ifdef _USE_GLOBAL_MEMPOOL
	char *sBuf = (char *)pMem->Allocate(sizeof(char), iMemoryRequired + 1);
#else
	char *sBuf = (char *)calloc(sizeof(char), iMemoryRequired + 1);
#endif

	int iSz = _vsprintf_s_l(sBuf, iMemoryRequired + 1, sFormat, NULL, ArgList);
	va_end(ArgList);

	DWORD dwBtyesWritten = 0;

	if (this->Debug && this->Debug->IsDebuggerAttached)
	{
		WriteFile(this->Debug->hWriteErrPipe, sBuf, iSz, &dwBtyesWritten, NULL);
	}
	else {
		fprintf(stderr, "%s", sBuf);
	}

#ifdef _USE_GLOBAL_MEMPOOL
	pMem->Free(sBuf);
#else
	free(sBuf);
#endif

	return (int)dwBtyesWritten;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef ENGINE_DEBUG_FILE_NAME
/*
	Example Usage:
		WriteDebugLog("\t(%s): %s\r\n", __FUNCTION__, pLine->Text);
*/
void CEngine::WriteDebugLog_Internal(const char *sFormat, ...)
{
	va_list ArgList;
	va_start(ArgList, sFormat);

	int iMemoryRequired = _vscprintf(sFormat, ArgList);

#ifdef _USE_GLOBAL_MEMPOOL
	char *sBuf = (char *)pMem->Allocate(sizeof(char), iMemoryRequired + 1);
#else
	char *sBuf = (char *)calloc(sizeof(char), iMemoryRequired + 1);
#endif

	int iSz = _vsprintf_s_l(sBuf, iMemoryRequired + 1, sFormat, NULL, ArgList);
	va_end(ArgList);

	fprintf(EngineDebugFile, "%s", sBuf);
	fflush(EngineDebugFile);

#ifdef _USE_GLOBAL_MEMPOOL
	pMem->Free(sBuf);
#else
	free(sBuf);
#endif
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
