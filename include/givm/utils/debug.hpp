#ifndef GIVM_UTILS_DEBUG_HPP
#define GIVM_UTILS_DEBUG_HPP

#include <cstdint>
#include <format>
#include <iostream>
#include <source_location>
#include <string_view>
#include <utility>
#include <version>

#if defined(__cpp_lib_stacktrace)
#include <stacktrace>
#endif

namespace givm
{
    template<class...Args>
    inline void debug_log(const std::format_string<Args...> fmt, Args&&...args)
    {
        std::cout << std::format(fmt, (Args&&)args...) << '\n';
    }

    // The parentheses keep the C assert macro from treating this declaration as an invocation.
#if defined(__cpp_lib_stacktrace)
    inline void (assert)(bool value, std::string_view expression = "", std::stacktrace stack = std::stacktrace::current())
    {
        if(value)
        {
            return;
        }
        debug_log("assert faild({}):\n{}", expression, stack);
    }
#else
    inline void (assert)(bool value, std::string_view expression = "")
    {
        if(value)
        {
            return;
        }
        debug_log("assert faild({})", expression);
    }
#endif
}

#endif
