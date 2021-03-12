// FEAT3: Finite Element Analysis Toolbox, Version 3
// Copyright (C) 2010 - 2021 by Stefan Turek & the FEAT group
// FEAT3 is released under the GNU General Public License version 3,
// see the file 'copyright.txt' in the top level directory for details.

#pragma once
#ifndef KERNEL_LAFEM_ARCH_NORM_HPP
#define KERNEL_LAFEM_ARCH_NORM_HPP 1

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
      struct Norm2;

      template <>
      struct Norm2<Mem::Main>
      {
        template <typename DT_>
        static DT_ value(const DT_ * const x, const Index size)
        {
          return value_generic(x, size);
        }

#ifdef FEAT_HAVE_MKL
        static float value(const float * const x, const Index size)
        {
          return value_mkl(x, size);
        }

        static double value(const double * const x, const Index size)
        {
          return value_mkl(x, size);
        }
#endif

#if defined(FEAT_HAVE_QUADMATH) && !defined(__CUDACC__)
        static __float128 value(const __float128 * const x, const Index size)
        {
          return value_generic(x, size);
        }
#endif

        template <typename DT_>
        static DT_ value_generic(const DT_ * const x, const Index size);

        static float value_mkl(const float * const x, const Index size);
        static double value_mkl(const double * const x, const Index size);
      };

#ifdef FEAT_EICKT
      extern template float Norm2<Mem::Main>::value_generic(const float * const, const Index);
      extern template double Norm2<Mem::Main>::value_generic(const double * const, const Index);
#endif

      template <>
      struct Norm2<Mem::CUDA>
      {
        template <typename DT_>
        static DT_ value(const DT_ * const x, const Index size);
      };

    } // namespace Arch
  } // namespace LAFEM
} // namespace FEAT

#ifndef  __CUDACC__
#include <kernel/lafem/arch/norm_generic.hpp>
#endif
#endif // KERNEL_LAFEM_ARCH_NORM2_HPP
