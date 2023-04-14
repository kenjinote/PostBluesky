#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment(lib,"wininet")

#include <windows.h>
#include <wininet.h>
#include <atlconv.h>
#include "json.hpp"
#include "resource.h"

TCHAR szClassName[] = TEXT("postbluesky");
using json = nlohmann::json;

std::string did;
std::string handle;
std::string email;
std::string accessJwt;
std::string refreshJwt;

BOOL init(IN LPCWSTR lpszID, IN LPCWSTR lpszPW)
{
	BOOL bRet = FALSE;
	if (lpszID != NULL && lpszPW != NULL) {
		const HINTERNET hSession = InternetOpen(TEXT("Mozilla/5.0 (Windows NT 6.3; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/38.0.2125.111 Safari/537.36"), INTERNET_OPEN_TYPE_PRECONFIG, 0, 0, INTERNET_FLAG_NO_COOKIES);
		if (hSession) {
			const HINTERNET hConnection = InternetConnect(hSession, L"bsky.social", INTERNET_DEFAULT_HTTPS_PORT, 0, 0, INTERNET_SERVICE_HTTP, 0, 0);
			if (hConnection) {
				const HINTERNET hRequest = HttpOpenRequest(hConnection, L"POST", L"/xrpc/com.atproto.server.createSession", 0, 0, 0, INTERNET_FLAG_SECURE, 0);
				if (hRequest) {
					USES_CONVERSION;
					WCHAR szHeader1[] = L"Content-Type: application/json; charset=UTF-8";
					HttpAddRequestHeaders(hRequest, szHeader1, lstrlen(szHeader1), HTTP_ADDREQ_FLAG_REPLACE | HTTP_ADDREQ_FLAG_ADD);
					WCHAR szHeader2[] = L"Accept: */*";
					HttpAddRequestHeaders(hRequest, szHeader2, lstrlen(szHeader2), HTTP_ADDREQ_FLAG_REPLACE | HTTP_ADDREQ_FLAG_ADD);
					json j;
					j["identifier"] = W2A(lpszID);
					j["password"] = W2A(lpszPW);
					std::string js = j.dump();
					if (HttpSendRequest(hRequest, 0, 0, (LPVOID)js.c_str(), (DWORD)js.size()))
					{
						DWORD dwStatusCode = 0;
						DWORD dwLength = sizeof(DWORD);
						if (HttpQueryInfo(hRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &dwStatusCode, &dwLength, 0))
						{
							if (HTTP_STATUS_OK == dwStatusCode)
							{
								LPBYTE lpszReturn = 0;
								lpszReturn = (LPBYTE)GlobalAlloc(GMEM_FIXED, 1);
								if (lpszReturn) {
									DWORD dwRead;
									static BYTE szBuf[1024 * 4];
									LPBYTE lpTmp;
									DWORD dwSize = 0;
									for (;;) {
										if (!InternetReadFile(hRequest, szBuf, (DWORD)sizeof(szBuf), &dwRead) || !dwRead) break;
										lpTmp = (LPBYTE)GlobalReAlloc(lpszReturn, (SIZE_T)(dwSize + dwRead + 1), GMEM_MOVEABLE);
										if (lpTmp == NULL) break;
										lpszReturn = lpTmp;
										CopyMemory(lpszReturn + dwSize, szBuf, dwRead);
										dwSize += dwRead;
									}
									if (dwSize > 0) {
										lpszReturn[dwSize] = 0;
										auto jss = json::parse(std::string((char*)lpszReturn));
										accessJwt = jss["accessJwt"];
										did = jss["did"];
										handle = jss["handle"];
										email = jss["email"];
										refreshJwt = jss["refreshJwt"];
										bRet = TRUE;
									}
									GlobalFree(lpszReturn);
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
	return bRet;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HWND hEditID;
	static HWND hEditPW;
	static HWND hButtonPost;
	static HWND hEditText;
	switch (msg)
	{
	case WM_CREATE:
		hEditID = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), 0, WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL, 0, 0, 0, 0, hWnd, 0, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		hEditPW = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), 0, WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL, 0, 0, 0, 0, hWnd, 0, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		hEditText = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), 0, WS_VISIBLE | WS_CHILD | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_AUTOHSCROLL | ES_AUTOVSCROLL, 0, 0, 0, 0, hWnd, 0, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		hButtonPost = CreateWindow(TEXT("BUTTON"), TEXT("投稿"), WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hWnd, (HMENU)IDOK, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		break;
	case WM_SIZE:
		MoveWindow(hEditID, (10), (10), LOWORD(lParam) - (20), 32, TRUE);
		MoveWindow(hEditPW, (10), (50), LOWORD(lParam) - (20), 32, TRUE);
		MoveWindow(hEditText, (10), (90), LOWORD(lParam) - (20), HIWORD(lParam) - (90 + 32 + 20), TRUE);
		MoveWindow(hButtonPost, (10), HIWORD(lParam) - (32 + 10), (256), (32), TRUE);
		break;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			WCHAR lpszID[1024];
			WCHAR lpszPW[1024];
			GetWindowText(hEditID, lpszID, _countof(lpszID));
			GetWindowText(hEditPW, lpszPW, _countof(lpszPW));
			lstrcat(lpszID, L".bsky.social");
			init(lpszID, lpszPW);			
			SendMessageA(hEditText, EM_REPLACESEL, 0, (LPARAM)did.c_str());
			SendMessageA(hEditText, EM_REPLACESEL, 0, (LPARAM)"\r\n");
			SendMessageA(hEditText, EM_REPLACESEL, 0, (LPARAM)handle.c_str());
			SendMessageA(hEditText, EM_REPLACESEL, 0, (LPARAM)"\r\n");
			SendMessageA(hEditText, EM_REPLACESEL, 0, (LPARAM)email.c_str());
			SendMessageA(hEditText, EM_REPLACESEL, 0, (LPARAM)"\r\n");
			SendMessageA(hEditText, EM_REPLACESEL, 0, (LPARAM)accessJwt.c_str());
			SendMessageA(hEditText, EM_REPLACESEL, 0, (LPARAM)"\r\n");
			SendMessageA(hEditText, EM_REPLACESEL, 0, (LPARAM)refreshJwt.c_str());
			SendMessageA(hEditText, EM_REPLACESEL, 0, (LPARAM)"\r\n");
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPreInst, LPWSTR pCmdLine, int nCmdShow)
{
	MSG msg;
	WNDCLASS wndclass = {
		CS_HREDRAW | CS_VREDRAW,
		WndProc,
		0,
		0,
		hInstance,
		LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1)),
		LoadCursor(0,IDC_ARROW),
		(HBRUSH)(COLOR_WINDOW + 1),
		0,
		szClassName
	};
	RegisterClass(&wndclass);
	HWND hWnd = CreateWindow(
		szClassName,
		TEXT("blueskyにポスト"),
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
		CW_USEDEFAULT,
		0,
		CW_USEDEFAULT,
		0,
		0,
		0,
		hInstance,
		0
	);
	ShowWindow(hWnd, SW_SHOWDEFAULT);
	UpdateWindow(hWnd);
	while (GetMessage(&msg, 0, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (int)msg.wParam;
}
