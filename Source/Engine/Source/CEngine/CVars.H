#ifndef _CVariable_H_
#define _CVariable_H_
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../../../Shared/CSQLBinding/CSQL.H"
#include "../../../../Shared/CSocketServer/CSocketServer.H"

#include "CException.H"
#include "CLines.H"
#include "CParams.H"
#include "CReqParams.H"
#include "CState.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define FRIENDLY_DEBUG_VAR_SIZE   150
#define QUICKWATCH_DEBUG_VAR_SIZE 8*1024

#define PARENT_INDEX_VAR_NAME	"%ParentIndex%"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define STRICT_TYPE_INVALID			INVALID_INDEX
#define STRICT_TYPE_INTEGER16		1
#define STRICT_TYPE_INTEGER32		3
#define STRICT_TYPE_INTEGER64		4
#define STRICT_TYPE_UINTEGER16		5
#define STRICT_TYPE_UINTEGER32		6
#define STRICT_TYPE_UINTEGER64		7
#define STRICT_TYPE_FLOAT			8
#define STRICT_TYPE_DOUBLE			9
#define STRICT_TYPE_STRING			10 //Resizable and NULL terminated.
#define STRICT_TYPE_BYTE			11 //NOT resizable and are NOT NULL terminated.
#define STRICT_TYPE_SQLCONNECTION	12
#define STRICT_TYPE_SQLRECORDSET	13
#define STRICT_TYPE_FORM			14
#define STRICT_TYPE_FILE			15
#define STRICT_TYPE_GENERIC			16
#ifdef COM_PROCEDURE_CALL_IMPLEMENTATION
#define STRICT_TYPE_COM_OBJECT
#endif //COM_PROCEDURE_CALL_IMPLEMENTATION
#define STRICT_TYPE_SOCKETSERVER	17
#define STRICT_TYPE_SOCKETCLIENT	18
#define STRICT_TYPE_EXCEPTION		19
#define STRICT_TYPE_XMLREADER		20
#define STRICT_TYPE_XMLWRITER		21

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct tag_Array_Definition {
	int Type;
	int NumberOfElements;
} ARRAYDEFINITION, *LPARRAYDEFINITION;

typedef struct tag_Variable_Item {
	int Scope;
	int Type;
	char *Name; //Free variables have NULL names.
	char *Value;
	int Length;
	int Alloc; //Number of bytes actually allocated.

	bool IsVariableIndex; //Is a variable that contains the index of another variable

	ARRAYDEFINITION ArrayDefinition;

	int Index;
	int ParentVariableIndex;
	int ChildVariableIndex;
	bool Reserved;
} VARIABLEITEM, *LPVARIABLEITEM;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct tag_Variable_Collection {
	int Count;
	int Alloc;
	VARIABLEITEM *Items;
} VARIABLECOLLECTION, *LPVARIABLECOLLECTION;

