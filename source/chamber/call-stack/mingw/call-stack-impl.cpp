#include "./call-stack-impl.hpp"

#include <cxxabi.h>
#include <windows.h>

#include <DbgHelp.h> // @note To be kept below windows.h

using namespace lava::chamber;

void CallStack::Impl::refresh(uint32_t discardCount)
{
    m_entries.clear();
    uint32_t toBeDiscarded = discardCount;

    HANDLE process = GetCurrentProcess();
    HANDLE thread = GetCurrentThread();

    CONTEXT context;
    memset(&context, 0, sizeof(CONTEXT));
    context.ContextFlags = CONTEXT_FULL;
    RtlCaptureContext(&context);

    SymInitialize(process, NULL, TRUE);
    SymSetOptions(0);

    DWORD image;
    STACKFRAME64 stackframe;
    ZeroMemory(&stackframe, sizeof(STACKFRAME64));

    image = IMAGE_FILE_MACHINE_AMD64;
    stackframe.AddrPC.Offset = context.Rip;
    stackframe.AddrPC.Mode = AddrModeFlat;
    stackframe.AddrFrame.Offset = context.Rsp;
    stackframe.AddrFrame.Mode = AddrModeFlat;
    stackframe.AddrStack.Offset = context.Rsp;
    stackframe.AddrStack.Mode = AddrModeFlat;

    while (StackWalk64(image, process, thread, &stackframe, &context, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL)) {
        char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
        PSYMBOL_INFO symbol = (PSYMBOL_INFO)buffer;
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        symbol->MaxNameLen = MAX_SYM_NAME;

        DWORD64 displacement = 0;
        BOOL foundSymbol = SymFromAddr(process, stackframe.AddrPC.Offset, &displacement, symbol);
        if (!foundSymbol) continue;

        if (toBeDiscarded > 0) {
            toBeDiscarded--;
            continue;
        }

        //----- File name
        // Find module name
        MEMORY_BASIC_INFORMATION infoMemory;
        VirtualQuery(reinterpret_cast<LPVOID>(stackframe.AddrPC.Offset), &infoMemory, sizeof(infoMemory));
        auto module = reinterpret_cast<HMODULE>(infoMemory.AllocationBase);
        TCHAR wModuleName[MAX_PATH];
        GetModuleFileName(module, wModuleName, MAX_PATH);

        std::wstring_convert<std::codecvt_utf8<wchar_t>> wstrConvert;
        const auto moduleName = wstrConvert.to_bytes(reinterpret_cast<wchar_t*>(wModuleName));

        // From module name, find line infos
        std::stringstream addr2lineCommand;
        addr2lineCommand << "addr2line -e \"" << moduleName << "\" " << std::hex << stackframe.AddrPC.Offset << std::dec;

        std::string fileBuffer;
        fileBuffer.resize(1024);
        FILE* syscomFile = popen(addr2lineCommand.str().c_str(), "r");
        fgets(const_cast<char*>(fileBuffer.data()), fileBuffer.size(), syscomFile);
        pclose(syscomFile);

        //----- Function name
        // Tricky trick to add missing underscore of C++ functions
        // This can lead to errors on some case, but hey, I'm not here for that
        std::string functionName = symbol->Name;
        if (functionName[0] == 'Z') {
            functionName = "_" + functionName;
        }

        int status;
        char* demangled = abi::__cxa_demangle(functionName.c_str(), nullptr, 0, &status);
        if (status == 0 && demangled) {
            functionName = demangled;
            free(demangled);
        }

        // Create entry
        const uint32_t fileSeperator = fileBuffer.rfind("/");
        const uint32_t numberSeparator = fileBuffer.rfind(":");

        Entry entry;
        entry.file = fileBuffer.substr(fileSeperator + 1, numberSeparator - fileSeperator);
        entry.line = atoi(fileBuffer.substr(numberSeparator + 1).c_str());
        entry.function = functionName;
        m_entries.emplace_back(std::move(entry));
    }

    SymCleanup(process);
}
