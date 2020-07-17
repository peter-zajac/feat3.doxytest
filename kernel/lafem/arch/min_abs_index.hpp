// FEAT3: Finite Element Analysis Toolbox, Version 3
// Copyright (C) 2010 - 2020 by Stefan Turek & the FEAT group
// FEAT3 is released under the GNU General Public License version 3,
// see the file 'copyright.txt' in the top level directory for details.

#pragma once
#ifndef KERNEL_LAFEM_ARCH_MIN_ABS_INDEX_HPP
#define KERNEL_LAFEM_ARCH_MIN_ABS_INDEX_HPP 1

// includes, FEAT
#include <kernel/base_header.hpp>
#include <kernel/archs.hpp>



namespace FEAT
{
  namespace LAFEM
  {
    namespace Arch
    {
      template <typename Mem_>
      struct MinAbsIndex;

      template <>
      struct MinAbsIndex<Mem::Main>
      {
        template <typename DT_>
        static Index value(const DT_ * const x, const Index size)
        {
          return value_generic(x, size);
        }

#ifdef FEAT_HAVE_MKL
        static Index value(const float * const x, const Index size)
        {
          return value_mkl(x, size);
        }

        static Index value(const double * const x, const Index size)
        {
          return value_mkl(x, size);
        }
#endif

#if defined(FEAT_HAVE_QUADMATH) && !defined(__CUDACC__)
        static Index value(const __float128 * const x, const Index size)
        {
          return value_generic(x, size);
        }
#endif

        template <typename DT_>
        static Index value_generic(const DT_ * const x, const Index size);

        static Index value_mkl(const float * const x, const Index size);
        static Index value_mkl(const double * const x, const Index size);
      };

#ifdef FEAT_EICKT
      extern template Index MinAbsIndex<Mem::Main>::value_generic(const float * const, const Index);
      extern template Index MinAbsIndex<Mem::Main>::value_generic(const double * const, const Index);
#endif

      template <>
      struct MinAbsIndex<Mem::CUDA>
      {
        template <typename DT_>
        static Index value(const DT_ * const x, const Index size);
      };

    } // namespace Arch
  } // namespace LAFEM
} // namespace FEAT

#ifndef  __CUDACC__
#include <kernel/lafem/arch/min_abs_index_generic.hpp>
#endif
#endif // KERNEL_LAFEM_ARCH_MIN_ABS_INDEX_HPP
