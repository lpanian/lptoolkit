#include "toolkit/compat.hh"
#ifdef LINUX
#include <unistd.h>
#endif
#include <cstring>

bool StrCaseEqual(const char* str1, const char* str2)
{
#ifdef USING_VS
	return 0 == _stricmp(str1, str2);
#else
	return 0 == strcasecmp(str1, str2);
#endif
}

bool StrNCaseEqual(const char* str1, const char* str2, unsigned int len)
{
#ifdef USING_VS
	return 0 == _strnicmp(str1, str2, len);
#else
	return 0 == strncasecmp(str1, str2, len);
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
