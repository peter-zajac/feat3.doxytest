// FEAT3: Finite Element Analysis Toolbox, Version 3
// Copyright (C) 2010 - 2019 by Stefan Turek & the FEAT group
// FEAT3 is released under the GNU General Public License version 3,
// see the file 'copyright.txt' in the top level directory for details.

#pragma once
#ifndef KERNEL_ASSEMBLY_COMMON_OPERATORS_HPP
#define KERNEL_ASSEMBLY_COMMON_OPERATORS_HPP 1

// includes, FEAT
#include <kernel/assembly/bilinear_operator.hpp>

namespace FEAT
{
  namespace Assembly
  {
    /**
     * \brief Assembly Common namespace
     *
     * This namespace encapsulated commonly used operators and functionals for use with the
     * various assembly classes, which are often used in standard benchmark problems.
     */
    namespace Common
    {
      /**
       * \brief -Laplace operator implementation
       *
       * This functor implements the weak formulation of the bilinear scalar Laplace operator, i.e.
       *   \f[ \nabla \varphi \cdot \nabla\psi \f]
       *
       * This functor can be used with the BilinearOperator assembly class template to assemble a
       * scalar Laplace/Stiffness matrix.
       *
       * \author Peter Zajac
       */
      class LaplaceOperator :
        public BilinearOperator
      {
      public:
        static constexpr TrafoTags trafo_config = TrafoTags::none;
        static constexpr SpaceTags test_config = SpaceTags::grad;
        static constexpr SpaceTags trial_config = SpaceTags::grad;

        /**
         * \brief Laplace evaluator class template
         *
         * \tparam AsmTraits_
         * The assembly traits class.
         *
         * \author Peter Zajac
         */
        template<typename AsmTraits_>
        class Evaluator :
          public BilinearOperator::Evaluator<AsmTraits_>
        {
        public:
          /// the data type to be used
          typedef typename AsmTraits_::DataType DataType;
          /// the assembler's trafo data type
          typedef typename AsmTraits_::TrafoData TrafoData;
          /// the assembler's test-function data type
          typedef typename AsmTraits_::TestBasisData TestBasisData;
          /// the assembler's trial-function data type
          typedef typename AsmTraits_::TrialBasisData TrialBasisData;

        public:
          /**
           * \brief Constructor
           *
           * \param[in] operat
           * A reference to the Laplace operator object.
           */
          explicit Evaluator(const LaplaceOperator& DOXY(operat))
          {
          }

          /**
           * \brief Evaluation operator
           *
           * This operator evaluates the bilinear operator for a given combination of test- and trial-functions in
           * a single point.
           *
           * \param[in] phi
           * The trial function data in the current evaluation point. \see Space::EvalData
           *
           * \param[in] psi
           * The test function data in the current evaluation point. \see Space::EvalData
           *
           * \returns
           * The value of the bilinear functor.
           **/
          DataType operator()(const TrialBasisData& phi, const TestBasisData& psi)
          {
            return dot(phi.grad, psi.grad);
          }
        }; // class LaplaceOperator::Evaluator<...>
      }; // class LaplaceOperator

      /**
       * \brief -Laplace operator implementation
       *
       * This functor implements the weak formulation of the bilinear blocked Laplace operator, i.e.
       *   \f[ \nabla \varphi \cdot \nabla\psi \f]
       *
       * This functor can be used with the BilinearOperator assembly class template to assemble a
       * blocked Laplace/Stiffness matrix.
       *
       * \author Peter Zajac
       */
      template<int dimension_>
      class LaplaceOperatorBlocked :
        public BilinearOperator
      {
      public:
        /// Every block is a dimension x dimension matrix
        static constexpr int BlockHeight = dimension_;
        /// Every block is a dimension x dimension matrix
        static constexpr int BlockWidth = dimension_;

        static constexpr TrafoTags trafo_config = TrafoTags::none;
        static constexpr SpaceTags test_config = SpaceTags::grad;
        static constexpr SpaceTags trial_config = SpaceTags::grad;

        /**
         * \brief Laplace evaluator class template
         *
         * \tparam AsmTraits_
         * The assembly traits class.
         *
         * \author Peter Zajac
         */
        template<typename AsmTraits_>
        class Evaluator :
          public BilinearOperator::Evaluator<AsmTraits_>
        {
        public:
          /// the data type to be used
          typedef typename AsmTraits_::DataType DataType;
          /// the data type for the block system
          typedef typename AsmTraits_::OperatorValueType OperatorValueType;
          /// the assembler's trafo data type
          typedef typename AsmTraits_::TrafoData TrafoData;
          /// the assembler's test-function data type
          typedef typename AsmTraits_::TestBasisData TestBasisData;
          /// the assembler's trial-function data type
          typedef typename AsmTraits_::TrialBasisData TrialBasisData;

        public:
          /**
           * \brief Constructor
           *
           * \param[in] operat
           * A reference to the Laplace operator object.
           */
          explicit Evaluator(const LaplaceOperatorBlocked& DOXY(operat))
          {
          }

          /**
           * \brief Evaluation operator
           *
           * This operator evaluates the bilinear operator for a given combination of test- and trial-functions in
           * a single point.
           *
           * \param[in] phi
           * The trial function data in the current evaluation point. \see Space::EvalData
           *
           * \param[in] psi
           * The test function data in the current evaluation point. \see Space::EvalData
           *
           * \returns
           * The value of the bilinear functor.
           **/
          OperatorValueType operator()(const TrialBasisData& phi, const TestBasisData& psi)
          {
            const DataType scalar = dot(phi.grad, psi.grad);
            OperatorValueType r(DataType(0));
            for(int i(0); i < OperatorValueType::m; ++i)
            {
              r(i,i) = scalar;
            }
            return r;
          }
        }; // class LaplaceOperatorBlocked::Evaluator<...>
      }; // class LaplaceOperatorBlocked

