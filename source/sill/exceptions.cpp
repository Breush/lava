#include <lava/chamber/logger.hpp>

using namespace lava;

#if defined(LAVA_CHAMBER_CALLSTACK_GCC)
namespace {
    void logException(const std::string& exceptionName)
    {
        chamber::logger.error("sill.exception") << "Caught an exception of type " << exceptionName << "." << std::endl;
    }
}

#include <cxxabi.h>
#include <dlfcn.h>

extern "C" {
void __cxa_throw(void* ex, void* info, void (*dest)(void*))
{
    int status = 0;
    auto typeInfo = reinterpret_cast<const std::type_info*>(info);
    std::string exceptionName = abi::__cxa_demangle(typeInfo->name(), 0, 0, &status);

    logException(exceptionName);

    static void (*const rethrow)(void*, void*, void (*)(void*)) __attribute__((noreturn)) =
        (void (*)(void*, void*, void (*)(void*)))dlsym(RTLD_NEXT, "__cxa_throw");
    rethrow(ex, info, dest);
}
}
#endif
