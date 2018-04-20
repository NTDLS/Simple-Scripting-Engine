#ifndef _CException_H_
#define _CException_H_
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef int ErrorLevel;

#define ERROR_OK              ((ErrorLevel) 0)
#define EXECUTE_RETURN        ((ErrorLevel)-1) //Return from a function call
#define CONDITION_TRUE        ((ErrorLevel)-2)
#define CONDITION_FALSE       ((ErrorLevel)-3)
#define CONDITION_SKIP        ((ErrorLevel)-4)
#define EXECUTE_LOOP_END      ((ErrorLevel)-5) //End of a loop has been reached (For: Next / While: Wend)
#define EXECUTE_LOOP_BREAK    ((ErrorLevel)-6)
#define EXECUTE_LOOP_CONTINUE ((ErrorLevel)-7)

#define ERROR_HARD 0
#define ERROR_SOFT 1

typedef struct tag_Exception_Item {
	char *Message;
	int LineIndex;
	int Type;
} EXCEPTIONITEM, *LPEXCEPTIONITEM;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct tag_Exception_Collection {
	int Count;
	int HardCount;
	int SoftCount;
	EXCEPTIONITEM *Items;
} EXCEPTIONCOLLECTION, *LPEXCEPTIONCOLLECTION;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CException {
public:
	CException::CException(void *lpEngine);
	~CException();

	int Inform(const char *sFormat, ...);
	int Soft(const char *sFormat, ...);
	int Soft(int iLineIndex, const char *sFormat, ...);
	int Hard(const char *sFormat, ...);
	int Hard(int iLineIndex, const char *sFormat, ...);

	int HardCount(void);
	int SoftCount(void);

	int Clear(void);

	int Throw(void);
	int Throw(bool bThrowAll);

	EXCEPTIONITEM *GetPointer(int iIndex);
	const char *Text(int iIndex);
	const char *File(int iIndex);
	int Line(int iIndex);
	int LastSoftCollectionIndex(void);

	bool AbsorbExceptions(void);
	void AbsorbExceptions(bool bIgnore);

private:
	void *pEngine; //Pointer to the owning CEngine class.

	bool _bAbsorbExceptions;
	int _iAdsorbedHardCount;
	int _iAdsorbedSoftCount;

	int AddBase(int iErrorType, const char *sText);
	int AddBase(int iErrorType, int iLineIndex, const char *sText);

	EXCEPTIONCOLLECTION Collection;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

