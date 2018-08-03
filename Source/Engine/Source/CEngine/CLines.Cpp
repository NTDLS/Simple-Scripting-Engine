#ifndef _CLines_CPP_
#define _CLines_CPP_
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>

#include "CEngine.H"
#include "CLines.H"

#include "../../../../NSWFL/NSWFL.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::String;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function is used to populate the Lines[] array
*/
CLines::CLines(void *lpEngine)
{
	this->pEngine = lpEngine;
	this->Error = ((CEngine *)lpEngine)->Error;

	memset(&this->Collection, 0, sizeof(this->Collection));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CLines::Add(const char *sPreservedWhitespace, int iPreservedWhitespaceLen, const char *sLine, int iLength, int iScriptIndex, int iLine, int iCodeBlockLevel, bool bIsInline, bool bAppendLewLine)
{
	int iOldLine = ((CEngine *)this->pEngine)->State->LineIndex;

	((CEngine *)this->pEngine)->State->LineIndex = this->Collection.Count;

	ErrorLevel result = AddBase(sPreservedWhitespace, iPreservedWhitespaceLen, sLine, iLength, iScriptIndex, iLine, iCodeBlockLevel, bIsInline, bAppendLewLine);

	((CEngine *)this->pEngine)->State->LineIndex = iOldLine;

	return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CLines::AddBase(const char *sPreservedWhitespace, int iPreservedWhitespaceLen, const char *sLine, int iLength, int iScriptIndex, int iLine, int iCodeBlockLevel, bool bIsInline, bool bAppendLewLine)
{
	char sTok[MAX_TOKEN_SIZE];
	int iTokSz = 0;

	CEngine *pEngine = (CEngine *) this->pEngine;

	if (iLength > 0)
	{
		if (this->Collection.Count == this->Collection.Alloc)
		{
			if (this->Collection.Alloc == 0)
			{
				this->Collection.Alloc = 10;
			}
			else {
				this->Collection.Alloc += this->Collection.Alloc / 2;
			}

			this->Collection.Items = (LINEITEM *)
				pMem->ReAllocate(this->Collection.Items, sizeof(LINEITEM), this->Collection.Alloc);
		}

		bool bInlinePrint = false;
		int iOriginalLength = iLength;
		bool bInQuotes = false;

		if (iCodeBlockLevel > 0)
		{
			//Search for end of line comments.
			for (int iIndex = 0; iIndex < iLength; iIndex++)
			{
				if (sLine[iIndex] == '\\')
				{
					continue; //No need to parse escape sequences, we are only looking for comments.
				}
				else if (sLine[iIndex] == '\"')
				{
					bInQuotes = !bInQuotes;
				}
				else if (sLine[iIndex] == ';' && !bInQuotes) //End of line comment.
				{
					iLength = iIndex;
					iOriginalLength = iIndex;
					while (IsWhiteSpace(sLine[iLength - 1]))
					{
						iLength--;
					}
					break;
				}
			}
		}

		if (iLength)
		{
			if (iCodeBlockLevel > 0 && sLine[0] == '=')
			{
				bInlinePrint = true;
				iLength += 7; //"Prints()" minus the '='
			}

			LINEITEM *pLine = &this->Collection.Items[this->Collection.Count];
			pLine->Length = iLength;
			pLine->ScriptIndex = iScriptIndex;
			pLine->FileLine = iLine + 1;
			pLine->LineIndex = this->Collection.Count++;
			pLine->CodeBlockLevel = iCodeBlockLevel;
			pLine->IsInline = bIsInline;

			if (bInlinePrint)
			{
				if (bAppendLewLine && sLine[iLength - 1] != '\n')
				{
					pLine->Length += 7; //For the  " & "CRLF""
				}
				pLine->Length += iPreservedWhitespaceLen + 5;
			}
			else
			{
				if (bAppendLewLine && sLine[iLength - 1] != '\n')
				{
					pLine->Length += 2; //For the CRLF
				}
				if (iCodeBlockLevel == 0)
				{
					pLine->Length += iPreservedWhitespaceLen;
				}
			}

			if ((pLine->Text = (char *) pMem->Allocate(sizeof(char), pLine->Length + 1)))
			{
				if (iCodeBlockLevel == 0 && bInlinePrint == false)
				{
					strncpy_s(pLine->Text, pLine->Length + 1, sPreservedWhitespace, iPreservedWhitespaceLen);
				}

				if (bInlinePrint)
				{
					strcpy_s(pLine->Text, pLine->Length + 1, "Prints(");
					if ((iOriginalLength - 1) == 0)
					{
						return this->Error->Soft("Nothing to print");
					}

					strcat_s(pLine->Text, pLine->Length + 1, "\"");
					strcat_s(pLine->Text, pLine->Length + 1, sPreservedWhitespace);
					strcat_s(pLine->Text, pLine->Length + 1, "\" & ");

					strncat_s(pLine->Text, pLine->Length + 1, sLine + 1, iOriginalLength - 1);

					if (bAppendLewLine)
					{
						strcat_s(pLine->Text, pLine->Length + 1, " & CRLF");
					}

					strcat_s(pLine->Text, pLine->Length + 1, ")");
				}
				else
				{
					strncat_s(pLine->Text, pLine->Length + 1, sLine, iLength);
					if (bAppendLewLine)
					{
						strcat_s(pLine->Text, pLine->Length + 1, "\r\n");
					}
				}
			}
			else {
				return this->Error->Hard("Memory allocation error");
			}

			/*
			if (pLine->Length != strlen(pLine->Text))
			{
				printf("%d / %d\n", pLine->Length, strlen(pLine->Text));
			}
			pLine->Length = strlen(pLine->Text);
			*/

			if (iCodeBlockLevel > 0)
			{
				if (Token(pLine->Text, pLine->Length, sTok, sizeof(sTok), 0) > 0)
				{
					if (_StrEqlI(sTok, "Function"))
					{
						return ((CEngine *)this->pEngine)->Procs->AddScript(pLine);
					}
				}

				if (iCodeBlockLevel > 0 && sLine[0] == '#')
				{
					if (((CEngine *)this->pEngine)->ParsePreprocessor(pLine) != ERROR_OK)
					{
						return this->Error->HardCount();
					}
				}
			}
		}
	}
	return this->Error->HardCount();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CLines::LoadFromScript(int iScriptIndex)
{
	CEngine *pEngine = (CEngine *)this->pEngine;

	int iOldLine = pEngine->State->LineIndex;
	ErrorLevel Result = LoadFromScriptEx(iScriptIndex);
	pEngine->State->LineIndex = iOldLine;
	return Result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorLevel CLines::LoadFromScriptEx(int iScriptIndex)
{
	CEngine *pEngine = (CEngine *)this->pEngine;
	char *sFileName = pEngine->Scripts->GetPointer(iScriptIndex)->FileName;
	FILE *hFile = NULL;

	if (fopen_s(&hFile, sFileName, "rb") != 0)
	{
		return this->Error->Soft("File not found: [%s]", sFileName);
	}

	fseek(hFile, 0, SEEK_END);
	int iFileSize = ftell(hFile);
	fseek(hFile, 0, SEEK_SET);

	if (iFileSize == 0)
	{
		return this->Error->HardCount(); //File is empty.
	}

	char *sBuffer = (char *) pMem->Allocate(sizeof(char), iFileSize + 1);
	if (!sBuffer)
	{
		fclose(hFile);
		return this->Error->Hard("Memory allocation error");
	}

	if (fread(sBuffer, sizeof(char), iFileSize, hFile) != iFileSize)
	{
		free(sBuffer);
		fclose(hFile);
		return this->Error->Hard("File read error");
	}
	fclose(hFile);

	char **sPreservedWhitespace = NULL;
	char **sLines = NULL;
	int iLines = 0;
	int iActualLines = 0;

	if (!(sLines = Split(sBuffer, iFileSize, &iLines, "\n")))
	{
		pMem->Free(sBuffer);
		return this->Error->Hard("File contains data but no lines");
	}
	sPreservedWhitespace = (char**)pMem->Allocate(sizeof(char*), iLines);
	pMem->Free(sBuffer);

	int *iLengths = (int *)pMem->Allocate(sizeof(int), iLines);
	int *iPreservedWhitespaceLen = (int *)pMem->Allocate(sizeof(int), iLines);

	//Calculate lengths, clean whitespaces and determine how many non-blank lines we have.
	for (int iLine = 0; iLine < iLines; iLine++)
	{
		while (IsWhiteSpace(sLines[iLine][iPreservedWhitespaceLen[iLine]]))
		{
			iPreservedWhitespaceLen[iLine]++;
		}

		sPreservedWhitespace[iLine] = (char*)pMem->Allocate(sizeof(char), iPreservedWhitespaceLen[iLine] + 1);
		strncpy_s(sPreservedWhitespace[iLine], iPreservedWhitespaceLen[iLine] + 1, sLines[iLine], iPreservedWhitespaceLen[iLine]);

		iLengths[iLine] = CleanWhiteSpaces(sLines[iLine]);

		if (sLines[iLine][0] == ';')
		{
			iLengths[iLine] = 0;
		}
		else if (_StrEql(sLines[iLine], "<%"))
		{
		}
		else if (_StrEql(sLines[iLine], "%>"))
		{
		}
		else if (iLengths[iLine] > 0)
		{
			iActualLines++;
		}
	}

	if (iActualLines)
	{
		StringBuilder Line;
		int iStartingLine = 0;

		int iCodeBlockLevel = 0;

		for (int iLine = 0; iLine < iLines; iLine++)
		{
			pEngine->State->LineIndex = this->Collection.Count;
			Line.Clear();

			if (iLengths[iLine] > 0)
			{
				if (sLines[iLine][iLengths[iLine] - 1] == '_')
				{
					sLines[iLine][iLengths[iLine] - 1] = '\0'; //Trim off the underscore.
					iLengths[iLine] = CleanWhiteSpaces(sLines[iLine], iLengths[iLine] - 1);

					Line.Append(sLines[iLine], iLengths[iLine]);
					//Line.Append(" ", 1);

					iLine++;

					while (sLines[iLine][iLengths[iLine] - 1] == '_')
					{
						sLines[iLine][iLengths[iLine] - 1] = '\0'; //Trim off the underscore.
						iLengths[iLine] = CleanWhiteSpaces(sLines[iLine], iLengths[iLine] - 1);

						Line.Append(sLines[iLine], iLengths[iLine]);

						iLine++;
					}

					if (iLine < iLines)
					{
						Line.Append(sLines[iLine], iLengths[iLine]);
					}
				}
				else {
					Line.Append(sLines[iLine], iLengths[iLine]);
				}
			}

			if (Line.Length > 0)
			{
				iStartingLine = iLine;
				if (_StrEql(Line.Buffer, "<%"))
				{
					iCodeBlockLevel++;
					if (iCodeBlockLevel < 0 || iCodeBlockLevel > 1)
					{
						pMem->Free(iLengths);
						pMem->Free(iPreservedWhitespaceLen);
						FreeArray(sLines, iLines);
						FreeArray(sPreservedWhitespace, iLines);
						this->Add(sPreservedWhitespace[iLine], iPreservedWhitespaceLen[iLine], Line.Buffer, Line.Length, iScriptIndex, iStartingLine, iCodeBlockLevel, false, false); //Just add for debugging purposes.
						return this->Error->Hard("Code block nesting error, nesting is not allowed");
					}
					continue;
				}
				else if (_StrEql(Line.Buffer, "%>"))
				{
					iCodeBlockLevel--;
					if (iCodeBlockLevel < 0 || iCodeBlockLevel > 1)
					{
						pMem->Free(iLengths);
						pMem->Free(iPreservedWhitespaceLen);
						FreeArray(sLines, iLines);
						FreeArray(sPreservedWhitespace, iLines);
						this->Add(sPreservedWhitespace[iLine], iPreservedWhitespaceLen[iLine], Line.Buffer, Line.Length, iScriptIndex, iStartingLine, iCodeBlockLevel, false, false); //Just add for debugging purposes.
						return this->Error->Hard(pEngine->State->LineIndex, "Code block nesting error, nesting is not allowed");
					}
					continue;
				}

				int iCodeTagIn = InStr("<%", 2, Line.Buffer, Line.Length, 0);
				int iCodeTagOut = InStr("%>", 2, Line.Buffer, Line.Length, 0);

				if (iCodeTagIn >= 0 && iCodeTagOut >= 0)
				{
					if (!this->SplitComplexInlines(sPreservedWhitespace[iLine], iPreservedWhitespaceLen[iLine], Line.Buffer, Line.Length, iScriptIndex, iStartingLine, &iCodeBlockLevel))
					{
						pMem->Free(iLengths);
						pMem->Free(iPreservedWhitespaceLen);
						FreeArray(sLines, iLines);
						FreeArray(sPreservedWhitespace, iLines);
						this->Add(sPreservedWhitespace[iLine], iPreservedWhitespaceLen[iLine], Line.Buffer, Line.Length, iScriptIndex, iStartingLine, iCodeBlockLevel, false, false); //Just add for debugging purposes.
						return this->Error->Hard("Failed to parse complex inline code blocks");
					}
				}
				else if (iCodeTagIn >= 0)
				{
					int iLastStart = 0;
					int iLastEnd = 0;
					int iStart = 0;
					int iEnd = 0;

					while ((iStart = InStr("<%", 2, Line.Buffer, Line.Length, iEnd)) >= 0)
					{
						this->Add(sPreservedWhitespace[iLine], iPreservedWhitespaceLen[iLine], Line.Buffer + iLastEnd, iStart - iLastEnd, iScriptIndex, iStartingLine, iCodeBlockLevel, true, false);

						iCodeBlockLevel++;
						if (iCodeBlockLevel < 0 || iCodeBlockLevel > 1)
						{
							pMem->Free(iLengths);
							pMem->Free(iPreservedWhitespaceLen);
							FreeArray(sLines, iLines);
							FreeArray(sPreservedWhitespace, iLines);
							this->Add(sPreservedWhitespace[iLine], iPreservedWhitespaceLen[iLine], Line.Buffer, Line.Length, iScriptIndex, iStartingLine, iCodeBlockLevel, false, false); //Just add for debugging purposes.
							return this->Error->Hard("Code block nesting error, nesting is not allowed");
						}

						iLastStart = iStart;
						iStart += 2; //Skip "<%".

						if (((iEnd = InStr("%>", 2, Line.Buffer, Line.Length, iStart))) >= 0)
						{
							this->Add(sPreservedWhitespace[iLine], iPreservedWhitespaceLen[iLine], Line.Buffer + iStart, iEnd - iStart, iScriptIndex, iStartingLine, iCodeBlockLevel, true, false);

							iCodeBlockLevel--;
							if (iCodeBlockLevel < 0 || iCodeBlockLevel > 1)
							{
								pMem->Free(iLengths);
								pMem->Free(iPreservedWhitespaceLen);
								FreeArray(sLines, iLines);
								FreeArray(sPreservedWhitespace, iLines);
								this->Add(sPreservedWhitespace[iLine], iPreservedWhitespaceLen[iLine], Line.Buffer, Line.Length, iScriptIndex, iStartingLine, iCodeBlockLevel, false, false); //Just add for debugging purposes.
								return this->Error->Hard("Code block nesting error, nesting is not allowed");
							}
							iEnd += 2; //Skip "%>".
						}
						else {
							iEnd = iStart;
						}

						iLastEnd = iEnd;
					}

					this->Add(sPreservedWhitespace[iLine], iPreservedWhitespaceLen[iLine], Line.Buffer + iLastEnd, Line.Length - iLastEnd, iScriptIndex, iStartingLine, iCodeBlockLevel, false, false);
				}
				else if (iCodeTagOut >= 0)
				{
					int iIndex = 0;

					iIndex = InStr("%>", 2, Line.Buffer, Line.Length, 0);

					this->Add(sPreservedWhitespace[iLine], iPreservedWhitespaceLen[iLine], Line.Buffer, iIndex, iScriptIndex, iStartingLine, iCodeBlockLevel, false, false);

					iCodeBlockLevel--;
					if (iCodeBlockLevel < 0 || iCodeBlockLevel > 1)
					{
						pMem->Free(iLengths);
						pMem->Free(iPreservedWhitespaceLen);
						FreeArray(sLines, iLines);
						FreeArray(sPreservedWhitespace, iLines);
						this->Add(sPreservedWhitespace[iLine], iPreservedWhitespaceLen[iLine], Line.Buffer, Line.Length, iScriptIndex, iStartingLine, iCodeBlockLevel, false, false); //Just add for debugging purposes.
						return this->Error->Hard("Code block nesting error, nesting is not allowed");
					}

					iIndex += 2; //Skip "%>".

					if (this->Add(sPreservedWhitespace[iLine], iPreservedWhitespaceLen[iLine], Line.Buffer + iIndex, Line.Length - iIndex, iScriptIndex, iStartingLine, iCodeBlockLevel, false, false) != ERROR_OK)
					{
						((CEngine *)this->pEngine)->State->LineIndex = 0;
						break;
					}
					((CEngine *)this->pEngine)->State->LineIndex = 0;
				}
				else {

					if (iCodeBlockLevel == 0)
					{
						if (this->Add(sPreservedWhitespace[iLine], iPreservedWhitespaceLen[iLine], Line.Buffer, Line.Length, iScriptIndex, iStartingLine, iCodeBlockLevel, false, false) != ERROR_OK)
						{
							((CEngine *)this->pEngine)->State->LineIndex = 0;
							break;
						}
					}
					else
					{
						if (this->Add(sPreservedWhitespace[iLine], iPreservedWhitespaceLen[iLine], Line.Buffer, Line.Length, iScriptIndex, iStartingLine, iCodeBlockLevel, false, false) != ERROR_OK)
						{
							((CEngine *)this->pEngine)->State->LineIndex = 0;
							break;
						}
					}
					((CEngine *)this->pEngine)->State->LineIndex = 0;
				}

				if (iCodeBlockLevel < 0 || iCodeBlockLevel > 1)
				{
					pMem->Free(iLengths);
					pMem->Free(iPreservedWhitespaceLen);
					FreeArray(sLines, iLines);
					FreeArray(sPreservedWhitespace, iLines);
					return this->Error->Hard("Code block nesting error, nesting is not allowed");
				}
			}
		}
		Line.Destroy();
	}

	pMem->Free(iLengths);
	pMem->Free(iPreservedWhitespaceLen);
	FreeArray(sLines, iLines);
	FreeArray(sPreservedWhitespace, iLines);

	return this->Error->HardCount();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CLines::SplitComplexInlines(const char *sPreservedWhitespace, int iPreservedWhitespaceLen, const char *sLine, int iLength, int iScriptIndex, int iStartingLine, int *piCodeBlockLevel)
{
#ifdef _DEBUG
	if (strcmp("<%=tableName%>", sLine) == 0)
	{
		printf("");
	}
#endif

	int iTagIn = 0;
	int iTagOut = 0;
	int iLastStart = 0;
	int iStart = 0;
	int iEnd = 0;
	int iCodeBlockLevel = (int)*piCodeBlockLevel;
	int iFirstChar = 0;
	int iLastChar = 0;

	//char sDebug[5120];
	//memset(sDebug, 0, sizeof(sDebug));

	do {
		iTagIn = InStr("<%", 2, sLine, iLength, iStart);
		iTagOut = InStr("%>", 2, sLine, iLength, iStart);

		if ((iTagOut < iTagIn && iTagOut >= 0) || (iTagOut >= 0 && iTagIn < 0)) //Tag [out] first.
		{
			iEnd = iTagOut;

			if (iEnd - iLastStart > 0)
			{
				if (iCodeBlockLevel > 0)
				{
					iFirstChar = GetFirstCharPos(sLine + iLastStart, iEnd - iLastStart);
					if ((iLastChar = GetLastCharPos(sLine + iLastStart, iEnd - iLastStart)) > 0)
					{
						iLastChar = (iLastChar + 1) - iFirstChar;
					}

					iLastStart += iFirstChar;
				}
				else {
					iFirstChar = 0;
					iLastChar = iEnd - iLastStart;
				}

				//strncpy_s(sDebug, sizeof(sDebug), sLine + iLastStart, iLastChar);

				//Is the entire line an "inline"?
				if (iLastChar + 4 == iLength)
				{
					this->Add(sPreservedWhitespace, iPreservedWhitespaceLen, sLine + iLastStart, iLastChar, iScriptIndex, iStartingLine, iCodeBlockLevel, true, true);
				}
				else
				{
					this->Add("", 0, sLine + iLastStart, iLastChar, iScriptIndex, iStartingLine, iCodeBlockLevel, true, false);
				}

				//printf("Code:[%d]/[%s]\n", iCodeBlockLevel, sDebug);
			}
			iCodeBlockLevel--;
			iStart = iTagOut + 2;
		}
		else if (iTagIn >= 0) { //Tag [in] first.
			iEnd = iTagIn;

			if (iEnd - iLastStart > 0)
			{
				if (iCodeBlockLevel > 0)
				{
					iFirstChar = GetFirstCharPos(sLine + iLastStart, iEnd - iLastStart);
					if ((iLastChar = GetLastCharPos(sLine + iLastStart, iEnd - iLastStart)) > 0)
					{
						iLastChar = (iLastChar + 1) - iFirstChar;
					}

					iLastStart += iFirstChar;
				}
				else {
					iFirstChar = 0;
					iLastChar = iEnd - iLastStart;
				}

				//strncpy_s(sDebug, sizeof(sDebug), sLine + iLastStart, iLastChar);
				this->Add(sPreservedWhitespace, iPreservedWhitespaceLen, sLine + iLastStart, iLastChar, iScriptIndex, iStartingLine, iCodeBlockLevel, true, false);
				//printf("Code:[%d]/[%s]\n", iCodeBlockLevel, sDebug);
			}
			iCodeBlockLevel++;
			iStart = iTagIn + 2;
		}
		else {
			if (iLength - iLastStart > 0)
			{
				if (iCodeBlockLevel > 0)
				{
					iFirstChar = GetFirstCharPos(sLine + iLastStart, iLength - iLastStart);
					if ((iLastChar = GetLastCharPos(sLine + iLastStart, iLength - iLastStart)) > 0)
					{
						iLastChar = (iLastChar + 1) - iFirstChar;
					}

					iLastStart += iFirstChar;
				}
				else {
					iFirstChar = 0;
					iLastChar = iLength - iLastStart;
				}

				//strncpy_s(sDebug, sizeof(sDebug), sLine + iLastStart, iLastChar);
				this->Add("", 0, sLine + iLastStart, iLastChar, iScriptIndex, iStartingLine, iCodeBlockLevel, true, true);
				//printf("Code:[%d]/[%s]\n", iCodeBlockLevel, sDebug);
			}
		}

		iLastStart = iStart;
	} while (iTagIn >= 0 || iTagOut >= 0);

	*piCodeBlockLevel = iCodeBlockLevel;

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CLines::IsDefined(int iIndex)
{
	return (iIndex >= 0 && iIndex < this->Collection.Count && this->Collection.Items[iIndex].Text);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

LINEITEM *CLines::GetPointer(int iIndex)
{
	if (this->IsDefined(iIndex))
	{
		return &this->Collection.Items[iIndex];
	}

	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CLines::~CLines()
{
	if (this->Collection.Alloc)
	{
		for (int iItem = 0; iItem < this->Collection.Count; iItem++)
		{
			pMem->Free(this->Collection.Items[iItem].Text);
		}
		pMem->Free(this->Collection.Items);
	}
	memset(&this->Collection, 0, sizeof(this->Collection));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

