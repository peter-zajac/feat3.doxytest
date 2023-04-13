// FEAT3: Finite Element Analysis Toolbox, Version 3
// Copyright (C) 2010 - 2023 by Stefan Turek & the FEAT group
// FEAT3 is released under the GNU General Public License version 3,
// see the file 'copyright.txt' in the top level directory for details.

#pragma once
#ifndef KERNEL_UTIL_COMPILER_PGI_HPP
#define KERNEL_UTIL_COMPILER_PGI_HPP 1

/**
 * \file compiler_pgi.hpp
 *
 * \brief Compiler detection header for PGI C++ compiler.
 */

#if !defined(FEAT_COMPILER) && defined(__PGI)

// calc linear sortable pgi version
# define FEAT_COMPILER_PGI (__PGIC__ * 10000 + __PGIC_MINOR__ * 100 + __PGIC_PATCHLEVEL__)

# define FEAT_COMPILER "PGI C/C++ compiler"

#endif // !defined(FEAT_COMPILER) && defined(__PGI)

#endif // KERNEL_UTIL_COMPILER_PGI_HPP
