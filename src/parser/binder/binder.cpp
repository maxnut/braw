#include "binder.hpp"

#include <filesystem>
#include <dlfcn.h>
#include <spdlog/spdlog.h>
#include <unordered_map>

using NativeFunctionPtr = void(*)(Stack&, Memory*, const std::vector<TypeInfo>&);

std::unordered_map<std::filesystem::path, void*> s_handles;

std::function<void(Stack&, Memory*, const std::vector<TypeInfo>&)> Binder::getFunction(const std::string& lib, const std::string& name) {
    std::filesystem::path libPath = std::filesystem::current_path() / ("lib" + lib + ".so");

    void* handle = nullptr;

    if(s_handles.contains(libPath))
        handle = s_handles[libPath];
    else
        handle = dlopen(libPath.string().c_str(), RTLD_LAZY);

    if(!handle)
        return nullptr;

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