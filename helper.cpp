#include "helper.hpp"
#include <Windows.h>
#include <TlHelp32.h>

optional<int> Helper::GetProcessId(string_view process_name) {
	PROCESSENTRY32W process_entry = { 0 };
	HANDLE toolhelp_handle;

	// char to wchar
	size_t converted_size = process_name.size();
	wchar_t buffer[MAXCHAR];
	mbstowcs_s(&converted_size, buffer, process_name.data(), MAXCHAR);
	wstring_view wchar_process_name(buffer);

	process_entry.dwSize = sizeof(process_entry);
	toolhelp_handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (!toolhelp_handle) return -1;

	Process32FirstW(toolhelp_handle, &process_entry);
	do {
		if (!wchar_process_name.compare(process_entry.szExeFile))
			return process_entry.th32ProcessID;
	} while (Process32NextW(toolhelp_handle, &process_entry));

	return std::nullopt;
}
