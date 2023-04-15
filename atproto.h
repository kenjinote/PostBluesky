#pragma once

#include <windows.h>
#include <commctrl.h>
#include <wininet.h>
#include <atlconv.h>
#include <chrono>
#include <format>
#include "json.hpp"

using namespace std::chrono;
using json = nlohmann::json;

class atproto
{
	std::string did;
	std::string handle;
	std::string email;
	std::string accessJwt;
	std::string refreshJwt;

	LPBYTE post_api(LPCWSTR lpszUri, json& data);

public:
	BOOL init(IN LPCWSTR lpszID, IN LPCWSTR lpszPW);
	BOOL post(LPCWSTR lpszMessage);
};

