#pragma once
#ifndef KERNEL_LAFEM_ARCH_COMPONENT_INVERT_HPP
#define KERNEL_LAFEM_ARCH_COMPONENT_INVERT_HPP 1

// includes, FEAST
#include <kernel/base_header.hpp>
#include <kernel/archs.hpp>

namespace FEAST
{
  namespace LAFEM
  {
    namespace Arch
    {
      template<typename Mem_, typename Algo_>
      struct ComponentInvert;

      template<>
      struct ComponentInvert<Mem::Main, Algo::Generic>
      {
        template<typename DT_>
        static void value(DT_* r, const DT_* const x, const DT_ s, const Index size)
        {
          if (r == x)
          {
            for(Index i(0); i < size; ++i)
            {
              r[i] = s / r[i];
            }
          }
          else
          {
            for(Index i(0); i < size; ++i)
            {
              r[i] = s / x[i];
            }
          }
        }
      };

      extern template void ComponentInvert<Mem::Main, Algo::Generic>::value(float*, const float* const, const float, const Index);
      extern template void ComponentInvert<Mem::Main, Algo::Generic>::value(double*, const double* const, const double, const Index);

      template<>
      struct ComponentInvert<Mem::CUDA, Algo::CUDA>
      {
        template<typename DT_>
        static void value(DT_* r, const DT_* const x, const DT_ s, const Index size);
      };
    } // namespace Arch
  } // namespace LAFEM
} // namespace FEAST

#endif // KERNEL_LAFEM_ARCH_COMPONENT_INVERT_HPP
