#include<Windows.h>

LPVOID GTI(HANDLE token, TOKEN_INFORMATION_CLASS cls)
{
	DWORD info_sz;
	GetTokenInformation(token, cls, NULL, 0, &info_sz);
	LPVOID user = VirtualAlloc(NULL, info_sz, MEM_COMMIT, PAGE_READWRITE);
	GetTokenInformation(token, cls, user, info_sz, &info_sz);
	return user;
}

int wWinMain(
	_In_ HINSTANCE hInstance,
	_In_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow
)
{
	UNREFERENCED_PARAMETER(hInstance);
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);

	HANDLE device = CreateFile(L"\\\\.\\KMDFDriver1", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	DWORD err = GetLastError();

	DWORD pid = GetProcessId(GetCurrentProcess());

	LPWSTR mystr = (LPWSTR)(&pid);

	LPWSTR outstr = (LPWSTR)VirtualAlloc(NULL, 65536, MEM_COMMIT, PAGE_READWRITE);

	DWORD outlen;

	DeviceIoControl(device, 0, mystr, 4, outstr, 65535, &outlen, NULL);

	DebugBreak();

	UNREFERENCED_PARAMETER(device);
	UNREFERENCED_PARAMETER(err);
}