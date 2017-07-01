#include <lava/chamber/call-stack.hpp>

#if defined(__GNUC__) && not defined(__MINGW32__)
#include <execinfo.h>
#endif

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cxxabi.h>
#include <dlfcn.h>
#include <memory>
#include <string>
#include <typeinfo>

#include <sstream>

using namespace lava;

std::ostream& chamber::operator<<(std::ostream& stream, chamber::CallStack& callStack)
{
    stream << callStack.toString();
    return stream;
}

using namespace lava::chamber;

void CallStack::refresh(const uint32_t numDiscard)
{
    m_entries.clear();

#if defined(__GNUC__) && not defined(__MINGW32__)

    using namespace abi;

    // retrieve call-stack
    void* trace[64];
    int stackDepth = backtrace(trace, sizeof(trace) / sizeof(void*));
    char** messages = backtrace_symbols(trace, stackDepth);

    // We discard this function for sure.
    for (int i = numDiscard + 1; i < stackDepth; i++) {
        // Find first occurence of '(' or ' ' in message[i]
        int p = 0;
        while (messages[i][p] != '(' && messages[i][p] != ' ' && messages[i][p] != 0) ++p;

        // We'll get filename and line number through addr2line function
        char syscom[256];
        sprintf(syscom, "addr2line %p -e %.*s", trace[i], p, messages[i]);

        char fileBuffer[1024];
        FILE* syscomFile = popen(syscom, "r");
        fgets(fileBuffer, sizeof(fileBuffer), syscomFile);
        pclose(syscomFile);

        // Get function name
        Dl_info dlinfo;
        dladdr(trace[i], &dlinfo);

        int status;
        const char* symname = dlinfo.dli_sname;
        char* demangled = abi::__cxa_demangle(symname, nullptr, 0, &status);
        if (status == 0 && demangled) symname = demangled;

        // Create entry
        Entry entry;
        entry.file = fileBuffer;
        entry.file = entry.file.substr(entry.file.rfind("/") + 1);
        entry.file = entry.file.substr(0, entry.file.find(":"));
        if (entry.file[0] == '?' && dlinfo.dli_fname) entry.file = dlinfo.dli_fname;
        entry.line = atoi(strstr(fileBuffer, ":") + 1);
        entry.function = (symname) ? symname : "??";
        m_entries.emplace_back(std::move(entry));

        // Freeing
        if (demangled) free(demangled);

        // Stop when we hit main
        if (entry.function == "main") break;
    }

#else

    Entry entry;
    entry.file = "Unable to generate the callStack on your system.";
    m_entries.emplace_back(std::move(entry));

#endif
}

std::string CallStack::toString() const
{
    std::ostringstream os;
    for (auto& entry : m_entries) {
        os << entry.file << ":" << entry.line << " [" << entry.function << "]" << std::endl;
    }
    return os.str();
}
