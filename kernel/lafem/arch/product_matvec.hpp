#pragma once
#ifndef KERNEL_LAFEM_ARCH_PRODUCT_MATVEC_HPP
#define KERNEL_LAFEM_ARCH_PRODUCT_MATVEC_HPP 1

// includes, FEAST
#include <kernel/base_header.hpp>
#include <kernel/archs.hpp>
#include <kernel/util/math.hpp>
#include <kernel/util/tiny_algebra.hpp>

namespace FEAST
{
  namespace LAFEM
  {
    namespace Arch
    {
      template <typename Mem_, typename Algo_>
      struct ProductMatVec;

      template <>
      struct ProductMatVec<Mem::Main, Algo::Generic>
      {
        template <typename DT_, typename IT_>
        static void csr(DT_ * r, const DT_ * const val, const IT_ * const col_ind, const IT_ * const row_ptr, const DT_ * const x, const Index rows, const Index, const Index)
        {
          for (Index row(0) ; row < rows ; ++row)
          {
            DT_ sum(0);
            const IT_ end(row_ptr[row + 1]);
            for (IT_ i(row_ptr[row]) ; i < end ; ++i)
            {
              sum += val[i] * x[col_ind[i]];
            }
            r[row] = sum;
          }
        }

        template <typename DT_, typename IT_, Index BlockHeight_, Index BlockWidth_>
        static void csrb(DT_ * r, const DT_ * const val, const IT_ * const col_ind, const IT_ * const row_ptr, const DT_ * const x, const Index rows, const Index, const Index)
        {
          Tiny::Vector<DT_, BlockHeight_> * br(reinterpret_cast<Tiny::Vector<DT_, BlockHeight_> *>(r));
          const Tiny::Matrix<DT_, BlockHeight_, BlockWidth_> * const bval(reinterpret_cast<const Tiny::Matrix<DT_, BlockHeight_, BlockWidth_> * const>(val));
          const Tiny::Vector<DT_, BlockWidth_> * const bx(reinterpret_cast<const Tiny::Vector<DT_, BlockWidth_> * const>(x));

          for (Index row(0) ; row < rows ; ++row)
          {
            Tiny::Vector<DT_, BlockHeight_> bsum(0);
            const IT_ end(row_ptr[row + 1]);
            for (IT_ i(row_ptr[row]) ; i < end ; ++i)
            {
              for (Index h(0) ; h < BlockHeight_ ; ++h)
              {
                for (Index w(0) ; w < BlockWidth_ ; ++w)
                {
                  bsum[h] += bval[i][h][w] * bx[col_ind[i]][w];
                }
              }
            }
            br[row] = bsum;
          }
        }

        template <typename DT_, typename IT_>
        static void ell(DT_ * r, const DT_ * const Ax, const IT_ * const Aj, const IT_ * const Arl, const DT_ * const x, const Index stride, const Index rows)
        {
          for (Index row(0) ; row < rows ; ++row)
          {
            const IT_ * tAj(Aj);
            const DT_ * tAx(Ax);
            DT_ sum(0);
            tAj += row;
            tAx += row;

            const IT_ max(Arl[row]);
            for(IT_ n(0); n < max ; n++)
            {
              const DT_ A_ij = *tAx;

              const IT_ col = *tAj;
              sum += A_ij * x[col];

              tAj += stride;
              tAx += stride;
            }
            r[row] = sum;
          }
        }

        template <typename DT_, typename IT_>
        static void coo(DT_ * r, const DT_ * const val, const IT_ * const row_ptr, const IT_ * const col_ptr, const DT_ * const x, const Index rows, const Index used_elements)
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
            r[row] = sum;
          }
        }

        template <typename DT_, typename IT_>
        static void banded(DT_ * r, const DT_ * const val, const IT_ * const offsets, const DT_ * const x, const Index num_of_offsets, const Index rows, const Index columns)
        {
          #ifdef START_OFFSET
            #warning Overwriting definition of START_OFFSET
            #undef START_OFFSET
          #endif

          #ifdef END_OFFSET
            #error Overwriting definition of END_OFFSET
            #undef END_OFFSET
          #endif

          #define START_OFFSET(j) ((j == Index(-1)) ? rows : ((j == k) ? 0 : rows - offsets[j] - 1))
          #define END_OFFSET(j) ((j == Index(-1)) ? rows : ((j == num_of_offsets) ? 0 : columns + rows - offsets[j] - 1))

          // Search first offset of the upper triangular matrix
          Index k(0);
          while (k < num_of_offsets && offsets[k] + 1 < rows)
          {
            ++k;
          }

          // iteration over all offsets of the lower triangular matrix
          for (Index i(k + 1); i > 0;)
          {
            --i;

            // iteration over all offsets of the upper triangular matrix
            for (Index j(num_of_offsets + 1); j > 0;)
            {
              --j;

              // iteration over all rows which contain the offsets between offset i and offset j
              for (Index l(Math::max(START_OFFSET(i), END_OFFSET(j))); l < Math::min(START_OFFSET(i-1), END_OFFSET(j-1)); ++l)
              {
                DT_ s(0);
                for (Index a(i); a < j; ++a)
                {
                  s += val[a * rows + l] * x[l + offsets[a] + 1 - rows];
                }
                r[l] = s;
              }
            }
          }
          #undef START_OFFSET
          #undef END_OFFSET

          /**
           * \TODO: Implementation for CUDA
           */

          /*
          const Index k1(rows - 1);
          const Index k2(rows + columns - 1);

          for (Index i(0); i < rows; ++i)
          {
            Index start(0);

            while (k1 > offsets[start] + i)
            {
              ++start;
            }

            Index end(start);

            while (end < num_of_offsets && i + offsets[end] <= k2)
            {
              ++end;
            }

            DT_ gamma(DT_(0.0));
            for (Index diag(start); diag < end; ++diag)
            {
              gamma += val[rows * diag + i] * x[i + offsets[diag] - rows + 1];
            }
            r[i] = gamma;
          }
           */
        }
      };

