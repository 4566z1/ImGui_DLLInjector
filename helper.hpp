#pragma once
#include <string>
#include <optional>
using std::optional;
using std::string_view;
using std::wstring_view;

namespace Helper {
	optional<int> GetProcessId(string_view process_name);
}