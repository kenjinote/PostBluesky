#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment(lib,"wininet")

#include <windows.h>
#include <commctrl.h>
#include <wininet.h>
#include <atlconv.h>
#include <chrono>
#include <format>
#include "json.hpp"
#include "resource.h"

TCHAR szClassName[] = TEXT("postbluesky");
using json = nlohmann::json;
using namespace std::chrono;

std::string did;
std::string handle;
std::string email;
std::string accessJwt;
std::string refreshJwt;

LPBYTE post_api(LPCWSTR lpszUri, json& data)
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

BOOL init(IN LPCWSTR lpszID, IN LPCWSTR lpszPW)
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

BOOL post(LPCWSTR lpszMessage)
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

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HWND hEditID;
	static HWND hEditPW;
	static HWND hButtonPost;
	static HWND hEditText;
	switch (msg)
	{
	case WM_CREATE:
		hEditID = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), L"", WS_VISIBLE | WS_CHILD | WS_TABSTOP | ES_AUTOHSCROLL, 0, 0, 0, 0, hWnd, 0, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		SendMessage(hEditID, EM_SETCUEBANNER, TRUE, (LPARAM)L"ID");
		hEditPW = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), L"", WS_VISIBLE | WS_CHILD | WS_TABSTOP | ES_AUTOHSCROLL, 0, 0, 0, 0, hWnd, 0, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		SendMessage(hEditPW, EM_SETCUEBANNER, TRUE, (LPARAM)L"Password");
		hEditText = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), L"", WS_VISIBLE | WS_CHILD | WS_TABSTOP | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_AUTOHSCROLL | ES_AUTOVSCROLL, 0, 0, 0, 0, hWnd, 0, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		hButtonPost = CreateWindow(TEXT("BUTTON"), TEXT("投稿"), WS_VISIBLE | WS_CHILD | WS_TABSTOP, 0, 0, 0, 0, hWnd, (HMENU)IDOK, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		break;
	case WM_SIZE:
		MoveWindow(hEditID, 10, 10, LOWORD(lParam) - 20, 32, TRUE);
		MoveWindow(hEditPW, 10, 50, LOWORD(lParam) - 20, 32, TRUE);
		MoveWindow(hEditText, 10, 90, LOWORD(lParam) - 20, HIWORD(lParam) - (90 + 32 + 20), TRUE);
		MoveWindow(hButtonPost, 10, HIWORD(lParam) - (32 + 10), 256, 32, TRUE);
		break;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			WCHAR lpszID[1024];
			WCHAR lpszPW[1024];
			GetWindowText(hEditID, lpszID, _countof(lpszID));
			GetWindowText(hEditPW, lpszPW, _countof(lpszPW));
			lstrcat(lpszID, L".bsky.social");
			if (init(lpszID, lpszPW)) {
				DWORD dwSize = GetWindowTextLength(hEditText);
				LPWSTR lpszMessage = (LPWSTR)GlobalAlloc(0, sizeof(WCHAR) * (dwSize + 1));
				if (lpszMessage) {
					GetWindowText(hEditText, lpszMessage, dwSize + 1);
					if (post(lpszMessage)) {
						MessageBox(hWnd, L"投稿成功", L"確認", 0);
					}
					GlobalFree(lpszMessage);
				}
			}
		}
		break;
	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefDlgProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
	MSG msg;
	WNDCLASS wndclass = {
		0,
		WndProc,
		0,
		DLGWINDOWEXTRA,
		hInstance,
		LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1)),
		LoadCursor(0,IDC_ARROW),
		0,
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
		if (!IsDialogMessage(hWnd, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return (int)msg.wParam;
}
