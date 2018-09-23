#include <lava/chamber/call-stack.hpp>

#if defined(LAVA_CHAMBER_CALLSTACK_GCC)
#include "./call-stack/gcc/call-stack-impl.hpp"
#elif defined(LAVA_CHAMBER_CALLSTACK_MINGW)
#include "./call-stack/mingw/call-stack-impl.hpp"
#else
#error "[lava.chamber.call-stack] No call-stack system defined."
#endif

std::ostream& operator<<(std::ostream& stream, lava::chamber::CallStack& callStack)
{
    stream << callStack.toString();
    return stream;
}

using namespace lava::chamber;

$pimpl_class(CallStack);

$pimpl_method(CallStack, void, refresh, uint32_t, discardCount);
$pimpl_method_const(CallStack, std::string, toString);
