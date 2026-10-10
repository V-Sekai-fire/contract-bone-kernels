#ifndef SLANG_CPP_PRELUDE_H
#define SLANG_CPP_PRELUDE_H

// Because the signature of isnan, isfinite, and is isinf changed in C++, we use the macro
// to use the version in the std namespace.
// https://stackoverflow.com/questions/39130040/cmath-hides-isnan-in-math-h-in-c14-c11

#ifdef SLANG_LLVM
#ifndef SLANG_LLVM_H
#define SLANG_LLVM_H

// TODO(JS):
// Disable exception declspecs, as not supported on LLVM without some extra options.
// We could enable with `-fms-extensions`
#define SLANG_DISABLE_EXCEPTIONS 1

#ifndef SLANG_PRELUDE_ASSERT
#ifdef SLANG_PRELUDE_ENABLE_ASSERT
extern "C" void assertFailure(const char* msg);
#define SLANG_PRELUDE_EXPECT(VALUE, MSG) \
    if (VALUE)                           \
    {                                    \
    }                                    \
    else                                 \
        assertFailure("assertion failed: '" MSG "'")
#define SLANG_PRELUDE_ASSERT(VALUE) SLANG_PRELUDE_EXPECT(VALUE, #VALUE)
#else // SLANG_PRELUDE_ENABLE_ASSERT
#define SLANG_PRELUDE_EXPECT(VALUE, MSG)
#define SLANG_PRELUDE_ASSERT(x)
#endif // SLANG_PRELUDE_ENABLE_ASSERT
#endif

/*
Taken from stddef.h
*/

typedef __PTRDIFF_TYPE__ ptrdiff_t;
typedef __SIZE_TYPE__ size_t;
typedef __SIZE_TYPE__ rsize_t;

// typedef __WCHAR_TYPE__ wchar_t;

#if defined(__need_NULL)
#undef NULL
#ifdef __cplusplus
#if !defined(__MINGW32__) && !defined(_MSC_VER)
#define NULL __null
#else
#define NULL 0
#endif
#else
#define NULL ((void*)0)
#endif
#ifdef __cplusplus
#if defined(_MSC_EXTENSIONS) && defined(_NATIVE_NULLPTR_SUPPORTED)
namespace std
{
typedef decltype(nullptr) nullptr_t;
}
using ::std::nullptr_t;
#endif
#endif
#undef __need_NULL
#endif /* defined(__need_NULL) */


/*
The following are taken verbatim from stdint.h from Clang in LLVM. Only 8/16/32/64 types are needed.
*/

// LLVM/Clang types such that we can use LLVM/Clang without headers for C++ output from Slang

#ifdef __INT64_TYPE__
#ifndef __int8_t_defined /* glibc sys/types.h also defines int64_t*/
typedef __INT64_TYPE__ int64_t;
#endif /* __int8_t_defined */
typedef __UINT64_TYPE__ uint64_t;
#define __int_least64_t int64_t
#define __uint_least64_t uint64_t
#endif /* __INT64_TYPE__ */

#ifdef __int_least64_t
typedef __int_least64_t int_least64_t;
typedef __uint_least64_t uint_least64_t;
typedef __int_least64_t int_fast64_t;
typedef __uint_least64_t uint_fast64_t;
#endif /* __int_least64_t */

#ifdef __INT32_TYPE__

#ifndef __int8_t_defined /* glibc sys/types.h also defines int32_t*/
typedef __INT32_TYPE__ int32_t;
#endif /* __int8_t_defined */

#ifndef __uint32_t_defined /* more glibc compatibility */
#define __uint32_t_defined
typedef __UINT32_TYPE__ uint32_t;
#endif /* __uint32_t_defined */

#define __int_least32_t int32_t
#define __uint_least32_t uint32_t
#endif /* __INT32_TYPE__ */

#ifdef __int_least32_t
typedef __int_least32_t int_least32_t;
typedef __uint_least32_t uint_least32_t;
typedef __int_least32_t int_fast32_t;
typedef __uint_least32_t uint_fast32_t;
#endif /* __int_least32_t */

#ifdef __INT16_TYPE__
#ifndef __int8_t_defined /* glibc sys/types.h also defines int16_t*/
typedef __INT16_TYPE__ int16_t;
#endif /* __int8_t_defined */
typedef __UINT16_TYPE__ uint16_t;
#define __int_least16_t int16_t
#define __uint_least16_t uint16_t
#endif /* __INT16_TYPE__ */

#ifdef __int_least16_t
typedef __int_least16_t int_least16_t;
typedef __uint_least16_t uint_least16_t;
typedef __int_least16_t int_fast16_t;
typedef __uint_least16_t uint_fast16_t;
#endif /* __int_least16_t */

#ifdef __INT8_TYPE__
#ifndef __int8_t_defined /* glibc sys/types.h also defines int8_t*/
typedef __INT8_TYPE__ int8_t;
#endif /* __int8_t_defined */
typedef __UINT8_TYPE__ uint8_t;
#define __int_least8_t int8_t
#define __uint_least8_t uint8_t
#endif /* __INT8_TYPE__ */

#ifdef __int_least8_t
typedef __int_least8_t int_least8_t;
typedef __uint_least8_t uint_least8_t;
typedef __int_least8_t int_fast8_t;
typedef __uint_least8_t uint_fast8_t;
#endif /* __int_least8_t */

/* prevent glibc sys/types.h from defining conflicting types */
#ifndef __int8_t_defined
#define __int8_t_defined
#endif /* __int8_t_defined */

/* C99 7.18.1.4 Integer types capable of holding object pointers.
 */
#define __stdint_join3(a, b, c) a##b##c

#ifndef _INTPTR_T
#ifndef __intptr_t_defined
typedef __INTPTR_TYPE__ intptr_t;
#define __intptr_t_defined
#define _INTPTR_T
#endif
#endif

#ifndef _UINTPTR_T
typedef __UINTPTR_TYPE__ uintptr_t;
#define _UINTPTR_T
#endif

/* C99 7.18.1.5 Greatest-width integer types.
 */
typedef __INTMAX_TYPE__ intmax_t;
typedef __UINTMAX_TYPE__ uintmax_t;

/* C99 7.18.4 Macros for minimum-width integer constants.
 *
 * The standard requires that integer constant macros be defined for all the
 * minimum-width types defined above. As 8-, 16-, 32-, and 64-bit minimum-width
 * types are required, the corresponding integer constant macros are defined
 * here. This implementation also defines minimum-width types for every other
 * integer width that the target implements, so corresponding macros are
 * defined below, too.
 *
 * These macros are defined using the same successive-shrinking approach as
 * the type definitions above. It is likewise important that macros are defined
 * in order of decending width.
 *
 * Note that C++ should not check __STDC_CONSTANT_MACROS here, contrary to the
 * claims of the C standard (see C++ 18.3.1p2, [cstdint.syn]).
 */

#define __int_c_join(a, b) a##b
#define __int_c(v, suffix) __int_c_join(v, suffix)
#define __uint_c(v, suffix) __int_c_join(v##U, suffix)

#ifdef __INT64_TYPE__
#ifdef __INT64_C_SUFFIX__
#define __int64_c_suffix __INT64_C_SUFFIX__
#else
#undef __int64_c_suffix
#endif /* __INT64_C_SUFFIX__ */
#endif /* __INT64_TYPE__ */

#ifdef __int_least64_t
#ifdef __int64_c_suffix
#define INT64_C(v) __int_c(v, __int64_c_suffix)
#define UINT64_C(v) __uint_c(v, __int64_c_suffix)
#else
#define INT64_C(v) v
#define UINT64_C(v) v##U
#endif /* __int64_c_suffix */
#endif /* __int_least64_t */


#ifdef __INT32_TYPE__
#ifdef __INT32_C_SUFFIX__
#define __int32_c_suffix __INT32_C_SUFFIX__
#else
#undef __int32_c_suffix
#endif /* __INT32_C_SUFFIX__ */
#endif /* __INT32_TYPE__ */

#ifdef __int_least32_t
#ifdef __int32_c_suffix
#define INT32_C(v) __int_c(v, __int32_c_suffix)
#define UINT32_C(v) __uint_c(v, __int32_c_suffix)
#else
#define INT32_C(v) v
#define UINT32_C(v) v##U
#endif /* __int32_c_suffix */
#endif /* __int_least32_t */

#ifdef __INT16_TYPE__
#ifdef __INT16_C_SUFFIX__
#define __int16_c_suffix __INT16_C_SUFFIX__
#else
#undef __int16_c_suffix
#endif /* __INT16_C_SUFFIX__ */
#endif /* __INT16_TYPE__ */

#ifdef __int_least16_t
#ifdef __int16_c_suffix
#define INT16_C(v) __int_c(v, __int16_c_suffix)
#define UINT16_C(v) __uint_c(v, __int16_c_suffix)
#else
#define INT16_C(v) v
#define UINT16_C(v) v##U
#endif /* __int16_c_suffix */
#endif /* __int_least16_t */


#ifdef __INT8_TYPE__
#ifdef __INT8_C_SUFFIX__
#define __int8_c_suffix __INT8_C_SUFFIX__
#else
#undef __int8_c_suffix
#endif /* __INT8_C_SUFFIX__ */
#endif /* __INT8_TYPE__ */

#ifdef __int_least8_t
#ifdef __int8_c_suffix
#define INT8_C(v) __int_c(v, __int8_c_suffix)
#define UINT8_C(v) __uint_c(v, __int8_c_suffix)
#else
#define INT8_C(v) v
#define UINT8_C(v) v##U
#endif /* __int8_c_suffix */
#endif /* __int_least8_t */

/* C99 7.18.2.1 Limits of exact-width integer types.
 * C99 7.18.2.2 Limits of minimum-width integer types.
 * C99 7.18.2.3 Limits of fastest minimum-width integer types.
 *
 * The presence of limit macros are completely optional in C99.  This
 * implementation defines limits for all of the types (exact- and
 * minimum-width) that it defines above, using the limits of the minimum-width
 * type for any types that do not have exact-width representations.
 *
 * As in the type definitions, this section takes an approach of
 * successive-shrinking to determine which limits to use for the standard (8,
 * 16, 32, 64) bit widths when they don't have exact representations. It is
 * therefore important that the definitions be kept in order of decending
 * widths.
 *
 * Note that C++ should not check __STDC_LIMIT_MACROS here, contrary to the
 * claims of the C standard (see C++ 18.3.1p2, [cstdint.syn]).
 */

#ifdef __INT64_TYPE__
#define INT64_MAX INT64_C(9223372036854775807)
#define INT64_MIN (-INT64_C(9223372036854775807) - 1)
#define UINT64_MAX UINT64_C(18446744073709551615)
#define __INT_LEAST64_MIN INT64_MIN
#define __INT_LEAST64_MAX INT64_MAX
#define __UINT_LEAST64_MAX UINT64_MAX
#endif /* __INT64_TYPE__ */

#ifdef __INT_LEAST64_MIN
#define INT_LEAST64_MIN __INT_LEAST64_MIN
#define INT_LEAST64_MAX __INT_LEAST64_MAX
#define UINT_LEAST64_MAX __UINT_LEAST64_MAX
#define INT_FAST64_MIN __INT_LEAST64_MIN
#define INT_FAST64_MAX __INT_LEAST64_MAX
#define UINT_FAST64_MAX __UINT_LEAST64_MAX
#endif /* __INT_LEAST64_MIN */

#ifdef __INT32_TYPE__
#define INT32_MAX INT32_C(2147483647)
#define INT32_MIN (-INT32_C(2147483647) - 1)
#define UINT32_MAX UINT32_C(4294967295)
#define __INT_LEAST32_MIN INT32_MIN
#define __INT_LEAST32_MAX INT32_MAX
#define __UINT_LEAST32_MAX UINT32_MAX
#endif /* __INT32_TYPE__ */

#ifdef __INT_LEAST32_MIN
#define INT_LEAST32_MIN __INT_LEAST32_MIN
#define INT_LEAST32_MAX __INT_LEAST32_MAX
#define UINT_LEAST32_MAX __UINT_LEAST32_MAX
#define INT_FAST32_MIN __INT_LEAST32_MIN
#define INT_FAST32_MAX __INT_LEAST32_MAX
#define UINT_FAST32_MAX __UINT_LEAST32_MAX
#endif /* __INT_LEAST32_MIN */

#ifdef __INT16_TYPE__
#define INT16_MAX INT16_C(32767)
#define INT16_MIN (-INT16_C(32767) - 1)
#define UINT16_MAX UINT16_C(65535)
#define __INT_LEAST16_MIN INT16_MIN
#define __INT_LEAST16_MAX INT16_MAX
#define __UINT_LEAST16_MAX UINT16_MAX
#endif /* __INT16_TYPE__ */

#ifdef __INT_LEAST16_MIN
#define INT_LEAST16_MIN __INT_LEAST16_MIN
#define INT_LEAST16_MAX __INT_LEAST16_MAX
#define UINT_LEAST16_MAX __UINT_LEAST16_MAX
#define INT_FAST16_MIN __INT_LEAST16_MIN
#define INT_FAST16_MAX __INT_LEAST16_MAX
#define UINT_FAST16_MAX __UINT_LEAST16_MAX
#endif /* __INT_LEAST16_MIN */


#ifdef __INT8_TYPE__
#define INT8_MAX INT8_C(127)
#define INT8_MIN (-INT8_C(127) - 1)
#define UINT8_MAX UINT8_C(255)
#define __INT_LEAST8_MIN INT8_MIN
#define __INT_LEAST8_MAX INT8_MAX
#define __UINT_LEAST8_MAX UINT8_MAX
#endif /* __INT8_TYPE__ */

#ifdef __INT_LEAST8_MIN
#define INT_LEAST8_MIN __INT_LEAST8_MIN
#define INT_LEAST8_MAX __INT_LEAST8_MAX
#define UINT_LEAST8_MAX __UINT_LEAST8_MAX
#define INT_FAST8_MIN __INT_LEAST8_MIN
#define INT_FAST8_MAX __INT_LEAST8_MAX
#define UINT_FAST8_MAX __UINT_LEAST8_MAX
#endif /* __INT_LEAST8_MIN */

/* Some utility macros */
#define __INTN_MIN(n) __stdint_join3(INT, n, _MIN)
#define __INTN_MAX(n) __stdint_join3(INT, n, _MAX)
#define __UINTN_MAX(n) __stdint_join3(UINT, n, _MAX)
#define __INTN_C(n, v) __stdint_join3(INT, n, _C(v))
#define __UINTN_C(n, v) __stdint_join3(UINT, n, _C(v))

/* C99 7.18.2.4 Limits of integer types capable of holding object pointers. */
/* C99 7.18.3 Limits of other integer types. */

#define INTPTR_MIN (-__INTPTR_MAX__ - 1)
#define INTPTR_MAX __INTPTR_MAX__
#define UINTPTR_MAX __UINTPTR_MAX__
#define PTRDIFF_MIN (-__PTRDIFF_MAX__ - 1)
#define PTRDIFF_MAX __PTRDIFF_MAX__
#define SIZE_MAX __SIZE_MAX__

/* ISO9899:2011 7.20 (C11 Annex K): Define RSIZE_MAX if __STDC_WANT_LIB_EXT1__
 * is enabled. */
#if defined(__STDC_WANT_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__ >= 1
#define RSIZE_MAX (SIZE_MAX >> 1)
#endif

/* C99 7.18.2.5 Limits of greatest-width integer types. */
#define INTMAX_MIN (-__INTMAX_MAX__ - 1)
#define INTMAX_MAX __INTMAX_MAX__
#define UINTMAX_MAX __UINTMAX_MAX__

/* C99 7.18.3 Limits of other integer types. */
#define SIG_ATOMIC_MIN __INTN_MIN(__SIG_ATOMIC_WIDTH__)
#define SIG_ATOMIC_MAX __INTN_MAX(__SIG_ATOMIC_WIDTH__)
#ifdef __WINT_UNSIGNED__
#define WINT_MIN __UINTN_C(__WINT_WIDTH__, 0)
#define WINT_MAX __UINTN_MAX(__WINT_WIDTH__)
#else
#define WINT_MIN __INTN_MIN(__WINT_WIDTH__)
#define WINT_MAX __INTN_MAX(__WINT_WIDTH__)
#endif

#ifndef WCHAR_MAX
#define WCHAR_MAX __WCHAR_MAX__
#endif
#ifndef WCHAR_MIN
#if __WCHAR_MAX__ == __INTN_MAX(__WCHAR_WIDTH__)
#define WCHAR_MIN __INTN_MIN(__WCHAR_WIDTH__)
#else
#define WCHAR_MIN __UINTN_C(__WCHAR_WIDTH__, 0)
#endif
#endif

/* 7.18.4.2 Macros for greatest-width integer constants. */
#define INTMAX_C(v) __int_c(v, __INTMAX_C_SUFFIX__)
#define UINTMAX_C(v) __int_c(v, __UINTMAX_C_SUFFIX__)


#endif // SLANG_LLVM_H

#else // SLANG_LLVM
#if SLANG_GCC_FAMILY && __GNUC__ < 6
#include <cmath>
#define SLANG_PRELUDE_STD std::
#else
#include <math.h>
#define SLANG_PRELUDE_STD
#endif

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#endif // SLANG_LLVM

// Is intptr_t not equal to equal-width sized integer type?
#if defined(__APPLE__)
#define SLANG_INTPTR_TYPE_IS_DISTINCT 1
#else
#define SLANG_INTPTR_TYPE_IS_DISTINCT 0
#endif

#if defined(_MSC_VER)
#define SLANG_PRELUDE_SHARED_LIB_EXPORT __declspec(dllexport)
#else
#define SLANG_PRELUDE_SHARED_LIB_EXPORT __attribute__((__visibility__("default")))
// #   define SLANG_PRELUDE_SHARED_LIB_EXPORT __attribute__ ((dllexport))
// __attribute__((__visibility__("default")))
#endif

