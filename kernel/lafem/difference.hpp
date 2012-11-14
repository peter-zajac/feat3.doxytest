#pragma once
#ifndef KERNEL_LAFEM_DIFFERENCE_HPP
#define KERNEL_LAFEM_DIFFERENCE_HPP 1

// includes, FEAST
#include <kernel/base_header.hpp>
#include <kernel/archs.hpp>
#include <kernel/util/exception.hpp>
#include <kernel/lafem/dense_vector.hpp>



namespace FEAST
{
  namespace LAFEM
  {
    template <typename Algo_>
    struct Difference
    {
    };

    /**
     * \brief Difference calculations.
     *
     * This class calculates difference operations.
     *
     * \author Dirk Ribbrock
     */
    template <>
    struct Difference<Algo::Generic>
    {
      /**
       * \brief Calculate \f$r \leftarrow x - y\f$
       *
       * \param[out] r The difference result.
       * \param[in] x.The minuend.
       * \param[in] y The subtrahend.
       */
      template <typename DT_>
      static void value(DenseVector<Mem::Main, DT_> & r, const DenseVector<Mem::Main, DT_> & x, const DenseVector<Mem::Main, DT_> & y)
      {
        if (x.size() != y.size())
          throw InternalError("Vector size does not match!");
        if (x.size() != r.size())
          throw InternalError("Vector size does not match!");

        const DT_ * xp(x.elements());
        const DT_ * yp(y.elements());
        DT_ * rp(r.elements());
        const Index size(r.size());

        if (xp == rp)
        {
          for (Index i(0) ; i < size ; ++i)
          {
            rp[i] -= yp[i];
          }
        }
        else if(yp == rp)
        {
          for (Index i(0) ; i < size ; ++i)
          {
            rp[i] = -rp[i];
            rp[i] += xp[i];
          }
        }
        else
        {
          for (Index i(0) ; i < size ; ++i)
          {
            rp[i] = xp[i] - yp[i];
          }
        }
      }
    };

    template <>
    struct Difference<Algo::CUDA>
    {
      template <typename DT_>
      static void value(DenseVector<Mem::CUDA, DT_> & r, const DenseVector<Mem::CUDA, DT_> & x, const DenseVector<Mem::CUDA, DT_> & y);
    };

  } // namespace LAFEM
} // namespace FEAST

#endif // KERNEL_LAFEM_DIFFERENCE_HPP
