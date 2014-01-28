#pragma once
#ifndef KERNEL_SPACE_CROUZEIX_RAVIART_EVALUATOR_HPP
#define KERNEL_SPACE_CROUZEIX_RAVIART_EVALUATOR_HPP 1

// includes, FEAST
#include <kernel/space/parametric_evaluator.hpp>
#include <kernel/space/dof_mapping_common.hpp>

namespace FEAST
{
  namespace Space
  {
    namespace CrouzeixRaviart
    {
      /**
       * \brief Crouzeix-Raviart Element Evaluator reference capabilities
       *
       * \author Peter Zajac
       */
      struct ReferenceCapabilities
      {
        /// dummy enum
        enum
        {
          /// can compute reference function values
          can_ref_value = 1,
          /// can compute reference gradients
          can_ref_grad = 1,
          /// can't compute reference hessians
          can_ref_hess = 0,
        };
      };

      /**
       * \brief Crouzeix-Raviart Element Evaluator class template declaration.
       *
       * \author Peter Zajac
       */
      template<
        typename Space_,
        typename TrafoEvaluator_,
        typename SpaceEvalTraits_,
        typename Shape_ = typename Space_::ShapeType>
      class Evaluator DOXY({});

      /**
       * \brief Crouzeix-Raviart Element evaluator implementation for Triangle shape
       *
       * \author Peter Zajac
       */
      template<
        typename Space_,
        typename TrafoEvaluator_,
        typename SpaceEvalTraits_>
      class Evaluator<Space_, TrafoEvaluator_, SpaceEvalTraits_, Shape::Simplex<2> > :
        public ParametricEvaluator<
          Evaluator<
            Space_,
            TrafoEvaluator_,
            SpaceEvalTraits_,
            Shape::Simplex<2> >,
          TrafoEvaluator_,
          SpaceEvalTraits_,
          ReferenceCapabilities>
      {
      public:
        /// base-class typedef
        typedef ParametricEvaluator<Evaluator, TrafoEvaluator_, SpaceEvalTraits_, ReferenceCapabilities> BaseClass;

        /// space type
        typedef Space_ SpaceType;

        /// space evaluation traits
        typedef SpaceEvalTraits_ SpaceEvalTraits;

        /// evaluation policy
        typedef typename SpaceEvalTraits::EvalPolicy EvalPolicy;

        /// domain point type
        typedef typename EvalPolicy::DomainPointType DomainPointType;

        /// data type
        typedef typename SpaceEvalTraits::DataType DataType;

      public:
        /**
         * \brief Constructor.
         *
         * \param[in] space
         * A reference to the Element using this evaluator.
         */
        explicit Evaluator(const SpaceType& DOXY(space))
        {
        }

        /**
         * \brief Returns the number of local DOFs.
         *
         * \returns
         * The number of local dofs.
         */
        Index get_num_local_dofs() const
        {
          return 3;
        }

        /**
         * \brief Evaluates the basis function values on the reference cell.
         *
         * \param[out] values
         * A reference to a basis value vector receiving the result.
         *
         * \param[in] point
         * A reference to the point on the reference cell where to evaluate.
         */
        template<typename EvalData_>
        void eval_ref_values(
          EvalData_& data,
          const DomainPointType& point) const
        {
          data.phi[0].ref_value = DataType(2) * (point[0] + point[1]) - DataType(1);
          data.phi[1].ref_value = DataType(1) - DataType(2)*point[0];
          data.phi[2].ref_value = DataType(1) - DataType(2)*point[1];
        }

        /**
         * \brief Evaluates the basis function gradients on the reference cell.
         *
         * \param[out] data
         * A reference to a basis gradient vector receiveing the result.
         *
         * \param[in] point
         * A reference to the point on the reference cell where to evaluate.
         */
        template<typename EvalData_>
        void eval_ref_gradients(
          EvalData_& data,
          const DomainPointType& DOXY(point)) const
        {
          data.phi[0].ref_grad[0] = DataType(2);
          data.phi[0].ref_grad[1] = DataType(2);
          data.phi[1].ref_grad[0] = -DataType(2);
          data.phi[1].ref_grad[1] = DataType(0);
          data.phi[2].ref_grad[0] = DataType(0);
          data.phi[2].ref_grad[1] = -DataType(2);
        }
      }; // class Evaluator<...,Simplex<2>>
    } // namespace CrouzeixRaviart
  } // namespace Space
} // namespace FEAST

#endif // KERNEL_SPACE_CROUZEIX_RAVIART_EVALUATOR_HPP