      /**
       * \brief Identity operator implementation
       *
       * This functor implements the weak formulation of the bilinear scalar Identity operator, i.e.
       *   \f[ \varphi \cdot \psi \f]
       *
       * This functor can be used with the BilinearOperator assembler class template to assemble a
       * scalar mass matrix.
       *
       * \author Peter Zajac
       */
      class IdentityOperator :
        public BilinearOperator
      {
      public:
        static constexpr TrafoTags trafo_config = TrafoTags::none;
        static constexpr SpaceTags test_config = SpaceTags::value;
        static constexpr SpaceTags trial_config = SpaceTags::value;

        /**
         * \brief Bilinear scalar Identity evaluator class template
         *
         * \tparam AsmTraits_
         * The assembly traits class.
         *
         * \author Peter Zajac
         */
        template<typename AsmTraits_>
        class Evaluator :
          public BilinearOperator::Evaluator<AsmTraits_>
        {
        public:
          /// the data type to be used
          typedef typename AsmTraits_::DataType DataType;
          /// the assembler's trafo data type
          typedef typename AsmTraits_::TrafoData TrafoData;
          /// the assembler's test-function data type
          typedef typename AsmTraits_::TestBasisData TestBasisData;
          /// the assembler's trial-function data type
          typedef typename AsmTraits_::TrialBasisData TrialBasisData;

        public:
          /**
           * \brief Constructor
           *
           * \param[in] operat
           * A reference to the Identity operator object.
           */
          explicit Evaluator(const IdentityOperator& DOXY(operat))
          {
          }

          /**
           * \brief Evaluation operator
           *
           * This operator evaluates the bilinear operator for a given combination of test- and trial-functions in
           * a single point.
           *
           * \param[in] phi
           * The trial function data in the current evaluation point. \see Space::EvalData
           *
           * \param[in] psi
           * The test function data in the current evaluation point. \see Space::EvalData
           *
           * \returns
           * The value of the bilinear functor.
           **/
          DataType operator()(const TrialBasisData& phi, const TestBasisData& psi)
          {
            return phi.value * psi.value;
          }
        }; // class IdentityOperator::Evaluator<...>
      }; // class IdentityOperator

      /**
       * \brief Vector-valued identity operator implementation
       *
       * This functor can be used with the BilinearOperator assembly class template to assemble one
       * scalar matrix corresponding to one block of the \f$ d \times d \f$ block matrix, where \f$ d \f$ is the
       * number of space dimensions.
       *
       * \author Jordi Paul
       */
      template<int dimension_>
      class IdentityOperatorBlocked :
        public BilinearOperator
      {
      public:
        /// Every block is a dimension x dimension matrix
        static constexpr int BlockHeight = dimension_;
        /// Every block is a dimension x dimension matrix
        static constexpr int BlockWidth = dimension_;

        static constexpr TrafoTags trafo_config = TrafoTags::none;
        static constexpr SpaceTags test_config = SpaceTags::value;
        static constexpr SpaceTags trial_config = SpaceTags::value;

        /**
         * \brief Vector-valued identity operator evaluator class template
         *
         * \tparam AsmTraits_
         * The assembly traits class.
         *
         * \author Jordi Paul
         */
        template<typename AsmTraits_>
        class Evaluator :
          public BilinearOperator::Evaluator<AsmTraits_>
        {
        public:
          /// the data type to be used
          typedef typename AsmTraits_::DataType DataType;
          /// the data type for the block system
          typedef typename AsmTraits_::OperatorValueType OperatorValueType;
          /// the assembler's trafo data type
          typedef typename AsmTraits_::TrafoData TrafoData;
          /// the assembler's test-function data type
          typedef typename AsmTraits_::TestBasisData TestBasisData;
          /// the assembler's trial-function data type
          typedef typename AsmTraits_::TrialBasisData TrialBasisData;

        public:
          /**
           * \brief Constructor
           *
           * \param[in] operat
           * A reference to the Du : Dv operator object.
           */
          explicit Evaluator(const IdentityOperatorBlocked& DOXY(operat))
          {
          }

          /**
           * \brief Evaluation operator
           *
           * This operator evaluates the bilinear operator for a given combination of test- and trial-functions in
           * a single point.
           *
           * \param[in] phi
           * The trial function data in the current evaluation point. \see Space::EvalData
           *
           * \param[in] psi
           * The test function data in the current evaluation point. \see Space::EvalData
           *
           * \returns
           * The value of the bilinear functor.
           **/
          OperatorValueType operator()(const TrialBasisData& phi, const TestBasisData& psi)
          {
            OperatorValueType r(DataType(0));
            for(int i(0); i < OperatorValueType::m; ++i)
            {
              r(i,i) = phi.value * psi.value;
            }
            return r;
          }
        }; // class IdentityOperatorBlocked::Evaluator<...>
      }; // class IdentityOperatorBlocked

