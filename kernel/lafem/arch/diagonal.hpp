// FEAT3: Finite Element Analysis Toolbox, Version 3
// Copyright (C) 2010 - 2021 by Stefan Turek & the FEAT group
// FEAT3 is released under the GNU General Public License version 3,
// see the file 'copyright.txt' in the top level directory for details.

#pragma once
#ifndef KERNEL_LAFEM_ARCH_DIAGONAL_HPP
#define KERNEL_LAFEM_ARCH_DIAGONAL_HPP 1

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
      struct Diagonal;

      template <>
      struct Diagonal<Mem::Main>
      {
        template <typename DT_, typename IT_>
        static void csr(DT_ * diag, const DT_ * const val, const IT_ * const col_ind, const IT_ * const row_ptr, const Index rows)
        {
          csr_generic(diag, val, col_ind, row_ptr, rows);
        }

        template <typename DT_, typename IT_>
        static void csr_generic(DT_ * diag, const DT_ * const val, const IT_ * const col_ind, const IT_ * const row_ptr, const Index rows);

        template <typename DT_, typename IT_, int BlockHeight_, int BlockWidth_>
        static void csrb(DT_ * diag, const DT_ * const val, const IT_ * const col_ind, const IT_ * const row_ptr, const Index rows)
        {
          csrb_generic<DT_, IT_, BlockHeight_, BlockWidth_>(diag, val, col_ind, row_ptr, rows);
        }

        template <typename DT_, typename IT_, int BlockHeight_, int BlockWidth_>
        static void csrb_generic(DT_ * diag, const DT_ * const val, const IT_ * const col_ind, const IT_ * const row_ptr, const Index rows);

        template <typename DT_ , typename IT_>
        static void ell(DT_ * diag, const DT_ * const val, const IT_ * const col_ind,
          const IT_ * const cs, const IT_ * const cl, const Index C, const Index rows)
        {
          ell_generic(diag, val, col_ind, cs, cl, C, rows);
        }

        template <typename DT_, typename IT_>
        static void ell_generic(DT_ * diag, const DT_ * const val, const IT_ * const col_ind,
          const IT_ * const cs, const IT_ * const cl, const Index C, const Index rows);

      };

#ifdef FEAT_EICKT
      extern template void Diagonal<Mem::Main>::csr_generic(float *, const float * const, const Index * const, const Index * const, const Index);
      extern template void Diagonal<Mem::Main>::csr_generic(double *, const double * const, const Index * const, const Index * const, const Index);

      extern template void Diagonal<Mem::Main>::ell_generic(float *, const float * const, const Index * const,
        const Index * const, const Index * const, Index, const Index);
      extern template void Diagonal<Mem::Main>::ell_generic(double *, const double * const, const Index * const,
        const Index * const, const Index * const, Index, const Index);
#endif

      template <>
      struct Diagonal<Mem::CUDA>
      {
        template <typename DT_, typename IT_>
        static void csr(DT_ * diag, const DT_ * const val, const IT_ * const col_ind, const IT_ * const row_ptr, const Index rows);

        template <typename DT_, typename IT_, int BlockHeight_, int BlockWidth_>
        static void csrb(DT_ * diag, const DT_ * const val, const IT_ * const col_ind, const IT_ * const row_ptr, const Index rows);

        template <typename DT_, typename IT_>
        static void ell(DT_ * diag, const DT_ * const val, const IT_ * const col_ind,
          const IT_ * const cs, const IT_ * const cl, const Index C, const Index rows);
      };

    } // namespace Arch
  } // namespace LAFEM
} // namespace FEAT

#ifndef  __CUDACC__
#include <kernel/lafem/arch/diagonal_generic.hpp>
#endif
#endif // KERNEL_LAFEM_ARCH_DIAGONAL_HPP
