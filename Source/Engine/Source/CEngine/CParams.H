#ifndef _CParams_H_
#define _CParams_H_
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CException.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _tag_CParam_Item {
	int Length;
	char *Value;
} PARAMITEM, *LPPARAMITEM;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _tag_CParam_Collection {
	int Count;
	PARAMITEM *Items;
} PARAMCOLLECTION, *LPPARAMCOLLECTION;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CParams {

public:
	CParams(void *lpEngine);
	CParams(void *lpEngine, int iRequiredType);

	~CParams();
	CException *Error;
	int Type;
	bool EnforceTypes;
	PARAMCOLLECTION Collection;

	int ToInt(int iIndex);
	size_t ToIntPtr(int iIndex);
	unsigned int ToUInt(int iIndex);
	bool ToBool(int iIndex);
	double ToDouble(int iIndex);
	char *ToString(int iIndex);

	bool IsDefined(int iIndex);
	PARAMITEM *GetPointer(int iIndex);

	void Clear(void);

	int AssessContentType(int iIndex);
	int AssessContentType(PARAMITEM *pParam);

	int ToObjectIndex(int iIndex);
	int ToObjectIndex(int iIndex, int *iMemberIndex);
	int ToVarIndex(int iIndex);
	void *ToProcIndexPointer(int iIndex);
	void *ToVarIndexPointer(int iIndex);

	PARAMITEM *Add(void);
	ErrorLevel Add(double dValue, int iDecimalPlaces);
	ErrorLevel Add(double dValue);
	ErrorLevel Add(const char *sText, int iLength);
	ErrorLevel Add(const char *sText);
	ErrorLevel Add(unsigned int iValue);
	ErrorLevel Add(int iValue);
	ErrorLevel AddIndex(int iObjectIndex);
	ErrorLevel AddIndex(int iObjectIndex, int iMemberIndex);
	ErrorLevel Add(PARAMITEM *pParam);

	PARAMITEM *First(void);
	PARAMITEM *Second(void);
	PARAMITEM *Third(void);

private:
	void Destroy(void);
	void *pEngine; //Pointer to the Engine Class.
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