      /**
       * \brief Trial-Derivative operator implementation
       *
       * This functor implements the weak formulation of the bilinear trial-function derivative operator, i.e.
       *   \f[ \partial_i \varphi \cdot \psi \f]
       *
       * This functor can be used with the BilinearOperator assembly class template to assemble a
       * scalar matrix for the pressure-gradient operator of the Stokes equation.
       *
       * \author Peter Zajac
       */
      class TrialDerivativeOperator :
        public BilinearOperator
      {
      public:
        /// the desired derivative
        int deriv;

        static constexpr TrafoTags trafo_config = TrafoTags::none;
        static constexpr SpaceTags test_config = SpaceTags::grad;
        static constexpr SpaceTags trial_config = SpaceTags::value;

        /**
         * \brief Trial-Derivative evaluator class template
         *
         * \tparam AsmTraits_
         * The assembly traits class.
         *
         * \author Peter Zajac
         */
        template<typename AsmTraits_>
        class Evaluator :
          public BilinearOperator::Evaluator<AsmTraits_>
        {
        public:
          /// the data type to be used
          typedef typename AsmTraits_::DataType DataType;
          /// the assembler's trafo data type
          typedef typename AsmTraits_::TrafoData TrafoData;
          /// the assembler's test-function data type
          typedef typename AsmTraits_::TestBasisData TestBasisData;
          /// the assembler's trial-function data type
          typedef typename AsmTraits_::TrialBasisData TrialBasisData;

        protected:
          /// the desired derivative
          int deriv;

        public:
          /**
           * \brief Constructor
           *
           * \param[in] operat
           * A reference to the Laplace operator object.
           */
          explicit Evaluator(const TrialDerivativeOperator& operat) :
            deriv(operat.deriv)
          {
          }

          // copy pasted since Doxygen does not like the operator part in
          // \copydoc BilinearOperator::Evaluator::operator()
          /**
           * \brief Evaluation operator
           *
           * This operator evaluates the bilinear operator for a given combination of test- and trial-functions in
           * a single point.
           *
           * \param[in] phi
           * The trial function data in the current evaluation point. \see Space::EvalData
           *
           * \param[in] psi
           * The test function data in the current evaluation point. \see Space::EvalData
           *
           * \returns
           * The value of the bilinear functor.
           **/
          DataType operator()(const TrialBasisData& phi, const TestBasisData& psi)
          {
            return phi.value * psi.grad[deriv];
          }
        }; // class TrialDerivativeOperator::Evaluator<...>

        /*
         *
         * \param[in] derivative
         * The index of the derivative for this operator:
         *  - 0: X-derivative
         *  - 1: Y-derivative
         *  - 2: Z-derivative
         *  - ...
        */
        explicit TrialDerivativeOperator(int derivative) :
          deriv(derivative)
        {
        }
      }; // class TrialDerivativeOperator

      /**
       * \brief Test-Derivative operator implementation
       *
       * This functor implements the weak formulation of the bilinear test-function derivative operator, i.e.
       *   \f[ \varphi \cdot \partial_i \psi \f]
       *
       * This functor can be used with the BilinearOperator assembly class template to assemble a
       * scalar matrix for the pressure-gradient operator of the Stokes equation.
       *
       * \author Peter Zajac
       */
      class TestDerivativeOperator :
        public BilinearOperator
      {
      public:
        /// the desired derivative
        int deriv;

        static constexpr TrafoTags trafo_config = TrafoTags::none;
        static constexpr SpaceTags test_config = SpaceTags::grad;
        static constexpr SpaceTags trial_config = SpaceTags::value;

        /**
         * \brief Test-Derivative evaluator class template
         *
         * \tparam AsmTraits_
         * The assembly traits class.
         *
         * \author Peter Zajac
         */
        template<typename AsmTraits_>
        class Evaluator :
          public BilinearOperator::Evaluator<AsmTraits_>
        {
        public:
          /// the data type to be used
          typedef typename AsmTraits_::DataType DataType;
          /// the assembler's trafo data type
          typedef typename AsmTraits_::TrafoData TrafoData;
          /// the assembler's test-function data type
          typedef typename AsmTraits_::TestBasisData TestBasisData;
          /// the assembler's trial-function data type
          typedef typename AsmTraits_::TrialBasisData TrialBasisData;

        protected:
          /// the desired derivative
          int deriv;

        public:
          /**
           * \brief Constructor
           *
           * \param[in] operat
           * A reference to the Laplace operator object.
           */
          explicit Evaluator(const TestDerivativeOperator& operat) :
            deriv(operat.deriv)
          {
          }

          // copy pasted since Doxygen does not like the operator part in
          // \copydoc BilinearOperator::Evaluator::operator()
          /**
           * \brief Evaluation operator
           *
           * This operator evaluates the bilinear operator for a given combination of test- and trial-functions in
           * a single point.
           *
           * \param[in] phi
           * The trial function data in the current evaluation point. \see Space::EvalData
           *
           * \param[in] psi
           * The test function data in the current evaluation point. \see Space::EvalData
           *
           * \returns
           * The value of the bilinear functor.
           **/
          DataType operator()(const TrialBasisData& phi, const TestBasisData& psi)
          {
            return phi.value * psi.grad[deriv];
          }
        }; // class TestDerivativeOperator::Evaluator<...>

        /*
         *
         * \param[in] derivative
         * The index of the derivative for this operator:
         *  - 0: X-derivative
         *  - 1: Y-derivative
         *  - 2: Z-derivative
         *  - ...
        */
        explicit TestDerivativeOperator(int derivative) :
          deriv(derivative)
        {
        }
      }; // class TestDerivativeOperator

