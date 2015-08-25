#pragma once
#ifndef KERNEL_BASE_HEADER_HPP
#define KERNEL_BASE_HEADER_HPP 1

/**
 * \file
 * \brief FEAST Kernel base header.
 *
 * This file is the base header for the FEAST kernel, which is included by all other FEAST header and source files.
 * It defines macros and data types which are frequently used in other files.
 */

// Include FEAST configuration header.
#ifndef FEAST_NO_CONFIG
#  include <feast_config.hpp>
#endif

// Make sure the DOXYGEN macro is not defined at compile-time;
// it is reserved for doxygen's preprocessor.
#ifdef DOXYGEN
#  error The DOXYGEN macro must not be defined at compile-time
#else
#  define DOXY(x)
#endif

/// \cond nodoxy
// Activate DEBUG macro if the build system tells us to do so.
#if defined(FEAST_DEBUG_MODE) && !defined(DEBUG)
#  define DEBUG 1
#endif

// Activate SERIAL macro if the build system tells us to do so.
#if defined(FEAST_SERIAL_MODE) && !defined(SERIAL)
#  define SERIAL 1
#endif
/// \endcond

// include compiler detection headers
#include <kernel/util/compiler_clang.hpp>      // Clang/LLVM Compiler.
#include <kernel/util/compiler_intel.hpp>      // Intel(R) C/C++ compiler
#include <kernel/util/compiler_microsoft.hpp>  // Microsoft(R) (Visual) C/C++ compiler
#include <kernel/util/compiler_oracle.hpp>     // SunStudio/OracleStudio C/C++ compiler
#include <kernel/util/compiler_open64.hpp>     // Open64 C/C++ compiler
#include <kernel/util/compiler_pgi.hpp>        // PGI C/C++ compiler
// The GNU compiler must be the last one in this list, because other compilers (e.g. Intel and Open64, Clang)
// also define the __GNUC__ macro used to identify the GNU C/C++ compiler, thus leading to incorrect
// compiler detection.
#include <kernel/util/compiler_gnu.hpp>        // GNU C/C++ compiler

// hide the following block from doxygen
/// \cond nodoxy

// If the compiler does not support a 'noinline' specifier, we'll define it as an empty macro.
#ifndef NOINLINE
#define NOINLINE
#endif

// If the compiler does not support a 'force-inline' specifier, we'll define it as a simple inline.
#ifndef FORCE_INLINE
#define FORCE_INLINE inline
#endif

// If the compiler does not support a loop vectorisation specifier, we'll define it as an empty macro.
#ifndef FEAST_IVDEP
#define FEAST_IVDEP
#endif

// If the compiler does not support disabling/restoring warnings, we'll define the corresponding
// macros as empty.
#ifndef FEAST_DISABLE_WARNINGS
#define FEAST_DISABLE_WARNINGS
#endif
#ifndef FEAST_RESTORE_WARNINGS
#define FEAST_RESTORE_WARNINGS
#endif

///\endcond
// end of block hidden from doxygen

/**
 * \brief FEAST namespace
 */
namespace FEAST
{
  /// FEAST version enum
  enum
  {
    /// FEAST major version number
    version_major = 0,
    /// FEAST minor version number
    version_minor = 1,
    /// FEAST patch version number
    version_patch = 1
  };

  /**
   * \brief Index data type.
   */
#ifdef FEAST_INDEX_ULL
  typedef unsigned long long Index;
#else
  typedef unsigned long Index;
#endif

  /**
   * \brief Real data type.
   */
  typedef double Real;
} // namespace FEAST

#endif // KERNEL_BASE_HEADER_HPP
