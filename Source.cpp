#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment(lib,"wininet")

#include <windows.h>
#include <commctrl.h>
#include <wininet.h>
#include <atlconv.h>
#include <chrono>
#include <format>
#include "json.hpp"
#include "atproto.h"
#include "resource.h"

TCHAR szClassName[] = TEXT("postbluesky");

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HWND hEditID;
	static HWND hEditPW;
	static HWND hButtonPost;
	static HWND hEditText;
	static atproto at;
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
			if (at.init(lpszID, lpszPW)) {
				DWORD dwSize = GetWindowTextLength(hEditText);
				LPWSTR lpszMessage = (LPWSTR)GlobalAlloc(0, sizeof(WCHAR) * (dwSize + 1));
				if (lpszMessage) {
					GetWindowText(hEditText, lpszMessage, dwSize + 1);
					if (at.post(lpszMessage)) {
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
