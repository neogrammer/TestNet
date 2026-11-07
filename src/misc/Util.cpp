#include <misc/include/Util.h>

std::string util::LPCWSTR_to_string(LPCWSTR lpwsz)
{
	int len = WideCharToMultiByte(CP_UTF8, 0, lpwsz, -1, nullptr, 0, nullptr, nullptr);
	if (len <= 0) return {};
	std::string result(len, '\0');
	WideCharToMultiByte(CP_UTF8, 0, lpwsz, -1, &result[0], len, nullptr, nullptr);
	return result;
}
