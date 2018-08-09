#include "./call-stack-impl.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cxxabi.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <memory>
#include <sstream>
#include <string>
#include <typeinfo>

namespace {
    std::string executeCommand(const std::string& command)
    {
        char fileBuffer[1024];
        FILE* syscomFile = popen(command.c_str(), "r");
        fgets(fileBuffer, sizeof(fileBuffer), syscomFile);
        pclose(syscomFile);

        // Remove any line return
        for (auto i = 0u; i < 1024; ++i) {
            if (fileBuffer[i] == '\n' || fileBuffer[i] == '\r') {
                fileBuffer[i] = 0;
            }
        }

        return fileBuffer;
    }
}

using namespace lava::chamber;

void CallStack::Impl::refresh(uint32_t discardCount)
{
    m_entries.clear();

    using namespace abi;

    // retrieve call-stack
    void* trace[64];
    int stackDepth = backtrace(trace, sizeof(trace) / sizeof(void*));
    char** messages = backtrace_symbols(trace, stackDepth);

    // We discard this function for sure.
    for (int i = discardCount + 1; i < stackDepth; i++) {
        std::string message = messages[i];

        // We'll get filename and line number through addr2line function
        uint32_t leftParenthesisPos = message.find("(");
        uint32_t plusPos = message.find("+", leftParenthesisPos);
        if (plusPos > message.size()) plusPos = leftParenthesisPos;
        uint32_t rightParenthesisPos = message.find(")", plusPos);

        auto libPath = message.substr(0, leftParenthesisPos);
        auto reference = message.substr(leftParenthesisPos + 1, plusPos - leftParenthesisPos - 1);
        auto relativeOffset = message.substr(plusPos + 1, rightParenthesisPos - plusPos - 1);

        // Find lib offset
        std::string libOffset("0x0");
        if (reference.size() > 0 && reference != "__libc_start_main") {
            auto nmCommand = "nm \"" + libPath + "\" | grep " + reference + " | cut -d' ' -f 1";
            libOffset = "0x" + executeCommand(nmCommand);

            // If no symbol, the output is "nm: <libname>: no symbol".
            if ((libOffset[2] < '0' || libOffset[2] > '9') && (libOffset[2] < 'a' || libOffset[2] > 'f')) {
                libOffset = "0x0";
            }
        }

        auto addr2lineCommand =
            "addr2line -e \"" + libPath + "\" $(printf \"%X\" $((" + libOffset + " + " + relativeOffset + ")))";
        auto fileInfo = executeCommand(addr2lineCommand);

        // Get function name
        int status;
        Dl_info dlinfo;
        dladdr(trace[i], &dlinfo);
        const char* symname = dlinfo.dli_sname;
        char* demangled = __cxa_demangle(symname, nullptr, 0, &status);

        std::string functionName("??");
        if (status == 0 && demangled) {
            functionName = demangled;
            free(demangled);
        }
        else if (symname) {
            functionName = symname;
        }

        // Create entry
        Entry entry;
        entry.file = fileInfo;
        entry.file = entry.file.substr(entry.file.rfind("/") + 1);
        entry.file = entry.file.substr(0, entry.file.find(":"));
        if (entry.file[0] == '?' && dlinfo.dli_fname) {
            entry.file = dlinfo.dli_fname;
        }
        entry.line = atoi(fileInfo.substr(fileInfo.find(":") + 1).c_str());
        entry.function = functionName;
        m_entries.emplace_back(std::move(entry));
    }
}
