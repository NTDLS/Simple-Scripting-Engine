#ifndef _CProcs_H_
#define _CProcs_H_
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CException.H"
#include "CLines.H"
#include "CParams.H"
#include "CReqParams.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define EXTERNAL_TYPE_INVALID	INVALID_INDEX
#define EXTERNAL_TYPE_STDCALL	1
#define EXTERNAL_TYPE_CDECL		2
#define EXTERNAL_TYPE_FASTCALL	3

#define PROC_TYPE_INVALID	INVALID_INDEX
#define PROC_TYPE_SYSTEM	1
#define PROC_TYPE_SCRIPT	2
#ifdef EXTERNAL_PROCEDURE_CALL_IMPLEMENTATION
#define PROC_TYPE_EXTERNAL	3
#endif //EXTERNAL_PROCEDURE_CALL_IMPLEMENTATION
#ifdef COM_PROCEDURE_CALL_IMPLEMENTATION
#define PROC_TYPE_COM		4
#endif //COM_PROCEDURE_CALL_IMPLEMENTATION

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct tag_Proc_Item {
	int Type;				//System, Script, ect...
	int ReturnType;			//String, Integer, Double, ect...
	int AssessedReturnType; //Currently used for external procedures only.
	char *Name;				//Name of the function.
	CReqParams *ReqParams;	//Required Parameters, 0 count means no params. NULL means infinite params.
	int LineBegin;			//The index of the line that contains the beginning of the function.
	HANDLE hThread;			//The handle of the thread that this function created.

#ifdef EXTERNAL_PROCEDURE_CALL_IMPLEMENTATION
	//External
	char *Library;
	char *Alias;
	int CallingConvention;
#endif //EXTERNAL_PROCEDURE_CALL_IMPLEMENTATION

} PROCITEM, *LPPROCITEM;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct tag_Proc_Collection {
	int Count;
	int Alloc;
	PROCITEM *Items;
} PROCCOLLECTION, *LPPROCCOLLECTION;

typedef int ProcIndex;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CProcs {

public:
	CException *Error;

	CProcs(void *lpEngine);
	~CProcs();

	ErrorLevel Add(const char *sName, int iType, int iReturnType, CReqParams *pReqParams);
	ErrorLevel AddSystem(const char *sName, int iReturnType, CReqParams *pReqParams);
	ErrorLevel AddScript(LINEITEM *pLine);

#ifdef EXTERNAL_PROCEDURE_CALL_IMPLEMENTATION
	ErrorLevel AddExternal(LINEITEM *pLine);
	ErrorLevel CallExternalProc(PROCITEM *pProc, CParams *pParams, void **pvParams, void **pReturnValue);
#endif //EXTERNAL_PROCEDURE_CALL_IMPLEMENTATION

	ErrorLevel Execute(int iIndex, CParams *pParams, CParams *pReturn);
	ErrorLevel ExecuteByEval(const char *sText, int iLength, CParams *pAdditionalParams, CParams *pReturn);

#ifdef EXTERNAL_PROCEDURE_CALL_IMPLEMENTATION
	int TranslateCallingConvention(const char *sName);
#endif //EXTERNAL_PROCEDURE_CALL_IMPLEMENTATION

#ifdef _DEBUG
	ErrorLevel GenerateDocumentation(const char *sOutputPath);
#endif //_DEBUG

	bool IsDefined(const char *sName);
	bool IsDefined(int iIndex);
	PROCITEM *GetPointer(const char *sName);
	PROCITEM *GetPointer(int iIndex);
	ProcIndex GetIndex(const char *sName);
	int Count(void);

	char *SQLBuffer(int *iAllocatedSize);

	ProcIndex _EndOfBuiltInIndex;

private:
	void *pEngine; //Pointer to the Engine Class.
	char *sSQLBuffer; //Not thread safe, used for performance.
	int iAllocatedSize;

	ErrorLevel ExecuteInternal(int iIndex, CParams *pParams, CParams *pReturn);

	PROCCOLLECTION Collection;
};

#ifdef EXTERNAL_PROCEDURE_CALL_IMPLEMENTATION
typedef VOID *(__stdcall *StdProc0)  (void);
typedef VOID *(__stdcall *StdProc1)  (VOID *);
typedef VOID *(__stdcall *StdProc2)  (VOID *, VOID *);
typedef VOID *(__stdcall *StdProc3)  (VOID *, VOID *, VOID *);
typedef VOID *(__stdcall *StdProc4)  (VOID *, VOID *, VOID *, VOID *);
typedef VOID *(__stdcall *StdProc5)  (VOID *, VOID *, VOID *, VOID *, VOID *);
typedef VOID *(__stdcall *StdProc6)  (VOID *, VOID *, VOID *, VOID *, VOID *, VOID *);
typedef VOID *(__stdcall *StdProc7)  (VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *);
typedef VOID *(__stdcall *StdProc8)  (VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *);
typedef VOID *(__stdcall *StdProc9)  (VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *);
typedef VOID *(__stdcall *StdProc10) (VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *);
typedef VOID *(__stdcall *StdProc11) (VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *);
typedef VOID *(__stdcall *StdProc12) (VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *);
typedef VOID *(__stdcall *StdProc13) (VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *);
typedef VOID *(__stdcall *StdProc14) (VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *);
typedef VOID *(__stdcall *StdProc15) (VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *);
typedef VOID *(__stdcall *StdProc16) (VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *);
typedef VOID *(__stdcall *StdProc17) (VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *);
typedef VOID *(__stdcall *StdProc18) (VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *);
typedef VOID *(__stdcall *StdProc19) (VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *);
typedef VOID *(__stdcall *StdProc20) (VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *);
typedef VOID *(__stdcall *StdProc21) (VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *);
typedef VOID *(__stdcall *StdProc22) (VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *);
typedef VOID *(__stdcall *StdProc23) (VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *);
typedef VOID *(__stdcall *StdProc24) (VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *);
typedef VOID *(__stdcall *StdProc25) (VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *);

