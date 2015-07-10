#include "toolkit/compat.hh"
#ifdef LINUX
#include <unistd.h>
#endif
#include <cstring>

bool StrCaseEqual(const char* str1, const char* str2)
{
	return 0 == StrCaseCmp(str1, str2);
}

int StrCaseCmp(const char* str1, const char* str2)
{
#ifdef USING_VS
	return _stricmp(str1, str2);
#else
	return strcasecmp(str1, str2);
#endif
}

bool StrNCaseEqual(const char* str1, const char* str2, unsigned int len)
{
	return 0 == StrNCaseCmp(str1, str2, len);
}

int StrNCaseCmp(const char* str1, const char* str2, unsigned int len)
{
#ifdef USING_VS
	return _strnicmp(str1, str2, len);
#else
	return strncasecmp(str1, str2, len);
#endif
}

void SleepMS(unsigned int ms)
{
#ifdef USING_VS
	Sleep(ms);
#else
	usleep(ms*1000);
#endif
}

