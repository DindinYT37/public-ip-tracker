#include <windows.h>
#include <winhttp.h>
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <thread>
#include <ctime>
#include <filesystem>

#define UNICODE
#define _UNICODE

#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAY_EXIT 1001
#define ID_TRAY_REVEAL_CSV 1002
#define ID_TRAY_OPEN_CSV 1003

NOTIFYICONDATAW nid;
HWND hwnd;
std::string logFilePath; // Global variable for log file path

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_TRAYICON)
	{
		if (lParam == WM_RBUTTONUP)
		{
			POINT pt;
			GetCursorPos(&pt);
			HMENU hMenu = CreatePopupMenu();
			InsertMenuW(hMenu, 0, MF_BYPOSITION | MF_STRING, ID_TRAY_REVEAL_CSV, L"Reveal CSV");
			InsertMenuW(hMenu, 1, MF_BYPOSITION | MF_STRING, ID_TRAY_OPEN_CSV, L"Open CSV");
			InsertMenuW(hMenu, 2, MF_BYPOSITION | MF_STRING, ID_TRAY_EXIT, L"Exit");
			SetForegroundWindow(hwnd);
			TrackPopupMenu(hMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, hwnd, NULL);
			DestroyMenu(hMenu);
		}
	}
	else if (uMsg == WM_COMMAND)
	{
		if (LOWORD(wParam) == ID_TRAY_EXIT)
		{
			Shell_NotifyIconW(NIM_DELETE, &nid);
			PostQuitMessage(0);
		}
		else if (LOWORD(wParam) == ID_TRAY_REVEAL_CSV)
		{
			// Reveal CSV in Explorer
			std::wstring command = L"/select,\"" + std::wstring(logFilePath.begin(), logFilePath.end()) + L"\"";
			ShellExecuteW(NULL, L"open", L"explorer.exe", command.c_str(), NULL, SW_SHOW);
		}
		else if (LOWORD(wParam) == ID_TRAY_OPEN_CSV)
		{
			// Open CSV with default app
			std::wstring wLogFilePath(logFilePath.begin(), logFilePath.end());
			ShellExecuteW(NULL, L"open", wLogFilePath.c_str(), NULL, NULL, SW_SHOW);
		}
	}
	else if (uMsg == WM_DESTROY)
	{
		Shell_NotifyIconW(NIM_DELETE, &nid);
		PostQuitMessage(0);
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void CreateTrayIcon(HWND hwnd)
{
	nid.cbSize = sizeof(NOTIFYICONDATAW);
	nid.hWnd = hwnd;
	nid.uID = 1;
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	nid.uCallbackMessage = WM_TRAYICON;
	HICON hIcon = (HICON)LoadImageW(
		GetModuleHandleW(NULL),
		L"MAINICON", // Add an icon resource with this identifier
		IMAGE_ICON,
		GetSystemMetrics(SM_CXSMICON),
		GetSystemMetrics(SM_CYSMICON),
		LR_DEFAULTCOLOR);
	nid.hIcon = hIcon ? hIcon : LoadIcon(NULL, IDI_APPLICATION);
	wcscpy_s(nid.szTip, L"IP Logger");
	Shell_NotifyIconW(NIM_ADD, &nid);
}

std::wstring GetFullPath(const std::string &relativePath)
{
	char fullPath[MAX_PATH];
	GetFullPathNameA(relativePath.c_str(), MAX_PATH, fullPath, NULL);
	return std::wstring(fullPath, fullPath + strlen(fullPath));
}

// Returns timestamp in YYYY-MM-DD HH:MM:SS format
// Note: Not thread-safe, but we only use it for logging so it's fine
std::string getCurrentTimestamp()
{
	std::time_t now = std::time(nullptr);
	char buf[20];
	std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
	return std::string(buf);
}

// Makes a GET request using WinHTTP
// TODO: Add timeout parameter? Current default might be too long
std::string httpGet(const std::wstring &domain, const std::wstring &path)
{
	std::string result;
	HINTERNET hSession = WinHttpOpen(
		L"IP Checker/1.0",
		WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
		WINHTTP_NO_PROXY_NAME,
		WINHTTP_NO_PROXY_BYPASS,
		0);

	if (!hSession)
	{
		std::cerr << "WinHTTP init failed (error " << GetLastError() << ") - check your internet connection\n";
		return result;
	}

	// Connect to domain
	HINTERNET hConnect = WinHttpConnect(hSession, domain.c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0);
	if (!hConnect)
	{
		std::cerr << "Failed to connect to domain. Error: " << GetLastError() << std::endl;
		WinHttpCloseHandle(hSession);
		return result;
	}

	// Create HTTP request handle
	HINTERNET hRequest = WinHttpOpenRequest(
		hConnect, L"GET", path.c_str(),
		NULL, WINHTTP_NO_REFERER,
		WINHTTP_DEFAULT_ACCEPT_TYPES,
		WINHTTP_FLAG_SECURE);

	if (!hRequest)
	{
		std::cerr << "Failed to open HTTP request handle. Error: " << GetLastError() << std::endl;
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hSession);
		return result;
	}

	/*
	// Disable SSL/TLS certificate validation (for testing purposes only)
	DWORD dwFlags = SECURITY_FLAG_IGNORE_UNKNOWN_CA |
					SECURITY_FLAG_IGNORE_CERT_DATE_INVALID |
					SECURITY_FLAG_IGNORE_CERT_CN_INVALID |
					SECURITY_FLAG_IGNORE_CERT_WRONG_USAGE;
	WinHttpSetOption(hRequest, WINHTTP_OPTION_SECURITY_FLAGS, &dwFlags, sizeof(dwFlags));
	*/

	// Set SSL/TLS version (optional, if needed)
	DWORD dwProtocols = WINHTTP_FLAG_SECURE_PROTOCOL_TLS1 |
						WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_1 |
						WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_2;
	WinHttpSetOption(hRequest, WINHTTP_OPTION_SECURE_PROTOCOLS, &dwProtocols, sizeof(dwProtocols));

	BOOL bResult = WinHttpSendRequest(
		hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
		WINHTTP_NO_REQUEST_DATA, 0,
		0, 0);

	if (!bResult)
	{
		std::cerr << "Failed to send HTTP request. Error: " << GetLastError() << std::endl;
		WinHttpCloseHandle(hRequest);
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hSession);
		return result;
	}

	bResult = WinHttpReceiveResponse(hRequest, NULL);
	if (!bResult)
	{
		std::cerr << "Failed to receive HTTP response. Error: " << GetLastError() << std::endl;
		WinHttpCloseHandle(hRequest);
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hSession);
		return result;
	}

	const DWORD bufferSize = 4096;
	char buffer[bufferSize];
	DWORD bytesRead = 0;
	while (WinHttpReadData(hRequest, buffer, bufferSize, &bytesRead) && bytesRead)
	{
		result.append(buffer, bytesRead);
		bytesRead = 0;
	}

	// Clean handles
	WinHttpCloseHandle(hRequest);
	WinHttpCloseHandle(hConnect);
	WinHttpCloseHandle(hSession);

	return result;
}

