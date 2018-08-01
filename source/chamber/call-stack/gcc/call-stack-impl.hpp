#pragma once

#include "../../i-call-stack-impl.hpp"

#include <lava/chamber/call-stack.hpp>

namespace lava::chamber {
    /**
     * GCC-based implmentation of the call-stack generation.
     */
    class CallStack::Impl final : public ICallStackImpl {
    public:
        // ICallStackImpl
        void refresh(uint32_t discardCount) override final;
    };
}
