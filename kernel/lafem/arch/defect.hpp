#pragma once
#ifndef KERNEL_LAFEM_ARCH_DEFECT_HPP
#define KERNEL_LAFEM_ARCH_DEFECT_HPP 1

// includes, FEAST
#include <kernel/base_header.hpp>
#include <kernel/archs.hpp>


namespace FEAST
{
  namespace LAFEM
  {
    namespace Arch
    {
      template <typename Mem_, typename Algo_>
      struct Defect;

      template <>
      struct Defect<Mem::Main, Algo::Generic>
      {
        template <typename DT_>
        static void csr(DT_ * r, const DT_ * const rhs, const DT_ * const val, const Index * const col_ind, const Index * const row_ptr, const DT_ * const x, const Index rows)
        {
          for (Index row(0) ; row < rows ; ++row)
          {
            DT_ sum(0);
            const Index end(row_ptr[row + 1]);
            for (Index i(row_ptr[row]) ; i < end ; ++i)
            {
              sum += val[i] * x[col_ind[i]];
            }
            r[row] = rhs[row] - sum;
          }
        }

        template <typename DT_>
        static void ell(DT_ * r, const DT_ * const rhs, const DT_ * const Ax, const Index * const Aj, const Index * const Arl, const DT_ * const x, const Index stride, const Index rows)
        {
          for (Index row(0) ; row < rows ; ++row)
          {
            const Index * tAj(Aj);
            const DT_ * tAx(Ax);
            DT_ sum(0);
            tAj += row;
            tAx += row;

            const Index max(Arl[row]);
            for(Index n(0); n < max ; n++)
            {
              const DT_ A_ij = *tAx;

              const Index col = *tAj;
              sum += A_ij * x[col];

              tAj += stride;
              tAx += stride;
            }
            r[row] = rhs[row] - sum;
          }
        }

        template <typename DT_>
        static void coo(DT_ * r, const DT_ * const rhs, const DT_ * const val, const Index * const row_ptr, const Index * const col_ptr, const DT_ * const x, const Index rows, const Index used_elements)
        {
          Index iter(0);
          for (Index row(0); row < rows; ++row)
          {
            DT_ sum(DT_(0));
            while (iter < used_elements && row_ptr[iter] == row)
            {
              sum += val[iter] * x[col_ptr[iter]];
              ++iter;
            }
            r[row] = rhs[row] - sum;
          }
        }
      };

      extern template void Defect<Mem::Main, Algo::Generic>::csr(float *, const float * const, const float * const, const Index * const, const Index * const, const float * const, const Index);
      extern template void Defect<Mem::Main, Algo::Generic>::csr(double *, const double * const, const double * const, const Index * const, const Index * const, const double * const, const Index);

      extern template void Defect<Mem::Main, Algo::Generic>::ell(float *, const float * const, const float * const, const Index * const, const Index * const, const float * const, const Index, const Index);
      extern template void Defect<Mem::Main, Algo::Generic>::ell(double *,const double * const,  const double * const, const Index * const, const Index * const, const double * const, const Index, const Index);

      extern template void Defect<Mem::Main, Algo::Generic>::coo(float *, const float * const, const float * const, const Index * const, const Index * const, const float * const, const Index, const Index);
      extern template void Defect<Mem::Main, Algo::Generic>::coo(double *, const double * const, const double * const, const Index * const, const Index * const, const double * const, const Index, const Index);

      template <>
        struct Defect<Mem::Main, Algo::MKL>
      {
        static void csr(float * r, const float * const rhs, const float * const val, const Index * const col_ind, const Index * const row_ptr, const float * const x, const Index rows);
        static void csr(double * r, const double * const rhs, const double * const val, const Index * const col_ind, const Index * const row_ptr, const double * const x, const Index rows);

        static void coo(float * r, const float * const rhs, const float * const val, const Index * const row_ptr, const Index * const col_ptr, const float * const x, const Index rows, const Index used_elements);
        static void coo(double * r, const double * const rhs, const double * const val, const Index * const row_ptr, const Index * const col_ptr, const double * const x, const Index rows, const Index used_elements);
      };

      template <>
      struct Defect<Mem::CUDA, Algo::CUDA>
      {
        template <typename DT_>
        static void csr(DT_ * r, const DT_ * const rhs, const DT_ * const val, const Index * const col_ind, const Index * const row_ptr, const DT_ * const x, const Index rows);

        template <typename DT_>
        static void ell(DT_ * r, const DT_ * const rhs, const DT_ * const Ax, const Index * const Aj, const Index * const Arl, const DT_ * const x, const Index stride, const Index rows);
      };

    } // namespace Arch
  } // namespace LAFEM
} // namespace FEAST

#endif // KERNEL_LAFEM_ARCH_DEFECT_HPP
