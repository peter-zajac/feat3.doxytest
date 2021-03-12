// FEAT3: Finite Element Analysis Toolbox, Version 3
// Copyright (C) 2010 - 2021 by Stefan Turek & the FEAT group
// FEAT3 is released under the GNU General Public License version 3,
// see the file 'copyright.txt' in the top level directory for details.

#pragma once
#ifndef KERNEL_LAFEM_ARCH_LUMPING_HPP
#define KERNEL_LAFEM_ARCH_LUMPING_HPP 1

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
      struct Lumping;

      template <>
      struct Lumping<Mem::Main>
      {
        template <typename DT_, typename IT_>
        static void csr(DT_ * lump, const DT_ * const val, const IT_ * const col_ind, const IT_ * const row_ptr, const Index rows)
        {
          csr_generic(lump, val, col_ind, row_ptr, rows);
        }

        template <typename DT_, typename IT_>
        static void csr_generic(DT_ * lump, const DT_ * const val, const IT_ * const col_ind, const IT_ * const row_ptr, const Index rows);

        template <typename DT_, typename IT_>
        static void bcsr(DT_ * lump, const DT_ * const val, const IT_ * const col_ind, const IT_ * const row_ptr, const Index rows, const int BlockHeight, const int BlockWidth)
        {
          bcsr_generic(lump, val, col_ind, row_ptr, rows, BlockHeight, BlockWidth);
        }

        template <typename DT_, typename IT_>
        static void bcsr_generic(DT_* lump, const DT_* const val, const IT_* const col_ind, const IT_ * const row_ptr, const Index rows, const int BlockHeight, const int BlockWidth);

        template <typename DT_ , typename IT_>
        static void ell(DT_ * lump, const DT_ * const val, const IT_ * const col_ind,
          const IT_ * const cs, const IT_ * const cl, const Index C, const Index rows)
        {
          ell_generic(lump, val, col_ind, cs, cl, C, rows);
        }

        template <typename DT_, typename IT_>
        static void ell_generic(DT_ * lump, const DT_ * const val, const IT_ * const col_ind,
          const IT_ * const cs, const IT_ * const cl, const Index C, const Index rows);
      };

#ifdef FEAT_EICKT
      extern template void Lumping<Mem::Main>::csr_generic(float *, const float * const, const Index * const, const Index * const, const Index);
      extern template void Lumping<Mem::Main>::csr_generic(double *, const double * const, const Index * const, const Index * const, const Index);

      extern template void Lumping<Mem::Main>::bcsr_generic(float *, const float * const, const Index * const, const Index * const, const Index, const int, const int);
      extern template void Lumping<Mem::Main>::bcsr_generic(double *, const double * const, const Index * const, const Index * const, const Index, const int, const int);

      extern template void Lumping<Mem::Main>::ell_generic(float *, const float * const, const Index * const,
        const Index * const, const Index * const, Index, const Index);
      extern template void Lumping<Mem::Main>::ell_generic(double *, const double * const, const Index * const,
        const Index * const, const Index * const, Index, const Index);
#endif


      template <>
      struct Lumping<Mem::CUDA>
      {
        template <typename DT_, typename IT_>
        static void csr(DT_ * lump, const DT_ * const val, const IT_ * const col_ind, const IT_ * const row_ptr, const Index rows);

        template <typename DT_, typename IT_>
        static void ell(DT_ * lump, const DT_ * const val, const IT_ * const col_ind,
          const IT_ * const cs, const IT_ * const cl, const Index C, const Index rows);

        template <typename DT_, typename IT_>
        static void bcsr(DT_ * lump, const DT_ * const val, const IT_ * const col_ind, const IT_ * const row_ptr, const Index rows, const int BlockHeight, const int BlockWidth);
      };

    } // namespace Arch
  } // namespace LAFEM
} // namespace FEAT

#ifndef  __CUDACC__
#include <kernel/lafem/arch/lumping_generic.hpp>
#endif
#endif // KERNEL_LAFEM_ARCH_LUMPING_HPP
