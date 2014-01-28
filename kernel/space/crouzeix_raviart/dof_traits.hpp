#pragma once
#ifndef KERNEL_SPACE_CROUZEIX_RAVIART_DOF_TRAITS_HPP
#define KERNEL_SPACE_CROUZEIX_RAVIART_DOF_TRAITS_HPP 1

// includes, FEAST
#include <kernel/shape.hpp>

namespace FEAST
{
  namespace Space
  {
    namespace CrouzeixRaviart
    {
      /**
       * \brief Crouzeix-Raviart Dof-Traits class template.
       *
       * \author Peter Zajac
       */
      template<typename Shape_, int dim_>
      struct DofTraits
      {
        enum
        {
          /// no dofs for any shape dimension > 0
          count = 0
        };

        static Index derive_order(Index)
        {
          return Index(0);
        }
      };

      template<int dim_>
      struct DofTraits<Shape::Simplex<dim_+1>, dim_>
      {
        enum
        {
          /// 1 dof per facet
          count = 1
        };

        static Index derive_order(Index)
        {
          return Index(0);
        }
      };
    } // namespace CrouzeixRaviart
  } // namespace Space
} // namespace FEAST

#endif // KERNEL_SPACE_CROUZEIX_RAVIART_DOF_TRAITS_HPP
