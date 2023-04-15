#include "atproto.h"

LPBYTE atproto::post_api(LPCWSTR lpszUri, json& data)
{
	LPBYTE lpszRet = NULL;
	if (lpszUri != NULL) {
		const HINTERNET hSession = InternetOpen(0, INTERNET_OPEN_TYPE_PRECONFIG, 0, 0, INTERNET_FLAG_NO_COOKIES);
		if (hSession) {
			const HINTERNET hConnection = InternetConnect(hSession, L"bsky.social", INTERNET_DEFAULT_HTTPS_PORT, 0, 0, INTERNET_SERVICE_HTTP, 0, 0);
			if (hConnection) {
				const HINTERNET hRequest = HttpOpenRequest(hConnection, L"POST", lpszUri, 0, 0, 0, INTERNET_FLAG_SECURE, 0);
				if (hRequest) {
					WCHAR szHeader1[] = L"Content-Type: application/json; charset=UTF-8";
					HttpAddRequestHeaders(hRequest, szHeader1, lstrlen(szHeader1), HTTP_ADDREQ_FLAG_REPLACE | HTTP_ADDREQ_FLAG_ADD);
					WCHAR szHeader2[] = L"Accept: */*";
					HttpAddRequestHeaders(hRequest, szHeader2, lstrlen(szHeader2), HTTP_ADDREQ_FLAG_REPLACE | HTTP_ADDREQ_FLAG_ADD);
					if (accessJwt.size() > 0) {
						USES_CONVERSION;
						WCHAR szHeader3[1024];
						wsprintf(szHeader3, L"Authorization: Bearer %s", A2W(accessJwt.c_str()));
						HttpAddRequestHeaders(hRequest, szHeader3, lstrlen(szHeader3), HTTP_ADDREQ_FLAG_REPLACE | HTTP_ADDREQ_FLAG_ADD);
					}
					auto data_string = data.dump();
					if (HttpSendRequest(hRequest, 0, 0, (LPVOID)data_string.c_str(), (DWORD)data_string.size())) {
						DWORD dwStatusCode = 0;
						DWORD dwLength = sizeof(DWORD);
						if (HttpQueryInfo(hRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &dwStatusCode, &dwLength, 0)) {
							if (HTTP_STATUS_OK == dwStatusCode) {
								lpszRet = (LPBYTE)GlobalAlloc(GMEM_FIXED, 1);
								if (lpszRet) {
									DWORD dwRead;
									static BYTE szBuf[1024 * 4];
									LPBYTE lpTmp;
									DWORD dwSize = 0;
									for (;;) {
										if (!InternetReadFile(hRequest, szBuf, (DWORD)sizeof(szBuf), &dwRead) || !dwRead) break;
										lpTmp = (LPBYTE)GlobalReAlloc(lpszRet, (SIZE_T)(dwSize + dwRead + 1), GMEM_MOVEABLE);
										if (lpTmp == NULL) break;
										lpszRet = lpTmp;
										CopyMemory(lpszRet + dwSize, szBuf, dwRead);
										dwSize += dwRead;
									}
									if (dwSize > 0) {
										lpszRet[dwSize] = 0;
									}
								}
							}
						}
					}
					InternetCloseHandle(hRequest);
				}
				InternetCloseHandle(hConnection);
			}
			InternetCloseHandle(hSession);
		}
	}
	return lpszRet;
}

BOOL atproto::createSession(IN LPCWSTR lpszID, IN LPCWSTR lpszPW)
{
	if (lpszID == NULL || lpszPW == NULL) return FALSE;
	USES_CONVERSION;
	BOOL bRet = FALSE;
	json json1;
	json1["identifier"] = W2A(lpszID);
	json1["password"] = W2A(lpszPW);
	LPBYTE lpszByte = post_api(L"/xrpc/com.atproto.server.createSession", json1);
	if (lpszByte) {
		auto json1 = json::parse(std::string((char*)lpszByte));
		GlobalFree(lpszByte);
		accessJwt = json1["accessJwt"];
		did = json1["did"];
		handle = json1["handle"];
		email = json1["email"];
		refreshJwt = json1["refreshJwt"];
		bRet = TRUE;
	}
	return bRet;
}

BOOL atproto::createRecord(LPCWSTR lpszMessage)
{
	if (lpszMessage == NULL) return FALSE;
	USES_CONVERSION;
	BOOL bRet = FALSE;
	json json1;
	json child;
	child["text"] = W2A_CP(lpszMessage, CP_UTF8);
	child["createdAt"] = std::format("{:%FT%TZ}", system_clock::now());
	json1["collection"] = "app.bsky.feed.post";
	json1["repo"] = did;
	json1["record"] = child;
	LPBYTE lpszByte = post_api(L"/xrpc/com.atproto.repo.createRecord", json1);
	if (lpszByte) {
		auto json1 = json::parse(std::string((char*)lpszByte));
		GlobalFree(lpszByte);
		bRet = TRUE;
	}
	return bRet;
}