      /**
       * \brief div(phi) * div(psi) operator implementation
       *
       * \author Peter Zajac
       */
      class DivDivOperator :
        public Assembly::BilinearOperator
      {
      public:
        /// Row index
        int ir;
        /// Column index
        int ic;

        static constexpr TrafoTags trafo_config = TrafoTags::none;
        static constexpr SpaceTags test_config = SpaceTags::grad;
        static constexpr SpaceTags trial_config = SpaceTags::grad;

        template<typename AsmTraits_>
        class Evaluator :
          public Assembly::BilinearOperator::Evaluator<AsmTraits_>
        {
        public:
          /// the data type to be used
          typedef typename AsmTraits_::DataType DataType;
          /// the assembler's trafo data type
          typedef typename AsmTraits_::TrafoData TrafoData;
          /// the assembler's test-function data type
          typedef typename AsmTraits_::TestBasisData TestBasisData;
          /// the assembler's trial-function data type
          typedef typename AsmTraits_::TrialBasisData TrialBasisData;

        protected:
          /// Row index
          int ir;
          /// Column index
          int ic;

        public:
          /**
           * \brief Constructor
           *
           * \param[in] operat
           * A reference to the operator object.
           */
          explicit Evaluator(const DivDivOperator& operat) :
            ir(operat.ir), ic(operat.ic)
          {
          }

          /**
           * \brief Evaluation operator
           *
           * This operator evaluates the bilinear operator for a given combination of test- and trial-functions in
           * a single point.
           *
           * \param[in] phi
           * The trial function data in the current evaluation point. \see Space::EvalData
           *
           * \param[in] psi
           * The test function data in the current evaluation point. \see Space::EvalData
           *
           * \returns
           * The value of the bilinear functor.
           **/
          DataType operator()(const TrialBasisData& phi, const TestBasisData& psi)
          {
            return phi.grad[ic] * psi.grad[ir];
          }
        }; // class DivDivOperator::Evaluator<...>

        /**
         * \brief Constructor
         *
         * \param[in] ir_
         * The row index of the block
         *
         * \param[in] ic_
         * The column index of the block
         */
        explicit DivDivOperator(int ir_, int ic_) :
          ir(ir_), ic(ic_)
        {
        }
      }; // class DivDivOperator

      /**
       * \brief Du:Dv operator implementation
       *
       * This functor implements the weak formulation of the bilinear Du : Dv operator, i.e.
       * \f[
       *   \mathbf{D} \varphi : \mathbf{D} \psi
       * \f]
       * with the operator
       * \f[
       *   \mathbf{D} \varphi = \frac{1}{2} \left( \nabla \varphi + \left( \nabla \varphi \right)^T \right).
       * \f]
       *
       * Note that
       * \f[
       *   \mathbf{D} \varphi : \mathbf{D} \psi = 2 \left( \nabla \varphi : \nabla \psi + \nabla \varphi : \left( \nabla \psi \right)^T \right),
       * \f]
       * so the \f$ (k,l) \f$-block consists of entries corresponding to \f$ \partial_k \varphi \partial_l \psi \f$.
       *
       *
       * This functor can be used with the BilinearOperator assembly class template to assemble one
       * scalar matrix corresponding to one block of the \f$ d \times d \f$ block matrix, where \f$ d \f$ is the
       * number of space dimensions.
       *
       * \author Jordi Paul
       */
      class DuDvOperator :
        public BilinearOperator
      {
      public:
        /// Row index
        int ir;
        /// Column index
        int ic;

        static constexpr TrafoTags trafo_config = TrafoTags::none;
        static constexpr SpaceTags test_config = SpaceTags::grad;
        static constexpr SpaceTags trial_config = SpaceTags::grad;

        /**
         * \brief Du : Dv evaluator class template
         *
         * \tparam AsmTraits_
         * The assembly traits class.
         *
         * \author Jordi Paul
         */
        template<typename AsmTraits_>
        class Evaluator :
          public BilinearOperator::Evaluator<AsmTraits_>
        {
        public:
          /// the data type to be used
          typedef typename AsmTraits_::DataType DataType;
          /// the assembler's trafo data type
          typedef typename AsmTraits_::TrafoData TrafoData;
          /// the assembler's test-function data type
          typedef typename AsmTraits_::TestBasisData TestBasisData;
          /// the assembler's trial-function data type
          typedef typename AsmTraits_::TrialBasisData TrialBasisData;

        protected:
          /// Row index
          int ir;
          /// Column index
          int ic;

        public:
          /**
           * \brief Constructor
           *
           * \param[in] operat
           * A reference to the Du : Dv operator object.
           */
          explicit Evaluator(const DuDvOperator& operat) :
            ir(operat.ir), ic(operat.ic)
          {
          }

          /**
           * \brief Evaluation operator
           *
           * This operator evaluates the bilinear operator for a given combination of test- and trial-functions in
           * a single point.
           *
           * \param[in] phi
           * The trial function data in the current evaluation point. \see Space::EvalData
           *
           * \param[in] psi
           * The test function data in the current evaluation point. \see Space::EvalData
           *
           * \returns
           * The value of the bilinear functor.
           **/
          DataType operator()(const TrialBasisData& phi, const TestBasisData& psi)
          {
            return ((ir==ic) ? dot(phi.grad,psi.grad) : DataType(0)) + phi.grad[ir] * psi.grad[ic];
          }
        }; // class DuDvOperator::Evaluator<...>

        /**
         * \brief Constructor
         *
         * \param[in] ir_
         * The row index of the block
         *
         * \param[in] ic_
         * The column index of the block
         */
        explicit DuDvOperator(int ir_, int ic_) :
          ir(ir_), ic(ic_)
        {
        }
      }; // class DuDvOperator

