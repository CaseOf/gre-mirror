/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CAIRO_COMPILER_PRIVATE_H
#define CAIRO_COMPILER_PRIVATE_H

#include "cairo.h"

#if HAVE_CONFIG_H
#include "config.h"
#endif

/* Size in bytes of buffer to use off the stack per functions.
 * Mostly used by text functions.  For larger allocations, they'll
 * malloc(). */
#ifndef CAIRO_STACK_BUFFER_SIZE
#define CAIRO_STACK_BUFFER_SIZE (512 * sizeof (int))
#endif

#define CAIRO_STACK_ARRAY_LENGTH(T) (CAIRO_STACK_BUFFER_SIZE / sizeof(T))

/*
 * The goal of this block is to define the following macros for
 * providing faster linkage to functions in the public API for calls
 * from within cairo.
 *
 * slim_hidden_proto(f)
 * slim_hidden_proto_no_warn(f)
 *
 *   Declares `f' as a library internal function and hides the
 *   function from the global symbol table.  This macro must be
 *   expanded after `f' has been declared with a prototype but before
 *   any calls to the function are seen by the compiler.  The no_warn
 *   variant inhibits warnings about the return value being unused at
 *   call sites.  The macro works by renaming `f' to an internal name
 *   in the symbol table and hiding that.  As far as cairo internal
 *   calls are concerned they're calling a library internal function
 *   and thus don't need to bounce via the PLT.
 *
 * slim_hidden_def(f)
 *
 *   Exports `f' back to the global symbol table.  This macro must be
 *   expanded right after the function definition and only for symbols
 *   hidden previously with slim_hidden_proto().  The macro works by
 *   adding a global entry to the symbol table which points at the
 *   internal name of `f' created by slim_hidden_proto().
 *
 * Functions in the public API which aren't called by the library
 * don't need to be hidden and re-exported using the slim hidden
 * macros.
 */
#if __GNUC__ >= 3 && defined(__ELF__) && !defined(__sun)
# define slim_hidden_proto(name)        slim_hidden_proto1(name, slim_hidden_int_name(name)) cairo_private
# define slim_hidden_proto_no_warn(name)    slim_hidden_proto1(name, slim_hidden_int_name(name)) cairo_private_no_warn
# define slim_hidden_def(name)            slim_hidden_def1(name, slim_hidden_int_name(name))
# define slim_hidden_int_name(name) INT_##name
# define slim_hidden_proto1(name, internal)                \
  extern __typeof (name) name                        \
    __asm__ (slim_hidden_asmname (internal))
# define slim_hidden_def1(name, internal)                \
  extern __typeof (name) EXT_##name __asm__(slim_hidden_asmname(name))    \
    __attribute__((__alias__(slim_hidden_asmname(internal))))
# define slim_hidden_ulp        slim_hidden_ulp1(__USER_LABEL_PREFIX__)
# define slim_hidden_ulp1(x)        slim_hidden_ulp2(x)
# define slim_hidden_ulp2(x)        #x
# define slim_hidden_asmname(name)    slim_hidden_asmname1(name)
# define slim_hidden_asmname1(name)    slim_hidden_ulp #name
#else
# define slim_hidden_proto(name)        int _cairo_dummy_prototype(void)
# define slim_hidden_proto_no_warn(name)    int _cairo_dummy_prototype(void)
# define slim_hidden_def(name)            int _cairo_dummy_prototype(void)
#endif

#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ > 4)
#define CAIRO_PRINTF_FORMAT(fmt_index, va_index) \
    __attribute__((__format__(__printf__, fmt_index, va_index)))
#else
#define CAIRO_PRINTF_FORMAT(fmt_index, va_index)
#endif

/* slim_internal.h */
#define CAIRO_HAS_HIDDEN_SYMBOLS 1
#if (__GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 3)) && defined(__ELF__) && !defined(__sun)
#define cairo_private_no_warn    __attribute__((__visibility__("hidden")))
#elif defined(__SUNPRO_C) && (__SUNPRO_C >= 0x550)
#define cairo_private_no_warn    __hidden
#else /* not gcc >= 3.3 and not Sun Studio >= 8 */
#define cairo_private_no_warn
#undef CAIRO_HAS_HIDDEN_SYMBOLS
#endif

#ifndef WARN_UNUSED_RESULT
#define WARN_UNUSED_RESULT
#endif
/* Add attribute(warn_unused_result) if supported */
#define cairo_warn      WARN_UNUSED_RESULT
#define cairo_private      cairo_private_no_warn cairo_warn

