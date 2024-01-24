#include "ReloadDLL.h"

#include <filesystem>
#include <iostream>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#else
#include <dlfcn.h>
#endif

using update_type = decltype(Update);
static update_type *update_ptr;
void Update(int n) {
	update_ptr(n);
}

#ifndef _WIN32
static void PrintDLError(const std::string &function_name) {
	char *err = dlerror();
	if (err == nullptr) {
		abort();
	}
	std::cerr << "dlerror in " << function_name << "()" << ":\n" << err << "\n";
	abort();
}
#endif

static void *LoadDynamicFunction(void *dll, const std::string &function_name) {
	std::cout << "Loading dynamic function '" << function_name << "'\n";
#ifdef _WIN32
	void *proc = (void *)GetProcAddress((HMODULE)dll, function_name.c_str());
	if (!proc) {
		abort();
	}
#else
	void* proc = dlsym(dll, function_name.c_str());
	if (!proc) {
		PrintDLError("LoadDynamicFunction");
	}
#endif
	return proc;
}

static void *LoadDynamicLibrary(const std::string &dll_path) {
#ifdef _WIN32
	HMODULE lib = LoadLibraryA(dll_path.c_str());
	if (!lib) {
		abort();
	}
#else
	void *lib = dlopen(("./" + dll_path).c_str(), RTLD_NOW);
	if (!lib) {
		PrintDLError("LoadDynamicLibrary");
	}
#endif
	return lib;
}

static void FreeDynamicLibrary(void *dll) {
#ifdef _WIN32
	if (!FreeLibrary((HMODULE)dll)) {
		abort();
	}
#else
	if (dlclose(dll)) {
		PrintDLError("FreeDynamicLibrary");
	}
#endif
}

void ReloadDLL() {
	static void *dll;
	static std::filesystem::file_time_type prev_write_time = std::filesystem::file_time_type::min();

	static const std::string dll_path = "program.dll";

	const std::filesystem::file_time_type current_write_time = std::filesystem::last_write_time(dll_path);

	if (current_write_time > prev_write_time) {
		if (dll) {
			FreeDynamicLibrary(dll);
			dll = nullptr;
			std::cout << "Freed " << dll_path << "\n";
		}

		dll = LoadDynamicLibrary(dll_path);
		std::cout << "Loaded " << dll_path << "\n";

		update_ptr = (update_type *)LoadDynamicFunction(dll, "Update");

		std::cout << "Loaded all dynamic functions\n";

		prev_write_time = current_write_time;
	}
}