      /**
       * \brief Du:Dv operator implementation
       *
       * This functor implements the weak formulation of the bilinear Du : Dv operator, i.e.
       * \f[
       *   \mathbf{D} \varphi : \mathbf{D} \psi
       * \f]
       * with the operator
       * \f[
       *   \mathbf{D} \varphi = \frac{1}{2} \left( \nabla \varphi + \left( \nabla \varphi \right)^T \right).
       * \f]
       *
       * This functor can be used with the BilinearOperator assembly class template to assemble one
       * scalar matrix corresponding to one block of the \f$ d \times d \f$ block matrix, where \f$ d \f$ is the
       * number of space dimensions.
       *
       * Note that
       * \f[
       *   \mathbf{D} \varphi : \mathbf{D} \psi = 2 \left( \nabla \varphi : \nabla \psi + \nabla \varphi : \left( \nabla \psi \right)^T \right),
       * \f]
       * so the \f$ (k,l) \f$-block consists of entries corresponding to \f$ \partial_k \varphi \partial_l \psi \f$.
       *
       * \author Jordi Paul
       */
      template<int dimension_>
      class DuDvOperatorBlocked :
        public BilinearOperator
      {
      public:
        /// Every block is a dimension x dimension matrix
        static constexpr int BlockHeight = dimension_;
        /// Every block is a dimension x dimension matrix
        static constexpr int BlockWidth = dimension_;

        static constexpr TrafoTags trafo_config = TrafoTags::none;
        static constexpr SpaceTags test_config = SpaceTags::grad;
        static constexpr SpaceTags trial_config = SpaceTags::grad;

        /**
         * \brief Du : Dv evaluator class template
         *
         * \tparam AsmTraits_
         * The assembly traits class.
         *
         * \author Jordi Paul
         */
        template<typename AsmTraits_>
        class Evaluator :
          public BilinearOperator::Evaluator<AsmTraits_>
        {
        public:
          /// the data type to be used
          typedef typename AsmTraits_::DataType DataType;
          /// the data type for the block system
          typedef typename AsmTraits_::OperatorValueType OperatorValueType;
          /// the assembler's trafo data type
          typedef typename AsmTraits_::TrafoData TrafoData;
          /// the assembler's test-function data type
          typedef typename AsmTraits_::TestBasisData TestBasisData;
          /// the assembler's trial-function data type
          typedef typename AsmTraits_::TrialBasisData TrialBasisData;

        public:
          /**
           * \brief Constructor
           *
           * \param[in] operat
           * A reference to the Du : Dv operator object.
           */
          explicit Evaluator(const DuDvOperatorBlocked& DOXY(operat))
          {
          }

          /**
           * \brief Evaluation operator
           *
           * This operator evaluates the bilinear operator for a given combination of test- and trial-functions in
           * a single point.
           *
           * \param[in] phi
           * The trial function data in the current evaluation point. \see Space::EvalData
           *
           * \param[in] psi
           * The test function data in the current evaluation point. \see Space::EvalData
           *
           * \returns
           * The value of the bilinear functor.
           **/
          OperatorValueType operator()(const TrialBasisData& phi, const TestBasisData& psi)
          {
            OperatorValueType r(DataType(0));
            for(int i(0); i < OperatorValueType::m; ++i)
            {
              r(i,i) = dot(phi.grad, psi.grad) + phi.grad[i]*psi.grad[i];
              for(int j(0); j < i; ++j)
              {
                r(i,j) = phi.grad[i] * psi.grad[j];
                r(j,i) = phi.grad[j] * psi.grad[i];
              }
            }
            return r;
          }
        }; // class DuDvOperatorBlocked::Evaluator<...>
      }; // class DuDVOperatorBlocked

