/* Copyright (c) 2013-2018 by Mercer Road Corp
 *
 * Permission to use, copy, modify or distribute this software in binary or source form
 * for any purpose is allowed only under explicit prior consent in writing from Mercer Road Corp
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND MERCER ROAD CORP DISCLAIMS
 * ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL MERCER ROAD CORP
 * BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */
#pragma once

# define _VX_STR(x) #x
# define VX_STR(x) _VX_STR(x)

# if defined(VIVOXDOC) || defined(SWIG)
#   define VIVOXSDK_DLLEXPORT
# else
#   ifdef _MSC_VER
#       ifdef BUILD_SHARED
#           ifdef BUILDING_VIVOXSDK
#               define VIVOXSDK_DLLEXPORT __declspec(dllexport)
#           else
#               define VIVOXSDK_DLLEXPORT __declspec(dllimport)
#           endif
#       else
#           define VIVOXSDK_DLLEXPORT
#       endif
#   else
#       define VIVOXSDK_DLLEXPORT __attribute__ ((visibility("default")))
#   endif
# endif

# if defined(__GNUC__)
#   define _VX_DO_PRAGMA(x) _Pragma(#x)
#   define VX_DEPRECATED_IGNORE_BEGIN()                                 \
    _VX_DO_PRAGMA(clang diagnostic push)                                \
    _VX_DO_PRAGMA(clang diagnostic ignored "-Wdeprecated-declarations") \
    _VX_DO_PRAGMA(clang diagnostic ignored "-W#pragma-messages")
#   define VX_DEPRECATED_IGNORE_END() \
    _VX_DO_PRAGMA(clang diagnostic pop)

# elif defined(_MSC_VER)
#   define _VX_DO_PRAGMA(x) __pragma(x)
#   define VX_DEPRECATED_IGNORE_BEGIN() \
    _VX_DO_PRAGMA(warning(push))        \
    _VX_DO_PRAGMA(warning(disable: 4995 4996))
#   define VX_DEPRECATED_IGNORE_END() \
    _VX_DO_PRAGMA(warning(pop))

# else
#   define _VX_DO_PRAGMA(x)
#   define VX_DEPRECATED_IGNORE_BEGIN()
#   define VX_DEPRECATED_IGNORE_END()
# endif

# if defined(__GNUC__)
#   define VX_DEPRECATED(since) __attribute__ ((deprecated("Since "#since)))
#   define VX_DEPRECATED_FOR(since, replacement) __attribute__ ((deprecated("Since "#since "; use "#replacement)))
#   define VX_DEPRECATED_ENUM_GCC(symbol, since) symbol VX_DEPRECATED(since)
#   define VX_DEPRECATED_ENUM_MSC(symbol)

# elif defined(_MSC_VER)
#   define VX_DEPRECATED(since) __declspec(deprecated("Since "#since))
#   define VX_DEPRECATED_FOR(since, replacement) __declspec(deprecated("Since "#since "; use "#replacement))
#   define VX_DEPRECATED_ENUM_GCC(symbol, since) symbol
#   define VX_DEPRECATED_ENUM_MSC(symbol) __pragma(deprecated(symbol))

# else
#   pragma message("WARNING: VX_DEPRECATED is not implemented for this compiler")
#   define VX_DEPRECATED(since)
#   define VX_DEPRECATED_FOR(since, replacement)
#   define VX_DEPRECATED_ENUM_GCC(symbol, since) symbol
#   define VX_DEPRECATED_ENUM_MSC(symbol)
# endif

# if defined(__GNUC__)
#   define VX_DEPRECATED_MACRO(name, since) _VX_DO_PRAGMA(message("Warning: '" _VX_STR(name) "' is deprecated: Since " VX_STR(since)))

# elif defined(_MSC_VER)
#   define VX_DEPRECATED_MACRO(name, since) _VX_DO_PRAGMA(message(__FILE__ "(" VX_STR(__LINE__) "): warning: '" _VX_STR(name) "' is deprecated: Since " VX_STR(since)))

# else
#   define VX_DEPRECATED_MACRO(name, since) name
# endif