/* This macro allow us to deprecate a function by providing an alias
   for the old function name to the new function name. With this
   macro, binary compatibility is preserved. The macro only works on
   some platforms --- tough.

   Meanwhile, new definitions in the public header file break the
   source code so that it will no longer link against the old
   symbols. Instead it will give a descriptive error message
   indicating that the old function has been deprecated by the new
   function.
*/
#if __GNUC__ >= 2 && defined(__ELF__)
# define CAIRO_FUNCTION_ALIAS(old, new)        \
    extern __typeof (new) old        \
    __asm__ ("" #old)            \
    __attribute__((__alias__("" #new)))
#else
# define CAIRO_FUNCTION_ALIAS(old, new)
#endif

/*
 * Cairo uses the following function attributes in order to improve the
 * generated code (effectively by manual inter-procedural analysis).
 *
 *   'cairo_pure': The function is only allowed to read from its arguments
 *         and global memory (i.e. following a pointer argument or
 *         accessing a shared variable). The return value should
 *         only depend on its arguments, and for an identical set of
 *         arguments should return the same value.
 *
 *   'cairo_const': The function is only allowed to read from its arguments.
 *          It is not allowed to access global memory. The return
 *          value should only depend its arguments, and for an
 *          identical set of arguments should return the same value.
 *          This is currently the most strict function attribute.
 *
 * Both these function attributes allow gcc to perform CSE and
 * constant-folding, with 'cairo_const 'also guaranteeing that pointer contents
 * do not change across the function call.
 */
#if __GNUC__ >= 3
#define cairo_pure __attribute__((pure))
#define cairo_const __attribute__((const))
#define cairo_always_inline inline __attribute__((always_inline))
#else
#define cairo_pure
#define cairo_const
#define cairo_always_inline inline
#endif

#if defined(__GNUC__) && (__GNUC__ > 2) && defined(__OPTIMIZE__)
#define _CAIRO_BOOLEAN_EXPR(expr)           \
 __extension__ ({                 \
   int _cairo_boolean_var_;             \
   if (expr)                  \
    _cairo_boolean_var_ = 1;            \
   else                     \
    _cairo_boolean_var_ = 0;            \
   _cairo_boolean_var_;               \
})
#define likely(expr) (__builtin_expect (_CAIRO_BOOLEAN_EXPR(expr), 1))
#define unlikely(expr) (__builtin_expect (_CAIRO_BOOLEAN_EXPR(expr), 0))
#else
#define likely(expr) (expr)
#define unlikely(expr) (expr)
#endif

/*
 * clang-cl supports __attribute__, but MSVC doesn't, so we need to make sure
 * we do this if not GNUC but also if not clang either.
 */
#if !defined(__GNUC__) && !defined(__clang__)
#undef __attribute__
#define __attribute__(x)
#endif

#if (defined(__WIN32__) && !defined(__WINE__)) || defined(_MSC_VER)
#define snprintf _snprintf
#define popen _popen
#define pclose _pclose
#define hypot _hypot
#endif

#ifdef _MSC_VER

#define HAVE_WIN32_ATOMIC_PRIMITIVES 1

#ifndef __cplusplus
#undef inline
#define inline __inline
#endif

/* there are currently linkage problems that arise when trying to include intrin.h in c++:
 * D:\sdks\v7.0\include\winnt.h(3674) : error C2733: second C linkage of overloaded function '_interlockedbittestandset' not allowed
 * so avoid defining ffs in c++ code for now */
#ifndef  __cplusplus
/* Add a definition of ffs */
#include <intrin.h>
#pragma intrinsic(_BitScanForward)
static __forceinline int
ffs (int x)
{
  unsigned long i;

  if (_BitScanForward(&i, x) != 0)
    return i + 1;

  return 0;
}
#endif

#elif defined(__WIN32__) && defined(__GNUC__)

#define ffs(x) __builtin_ffs(x)

#endif

#if defined(_MSC_VER) && defined(_M_IX86)
/* When compiling with /Gy and /OPT:ICF identical functions will be folded in together.
   The CAIRO_ENSURE_UNIQUE macro ensures that a function is always unique and
   will never be folded into another one. Something like this might eventually
   be needed for GCC but it seems fine for now. */
#define CAIRO_ENSURE_UNIQUE             \
  do {                      \
    char file[] = __FILE__;           \
    __asm {                   \
      __asm jmp __internal_skip_line_no   \
      __asm _emit (__COUNTER__ & 0xff)    \
      __asm _emit ((__COUNTER__>>8) & 0xff) \
      __asm _emit ((__COUNTER__>>16) & 0xff)\
      __asm _emit ((__COUNTER__>>24) & 0xff)\
      __asm lea eax, dword ptr file     \
      __asm __internal_skip_line_no:    \
    };                    \
  } while (0)
#else
#define CAIRO_ENSURE_UNIQUE  do { } while (0)
#endif

#ifdef __STRICT_ANSI__
#undef inline
#define inline __inline__
#endif

#endif