#ifdef __cplusplus
#define SLANG_PRELUDE_EXTERN_C extern "C"
#define SLANG_PRELUDE_EXTERN_C_START \
    extern "C"                       \
    {
#define SLANG_PRELUDE_EXTERN_C_END }
#else
#define SLANG_PRELUDE_EXTERN_C
#define SLANG_PRELUDE_EXTERN_C_START
#define SLANG_PRELUDE_EXTERN_C_END
#endif

#define SLANG_PRELUDE_EXPORT SLANG_PRELUDE_EXTERN_C SLANG_PRELUDE_SHARED_LIB_EXPORT
#define SLANG_PRELUDE_EXPORT_START SLANG_PRELUDE_EXTERN_C_START SLANG_PRELUDE_SHARED_LIB_EXPORT
#define SLANG_PRELUDE_EXPORT_END SLANG_PRELUDE_EXTERN_C_END

#ifndef INFINITY
// Must overflow for double
#define INFINITY float(1e+300 * 1e+300)
#endif

#ifndef SLANG_INFINITY
#define SLANG_INFINITY INFINITY
#endif

// Detect the compiler type

#ifndef SLANG_COMPILER
#define SLANG_COMPILER

/*
Compiler defines, see http://sourceforge.net/p/predef/wiki/Compilers/
NOTE that SLANG_VC holds the compiler version - not just 1 or 0
*/
#if defined(_MSC_VER)
#if _MSC_VER >= 1900
#define SLANG_VC 14
#elif _MSC_VER >= 1800
#define SLANG_VC 12
#elif _MSC_VER >= 1700
#define SLANG_VC 11
#elif _MSC_VER >= 1600
#define SLANG_VC 10
#elif _MSC_VER >= 1500
#define SLANG_VC 9
#else
#error "unknown version of Visual C++ compiler"
#endif
#elif defined(__clang__)
#define SLANG_CLANG 1
#elif defined(__SNC__)
#define SLANG_SNC 1
#elif defined(__ghs__)
#define SLANG_GHS 1
#elif defined(__GNUC__) /* note: __clang__, __SNC__, or __ghs__ imply __GNUC__ */
#define SLANG_GCC 1
#else
#error "unknown compiler"
#endif
/*
Any compilers not detected by the above logic are now now explicitly zeroed out.
*/
#ifndef SLANG_VC
#define SLANG_VC 0
#endif
#ifndef SLANG_CLANG
#define SLANG_CLANG 0
#endif
#ifndef SLANG_SNC
#define SLANG_SNC 0
#endif
#ifndef SLANG_GHS
#define SLANG_GHS 0
#endif
#ifndef SLANG_GCC
#define SLANG_GCC 0
#endif
#endif /* SLANG_COMPILER */

/*
The following section attempts to detect the target platform being compiled for.

If an application defines `SLANG_PLATFORM` before including this header,
they take responsibility for setting any compiler-dependent macros
used later in the file.

Most applications should not need to touch this section.
*/
#ifndef SLANG_PLATFORM
#define SLANG_PLATFORM
/**
Operating system defines, see http://sourceforge.net/p/predef/wiki/OperatingSystems/
*/
#if defined(WINAPI_FAMILY) && WINAPI_FAMILY == WINAPI_PARTITION_APP
#define SLANG_WINRT 1 /* Windows Runtime, either on Windows RT or Windows 8 */
#elif defined(XBOXONE)
#define SLANG_XBOXONE 1
#elif defined(_WIN64) /* note: XBOXONE implies _WIN64 */
#define SLANG_WIN64 1
#elif defined(_M_PPC)
#define SLANG_X360 1
#elif defined(_WIN32) /* note: _M_PPC implies _WIN32 */
#define SLANG_WIN32 1
#elif defined(__ANDROID__)
#define SLANG_ANDROID 1
#elif defined(__linux__) || defined(__CYGWIN__) /* note: __ANDROID__ implies __linux__ */
#define SLANG_LINUX 1
#elif defined(__APPLE__) && !defined(SLANG_LLVM)
#include "TargetConditionals.h"
#if TARGET_OS_MAC
#define SLANG_OSX 1
#else
#define SLANG_IOS 1
#endif
#elif defined(__APPLE__)
// On `slang-llvm` we can't inclue "TargetConditionals.h" in general, so for now assume its
// OSX.
#define SLANG_OSX 1
#elif defined(__CELLOS_LV2__)
#define SLANG_PS3 1
#elif defined(__ORBIS__)
#define SLANG_PS4 1
#elif defined(__SNC__) && defined(__arm__)
#define SLANG_PSP2 1
#elif defined(__ghs__)
#define SLANG_WIIU 1
#else
#error "unknown target platform"
#endif


/*
Any platforms not detected by the above logic are now now explicitly zeroed out.
*/
#ifndef SLANG_WINRT
#define SLANG_WINRT 0
#endif
#ifndef SLANG_XBOXONE
#define SLANG_XBOXONE 0
#endif
#ifndef SLANG_WIN64
#define SLANG_WIN64 0
#endif
#ifndef SLANG_X360
#define SLANG_X360 0
#endif
#ifndef SLANG_WIN32
#define SLANG_WIN32 0
#endif
#ifndef SLANG_ANDROID
#define SLANG_ANDROID 0
#endif
#ifndef SLANG_LINUX
#define SLANG_LINUX 0
#endif
#ifndef SLANG_IOS
#define SLANG_IOS 0
#endif
#ifndef SLANG_OSX
#define SLANG_OSX 0
#endif
#ifndef SLANG_PS3
#define SLANG_PS3 0
#endif
#ifndef SLANG_PS4
#define SLANG_PS4 0
#endif
#ifndef SLANG_PSP2
#define SLANG_PSP2 0
#endif
#ifndef SLANG_WIIU
#define SLANG_WIIU 0
#endif
#endif /* SLANG_PLATFORM */

/* Shorthands for "families" of compilers/platforms */
#define SLANG_GCC_FAMILY (SLANG_CLANG || SLANG_SNC || SLANG_GHS || SLANG_GCC)
#define SLANG_WINDOWS_FAMILY (SLANG_WINRT || SLANG_WIN32 || SLANG_WIN64)
#define SLANG_MICROSOFT_FAMILY (SLANG_XBOXONE || SLANG_X360 || SLANG_WINDOWS_FAMILY)
#define SLANG_LINUX_FAMILY (SLANG_LINUX || SLANG_ANDROID)
#define SLANG_APPLE_FAMILY (SLANG_IOS || SLANG_OSX) /* equivalent to #if __APPLE__ */
#define SLANG_UNIX_FAMILY \
    (SLANG_LINUX_FAMILY || SLANG_APPLE_FAMILY) /* shortcut for unix/posix platforms */

// GCC Specific
#if SLANG_GCC_FAMILY

#if INTPTR_MAX == INT64_MAX
#define SLANG_64BIT 1
#else
#define SLANG_64BIT 0
#endif

#define SLANG_BREAKPOINT(id) __builtin_trap()

// Use this macro instead of offsetof, because gcc produces warning if offsetof is used on a
// non POD type, even though it produces the correct result
#define SLANG_OFFSET_OF(T, ELEMENT) (size_t(&((T*)1)->ELEMENT) - 1)
#endif // SLANG_GCC_FAMILY

// Microsoft VC specific
#if SLANG_VC

#define SLANG_BREAKPOINT(id) __debugbreak();

#endif // SLANG_VC

// Default impls

#ifndef SLANG_OFFSET_OF
#define SLANG_OFFSET_OF(X, Y) offsetof(X, Y)
#endif

#ifndef SLANG_BREAKPOINT
// Make it crash with a write to 0!
#define SLANG_BREAKPOINT(id) (*((int*)0) = int(id));
#endif

// If slang.h has been included we don't need any of these definitions
#ifndef SLANG_H

/* Macro for declaring if a method is no throw. Should be set before the return parameter. */
#ifndef SLANG_NO_THROW
#if SLANG_WINDOWS_FAMILY && !defined(SLANG_DISABLE_EXCEPTIONS)
#define SLANG_NO_THROW __declspec(nothrow)
#endif
#endif
#ifndef SLANG_NO_THROW
#define SLANG_NO_THROW
#endif

/* The `SLANG_STDCALL` and `SLANG_MCALL` defines are used to set the calling
convention for interface methods.
*/
#ifndef SLANG_STDCALL
#if SLANG_MICROSOFT_FAMILY
#define SLANG_STDCALL __stdcall
#else
#define SLANG_STDCALL
#endif
#endif
#ifndef SLANG_MCALL
#define SLANG_MCALL SLANG_STDCALL
#endif

#ifndef SLANG_FORCE_INLINE
#define SLANG_FORCE_INLINE inline
#endif

// TODO(JS): Should these be in slang-cpp-types.h?
// They are more likely to clash with slang.h

struct SlangUUID
{
    uint32_t data1;
    uint16_t data2;
    uint16_t data3;
    uint8_t data4[8];
};

typedef int32_t SlangResult;

struct ISlangUnknown
{
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    queryInterface(SlangUUID const& uuid, void** outObject) = 0;
    virtual SLANG_NO_THROW uint32_t SLANG_MCALL addRef() = 0;
    virtual SLANG_NO_THROW uint32_t SLANG_MCALL release() = 0;
};

#define SLANG_COM_INTERFACE(a, b, c, d0, d1, d2, d3, d4, d5, d6, d7)             \
public:                                                                          \
    SLANG_FORCE_INLINE static const SlangUUID& getTypeGuid()                     \
    {                                                                            \
        static const SlangUUID guid = {a, b, c, d0, d1, d2, d3, d4, d5, d6, d7}; \
        return guid;                                                             \
    }
#endif // SLANG_H

// Includes

#ifndef SLANG_PRELUDE_SCALAR_INTRINSICS_H
#define SLANG_PRELUDE_SCALAR_INTRINSICS_H

#if !defined(SLANG_LLVM) && SLANG_PROCESSOR_X86_64 && SLANG_VC
//  If we have visual studio and 64 bit processor, we can assume we have popcnt, and can include
//  x86 intrinsics
#include <intrin.h>
#endif

#ifndef SLANG_FORCE_INLINE
#define SLANG_FORCE_INLINE inline
#endif

#ifdef SLANG_PRELUDE_NAMESPACE
namespace SLANG_PRELUDE_NAMESPACE
{
#endif

#ifndef SLANG_PRELUDE_PI
#define SLANG_PRELUDE_PI 3.14159265358979323846
#endif

union Union32
{
    uint32_t u;
    int32_t i;
    float f;
};

union Union64
{
    uint64_t u;
    int64_t i;
    double d;
};

// 32 bit cast conversions
SLANG_FORCE_INLINE int32_t _bitCastFloatToInt(float f)
{
    Union32 u;
    u.f = f;
    return u.i;
}
SLANG_FORCE_INLINE float _bitCastIntToFloat(int32_t i)
{
    Union32 u;
    u.i = i;
    return u.f;
}
SLANG_FORCE_INLINE uint32_t _bitCastFloatToUInt(float f)
{
    Union32 u;
    u.f = f;
    return u.u;
}
SLANG_FORCE_INLINE float _bitCastUIntToFloat(uint32_t ui)
{
    Union32 u;
    u.u = ui;
    return u.f;
}

// ----------------------------- F32 -----------------------------------------

// Helpers
SLANG_FORCE_INLINE float F32_calcSafeRadians(float radians);

#ifdef SLANG_LLVM

SLANG_PRELUDE_EXTERN_C_START

// Unary
float F32_ceil(float f);
float F32_floor(float f);
float F32_round(float f);
float F32_sin(float f);
float F32_cos(float f);
float F32_tan(float f);
float F32_asin(float f);
float F32_acos(float f);
float F32_atan(float f);
float F32_sinh(float f);
float F32_cosh(float f);
float F32_tanh(float f);
float F32_asinh(float f);
float F32_acosh(float f);
float F32_atanh(float f);
float F32_log2(float f);
float F32_log(float f);
float F32_log10(float f);
float F32_exp2(float f);
float F32_exp(float f);
float F32_abs(float f);
float F32_trunc(float f);
float F32_sqrt(float f);

bool F32_isnan(float f);
bool F32_isfinite(float f);
bool F32_isinf(float f);

// Binary
SLANG_FORCE_INLINE float F32_min(float a, float b)
{
    return a < b ? a : b;
}
SLANG_FORCE_INLINE float F32_max(float a, float b)
{
    return a > b ? a : b;
}
float F32_pow(float a, float b);
float F32_fmod(float a, float b);
float F32_remainder(float a, float b);
float F32_atan2(float a, float b);

float F32_frexp(float x, int* e);

float F32_modf(float x, float* ip);

// Ternary
SLANG_FORCE_INLINE float F32_fma(float a, float b, float c)
{
    return a * b + c;
}

SLANG_PRELUDE_EXTERN_C_END

#else

// Unary
SLANG_FORCE_INLINE float F32_ceil(float f)
{
    return ::ceilf(f);
}
SLANG_FORCE_INLINE float F32_floor(float f)
{
    return ::floorf(f);
}
SLANG_FORCE_INLINE float F32_round(float f)
{
    return ::roundf(f);
}
SLANG_FORCE_INLINE float F32_sin(float f)
{
    return ::sinf(f);
}
SLANG_FORCE_INLINE float F32_cos(float f)
{
    return ::cosf(f);
}
SLANG_FORCE_INLINE float F32_tan(float f)
{
    return ::tanf(f);
}
SLANG_FORCE_INLINE float F32_asin(float f)
{
    return ::asinf(f);
}
SLANG_FORCE_INLINE float F32_acos(float f)
{
    return ::acosf(f);
}
SLANG_FORCE_INLINE float F32_atan(float f)
{
    return ::atanf(f);
}
SLANG_FORCE_INLINE float F32_sinh(float f)
{
    return ::sinhf(f);
}
SLANG_FORCE_INLINE float F32_cosh(float f)
{
    return ::coshf(f);
}
SLANG_FORCE_INLINE float F32_tanh(float f)
{
    return ::tanhf(f);
}
SLANG_FORCE_INLINE float F32_asinh(float f)
{
    return ::asinhf(f);
}
SLANG_FORCE_INLINE float F32_acosh(float f)
{
    return ::acoshf(f);
}
SLANG_FORCE_INLINE float F32_atanh(float f)
{
    return ::atanhf(f);
}
SLANG_FORCE_INLINE float F32_log2(float f)
{
    return ::log2f(f);
}
SLANG_FORCE_INLINE float F32_log(float f)
{
    return ::logf(f);
}
SLANG_FORCE_INLINE float F32_log10(float f)
{
    return ::log10f(f);
}
SLANG_FORCE_INLINE float F32_exp2(float f)
{
    return ::exp2f(f);
}
SLANG_FORCE_INLINE float F32_exp(float f)
{
    return ::expf(f);
}
SLANG_FORCE_INLINE float F32_abs(float f)
{
    return ::fabsf(f);
}
SLANG_FORCE_INLINE float F32_trunc(float f)
{
    return ::truncf(f);
}
SLANG_FORCE_INLINE float F32_sqrt(float f)
{
    return ::sqrtf(f);
}

SLANG_FORCE_INLINE bool F32_isnan(float f)
{
    return SLANG_PRELUDE_STD isnan(f);
}
SLANG_FORCE_INLINE bool F32_isfinite(float f)
{
    return SLANG_PRELUDE_STD isfinite(f);
}
SLANG_FORCE_INLINE bool F32_isinf(float f)
{
    return SLANG_PRELUDE_STD isinf(f);
}

// Binary
SLANG_FORCE_INLINE float F32_min(float a, float b)
{
    return ::fminf(a, b);
}
SLANG_FORCE_INLINE float F32_max(float a, float b)
{
    return ::fmaxf(a, b);
}
SLANG_FORCE_INLINE float F32_pow(float a, float b)
{
    return ::powf(a, b);
}
SLANG_FORCE_INLINE float F32_fmod(float a, float b)
{
    return ::fmodf(a, b);
}
SLANG_FORCE_INLINE float F32_remainder(float a, float b)
{
    return ::remainderf(a, b);
}
SLANG_FORCE_INLINE float F32_atan2(float a, float b)
{
    return float(::atan2(a, b));
}

SLANG_FORCE_INLINE float F32_frexp(float x, int* e)
{
    return ::frexpf(x, e);
}

SLANG_FORCE_INLINE float F32_modf(float x, float* ip)
{
    return ::modff(x, ip);
}

// Ternary
SLANG_FORCE_INLINE float F32_fma(float a, float b, float c)
{
    return ::fmaf(a, b, c);
}

#endif

SLANG_FORCE_INLINE float F32_calcSafeRadians(float radians)
{
    // Put 0 to 2pi cycles to cycle around 0 to 1
    float a = radians * (1.0f / float(SLANG_PRELUDE_PI * 2));
    // Get truncated fraction, as value in  0 - 1 range
    a = a - F32_floor(a);
    // Convert back to 0 - 2pi range
    return (a * float(SLANG_PRELUDE_PI * 2));
}

SLANG_FORCE_INLINE float F32_rsqrt(float f)
{
    return 1.0f / F32_sqrt(f);
}
SLANG_FORCE_INLINE int F32_sign(float f)
{
    return (f == 0.0f) ? 0 : ((f < 0.0f) ? -1 : 1);
}
SLANG_FORCE_INLINE float F32_frac(float f)
{
    return f - F32_floor(f);
}

SLANG_FORCE_INLINE uint32_t F32_asuint(float f)
{
    Union32 u;
    u.f = f;
    return u.u;
}
SLANG_FORCE_INLINE int32_t F32_asint(float f)
{
    Union32 u;
    u.f = f;
    return u.i;
}

// ----------------------------- F64 -----------------------------------------

SLANG_FORCE_INLINE double F64_calcSafeRadians(double radians);

#ifdef SLANG_LLVM

SLANG_PRELUDE_EXTERN_C_START

// Unary
double F64_ceil(double f);
double F64_floor(double f);
double F64_round(double f);
double F64_sin(double f);
double F64_cos(double f);
double F64_tan(double f);
double F64_asin(double f);
double F64_acos(double f);
double F64_atan(double f);
double F64_sinh(double f);
double F64_cosh(double f);
double F64_tanh(double f);
double F64_asinh(double f);
double F64_acosh(double f);
double F64_atanh(double f);
double F64_log2(double f);
double F64_log(double f);
double F64_log10(double f);
double F64_exp2(double f);
double F64_exp(double f);
double F64_abs(double f);
double F64_trunc(double f);
double F64_sqrt(double f);

bool F64_isnan(double f);
bool F64_isfinite(double f);
bool F64_isinf(double f);

// Binary
SLANG_FORCE_INLINE double F64_min(double a, double b)
{
    return a < b ? a : b;
}
SLANG_FORCE_INLINE double F64_max(double a, double b)
{
    return a > b ? a : b;
}
double F64_pow(double a, double b);
double F64_fmod(double a, double b);
double F64_remainder(double a, double b);
double F64_atan2(double a, double b);

double F64_frexp(double x, int* e);

double F64_modf(double x, double* ip);

// Ternary
SLANG_FORCE_INLINE double F64_fma(double a, double b, double c)
{
    return a * b + c;
}

SLANG_PRELUDE_EXTERN_C_END

#else // SLANG_LLVM

// Unary
SLANG_FORCE_INLINE double F64_ceil(double f)
{
    return ::ceil(f);
}
SLANG_FORCE_INLINE double F64_floor(double f)
{
    return ::floor(f);
}
SLANG_FORCE_INLINE double F64_round(double f)
{
    return ::round(f);
}
SLANG_FORCE_INLINE double F64_sin(double f)
{
    return ::sin(f);
}
SLANG_FORCE_INLINE double F64_cos(double f)
{
    return ::cos(f);
}
SLANG_FORCE_INLINE double F64_tan(double f)
{
    return ::tan(f);
}
SLANG_FORCE_INLINE double F64_asin(double f)
{
    return ::asin(f);
}
SLANG_FORCE_INLINE double F64_acos(double f)
{
    return ::acos(f);
}
SLANG_FORCE_INLINE double F64_atan(double f)
{
    return ::atan(f);
}
SLANG_FORCE_INLINE double F64_sinh(double f)
{
    return ::sinh(f);
}
SLANG_FORCE_INLINE double F64_cosh(double f)
{
    return ::cosh(f);
}
SLANG_FORCE_INLINE double F64_tanh(double f)
{
    return ::tanh(f);
}
SLANG_FORCE_INLINE double F64_log2(double f)
{
    return ::log2(f);
}
SLANG_FORCE_INLINE double F64_log(double f)
{
    return ::log(f);
}
SLANG_FORCE_INLINE double F64_log10(float f)
{
    return ::log10(f);
}
SLANG_FORCE_INLINE double F64_exp2(double f)
{
    return ::exp2(f);
}
SLANG_FORCE_INLINE double F64_exp(double f)
{
    return ::exp(f);
}
SLANG_FORCE_INLINE double F64_abs(double f)
{
    return ::fabs(f);
}
SLANG_FORCE_INLINE double F64_trunc(double f)
{
    return ::trunc(f);
}
SLANG_FORCE_INLINE double F64_sqrt(double f)
{
    return ::sqrt(f);
}


SLANG_FORCE_INLINE bool F64_isnan(double f)
{
    return SLANG_PRELUDE_STD isnan(f);
}
SLANG_FORCE_INLINE bool F64_isfinite(double f)
{
    return SLANG_PRELUDE_STD isfinite(f);
}
SLANG_FORCE_INLINE bool F64_isinf(double f)
{
    return SLANG_PRELUDE_STD isinf(f);
}

// Binary
SLANG_FORCE_INLINE double F64_min(double a, double b)
{
    return ::fmin(a, b);
}
SLANG_FORCE_INLINE double F64_max(double a, double b)
{
    return ::fmax(a, b);
}
SLANG_FORCE_INLINE double F64_pow(double a, double b)
{
    return ::pow(a, b);
}
SLANG_FORCE_INLINE double F64_fmod(double a, double b)
{
    return ::fmod(a, b);
}
SLANG_FORCE_INLINE double F64_remainder(double a, double b)
{
    return ::remainder(a, b);
}
SLANG_FORCE_INLINE double F64_atan2(double a, double b)
{
    return ::atan2(a, b);
}

SLANG_FORCE_INLINE double F64_frexp(double x, int* e)
{
    return ::frexp(x, e);
}

SLANG_FORCE_INLINE double F64_modf(double x, double* ip)
{
    return ::modf(x, ip);
}

// Ternary
SLANG_FORCE_INLINE double F64_fma(double a, double b, double c)
{
    return ::fma(a, b, c);
}

#endif // SLANG_LLVM

SLANG_FORCE_INLINE double F64_rsqrt(double f)
{
    return 1.0 / F64_sqrt(f);
}
SLANG_FORCE_INLINE int F64_sign(double f)
{
    return (f == 0.0) ? 0 : ((f < 0.0) ? -1 : 1);
}
SLANG_FORCE_INLINE double F64_frac(double f)
{
    return f - F64_floor(f);
}

SLANG_FORCE_INLINE void F64_asuint(double d, uint32_t* low, uint32_t* hi)
{
    Union64 u;
    u.d = d;
    *low = uint32_t(u.u);
    *hi = uint32_t(u.u >> 32);
}

SLANG_FORCE_INLINE void F64_asint(double d, int32_t* low, int32_t* hi)
{
    Union64 u;
    u.d = d;
    *low = int32_t(u.u);
    *hi = int32_t(u.u >> 32);
}

SLANG_FORCE_INLINE double F64_calcSafeRadians(double radians)
{
    // Put 0 to 2pi cycles to cycle around 0 to 1
    double a = radians * (1.0f / (SLANG_PRELUDE_PI * 2));
    // Get truncated fraction, as value in  0 - 1 range
    a = a - F64_floor(a);
    // Convert back to 0 - 2pi range
    return (a * (SLANG_PRELUDE_PI * 2));
}

// ----------------------------- F16 -----------------------------------------

// This impl is based on FloatToHalf that is in Slang codebase
SLANG_FORCE_INLINE uint32_t f32tof16(const float value)
{
    const uint32_t inBits = _bitCastFloatToUInt(value);

    // bits initially set to just the sign bit
    uint32_t bits = (inBits >> 16) & 0x8000;
    // Mantissa can't be used as is, as it holds last bit, for rounding.
    uint32_t m = (inBits >> 12) & 0x07ff;
    uint32_t e = (inBits >> 23) & 0xff;

    if (e < 103)
    {
        // It's zero
        return bits;
    }
    if (e == 0xff)
    {
        // Could be a NAN or INF. Is INF if *input* mantissa is 0.

        // Remove last bit for rounding to make output mantissa.
        m >>= 1;

        // We *assume* float16/float32 signaling bit and remaining bits
        // semantics are the same. (The signalling bit convention is target specific!).
        // Non signal bit's usage within mantissa for a NAN are also target specific.

        // If the m is 0, it could be because the result is INF, but it could also be because all
        // the bits that made NAN were dropped as we have less mantissa bits in f16.

        // To fix for this we make non zero if m is 0 and the input mantissa was not.
        // This will (typically) produce a signalling NAN.
        m += uint32_t(m == 0 && (inBits & 0x007fffffu));

        // Combine for output
        return (bits | 0x7c00u | m);
    }
    if (e > 142)
    {
        // INF.
        return bits | 0x7c00u;
    }
    if (e < 113)
    {
        m |= 0x0800u;
        bits |= (m >> (114 - e)) + ((m >> (113 - e)) & 1);
        return bits;
    }
    bits |= ((e - 112) << 10) | (m >> 1);
    bits += m & 1;
    return bits;
}

static const float g_f16tof32Magic = _bitCastIntToFloat((127 + (127 - 15)) << 23);

SLANG_FORCE_INLINE float f16tof32(const uint32_t value)
{
    const uint32_t sign = (value & 0x8000) << 16;
    uint32_t exponent = (value & 0x7c00) >> 10;
    uint32_t mantissa = (value & 0x03ff);

    if (exponent == 0)
    {
        // If mantissa is 0 we are done, as output is 0.
        // If it's not zero we must have a denormal.
        if (mantissa)
        {
            // We have a denormal so use the magic to do exponent adjust
            return _bitCastIntToFloat(sign | ((value & 0x7fff) << 13)) * g_f16tof32Magic;
        }
    }
    else
    {
        // If the exponent is NAN or INF exponent is 0x1f on input.
        // If that's the case, we just need to set the exponent to 0xff on output
        // and the mantissa can just stay the same. If its 0 it's INF, else it is NAN and we just
        // copy the bits
        //
        // Else we need to correct the exponent in the normalized case.
        exponent = (exponent == 0x1F) ? 0xff : (exponent + (-15 + 127));
    }

    return _bitCastUIntToFloat(sign | (exponent << 23) | (mantissa << 13));
}

#ifndef SLANG_LLVM
#if __cplusplus >= 202302L
#include <stdfloat> // C++23
#else
// Define __STDC_WANT_IEC_60559_TYPES_EXT__ for compilers with reliable _Float16 support:
// - Clang 15+
// - GCC 12+
#if (defined(__clang__) && __clang_major__ >= 15) || (defined(__GNUC__) && __GNUC__ >= 12)
#ifndef __STDC_WANT_IEC_60559_TYPES_EXT__
#define __STDC_WANT_IEC_60559_TYPES_EXT__
#endif
#include <float.h>
#endif // __STDC_WANT_IEC_60559_TYPES_EXT__
#endif // (defined(__clang__) && __clang_major__ >= 15) || (defined(__GNUC__) && __GNUC__ >= 12)
#endif // C++23

#ifdef FLT16_MIN
typedef _Float16 half;
#elif __STDCPP_FLOAT16_T__ == 1
typedef std::float16_t half;
#else
uint32_t f32tof16(const float value);
float f16tof32(const uint32_t value);
struct half
{
    uint16_t data;

    half() = default;
    explicit half(float f) { store(f); }

    SLANG_FORCE_INLINE void store(float f) { data = f32tof16(f); }
    SLANG_FORCE_INLINE float load() const { return f16tof32(data); }

    half operator+(half other) const { return half(load() + other.load()); }
    half operator-(half other) const { return half(load() - other.load()); }
    half operator*(half other) const { return half(load() * other.load()); }
    half operator/(half other) const { return half(load() / other.load()); }
    half& operator+=(half other)
    {
        store(load() + other.load());
        return *this;
    }
    half& operator-=(half other)
    {
        store(load() - other.load());
        return *this;
    }
    half& operator*=(half other)
    {
        store(load() * other.load());
        return *this;
    }
    half& operator/=(half other)
    {
        store(load() / other.load());
        return *this;
    }

    bool operator<(half other) const { return load() < other.load(); }
    bool operator>(half other) const { return load() > other.load(); }
    bool operator<=(half other) const { return load() <= other.load(); }
    bool operator>=(half other) const { return load() >= other.load(); }
    bool operator==(half other) const { return load() == other.load(); }
    bool operator!=(half other) const { return load() != other.load(); }

    explicit operator float() const { return load(); }
};
#endif

half U16_ashalf(uint16_t x);

union Union16
{
    uint16_t u;
    int16_t i;
    half h;
};

SLANG_FORCE_INLINE uint16_t F16_asuint(half h)
{
    Union16 u;
    u.h = h;
    return u.u;
}

SLANG_FORCE_INLINE int16_t F16_asint(half h)
{
    Union16 u;
    u.h = h;
    return u.i;
}

SLANG_FORCE_INLINE half F16_ceil(half f)
{
    return half(F32_ceil(float(f)));
}

SLANG_FORCE_INLINE half F16_floor(half f)
{
    return half(F32_floor(float(f)));
}

SLANG_FORCE_INLINE half F16_round(half f)
{
    return half(F32_round(float(f)));
}

SLANG_FORCE_INLINE half F16_sin(half f)
{
    return half(F32_sin(float(f)));
}

SLANG_FORCE_INLINE half F16_cos(half f)
{
    return half(F32_cos(float(f)));
}

SLANG_FORCE_INLINE half F16_tan(half f)
{
    return half(F32_tan(float(f)));
}

SLANG_FORCE_INLINE half F16_asin(half f)
{
    return half(F32_asin(float(f)));
}

SLANG_FORCE_INLINE half F16_acos(half f)
{
    return half(F32_acos(float(f)));
}

SLANG_FORCE_INLINE half F16_atan(half f)
{
    return half(F32_atan(float(f)));
}

SLANG_FORCE_INLINE half F16_sinh(half f)
{
    return half(F32_sinh(float(f)));
}

SLANG_FORCE_INLINE half F16_cosh(half f)
{
    return half(F32_cosh(float(f)));
}

SLANG_FORCE_INLINE half F16_tanh(half f)
{
    return half(F32_tanh(float(f)));
}

SLANG_FORCE_INLINE half F16_asinh(half f)
{
    return half(F32_asinh(float(f)));
}

SLANG_FORCE_INLINE half F16_acosh(half f)
{
    return half(F32_acosh(float(f)));
}

SLANG_FORCE_INLINE half F16_atanh(half f)
{
    return half(F32_atanh(float(f)));
}

SLANG_FORCE_INLINE half F16_log2(half f)
{
    return half(F32_log2(float(f)));
}

SLANG_FORCE_INLINE half F16_log(half f)
{
    return half(F32_log(float(f)));
}

SLANG_FORCE_INLINE half F16_log10(half f)
{
    return half(F32_log10(float(f)));
}

SLANG_FORCE_INLINE half F16_exp2(half f)
{
    return half(F32_exp2(float(f)));
}

SLANG_FORCE_INLINE half F16_exp(half f)
{
    return half(F32_exp(float(f)));
}

SLANG_FORCE_INLINE half F16_abs(half f)
{
    return U16_ashalf(F16_asuint(f) & 0x7FFF);
}

SLANG_FORCE_INLINE half F16_trunc(half f)
{
    return half(F32_trunc(float(f)));
}

SLANG_FORCE_INLINE half F16_sqrt(half f)
{
    return half(F32_sqrt(float(f)));
}

SLANG_FORCE_INLINE bool F16_isnan(half f)
{
    uint16_t u = F16_asuint(f);
    return (u & 0x7C00) == 0x7C00 && (u & 0x3FF) != 0;
}

SLANG_FORCE_INLINE bool F16_isfinite(half f)
{
    uint16_t u = F16_asuint(f);
    return (u & 0x7C00) != 0x7C00;
}

SLANG_FORCE_INLINE bool F16_isinf(half f)
{
    uint16_t u = F16_asuint(f);
    return (u & 0x7C00) == 0x7C00 && (u & 0x3FF) == 0;
}

SLANG_FORCE_INLINE half F16_min(half a, half b)
{
    if (F16_isnan(a))
        return b;
    if (F16_isnan(b))
        return a;
    return a < b ? a : b;
}

SLANG_FORCE_INLINE half F16_max(half a, half b)
{
    if (F16_isnan(a))
        return b;
    if (F16_isnan(b))
        return a;
    return a > b ? a : b;
}

SLANG_FORCE_INLINE half F16_pow(half a, half b)
{
    return half(F32_pow(float(a), float(b)));
}

SLANG_FORCE_INLINE half F16_fmod(half a, half b)
{
    return half(F32_fmod(float(a), float(b)));
}

SLANG_FORCE_INLINE half F16_remainder(half a, half b)
{
    return half(F32_remainder(float(a), float(b)));
}

SLANG_FORCE_INLINE half F16_atan2(half a, half b)
{
    return half(F32_atan2(float(a), float(b)));
}

SLANG_FORCE_INLINE half F16_frexp(half x, int* e)
{
    return half(F32_frexp(float(x), e));
}

SLANG_FORCE_INLINE half F16_modf(half x, half* ip)
{
    float ipf;
    float res = F32_modf(float(x), &ipf);
    *ip = half(ipf);
    return half(res);
}

SLANG_FORCE_INLINE half F16_fma(half a, half b, half c)
{
    return half(F32_fma(float(a), float(b), float(c)));
}

SLANG_FORCE_INLINE half F16_calcSafeRadians(half radians)
{
    // Put 0 to 2pi cycles to cycle around 0 to 1
    float a = float(radians) * (1.0f / float(SLANG_PRELUDE_PI * 2));
    // Get truncated fraction, as value in  0 - 1 range
    a = a - F32_floor(a);
    // Convert back to 0 - 2pi range
    return half(a * float(SLANG_PRELUDE_PI * 2));
}

SLANG_FORCE_INLINE half F16_rsqrt(half f)
{
    return half(1.0f / F32_sqrt(float(f)));
}

SLANG_FORCE_INLINE int F16_sign(half f)
{
    uint16_t u = F16_asuint(f);
    if ((u & 0x7FFF) == 0)
        return 0;
    return (u & 0x8000) != 0 ? -1 : 1;
}

SLANG_FORCE_INLINE half F16_frac(half h)
{
    float f = float(h);
    return half(f - F32_floor(f));
}

// ----------------------------- U16 -----------------------------------------
SLANG_FORCE_INLINE uint32_t U16_countbits(uint16_t v)
{
#if SLANG_GCC_FAMILY && !defined(SLANG_LLVM)
    return __builtin_popcount(uint32_t(v));
#elif SLANG_PROCESSOR_X86_64 && SLANG_VC
    return __popcnt16(v);
#else
    uint32_t c = 0;
    while (v)
    {
        c++;
        v &= v - 1;
    }
    return c;
#endif
}

SLANG_FORCE_INLINE half U16_ashalf(uint16_t x)
{
    Union16 u;
    u.u = x;
    return u.h;
}

// ----------------------------- I16 -----------------------------------------
SLANG_FORCE_INLINE uint32_t I16_countbits(int16_t v)
{
    return U16_countbits(uint16_t(v));
}

// ----------------------------- U8 -----------------------------------------
SLANG_FORCE_INLINE uint32_t U8_countbits(uint8_t v)
{
    // No native 8bit __popcnt yet, just cast and use 16bit variant
    return U16_countbits(uint16_t(v));
}

// ----------------------------- I8 -----------------------------------------
SLANG_FORCE_INLINE uint32_t I8_countbits(int16_t v)
{
    return U8_countbits(uint8_t(v));
}

// ----------------------------- U32 -----------------------------------------

SLANG_FORCE_INLINE uint32_t U32_abs(uint32_t f)
{
    return f;
}

SLANG_FORCE_INLINE uint32_t U32_min(uint32_t a, uint32_t b)
{
    return a < b ? a : b;
}
SLANG_FORCE_INLINE uint32_t U32_max(uint32_t a, uint32_t b)
{
    return a > b ? a : b;
}

SLANG_FORCE_INLINE float U32_asfloat(uint32_t x)
{
    Union32 u;
    u.u = x;
    return u.f;
}
SLANG_FORCE_INLINE uint32_t U32_asint(int32_t x)
{
    return uint32_t(x);
}

SLANG_FORCE_INLINE double U32_asdouble(uint32_t low, uint32_t hi)
{
    Union64 u;
    u.u = (uint64_t(hi) << 32) | low;
    return u.d;
}

SLANG_FORCE_INLINE uint32_t U32_countbits(uint32_t v)
{
#if SLANG_GCC_FAMILY && !defined(SLANG_LLVM)
    return __builtin_popcount(v);
#elif SLANG_PROCESSOR_X86_64 && SLANG_VC
    return __popcnt(v);
#else
    uint32_t c = 0;
    while (v)
    {
        c++;
        v &= v - 1;
    }
    return c;
#endif
}

SLANG_FORCE_INLINE uint32_t U32_firstbitlow(uint32_t v)
{
    if (v == 0)
        return ~0u;

#if SLANG_GCC_FAMILY && !defined(SLANG_LLVM)
    // __builtin_ctz returns number of trailing zeros, which is the 0-based index of first set bit
    return __builtin_ctz(v);
#elif SLANG_PROCESSOR_X86_64 && SLANG_VC
    // _BitScanForward returns 1 on success, 0 on failure, and sets index
    unsigned long index;
    return _BitScanForward(&index, v) ? index : ~0u;
#else
    // Generic implementation - find first set bit
    uint32_t result = 0;
    while (result < 32 && !(v & (1u << result)))
        result++;
    return result;
#endif
}

SLANG_FORCE_INLINE uint32_t U32_firstbithigh(uint32_t v)
{
    if (v == 0)
        return ~0u;
#if SLANG_GCC_FAMILY && !defined(SLANG_LLVM)
    // __builtin_clz returns number of leading zeros
    // firstbithigh should return 0-based bit position of MSB
    return 31 - __builtin_clz(v);
#elif SLANG_PROCESSOR_X86_64 && SLANG_VC
    // _BitScanReverse returns 1 on success, 0 on failure, and sets index
    unsigned long index;
    return _BitScanReverse(&index, v) ? index : ~0u;
#else
    // Generic implementation - find highest set bit
    int result = 31;
    while (result >= 0 && !(v & (1u << result)))
        result--;
    return result;
#endif
}

SLANG_FORCE_INLINE uint32_t U32_reversebits(uint32_t v)
{
    v = ((v >> 1) & 0x55555555u) | ((v & 0x55555555u) << 1);
    v = ((v >> 2) & 0x33333333u) | ((v & 0x33333333u) << 2);
    v = ((v >> 4) & 0x0F0F0F0Fu) | ((v & 0x0F0F0F0Fu) << 4);
    v = ((v >> 8) & 0x00FF00FFu) | ((v & 0x00FF00FFu) << 8);
    v = (v >> 16) | (v << 16);
    return v;
}

// ----------------------------- I32 -----------------------------------------

SLANG_FORCE_INLINE int32_t I32_abs(int32_t f)
{
    return (f < 0) ? -f : f;
}

SLANG_FORCE_INLINE int32_t I32_min(int32_t a, int32_t b)
{
    return a < b ? a : b;
}
SLANG_FORCE_INLINE int32_t I32_max(int32_t a, int32_t b)
{
    return a > b ? a : b;
}

SLANG_FORCE_INLINE float I32_asfloat(int32_t x)
{
    Union32 u;
    u.i = x;
    return u.f;
}
SLANG_FORCE_INLINE uint32_t I32_asuint(int32_t x)
{
    return uint32_t(x);
}
SLANG_FORCE_INLINE double I32_asdouble(int32_t low, int32_t hi)
{
    Union64 u;
    u.u = (uint64_t(hi) << 32) | uint32_t(low);
    return u.d;
}

SLANG_FORCE_INLINE uint32_t I32_countbits(int32_t v)
{
    return U32_countbits(uint32_t(v));
}

SLANG_FORCE_INLINE uint32_t I32_firstbitlow(int32_t v)
{
    return U32_firstbitlow(uint32_t(v));
}

SLANG_FORCE_INLINE uint32_t I32_firstbithigh(int32_t v)
{
    if (v < 0)
        v = ~v;
    return U32_firstbithigh(uint32_t(v));
}

SLANG_FORCE_INLINE int32_t I32_reversebits(int32_t v)
{
    return U32_reversebits(int32_t(v));
}

// ----------------------------- U64 -----------------------------------------

SLANG_FORCE_INLINE uint64_t U64_abs(uint64_t f)
{
    return f;
}

SLANG_FORCE_INLINE uint64_t U64_min(uint64_t a, uint64_t b)
{
    return a < b ? a : b;
}
SLANG_FORCE_INLINE uint64_t U64_max(uint64_t a, uint64_t b)
{
    return a > b ? a : b;
}

SLANG_FORCE_INLINE uint32_t U64_countbits(uint64_t v)
{
#if SLANG_GCC_FAMILY && !defined(SLANG_LLVM)
    return uint32_t(__builtin_popcountll(v));
#elif SLANG_PROCESSOR_X86_64 && SLANG_VC
    return uint32_t(__popcnt64(v));
#else
    uint32_t c = 0;
    while (v)
    {
        c++;
        v &= v - 1;
    }
    return c;
#endif
}

SLANG_FORCE_INLINE uint32_t U64_firstbitlow(uint64_t v)
{
    if (v == 0)
        return ~uint32_t(0);

#if SLANG_GCC_FAMILY && !defined(SLANG_LLVM)
    // __builtin_ctz returns number of trailing zeros, which is the 0-based index of first set bit
    return __builtin_ctz(v);
#elif SLANG_PROCESSOR_X86_64 && SLANG_VC
    // _BitScanForward returns 1 on success, 0 on failure, and sets index
    unsigned long index;
    return _BitScanForward64(&index, v) ? index : ~uint32_t(0);
#else
    // Generic implementation - find first set bit
    uint32_t result = 0;
    while (result < 64 && !(v & (uint64_t(1) << result)))
        result++;
    return result;
#endif
}

SLANG_FORCE_INLINE uint32_t U64_firstbithigh(uint64_t v)
{
    if (v == 0)
        return ~uint32_t(0);

#if SLANG_GCC_FAMILY && !defined(SLANG_LLVM)
    // __builtin_clz returns number of leading zeros
    // firstbithigh should return 0-based bit position of MSB
    return 63 - __builtin_clz(v);
#elif SLANG_PROCESSOR_X86_64 && SLANG_VC
    // _BitScanReverse returns 1 on success, 0 on failure, and sets index
    unsigned long index;
    return _BitScanReverse64(&index, v) ? index : ~uint32_t(0);
#else
    // Generic implementation - find highest set bit
    int result = 63;
    while (result >= 0 && !(v & (uint64_t(1) << result)))
        result--;
    return result;
#endif
}

SLANG_FORCE_INLINE uint64_t U64_reversebits(uint64_t v)
{
    v = ((v >> 1) & 0x5555555555555555ull) | ((v & 0x5555555555555555ull) << 1);
    v = ((v >> 2) & 0x3333333333333333ull) | ((v & 0x3333333333333333ull) << 2);
    v = ((v >> 4) & 0x0F0F0F0F0F0F0F0Full) | ((v & 0x0F0F0F0F0F0F0F0Full) << 4);
    v = ((v >> 8) & 0x00FF00FF00FF00FFull) | ((v & 0x00FF00FF00FF00FFull) << 8);
    v = ((v >> 16) & 0x0000FFFF0000FFFFull) | ((v & 0x0000FFFF0000FFFFull) << 16);
    v = (v >> 32) | (v << 32);
    return v;
}

// ----------------------------- I64 -----------------------------------------

SLANG_FORCE_INLINE int64_t I64_abs(int64_t f)
{
    return (f < 0) ? -f : f;
}

SLANG_FORCE_INLINE int64_t I64_min(int64_t a, int64_t b)
{
    return a < b ? a : b;
}
SLANG_FORCE_INLINE int64_t I64_max(int64_t a, int64_t b)
{
    return a > b ? a : b;
}

SLANG_FORCE_INLINE uint32_t I64_countbits(int64_t v)
{
    return U64_countbits(uint64_t(v));
}

SLANG_FORCE_INLINE uint32_t I64_firstbitlow(int64_t v)
{
    return U64_firstbitlow(uint64_t(v));
}

SLANG_FORCE_INLINE uint32_t I64_firstbithigh(int64_t v)
{
    if (v < 0)
        v = ~v;
    return U64_firstbithigh(uint64_t(v));
}

SLANG_FORCE_INLINE int64_t I64_reversebits(int64_t v)
{
    return int64_t(U64_reversebits(uint64_t(v)));
}

// ----------------------------- UPTR -----------------------------------------

SLANG_FORCE_INLINE uintptr_t UPTR_abs(uintptr_t f)
{
    return f;
}

SLANG_FORCE_INLINE uintptr_t UPTR_min(uintptr_t a, uintptr_t b)
{
    return a < b ? a : b;
}

SLANG_FORCE_INLINE uintptr_t UPTR_max(uintptr_t a, uintptr_t b)
{
    return a > b ? a : b;
}

// ----------------------------- IPTR -----------------------------------------

SLANG_FORCE_INLINE intptr_t IPTR_abs(intptr_t f)
{
    return (f < 0) ? -f : f;
}

SLANG_FORCE_INLINE intptr_t IPTR_min(intptr_t a, intptr_t b)
{
    return a < b ? a : b;
}

SLANG_FORCE_INLINE intptr_t IPTR_max(intptr_t a, intptr_t b)
{
    return a > b ? a : b;
}

// ----------------------------- Interlocked ---------------------------------

#if SLANG_LLVM

#else // SLANG_LLVM

#ifdef _WIN32
#include <intrin.h>
#endif

SLANG_FORCE_INLINE void InterlockedAdd(uint32_t* dest, uint32_t value, uint32_t* oldValue)
{
#ifdef _WIN32
    *oldValue = _InterlockedExchangeAdd((long*)dest, (long)value);
#else
    *oldValue = __sync_fetch_and_add(dest, value);
#endif
}

#endif // SLANG_LLVM


// ----------------------- fmod --------------------------
SLANG_FORCE_INLINE float _slang_fmod(float x, float y)
{
    return F32_fmod(x, y);
}
SLANG_FORCE_INLINE double _slang_fmod(double x, double y)
{
    return F64_fmod(x, y);
}

#ifdef SLANG_PRELUDE_NAMESPACE
}
#endif

#endif

#ifndef SLANG_PRELUDE_CPP_TYPES_H
#define SLANG_PRELUDE_CPP_TYPES_H

#ifdef SLANG_PRELUDE_NAMESPACE
namespace SLANG_PRELUDE_NAMESPACE
{
#endif

#ifndef SLANG_FORCE_INLINE
#define SLANG_FORCE_INLINE inline
#endif

#ifndef SLANG_PRELUDE_CPP_TYPES_CORE_H
#define SLANG_PRELUDE_CPP_TYPES_CORE_H

#ifndef SLANG_PRELUDE_ASSERT
#ifdef SLANG_PRELUDE_ENABLE_ASSERT
#define SLANG_PRELUDE_ASSERT(VALUE) assert(VALUE)
#else
#define SLANG_PRELUDE_ASSERT(VALUE)
#endif
#endif

// Since we are using unsigned arithmatic care is need in this comparison.
// It is *assumed* that sizeInBytes >= elemSize. Which means (sizeInBytes >= elemSize) >= 0
// Which means only a single test is needed

// Asserts for bounds checking.
// It is assumed index/count are unsigned types.
#define SLANG_BOUND_ASSERT(index, count) SLANG_PRELUDE_ASSERT(index < count);
#define SLANG_BOUND_ASSERT_BYTE_ADDRESS(index, elemSize, sizeInBytes) \
    SLANG_PRELUDE_ASSERT(index <= (sizeInBytes - elemSize) && (index & 3) == 0);

// Macros to zero index if an access is out of range
#define SLANG_BOUND_ZERO_INDEX(index, count) index = (index < count) ? index : 0;
#define SLANG_BOUND_ZERO_INDEX_BYTE_ADDRESS(index, elemSize, sizeInBytes) \
    index = (index <= (sizeInBytes - elemSize)) ? index : 0;

// The 'FIX' macro define how the index is fixed. The default is to do nothing. If
// SLANG_ENABLE_BOUND_ZERO_INDEX the fix macro will zero the index, if out of range
#ifdef SLANG_ENABLE_BOUND_ZERO_INDEX
#define SLANG_BOUND_FIX(index, count) SLANG_BOUND_ZERO_INDEX(index, count)
#define SLANG_BOUND_FIX_BYTE_ADDRESS(index, elemSize, sizeInBytes) \
    SLANG_BOUND_ZERO_INDEX_BYTE_ADDRESS(index, elemSize, sizeInBytes)
#define SLANG_BOUND_FIX_FIXED_ARRAY(index, count) SLANG_BOUND_ZERO_INDEX(index, count)
#else
#define SLANG_BOUND_FIX(index, count)
#define SLANG_BOUND_FIX_BYTE_ADDRESS(index, elemSize, sizeInBytes)
#define SLANG_BOUND_FIX_FIXED_ARRAY(index, count)
#endif

#ifndef SLANG_BOUND_CHECK
#define SLANG_BOUND_CHECK(index, count) \
    SLANG_BOUND_ASSERT(index, count) SLANG_BOUND_FIX(index, count)
#endif

#ifndef SLANG_BOUND_CHECK_BYTE_ADDRESS
#define SLANG_BOUND_CHECK_BYTE_ADDRESS(index, elemSize, sizeInBytes) \
    SLANG_BOUND_ASSERT_BYTE_ADDRESS(index, elemSize, sizeInBytes)    \
    SLANG_BOUND_FIX_BYTE_ADDRESS(index, elemSize, sizeInBytes)
#endif

#ifndef SLANG_BOUND_CHECK_FIXED_ARRAY
#define SLANG_BOUND_CHECK_FIXED_ARRAY(index, count) \
    SLANG_BOUND_ASSERT(index, count) SLANG_BOUND_FIX_FIXED_ARRAY(index, count)
#endif

struct TypeInfo
{
    size_t typeSize;
};

template<typename T, size_t SIZE>
struct FixedArray
{
    const T& operator[](size_t index) const
    {
        SLANG_BOUND_CHECK_FIXED_ARRAY(index, SIZE);
        return m_data[index];
    }
    T& operator[](size_t index)
    {
        SLANG_BOUND_CHECK_FIXED_ARRAY(index, SIZE);
        return m_data[index];
    }

    T m_data[SIZE];
};

// An array that has no specified size, becomes a 'Array'. This stores the size so it can
// potentially do bounds checking.
template<typename T>
struct Array
{
    const T& operator[](size_t index) const
    {
        SLANG_BOUND_CHECK(index, count);
        return data[index];
    }
    T& operator[](size_t index)
    {
        SLANG_BOUND_CHECK(index, count);
        return data[index];
    }

    T* data;
    size_t count;
};

/* Constant buffers become a pointer to the contained type, so ConstantBuffer<T> becomes T* in C++
 * code.
 */

template<typename T, int COUNT>
struct Vector;

template<typename T>
struct Vector<T, 1>
{
    T x;
    const T& operator[](size_t /*index*/) const { return x; }
    T& operator[](size_t /*index*/) { return x; }
    operator T() const { return x; }
    Vector() = default;
    Vector(T scalar) { x = scalar; }
    template<typename U>
    Vector(Vector<U, 1> other)
    {
        x = (T)other.x;
    }
    template<typename U, int otherSize>
    Vector(Vector<U, otherSize> other)
    {
        int minSize = 1;
        if (otherSize < minSize)
            minSize = otherSize;
        for (int i = 0; i < minSize; i++)
            (*this)[i] = (T)other[i];
    }
};

template<typename T>
struct Vector<T, 2>
{
    T x, y;
    const T& operator[](size_t index) const { return index == 0 ? x : y; }
    T& operator[](size_t index) { return index == 0 ? x : y; }
    Vector() = default;
    Vector(T scalar) { x = y = scalar; }
    Vector(T _x, T _y)
    {
        x = _x;
        y = _y;
    }
    template<typename U>
    Vector(Vector<U, 2> other)
    {
        x = (T)other.x;
        y = (T)other.y;
    }
    template<typename U, int otherSize>
    Vector(Vector<U, otherSize> other)
    {
        int minSize = 2;
        if (otherSize < minSize)
            minSize = otherSize;
        for (int i = 0; i < minSize; i++)
            (*this)[i] = (T)other[i];
    }
};

template<typename T>
struct Vector<T, 3>
{
    T x, y, z;
    const T& operator[](size_t index) const { return *((T*)(this) + index); }
    T& operator[](size_t index) { return *((T*)(this) + index); }

    Vector() = default;
    Vector(T scalar) { x = y = z = scalar; }
    Vector(T _x, T _y, T _z)
    {
        x = _x;
        y = _y;
        z = _z;
    }
    template<typename U>
    Vector(Vector<U, 3> other)
    {
        x = (T)other.x;
        y = (T)other.y;
        z = (T)other.z;
    }
    template<typename U, int otherSize>
    Vector(Vector<U, otherSize> other)
    {
        int minSize = 3;
        if (otherSize < minSize)
            minSize = otherSize;
        for (int i = 0; i < minSize; i++)
            (*this)[i] = (T)other[i];
    }
};

template<typename T>
struct Vector<T, 4>
{
    T x, y, z, w;

    const T& operator[](size_t index) const { return *((T*)(this) + index); }
    T& operator[](size_t index) { return *((T*)(this) + index); }
    Vector() = default;
    Vector(T scalar) { x = y = z = w = scalar; }
    Vector(T _x, T _y, T _z, T _w)
    {
        x = _x;
        y = _y;
        z = _z;
        w = _w;
    }
    template<typename U, int otherSize>
    Vector(Vector<U, otherSize> other)
    {
        int minSize = 4;
        if (otherSize < minSize)
            minSize = otherSize;
        for (int i = 0; i < minSize; i++)
            (*this)[i] = (T)other[i];
    }
};

template<typename T, int N>
SLANG_FORCE_INLINE Vector<T, N> _slang_select(
    Vector<bool, N> condition,
    Vector<T, N> v0,
    Vector<T, N> v1)
{
    Vector<T, N> result;
    for (int i = 0; i < N; i++)
    {
        result[i] = condition[i] ? v0[i] : v1[i];
    }
    return result;
}

template<typename T>
SLANG_FORCE_INLINE T _slang_select(bool condition, T v0, T v1)
{
    return condition ? v0 : v1;
}

template<typename T, int N>
SLANG_FORCE_INLINE T _slang_vector_get_element(Vector<T, N> x, int index)
{
    return x[index];
}

template<typename T, int N>
SLANG_FORCE_INLINE const T* _slang_vector_get_element_ptr(const Vector<T, N>* x, int index)
{
    return &((*const_cast<Vector<T, N>*>(x))[index]);
}

template<typename T, int N>
SLANG_FORCE_INLINE T* _slang_vector_get_element_ptr(Vector<T, N>* x, int index)
{
    return &((*x)[index]);
}

template<typename T, int n, typename OtherT, int m>
SLANG_FORCE_INLINE Vector<T, n> _slang_vector_reshape(const Vector<OtherT, m> other)
{
    Vector<T, n> result;
    for (int i = 0; i < n; i++)
    {
        OtherT otherElement = T(0);
        if (i < m)
            otherElement = _slang_vector_get_element(other, i);
        *_slang_vector_get_element_ptr(&result, i) = (T)otherElement;
    }
    return result;
}

typedef uint32_t uint;

#define SLANG_VECTOR_BINARY_OP(T, op)            \
    template<int n>                              \
    SLANG_FORCE_INLINE Vector<T, n> operator op( \
        const Vector<T, n>& thisVal,             \
        const Vector<T, n>& other)               \
    {                                            \
        Vector<T, n> result;                     \
        for (int i = 0; i < n; i++)              \
            result[i] = thisVal[i] op other[i];  \
        return result;                           \
    }
#define SLANG_VECTOR_BINARY_COMPARE_OP(T, op)       \
    template<int n>                                 \
    SLANG_FORCE_INLINE Vector<bool, n> operator op( \
        const Vector<T, n>& thisVal,                \
        const Vector<T, n>& other)                  \
    {                                               \
        Vector<bool, n> result;                     \
        for (int i = 0; i < n; i++)                 \
            result[i] = thisVal[i] op other[i];     \
        return result;                              \
    }

#define SLANG_VECTOR_UNARY_OP(T, op)                                         \
    template<int n>                                                          \
    SLANG_FORCE_INLINE Vector<T, n> operator op(const Vector<T, n>& thisVal) \
    {                                                                        \
        Vector<T, n> result;                                                 \
        for (int i = 0; i < n; i++)                                          \
            result[i] = op thisVal[i];                                       \
        return result;                                                       \
    }
#define SLANG_INT_VECTOR_OPS(T)           \
    SLANG_VECTOR_BINARY_OP(T, +)          \
    SLANG_VECTOR_BINARY_OP(T, -)          \
    SLANG_VECTOR_BINARY_OP(T, *)          \
    SLANG_VECTOR_BINARY_OP(T, /)          \
    SLANG_VECTOR_BINARY_OP(T, &)          \
    SLANG_VECTOR_BINARY_OP(T, |)          \
    SLANG_VECTOR_BINARY_OP(T, &&)         \
    SLANG_VECTOR_BINARY_OP(T, ||)         \
    SLANG_VECTOR_BINARY_OP(T, ^)          \
    SLANG_VECTOR_BINARY_OP(T, %)          \
    SLANG_VECTOR_BINARY_OP(T, >>)         \
    SLANG_VECTOR_BINARY_OP(T, <<)         \
    SLANG_VECTOR_BINARY_COMPARE_OP(T, >)  \
    SLANG_VECTOR_BINARY_COMPARE_OP(T, <)  \
    SLANG_VECTOR_BINARY_COMPARE_OP(T, >=) \
    SLANG_VECTOR_BINARY_COMPARE_OP(T, <=) \
    SLANG_VECTOR_BINARY_COMPARE_OP(T, ==) \
    SLANG_VECTOR_BINARY_COMPARE_OP(T, !=) \
    SLANG_VECTOR_UNARY_OP(T, !)           \
    SLANG_VECTOR_UNARY_OP(T, ~)
#define SLANG_FLOAT_VECTOR_OPS(T)         \
    SLANG_VECTOR_BINARY_OP(T, +)          \
    SLANG_VECTOR_BINARY_OP(T, -)          \
    SLANG_VECTOR_BINARY_OP(T, *)          \
    SLANG_VECTOR_BINARY_OP(T, /)          \
    SLANG_VECTOR_UNARY_OP(T, -)           \
    SLANG_VECTOR_BINARY_COMPARE_OP(T, >)  \
    SLANG_VECTOR_BINARY_COMPARE_OP(T, <)  \
    SLANG_VECTOR_BINARY_COMPARE_OP(T, >=) \
    SLANG_VECTOR_BINARY_COMPARE_OP(T, <=) \
    SLANG_VECTOR_BINARY_COMPARE_OP(T, ==) \
    SLANG_VECTOR_BINARY_COMPARE_OP(T, !=)

SLANG_INT_VECTOR_OPS(bool)
SLANG_INT_VECTOR_OPS(int)
SLANG_INT_VECTOR_OPS(int8_t)
SLANG_INT_VECTOR_OPS(int16_t)
SLANG_INT_VECTOR_OPS(int64_t)
SLANG_INT_VECTOR_OPS(uint)
SLANG_INT_VECTOR_OPS(uint8_t)
SLANG_INT_VECTOR_OPS(uint16_t)
SLANG_INT_VECTOR_OPS(uint64_t)
#if SLANG_INTPTR_TYPE_IS_DISTINCT
SLANG_INT_VECTOR_OPS(intptr_t)
SLANG_INT_VECTOR_OPS(uintptr_t)
#endif

SLANG_FLOAT_VECTOR_OPS(float)
SLANG_FLOAT_VECTOR_OPS(double)

#define SLANG_VECTOR_INT_NEG_OP(T)                      \
    template<int N>                                     \
    Vector<T, N> operator-(const Vector<T, N>& thisVal) \
    {                                                   \
        Vector<T, N> result;                            \
        for (int i = 0; i < N; i++)                     \
            result[i] = 0 - thisVal[i];                 \
        return result;                                  \
    }
SLANG_VECTOR_INT_NEG_OP(int)
SLANG_VECTOR_INT_NEG_OP(int8_t)
SLANG_VECTOR_INT_NEG_OP(int16_t)
SLANG_VECTOR_INT_NEG_OP(int64_t)
SLANG_VECTOR_INT_NEG_OP(uint)
SLANG_VECTOR_INT_NEG_OP(uint8_t)
SLANG_VECTOR_INT_NEG_OP(uint16_t)
SLANG_VECTOR_INT_NEG_OP(uint64_t)
#if SLANG_INTPTR_TYPE_IS_DISTINCT
SLANG_VECTOR_INT_NEG_OP(intptr_t)
SLANG_VECTOR_INT_NEG_OP(uintptr_t)
#endif

#define SLANG_FLOAT_VECTOR_MOD(T)                                               \
    template<int N>                                                             \
    Vector<T, N> operator%(const Vector<T, N>& left, const Vector<T, N>& right) \
    {                                                                           \
        Vector<T, N> result;                                                    \
        for (int i = 0; i < N; i++)                                             \
            result[i] = _slang_fmod(left[i], right[i]);                         \
        return result;                                                          \
    }

SLANG_FLOAT_VECTOR_MOD(float)
SLANG_FLOAT_VECTOR_MOD(double)
#undef SLANG_FLOAT_VECTOR_MOD
#undef SLANG_VECTOR_BINARY_OP
#undef SLANG_VECTOR_UNARY_OP
#undef SLANG_INT_VECTOR_OPS
#undef SLANG_FLOAT_VECTOR_OPS
#undef SLANG_VECTOR_INT_NEG_OP
#undef SLANG_FLOAT_VECTOR_MOD

template<typename T, int ROWS, int COLS>
struct Matrix
{
    Vector<T, COLS> rows[ROWS];
    const Vector<T, COLS>& operator[](size_t index) const { return rows[index]; }
    Vector<T, COLS>& operator[](size_t index) { return rows[index]; }
    Matrix() = default;
    Matrix(T scalar)
    {
        for (int i = 0; i < ROWS; i++)
            rows[i] = Vector<T, COLS>(scalar);
    }
    Matrix(const Vector<T, COLS>& row0) { rows[0] = row0; }
    Matrix(const Vector<T, COLS>& row0, const Vector<T, COLS>& row1)
    {
        rows[0] = row0;
        rows[1] = row1;
    }
    Matrix(const Vector<T, COLS>& row0, const Vector<T, COLS>& row1, const Vector<T, COLS>& row2)
    {
        rows[0] = row0;
        rows[1] = row1;
        rows[2] = row2;
    }
    Matrix(
        const Vector<T, COLS>& row0,
        const Vector<T, COLS>& row1,
        const Vector<T, COLS>& row2,
        const Vector<T, COLS>& row3)
    {
        rows[0] = row0;
        rows[1] = row1;
        rows[2] = row2;
        rows[3] = row3;
    }
    template<typename U, int otherRow, int otherCol>
    Matrix(const Matrix<U, otherRow, otherCol>& other)
    {
        int minRow = ROWS;
        int minCol = COLS;
        if (minRow > otherRow)
            minRow = otherRow;
        if (minCol > otherCol)
            minCol = otherCol;
        for (int i = 0; i < minRow; i++)
            for (int j = 0; j < minCol; j++)
                rows[i][j] = (T)other.rows[i][j];
    }
    Matrix(T v0, T v1, T v2, T v3)
    {
        rows[0][0] = v0;
        rows[0][1] = v1;
        rows[1][0] = v2;
        rows[1][1] = v3;
    }
    Matrix(T v0, T v1, T v2, T v3, T v4, T v5)
    {
        if (COLS == 3)
        {
            rows[0][0] = v0;
            rows[0][1] = v1;
            rows[0][2] = v2;
            rows[1][0] = v3;
            rows[1][1] = v4;
            rows[1][2] = v5;
        }
        else
        {
            rows[0][0] = v0;
            rows[0][1] = v1;
            rows[1][0] = v2;
            rows[1][1] = v3;
            rows[2][0] = v4;
            rows[2][1] = v5;
        }
    }
    Matrix(T v0, T v1, T v2, T v3, T v4, T v5, T v6, T v7)
    {
        if (COLS == 4)
        {
            rows[0][0] = v0;
            rows[0][1] = v1;
            rows[0][2] = v2;
            rows[0][3] = v3;
            rows[1][0] = v4;
            rows[1][1] = v5;
            rows[1][2] = v6;
            rows[1][3] = v7;
        }
        else
        {
            rows[0][0] = v0;
            rows[0][1] = v1;
            rows[1][0] = v2;
            rows[1][1] = v3;
            rows[2][0] = v4;
            rows[2][1] = v5;
            rows[3][0] = v6;
            rows[3][1] = v7;
        }
    }
    Matrix(T v0, T v1, T v2, T v3, T v4, T v5, T v6, T v7, T v8)
    {
        rows[0][0] = v0;
        rows[0][1] = v1;
        rows[0][2] = v2;
        rows[1][0] = v3;
        rows[1][1] = v4;
        rows[1][2] = v5;
        rows[2][0] = v6;
        rows[2][1] = v7;
        rows[2][2] = v8;
    }
    Matrix(T v0, T v1, T v2, T v3, T v4, T v5, T v6, T v7, T v8, T v9, T v10, T v11)
    {
        if (COLS == 4)
        {
            rows[0][0] = v0;
            rows[0][1] = v1;
            rows[0][2] = v2;
            rows[0][3] = v3;
            rows[1][0] = v4;
            rows[1][1] = v5;
            rows[1][2] = v6;
            rows[1][3] = v7;
            rows[2][0] = v8;
            rows[2][1] = v9;
            rows[2][2] = v10;
            rows[2][3] = v11;
        }
        else
        {
            rows[0][0] = v0;
            rows[0][1] = v1;
            rows[0][2] = v2;
            rows[1][0] = v3;
            rows[1][1] = v4;
            rows[1][2] = v5;
            rows[2][0] = v6;
            rows[2][1] = v7;
            rows[2][2] = v8;
            rows[3][0] = v9;
            rows[3][1] = v10;
            rows[3][2] = v11;
        }
    }
    Matrix(
        T v0,
        T v1,
        T v2,
        T v3,
        T v4,
        T v5,
        T v6,
        T v7,
        T v8,
        T v9,
        T v10,
        T v11,
        T v12,
        T v13,
        T v14,
        T v15)
    {
        rows[0][0] = v0;
        rows[0][1] = v1;
        rows[0][2] = v2;
        rows[0][3] = v3;
        rows[1][0] = v4;
        rows[1][1] = v5;
        rows[1][2] = v6;
        rows[1][3] = v7;
        rows[2][0] = v8;
        rows[2][1] = v9;
        rows[2][2] = v10;
        rows[2][3] = v11;
        rows[3][0] = v12;
        rows[3][1] = v13;
        rows[3][2] = v14;
        rows[3][3] = v15;
    }
};

#define SLANG_MATRIX_BINARY_OP(T, op)                                                         \
    template<int R, int C>                                                                    \
    Matrix<T, R, C> operator op(const Matrix<T, R, C>& thisVal, const Matrix<T, R, C>& other) \
    {                                                                                         \
        Matrix<T, R, C> result;                                                               \
        for (int i = 0; i < R; i++)                                                           \
            for (int j = 0; j < C; j++)                                                       \
                result.rows[i][j] = thisVal.rows[i][j] op other.rows[i][j];                   \
        return result;                                                                        \
    }

#define SLANG_MATRIX_BINARY_COMPARE_OP(T, op)                                                    \
    template<int R, int C>                                                                       \
    Matrix<bool, R, C> operator op(const Matrix<T, R, C>& thisVal, const Matrix<T, R, C>& other) \
    {                                                                                            \
        Matrix<bool, R, C> result;                                                               \
        for (int i = 0; i < R; i++)                                                              \
            for (int j = 0; j < C; j++)                                                          \
                result.rows[i][j] = thisVal.rows[i][j] op other.rows[i][j];                      \
        return result;                                                                           \
    }

#define SLANG_MATRIX_UNARY_OP(T, op)                            \
    template<int R, int C>                                      \
    Matrix<T, R, C> operator op(const Matrix<T, R, C>& thisVal) \
    {                                                           \
        Matrix<T, R, C> result;                                 \
        for (int i = 0; i < R; i++)                             \
            for (int j = 0; j < C; j++)                         \
                result[i].rows[i][j] = op thisVal.rows[i][j];   \
        return result;                                          \
    }

#define SLANG_INT_MATRIX_OPS(T)           \
    SLANG_MATRIX_BINARY_OP(T, +)          \
    SLANG_MATRIX_BINARY_OP(T, -)          \
    SLANG_MATRIX_BINARY_OP(T, *)          \
    SLANG_MATRIX_BINARY_OP(T, /)          \
    SLANG_MATRIX_BINARY_OP(T, &)          \
    SLANG_MATRIX_BINARY_OP(T, |)          \
    SLANG_MATRIX_BINARY_OP(T, &&)         \
    SLANG_MATRIX_BINARY_OP(T, ||)         \
    SLANG_MATRIX_BINARY_OP(T, ^)          \
    SLANG_MATRIX_BINARY_OP(T, %)          \
    SLANG_MATRIX_BINARY_COMPARE_OP(T, >)  \
    SLANG_MATRIX_BINARY_COMPARE_OP(T, <)  \
    SLANG_MATRIX_BINARY_COMPARE_OP(T, >=) \
    SLANG_MATRIX_BINARY_COMPARE_OP(T, <=) \
    SLANG_MATRIX_BINARY_COMPARE_OP(T, ==) \
    SLANG_MATRIX_BINARY_COMPARE_OP(T, !=) \
    SLANG_MATRIX_UNARY_OP(T, !)           \
    SLANG_MATRIX_UNARY_OP(T, ~)
#define SLANG_FLOAT_MATRIX_OPS(T)         \
    SLANG_MATRIX_BINARY_OP(T, +)          \
    SLANG_MATRIX_BINARY_OP(T, -)          \
    SLANG_MATRIX_BINARY_OP(T, *)          \
    SLANG_MATRIX_BINARY_OP(T, /)          \
    SLANG_MATRIX_UNARY_OP(T, -)           \
    SLANG_MATRIX_BINARY_COMPARE_OP(T, >)  \
    SLANG_MATRIX_BINARY_COMPARE_OP(T, <)  \
    SLANG_MATRIX_BINARY_COMPARE_OP(T, >=) \
    SLANG_MATRIX_BINARY_COMPARE_OP(T, <=) \
    SLANG_MATRIX_BINARY_COMPARE_OP(T, ==) \
    SLANG_MATRIX_BINARY_COMPARE_OP(T, !=)
SLANG_INT_MATRIX_OPS(int)
SLANG_INT_MATRIX_OPS(int8_t)
SLANG_INT_MATRIX_OPS(int16_t)
SLANG_INT_MATRIX_OPS(int64_t)
SLANG_INT_MATRIX_OPS(uint)
SLANG_INT_MATRIX_OPS(uint8_t)
SLANG_INT_MATRIX_OPS(uint16_t)
SLANG_INT_MATRIX_OPS(uint64_t)
#if SLANG_INTPTR_TYPE_IS_DISTINCT
SLANG_INT_MATRIX_OPS(intptr_t)
SLANG_INT_MATRIX_OPS(uintptr_t)
#endif

SLANG_FLOAT_MATRIX_OPS(float)
SLANG_FLOAT_MATRIX_OPS(double)

#define SLANG_MATRIX_INT_NEG_OP(T)                                        \
    template<int R, int C>                                                \
    SLANG_FORCE_INLINE Matrix<T, R, C> operator-(Matrix<T, R, C> thisVal) \
    {                                                                     \
        Matrix<T, R, C> result;                                           \
        for (int i = 0; i < R; i++)                                       \
            for (int j = 0; j < C; j++)                                   \
                result.rows[i][j] = 0 - thisVal.rows[i][j];               \
        return result;                                                    \
    }
SLANG_MATRIX_INT_NEG_OP(int)
SLANG_MATRIX_INT_NEG_OP(int8_t)
SLANG_MATRIX_INT_NEG_OP(int16_t)
SLANG_MATRIX_INT_NEG_OP(int64_t)
SLANG_MATRIX_INT_NEG_OP(uint)
SLANG_MATRIX_INT_NEG_OP(uint8_t)
SLANG_MATRIX_INT_NEG_OP(uint16_t)
SLANG_MATRIX_INT_NEG_OP(uint64_t)
#if SLANG_INTPTR_TYPE_IS_DISTINCT
SLANG_MATRIX_INT_NEG_OP(intptr_t)
SLANG_MATRIX_INT_NEG_OP(uintptr_t)
#endif

#define SLANG_FLOAT_MATRIX_MOD(T)                                                             \
    template<int R, int C>                                                                    \
    SLANG_FORCE_INLINE Matrix<T, R, C> operator%(Matrix<T, R, C> left, Matrix<T, R, C> right) \
    {                                                                                         \
        Matrix<T, R, C> result;                                                               \
        for (int i = 0; i < R; i++)                                                           \
            for (int j = 0; j < C; j++)                                                       \
                result.rows[i][j] = _slang_fmod(left.rows[i][j], right.rows[i][j]);           \
        return result;                                                                        \
    }

SLANG_FLOAT_MATRIX_MOD(float)
SLANG_FLOAT_MATRIX_MOD(double)
#undef SLANG_FLOAT_MATRIX_MOD
#undef SLANG_MATRIX_BINARY_OP
#undef SLANG_MATRIX_UNARY_OP
#undef SLANG_INT_MATRIX_OPS
#undef SLANG_FLOAT_MATRIX_OPS
#undef SLANG_MATRIX_INT_NEG_OP
#undef SLANG_FLOAT_MATRIX_MOD

template<typename TResult, typename TInput>
TResult slang_bit_cast(TInput val)
{
    return *(TResult*)(&val);
}

#endif


typedef Vector<float, 2> float2;
typedef Vector<float, 3> float3;
typedef Vector<float, 4> float4;

typedef Vector<int32_t, 2> int2;
typedef Vector<int32_t, 3> int3;
typedef Vector<int32_t, 4> int4;

typedef Vector<uint32_t, 2> uint2;
typedef Vector<uint32_t, 3> uint3;
typedef Vector<uint32_t, 4> uint4;

// We can just map `NonUniformResourceIndex` type directly to the index type on CPU, as CPU does not
// require any special handling around such accesses.
typedef size_t NonUniformResourceIndex;

// ----------------------------- ResourceType -----------------------------------------

// https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/sm5-object-structuredbuffer-getdimensions
// Missing  Load(_In_  int  Location, _Out_ uint Status);

template<typename T>
struct RWStructuredBuffer
{
    SLANG_FORCE_INLINE T& operator[](size_t index) const
    {
        SLANG_BOUND_CHECK(index, count);
        return data[index];
    }
    const T& Load(size_t index) const
    {
        SLANG_BOUND_CHECK(index, count);
        return data[index];
    }
    void GetDimensions(uint32_t* outNumStructs, uint32_t* outStride)
    {
        *outNumStructs = uint32_t(count);
        *outStride = uint32_t(sizeof(T));
    }

    T* data;
    size_t count;
};

template<typename T>
struct StructuredBuffer
{
    SLANG_FORCE_INLINE T& operator[](size_t index) const
    {
        SLANG_BOUND_CHECK(index, count);
        return data[index];
    }
    T& Load(size_t index) const
    {
        SLANG_BOUND_CHECK(index, count);
        return data[index];
    }
    void GetDimensions(uint32_t* outNumStructs, uint32_t* outStride)
    {
        *outNumStructs = uint32_t(count);
        *outStride = uint32_t(sizeof(T));
    }

    T* data;
    size_t count;
};


template<typename T>
struct RWBuffer
{
    SLANG_FORCE_INLINE T& operator[](size_t index) const
    {
        SLANG_BOUND_CHECK(index, count);
        return data[index];
    }
    const T& Load(size_t index) const
    {
        SLANG_BOUND_CHECK(index, count);
        return data[index];
    }
    void GetDimensions(uint32_t* outCount) { *outCount = uint32_t(count); }

    T* data;
    size_t count;
};

template<typename T>
struct Buffer
{
    SLANG_FORCE_INLINE const T& operator[](size_t index) const
    {
        SLANG_BOUND_CHECK(index, count);
        return data[index];
    }
    const T& Load(size_t index) const
    {
        SLANG_BOUND_CHECK(index, count);
        return data[index];
    }
    void GetDimensions(uint32_t* outCount) { *outCount = uint32_t(count); }

    T* data;
    size_t count;
};

// Missing  Load(_In_  int  Location, _Out_ uint Status);
struct ByteAddressBuffer
{
    void GetDimensions(uint32_t* outDim) const { *outDim = uint32_t(sizeInBytes); }
    uint32_t Load(size_t index) const
    {
        SLANG_BOUND_CHECK_BYTE_ADDRESS(index, 4, sizeInBytes);
        return data[index >> 2];
    }
    uint2 Load2(size_t index) const
    {
        SLANG_BOUND_CHECK_BYTE_ADDRESS(index, 8, sizeInBytes);
        const size_t dataIdx = index >> 2;
        return uint2{data[dataIdx], data[dataIdx + 1]};
    }
    uint3 Load3(size_t index) const
    {
        SLANG_BOUND_CHECK_BYTE_ADDRESS(index, 12, sizeInBytes);
        const size_t dataIdx = index >> 2;
        return uint3{data[dataIdx], data[dataIdx + 1], data[dataIdx + 2]};
    }
    uint4 Load4(size_t index) const
    {
        SLANG_BOUND_CHECK_BYTE_ADDRESS(index, 16, sizeInBytes);
        const size_t dataIdx = index >> 2;
        return uint4{data[dataIdx], data[dataIdx + 1], data[dataIdx + 2], data[dataIdx + 3]};
    }
    template<typename T>
    T Load(size_t index) const
    {
        SLANG_BOUND_CHECK_BYTE_ADDRESS(index, sizeof(T), sizeInBytes);
        return *(const T*)(((const char*)data) + index);
    }

    const uint32_t* data;
    size_t sizeInBytes; //< Must be multiple of 4
};

// https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/sm5-object-rwbyteaddressbuffer
// Missing support for Atomic operations
// Missing support for Load with status
struct RWByteAddressBuffer
{
    void GetDimensions(uint32_t* outDim) const { *outDim = uint32_t(sizeInBytes); }

    uint32_t Load(size_t index) const
    {
        SLANG_BOUND_CHECK_BYTE_ADDRESS(index, 4, sizeInBytes);
        return data[index >> 2];
    }
    uint2 Load2(size_t index) const
    {
        SLANG_BOUND_CHECK_BYTE_ADDRESS(index, 8, sizeInBytes);
        const size_t dataIdx = index >> 2;
        return uint2{data[dataIdx], data[dataIdx + 1]};
    }
    uint3 Load3(size_t index) const
    {
        SLANG_BOUND_CHECK_BYTE_ADDRESS(index, 12, sizeInBytes);
        const size_t dataIdx = index >> 2;
        return uint3{data[dataIdx], data[dataIdx + 1], data[dataIdx + 2]};
    }
    uint4 Load4(size_t index) const
    {
        SLANG_BOUND_CHECK_BYTE_ADDRESS(index, 16, sizeInBytes);
        const size_t dataIdx = index >> 2;
        return uint4{data[dataIdx], data[dataIdx + 1], data[dataIdx + 2], data[dataIdx + 3]};
    }
    template<typename T>
    T Load(size_t index) const
    {
        SLANG_BOUND_CHECK_BYTE_ADDRESS(index, sizeof(T), sizeInBytes);
        return *(const T*)(((const char*)data) + index);
    }

    void Store(size_t index, uint32_t v) const
    {
        SLANG_BOUND_CHECK_BYTE_ADDRESS(index, 4, sizeInBytes);
        data[index >> 2] = v;
    }
    void Store2(size_t index, uint2 v) const
    {
        SLANG_BOUND_CHECK_BYTE_ADDRESS(index, 8, sizeInBytes);
        const size_t dataIdx = index >> 2;
        data[dataIdx + 0] = v.x;
        data[dataIdx + 1] = v.y;
    }
    void Store3(size_t index, uint3 v) const
    {
        SLANG_BOUND_CHECK_BYTE_ADDRESS(index, 12, sizeInBytes);
        const size_t dataIdx = index >> 2;
        data[dataIdx + 0] = v.x;
        data[dataIdx + 1] = v.y;
        data[dataIdx + 2] = v.z;
    }
    void Store4(size_t index, uint4 v) const
    {
        SLANG_BOUND_CHECK_BYTE_ADDRESS(index, 16, sizeInBytes);
        const size_t dataIdx = index >> 2;
        data[dataIdx + 0] = v.x;
        data[dataIdx + 1] = v.y;
        data[dataIdx + 2] = v.z;
        data[dataIdx + 3] = v.w;
    }
    template<typename T>
    void Store(size_t index, T const& value) const
    {
        SLANG_BOUND_CHECK_BYTE_ADDRESS(index, sizeof(T), sizeInBytes);
        *(T*)(((char*)data) + index) = value;
    }

    uint32_t* data;
    size_t sizeInBytes; //< Must be multiple of 4
};

struct ISamplerState;
struct ISamplerComparisonState;

struct SamplerState
{
    ISamplerState* state;
};

struct SamplerComparisonState
{
    ISamplerComparisonState* state;
};

#ifndef SLANG_RESOURCE_SHAPE
#define SLANG_RESOURCE_SHAPE
typedef unsigned int SlangResourceShape;
enum
{
    SLANG_RESOURCE_BASE_SHAPE_MASK = 0x0F,

    SLANG_RESOURCE_NONE = 0x00,

    SLANG_TEXTURE_1D = 0x01,
    SLANG_TEXTURE_2D = 0x02,
    SLANG_TEXTURE_3D = 0x03,
    SLANG_TEXTURE_CUBE = 0x04,
    SLANG_TEXTURE_BUFFER = 0x05,

    SLANG_STRUCTURED_BUFFER = 0x06,
    SLANG_BYTE_ADDRESS_BUFFER = 0x07,
    SLANG_RESOURCE_UNKNOWN = 0x08,
    SLANG_ACCELERATION_STRUCTURE = 0x09,
    SLANG_TEXTURE_SUBPASS = 0x0A,

    SLANG_RESOURCE_EXT_SHAPE_MASK = 0xF0,

    SLANG_TEXTURE_FEEDBACK_FLAG = 0x10,
    SLANG_TEXTURE_ARRAY_FLAG = 0x40,
    SLANG_TEXTURE_MULTISAMPLE_FLAG = 0x80,

    SLANG_TEXTURE_1D_ARRAY = SLANG_TEXTURE_1D | SLANG_TEXTURE_ARRAY_FLAG,
    SLANG_TEXTURE_2D_ARRAY = SLANG_TEXTURE_2D | SLANG_TEXTURE_ARRAY_FLAG,
    SLANG_TEXTURE_CUBE_ARRAY = SLANG_TEXTURE_CUBE | SLANG_TEXTURE_ARRAY_FLAG,

    SLANG_TEXTURE_2D_MULTISAMPLE = SLANG_TEXTURE_2D | SLANG_TEXTURE_MULTISAMPLE_FLAG,
    SLANG_TEXTURE_2D_MULTISAMPLE_ARRAY =
        SLANG_TEXTURE_2D | SLANG_TEXTURE_MULTISAMPLE_FLAG | SLANG_TEXTURE_ARRAY_FLAG,
    SLANG_TEXTURE_SUBPASS_MULTISAMPLE = SLANG_TEXTURE_SUBPASS | SLANG_TEXTURE_MULTISAMPLE_FLAG,
};
#endif

//
struct TextureDimensions
{
    void reset()
    {
        shape = 0;
        width = height = depth = 0;
        numberOfLevels = 0;
        arrayElementCount = 0;
    }
    int getDimSizes(uint32_t outDims[4]) const
    {
        const auto baseShape = (shape & SLANG_RESOURCE_BASE_SHAPE_MASK);
        int count = 0;
        switch (baseShape)
        {
        case SLANG_TEXTURE_1D:
            {
                outDims[count++] = width;
                break;
            }
        case SLANG_TEXTURE_2D:
            {
                outDims[count++] = width;
                outDims[count++] = height;
                break;
            }
        case SLANG_TEXTURE_3D:
            {
                outDims[count++] = width;
                outDims[count++] = height;
                outDims[count++] = depth;
                break;
            }
        case SLANG_TEXTURE_CUBE:
            {
                outDims[count++] = width;
                outDims[count++] = height;
                outDims[count++] = 6;
                break;
            }
        }

        if (shape & SLANG_TEXTURE_ARRAY_FLAG)
        {
            outDims[count++] = arrayElementCount;
        }
        return count;
    }
    int getMIPDims(int outDims[3]) const
    {
        const auto baseShape = (shape & SLANG_RESOURCE_BASE_SHAPE_MASK);
        int count = 0;
        switch (baseShape)
        {
        case SLANG_TEXTURE_1D:
            {
                outDims[count++] = width;
                break;
            }
        case SLANG_TEXTURE_CUBE:
        case SLANG_TEXTURE_2D:
            {
                outDims[count++] = width;
                outDims[count++] = height;
                break;
            }
        case SLANG_TEXTURE_3D:
            {
                outDims[count++] = width;
                outDims[count++] = height;
                outDims[count++] = depth;
                break;
            }
        }
        return count;
    }
    int calcMaxMIPLevels() const
    {
        int dims[3];
        const int dimCount = getMIPDims(dims);
        for (int count = 1; true; count++)
        {
            bool allOne = true;
            for (int i = 0; i < dimCount; ++i)
            {
                if (dims[i] > 1)
                {
                    allOne = false;
                    dims[i] >>= 1;
                }
            }
            if (allOne)
            {
                return count;
            }
        }
    }

    uint32_t shape;
    uint32_t width, height, depth;
    uint32_t numberOfLevels;
    uint32_t arrayElementCount; ///< For array types, 0 otherwise
};


// Texture

struct ITexture
{
    virtual TextureDimensions GetDimensions(int mipLevel = -1) = 0;
    virtual void Load(const int32_t* v, void* outData, size_t dataSize) = 0;
    virtual void Sample(
        SamplerState samplerState,
        const float* loc,
        void* outData,
        size_t dataSize) = 0;
    virtual void SampleLevel(
        SamplerState samplerState,
        const float* loc,
        float level,
        void* outData,
        size_t dataSize) = 0;
};

template<typename T>
struct Texture1D
{
    void GetDimensions(uint32_t* outWidth) { *outWidth = texture->GetDimensions().width; }
    void GetDimensions(uint32_t mipLevel, uint32_t* outWidth, uint32_t* outNumberOfLevels)
    {
        auto dims = texture->GetDimensions(mipLevel);
        *outWidth = dims.width;
        *outNumberOfLevels = dims.numberOfLevels;
    }

    void GetDimensions(float* outWidth) { *outWidth = texture->GetDimensions().width; }
    void GetDimensions(uint32_t mipLevel, float* outWidth, float* outNumberOfLevels)
    {
        auto dims = texture->GetDimensions(mipLevel);
        *outWidth = dims.width;
        *outNumberOfLevels = dims.numberOfLevels;
    }

    T Load(const int2& loc) const
    {
        T out;
        texture->Load(&loc.x, &out, sizeof(out));
        return out;
    }
    T Sample(SamplerState samplerState, float loc) const
    {
        T out;
        texture->Sample(samplerState, &loc, &out, sizeof(out));
        return out;
    }
    T SampleLevel(SamplerState samplerState, float loc, float level) const
    {
        T out;
        texture->SampleLevel(samplerState, &loc, level, &out, sizeof(out));
        return out;
    }

    ITexture* texture;
};

template<typename T>
struct Texture2D
{
    void GetDimensions(uint32_t* outWidth, uint32_t* outHeight)
    {
        const auto dims = texture->GetDimensions();
        *outWidth = dims.width;
        *outHeight = dims.height;
    }
    void GetDimensions(
        uint32_t mipLevel,
        uint32_t* outWidth,
        uint32_t* outHeight,
        uint32_t* outNumberOfLevels)
    {
        const auto dims = texture->GetDimensions(mipLevel);
        *outWidth = dims.width;
        *outHeight = dims.height;
        *outNumberOfLevels = dims.numberOfLevels;
    }
    void GetDimensions(float* outWidth, float* outHeight)
    {
        const auto dims = texture->GetDimensions();
        *outWidth = dims.width;
        *outHeight = dims.height;
    }
    void GetDimensions(
        uint32_t mipLevel,
        float* outWidth,
        float* outHeight,
        float* outNumberOfLevels)
    {
        const auto dims = texture->GetDimensions(mipLevel);
        *outWidth = dims.width;
        *outHeight = dims.height;
        *outNumberOfLevels = dims.numberOfLevels;
    }

    T Load(const int3& loc) const
    {
        T out;
        texture->Load(&loc.x, &out, sizeof(out));
        return out;
    }
    T Sample(SamplerState samplerState, const float2& loc) const
    {
        T out;
        texture->Sample(samplerState, &loc.x, &out, sizeof(out));
        return out;
    }
    T SampleLevel(SamplerState samplerState, const float2& loc, float level) const
    {
        T out;
        texture->SampleLevel(samplerState, &loc.x, level, &out, sizeof(out));
        return out;
    }

    ITexture* texture;
};

template<typename T>
struct Texture3D
{
    void GetDimensions(uint32_t* outWidth, uint32_t* outHeight, uint32_t* outDepth)
    {
        const auto dims = texture->GetDimensions();
        *outWidth = dims.width;
        *outHeight = dims.height;
        *outDepth = dims.depth;
    }
    void GetDimensions(
        uint32_t mipLevel,
        uint32_t* outWidth,
        uint32_t* outHeight,
        uint32_t* outDepth,
        uint32_t* outNumberOfLevels)
    {
        const auto dims = texture->GetDimensions(mipLevel);
        *outWidth = dims.width;
        *outHeight = dims.height;
        *outDepth = dims.depth;
        *outNumberOfLevels = dims.numberOfLevels;
    }
    void GetDimensions(float* outWidth, float* outHeight, float* outDepth)
    {
        const auto dims = texture->GetDimensions();
        *outWidth = dims.width;
        *outHeight = dims.height;
        *outDepth = dims.depth;
    }
    void GetDimensions(
        uint32_t mipLevel,
        float* outWidth,
        float* outHeight,
        float* outDepth,
        float* outNumberOfLevels)
    {
        const auto dims = texture->GetDimensions(mipLevel);
        *outWidth = dims.width;
        *outHeight = dims.height;
        *outDepth = dims.depth;
        *outNumberOfLevels = dims.numberOfLevels;
    }

    T Load(const int4& loc) const
    {
        T out;
        texture->Load(&loc.x, &out, sizeof(out));
        return out;
    }
    T Sample(SamplerState samplerState, const float3& loc) const
    {
        T out;
        texture->Sample(samplerState, &loc.x, &out, sizeof(out));
        return out;
    }
    T SampleLevel(SamplerState samplerState, const float3& loc, float level) const
    {
        T out;
        texture->SampleLevel(samplerState, &loc.x, level, &out, sizeof(out));
        return out;
    }

    ITexture* texture;
};

template<typename T>
struct TextureCube
{
    void GetDimensions(uint32_t* outWidth, uint32_t* outHeight)
    {
        const auto dims = texture->GetDimensions();
        *outWidth = dims.width;
        *outHeight = dims.height;
    }
    void GetDimensions(
        uint32_t mipLevel,
        uint32_t* outWidth,
        uint32_t* outHeight,
        uint32_t* outNumberOfLevels)
    {
        const auto dims = texture->GetDimensions(mipLevel);
        *outWidth = dims.width;
        *outHeight = dims.height;
        *outNumberOfLevels = dims.numberOfLevels;
    }
    void GetDimensions(float* outWidth, float* outHeight)
    {
        const auto dims = texture->GetDimensions();
        *outWidth = dims.width;
        *outHeight = dims.height;
    }
    void GetDimensions(
        uint32_t mipLevel,
        float* outWidth,
        float* outHeight,
        float* outNumberOfLevels)
    {
        const auto dims = texture->GetDimensions(mipLevel);
        *outWidth = dims.width;
        *outHeight = dims.height;
        *outNumberOfLevels = dims.numberOfLevels;
    }

    T Sample(SamplerState samplerState, const float3& loc) const
    {
        T out;
        texture->Sample(samplerState, &loc.x, &out, sizeof(out));
        return out;
    }
    T SampleLevel(SamplerState samplerState, const float3& loc, float level) const
    {
        T out;
        texture->SampleLevel(samplerState, &loc.x, level, &out, sizeof(out));
        return out;
    }

    ITexture* texture;
};

template<typename T>
struct Texture1DArray
{
    void GetDimensions(uint32_t* outWidth, uint32_t* outElements)
    {
        auto dims = texture->GetDimensions();
        *outWidth = dims.width;
        *outElements = dims.arrayElementCount;
    }
    void GetDimensions(
        uint32_t mipLevel,
        uint32_t* outWidth,
        uint32_t* outElements,
        uint32_t* outNumberOfLevels)
    {
        auto dims = texture->GetDimensions(mipLevel);
        *outWidth = dims.width;
        *outNumberOfLevels = dims.numberOfLevels;
        *outElements = dims.arrayElementCount;
    }
    void GetDimensions(float* outWidth, float* outElements)
    {
        auto dims = texture->GetDimensions();
        *outWidth = dims.width;
        *outElements = dims.arrayElementCount;
    }
    void GetDimensions(
        uint32_t mipLevel,
        float* outWidth,
        float* outElements,
        float* outNumberOfLevels)
    {
        auto dims = texture->GetDimensions(mipLevel);
        *outWidth = dims.width;
        *outNumberOfLevels = dims.numberOfLevels;
        *outElements = dims.arrayElementCount;
    }

    T Load(const int3& loc) const
    {
        T out;
        texture->Load(&loc.x, &out, sizeof(out));
        return out;
    }
    T Sample(SamplerState samplerState, const float2& loc) const
    {
        T out;
        texture->Sample(samplerState, &loc.x, &out, sizeof(out));
        return out;
    }
    T SampleLevel(SamplerState samplerState, const float2& loc, float level) const
    {
        T out;
        texture->SampleLevel(samplerState, &loc.x, level, &out, sizeof(out));
        return out;
    }

    ITexture* texture;
};

template<typename T>
struct Texture2DArray
{
    void GetDimensions(uint32_t* outWidth, uint32_t* outHeight, uint32_t* outElements)
    {
        auto dims = texture->GetDimensions();
        *outWidth = dims.width;
        *outHeight = dims.height;
        *outElements = dims.arrayElementCount;
    }
    void GetDimensions(
        uint32_t mipLevel,
        uint32_t* outWidth,
        uint32_t* outHeight,
        uint32_t* outElements,
        uint32_t* outNumberOfLevels)
    {
        auto dims = texture->GetDimensions(mipLevel);
        *outWidth = dims.width;
        *outHeight = dims.height;
        *outElements = dims.arrayElementCount;
        *outNumberOfLevels = dims.numberOfLevels;
    }

    void GetDimensions(uint32_t* outWidth, float* outHeight, float* outElements)
    {
        auto dims = texture->GetDimensions();
        *outWidth = dims.width;
        *outHeight = dims.height;
        *outElements = dims.arrayElementCount;
    }
    void GetDimensions(
        uint32_t mipLevel,
        float* outWidth,
        float* outHeight,
        float* outElements,
        float* outNumberOfLevels)
    {
        auto dims = texture->GetDimensions(mipLevel);
        *outWidth = dims.width;
        *outHeight = dims.height;
        *outElements = dims.arrayElementCount;
        *outNumberOfLevels = dims.numberOfLevels;
    }

    T Load(const int4& loc) const
    {
        T out;
        texture->Load(&loc.x, &out, sizeof(out));
        return out;
    }
    T Sample(SamplerState samplerState, const float3& loc) const
    {
        T out;
        texture->Sample(samplerState, &loc.x, &out, sizeof(out));
        return out;
    }
    T SampleLevel(SamplerState samplerState, const float3& loc, float level) const
    {
        T out;
        texture->SampleLevel(samplerState, &loc.x, level, &out, sizeof(out));
        return out;
    }

    ITexture* texture;
};

template<typename T>
struct TextureCubeArray
{
    void GetDimensions(uint32_t* outWidth, uint32_t* outHeight, uint32_t* outElements)
    {
        auto dims = texture->GetDimensions();
        *outWidth = dims.width;
        *outHeight = dims.height;
        *outElements = dims.arrayElementCount;
    }
    void GetDimensions(
        uint32_t mipLevel,
        uint32_t* outWidth,
        uint32_t* outHeight,
        uint32_t* outElements,
        uint32_t* outNumberOfLevels)
    {
        auto dims = texture->GetDimensions(mipLevel);
        *outWidth = dims.width;
        *outHeight = dims.height;
        *outElements = dims.arrayElementCount;
        *outNumberOfLevels = dims.numberOfLevels;
    }

    void GetDimensions(uint32_t* outWidth, float* outHeight, float* outElements)
    {
        auto dims = texture->GetDimensions();
        *outWidth = dims.width;
        *outHeight = dims.height;
        *outElements = dims.arrayElementCount;
    }
    void GetDimensions(
        uint32_t mipLevel,
        float* outWidth,
        float* outHeight,
        float* outElements,
        float* outNumberOfLevels)
    {
        auto dims = texture->GetDimensions(mipLevel);
        *outWidth = dims.width;
        *outHeight = dims.height;
        *outElements = dims.arrayElementCount;
        *outNumberOfLevels = dims.numberOfLevels;
    }

    T Sample(SamplerState samplerState, const float4& loc) const
    {
        T out;
        texture->Sample(samplerState, &loc.x, &out, sizeof(out));
        return out;
    }
    T SampleLevel(SamplerState samplerState, const float4& loc, float level) const
    {
        T out;
        texture->SampleLevel(samplerState, &loc.x, level, &out, sizeof(out));
        return out;
    }

    ITexture* texture;
};

/* !!!!!!!!!!!!!!!!!!!!!!!!!!! RWTexture !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

struct IRWTexture : ITexture
{
    /// Get the reference to the element at loc.
    virtual void* refAt(const uint32_t* loc) = 0;
};

template<typename T>
struct RWTexture1D
{
    void GetDimensions(uint32_t* outWidth) { *outWidth = texture->GetDimensions().width; }
    void GetDimensions(uint32_t mipLevel, uint32_t* outWidth, uint32_t* outNumberOfLevels)
    {
        auto dims = texture->GetDimensions(mipLevel);
        *outWidth = dims.width;
        *outNumberOfLevels = dims.numberOfLevels;
    }

    void GetDimensions(float* outWidth) { *outWidth = texture->GetDimensions().width; }
    void GetDimensions(uint32_t mipLevel, float* outWidth, float* outNumberOfLevels)
    {
        auto dims = texture->GetDimensions(mipLevel);
        *outWidth = dims.width;
        *outNumberOfLevels = dims.numberOfLevels;
    }

    T Load(int32_t loc) const
    {
        T out;
        texture->Load(&loc, &out, sizeof(out));
        return out;
    }
    T& operator[](uint32_t loc) { return *(T*)texture->refAt(&loc); }
    IRWTexture* texture;
};

template<typename T>
struct RWTexture2D
{
    void GetDimensions(uint32_t* outWidth, uint32_t* outHeight)
    {
        const auto dims = texture->GetDimensions();
        *outWidth = dims.width;
        *outHeight = dims.height;
    }
    void GetDimensions(
        uint32_t mipLevel,
        uint32_t* outWidth,
        uint32_t* outHeight,
        uint32_t* outNumberOfLevels)
    {
        const auto dims = texture->GetDimensions(mipLevel);
        *outWidth = dims.width;
        *outHeight = dims.height;
        *outNumberOfLevels = dims.numberOfLevels;
    }
    void GetDimensions(float* outWidth, float* outHeight)
    {
        const auto dims = texture->GetDimensions();
        *outWidth = dims.width;
        *outHeight = dims.height;
    }
    void GetDimensions(
        uint32_t mipLevel,
        float* outWidth,
        float* outHeight,
        float* outNumberOfLevels)
    {
        const auto dims = texture->GetDimensions(mipLevel);
        *outWidth = dims.width;
        *outHeight = dims.height;
        *outNumberOfLevels = dims.numberOfLevels;
    }

    T Load(const int2& loc) const
    {
        T out;
        texture->Load(&loc.x, &out, sizeof(out));
        return out;
    }
    T& operator[](const uint2& loc) { return *(T*)texture->refAt(&loc.x); }
    IRWTexture* texture;
};

template<typename T>
struct RWTexture3D
{
    void GetDimensions(uint32_t* outWidth, uint32_t* outHeight, uint32_t* outDepth)
    {
        const auto dims = texture->GetDimensions();
        *outWidth = dims.width;
        *outHeight = dims.height;
        *outDepth = dims.depth;
    }
    void GetDimensions(
        uint32_t mipLevel,
        uint32_t* outWidth,
        uint32_t* outHeight,
        uint32_t* outDepth,
        uint32_t* outNumberOfLevels)
    {
        const auto dims = texture->GetDimensions(mipLevel);
        *outWidth = dims.width;
        *outHeight = dims.height;
        *outDepth = dims.depth;
        *outNumberOfLevels = dims.numberOfLevels;
    }
    void GetDimensions(float* outWidth, float* outHeight, float* outDepth)
    {
        const auto dims = texture->GetDimensions();
        *outWidth = dims.width;
        *outHeight = dims.height;
        *outDepth = dims.depth;
    }
    void GetDimensions(
        uint32_t mipLevel,
        float* outWidth,
        float* outHeight,
        float* outDepth,
        float* outNumberOfLevels)
    {
        const auto dims = texture->GetDimensions(mipLevel);
        *outWidth = dims.width;
        *outHeight = dims.height;
        *outDepth = dims.depth;
        *outNumberOfLevels = dims.numberOfLevels;
    }

    T Load(const int3& loc) const
    {
        T out;
        texture->Load(&loc.x, &out, sizeof(out));
        return out;
    }
    T& operator[](const uint3& loc) { return *(T*)texture->refAt(&loc.x); }
    IRWTexture* texture;
};


template<typename T>
struct RWTexture1DArray
{
    void GetDimensions(uint32_t* outWidth, uint32_t* outElements)
    {
        auto dims = texture->GetDimensions();
        *outWidth = dims.width;
        *outElements = dims.arrayElementCount;
    }
    void GetDimensions(
        uint32_t mipLevel,
        uint32_t* outWidth,
        uint32_t* outElements,
        uint32_t* outNumberOfLevels)
    {
        const auto dims = texture->GetDimensions(mipLevel);
        *outWidth = dims.width;
        *outElements = dims.arrayElementCount;
        *outNumberOfLevels = dims.numberOfLevels;
    }
    void GetDimensions(float* outWidth, float* outElements)
    {
        auto dims = texture->GetDimensions();
        *outWidth = dims.width;
        *outElements = dims.arrayElementCount;
    }
    void GetDimensions(
        uint32_t mipLevel,
        float* outWidth,
        float* outElements,
        float* outNumberOfLevels)
    {
        const auto dims = texture->GetDimensions(mipLevel);
        *outWidth = dims.width;
        *outElements = dims.arrayElementCount;
        *outNumberOfLevels = dims.numberOfLevels;
    }

    T Load(int2 loc) const
    {
        T out;
        texture->Load(&loc.x, &out, sizeof(out));
        return out;
    }
    T& operator[](uint2 loc) { return *(T*)texture->refAt(&loc.x); }

    IRWTexture* texture;
};

template<typename T>
struct RWTexture2DArray
{
    void GetDimensions(uint32_t* outWidth, uint32_t* outHeight, uint32_t* outElements)
    {
        auto dims = texture->GetDimensions();
        *outWidth = dims.width;
        *outHeight = dims.height;
        *outElements = dims.arrayElementCount;
    }
    void GetDimensions(
        uint32_t mipLevel,
        uint32_t* outWidth,
        uint32_t* outHeight,
        uint32_t* outElements,
        uint32_t* outNumberOfLevels)
    {
        const auto dims = texture->GetDimensions(mipLevel);
        *outWidth = dims.width;
        *outHeight = dims.height;
        *outElements = dims.arrayElementCount;
        *outNumberOfLevels = dims.numberOfLevels;
    }
    void GetDimensions(float* outWidth, float* outHeight, float* outElements)
    {
        auto dims = texture->GetDimensions();
        *outWidth = dims.width;
        *outHeight = dims.height;
        *outElements = dims.arrayElementCount;
    }
    void GetDimensions(
        uint32_t mipLevel,
        float* outWidth,
        float* outHeight,
        float* outElements,
        float* outNumberOfLevels)
    {
        const auto dims = texture->GetDimensions(mipLevel);
        *outWidth = dims.width;
        *outHeight = dims.height;
        *outElements = dims.arrayElementCount;
        *outNumberOfLevels = dims.numberOfLevels;
    }

    T Load(const int3& loc) const
    {
        T out;
        texture->Load(&loc.x, &out, sizeof(out));
        return out;
    }
    T& operator[](const uint3& loc) { return *(T*)texture->refAt(&loc.x); }

    IRWTexture* texture;
};

// FeedbackTexture

struct FeedbackType
{
};
struct SAMPLER_FEEDBACK_MIN_MIP : FeedbackType
{
};
struct SAMPLER_FEEDBACK_MIP_REGION_USED : FeedbackType
{
};

struct IFeedbackTexture
{
    virtual TextureDimensions GetDimensions(int mipLevel = -1) = 0;

    // Note here we pass the optional clamp parameter as a pointer. Passing nullptr means no clamp.
    // This was preferred over having two function definitions, and having to differentiate their
    // names
    virtual void WriteSamplerFeedback(
        ITexture* tex,
        SamplerState samp,
        const float* location,
        const float* clamp = nullptr) = 0;
    virtual void WriteSamplerFeedbackBias(
        ITexture* tex,
        SamplerState samp,
        const float* location,
        float bias,
        const float* clamp = nullptr) = 0;
    virtual void WriteSamplerFeedbackGrad(
        ITexture* tex,
        SamplerState samp,
        const float* location,
        const float* ddx,
        const float* ddy,
        const float* clamp = nullptr) = 0;

    virtual void WriteSamplerFeedbackLevel(
        ITexture* tex,
        SamplerState samp,
        const float* location,
        float lod) = 0;
};

template<typename T>
struct FeedbackTexture2D
{
    void GetDimensions(uint32_t* outWidth, uint32_t* outHeight)
    {
        const auto dims = texture->GetDimensions();
        *outWidth = dims.width;
        *outHeight = dims.height;
    }
    void GetDimensions(
        uint32_t mipLevel,
        uint32_t* outWidth,
        uint32_t* outHeight,
        uint32_t* outNumberOfLevels)
    {
        const auto dims = texture->GetDimensions(mipLevel);
        *outWidth = dims.width;
        *outHeight = dims.height;
        *outNumberOfLevels = dims.numberOfLevels;
    }
    void GetDimensions(float* outWidth, float* outHeight)
    {
        const auto dims = texture->GetDimensions();
        *outWidth = dims.width;
        *outHeight = dims.height;
    }
    void GetDimensions(
        uint32_t mipLevel,
        float* outWidth,
        float* outHeight,
        float* outNumberOfLevels)
    {
        const auto dims = texture->GetDimensions(mipLevel);
        *outWidth = dims.width;
        *outHeight = dims.height;
        *outNumberOfLevels = dims.numberOfLevels;
    }

    template<typename S>
    void WriteSamplerFeedback(Texture2D<S> tex, SamplerState samp, float2 location, float clamp)
    {
        texture->WriteSamplerFeedback(tex.texture, samp, &location.x, &clamp);
    }

    template<typename S>
    void WriteSamplerFeedbackBias(
        Texture2D<S> tex,
        SamplerState samp,
        float2 location,
        float bias,
        float clamp)
    {
        texture->WriteSamplerFeedbackBias(tex.texture, samp, &location.x, bias, &clamp);
    }

    template<typename S>
    void WriteSamplerFeedbackGrad(
        Texture2D<S> tex,
        SamplerState samp,
        float2 location,
        float2 ddx,
        float2 ddy,
        float clamp)
    {
        texture->WriteSamplerFeedbackGrad(tex.texture, samp, &location.x, &ddx.x, &ddy.x, &clamp);
    }

    // Level

    template<typename S>
    void WriteSamplerFeedbackLevel(Texture2D<S> tex, SamplerState samp, float2 location, float lod)
    {
        texture->WriteSamplerFeedbackLevel(tex.texture, samp, &location.x, lod);
    }

    // Without Clamp
    template<typename S>
    void WriteSamplerFeedback(Texture2D<S> tex, SamplerState samp, float2 location)
    {
        texture->WriteSamplerFeedback(tex.texture, samp, &location.x);
    }

    template<typename S>
    void WriteSamplerFeedbackBias(Texture2D<S> tex, SamplerState samp, float2 location, float bias)
    {
        texture->WriteSamplerFeedbackBias(tex.texture, samp, &location.x, bias);
    }

    template<typename S>
    void WriteSamplerFeedbackGrad(
        Texture2D<S> tex,
        SamplerState samp,
        float2 location,
        float2 ddx,
        float2 ddy)
    {
        texture->WriteSamplerFeedbackGrad(tex.texture, samp, &location.x, &ddx.x, &ddy.x);
    }

    IFeedbackTexture* texture;
};

template<typename T>
struct FeedbackTexture2DArray
{
    void GetDimensions(uint32_t* outWidth, uint32_t* outHeight, uint32_t* outElements)
    {
        auto dims = texture->GetDimensions();
        *outWidth = dims.width;
        *outHeight = dims.height;
        *outElements = dims.arrayElementCount;
    }
    void GetDimensions(
        uint32_t mipLevel,
        uint32_t* outWidth,
        uint32_t* outHeight,
        uint32_t* outElements,
        uint32_t* outNumberOfLevels)
    {
        const auto dims = texture->GetDimensions(mipLevel);
        *outWidth = dims.width;
        *outHeight = dims.height;
        *outElements = dims.arrayElementCount;
        *outNumberOfLevels = dims.numberOfLevels;
    }
    void GetDimensions(float* outWidth, float* outHeight, float* outElements)
    {
        auto dims = texture->GetDimensions();
        *outWidth = dims.width;
        *outHeight = dims.height;
        *outElements = dims.arrayElementCount;
    }
    void GetDimensions(
        uint32_t mipLevel,
        float* outWidth,
        float* outHeight,
        float* outElements,
        float* outNumberOfLevels)
    {
        const auto dims = texture->GetDimensions(mipLevel);
        *outWidth = dims.width;
        *outHeight = dims.height;
        *outElements = dims.arrayElementCount;
        *outNumberOfLevels = dims.numberOfLevels;
    }

    template<typename S>
    void WriteSamplerFeedback(
        Texture2DArray<S> texArray,
        SamplerState samp,
        float3 location,
        float clamp)
    {
        texture->WriteSamplerFeedback(texArray.texture, samp, &location.x, &clamp);
    }

    template<typename S>
    void WriteSamplerFeedbackBias(
        Texture2DArray<S> texArray,
        SamplerState samp,
        float3 location,
        float bias,
        float clamp)
    {
        texture->WriteSamplerFeedbackBias(texArray.texture, samp, &location.x, bias, &clamp);
    }

    template<typename S>
    void WriteSamplerFeedbackGrad(
        Texture2DArray<S> texArray,
        SamplerState samp,
        float3 location,
        float3 ddx,
        float3 ddy,
        float clamp)
    {
        texture
            ->WriteSamplerFeedbackGrad(texArray.texture, samp, &location.x, &ddx.x, &ddy.x, &clamp);
    }

    // Level
    template<typename S>
    void WriteSamplerFeedbackLevel(
        Texture2DArray<S> texArray,
        SamplerState samp,
        float3 location,
        float lod)
    {
        texture->WriteSamplerFeedbackLevel(texArray.texture, samp, &location.x, lod);
    }

    // Without Clamp

    template<typename S>
    void WriteSamplerFeedback(Texture2DArray<S> texArray, SamplerState samp, float3 location)
    {
        texture->WriteSamplerFeedback(texArray.texture, samp, &location.x);
    }

    template<typename S>
    void WriteSamplerFeedbackBias(
        Texture2DArray<S> texArray,
        SamplerState samp,
        float3 location,
        float bias)
    {
        texture->WriteSamplerFeedbackBias(texArray.texture, samp, &location.x, bias);
    }

    template<typename S>
    void WriteSamplerFeedbackGrad(
        Texture2DArray<S> texArray,
        SamplerState samp,
        float3 location,
        float3 ddx,
        float3 ddy)
    {
        texture->WriteSamplerFeedbackGrad(texArray.texture, samp, &location.x, &ddx.x, &ddy.x);
    }

    IFeedbackTexture* texture;
};

/* Varying input for Compute */

/* Used when running a single thread */
struct ComputeThreadVaryingInput
{
    uint3 groupID;
    uint3 groupThreadID;
};

struct ComputeVaryingInput
{
    uint3 startGroupID; ///< start groupID
    uint3 endGroupID;   ///< Non inclusive end groupID
};

// The uniformEntryPointParams and uniformState must be set to structures that match layout that the
// kernel expects. This can be determined via reflection for example.

typedef void (*ComputeThreadFunc)(
    ComputeThreadVaryingInput* varyingInput,
    void* uniformEntryPointParams,
    void* uniformState);
typedef void (*ComputeFunc)(
    ComputeVaryingInput* varyingInput,
    void* uniformEntryPointParams,
    void* uniformState);

#ifdef SLANG_PRELUDE_NAMESPACE
}
#endif

#endif


// Atomic helpers for the CPU target. Needed so kIROp_AtomicAdd and
// friends can lower to native atomic operations on the host — matching
// the semantics of HLSL `InterlockedAdd`, SPIR-V `OpAtomicIAdd`, etc.
// Uses compiler builtins so no <atomic> header is required.
//
// Contract for every `_slang_atomic_add_*` helper below (u32/i32/u64/
// i64): atomically add `val` to `*ptr` and return the PRIOR value (the
// value before the add), matching HLSL `InterlockedAdd` and GLSL
// `atomicAdd`. The 64-bit MSVC variants reinterpret the pointer as
// `volatile long long*` for `_InterlockedExchangeAdd64`, which is sound
// because `sizeof(long long) == 8` (asserted below).
//
// Compiler coverage:
//   - MSVC:            `_InterlockedExchangeAdd`
//   - GCC / Clang:     `__atomic_fetch_add`
//   - SNC / GHS etc.:  falls back to a non-atomic load/add/store.
//     These are console / embedded toolchains not known to ship
//     `__atomic_*` builtins; the fallback is racy under concurrency
//     but keeps the prelude buildable there. Platforms that need
//     real atomics on those compilers can add a bespoke branch.
// === MSVC implementations ===
//
// Use the `_Interlocked*` intrinsics. The 32-bit overload operates on
// `long`; the 64-bit overload operates on `long long`. The static
// asserts above pin the expected widths.
#if SLANG_VC
#include <intrin.h>
static_assert(
    sizeof(long) == 4,
    "_InterlockedExchangeAdd uses `long`; MSVC LLP64 requires sizeof(long)==4");
static_assert(
    sizeof(long long) == 8,
    "_InterlockedExchangeAdd64 uses `long long`; expected 8-byte width");
static inline uint32_t _slang_atomic_add_u32(uint32_t* ptr, uint32_t val)
{
    // Returns the PRIOR value, matching HLSL InterlockedAdd and GLSL atomicAdd.
    return static_cast<uint32_t>(
        _InterlockedExchangeAdd(reinterpret_cast<volatile long*>(ptr), static_cast<long>(val)));
}
static inline int32_t _slang_atomic_add_i32(int32_t* ptr, int32_t val)
{
    return static_cast<int32_t>(
        _InterlockedExchangeAdd(reinterpret_cast<volatile long*>(ptr), static_cast<long>(val)));
}
static inline uint64_t _slang_atomic_add_u64(uint64_t* ptr, uint64_t val)
{
    return static_cast<uint64_t>(_InterlockedExchangeAdd64(
        reinterpret_cast<volatile long long*>(ptr),
        static_cast<long long>(val)));
}
static inline int64_t _slang_atomic_add_i64(int64_t* ptr, int64_t val)
{
    return static_cast<int64_t>(_InterlockedExchangeAdd64(
        reinterpret_cast<volatile long long*>(ptr),
        static_cast<long long>(val)));
}
// === GCC / Clang implementations ===
//
// Use the `__atomic_fetch_add` built-in with relaxed ordering; the
// IR-side AtomicAdd emit ignores the memory-order operand and lets
// each toolchain pick its native default.
#elif SLANG_GCC || SLANG_CLANG
static inline uint32_t _slang_atomic_add_u32(uint32_t* ptr, uint32_t val)
{
    return __atomic_fetch_add(ptr, val, __ATOMIC_RELAXED);
}
static inline int32_t _slang_atomic_add_i32(int32_t* ptr, int32_t val)
{
    return __atomic_fetch_add(ptr, val, __ATOMIC_RELAXED);
}
static inline uint64_t _slang_atomic_add_u64(uint64_t* ptr, uint64_t val)
{
    return __atomic_fetch_add(ptr, val, __ATOMIC_RELAXED);
}
static inline int64_t _slang_atomic_add_i64(int64_t* ptr, int64_t val)
{
    return __atomic_fetch_add(ptr, val, __ATOMIC_RELAXED);
}
// === Non-atomic fallback implementations ===
//
// For compilers without a known atomic builtin (Sony SNC, Green Hills
// MULTI, etc.). Racy under concurrent invocation but keeps the
// prelude compilable; CPU-target coverage on these platforms is
// single-threaded in practice.
#else
static inline uint32_t _slang_atomic_add_u32(uint32_t* ptr, uint32_t val)
{
    uint32_t old = *ptr;
    *ptr = old + val;
    return old;
}
static inline int32_t _slang_atomic_add_i32(int32_t* ptr, int32_t val)
{
    int32_t old = *ptr;
    *ptr = old + val;
    return old;
}
static inline uint64_t _slang_atomic_add_u64(uint64_t* ptr, uint64_t val)
{
    uint64_t old = *ptr;
    *ptr = old + val;
    return old;
}
static inline int64_t _slang_atomic_add_i64(int64_t* ptr, int64_t val)
{
    int64_t old = *ptr;
    *ptr = old + val;
    return old;
}
#endif

// TODO(JS): Hack! Output C++ code from slang can copy uninitialized variables.
#if defined(_MSC_VER)
#pragma warning(disable : 4700)
#endif

#ifndef SLANG_UNROLL
#define SLANG_UNROLL
#endif

#endif

#ifdef SLANG_PRELUDE_NAMESPACE
using namespace SLANG_PRELUDE_NAMESPACE;
#endif


#line 1 "slang/bone_sample.slang"
struct BoneSampleParams_0
{
    uint32_t T_0;
    float time_0;
    float length_0;
    uint32_t loopMode_0;
    uint32_t backward_0;
    float timeLo_0;
    float lengthLo_0;
};


#line 364
struct GlobalParams_0
{
    BoneSampleParams_0* params_0;
    StructuredBuffer<uint32_t> trackDst_0;
    RWStructuredBuffer<float> localPose_0;
    StructuredBuffer<uint32_t> trackKeys_0;
    StructuredBuffer<uint32_t> trackKind_0;
    StructuredBuffer<float> keyTime_0;
    StructuredBuffer<float> keyVal_0;
    StructuredBuffer<float> keyTrans_0;
    StructuredBuffer<float> keyTimeLo_0;
};


#line 364
struct KernelContext_0
{
    GlobalParams_0* globalParams_0;
};


#line 10098 "hlsl.meta.slang"
static float dot_0(Vector<float, 3>  x_0, Vector<float, 3>  y_0)
{

#line 10098
    int32_t i_0 = int(0);

#line 10098
    float result_0 = 0.0f;

#line 10117
    for(;;)
    {

#line 10117
        if(i_0 < int(3))
        {
        }
        else
        {

#line 10117
            break;
        }

#line 10118
        float result_1 = result_0 + _slang_vector_get_element(x_0, i_0) * _slang_vector_get_element(y_0, i_0);

#line 10117
        i_0 = i_0 + int(1);

#line 10117
        result_0 = result_1;

#line 10117
    }

    return result_0;
}


#line 9543
static Vector<float, 3>  cross_0(Vector<float, 3>  left_0, Vector<float, 3>  right_0)
{

#line 9557
    float _S1 = left_0.y;

#line 9557
    float _S2 = right_0.z;

#line 9557
    float _S3 = left_0.z;

#line 9557
    float _S4 = right_0.y;
    float _S5 = right_0.x;

#line 9558
    float _S6 = left_0.x;

#line 9556
    return Vector<float, 3> (_S1 * _S2 - _S3 * _S4, _S3 * _S5 - _S6 * _S2, _S6 * _S4 - _S1 * _S5);
}


#line 12353
static float length_1(Vector<float, 3>  x_1)
{

#line 12365
    return (F32_sqrt((dot_0(x_1, x_1))));
}


#line 10098
static float dot_1(Vector<float, 4>  x_2, Vector<float, 4>  y_1)
{

#line 10098
    int32_t i_1 = int(0);

#line 10098
    float result_2 = 0.0f;

#line 10117
    for(;;)
    {

#line 10117
        if(i_1 < int(4))
        {
        }
        else
        {

#line 10117
            break;
        }

#line 10118
        float result_3 = result_2 + _slang_vector_get_element(x_2, i_1) * _slang_vector_get_element(y_1, i_1);

#line 10117
        i_1 = i_1 + int(1);

#line 10117
        result_2 = result_3;

#line 10117
    }

    return result_2;
}


#line 12353
static float length_2(Vector<float, 4>  x_3)
{

#line 12365
    return (F32_sqrt((dot_1(x_3, x_3))));
}


#line 9089
static int32_t clamp_0(int32_t x_4, int32_t minBound_0, int32_t maxBound_0)
{

#line 9107
    return (I32_min(((I32_max((x_4), (minBound_0)))), (maxBound_0)));
}


#line 30 "slang/bone_sample.slang"
static Vector<float, 2>  bk_two_sum_0(float a_0, float b_0)
{

#line 31
    precise float s_0 = a_0 + b_0;
    precise float bb_0 = s_0 - a_0;
    precise float e_0 = a_0 - (s_0 - bb_0) + (b_0 - bb_0);
    return Vector<float, 2> (s_0, e_0);
}


#line 52
static Vector<float, 2>  bk_df_add_0(Vector<float, 2>  a_1, Vector<float, 2>  b_1)
{

#line 53
    precise Vector<float, 2>  s_1 = bk_two_sum_0(a_1.x, b_1.x);
    precise float e_1 = s_1.y + a_1.y + b_1.y;
    float _S7 = s_1.x;

#line 55
    precise float h_0 = _S7 + e_1;
    precise float l_0 = e_1 - (h_0 - _S7);
    return Vector<float, 2> (h_0, l_0);
}

static Vector<float, 2>  bk_df_sub_0(Vector<float, 2>  a_2, Vector<float, 2>  b_2)
{

#line 61
    return bk_df_add_0(a_2, - b_2);
}


#line 324
static int32_t bk_fetch_idx_0(int32_t p_0, int32_t len_0, bool wrap_0, uint32_t mode_0)
{

#line 325
    if(!wrap_0)
    {

#line 326
        return clamp_0(p_0, int(0), len_0 - int(1));
    }
    int32_t idx_0 = p_0 % len_0;

#line 328
    int32_t idx_1;
    if(idx_0 < int(0))
    {

#line 329
        idx_1 = idx_0 + len_0;

#line 329
    }
    else
    {

#line 329
        idx_1 = idx_0;

#line 329
    }


    int32_t cyc_0 = (p_0 - idx_1) / len_0;

#line 332
    bool _S8;
    if(mode_0 == 1U)
    {

#line 333
        _S8 = true;

#line 333
    }
    else
    {

#line 333
        _S8 = (cyc_0 % int(2)) == int(0);

#line 333
    }

#line 333
    if(_S8)
    {

#line 334
        return idx_1;
    }
    return len_0 - int(1) - idx_1;
}


#line 37
static Vector<float, 2>  bk_split_0(float a_3)
{

#line 38
    precise float c_0 = 4097.0f * a_3;
    precise float hi_0 = c_0 - (c_0 - a_3);
    precise float lo_0 = a_3 - hi_0;
    return Vector<float, 2> (hi_0, lo_0);
}

static Vector<float, 2>  bk_two_prod_0(float a_4, float b_3)
{

#line 45
    precise float pr_0 = a_4 * b_3;
    precise Vector<float, 2>  sa_0 = bk_split_0(a_4);
    precise Vector<float, 2>  sb_0 = bk_split_0(b_3);
    float _S9 = sa_0.x;

#line 48
    float _S10 = sb_0.x;

#line 48
    float _S11 = sb_0.y;

#line 48
    float _S12 = sa_0.y;

#line 48
    precise float e_2 = _S9 * _S10 - pr_0 + _S9 * _S11 + _S12 * _S10 + _S12 * _S11;
    return Vector<float, 2> (pr_0, e_2);
}


#line 64
static Vector<float, 2>  bk_df_mulf_0(Vector<float, 2>  a_5, float b_4)
{

#line 65
    precise Vector<float, 2>  pr_1 = bk_two_prod_0(a_5.x, b_4);
    precise float e_3 = pr_1.y + a_5.y * b_4;
    float _S13 = pr_1.x;

#line 67
    precise float h_1 = _S13 + e_3;
    precise float l_1 = e_3 - (h_1 - _S13);
    return Vector<float, 2> (h_1, l_1);
}


#line 339
static Vector<float, 2>  bk_fetch_time_0(int32_t p_1, int32_t len_1, bool wrap_1, uint32_t mode_1, uint32_t off_0, Vector<float, 2>  L_0, KernelContext_0 * kernelContext_0)
{

#line 340
    if(!wrap_1)
    {

#line 341
        uint32_t _S14 = off_0 + uint32_t(clamp_0(p_1, int(0), len_1 - int(1)));

#line 341
        return Vector<float, 2> (kernelContext_0->globalParams_0->keyTime_0.Load(_S14), kernelContext_0->globalParams_0->keyTimeLo_0.Load(_S14));
    }
    int32_t idx_2 = p_1 % len_1;

#line 343
    int32_t idx_3;
    if(idx_2 < int(0))
    {

#line 344
        idx_3 = idx_2 + len_1;

#line 344
    }
    else
    {

#line 344
        idx_3 = idx_2;

#line 344
    }


    int32_t cyc_1 = (p_1 - idx_3) / len_1;

#line 347
    bool _S15;
    if(mode_1 == 1U)
    {

#line 348
        _S15 = true;

#line 348
    }
    else
    {

#line 348
        _S15 = (cyc_1 % int(2)) == int(0);

#line 348
    }

#line 348
    if(_S15)
    {

#line 349
        uint32_t _S16 = off_0 + uint32_t(idx_3);

#line 349
        return bk_df_add_0(bk_df_mulf_0(L_0, float(cyc_1)), Vector<float, 2> (kernelContext_0->globalParams_0->keyTime_0.Load(_S16), kernelContext_0->globalParams_0->keyTimeLo_0.Load(_S16)));
    }
    uint32_t _S17 = off_0 + uint32_t(len_1 - int(1) - idx_3);

#line 351
    return bk_df_sub_0(bk_df_mulf_0(L_0, float(cyc_1 + int(1))), Vector<float, 2> (kernelContext_0->globalParams_0->keyTime_0.Load(_S17), kernelContext_0->globalParams_0->keyTimeLo_0.Load(_S17)));
}


#line 72
static float bk_ease_0(float x0_0, float c_1)
{

#line 73
    float _S18 = (F32_min(((F32_max((x0_0), (0.0f)))), (1.0f)));
    if(c_1 > 0.0f)
    {

#line 75
        if(c_1 < 1.0f)
        {

#line 76
            return 1.0f - (F32_pow((1.0f - _S18), (1.0f / c_1)));
        }
        else
        {

#line 78
            return (F32_pow((_S18), (c_1)));
        }

#line 78
    }
    else
    {
        if(c_1 < 0.0f)
        {

#line 82
            if(_S18 < 0.5f)
            {

#line 83
                return (F32_pow((_S18 * 2.0f), (- c_1))) * 0.5f;
            }
            else
            {

#line 85
                return (1.0f - (F32_pow((1.0f - (_S18 - 0.5f) * 2.0f), (- c_1)))) * 0.5f + 0.5f;
            }

#line 85
        }

#line 74
    }

#line 89
    return 0.0f;
}


#line 301
static float bk_angle_diff_0(float from_0, float to_0)
{

#line 302
    float d_0 = (F32_fmod((to_0 - from_0), (6.28318548202514648f)));
    return (F32_fmod((2.0f * d_0), (6.28318548202514648f))) - d_0;
}


#line 293
static float bk_fposmod_0(float x_5, float y_2)
{

#line 294
    float r_0 = (F32_fmod((x_5), (y_2)));

#line 294
    bool _S19;
    if(r_0 < 0.0f)
    {

#line 295
        _S19 = y_2 > 0.0f;

#line 295
    }
    else
    {

#line 295
        _S19 = false;

#line 295
    }

#line 295
    if(_S19)
    {

#line 295
        _S19 = true;

#line 295
    }
    else
    {

#line 295
        if(r_0 > 0.0f)
        {

#line 295
            _S19 = y_2 < 0.0f;

#line 295
        }
        else
        {

#line 295
            _S19 = false;

#line 295
        }

#line 295
    }

#line 295
    float r_1;

#line 295
    if(_S19)
    {

#line 295
        r_1 = r_0 + y_2;

#line 295
    }
    else
    {

#line 295
        r_1 = r_0;

#line 295
    }


    return r_1;
}


#line 92
static float bk_cubic_0(float from_1, float to_1, float pre_0, float post_0, float w_0, Vector<float, 2>  toT_0, Vector<float, 2>  preT_0, Vector<float, 2>  postT_0)
{

#line 93
    precise Vector<float, 2>  t_0 = bk_df_mulf_0(toT_0, w_0);
    float fTo_0 = toT_0.x + toT_0.y;
    float fPre_0 = preT_0.x + preT_0.y;
    float fPost_0 = postT_0.x + postT_0.y;
    Vector<float, 2>  _S20 = bk_df_sub_0(postT_0, toT_0);

#line 97
    float dPostTo_0 = _S20.x + _S20.y;
    Vector<float, 2>  _S21 = bk_df_sub_0(toT_0, preT_0);

#line 98
    float dToPre_0 = _S21.x + _S21.y;
    Vector<float, 2>  _S22 = bk_df_sub_0(t_0, preT_0);

#line 99
    float tPre_0 = _S22.x + _S22.y;
    Vector<float, 2>  _S23 = bk_df_sub_0(t_0, toT_0);

#line 100
    float tTo_0 = _S23.x + _S23.y;
    float tt_0 = t_0.x + t_0.y;

#line 101
    float _S24;
    if(fPre_0 == 0.0f)
    {

#line 102
        _S24 = 0.0f;

#line 102
    }
    else
    {

#line 102
        _S24 = tPre_0 / - fPre_0;

#line 102
    }

#line 102
    float a1_0 = pre_0 + _S24 * (from_1 - pre_0);
    bool _S25 = fTo_0 == 0.0f;

#line 103
    if(_S25)
    {

#line 103
        _S24 = 0.5f;

#line 103
    }
    else
    {

#line 103
        _S24 = tt_0 / fTo_0;

#line 103
    }

#line 103
    float a2_0 = from_1 + _S24 * (to_1 - from_1);
    if(dPostTo_0 == 0.0f)
    {

#line 104
        _S24 = 1.0f;

#line 104
    }
    else
    {

#line 104
        _S24 = tTo_0 / dPostTo_0;

#line 104
    }

#line 104
    float a3_0 = to_1 + _S24 * (post_0 - to_1);
    if(dToPre_0 == 0.0f)
    {

#line 105
        _S24 = 0.0f;

#line 105
    }
    else
    {

#line 105
        _S24 = tPre_0 / dToPre_0;

#line 105
    }

#line 105
    float b1_0 = a1_0 + _S24 * (a2_0 - a1_0);
    if(fPost_0 == 0.0f)
    {

#line 106
        _S24 = 1.0f;

#line 106
    }
    else
    {

#line 106
        _S24 = tt_0 / fPost_0;

#line 106
    }

#line 106
    float b2_0 = a2_0 + _S24 * (a3_0 - a2_0);
    if(_S25)
    {

#line 107
        _S24 = 0.5f;

#line 107
    }
    else
    {

#line 107
        _S24 = tt_0 / fTo_0;

#line 107
    }

#line 107
    return b1_0 + _S24 * (b2_0 - b1_0);
}


#line 306
static float bk_cubic_angle_0(float from_2, float to_2, float pre_1, float post_1, float w_1, Vector<float, 2>  toT_1, Vector<float, 2>  preT_1, Vector<float, 2>  postT_1)
{

#line 307
    float fr_0 = (F32_fmod((from_2), (6.28318548202514648f)));
    float _S26 = (F32_fmod((pre_1 - fr_0), (6.28318548202514648f)));
    float _S27 = (F32_fmod((to_2 - fr_0), (6.28318548202514648f)));

#line 309
    float tr_0 = fr_0 + (F32_fmod((2.0f * _S27), (6.28318548202514648f))) - _S27;
    float _S28 = (F32_fmod((post_1 - tr_0), (6.28318548202514648f)));
    return bk_cubic_0(fr_0, tr_0, fr_0 + (F32_fmod((2.0f * _S26), (6.28318548202514648f))) - _S26, tr_0 + (F32_fmod((2.0f * _S28), (6.28318548202514648f))) - _S28, w_1, toT_1, preT_1, postT_1);
}


#line 110
static float bk_makima_tan_0(float p0_0, float p1_0, float p2_0, float p3_0)
{

#line 111
    float w1_0 = (F32_abs((p3_0 - p2_0))) + (F32_abs((p3_0 + p2_0))) * 0.5f;
    float w2_0 = (F32_abs((p1_0 - p0_0))) + (F32_abs((p1_0 + p0_0))) * 0.5f;
    float _S29 = w1_0 + w2_0;

#line 113
    if(_S29 == 0.0f)
    {

#line 114
        return (p1_0 + p2_0) * 0.5f;
    }
    return (w1_0 * p1_0 + w2_0 * p2_0) / _S29;
}

static float bk_makima_0(float from_3, float to_3, float pre_2, float post_2, float pp_0, float qq_0, float w_2, Vector<float, 2>  toT_2, Vector<float, 2>  preT_2, Vector<float, 2>  postT_2, Vector<float, 2>  ppT_0, Vector<float, 2>  qqT_0)
{

#line 120
    float fTo_1 = toT_2.x + toT_2.y;
    float fPre_1 = preT_2.x + preT_2.y;
    Vector<float, 2>  _S30 = bk_df_sub_0(postT_2, toT_2);

#line 122
    float dPost_0 = _S30.x + _S30.y;
    Vector<float, 2>  _S31 = bk_df_sub_0(preT_2, ppT_0);

#line 123
    float dPp_0 = _S31.x + _S31.y;
    Vector<float, 2>  _S32 = bk_df_sub_0(qqT_0, postT_2);

#line 124
    float dQq_0 = _S32.x + _S32.y;

    bool cPre_0 = fPre_1 == 0.0f;
    bool cPost_0 = dPost_0 == 0.0f;
    bool cPp_0 = dPp_0 == 0.0f;
    bool cQq_0 = dQq_0 == 0.0f;

#line 129
    float mFrom_0;
    if(fTo_1 == 0.0f)
    {

#line 130
        mFrom_0 = 0.0f;

#line 130
    }
    else
    {

#line 130
        mFrom_0 = (to_3 - from_3) / fTo_1;

#line 130
    }

#line 130
    float mPre_0;
    if(cPre_0)
    {

#line 131
        mPre_0 = 0.0f;

#line 131
    }
    else
    {

#line 131
        mPre_0 = (from_3 - pre_2) / - fPre_1;

#line 131
    }

#line 131
    float mPost_0;
    if(cPost_0)
    {

#line 132
        mPost_0 = 0.0f;

#line 132
    }
    else
    {

#line 132
        mPost_0 = (post_2 - to_3) / dPost_0;

#line 132
    }
    if(cPre_0)
    {

#line 134
        if(cPost_0)
        {

#line 134
            mPre_0 = mFrom_0;

#line 134
        }
        else
        {

#line 134
            mPre_0 = 2.0f * mFrom_0 - mPost_0;

#line 134
        }

#line 133
    }


    if(cPost_0)
    {

#line 137
        if(cPre_0)
        {

#line 137
            mPost_0 = mFrom_0;

#line 137
        }
        else
        {

#line 137
            mPost_0 = 2.0f * mFrom_0 - mPre_0;

#line 137
        }

#line 136
    }

#line 136
    float mPp_0;


    if(cPp_0)
    {

#line 139
        mPp_0 = 2.0f * mPre_0 - mFrom_0;

#line 139
    }
    else
    {

#line 139
        mPp_0 = (pre_2 - pp_0) / dPp_0;

#line 139
    }

#line 139
    float mQq_0;
    if(cQq_0)
    {

#line 140
        mQq_0 = 2.0f * mPost_0 - mFrom_0;

#line 140
    }
    else
    {

#line 140
        mQq_0 = (qq_0 - post_2) / dQq_0;

#line 140
    }


    float s2_0 = w_2 * w_2;
    float s3_0 = s2_0 * w_2;
    float _S33 = 3.0f * s2_0;

#line 145
    return (2.0f * s3_0 - _S33 + 1.0f) * from_3 + (s3_0 - 2.0f * s2_0 + w_2) * fTo_1 * bk_makima_tan_0(mPp_0, mPre_0, mFrom_0, mPost_0) + (-2.0f * s3_0 + _S33) * to_3 + (s3_0 - s2_0) * fTo_1 * bk_makima_tan_0(mPre_0, mFrom_0, mPost_0, mQq_0);
}


#line 314
static float bk_makima_angle_0(float from_4, float to_4, float pre_3, float post_3, float pp_1, float qq_1, float w_3, Vector<float, 2>  toT_3, Vector<float, 2>  preT_3, Vector<float, 2>  postT_3, Vector<float, 2>  ppT_1, Vector<float, 2>  qqT_1)
{

#line 315
    float fr_1 = (F32_fmod((from_4), (6.28318548202514648f)));
    float _S34 = (F32_fmod((pre_3 - fr_1), (6.28318548202514648f)));

#line 316
    float pr_2 = fr_1 + (F32_fmod((2.0f * _S34), (6.28318548202514648f))) - _S34;
    float _S35 = (F32_fmod((pp_1 - pr_2), (6.28318548202514648f)));
    float _S36 = (F32_fmod((to_4 - fr_1), (6.28318548202514648f)));

#line 318
    float tr_1 = fr_1 + (F32_fmod((2.0f * _S36), (6.28318548202514648f))) - _S36;
    float _S37 = (F32_fmod((post_3 - tr_1), (6.28318548202514648f)));

#line 319
    float po_0 = tr_1 + (F32_fmod((2.0f * _S37), (6.28318548202514648f))) - _S37;
    float _S38 = (F32_fmod((qq_1 - po_0), (6.28318548202514648f)));
    return bk_makima_0(fr_1, tr_1, pr_2, po_0, pr_2 + (F32_fmod((2.0f * _S35), (6.28318548202514648f))) - _S35, po_0 + (F32_fmod((2.0f * _S38), (6.28318548202514648f))) - _S38, w_3, toT_3, preT_3, postT_3, ppT_1, qqT_1);
}


#line 173
static Vector<float, 4>  bk_slerp_0(Vector<float, 4>  a_6, Vector<float, 4>  b0_0, float w_4)
{
    float c_2 = dot_1(a_6, b0_0);

#line 175
    float c_3;

#line 175
    Vector<float, 4>  b_5;
    if(c_2 < 0.0f)
    {
        Vector<float, 4>  _S39 = - b0_0;

#line 178
        c_3 = - c_2;

#line 178
        b_5 = _S39;

#line 176
    }
    else
    {

#line 176
        c_3 = c_2;

#line 176
        b_5 = b0_0;

#line 176
    }



    float _S40 = 1.0f - w_4;

#line 180
    float s0_0;

#line 180
    float s1_0;

    if((1.0f - c_3) > 0.00000999999974738f)
    {

#line 183
        float om_0 = (F32_acos((c_3)));
        float si_0 = (F32_sin((om_0)));

        float _S41 = (F32_sin((w_4 * om_0))) / si_0;

#line 186
        s0_0 = (F32_sin((_S40 * om_0))) / si_0;

#line 186
        s1_0 = _S41;

#line 182
    }
    else
    {

#line 182
        s0_0 = _S40;

#line 182
        s1_0 = w_4;

#line 182
    }

#line 188
    Vector<float, 4>  r_2 = a_6 * (Vector<float, 4> )s0_0 + b_5 * (Vector<float, 4> )s1_0;
    return r_2 / (Vector<float, 4> )(F32_max((length_2(r_2)), (9.99999968265522539e-21f)));
}


#line 152
static Vector<float, 4>  bk_qinv_0(Vector<float, 4>  a_7)
{

#line 153
    return Vector<float, 4> (- a_7.x, - a_7.y, - a_7.z, a_7.w);
}


#line 148
static Vector<float, 4>  bk_qmul_0(Vector<float, 4>  a_8, Vector<float, 4>  b_6)
{

#line 149
    float _S42 = a_8.w;

#line 149
    float _S43 = b_6.x;

#line 149
    float _S44 = a_8.x;

#line 149
    float _S45 = b_6.w;

#line 149
    float _S46 = a_8.y;

#line 149
    float _S47 = b_6.z;

#line 149
    float _S48 = a_8.z;

#line 149
    float _S49 = b_6.y;

#line 149
    return Vector<float, 4> (_S42 * _S43 + _S44 * _S45 + _S46 * _S47 - _S48 * _S49, _S42 * _S49 + _S46 * _S45 + _S48 * _S43 - _S44 * _S47, _S42 * _S47 + _S48 * _S45 + _S44 * _S49 - _S46 * _S43, _S42 * _S45 - _S44 * _S43 - _S46 * _S49 - _S48 * _S47);
}


#line 156
static Vector<float, 3>  bk_qlog_0(Vector<float, 4>  a_9)
{

#line 157
    float _S50 = a_9.w;

#line 157
    if((F32_abs((_S50))) > 0.99998998641967773f)
    {

#line 158
        return Vector<float, 3> {a_9.x, a_9.y, a_9.z} * (Vector<float, 3> )2.0f;
    }

    return Vector<float, 3> {a_9.x, a_9.y, a_9.z} * (Vector<float, 3> )(1.0f / (F32_sqrt((1.0f - _S50 * _S50)))) * (Vector<float, 3> )(2.0f * (F32_acos((_S50))));
}

static Vector<float, 4>  bk_qexp_0(Vector<float, 3>  e_4)
{

#line 165
    float th_0 = length_1(e_4);
    if(th_0 < 0.00000999999974738f)
    {

#line 167
        return Vector<float, 4> (0.0f, 0.0f, 0.0f, 1.0f);
    }

    float _S51 = th_0 * 0.5f;

#line 170
    return Vector<float, 4> ((e_4 / (Vector<float, 3> )th_0 * (Vector<float, 3> )(F32_sin((_S51)))).x, (e_4 / (Vector<float, 3> )th_0 * (Vector<float, 3> )(F32_sin((_S51)))).y, (e_4 / (Vector<float, 3> )th_0 * (Vector<float, 3> )(F32_sin((_S51)))).z, (F32_cos((_S51))));
}


#line 207
static Vector<float, 4>  bk_blend_log_0(Vector<float, 4>  fq_0, Vector<float, 3>  fromLn_0, Vector<float, 3>  toLn0_0, Vector<float, 3>  tif_0, float w_5)
{
    float th_1 = length_1(tif_0);
    Vector<float, 3>  cr_0 = cross_0(tif_0, toLn0_0);
    Vector<float, 3>  toLn_0 = toLn0_0 + tif_0 + cr_0 * (Vector<float, 3> )0.5f;

#line 211
    Vector<float, 3>  toLn_1;
    if(th_1 >= 0.00000999999974738f)
    {

#line 213
        float c_4 = 1.0f / (th_1 * th_1);

#line 213
        float c_5;
        if((F32_abs((th_1 - 3.14159274101257324f))) >= (F32_max((0.00000999999974738f * (F32_abs((th_1)))), (0.00000999999974738f))))
        {

#line 214
            c_5 = c_4 - (1.0f + (F32_cos((th_1)))) / (2.0f * th_1 * (F32_sin((th_1))));

#line 214
        }
        else
        {

#line 214
            c_5 = c_4;

#line 214
        }

#line 214
        toLn_1 = toLn_0 + cross_0(tif_0, cr_0) * (Vector<float, 3> )c_5;

#line 212
    }
    else
    {

#line 212
        toLn_1 = toLn_0;

#line 212
    }

#line 220
    return bk_qmul_0(fq_0, bk_qexp_0(fromLn_0 + (toLn_1 - fromLn_0) * (Vector<float, 3> )w_5));
}

static Vector<float, 4>  bk_sq_cubic_0(Vector<float, 4>  a_10, Vector<float, 4>  b0_1, Vector<float, 4>  pa0_0, Vector<float, 4>  pb0_0, float w_6, Vector<float, 2>  toT_4, Vector<float, 2>  preT_4, Vector<float, 2>  postT_4)
{

#line 223
    Vector<float, 4>  pa_0;

#line 228
    if((dot_1(a_10, pa0_0)) < 0.0f)
    {

#line 228
        pa_0 = - pa0_0;

#line 228
    }
    else
    {

#line 228
        pa_0 = pa0_0;

#line 228
    }


    bool f_b_0 = (dot_1(a_10, b0_1)) < 0.0f;

#line 231
    Vector<float, 4>  b_7;
    if(f_b_0)
    {

#line 232
        b_7 = - b0_1;

#line 232
    }
    else
    {

#line 232
        b_7 = b0_1;

#line 232
    }

#line 232
    bool f_pb_0;


    if(f_b_0)
    {

#line 235
        f_pb_0 = (dot_1(b_7, pb0_0)) <= 0.0f;

#line 235
    }
    else
    {

#line 235
        f_pb_0 = (dot_1(b_7, pb0_0)) < 0.0f;

#line 235
    }

#line 235
    Vector<float, 4>  pb_0;
    if(f_pb_0)
    {

#line 236
        pb_0 = - pb0_0;

#line 236
    }
    else
    {

#line 236
        pb_0 = pb0_0;

#line 236
    }



    Vector<float, 4>  _S52 = bk_qinv_0(a_10);

#line 240
    Vector<float, 3>  aTo_0 = bk_qlog_0(bk_qmul_0(_S52, b_7));
    Vector<float, 3>  aPre_0 = bk_qlog_0(bk_qmul_0(_S52, pa_0));
    Vector<float, 3>  aPost_0 = bk_qlog_0(bk_qmul_0(_S52, pb_0));

    Vector<float, 4>  _S53 = bk_qinv_0(b_7);

#line 244
    Vector<float, 3>  bFrom_0 = bk_qlog_0(bk_qmul_0(_S53, a_10));
    Vector<float, 3>  bPre_0 = bk_qlog_0(bk_qmul_0(_S53, pa_0));
    Vector<float, 3>  bPost_0 = bk_qlog_0(bk_qmul_0(_S53, pb_0));

    return bk_blend_log_0(a_10, Vector<float, 3> (bk_cubic_0(0.0f, aTo_0.x, aPre_0.x, aPost_0.x, w_6, toT_4, preT_4, postT_4), bk_cubic_0(0.0f, aTo_0.y, aPre_0.y, aPost_0.y, w_6, toT_4, preT_4, postT_4), bk_cubic_0(0.0f, aTo_0.z, aPre_0.z, aPost_0.z, w_6, toT_4, preT_4, postT_4)), Vector<float, 3> (bk_cubic_0(bFrom_0.x, 0.0f, bPre_0.x, bPost_0.x, w_6, toT_4, preT_4, postT_4), bk_cubic_0(bFrom_0.y, 0.0f, bPre_0.y, bPost_0.y, w_6, toT_4, preT_4, postT_4), bk_cubic_0(bFrom_0.z, 0.0f, bPre_0.z, bPost_0.z, w_6, toT_4, preT_4, postT_4)), aTo_0, w_6);
}


#line 192
static Vector<float, 3>  bk_nearest_log_0(Vector<float, 4>  a_11, Vector<float, 3>  nb_0)
{
    Vector<float, 3>  ax_0 = Vector<float, 3> {a_11.x, a_11.y, a_11.z};
    float _S54 = length_1(ax_0);

#line 195
    float ang_0 = 2.0f * (F32_atan2((_S54), (a_11.w)));

#line 195
    bool axZero_0;
    if((F32_abs((ax_0.x))) < 0.00000999999974738f)
    {

#line 196
        axZero_0 = (F32_abs((ax_0.y))) < 0.00000999999974738f;

#line 196
    }
    else
    {

#line 196
        axZero_0 = false;

#line 196
    }

#line 196
    if(axZero_0)
    {

#line 196
        axZero_0 = (F32_abs((ax_0.z))) < 0.00000999999974738f;

#line 196
    }
    else
    {

#line 196
        axZero_0 = false;

#line 196
    }
    float nl_0 = length_1(nb_0);

#line 197
    Vector<float, 3>  ax_1;
    if(axZero_0)
    {

#line 199
        if(nl_0 == 0.0f)
        {

#line 199
            ax_1 = nb_0;

#line 199
        }
        else
        {

#line 199
            ax_1 = nb_0 / (Vector<float, 3> )nl_0;

#line 199
        }

#line 198
    }
    else
    {

#line 198
        ax_1 = ax_0 / (Vector<float, 3> )_S54;

#line 198
    }

#line 204
    return ax_1 * (Vector<float, 3> )(ang_0 + (F32_round(((dot_0(nb_0, ax_1) - ang_0) / 12.56637096405029297f))) * 12.56637096405029297f);
}


#line 251
static Vector<float, 4>  bk_sq_makima_0(Vector<float, 4>  a_12, Vector<float, 4>  b0_2, Vector<float, 4>  pa0_1, Vector<float, 4>  pb0_1, Vector<float, 4>  pp0_0, Vector<float, 4>  qq0_0, float w_7, Vector<float, 2>  toT_5, Vector<float, 2>  preT_5, Vector<float, 2>  postT_5, Vector<float, 2>  ppT_2, Vector<float, 2>  qqT_2)
{

#line 257
    bool f_pa_0 = (dot_1(a_12, pa0_1)) < 0.0f;

#line 257
    Vector<float, 4>  pa_1;
    if(f_pa_0)
    {

#line 258
        pa_1 = - pa0_1;

#line 258
    }
    else
    {

#line 258
        pa_1 = pa0_1;

#line 258
    }

#line 258
    bool f_pp_0;


    if(f_pa_0)
    {

#line 261
        f_pp_0 = (dot_1(pa_1, pp0_0)) <= 0.0f;

#line 261
    }
    else
    {

#line 261
        f_pp_0 = (dot_1(pa_1, pp0_0)) < 0.0f;

#line 261
    }

#line 261
    Vector<float, 4>  pp_2;
    if(f_pp_0)
    {

#line 262
        pp_2 = - pp0_0;

#line 262
    }
    else
    {

#line 262
        pp_2 = pp0_0;

#line 262
    }


    bool f_b_1 = (dot_1(a_12, b0_2)) < 0.0f;

#line 265
    Vector<float, 4>  b_8;
    if(f_b_1)
    {

#line 266
        b_8 = - b0_2;

#line 266
    }
    else
    {

#line 266
        b_8 = b0_2;

#line 266
    }

#line 266
    bool f_pb_1;


    if(f_b_1)
    {

#line 269
        f_pb_1 = (dot_1(b_8, pb0_1)) <= 0.0f;

#line 269
    }
    else
    {

#line 269
        f_pb_1 = (dot_1(b_8, pb0_1)) < 0.0f;

#line 269
    }

#line 269
    Vector<float, 4>  pb_1;
    if(f_pb_1)
    {

#line 270
        pb_1 = - pb0_1;

#line 270
    }
    else
    {

#line 270
        pb_1 = pb0_1;

#line 270
    }

#line 270
    bool f_qq_0;


    if(f_pb_1)
    {

#line 273
        f_qq_0 = (dot_1(pb_1, qq0_0)) <= 0.0f;

#line 273
    }
    else
    {

#line 273
        f_qq_0 = (dot_1(pb_1, qq0_0)) < 0.0f;

#line 273
    }

#line 273
    Vector<float, 4>  qq_2;
    if(f_qq_0)
    {

#line 274
        qq_2 = - qq0_0;

#line 274
    }
    else
    {

#line 274
        qq_2 = qq0_0;

#line 274
    }



    Vector<float, 4>  _S55 = bk_qinv_0(a_12);

#line 278
    Vector<float, 3>  aTo_1 = bk_qlog_0(bk_qmul_0(_S55, b_8));
    Vector<float, 3>  aPre_1 = bk_qlog_0(bk_qmul_0(_S55, pa_1));
    Vector<float, 3>  aPost_1 = bk_qlog_0(bk_qmul_0(_S55, pb_1));
    Vector<float, 3>  aPp_0 = bk_nearest_log_0(bk_qmul_0(_S55, pp_2), aPre_1);
    Vector<float, 3>  aQq_0 = bk_nearest_log_0(bk_qmul_0(_S55, qq_2), aPost_1);

    Vector<float, 4>  _S56 = bk_qinv_0(b_8);

#line 284
    Vector<float, 3>  bFrom_1 = bk_qlog_0(bk_qmul_0(_S56, a_12));
    Vector<float, 3>  bPre_1 = bk_qlog_0(bk_qmul_0(_S56, pa_1));
    Vector<float, 3>  bPost_1 = bk_qlog_0(bk_qmul_0(_S56, pb_1));
    Vector<float, 3>  bPp_0 = bk_nearest_log_0(bk_qmul_0(_S56, pp_2), bPre_1);
    Vector<float, 3>  bQq_0 = bk_nearest_log_0(bk_qmul_0(_S56, qq_2), bPost_1);

    return bk_blend_log_0(a_12, Vector<float, 3> (bk_makima_0(0.0f, aTo_1.x, aPre_1.x, aPost_1.x, aPp_0.x, aQq_0.x, w_7, toT_5, preT_5, postT_5, ppT_2, qqT_2), bk_makima_0(0.0f, aTo_1.y, aPre_1.y, aPost_1.y, aPp_0.y, aQq_0.y, w_7, toT_5, preT_5, postT_5, ppT_2, qqT_2), bk_makima_0(0.0f, aTo_1.z, aPre_1.z, aPost_1.z, aPp_0.z, aQq_0.z, w_7, toT_5, preT_5, postT_5, ppT_2, qqT_2)), Vector<float, 3> (bk_makima_0(bFrom_1.x, 0.0f, bPre_1.x, bPost_1.x, bPp_0.x, bQq_0.x, w_7, toT_5, preT_5, postT_5, ppT_2, qqT_2), bk_makima_0(bFrom_1.y, 0.0f, bPre_1.y, bPost_1.y, bPp_0.y, bQq_0.y, w_7, toT_5, preT_5, postT_5, ppT_2, qqT_2), bk_makima_0(bFrom_1.z, 0.0f, bPre_1.z, bPost_1.z, bPp_0.z, bQq_0.z, w_7, toT_5, preT_5, postT_5, ppT_2, qqT_2)), aTo_1, w_7);
}


#line 354
static Vector<float, 4>  bk_sample_0(uint32_t i_2, Vector<float, 2>  tdf_0, Vector<float, 2>  Ldf_0, uint32_t loopMode_1, uint32_t backward_1, KernelContext_0 * kernelContext_1)
{

#line 354
    int32_t low_0;

#line 354
    int32_t high_0;
    uint32_t _S57 = i_2 * 2U;

#line 355
    uint32_t off_1 = kernelContext_1->globalParams_0->trackKeys_0.Load(_S57);
    int32_t len_2 = int32_t(kernelContext_1->globalParams_0->trackKeys_0.Load(_S57 + 1U));
    uint32_t flags_0 = kernelContext_1->globalParams_0->trackKind_0.Load(i_2);
    uint32_t kind_0 = flags_0 & 1U;
    uint32_t interp_0 = (flags_0 >> 1U) & 7U;

#line 359
    bool wrap_2;
    if(((flags_0 >> 4U) & 1U) != 0U)
    {

#line 360
        wrap_2 = loopMode_1 != 0U;

#line 360
    }
    else
    {

#line 360
        wrap_2 = false;

#line 360
    }

#line 360
    bool back_0;
    if(backward_1 != 0U)
    {

#line 361
        back_0 = interp_0 == 0U;

#line 361
    }
    else
    {

#line 361
        back_0 = false;

#line 361
    }
    float t_1 = tdf_0.x;

    uint32_t _S58 = off_1 * 4U;

#line 364
    Vector<float, 4>  vk0_0 = Vector<float, 4> (kernelContext_1->globalParams_0->keyVal_0.Load(_S58), kernelContext_1->globalParams_0->keyVal_0.Load(_S58 + 1U), kernelContext_1->globalParams_0->keyVal_0.Load(_S58 + 2U), kernelContext_1->globalParams_0->keyVal_0.Load(_S58 + 3U));
    if(len_2 == int(1))
    {

#line 366
        return vk0_0;
    }

    int32_t _S59 = len_2 - int(1);

#line 369
    int32_t low_1 = int(0);

#line 369
    int32_t high_1 = _S59;

#line 369
    bool found_0 = false;

#line 369
    int32_t mid_0 = int(0);


    for(;;)
    {

#line 372
        bool _S60;

#line 372
        if(low_1 <= high_1)
        {

#line 372
            _S60 = !found_0;

#line 372
        }
        else
        {

#line 372
            _S60 = false;

#line 372
        }

#line 372
        if(_S60)
        {
        }
        else
        {

#line 372
            break;
        }

#line 373
        int32_t mid_1 = (low_1 + high_1) / int(2);
        float km_0 = kernelContext_1->globalParams_0->keyTime_0.Load(off_1 + uint32_t(mid_1));
        float _S61 = (F32_max((0.00000999999974738f * (F32_abs((t_1)))), (0.00000999999974738f)));

#line 375
        bool _S62;
        if(t_1 == km_0)
        {

#line 376
            _S62 = true;

#line 376
        }
        else
        {

#line 376
            _S62 = (F32_abs((t_1 - km_0))) < _S61;

#line 376
        }

#line 376
        if(_S62)
        {

#line 376
            found_0 = true;

#line 376
        }
        else
        {
            if(t_1 < km_0)
            {

#line 380
                int32_t _S63 = mid_1 - int(1);

#line 380
                low_0 = low_1;

#line 380
                high_0 = _S63;

#line 379
            }
            else
            {

#line 379
                low_0 = mid_1 + int(1);

#line 379
                high_0 = high_1;

#line 379
            }

#line 379
            low_1 = low_0;

#line 379
            high_1 = high_0;

#line 376
        }

#line 376
        mid_0 = mid_1;

#line 372
    }

#line 386
    if(!found_0)
    {

#line 387
        if(!back_0)
        {

#line 388
            if((kernelContext_1->globalParams_0->keyTime_0).Load(off_1 + uint32_t(mid_0)) > t_1)
            {

#line 388
                mid_0 = mid_0 - int(1);

#line 388
            }

#line 387
        }
        else
        {


            if((kernelContext_1->globalParams_0->keyTime_0).Load(off_1 + uint32_t(mid_0)) < t_1)
            {

#line 392
                mid_0 = mid_0 + int(1);

#line 392
            }

#line 387
        }

#line 386
    }

#line 397
    if(back_0)
    {

#line 397
        low_1 = (I32_min((mid_0), (len_2)));

#line 397
    }
    else
    {

#line 397
        low_1 = (I32_min((mid_0), (_S59)));

#line 397
    }
    bool _S64 = ((flags_0 >> 6U) & 1U) != 0U;

#line 398
    int32_t base_0;

#line 398
    Vector<float, 2>  _S65;

#line 398
    if(_S64)
    {

#line 399
        if(t_1 < 0.0f)
        {

#line 399
            _S65 = Vector<float, 2> (0.0f, 0.0f);

#line 399
        }
        else
        {

#line 399
            _S65 = tdf_0;

#line 399
        }
        Vector<float, 2>  _S66 = bk_df_sub_0(_S65, Ldf_0);

#line 400
        if((_S66.x + _S66.y) > 0.0f)
        {

#line 400
            _S65 = Ldf_0;

#line 400
        }

#line 400
        low_0 = int(0);

#line 400
        high_0 = len_2;


        for(;;)
        {

#line 403
            if(low_0 < high_0)
            {
            }
            else
            {

#line 403
                break;
            }

#line 404
            int32_t cm_0 = (low_0 + high_0) / int(2);
            uint32_t _S67 = off_1 + uint32_t(cm_0);

#line 405
            if((bk_df_sub_0(Vector<float, 2> (kernelContext_1->globalParams_0->keyTime_0.Load(_S67), kernelContext_1->globalParams_0->keyTimeLo_0.Load(_S67)), _S65).x + bk_df_sub_0(Vector<float, 2> (kernelContext_1->globalParams_0->keyTime_0.Load(_S67), kernelContext_1->globalParams_0->keyTimeLo_0.Load(_S67)), _S65).y) <= 0.0f)
            {

#line 405
                low_0 = cm_0 + int(1);

#line 405
            }
            else
            {

#line 405
                high_0 = cm_0;

#line 405
            }

#line 403
        }

#line 411
        int32_t base_1 = low_0 - int(1);
        if(base_1 >= int(0))
        {

#line 413
            int32_t ps_0 = base_1 - int32_t(kernelContext_1->globalParams_0->keyTrans_0.Load(off_1 + uint32_t(base_1)));
            uint32_t _S68 = off_1 + uint32_t(ps_0);

#line 414
            if((bk_df_sub_0(Vector<float, 2> (kernelContext_1->globalParams_0->keyTime_0.Load(_S68), kernelContext_1->globalParams_0->keyTimeLo_0.Load(_S68)), _S65).x + bk_df_sub_0(Vector<float, 2> (kernelContext_1->globalParams_0->keyTime_0.Load(_S68), kernelContext_1->globalParams_0->keyTimeLo_0.Load(_S68)), _S65).y) == 0.0f)
            {

#line 414
                base_0 = ps_0;

#line 414
            }
            else
            {

#line 414
                base_0 = base_1;

#line 414
            }

#line 412
        }
        else
        {

#line 412
            base_0 = base_1;

#line 412
        }

#line 398
    }
    else
    {

#line 398
        base_0 = low_1;

#line 398
        _S65 = tdf_0;

#line 398
    }

#line 419
    if(back_0)
    {

#line 419
        low_0 = base_0 - int(-2);

#line 419
    }
    else
    {

#line 419
        low_0 = base_0 + int(-2);

#line 419
    }
    if(back_0)
    {

#line 420
        high_0 = base_0 - int(-1);

#line 420
    }
    else
    {

#line 420
        high_0 = base_0 + int(-1);

#line 420
    }

#line 420
    int32_t pn1_0;

    if(back_0)
    {

#line 422
        pn1_0 = base_0 - int(1);

#line 422
    }
    else
    {

#line 422
        pn1_0 = base_0 + int(1);

#line 422
    }

#line 422
    int32_t pn2_0;
    if(back_0)
    {

#line 423
        pn2_0 = base_0 - int(2);

#line 423
    }
    else
    {

#line 423
        pn2_0 = base_0 + int(2);

#line 423
    }

#line 423
    int32_t pn3_0;
    if(back_0)
    {

#line 424
        pn3_0 = base_0 - int(3);

#line 424
    }
    else
    {

#line 424
        pn3_0 = base_0 + int(3);

#line 424
    }
    int32_t km2_0 = bk_fetch_idx_0(low_0, len_2, wrap_2, loopMode_1);
    int32_t km1_0 = bk_fetch_idx_0(high_0, len_2, wrap_2, loopMode_1);
    int32_t kn0_0 = bk_fetch_idx_0(base_0, len_2, wrap_2, loopMode_1);
    int32_t kn1_0 = bk_fetch_idx_0(pn1_0, len_2, wrap_2, loopMode_1);
    int32_t kn2_0 = bk_fetch_idx_0(pn2_0, len_2, wrap_2, loopMode_1);
    int32_t kn3_0 = bk_fetch_idx_0(pn3_0, len_2, wrap_2, loopMode_1);

#line 430
    Vector<float, 2>  _S69 = bk_fetch_time_0(low_0, len_2, wrap_2, loopMode_1, off_1, Ldf_0, kernelContext_1);

#line 430
    Vector<float, 2>  _S70 = bk_fetch_time_0(high_0, len_2, wrap_2, loopMode_1, off_1, Ldf_0, kernelContext_1);

#line 430
    Vector<float, 2>  _S71 = bk_fetch_time_0(base_0, len_2, wrap_2, loopMode_1, off_1, Ldf_0, kernelContext_1);

#line 430
    Vector<float, 2>  _S72 = bk_fetch_time_0(pn1_0, len_2, wrap_2, loopMode_1, off_1, Ldf_0, kernelContext_1);

#line 430
    Vector<float, 2>  _S73 = bk_fetch_time_0(pn2_0, len_2, wrap_2, loopMode_1, off_1, Ldf_0, kernelContext_1);

#line 430
    Vector<float, 2>  _S74 = bk_fetch_time_0(pn3_0, len_2, wrap_2, loopMode_1, off_1, Ldf_0, kernelContext_1);

#line 437
    uint32_t _S75 = (off_1 + uint32_t(km2_0)) * 4U;

#line 437
    float _S76 = kernelContext_1->globalParams_0->keyVal_0.Load(_S75);

#line 437
    float _S77 = kernelContext_1->globalParams_0->keyVal_0.Load(_S75 + 1U);

#line 437
    float _S78 = kernelContext_1->globalParams_0->keyVal_0.Load(_S75 + 2U);

#line 437
    Vector<float, 4>  vm2_0 = Vector<float, 4> (_S76, _S77, _S78, kernelContext_1->globalParams_0->keyVal_0.Load(_S75 + 3U));
    uint32_t _S79 = (off_1 + uint32_t(km1_0)) * 4U;

#line 438
    float _S80 = kernelContext_1->globalParams_0->keyVal_0.Load(_S79);

#line 438
    float _S81 = kernelContext_1->globalParams_0->keyVal_0.Load(_S79 + 1U);

#line 438
    float _S82 = kernelContext_1->globalParams_0->keyVal_0.Load(_S79 + 2U);

#line 438
    Vector<float, 4>  vm1_0 = Vector<float, 4> (_S80, _S81, _S82, kernelContext_1->globalParams_0->keyVal_0.Load(_S79 + 3U));
    uint32_t _S83 = off_1 + uint32_t(kn0_0);

#line 439
    uint32_t _S84 = _S83 * 4U;

#line 439
    float _S85 = kernelContext_1->globalParams_0->keyVal_0.Load(_S84);

#line 439
    float _S86 = kernelContext_1->globalParams_0->keyVal_0.Load(_S84 + 1U);

#line 439
    float _S87 = kernelContext_1->globalParams_0->keyVal_0.Load(_S84 + 2U);

#line 439
    Vector<float, 4>  vn0_0 = Vector<float, 4> (_S85, _S86, _S87, kernelContext_1->globalParams_0->keyVal_0.Load(_S84 + 3U));
    uint32_t _S88 = (off_1 + uint32_t(kn1_0)) * 4U;

#line 440
    float _S89 = kernelContext_1->globalParams_0->keyVal_0.Load(_S88);

#line 440
    float _S90 = kernelContext_1->globalParams_0->keyVal_0.Load(_S88 + 1U);

#line 440
    float _S91 = kernelContext_1->globalParams_0->keyVal_0.Load(_S88 + 2U);

#line 440
    Vector<float, 4>  vn1_0 = Vector<float, 4> (_S89, _S90, _S91, kernelContext_1->globalParams_0->keyVal_0.Load(_S88 + 3U));
    uint32_t _S92 = (off_1 + uint32_t(kn2_0)) * 4U;

#line 441
    float _S93 = kernelContext_1->globalParams_0->keyVal_0.Load(_S92);

#line 441
    float _S94 = kernelContext_1->globalParams_0->keyVal_0.Load(_S92 + 1U);

#line 441
    float _S95 = kernelContext_1->globalParams_0->keyVal_0.Load(_S92 + 2U);

#line 441
    Vector<float, 4>  vn2_0 = Vector<float, 4> (_S93, _S94, _S95, kernelContext_1->globalParams_0->keyVal_0.Load(_S92 + 3U));
    uint32_t _S96 = (off_1 + uint32_t(kn3_0)) * 4U;

#line 442
    float _S97 = kernelContext_1->globalParams_0->keyVal_0.Load(_S96);

#line 442
    float _S98 = kernelContext_1->globalParams_0->keyVal_0.Load(_S96 + 1U);

#line 442
    float _S99 = kernelContext_1->globalParams_0->keyVal_0.Load(_S96 + 2U);

#line 442
    Vector<float, 4>  vn3_0 = Vector<float, 4> (_S97, _S98, _S99, kernelContext_1->globalParams_0->keyVal_0.Load(_S96 + 3U));

#line 442
    float tr_2;
    if(_S64)
    {

#line 443
        tr_2 = 1.0f;

#line 443
    }
    else
    {

#line 443
        tr_2 = kernelContext_1->globalParams_0->keyTrans_0.Load(_S83);

#line 443
    }
    if(tr_2 == 0.0f)
    {

#line 445
        return vn0_0;
    }
    Vector<float, 2>  _S100 = bk_df_sub_0(_S72, _S71);

#line 447
    float dlt_0 = _S100.x + _S100.y;

#line 447
    float w_8;
    if((F32_abs((dlt_0))) < 0.00000999999974738f)
    {

#line 448
        w_8 = 0.0f;

#line 448
    }
    else
    {

#line 448
        Vector<float, 2>  _S101 = bk_df_sub_0(_S65, _S71);

#line 448
        w_8 = (_S101.x + _S101.y) / dlt_0;

#line 448
    }
    if(tr_2 != 1.0f)
    {

#line 449
        w_8 = bk_ease_0(w_8, tr_2);

#line 449
    }


    if(interp_0 == 0U)
    {

#line 453
        return vn0_0;
    }
    bool scalar_0 = ((flags_0 >> 5U) & 1U) != 0U;
    if(scalar_0)
    {

#line 456
        wrap_2 = interp_0 == 3U;

#line 456
    }
    else
    {

#line 456
        wrap_2 = false;

#line 456
    }

#line 456
    if(wrap_2)
    {

#line 457
        return Vector<float, 4> (bk_fposmod_0(_S85 + bk_angle_diff_0(_S85, _S89) * w_8, 6.28318548202514648f), 0.0f, 0.0f, 0.0f);
    }
    if(scalar_0)
    {

#line 459
        wrap_2 = interp_0 == 4U;

#line 459
    }
    else
    {

#line 459
        wrap_2 = false;

#line 459
    }

#line 459
    if(wrap_2)
    {

#line 460
        return Vector<float, 4> (bk_fposmod_0(bk_cubic_angle_0(_S85, _S89, _S80, _S93, w_8, _S100, bk_df_sub_0(_S70, _S71), bk_df_sub_0(_S73, _S71)), 6.28318548202514648f), 0.0f, 0.0f, 0.0f);
    }
    if(scalar_0)
    {

#line 462
        wrap_2 = interp_0 == 6U;

#line 462
    }
    else
    {

#line 462
        wrap_2 = false;

#line 462
    }

#line 462
    if(wrap_2)
    {

#line 463
        return Vector<float, 4> (bk_fposmod_0(bk_makima_angle_0(_S85, _S89, _S80, _S93, _S76, _S97, w_8, _S100, bk_df_sub_0(_S70, _S71), bk_df_sub_0(_S73, _S71), bk_df_sub_0(_S69, _S71), bk_df_sub_0(_S74, _S71)), 6.28318548202514648f), 0.0f, 0.0f, 0.0f);
    }
    if(interp_0 == 1U)
    {

#line 465
        wrap_2 = true;

#line 465
    }
    else
    {

#line 465
        wrap_2 = interp_0 == 3U;

#line 465
    }

#line 465
    if(wrap_2)
    {

#line 466
        if(kind_0 == 0U)
        {

#line 467
            return vn0_0 + (vn1_0 - vn0_0) * (Vector<float, 4> )w_8;
        }
        else
        {

#line 469
            return bk_slerp_0(vn0_0, vn1_0, w_8);
        }

#line 469
    }


    if(interp_0 == 2U)
    {

#line 472
        wrap_2 = true;

#line 472
    }
    else
    {

#line 472
        wrap_2 = interp_0 == 4U;

#line 472
    }

#line 472
    if(wrap_2)
    {

#line 473
        if(kind_0 == 0U)
        {

#line 474
            Vector<float, 2>  _S102 = bk_df_sub_0(_S70, _S71);

#line 474
            Vector<float, 2>  _S103 = bk_df_sub_0(_S73, _S71);

#line 474
            return Vector<float, 4> (bk_cubic_0(_S85, _S89, _S80, _S93, w_8, _S100, _S102, _S103), bk_cubic_0(_S86, _S90, _S81, _S94, w_8, _S100, _S102, _S103), bk_cubic_0(_S87, _S91, _S82, _S95, w_8, _S100, _S102, _S103), 0.0f);
        }
        else
        {

#line 476
            return bk_sq_cubic_0(vn0_0, vn1_0, vm1_0, vn2_0, w_8, _S100, bk_df_sub_0(_S70, _S71), bk_df_sub_0(_S73, _S71));
        }

#line 476
    }


    if(kind_0 == 0U)
    {

#line 480
        Vector<float, 2>  _S104 = bk_df_sub_0(_S70, _S71);

#line 480
        Vector<float, 2>  _S105 = bk_df_sub_0(_S73, _S71);

#line 480
        Vector<float, 2>  _S106 = bk_df_sub_0(_S69, _S71);

#line 480
        Vector<float, 2>  _S107 = bk_df_sub_0(_S74, _S71);

#line 480
        return Vector<float, 4> (bk_makima_0(_S85, _S89, _S80, _S93, _S76, _S97, w_8, _S100, _S104, _S105, _S106, _S107), bk_makima_0(_S86, _S90, _S81, _S94, _S77, _S98, w_8, _S100, _S104, _S105, _S106, _S107), bk_makima_0(_S87, _S91, _S82, _S95, _S78, _S99, w_8, _S100, _S104, _S105, _S106, _S107), 0.0f);
    }
    else
    {

#line 482
        return bk_sq_makima_0(vn0_0, vn1_0, vm1_0, vn2_0, vm2_0, vn3_0, w_8, _S100, bk_df_sub_0(_S70, _S71), bk_df_sub_0(_S73, _S71), bk_df_sub_0(_S69, _S71), bk_df_sub_0(_S74, _S71));
    }

#line 482
}




void _main_0(void* _S108, void* entryPointParams_0, void* globalParams_1)
{

#line 487
    ComputeThreadVaryingInput * _S109 = (slang_bit_cast<ComputeThreadVaryingInput *>(_S108));

#line 487
    KernelContext_0 kernelContext_2;

#line 487
    (&kernelContext_2)->globalParams_0 = (slang_bit_cast<GlobalParams_0*>(globalParams_1));
    uint32_t i_3 = (_S109->groupID * Vector<uint32_t, 3> (64U, 1U, 1U) + _S109->groupThreadID).x;
    if(i_3 >= ((slang_bit_cast<GlobalParams_0*>(globalParams_1))->params_0->T_0))
    {

#line 490
        return;
    }
    uint32_t dst_0 = (&kernelContext_2)->globalParams_0->trackDst_0.Load(i_3);
    uint32_t flags_1 = (&kernelContext_2)->globalParams_0->trackKind_0.Load(i_3);

#line 493
    Vector<float, 4>  _S110 = bk_sample_0(i_3, Vector<float, 2> ((slang_bit_cast<GlobalParams_0*>(globalParams_1))->params_0->time_0, (slang_bit_cast<GlobalParams_0*>(globalParams_1))->params_0->timeLo_0), Vector<float, 2> ((slang_bit_cast<GlobalParams_0*>(globalParams_1))->params_0->length_0, (slang_bit_cast<GlobalParams_0*>(globalParams_1))->params_0->lengthLo_0), (slang_bit_cast<GlobalParams_0*>(globalParams_1))->params_0->loopMode_0, (slang_bit_cast<GlobalParams_0*>(globalParams_1))->params_0->backward_0, &kernelContext_2);

    if((flags_1 & 1U) == 0U)
    {

#line 496
        *(&((&kernelContext_2)->globalParams_0->localPose_0)[dst_0]) = _S110.x;
        *(&((&kernelContext_2)->globalParams_0->localPose_0)[dst_0 + 1U]) = _S110.y;
        *(&((&kernelContext_2)->globalParams_0->localPose_0)[dst_0 + 2U]) = _S110.z;

#line 495
    }
    else
    {


        *(&((&kernelContext_2)->globalParams_0->localPose_0)[dst_0]) = _S110.x;
        *(&((&kernelContext_2)->globalParams_0->localPose_0)[dst_0 + 1U]) = _S110.y;
        *(&((&kernelContext_2)->globalParams_0->localPose_0)[dst_0 + 2U]) = _S110.z;
        *(&((&kernelContext_2)->globalParams_0->localPose_0)[dst_0 + 3U]) = _S110.w;

#line 495
    }

#line 505
    return;
}

// [numthreads(64, 1, 1)]
SLANG_PRELUDE_EXPORT
void main_0_Thread(ComputeThreadVaryingInput* varyingInput, void* entryPointParams, void* globalParams)
{
    _main_0(varyingInput, entryPointParams, globalParams);
}
// [numthreads(64, 1, 1)]
SLANG_PRELUDE_EXPORT
void main_0_Group(ComputeVaryingInput* varyingInput, void* entryPointParams, void* globalParams)
{
    ComputeThreadVaryingInput threadInput = {};
    threadInput.groupID = varyingInput->startGroupID;
    for (uint32_t x = 0; x < 64; ++x)
    {
        threadInput.groupThreadID.x = x;
        _main_0(&threadInput, entryPointParams, globalParams);
    }
}
// [numthreads(64, 1, 1)]
SLANG_PRELUDE_EXPORT
void main_0(ComputeVaryingInput* varyingInput, void* entryPointParams, void* globalParams)
{
    ComputeVaryingInput vi = *varyingInput;
    ComputeVaryingInput groupVaryingInput = {};
    for (uint32_t z = vi.startGroupID.z; z < vi.endGroupID.z; ++z)
    {
        groupVaryingInput.startGroupID.z = z;
        for (uint32_t y = vi.startGroupID.y; y < vi.endGroupID.y; ++y)
        {
            groupVaryingInput.startGroupID.y = y;
            for (uint32_t x = vi.startGroupID.x; x < vi.endGroupID.x; ++x)
            {
                groupVaryingInput.startGroupID.x = x;
                main_0_Group(&groupVaryingInput, entryPointParams, globalParams);
            }
        }
    }
}
