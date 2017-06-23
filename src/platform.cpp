#include <platform.hpp>

#if defined(_WIN32) || defined(__CYGWIN__)

int setenv(const char *name, const char *value, int overwrite) {
	int errcode = 0;

	std::wstring wname = cstring_to_wstring(name);
	std::wstring wvalue = cstring_to_wstring(value);

	if (!overwrite) {
		size_t envsize = 0;
		errcode = _wgetenv_s(&envsize, nullptr, 0, wname.c_str());
		if (errcode || envsize) return errcode;
	}
	return _wputenv_s(wname.c_str(), wvalue.c_str());
}
int unsetenv(const char *name) {
	return setenv(name, "", 1);
}

#endif