typedef int VarIndex;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CVars {

public:
	CException *Error;

	ErrorLevel Add(const char *sName, int iType, int *iOutVarIndex);
	ErrorLevel Add(const char *sName, int iValue);
	ErrorLevel Add(LINEITEM *pLine, int *iOutVarIndex);
	ErrorLevel Add(LINEITEM *pLine);
	ErrorLevel AddNewStrongReference(const char *sNewName, int iReferencedIndex);
	ErrorLevel AddUnique(int iType, int *iOutVarIndex);
	ErrorLevel Add(const char *sName, int iType, ARRAYDEFINITION *arrayDefinition, int *iOutVarIndex);

	ErrorLevel Clear(int iIndex);
	ErrorLevel Clear(VARIABLEITEM *pItem);
	ErrorLevel Free(int iIndex);
	ErrorLevel Free(VARIABLEITEM *pItem);
	ErrorLevel Free(const char *sName);
	ErrorLevel Free(VARIABLEITEM *pItem, bool bValueOnly);

	ErrorLevel SetByEval(int iIndex, const char *sMemberName, const char *sText, int iLength);
	ErrorLevel SetByEval(int iIndex, const char *sMemberName, int iArrayIndex, const char *sText, int iLength);
	ErrorLevel SetByEval(int iIndex, const char *sText, int iLength);
	ErrorLevel SetByEval(int iIndex, const char *sText, int iLength, int iArrayIndex);

	ErrorLevel SetPointer(VARIABLEITEM *pVar, void *pObject, int iSize);

	ErrorLevel Set(int iIndex, char cValue);
	ErrorLevel Set(int iIndex, int iValue);
	ErrorLevel Set(int iIndex, HANDLE hFile);
	ErrorLevel Set(VARIABLEITEM *pItem, int iValue);
	ErrorLevel Set(int iIndex, unsigned int iValue);
	ErrorLevel Set(int iIndex, CParams *pParams);
	ErrorLevel Set(int iIndex, const char *sValue);
	ErrorLevel Set(int iIndex, double dValue);
	ErrorLevel Set(VARIABLEITEM *pItem, double dValue);
	ErrorLevel Set(VARIABLEITEM *pItem, CParams *pParams);
	ErrorLevel Set(VARIABLEITEM *pItem, const char *sValue);
	ErrorLevel Set(int iIndex, const char *sValue, int iLength);
	ErrorLevel Set(VARIABLEITEM *pItem, const char *sValue, int iLength);
	ErrorLevel Set(VARIABLEITEM *pItem, const void *pValue, int iLength);
	ErrorLevel Set(VARIABLEITEM *pItem, const char *sMemberName, CParams *pParams);
	ErrorLevel Set(VARIABLEITEM *pItem, const char *sMemberName, int iArrayIndex, CParams *pParams);
	ErrorLevel Set(int iIndex, const char *sMemberName, CParams *pParams);
	ErrorLevel Set(int iIndex, const char *sMemberName, int iArrayIndex, CParams *pParams);

	ErrorLevel Append(VARIABLEITEM *pItem, const char *sValue, int iLength);

	ErrorLevel AddAndSet(const char *sName, int iValue);
	ErrorLevel AddAndSet(const char *sName, double dValue);
	ErrorLevel AddAndSet(const char *sName, const char *sValue);
	ErrorLevel AddAndSet(const char *sName, int iType, const char *sValue, int iLength);
	ErrorLevel AddAndSet(const char *sName, int iType, const char *sValue, int iLength, int iScope);

	ErrorLevel AddVariableParameterReference(REQPARAMITEM *requiredParam, PARAMITEM *param);
	ErrorLevel AddVariableReference(REQPARAMITEM *requiredParam, int iReferencedVarIndex);

	ErrorLevel CheckVariableName(const char *sVarName, int iLength);
	bool FriendlyValue(VARIABLEITEM *pVar, char *sOutput, int iMaxSz);

	//bool IsComplexType(VARIABLEITEM *pItem);
	//bool IsComplexType(int iType);
	//bool IsReadOnly(const char *sVarTypeName);
	//bool IsReadOnly(VARIABLEITEM *pItem);
	//bool IsReadOnly(int iVarType);
	//bool IsBuiltInClass(const char *sVarTypeName);
	//bool IsBuiltInClass(int iVarType);
	bool IsMathConstant(const char *sText);

	VARIABLEITEM *GetArraySubVariable(VARIABLEITEM *pVar, int iArrayIndex);
	VarIndex GetFreeIndex(void);
	VARIABLEITEM *GetFreePointer(void);

	ErrorLevel DropScope(int iScope);

	CVars(void *lpEngine);
	~CVars();

	//int TranslateType(const char *sName);
	//const char *TranslateType(int iType);
	int TranslateStrictType(const char *sName);
	int StrictTypeToLoseType(int iStrictType);
	int StrictTypeSize(int iStrictType);
	int AssessContentType(int iIndex);
	int AssessContentType(VARIABLEITEM *pItem);

	VarIndex GetIndex(const char *sName);
	VARIABLEITEM *GetPointer(const char *sName);
	VARIABLEITEM *GetPointer(int iIndex);
	HANDLE CVars::GetHandle(int iIndex);

	bool IsDefined(int iIndex);
	bool IsDefined(const char *sName);

	int Count(void);

	VarIndex _EndOfBuiltInIndex;

private:
	VarIndex GetIndexBase(const char *sName);
	VARIABLEITEM *GetPointerEx(const char *sName);

	unsigned _UniqueCounter;
	void *pEngine; //Pointer to the Engine Class.
	VARIABLECOLLECTION Collection;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

