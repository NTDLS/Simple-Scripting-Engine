#ifndef _CScriptTypes_H_
#define _CScriptTypes_H_
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../../../CSQLBinding/CSQL.H"
#include "../../../../CSocketServer/CSocketServer.H"

#include "CException.H"
#include "CLines.H"
#include "CParams.H"
#include "CState.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define INVALID_TYPE_NAME "Invalid"

//Built-in Types:
extern int VAR_TYPE_INVALID;
extern int VAR_TYPE_STRING;
extern int VAR_TYPE_NUMERIC;
extern int VAR_TYPE_SQLCONNECTION;
extern int VAR_TYPE_SQLRECORDSET;
extern int VAR_TYPE_FILE;
extern int VAR_TYPE_BYTES;
extern int VAR_TYPE_LIST;
extern int VAR_TYPE_GENERIC;
#ifdef COM_PROCEDURE_CALL_IMPLEMENTATION
extern int VAR_TYPE_COM_OBJECT;
#endif //COM_PROCEDURE_CALL_IMPLEMENTATION
extern int VAR_TYPE_SOCKETSERVER;
extern int VAR_TYPE_SOCKETCLIENT;
extern int VAR_TYPE_XMLREADER;
extern int VAR_TYPE_XMLWRITER;
extern int VAR_TYPE_EXCEPTION;
extern int VAR_TYPE_ARRAY;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct tag_Type_Member_Item {
	char *Name;
	int Type;
	int StrictType;
	int IsContiguous;
	int SizeInBytes;
	int ByteOffset;
	int Index;
	int ParentIndex;
} TYPEMEMBERITEM, *LPTYPEMEMBERITEM;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct tag_Type_Member_Collection {
	int Count;
	int SizeInBytes;
	TYPEMEMBERITEM *Items;
} TYPEMEMBERCOLLECTION, *LPTYPEMEMBERCOLLECTION;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct tag_Type_Item {
	char *Name;
	int Index;
	bool IsComplex; //non-string and non-numeric.
	bool IsBuiltIn;
	bool IsBuiltInClass;
	bool IsCustomType; //User defined type.
	bool IsStrictType; //Members are stored in binary format.
	TYPEMEMBERCOLLECTION Members;
} TYPEITEM, *LPTYPEITEM;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct tag_Type_Collection {
	int Count;
	TYPEITEM *Items;
} TYPECOLLECTION, *LPTYPECOLLECTION;

typedef struct _tag_MemberEndpointInfo_Info {
	TYPEMEMBERITEM *Member;
	int ByteOffset;
} MEMBERENDPOINTINFO, *LPMEMBERENDPOINTINFO;

typedef int VarTypeIndex;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CScriptTypes {

public:
	CException *Error;

	TYPEITEM *Add(const char *sName);
	ErrorLevel AddStrictType();

	ErrorLevel Free(TYPEITEM *pItem);

	ErrorLevel CheckTypeName(const char *sVarName, int iLength);

	VarTypeIndex GetFreeIndex(void);
	TYPEITEM *GetFreePointer(void);

	CScriptTypes(void *lpEngine);
	~CScriptTypes();

	VarTypeIndex GetIndex(const char *sName);
	TYPEITEM *GetPointer(const char *sName);
	TYPEITEM *GetPointer(int iIndex);

	const char *Name(int iIndex);

	bool IsReadOnly(const char *sName);
	bool IsReadOnly(int iIndex);

	bool IsDefined(int iIndex);
	bool IsDefined(const char *sName);

	TYPEMEMBERITEM *GetMember(TYPEITEM *pType, const char *sMemberName);
	TYPEMEMBERITEM *GetMember(TYPEITEM *pType, int iIndex);
	ErrorLevel TraverseMembers(VARIABLEITEM *pItem, const char *sMemberLineage, MEMBERENDPOINTINFO *lpMemberEndpoint, int *iNonMemberPosition);

	int Count(void);

private:
	void *pEngine; //Pointer to the Engine Class.
	TYPECOLLECTION Collection;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

