#include "binder.hpp"

#include <spdlog/spdlog.h>
#include <unordered_map>
#include <filesystem>

using NativeFunctionPtr = void(__cdecl *)(Stack&, Memory*);

#if not defined(_WIN32) || not defined(_WIN64)
#include <dlfcn.h>

std::unordered_map<std::filesystem::path, void*> s_handles;

std::function<void(Stack&, Memory*)> Binder::getFunction(const std::string& lib, const std::string& name) {
    std::filesystem::path libPath = std::filesystem::current_path() / ("lib" + lib + ".so");

    if(!std::filesystem::exists(libPath)) {
        spdlog::error("Cannot find library {}", libPath.string());
        return nullptr;
    }

    void* handle = nullptr;

    if(s_handles.contains(libPath))
        handle = s_handles[libPath];
    else
        handle = dlopen(libPath.string().c_str(), RTLD_LAZY);

    if(!handle) {
        const char* error = dlerror();
        spdlog::error("{}", error);
        return nullptr;
    }

    dlerror();

    NativeFunctionPtr funcPtr = 
        (NativeFunctionPtr)dlsym(handle, name.c_str());
    const char* error = dlerror();
    if (error) {
        spdlog::error("{}", error);
        dlclose(handle);
        return nullptr;
    }

    return funcPtr;
}

void Binder::closeHandles() {
    for(auto& handle : s_handles)
        dlclose(handle.second);
}
#else
#include <windows.h>

std::unordered_map<std::filesystem::path, HMODULE> s_handles;

std::function<void(Stack&, Memory*)> Binder::getFunction(const std::string& lib, const std::string& name) {
    std::filesystem::path libPath = std::filesystem::current_path() / (lib + ".dll");

    if(!std::filesystem::exists(libPath)) {
        spdlog::error("Cannot find library {}", libPath.string());
        return nullptr;
    }

    HMODULE handle = nullptr;

    if(s_handles.contains(libPath))
        handle = s_handles[libPath];
    else
        handle = LoadLibrary(libPath.string().c_str());

    if(!handle)
        return nullptr;

    NativeFunctionPtr funcPtr = 
        (NativeFunctionPtr)GetProcAddress(handle, name.c_str());
    
    if (!funcPtr) {
        spdlog::error("Error code {}", GetLastError());
        FreeLibrary(handle);
        return nullptr;
    }

    return funcPtr;
}

void Binder::closeHandles() {
    for(auto& handle : s_handles)
        FreeLibrary(handle.second);
}
#endif