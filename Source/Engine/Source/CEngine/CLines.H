#ifndef _CLine_H_
#define _CLine_H_
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CException.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _tag_CScript_Line_Item {
	char *Text;			//The line text.
	int Length;			//The length of the line text.
	int ScriptIndex;	//The index of the originating script.
	int FileLine;		//The line in the actual file from which the text originated.
	int LineIndex;		//The index of the line in the line array.
	int CodeBlockLevel;	//The nested level of <%%> codeblocks
	bool IsInline;		//Was this line derrived from an inline statement.
} LINEITEM, *LPLINEITEM;

typedef int LineIndex;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _tag_CScript_Line_Collection {
	int Count;
	int Alloc;
	LINEITEM *Items;
} LINECOLLECTION, *LPLINECOLLECTION;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CLines {

public:
	CLines(void *lpEngine);
	~CLines();
	CException *Error;
	LINECOLLECTION Collection;

	LINEITEM *GetPointer(int iIndex);
	bool IsDefined(int iIndex);
	ErrorLevel LoadFromScript(int iScriptIndex);
	ErrorLevel LoadFromScriptEx(int iScriptIndex);
	ErrorLevel Add(const char *sPreservedWhitespace, int iPreservedWhitespaceLen, const char *sLine, int iLength, int iScriptIndex, int iLine, int iCodeBlockLevel, bool bIsInline, bool bAppendLewLine);
	bool SplitComplexInlines(const char *sPreservedWhitespace, int iPreservedWhitespaceLen, const char *sLine, int iLength, int iScriptIndex, int iStartingLine, int *piCodeBlockLevel);

private:
	ErrorLevel AddBase(const char *sPreservedWhitespace, int iPreservedWhitespaceLen, const char *sLine, int iLength, int iScriptIndex, int iLine, int iCodeBlockLevel, bool bIsInline, bool bAppendLewLine);

	void *pEngine; //Pointer to the owning CEngine class.
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

