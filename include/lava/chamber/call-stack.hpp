#pragma once

#if defined(__GNUC__)
#if not defined(__MINGW32__)
#include <execinfo.h>
#endif
#endif

#include <string>
#include <vector>

namespace lava::chamber {
    /**
     * To retrieve the current call-stack.
     */
    class CallStack {
    public:
        /**
         * Recompute the current callstack.
         */
        void refresh(const uint32_t numDiscard = 0);

        std::string toString() const;

    protected:
        struct Entry {
            uint32_t line = 0;
            std::string file;
            std::string function;
        };

    private:
        std::vector<Entry> m_entries;
    };

    std::ostream& operator<<(std::ostream& stream, CallStack& callStack);
}