      extern template void ProductMatVec<Mem::Main, Algo::Generic>::csr(float *, const float * const, const unsigned long * const, const unsigned long * const, const float * const, const Index, const Index, const Index);
      extern template void ProductMatVec<Mem::Main, Algo::Generic>::csr(double *, const double * const, const unsigned long * const, const unsigned long * const, const double * const, const Index, const Index, const Index);
      extern template void ProductMatVec<Mem::Main, Algo::Generic>::csr(float *, const float * const, const unsigned int * const, const unsigned int * const, const float * const, const Index, const Index, const Index);
      extern template void ProductMatVec<Mem::Main, Algo::Generic>::csr(double *, const double * const, const unsigned int * const, const unsigned int * const, const double * const, const Index, const Index, const Index);

      extern template void ProductMatVec<Mem::Main, Algo::Generic>::ell(float *, const float * const, const unsigned long * const, const unsigned long * const, const float * const, const Index, const Index);
      extern template void ProductMatVec<Mem::Main, Algo::Generic>::ell(double *, const double * const, const unsigned long * const, const unsigned long * const, const double * const, const Index, const Index);
      extern template void ProductMatVec<Mem::Main, Algo::Generic>::ell(float *, const float * const, const unsigned int * const, const unsigned int * const, const float * const, const Index, const Index);
      extern template void ProductMatVec<Mem::Main, Algo::Generic>::ell(double *, const double * const, const unsigned int * const, const unsigned int * const, const double * const, const Index, const Index);

      extern template void ProductMatVec<Mem::Main, Algo::Generic>::coo(float *, const float * const, const unsigned long * const, const unsigned long * const, const float * const, const Index, const Index);
      extern template void ProductMatVec<Mem::Main, Algo::Generic>::coo(double *, const double * const, const unsigned long * const, const unsigned long * const, const double * const, const Index, const Index);
      extern template void ProductMatVec<Mem::Main, Algo::Generic>::coo(float *, const float * const, const unsigned int * const, const unsigned int * const, const float * const, const Index, const Index);
      extern template void ProductMatVec<Mem::Main, Algo::Generic>::coo(double *, const double * const, const unsigned int * const, const unsigned int * const, const double * const, const Index, const Index);

      extern template void ProductMatVec<Mem::Main, Algo::Generic>::banded(float *, const float * const, const unsigned long * const, const float * const, const Index, const Index, const Index);
      extern template void ProductMatVec<Mem::Main, Algo::Generic>::banded(double *, const double * const, const unsigned long * const, const double * const, const Index, const Index, const Index);
      extern template void ProductMatVec<Mem::Main, Algo::Generic>::banded(float *, const float * const, const unsigned int * const, const float * const, const Index, const Index, const Index);
      extern template void ProductMatVec<Mem::Main, Algo::Generic>::banded(double *, const double * const, const unsigned int * const, const double * const, const Index, const Index, const Index);


      template <>
      struct ProductMatVec<Mem::Main, Algo::MKL>
      {
        static void csr(float * r, const float * const val, const Index * const col_ind, const Index * const row_ptr, const float * const x, const Index rows, const Index columns, const Index used_elements);
        static void csr(double * r, const double * const val, const Index * const col_ind, const Index * const row_ptr, const double * const x, const Index rows, const Index columns, const Index used_elements);

        static void coo(float * r, const float * const val, const Index * const row_ptr, const Index * const col_ptr, const float * const x, const Index rows, const Index used_elements);
        static void coo(double * r, const double * const val, const Index * const row_ptr, const Index * const col_ptr, const double * const x, const Index rows, const Index used_elements);
      };

      template <>
      struct ProductMatVec<Mem::CUDA, Algo::CUDA>
      {
        template <typename DT_>
        static void csr(DT_ * r, const DT_ * const val, const unsigned long * const col_ind, const unsigned long * const row_ptr, const DT_ * const x, const Index rows, const Index columns, const Index used_elements);

        template <typename DT_>
        static void csr(DT_ * r, const DT_ * const val, const unsigned int * const col_ind, const unsigned int * const row_ptr, const DT_ * const x, const Index rows, const Index columns, const Index used_elements);

        template <typename DT_, typename IT_>
        static void ell(DT_ * r, const DT_ * const Ax, const IT_ * const Aj, const IT_ * const Arl, const DT_ * const x, const Index stride, const Index rows);
      };

    } // namespace Arch
  } // namespace LAFEM
} // namespace FEAST

#endif // KERNEL_LAFEM_ARCH_PRODUCT_MATVEC_HPP