bool isValidIPAddress(const std::string &ip)
{
	// Quick sanity check for IPs - good enough for our needs
	// Proper validation would need regex or proper parsing
	if (ip.find('.') != std::string::npos && ip.size() < 16) // IPv4
	{
		return true;
	}
	if (ip.find(':') != std::string::npos && ip.size() < 40) // IPv6
	{
		return true;
	}
	return false;
}

// Tries multiple IP services until we get a valid response
// Returns empty string if all fail (probably no internet)
std::string getPublicIP()
{
	// Services ordered by reliability/speed based on testing
	std::wstring endpoints[][2] = {
		{L"checkip.amazonaws.com", L"/"},
		{L"api.ipify.org", L"/"},
		{L"icanhazip.com", L"/"},
		{L"ipinfo.io", L"/ip"}};

	for (auto &ep : endpoints)
	{
		std::string response = httpGet(ep[0], ep[1]);

		if (isValidIPAddress(response))
		{
			// Clean up response - different services have different formats
			while (!response.empty() && (response.back() == '\n' || response.back() == '\r' || response.back() == ' '))
			{
				response.pop_back();
			}
			while (!response.empty() && (response.front() == '\n' || response.front() == '\r' || response.front() == ' '))
			{
				response.erase(response.begin());
			}

			return response;
		}
	}

	return "";
}

// Check if there's a change in IP and append to "ip_log.csv" if so
void logIPChange(const std::string &ipLogPath)
{
	std::error_code ec;
	std::filesystem::path path(ipLogPath);
	if (!std::filesystem::exists(path.parent_path(), ec))
	{
		try
		{
			std::filesystem::create_directories(path.parent_path());
		}
		catch (const std::exception &e)
		{
			std::cerr << getCurrentTimestamp() << " - Failed to create directory: " << e.what() << "\n";
			return;
		}
	}

	std::string currentIP = getPublicIP();
	if (currentIP.empty())
	{
		std::cout << getCurrentTimestamp() << " - Failed to retrieve a valid IP Address.\n";
		return;
	}

	// Create CSV if doesn't exist, add header
	std::ifstream infile(ipLogPath);
	bool fileExists = infile.good();
	infile.close();

	if (!fileExists)
	{
		std::ofstream outfile(ipLogPath);
		outfile << "Timestamp,IP Address\n";
		outfile.close();
	}

	// Read last line for previous IP
	std::string lastIP;
	{
		std::ifstream readfile(ipLogPath);
		std::string line;
		std::string lastLine;
		while (std::getline(readfile, line))
		{
			if (!line.empty())
			{
				lastLine = line;
			}
		}
		readfile.close();

		if (!lastLine.empty())
		{
			// Last line is "YYYY-MM-DD HH:MM:SS,IP Address"
			size_t commaPos = lastLine.find(",");
			if (commaPos != std::string::npos)
			{
				lastIP = lastLine.substr(commaPos + 1);
			}
		}
	}

	// Append only if IP changed
	if (currentIP != lastIP)
	{
		std::ofstream outfile(ipLogPath, std::ios_base::app);
		outfile << getCurrentTimestamp() << "," << currentIP << "\n";
		outfile.close();
		std::cout << getCurrentTimestamp() << " - IP Address changed to " << currentIP << ".\n";
	}
	else
	{
		std::cout << getCurrentTimestamp() << " - IP Address remains " << currentIP << ".\n";
	}
}

int main()
{
	std::wstring wLogFilePath = GetFullPath("ip_log.csv"); // Initialize global variable with full path
	logFilePath = std::string(wLogFilePath.begin(), wLogFilePath.end());

	// Create a hidden window for the tray icon
	WNDCLASSW wc = {0};
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = GetModuleHandle(NULL);
	wc.lpszClassName = L"IPLoggerClass";
	RegisterClassW(&wc);

	hwnd = CreateWindowExW(
		0, L"IPLoggerClass", L"IP Logger",
		0, 0, 0, 0, 0,
		NULL, NULL, GetModuleHandle(NULL), NULL);

	CreateTrayIcon(hwnd);

	// Run indefinitely, checking every 5 seconds
	std::thread logThread([](const std::string &path)
						  {
        while (true)
        {
            logIPChange(path);
            std::this_thread::sleep_for(std::chrono::seconds(5));
        } }, logFilePath);

	// Message loop for tray icon
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	logThread.join();
	return 0;
}