      /**
       * \brief Stress-Divergence Operator
       *
       * This operator implements the stress-divergence operator \f$\nabla\cdot\sigma\f$
       * which is used in the 3-field Stokes formulation. This operator supports both symmetric
       * and unsymmetric stress fields in 2 or 3 dimensions.
       *
       * \tparam dim_
       * The dimension of the space.
       *
       * \tparam nsc_
       * The number of stress components.
       *
       * Possible valid combinations of (dim_,nsc_) are:
       * - (2,3): symmetric 2D stress field with 3 components
       * - (2,4): unsymmetric 2D stress field with 4 components
       * - (3,6): symmetric 3D stress field with 6 components
       * - (3,9): unsymmetric 3D stress field with 9 components
       *
       * \author Peter Zajac
       */
      template<int dim_, int nsc_>
      class StressDivergenceOperator :
        public Assembly::BilinearOperator
      {
      public:
        static constexpr int BlockHeight = dim_;
        static constexpr int BlockWidth = nsc_;

        static constexpr TrafoTags trafo_config = TrafoTags::none;
        static constexpr SpaceTags test_config = SpaceTags::value;
        static constexpr SpaceTags trial_config = SpaceTags::grad;

        template<typename AsmTraits_>
        class Evaluator :
          public BilinearOperator::Evaluator<AsmTraits_>
        {
        public:
          typedef typename AsmTraits_::OperatorValueType OperatorValueType;
          typedef typename AsmTraits_::TestBasisData TestBasisData;
          typedef typename AsmTraits_::TrialBasisData TrialBasisData;

        public:
          explicit Evaluator(const StressDivergenceOperator& DOXY(operat)) {}

          /// unsymmetric 2D version with 4 stress components
          template<typename T_>
          static void eval(Tiny::Matrix<T_, 2, 4, 2, 4>& R, const Tiny::Vector<T_, 2, 2>& der, const T_ u)
          {
            // u_1 = dx sigma_11 + dy sigma_12 = dx sigma_1 + dy sigma_2
            //     = (dx, dy, 0, 0) : (sigma_11, sigma_12, sigma_21, sigma_22)
            R(0,0) = u * der(0);
            R(0,1) = u * der(1);
            R(0,2) = T_(0);
            R(0,3) = T_(0);

            // u_2 = dx sigma_21 + dy sigma_22 = dx sigma_3 + dy sigma_2
            //     = (0, 0, dx, dy) : (sigma_11, sigma_12, sigma_21, sigma_22)
            R(1,0) = T_(0);
            R(1,1) = T_(0);
            R(1,2) = u * der(0);
            R(1,3) = u * der(1);
          }

          /// symmetric 2D version with 3 stress components
          template<typename T_>
          static void eval(Tiny::Matrix<T_, 2, 3, 2, 3>& R, const Tiny::Vector<T_, 2, 2>& der, const T_ u)
          {
            // we have:
            //  [ sigma_11 sigma_12 ]   [ sigma_1 sigma_3 ]
            //  [ sigma_21 sigma_22 ] = [ sigma_3 sigma_2 ]

            // u_1 = dx sigma_11 + dy sigma_12 = dx sigma_1 + dy sigma_3
            //     = (dx, 0, dy) : (sigma_11, sigma_22, sigma_12)
            R(0,0) = u * der(0);
            R(0,1) = T_(0);
            R(0,2) = u * der(1);

            // u_2 = dx sigma_21 + dy sigma_22 = dx sigma_3 + dy sigma_2
            //     = (0, dy, dx) : (sigma_11, sigma_22, sigma_12)
            R(1,0) = T_(0);
            R(1,1) = u * der(1);
            R(1,2) = u * der(0);
          }

          /// unsymmetric 3D version with 9 stress components
          template<typename T_>
          static void eval(Tiny::Matrix<T_, 3, 9, 3, 9>& R, const Tiny::Vector<T_, 3, 3>& der, const T_ u)
          {
            // u_1 = dx sigma_11 + dy sigma_12 + dz sigma_13 = dx sigma_1 + dy sigma_2 + dz sigma_3
            R(0,0) = u * der(0);
            R(0,1) = u * der(1);
            R(0,2) = u * der(2);
            R(0,3) = T_(0);
            R(0,4) = T_(0);
            R(0,5) = T_(0);
            R(0,6) = T_(0);
            R(0,7) = T_(0);
            R(0,8) = T_(0);

            // u_2 = dx sigma_21 + dy sigma_22 + dz sigma_23 = dx sigma_4 + dy sigma_5 + dz sigma_6
            R(1,0) = T_(0);
            R(1,1) = T_(0);
            R(1,2) = T_(0);
            R(1,3) = u * der(0);
            R(1,4) = u * der(1);
            R(1,5) = u * der(2);
            R(1,6) = T_(0);
            R(1,7) = T_(0);
            R(1,8) = T_(0);

            // u_3 = dx sigma_31 + dy sigma_32 + dz sigma_33 = dx sigma_7 + dy sigma_8 + dz sigma_9
            R(2,0) = T_(0);
            R(2,1) = T_(0);
            R(2,2) = T_(0);
            R(2,3) = T_(0);
            R(2,4) = T_(0);
            R(2,5) = T_(0);
            R(2,6) = u * der(0);
            R(2,7) = u * der(1);
            R(2,8) = u * der(2);
          }

          /// symmetric 3D version with 6 stress components
          template<typename T_>
          static void eval(Tiny::Matrix<T_, 3, 6, 3, 6>& R, const Tiny::Vector<T_, 3, 3>& der, const T_ u)
          {
            // we have:
            //  [ sigma_11 sigma_12 sigma_13 ]   [ sigma_1 sigma_4 sigma_6 ]
            //  [ sigma_21 sigma_22 sigma_23 ] = [ sigma_4 sigma_2 sigma_5 ]
            //  [ sigma_22 sigma_23 sigma_33 ]   [ sigma_6 sigma_5 sigma_3 ]

            // u_1 = dx sigma_11 + dy sigma_12 + dz sigma_13 = dx sigma_1 + dy sigma_4 + dz sigma_6
            R(0,0) = u * der(0);
            R(0,1) = T_(0);
            R(0,2) = T_(0);
            R(0,3) = u * der(1);
            R(0,4) = T_(0);
            R(0,5) = u * der(2);

            // u_2 = dx sigma_21 + dy sigma_22 + dz sigma_23 = dx sigma_4 + dy sigma_2 + dz sigma_5
            R(1,0) = T_(0);
            R(1,1) = u * der(1);
            R(1,2) = T_(0);
            R(1,3) = u * der(0);
            R(1,4) = u * der(2);
            R(1,5) = T_(0);

            // u_3 = dx sigma_31 + dy sigma_32 + dz sigma_33 = dx sigma_6 + dy sigma_5 + dz sigma_3
            R(2,0) = T_(0);
            R(2,1) = T_(0);
            R(2,2) = u * der(2);
            R(2,3) = T_(0);
            R(2,4) = u * der(1);
            R(2,5) = u * der(0);
          }

          OperatorValueType operator()(const TrialBasisData& phi, const TestBasisData& psi)
          {
            OperatorValueType R;
            eval(R, phi.grad, psi.value);
            return R;
          }
        }; // class StressDivergenceOperator::Evaluator
      }; // class StressDivergenceOperator

