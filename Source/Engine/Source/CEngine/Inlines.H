#ifndef _Inlines_h
#define _Inlines_h
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//__forceinline//

__forceinline bool _StrEql(const char *sOne, const char *sTwo)
{
	while (*sOne == *sTwo && *sOne && *sTwo)
	{
		*sOne++;
		*sTwo++;
	}

	if (!*sOne && !*sTwo)
	{
		return true;
	}
	else return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Compare case-insensitive string.
*/
__forceinline bool _StrEqlI(const char *sOne, const char *sTwo)
{
	while (*sOne && *sTwo && (*sOne <= 65 || *sOne <= 90 ? (*sOne) + 32 : (*sOne)) == (*sTwo <= 65 || *sTwo <= 90 ? (*sTwo) + 32 : (*sTwo)))
	{
		*sOne++;
		*sTwo++;
	}

	if (!*sOne && !*sTwo)
	{
		return true;
	}
	else return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

