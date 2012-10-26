#pragma once
#ifndef KERNEL_CUBATURE_BARYCENTRE_DRIVER_HPP
#define KERNEL_CUBATURE_BARYCENTRE_DRIVER_HPP 1

// includes, FEAST
#include <kernel/cubature/driver_base.hpp>

namespace FEAST
{
  namespace Cubature
  {
    /// \cond internal
    namespace Intern
    {
      class BarycentreDriverBase :
        public DriverBase
      {
      public:
        enum
        {
          /// this rule is not variadic
          variadic = 0,
          /// this rule has one point
          num_points = 1
        };

        ///Returns the name of the cubature rule.
        static String name()
        {
          return "barycentre";
        }

        /**
         * \brief Adds the driver's aliases.
         *
         * \param[in] functor
         * The functor whose \p alias function is to be called.
         */
        template<typename Functor_>
        static void alias(Functor_& functor)
        {
          functor.alias("midpoint");
        }
      };
    } // namespace Intern
    /// \endcond

    /**
     * \brief Barycentre Rule driver class template
     *
     * This driver implements the barycentre cubature rule.
     * \see http://de.wikipedia.org/wiki/Mittelpunktsregel
     *
     * \tparam Shape_
     * The shape type of the element.
     *
     * \tparam Weight_
     * The data type for the cubature weights.
     *
     * \tparam Coord_
     * The data type for the cubature point coordinates.
     *
     * \tparam Point_
     *
     * \author Peter Zajac
     */
    template<
      typename Shape_,
      typename Weight_,
      typename Coord_,
      typename Point_>
    class BarycentreDriver DOXY({});

    // Simplex specialisation
    template<
      int dim_,
      typename Weight_,
      typename Coord_,
      typename Point_>
    class BarycentreDriver<Shape::Simplex<dim_>, Weight_, Coord_, Point_> :
      public Intern::BarycentreDriverBase
    {
    public:
      typedef Rule<Shape::Simplex<dim_>, Weight_, Coord_, Point_> RuleType;

      /**
       * \brief Fills the cubature rule structure.
       *
       * \param[in,out] rule
       * The cubature rule to be filled.
       */
      static void fill(RuleType& rule)
      {
        rule.get_weight(0) = Weight_(1) / Weight_(Factorial<dim_>::value);

        // create coords of barycentre point
        for(int i(0); i < dim_; ++i)
        {
          rule.get_coord(0, i) = Coord_(1) / Coord_(dim_ + 1);
        }
      }
    }; // class BarycentreDriver<Simplex<...>,...>

    // Hypercube specialisation
    template<
      int dim_,
      typename Weight_,
      typename Coord_,
      typename Point_>
    class BarycentreDriver<Shape::Hypercube<dim_>, Weight_, Coord_, Point_> :
      public Intern::BarycentreDriverBase
    {
    public:
      typedef Rule<Shape::Hypercube<dim_>, Weight_, Coord_, Point_> RuleType;

      /**
       * \brief Fills the cubature rule structure.
       *
       * \param[in,out] rule
       * The cubature rule to be filled.
       */
      static void fill(RuleType& rule)
      {
        rule.get_weight(0) = Weight_(1 << dim_);

        // create coords of barycentre point
        for(int i(0); i < dim_; ++i)
        {
          rule.get_coord(0, i) = Coord_(0);
        }
      }
    }; // class BarycentreDriver<Hypercube<...>,...>
  } // namespace Cubature
} // namespace FEAST

#endif // KERNEL_CUBATURE_BARYCENTRE_DRIVER_HPP