      /**
       * \brief Strain-Rate-Tensor Operator
       *
       * This operator implements the strain-rate-tensor operator \f$\mathbb{D}(u)\f$
       * which is used in the 3-field Stokes formulation. This operator supports both symmetric
       * and unsymmetric stress fields in 2 or 3 dimensions.
       *
       * \tparam dim_
       * The dimension of the space.
       *
       * \tparam nsc_
       * The number of stress components.
       *
       * Possible valid combinations of (dim_,nsc_) are:
       * - (2,3): symmetric 2D stress field with 3 components
       * - (2,4): unsymmetric 2D stress field with 4 components
       * - (3,6): symmetric 3D stress field with 6 components
       * - (3,9): unsymmetric 3D stress field with 9 components
       *
       * \author Peter Zajac
       */
      template<int dim_, int nsc_>
      class StrainRateTensorOperator :
        public Assembly::BilinearOperator
      {
      public:
        static constexpr int BlockHeight = nsc_;
        static constexpr int BlockWidth = dim_;

        static constexpr TrafoTags trafo_config = TrafoTags::none;
        static constexpr SpaceTags test_config = SpaceTags::value;
        static constexpr SpaceTags trial_config = SpaceTags::grad;

        template<typename AsmTraits_>
        class Evaluator :
          public BilinearOperator::Evaluator<AsmTraits_>
        {
        public:
          typedef typename AsmTraits_::OperatorValueType OperatorValueType;
          typedef typename AsmTraits_::TestBasisData TestBasisData;
          typedef typename AsmTraits_::TrialBasisData TrialBasisData;

        public:
          explicit Evaluator(const StrainRateTensorOperator& DOXY(operat)) {}

          /// unsymmetric 2D version with 4 stress components
          template<typename T_>
          static void eval(Tiny::Matrix<T_, 4, 2, 4, 2>& K, const Tiny::Vector<T_, 2, 2>& der, const T_ s)
          {
            // sigma_1 [11] = dx u_1 = (dx, 0) : (u_1, u_2)
            K(0,0) = s * der(0);
            K(0,1) = T_(0);

            // sigma_2 [12] = 1/2 * (dy u_1 + dx u_2) = 1/2 * (dy, dx) : (u_1, u_2)
            K(1,0) = s * der(1) / T_(2);
            K(1,1) = s * der(0) / T_(2);

            // sigma_3 [21] = 1/2 * (dy u_1 + dx u_2) = 1/2 * (dy, dx) : (u_1, u_2)
            K(2,0) = s * der(1) / T_(2);
            K(2,1) = s * der(0) / T_(2);

            // sigma_4 [22] = dy u_2 = (0, dy) : (u_1, u_2)
            K(3,0) = T_(0);
            K(3,1) = s * der(1);
          }

          /// symmetric 2D version with 3 stress components
          template<typename T_>
          static void eval(Tiny::Matrix<T_, 3, 2, 3, 2>& K, const Tiny::Vector<T_, 2, 2>& der, const T_ s)
          {
            // we have:
            //  [ sigma_11 sigma_12 ]   [ sigma_1 sigma_3 ]
            //  [ sigma_21 sigma_22 ] = [ sigma_3 sigma_2 ]

            // sigma_1 [11] = dx u_1 = (dx, 0) : (u_1, u_2)
            K(0,0) = s * der(0);
            K(0,1) = T_(0);

            // sigma_2 [22] = dy u_2 = (0, dy) : (u_1, u_2)
            K(1,0) = T_(0);
            K(1,1) = s * der(1);

            // sigma_3 [12] = 1/2 * (dy u_1 + dx u_2) = 1/2 * (dy, dx) : (u_1, u_2)
            K(2,0) = s * der(1) / T_(2);
            K(2,1) = s * der(0) / T_(2);
          }

          /// unsymmetric 3D version with 9 stress components
          template<typename T_>
          static void eval(Tiny::Matrix<T_, 9, 3, 9, 3>& K, const Tiny::Vector<T_, 3, 3>& der, const T_ s)
          {
            // sigma_1 [11] = dx u_1 = (dx, 0, 0) : (u_1, u_2, u_3)
            K(0,0) = s * der(0);
            K(0,1) = T_(0);
            K(0,2) = T_(0);

            // sigma_2 [12] = 1/2 * (dy u_1 + dx u_2) = 1/2 * (dy, dx, 0) : (u_1, u_2, u_3)
            K(1,0) = s * der(1) / T_(2);
            K(1,1) = s * der(0) / T_(2);
            K(1,2) = T_(0);

            // sigma_3 [13] = 1/2 * (dz u_1 + dx u_3) = 1/2 * (dz, 0, dx) : (u_1, u_2, u_3)
            K(2,0) = s * der(2) / T_(2);
            K(2,1) = T_(0);
            K(2,2) = s * der(0) / T_(2);

            // sigma_4 [21] = 1/2 * (dx u_2 + dy u_1) = 1/2 * (dy, dx, 0) : (u_1, u_2, u_3)
            K(3,0) = s * der(1) / T_(2);
            K(3,1) = s * der(0) / T_(2);
            K(3,2) = T_(0);

            // sigma_5 [22] = dy u_2 = (0, dy, 0) : (u_1, u_2, u_3)
            K(4,0) = T_(0);
            K(4,1) = s * der(1);
            K(4,2) = T_(0);

            // sigma_6 [23] = 1/2 * (dz u_2 + dy u_3) = 1/2 * (0, dz, dy) : (u_1, u_2, u_3)
            K(5,0) = T_(0);
            K(5,1) = s * der(2) / T_(2);
            K(5,2) = s * der(1) / T_(2);

            // sigma_7 [31] = 1/2 * (dx u_3 + dz u_1) = 1/2 * (dz, 0, dx) : (u_1, u_2, u_3)
            K(6,0) = s * der(2) / T_(2);
            K(6,1) = T_(0);
            K(6,1) = s * der(0) / T_(2);

            // sigma_8 [32] = 1/2 * (dy u_3 + dz u_2) = 1/2 * (0, dz, dy) : (u_1, u_2, u_3)
            K(7,0) = T_(0);
            K(7,1) = s * der(2) / T_(2);
            K(7,2) = s * der(1) / T_(2);

            // sigma_9 [33] = dz u3 = (0, 0, dz) : (u_1, u_2, u_3)
            K(8,0) = T_(0);
            K(8,1) = T_(0);
            K(8,2) = s * der(2);
          }

          /// symmetric 3D version with 6 stress components
          template<typename T_>
          static void eval(Tiny::Matrix<T_, 6, 3, 6, 3>& K, const Tiny::Vector<T_, 3, 3>& der, const T_ s)
          {
            // we have:
            //  [ sigma_11 sigma_12 sigma_13 ]   [ sigma_1 sigma_4 sigma_6 ]
            //  [ sigma_21 sigma_22 sigma_23 ] = [ sigma_4 sigma_2 sigma_5 ]
            //  [ sigma_22 sigma_23 sigma_33 ]   [ sigma_6 sigma_5 sigma_3 ]

            // sigma_1 [11] = dx u_1 = (dx, 0, 0) : (u_1, u_2, u_3)
            K(0,0) = s * der(0);
            K(0,1) = T_(0);
            K(0,2) = T_(0);

            // sigma_2 [22] = dy u_2 = (0, dy, 0) : (u_1, u_2, u_3)
            K(1,0) = T_(0);
            K(1,1) = s * der(1);
            K(1,2) = T_(0);

            // sigma_3 [33] = dz u3 = (0, 0, dz) : (u_1, u_2, u_3)
            K(2,0) = T_(0);
            K(2,1) = T_(0);
            K(2,2) = s * der(2);

            // sigma_4 [12] = 1/2 * (dy u_1 + dx u_2) = 1/2 * (dy, dx, 0) : (u_1, u_2, u_3)
            K(3,0) = s * der(1) / T_(2);
            K(3,1) = s * der(0) / T_(2);
            K(3,2) = T_(0);

            // sigma_5 [23] = 1/2 * (dz u_2 + dy u_3) = 1/2 * (0, dz, dy) : (u_1, u_2, u_3)
            K(4,0) = T_(0);
            K(4,1) = s * der(2) / T_(2);
            K(4,2) = s * der(1) / T_(2);

            // sigma_6 [13] = 1/2 * (dz u_1 + dx u_3) = 1/2 * (dz, 0, dx) : (u_1, u_2, u_3)
            K(5,0) = s * der(2) / T_(2);
            K(5,1) = T_(0);
            K(5,2) = s * der(0) / T_(2);
          }

          OperatorValueType operator()(const TrialBasisData& phi, const TestBasisData& psi)
          {
            OperatorValueType K;
            eval(K, phi.grad, psi.value);
            return K;
          }
        }; // class StrainRateTensorOperator::Evaluator
      }; // class StrainRateTensorOperator
    } // namespace Common
  } // namespace Assembly
} // namespace FEAT

#endif // KERNEL_ASSEMBLY_COMMON_OPERATORS_HPP