typedef VOID *(__cdecl *CdeclProc0)  (void);
typedef VOID *(__cdecl *CdeclProc1)  (VOID *);
typedef VOID *(__cdecl *CdeclProc2)  (VOID *, VOID *);
typedef VOID *(__cdecl *CdeclProc3)  (VOID *, VOID *, VOID *);
typedef VOID *(__cdecl *CdeclProc4)  (VOID *, VOID *, VOID *, VOID *);
typedef VOID *(__cdecl *CdeclProc5)  (VOID *, VOID *, VOID *, VOID *, VOID *);
typedef VOID *(__cdecl *CdeclProc6)  (VOID *, VOID *, VOID *, VOID *, VOID *, VOID *);
typedef VOID *(__cdecl *CdeclProc7)  (VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *);
typedef VOID *(__cdecl *CdeclProc8)  (VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *);
typedef VOID *(__cdecl *CdeclProc9)  (VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *);
typedef VOID *(__cdecl *CdeclProc10) (VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *);
typedef VOID *(__cdecl *CdeclProc11) (VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *);
typedef VOID *(__cdecl *CdeclProc12) (VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *);
typedef VOID *(__cdecl *CdeclProc13) (VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *);
typedef VOID *(__cdecl *CdeclProc14) (VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *);
typedef VOID *(__cdecl *CdeclProc15) (VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *);
typedef VOID *(__cdecl *CdeclProc16) (VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *);
typedef VOID *(__cdecl *CdeclProc17) (VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *);
typedef VOID *(__cdecl *CdeclProc18) (VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *);
typedef VOID *(__cdecl *CdeclProc19) (VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *);
typedef VOID *(__cdecl *CdeclProc20) (VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *);
typedef VOID *(__cdecl *CdeclProc21) (VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *);
typedef VOID *(__cdecl *CdeclProc22) (VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *);
typedef VOID *(__cdecl *CdeclProc23) (VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *);
typedef VOID *(__cdecl *CdeclProc24) (VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *);
typedef VOID *(__cdecl *CdeclProc25) (VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *);

typedef VOID *(__fastcall *FastProc0)  (void);
typedef VOID *(__fastcall *FastProc1)  (VOID *);
typedef VOID *(__fastcall *FastProc2)  (VOID *, VOID *);
typedef VOID *(__fastcall *FastProc3)  (VOID *, VOID *, VOID *);
typedef VOID *(__fastcall *FastProc4)  (VOID *, VOID *, VOID *, VOID *);
typedef VOID *(__fastcall *FastProc5)  (VOID *, VOID *, VOID *, VOID *, VOID *);
typedef VOID *(__fastcall *FastProc6)  (VOID *, VOID *, VOID *, VOID *, VOID *, VOID *);
typedef VOID *(__fastcall *FastProc7)  (VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *);
typedef VOID *(__fastcall *FastProc8)  (VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *);
typedef VOID *(__fastcall *FastProc9)  (VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *);
typedef VOID *(__fastcall *FastProc10) (VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *);
typedef VOID *(__fastcall *FastProc11) (VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *);
typedef VOID *(__fastcall *FastProc12) (VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *);
typedef VOID *(__fastcall *FastProc13) (VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *);
typedef VOID *(__fastcall *FastProc14) (VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *);
typedef VOID *(__fastcall *FastProc15) (VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *);
typedef VOID *(__fastcall *FastProc16) (VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *);
typedef VOID *(__fastcall *FastProc17) (VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *);
typedef VOID *(__fastcall *FastProc18) (VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *);
typedef VOID *(__fastcall *FastProc19) (VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *);
typedef VOID *(__fastcall *FastProc20) (VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *);
typedef VOID *(__fastcall *FastProc21) (VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *);
typedef VOID *(__fastcall *FastProc22) (VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *);
typedef VOID *(__fastcall *FastProc23) (VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *);
typedef VOID *(__fastcall *FastProc24) (VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *);
typedef VOID *(__fastcall *FastProc25) (VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *, VOID *);
#endif //EXTERNAL_PROCEDURE_CALL_IMPLEMENTATION

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

