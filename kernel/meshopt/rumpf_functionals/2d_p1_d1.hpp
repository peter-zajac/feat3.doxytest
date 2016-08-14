#pragma once
#ifndef KERNEL_MESHOPT_RUMPF_FUNCTIONALS_2D_P1_D1_HPP
#define KERNEL_MESHOPT_RUMPF_FUNCTIONALS_2D_P1_D1_HPP 1

#include <kernel/base_header.hpp>
#include <kernel/shape.hpp>
#include <kernel/util/tiny_algebra.hpp>
#include <kernel/meshopt//rumpf_functional.hpp>

namespace FEAT
{
  namespace Meshopt
  {
    /// \cond internal

    /**
     * @brief Rumpf functional, 2d P1, linear det term
     **/
    template<typename DataType_>
    class RumpfFunctional<DataType_, Shape::Simplex<2> > :
    public RumpfFunctionalBase<DataType_>
    {

      public:
        /// Our data type
        typedef DataType_ DataType;
        /// Shape type of the underlying transformation
        typedef Shape::Simplex<2> ShapeType;
        /// Our baseclass
        typedef RumpfFunctionalBase<DataType> BaseClass;
        /// Type for a pack of local vertex coordinates
        typedef Tiny::Matrix<DataType_, 3, 2> Tx;
        /// Type for the local cell sizes
        typedef Tiny::Vector<DataType_, 2> Th;
        /// Type for the gradient of the local cell sizes
        typedef Tiny::Vector<DataType_, 3*2> Tgradh;

        /**
         * \brief Constructor
         */
        explicit RumpfFunctional(
          const DataType fac_norm_,
          const DataType fac_det_,
          const DataType fac_cof_,
          const DataType fac_reg_) :
          BaseClass( fac_norm_,
          fac_det_,
          fac_det_*( Math::sqrt( Math::sqr(fac_reg_) + DataType(1) ) + Math::sqr(fac_reg_) + DataType(1)),
          fac_cof_,
          fac_reg_)
          {
          }

        /**
         * \brief The class name
         *
         * \returns String with the class name
         */
        static String name()
        {
          return "RumpfFunctional<"+ShapeType::name()+">";
        }

        /**
         * \brief Prints object parameters
         */
        void print()
        {
          Util::mpi_cout(name()+" settings:\n");
          BaseClass::print();
        }

        /**
         * \brief Computes value the Rumpf functional on one element.
         */
        DataType compute_local_functional(const Tx& x, const Th& h)
        {
          DataType norm_A = compute_norm_A(x,h);
          DataType det_A = compute_det_A(x,h);
          DataType rec_det_A = compute_rec_det_A(x,h);

          return this->_fac_norm*norm_A
            + this->_fac_det*det_A
            + this->_fac_rec_det*rec_det_A;

        }

        /**
         * \copydoc compute_local_functional()
         */
        DataType compute_local_functional(const Tx& x, const Th& h,
        DataType& func_norm,
        DataType& func_det,
        DataType& func_rec_det)
        {
          func_norm = this->_fac_norm*compute_norm_A(x,h);
          func_det = this->_fac_det*compute_det_A(x,h);
          func_rec_det = this->_fac_rec_det*compute_rec_det_A(x,h);

          return func_norm + func_det + func_rec_det;

        }

        /**
         * \brief Computes the det term on one element
         */
        /// \compilerhack icc < 16.0 gets confused by NOINLINE
#if defined(FEAT_COMPILER_INTEL) && (FEAT_COMPILER_INTEL < 1600)
        DataType compute_det_A( const Tx& x, const Th& h)
#else
        DataType NOINLINE compute_det_A( const Tx& x, const Th& h)
#endif
        {
          DataType det_;
          det_ = DataType(2) / DataType(3) * Math::sqrt(DataType(3)) * (x(0,0) * x(1,1) - x(0,0) * x(2,1) - x(1,0) * x(0,1) + x(0,1) * x(2,0) + x(1,0) * x(2,1) - x(1,1) * x(2,0)) * Math::pow(h(1), -DataType(2));
          return det_;
        }

        /**
         * \brief Computes the 1/det term on one element
         */
        /// \compilerhack icc < 16.0 gets confused by NOINLINE
#if defined(FEAT_COMPILER_INTEL) && (FEAT_COMPILER_INTEL < 1600)
        DataType compute_rec_det_A( const Tx& x, const Th& h)
#else
        DataType NOINLINE compute_rec_det_A( const Tx& x, const Th& h)
#endif
        {
          DataType rec_det_;
          rec_det_ = Math::pow(DataType(2) / DataType(3) * Math::sqrt(DataType(3)) * (x(0,0) * x(1,1) - x(0,0) * x(2,1) - x(1,0) * x(0,1) + x(0,1) * x(2,0) + x(1,0) * x(2,1) - x(1,1) * x(2,0)) * Math::pow(h(1), -DataType(2)) + Math::sqrt(DataType(9) * this->_fac_reg * this->_fac_reg + DataType(12) * Math::pow(x(0,0) * x(1,1) - x(0,0) * x(2,1) - x(1,0) * x(0,1) + x(0,1) * x(2,0) + x(1,0) * x(2,1) - x(1,1) * x(2,0), DataType(2)) * Math::pow(h(1), -DataType(4))) / DataType(3), -DataType(2));
          return rec_det_;
        }

        /**
         * \brief Computes the Frobenius norm term for one cell
         */
        /// \compilerhack icc < 16.0 gets confused by NOINLINE
#if defined(FEAT_COMPILER_INTEL) && (FEAT_COMPILER_INTEL < 1600)
        DataType compute_norm_A(const Tx& x, const Th& h)
#else
        DataType NOINLINE compute_norm_A(const Tx& x, const Th& h)
#endif
        {
          DataType norm_;
          norm_ = DataType(4) / DataType(9) * Math::pow(DataType(3) * Math::pow(h(0), DataType(2)) - DataType(2) * Math::pow(x(0,0), DataType(2)) + DataType(2) * x(0,0) * x(1,0) + DataType(2) * x(0,0) * x(2,0) - DataType(2) * Math::pow(x(0,1), DataType(2)) + DataType(2) * x(0,1) * x(1,1) + DataType(2) * x(0,1) * x(2,1) - DataType(2) * Math::pow(x(1,0), DataType(2)) + DataType(2) * x(1,0) * x(2,0) - DataType(2) * Math::pow(x(1,1), DataType(2)) + DataType(2) * x(1,1) * x(2,1) - DataType(2) * Math::pow(x(2,0), DataType(2)) - DataType(2) * Math::pow(x(2,1), DataType(2)), DataType(2)) * Math::pow(h(0), -DataType(4));
          return norm_;
        }

        /**
         * \brief Computes the functional gradient for one cell
         */
        /// \compilerhack icc < 16.0 gets confused by NOINLINE
#if defined(FEAT_COMPILER_INTEL) && (FEAT_COMPILER_INTEL < 1600)
        void compute_local_grad(const Tx& x, const Th& h, Tx& grad)
#else
        void NOINLINE compute_local_grad(const Tx& x, const Th& h, Tx& grad)
#endif
        {
          grad(0,0) = DataType(2) / DataType(3) * this->_fac_det * Math::sqrt(DataType(3)) * (x(1,1) - x(2,1)) * Math::pow(h(1), -DataType(2)) + DataType(8) / DataType(9) * this->_fac_norm * (DataType(3) * Math::pow(h(0), DataType(2)) - DataType(2) * Math::pow(x(0,0), DataType(2)) + DataType(2) * x(0,0) * x(1,0) + DataType(2) * x(0,0) * x(2,0) - DataType(2) * Math::pow(x(0,1), DataType(2)) + DataType(2) * x(0,1) * x(1,1) + DataType(2) * x(0,1) * x(2,1) - DataType(2) * Math::pow(x(1,0), DataType(2)) + DataType(2) * x(1,0) * x(2,0) - DataType(2) * Math::pow(x(1,1), DataType(2)) + DataType(2) * x(1,1) * x(2,1) - DataType(2) * Math::pow(x(2,0), DataType(2)) - DataType(2) * Math::pow(x(2,1), DataType(2))) * Math::pow(h(0), -DataType(4)) * (-DataType(4) * x(0,0) + DataType(2) * x(1,0) + DataType(2) * x(2,0)) - DataType(2) * this->_fac_rec_det * Math::pow(DataType(2) / DataType(3) * Math::sqrt(DataType(3)) * (x(0,0) * x(1,1) - x(0,0) * x(2,1) - x(1,0) * x(0,1) + x(0,1) * x(2,0) + x(1,0) * x(2,1) - x(1,1) * x(2,0)) * Math::pow(h(1), -DataType(2)) + Math::sqrt(DataType(9) * this->_fac_reg * this->_fac_reg + DataType(12) * Math::pow(x(0,0) * x(1,1) - x(0,0) * x(2,1) - x(1,0) * x(0,1) + x(0,1) * x(2,0) + x(1,0) * x(2,1) - x(1,1) * x(2,0), DataType(2)) * Math::pow(h(1), -DataType(4))) / DataType(3), -DataType(3)) * (DataType(2) / DataType(3) * Math::sqrt(DataType(3)) * (x(1,1) - x(2,1)) * Math::pow(h(1), -DataType(2)) + DataType(4) * Math::pow(DataType(9) * this->_fac_reg * this->_fac_reg + DataType(12) * Math::pow(x(0,0) * x(1,1) - x(0,0) * x(2,1) - x(1,0) * x(0,1) + x(0,1) * x(2,0) + x(1,0) * x(2,1) - x(1,1) * x(2,0), DataType(2)) * Math::pow(h(1), -DataType(4)), -DataType(1) / DataType(2)) * (x(0,0) * x(1,1) - x(0,0) * x(2,1) - x(1,0) * x(0,1) + x(0,1) * x(2,0) + x(1,0) * x(2,1) - x(1,1) * x(2,0)) * Math::pow(h(1), -DataType(4)) * (x(1,1) - x(2,1)));
          grad(0,1) = DataType(2) / DataType(3) * this->_fac_det * Math::sqrt(DataType(3)) * (-x(1,0) + x(2,0)) * Math::pow(h(1), -DataType(2)) + DataType(8) / DataType(9) * this->_fac_norm * (DataType(3) * Math::pow(h(0), DataType(2)) - DataType(2) * Math::pow(x(0,0), DataType(2)) + DataType(2) * x(0,0) * x(1,0) + DataType(2) * x(0,0) * x(2,0) - DataType(2) * Math::pow(x(0,1), DataType(2)) + DataType(2) * x(0,1) * x(1,1) + DataType(2) * x(0,1) * x(2,1) - DataType(2) * Math::pow(x(1,0), DataType(2)) + DataType(2) * x(1,0) * x(2,0) - DataType(2) * Math::pow(x(1,1), DataType(2)) + DataType(2) * x(1,1) * x(2,1) - DataType(2) * Math::pow(x(2,0), DataType(2)) - DataType(2) * Math::pow(x(2,1), DataType(2))) * Math::pow(h(0), -DataType(4)) * (-DataType(4) * x(0,1) + DataType(2) * x(1,1) + DataType(2) * x(2,1)) - DataType(2) * this->_fac_rec_det * Math::pow(DataType(2) / DataType(3) * Math::sqrt(DataType(3)) * (x(0,0) * x(1,1) - x(0,0) * x(2,1) - x(1,0) * x(0,1) + x(0,1) * x(2,0) + x(1,0) * x(2,1) - x(1,1) * x(2,0)) * Math::pow(h(1), -DataType(2)) + Math::sqrt(DataType(9) * this->_fac_reg * this->_fac_reg + DataType(12) * Math::pow(x(0,0) * x(1,1) - x(0,0) * x(2,1) - x(1,0) * x(0,1) + x(0,1) * x(2,0) + x(1,0) * x(2,1) - x(1,1) * x(2,0), DataType(2)) * Math::pow(h(1), -DataType(4))) / DataType(3), -DataType(3)) * (DataType(2) / DataType(3) * Math::sqrt(DataType(3)) * (-x(1,0) + x(2,0)) * Math::pow(h(1), -DataType(2)) + DataType(4) * Math::pow(DataType(9) * this->_fac_reg * this->_fac_reg + DataType(12) * Math::pow(x(0,0) * x(1,1) - x(0,0) * x(2,1) - x(1,0) * x(0,1) + x(0,1) * x(2,0) + x(1,0) * x(2,1) - x(1,1) * x(2,0), DataType(2)) * Math::pow(h(1), -DataType(4)), -DataType(1) / DataType(2)) * (x(0,0) * x(1,1) - x(0,0) * x(2,1) - x(1,0) * x(0,1) + x(0,1) * x(2,0) + x(1,0) * x(2,1) - x(1,1) * x(2,0)) * Math::pow(h(1), -DataType(4)) * (-x(1,0) + x(2,0)));
          grad(1,0) = DataType(2) / DataType(3) * this->_fac_det * Math::sqrt(DataType(3)) * (-x(0,1) + x(2,1)) * Math::pow(h(1), -DataType(2)) + DataType(8) / DataType(9) * this->_fac_norm * (DataType(3) * Math::pow(h(0), DataType(2)) - DataType(2) * Math::pow(x(0,0), DataType(2)) + DataType(2) * x(0,0) * x(1,0) + DataType(2) * x(0,0) * x(2,0) - DataType(2) * Math::pow(x(0,1), DataType(2)) + DataType(2) * x(0,1) * x(1,1) + DataType(2) * x(0,1) * x(2,1) - DataType(2) * Math::pow(x(1,0), DataType(2)) + DataType(2) * x(1,0) * x(2,0) - DataType(2) * Math::pow(x(1,1), DataType(2)) + DataType(2) * x(1,1) * x(2,1) - DataType(2) * Math::pow(x(2,0), DataType(2)) - DataType(2) * Math::pow(x(2,1), DataType(2))) * Math::pow(h(0), -DataType(4)) * (DataType(2) * x(0,0) - DataType(4) * x(1,0) + DataType(2) * x(2,0)) - DataType(2) * this->_fac_rec_det * Math::pow(DataType(2) / DataType(3) * Math::sqrt(DataType(3)) * (x(0,0) * x(1,1) - x(0,0) * x(2,1) - x(1,0) * x(0,1) + x(0,1) * x(2,0) + x(1,0) * x(2,1) - x(1,1) * x(2,0)) * Math::pow(h(1), -DataType(2)) + Math::sqrt(DataType(9) * this->_fac_reg * this->_fac_reg + DataType(12) * Math::pow(x(0,0) * x(1,1) - x(0,0) * x(2,1) - x(1,0) * x(0,1) + x(0,1) * x(2,0) + x(1,0) * x(2,1) - x(1,1) * x(2,0), DataType(2)) * Math::pow(h(1), -DataType(4))) / DataType(3), -DataType(3)) * (DataType(2) / DataType(3) * Math::sqrt(DataType(3)) * (-x(0,1) + x(2,1)) * Math::pow(h(1), -DataType(2)) + DataType(4) * Math::pow(DataType(9) * this->_fac_reg * this->_fac_reg + DataType(12) * Math::pow(x(0,0) * x(1,1) - x(0,0) * x(2,1) - x(1,0) * x(0,1) + x(0,1) * x(2,0) + x(1,0) * x(2,1) - x(1,1) * x(2,0), DataType(2)) * Math::pow(h(1), -DataType(4)), -DataType(1) / DataType(2)) * (x(0,0) * x(1,1) - x(0,0) * x(2,1) - x(1,0) * x(0,1) + x(0,1) * x(2,0) + x(1,0) * x(2,1) - x(1,1) * x(2,0)) * Math::pow(h(1), -DataType(4)) * (-x(0,1) + x(2,1)));
          grad(1,1) = DataType(2) / DataType(3) * this->_fac_det * Math::sqrt(DataType(3)) * (x(0,0) - x(2,0)) * Math::pow(h(1), -DataType(2)) + DataType(8) / DataType(9) * this->_fac_norm * (DataType(3) * Math::pow(h(0), DataType(2)) - DataType(2) * Math::pow(x(0,0), DataType(2)) + DataType(2) * x(0,0) * x(1,0) + DataType(2) * x(0,0) * x(2,0) - DataType(2) * Math::pow(x(0,1), DataType(2)) + DataType(2) * x(0,1) * x(1,1) + DataType(2) * x(0,1) * x(2,1) - DataType(2) * Math::pow(x(1,0), DataType(2)) + DataType(2) * x(1,0) * x(2,0) - DataType(2) * Math::pow(x(1,1), DataType(2)) + DataType(2) * x(1,1) * x(2,1) - DataType(2) * Math::pow(x(2,0), DataType(2)) - DataType(2) * Math::pow(x(2,1), DataType(2))) * Math::pow(h(0), -DataType(4)) * (DataType(2) * x(0,1) - DataType(4) * x(1,1) + DataType(2) * x(2,1)) - DataType(2) * this->_fac_rec_det * Math::pow(DataType(2) / DataType(3) * Math::sqrt(DataType(3)) * (x(0,0) * x(1,1) - x(0,0) * x(2,1) - x(1,0) * x(0,1) + x(0,1) * x(2,0) + x(1,0) * x(2,1) - x(1,1) * x(2,0)) * Math::pow(h(1), -DataType(2)) + Math::sqrt(DataType(9) * this->_fac_reg * this->_fac_reg + DataType(12) * Math::pow(x(0,0) * x(1,1) - x(0,0) * x(2,1) - x(1,0) * x(0,1) + x(0,1) * x(2,0) + x(1,0) * x(2,1) - x(1,1) * x(2,0), DataType(2)) * Math::pow(h(1), -DataType(4))) / DataType(3), -DataType(3)) * (DataType(2) / DataType(3) * Math::sqrt(DataType(3)) * (x(0,0) - x(2,0)) * Math::pow(h(1), -DataType(2)) + DataType(4) * Math::pow(DataType(9) * this->_fac_reg * this->_fac_reg + DataType(12) * Math::pow(x(0,0) * x(1,1) - x(0,0) * x(2,1) - x(1,0) * x(0,1) + x(0,1) * x(2,0) + x(1,0) * x(2,1) - x(1,1) * x(2,0), DataType(2)) * Math::pow(h(1), -DataType(4)), -DataType(1) / DataType(2)) * (x(0,0) * x(1,1) - x(0,0) * x(2,1) - x(1,0) * x(0,1) + x(0,1) * x(2,0) + x(1,0) * x(2,1) - x(1,1) * x(2,0)) * Math::pow(h(1), -DataType(4)) * (x(0,0) - x(2,0)));
          grad(2,0) = DataType(2) / DataType(3) * this->_fac_det * Math::sqrt(DataType(3)) * (x(0,1) - x(1,1)) * Math::pow(h(1), -DataType(2)) + DataType(8) / DataType(9) * this->_fac_norm * (DataType(3) * Math::pow(h(0), DataType(2)) - DataType(2) * Math::pow(x(0,0), DataType(2)) + DataType(2) * x(0,0) * x(1,0) + DataType(2) * x(0,0) * x(2,0) - DataType(2) * Math::pow(x(0,1), DataType(2)) + DataType(2) * x(0,1) * x(1,1) + DataType(2) * x(0,1) * x(2,1) - DataType(2) * Math::pow(x(1,0), DataType(2)) + DataType(2) * x(1,0) * x(2,0) - DataType(2) * Math::pow(x(1,1), DataType(2)) + DataType(2) * x(1,1) * x(2,1) - DataType(2) * Math::pow(x(2,0), DataType(2)) - DataType(2) * Math::pow(x(2,1), DataType(2))) * Math::pow(h(0), -DataType(4)) * (DataType(2) * x(0,0) + DataType(2) * x(1,0) - DataType(4) * x(2,0)) - DataType(2) * this->_fac_rec_det * Math::pow(DataType(2) / DataType(3) * Math::sqrt(DataType(3)) * (x(0,0) * x(1,1) - x(0,0) * x(2,1) - x(1,0) * x(0,1) + x(0,1) * x(2,0) + x(1,0) * x(2,1) - x(1,1) * x(2,0)) * Math::pow(h(1), -DataType(2)) + Math::sqrt(DataType(9) * this->_fac_reg * this->_fac_reg + DataType(12) * Math::pow(x(0,0) * x(1,1) - x(0,0) * x(2,1) - x(1,0) * x(0,1) + x(0,1) * x(2,0) + x(1,0) * x(2,1) - x(1,1) * x(2,0), DataType(2)) * Math::pow(h(1), -DataType(4))) / DataType(3), -DataType(3)) * (DataType(2) / DataType(3) * Math::sqrt(DataType(3)) * (x(0,1) - x(1,1)) * Math::pow(h(1), -DataType(2)) + DataType(4) * Math::pow(DataType(9) * this->_fac_reg * this->_fac_reg + DataType(12) * Math::pow(x(0,0) * x(1,1) - x(0,0) * x(2,1) - x(1,0) * x(0,1) + x(0,1) * x(2,0) + x(1,0) * x(2,1) - x(1,1) * x(2,0), DataType(2)) * Math::pow(h(1), -DataType(4)), -DataType(1) / DataType(2)) * (x(0,0) * x(1,1) - x(0,0) * x(2,1) - x(1,0) * x(0,1) + x(0,1) * x(2,0) + x(1,0) * x(2,1) - x(1,1) * x(2,0)) * Math::pow(h(1), -DataType(4)) * (x(0,1) - x(1,1)));
          grad(2,1) = DataType(2) / DataType(3) * this->_fac_det * Math::sqrt(DataType(3)) * (-x(0,0) + x(1,0)) * Math::pow(h(1), -DataType(2)) + DataType(8) / DataType(9) * this->_fac_norm * (DataType(3) * Math::pow(h(0), DataType(2)) - DataType(2) * Math::pow(x(0,0), DataType(2)) + DataType(2) * x(0,0) * x(1,0) + DataType(2) * x(0,0) * x(2,0) - DataType(2) * Math::pow(x(0,1), DataType(2)) + DataType(2) * x(0,1) * x(1,1) + DataType(2) * x(0,1) * x(2,1) - DataType(2) * Math::pow(x(1,0), DataType(2)) + DataType(2) * x(1,0) * x(2,0) - DataType(2) * Math::pow(x(1,1), DataType(2)) + DataType(2) * x(1,1) * x(2,1) - DataType(2) * Math::pow(x(2,0), DataType(2)) - DataType(2) * Math::pow(x(2,1), DataType(2))) * Math::pow(h(0), -DataType(4)) * (DataType(2) * x(0,1) + DataType(2) * x(1,1) - DataType(4) * x(2,1)) - DataType(2) * this->_fac_rec_det * Math::pow(DataType(2) / DataType(3) * Math::sqrt(DataType(3)) * (x(0,0) * x(1,1) - x(0,0) * x(2,1) - x(1,0) * x(0,1) + x(0,1) * x(2,0) + x(1,0) * x(2,1) - x(1,1) * x(2,0)) * Math::pow(h(1), -DataType(2)) + Math::sqrt(DataType(9) * this->_fac_reg * this->_fac_reg + DataType(12) * Math::pow(x(0,0) * x(1,1) - x(0,0) * x(2,1) - x(1,0) * x(0,1) + x(0,1) * x(2,0) + x(1,0) * x(2,1) - x(1,1) * x(2,0), DataType(2)) * Math::pow(h(1), -DataType(4))) / DataType(3), -DataType(3)) * (DataType(2) / DataType(3) * Math::sqrt(DataType(3)) * (-x(0,0) + x(1,0)) * Math::pow(h(1), -DataType(2)) + DataType(4) * Math::pow(DataType(9) * this->_fac_reg * this->_fac_reg + DataType(12) * Math::pow(x(0,0) * x(1,1) - x(0,0) * x(2,1) - x(1,0) * x(0,1) + x(0,1) * x(2,0) + x(1,0) * x(2,1) - x(1,1) * x(2,0), DataType(2)) * Math::pow(h(1), -DataType(4)), -DataType(1) / DataType(2)) * (x(0,0) * x(1,1) - x(0,0) * x(2,1) - x(1,0) * x(0,1) + x(0,1) * x(2,0) + x(1,0) * x(2,1) - x(1,1) * x(2,0)) * Math::pow(h(1), -DataType(4)) * (-x(0,0) + x(1,0)));

          return;
        }


        /**
         * \brief Adds the part coming from the chain rule involving h to the local gradient
         */
        /// \compilerhack icc < 16.0 gets confused by NOINLINE
#if defined(FEAT_COMPILER_INTEL) && (FEAT_COMPILER_INTEL < 1600)
        void add_grad_h_part(Tx& grad, const Tx& x, const Th& h, const Tgradh& grad_h)
#else
        void NOINLINE add_grad_h_part(Tx& grad, const Tx& x, const Th& h, const Tgradh& grad_h)
#endif
        {
          DataType der_h_(0);
          der_h_ = this->_fac_norm * (DataType(16) / DataType(3) * (DataType(3) * Math::pow(h(0), DataType(2)) - DataType(2) * Math::pow(x(0,0), DataType(2)) + DataType(2) * x(0,0) * x(1,0) + DataType(2) * x(0,0) * x(2,0) - DataType(2) * Math::pow(x(0,1), DataType(2)) + DataType(2) * x(0,1) * x(1,1) + DataType(2) * x(0,1) * x(2,1) - DataType(2) * Math::pow(x(1,0), DataType(2)) + DataType(2) * x(1,0) * x(2,0) - DataType(2) * Math::pow(x(1,1), DataType(2)) + DataType(2) * x(1,1) * x(2,1) - DataType(2) * Math::pow(x(2,0), DataType(2)) - DataType(2) * Math::pow(x(2,1), DataType(2))) * Math::pow(h(0), -DataType(3)) - DataType(16) / DataType(9) * Math::pow(DataType(3) * Math::pow(h(0), DataType(2)) - DataType(2) * Math::pow(x(0,0), DataType(2)) + DataType(2) * x(0,0) * x(1,0) + DataType(2) * x(0,0) * x(2,0) - DataType(2) * Math::pow(x(0,1), DataType(2)) + DataType(2) * x(0,1) * x(1,1) + DataType(2) * x(0,1) * x(2,1) - DataType(2) * Math::pow(x(1,0), DataType(2)) + DataType(2) * x(1,0) * x(2,0) - DataType(2) * Math::pow(x(1,1), DataType(2)) + DataType(2) * x(1,1) * x(2,1) - DataType(2) * Math::pow(x(2,0), DataType(2)) - DataType(2) * Math::pow(x(2,1), DataType(2)), DataType(2)) * Math::pow(h(0), -DataType(5))) - DataType(4) / DataType(3) * this->_fac_det * Math::sqrt(DataType(3)) * (x(0,0) * x(1,1) - x(0,0) * x(2,1) - x(1,0) * x(0,1) + x(0,1) * x(2,0) + x(1,0) * x(2,1) - x(1,1) * x(2,0)) * Math::pow(h(1), -DataType(3)) - DataType(2) * this->_fac_rec_det * Math::pow(DataType(2) / DataType(3) * Math::sqrt(DataType(3)) * (x(0,0) * x(1,1) - x(0,0) * x(2,1) - x(1,0) * x(0,1) + x(0,1) * x(2,0) + x(1,0) * x(2,1) - x(1,1) * x(2,0)) * Math::pow(h(1), -DataType(2)) + Math::sqrt(DataType(9) * this->_fac_reg * this->_fac_reg + DataType(12) * Math::pow(x(0,0) * x(1,1) - x(0,0) * x(2,1) - x(1,0) * x(0,1) + x(0,1) * x(2,0) + x(1,0) * x(2,1) - x(1,1) * x(2,0), DataType(2)) * Math::pow(h(1), -DataType(4))) / DataType(3), -DataType(3)) * (-DataType(4) / DataType(3) * Math::sqrt(DataType(3)) * (x(0,0) * x(1,1) - x(0,0) * x(2,1) - x(1,0) * x(0,1) + x(0,1) * x(2,0) + x(1,0) * x(2,1) - x(1,1) * x(2,0)) * Math::pow(h(1), -DataType(3)) - DataType(8) * Math::pow(DataType(9) * this->_fac_reg * this->_fac_reg + DataType(12) * Math::pow(x(0,0) * x(1,1) - x(0,0) * x(2,1) - x(1,0) * x(0,1) + x(0,1) * x(2,0) + x(1,0) * x(2,1) - x(1,1) * x(2,0), DataType(2)) * Math::pow(h(1), -DataType(4)), -DataType(1) / DataType(2)) * Math::pow(x(0,0) * x(1,1) - x(0,0) * x(2,1) - x(1,0) * x(0,1) + x(0,1) * x(2,0) + x(1,0) * x(2,1) - x(1,1) * x(2,0), DataType(2)) * Math::pow(h(1), -DataType(5)));

          for(int i(0); i < Tx::m; ++i)
          {
            for(int d(0); d < Tx::n; ++d)
            {
              grad(i,d) += der_h_*grad_h(i*Tx::n + d);
            }
          }
        } // add_grad_h_part

    }; // class RumpfFunctional
    extern template class RumpfFunctional<double, Shape::Simplex<2> >;
    /// \endcond
  } // namespace Meshopt
} // namespace FEAT

#endif // KERNEL_MESHOPT_RUMPF_FUNCTIONALS_2D_P1_D1_HPP
