#include <Windows.h>
#include <fstream>
#include "injector.hpp"
using std::ifstream;

bool Injector::checkDLL(string_view dll_path) {
	ifstream file_stream;
	char signature[3] = { 0 };
	file_stream.open(string(dll_path), std::ios_base::binary);
	if (!file_stream.is_open()) return false;
	file_stream.read(signature, 2);
	if (strcmp(signature, "MZ") != 0) return false;
	return true;
}

string Injector::injectProcess(
	const int& process_id, 
	string_view dll_path)
{
	bool bRet;
	HANDLE process_handle;
	HANDLE remote_thread_handle;
	LPTSTR remote_filename_addr;
	PTHREAD_START_ROUTINE loadlibrary_addr;
	if (!checkDLL(dll_path)) return "DLL is invalid";
	if (process_id <= 0) return "Process id is invalid";

	process_handle = OpenProcess(PROCESS_ALL_ACCESS, false, process_id);
	if (!process_handle) return "OpenProcess failed";
		 
	remote_filename_addr = static_cast<LPTSTR>(VirtualAllocEx(process_handle, 0, dll_path.size(), MEM_COMMIT, PAGE_READWRITE));
	if (!remote_filename_addr) return "VirtualAlloc remote memory failed";

	bRet = WriteProcessMemory(process_handle, remote_filename_addr, dll_path.data(), dll_path.size(), nullptr);
	if (!bRet) return "WriteProcessMemory failed";

	loadlibrary_addr = reinterpret_cast<PTHREAD_START_ROUTINE>(GetProcAddress(GetModuleHandleA("Kernel32.dll"), "LoadLibraryA"));
	if (!loadlibrary_addr) return "GetProcAddress failed";

	remote_thread_handle = CreateRemoteThread(process_handle, 0, 0, loadlibrary_addr, remote_filename_addr, 0, 0);
	if (!remote_thread_handle) return "CreateRemoteThread failed";

	WaitForSingleObject(remote_thread_handle, INFINITE);
	return "Injected Successfully";
}