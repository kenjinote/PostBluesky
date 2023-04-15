#include "util.h"

BOOL util::StrFindChar(LPCWSTR lpszText, WCHAR c)
{
	if (lpszText == NULL || c == 0) return FALSE;
	LPCWSTR p = lpszText;
	while (*p) {
		if (*p == c) return TRUE;
		p++;
	}
	return FALSE;
}