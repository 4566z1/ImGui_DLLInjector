#pragma once
#include <string>
using std::string;
using std::string_view;

class Injector {
private:
	static bool checkDLL(string_view dll_path);
public:
	static string injectProcess(const int& process_id, string_view dll_path);
};