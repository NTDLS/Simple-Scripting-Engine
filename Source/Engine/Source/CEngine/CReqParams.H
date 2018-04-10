#ifndef _CReqParams_H_
#define _CReqParams_H_
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../../../../@Libraries/CMemPool/CMemPool.H"

#include "CException.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _tag_CReqParams_Value {
	char *Value;
	int Length;
} REQPARAMVALUE, *LPREQPARAMVALUE;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _tag_CReqParams_Item {
	int Type;
	bool IsArray;
	int StrictType; //Currently used for external procedures only.
	char *Name;
	REQPARAMVALUE Default;
	bool IsOptional;
	bool ByReference;
} REQPARAMITEM, *LPREQPARAMITEM;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _tag_CReqParams_Collection {
	int Count;
	REQPARAMITEM *Items;
} REQPARAMCOLLECTION, *LPREQPARAMCOLLECTION;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CReqParams {

public:
	CReqParams(void *lpEngine);
	CReqParams(void *lpEngine, CReqParams *pReqParams);
	~CReqParams();
	CException *Error;
	REQPARAMCOLLECTION Collection;

	ErrorLevel Add(const char *sName, int iType);
	ErrorLevel Add(const char *sName, int iType, bool isArray);
	ErrorLevel Add(const char *sName, int iType, int iStrictType, bool bByReference, bool isArray);
	ErrorLevel AddOptional(const char *sName, int iType, int iStrictType, bool bByReference, const char *sDefault, int iDefaultLength);
	ErrorLevel AddOptional(const char *sName, int iType, const char *sDefault);
	ErrorLevel AddOptional(const char *sName, int iType, int iDefault);
	ErrorLevel AddOptional(const char *sName, int iType, unsigned int uDefault);
	ErrorLevel Add(CReqParams *pReqParams);

	REQPARAMITEM *First(void);
	REQPARAMITEM *Last(void);
	REQPARAMITEM *GetPointer(int iIndex);

private:
	void *pEngine; //Pointer to the Engine Class.
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

