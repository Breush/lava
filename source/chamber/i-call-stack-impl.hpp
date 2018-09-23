#pragma once

namespace lava::chamber {
    /**
     * Base interface for platform specific CallStack::Impl.
     */
    class ICallStackImpl {
    public:
        virtual ~ICallStackImpl() = default;

        /**
         * Recompute the current callstack.
         *
         * Specify how many fonctions in the stack to ignore.
         */
        virtual void refresh(uint32_t discardCount) = 0;

        /// Write the callstack at the last refresh.
        std::string toString() const;

    protected:
        /// Holds a stack information.
        struct Entry {
            uint32_t line = 0u;
            std::string file;
            std::string function;
        };

    protected:
        /// Holds all the stack information.
        std::vector<Entry> m_entries;
    };
}
