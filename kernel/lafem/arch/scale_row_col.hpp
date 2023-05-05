// FEAT3: Finite Element Analysis Toolbox, Version 3
// Copyright (C) 2010 - 2023 by Stefan Turek & the FEAT group
// FEAT3 is released under the GNU General Public License version 3,
// see the file 'copyright.txt' in the top level directory for details.

#pragma once
#ifndef KERNEL_LAFEM_ARCH_SCALE_ROW_COL_HPP
#define KERNEL_LAFEM_ARCH_SCALE_ROW_COL_HPP 1

// includes, FEAT
#include <kernel/base_header.hpp>
#include <kernel/backend.hpp>

namespace FEAT
{
  namespace LAFEM
  {
    namespace Arch
    {
      struct ScaleRows
      {
        template <typename DT_, typename IT_>
        static void csr(DT_ * r, const DT_ * const a, const IT_ * const col_ind, const IT_ * const row_ptr, const DT_ * const x, const Index rows, const Index columns, const Index used_elements)
        {
          csr_generic(r, a, col_ind, row_ptr, x, rows, columns, used_elements);
        }

        static void csr(float * r, const float * const a, const std::uint64_t * const col_ind, const std::uint64_t * const row_ptr, const float * const x, const Index rows, const Index columns, const Index used_elements)
        {
          BACKEND_SKELETON_VOID(csr_cuda, csr_generic, csr_generic, r, a, col_ind, row_ptr, x, rows, columns, used_elements)
        }

        static void csr(double * r, const double * const a, const std::uint64_t * const col_ind, const std::uint64_t * const row_ptr, const double * const x, const Index rows, const Index columns, const Index used_elements)
        {
          BACKEND_SKELETON_VOID(csr_cuda, csr_generic, csr_generic, r, a, col_ind, row_ptr, x, rows, columns, used_elements)
        }

        static void csr(float * r, const float * const a, const std::uint32_t * const col_ind, const std::uint32_t * const row_ptr, const float * const x, const Index rows, const Index columns, const Index used_elements)
        {
          BACKEND_SKELETON_VOID(csr_cuda, csr_generic, csr_generic, r, a, col_ind, row_ptr, x, rows, columns, used_elements)
        }

        static void csr(double * r, const double * const a, const std::uint32_t * const col_ind, const std::uint32_t * const row_ptr, const double * const x, const Index rows, const Index columns, const Index used_elements)
        {
          BACKEND_SKELETON_VOID(csr_cuda, csr_generic, csr_generic, r, a, col_ind, row_ptr, x, rows, columns, used_elements)
        }

        template <typename DT_, typename IT_>
        static void csr_generic(DT_ * r, const DT_ * const a, const IT_ * const /*col_ind*/, const IT_ * const row_ptr, const DT_ * const x, const Index rows, const Index, const Index);

        template <typename DT_, typename IT_>
        static void csr_cuda(DT_ * r, const DT_ * const a, const IT_ * const /*col_ind*/, const IT_ * const row_ptr, const DT_ * const x, const Index rows, const Index, const Index);
      };

#ifdef FEAT_EICKT
      extern template void ScaleRows::csr_generic(float *, const float * const, const std::uint64_t * const, const std::uint64_t * const, const float * const, const Index, const Index, const Index);
      extern template void ScaleRows::csr_generic(double *, const double * const, const std::uint64_t * const, const std::uint64_t * const, const double * const, const Index, const Index, const Index);
      extern template void ScaleRows::csr_generic(float *, const float * const, const std::uint32_t * const, const std::uint32_t * const, const float * const, const Index, const Index, const Index);
      extern template void ScaleRows::csr_generic(double *, const double * const, const std::uint32_t * const, const std::uint32_t * const, const double * const, const Index, const Index, const Index);
#endif


      // ***********************************************

      struct ScaleCols
      {
        template <typename DT_, typename IT_>
        static void csr(DT_ * r, const DT_ * const a, const IT_ * const col_ind, const IT_ * const row_ptr, const DT_ * const x, const Index rows, const Index columns, const Index used_elements)
        {
          csr_generic(r, a, col_ind, row_ptr, x, rows, columns, used_elements);
        }

        static void csr(float * r, const float * const a, const std::uint64_t * const col_ind, const std::uint64_t * const row_ptr, const float * const x, const Index rows, const Index columns, const Index used_elements)
        {
          BACKEND_SKELETON_VOID(csr_cuda, csr_generic, csr_generic, r, a, col_ind, row_ptr, x, rows, columns, used_elements)
        }

        static void csr(double * r, const double * const a, const std::uint64_t * const col_ind, const std::uint64_t * const row_ptr, const double * const x, const Index rows, const Index columns, const Index used_elements)
        {
          BACKEND_SKELETON_VOID(csr_cuda, csr_generic, csr_generic, r, a, col_ind, row_ptr, x, rows, columns, used_elements)
        }

        static void csr(float * r, const float * const a, const std::uint32_t * const col_ind, const std::uint32_t * const row_ptr, const float * const x, const Index rows, const Index columns, const Index used_elements)
        {
          BACKEND_SKELETON_VOID(csr_cuda, csr_generic, csr_generic, r, a, col_ind, row_ptr, x, rows, columns, used_elements)
        }

        static void csr(double * r, const double * const a, const std::uint32_t * const col_ind, const std::uint32_t * const row_ptr, const double * const x, const Index rows, const Index columns, const Index used_elements)
        {
          BACKEND_SKELETON_VOID(csr_cuda, csr_generic, csr_generic, r, a, col_ind, row_ptr, x, rows, columns, used_elements)
        }

        template <typename DT_, typename IT_>
        static void csr_generic(DT_ * r, const DT_ * const a, const IT_ * const col_ind, const IT_ * const row_ptr, const DT_ * const x, const Index rows, const Index, const Index);

        template <typename DT_, typename IT_>
        static void csr_cuda(DT_ * r, const DT_ * const a, const IT_ * const col_ind, const IT_ * const row_ptr, const DT_ * const x, const Index rows, const Index, const Index);
      };

#ifdef FEAT_EICKT
      extern template void ScaleCols::csr_generic(float *, const float * const, const std::uint64_t * const, const std::uint64_t * const, const float * const, const Index, const Index, const Index);
      extern template void ScaleCols::csr_generic(double *, const double * const, const std::uint64_t * const, const std::uint64_t * const, const double * const, const Index, const Index, const Index);
      extern template void ScaleCols::csr_generic(float *, const float * const, const std::uint32_t * const, const std::uint32_t * const, const float * const, const Index, const Index, const Index);
      extern template void ScaleCols::csr_generic(double *, const double * const, const std::uint32_t * const, const std::uint32_t * const, const double * const, const Index, const Index, const Index);
#endif
    } // namespace Arch
  } // namespace LAFEM
} // namespace FEAT

#ifndef  __CUDACC__
#include <kernel/lafem/arch/scale_row_col_generic.hpp>
#endif
#endif // KERNEL_LAFEM_ARCH_SCALE_ROW_COL_HPP
