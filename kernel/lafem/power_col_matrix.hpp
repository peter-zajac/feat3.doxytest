#pragma once
#ifndef KERNEL_LAFEM_POWER_COL_MATRIX_HPP
#define KERNEL_LAFEM_POWER_COL_MATRIX_HPP 1

// includes, FEAST
#include <kernel/lafem/power_vector.hpp>
#include <kernel/lafem/sparse_layout.hpp>
#include <kernel/lafem/meta_element.hpp>

namespace FEAST
{
  namespace LAFEM
  {
    /**
     * \brief Power-Col-Matrix meta class template
     *
     * This class template implements a vertical composition of \e n sub-matrices of the same class.
     * This can be interpreted as a dense m-by-1 matrix of other matrices.
     *
     * \tparam SubType_
     * The type of the sub-matrix.
     *
     * \tparam blocks_
     * The number of sub-matrix blocks.
     *
     * \author Peter Zajac
     */
    template<
      typename SubType_,
      Index blocks_>
    class PowerColMatrix
    {
      // declare this class template as a friend for recursive inheritance
      template<typename, Index>
      friend class PowerColMatrix;

      /// rest-class typedef
      typedef PowerColMatrix<SubType_, blocks_-1> RestClass;

    public:
      /// sub-matrix type
      typedef SubType_ SubMatrixType;
      /// sub-matrix memory type
      typedef typename SubMatrixType::MemType MemType;
      /// sub-matrix data type
      typedef typename SubMatrixType::DataType DataType;
      /// sub-matrix index type
      typedef typename SubMatrixType::IndexType IndexType;
      /// sub-matrix layout type
      static constexpr SparseLayoutId layout_id = SubMatrixType::layout_id;
      /// Compatible L-vector type
      typedef PowerVector<typename SubMatrixType::VectorTypeL, blocks_> VectorTypeL;
      /// Compatible R-vector type
      typedef typename SubMatrixType::VectorTypeR VectorTypeR;
      /// Our 'base' class type
      template <typename Mem2_, typename DT2_, typename IT2_ = IndexType>
      using ContainerType = class PowerColMatrix<typename SubType_::template ContainerType<Mem2_, DT2_, IT2_>, blocks_>;

        /// number of row blocks (vertical size)
      static constexpr Index num_row_blocks = blocks_;
        /// number of column blocks (horizontal size)
      static constexpr Index num_col_blocks = 1;

    protected:
      /// the last sub-matrix
      SubMatrixType _first;
      /// the remaining part
      RestClass _rest;

      /// base-class constructor; this one is protected for a reason
      explicit PowerColMatrix(SubMatrixType&& the_first, RestClass&& the_rest) :
        _first(the_first),
        _rest(the_rest)
      {
      }

    public:
      /// default ctor
      PowerColMatrix()
      {
      }

      /// sub-matrix layout ctor
      explicit PowerColMatrix(const SparseLayout<MemType, IndexType, layout_id>& layout) :
        _first(layout),
        _rest(layout)
      {
      }

      /// move ctor
      PowerColMatrix(PowerColMatrix&& other) :
        _first(std::move(other._first)),
        _rest(std::move(other._rest))
      {
      }

      /// move-assign operator
      PowerColMatrix& operator=(PowerColMatrix&& other)
      {
        if(this != &other)
        {
          _first = std::move(other._first);
          _rest = std::move(other._rest);
        }
        return *this;
      }

      /// deleted copy-ctor
      PowerColMatrix(const PowerColMatrix&) = delete;
      /// deleted copy-assign operator
      PowerColMatrix& operator=(const PowerColMatrix&) = delete;

      /// virtual destructor
      virtual ~PowerColMatrix()
      {
      }

      /**
       * \brief Creates and returns a deep copy of this matrix.
       */
      PowerColMatrix clone() const
      {
        return PowerColMatrix(_first.clone(), _rest.clone());
      }

      /**
       * \brief Returns a sub-matrix block.
       *
       * \tparam i_
       * The row index of the sub-matrix block that is to be returned.
       *
       * \tparam j_
       * The column index of the sub-matrix block that is to be returned.
       *
       * \returns
       * A (const) reference to the sub-matrix at position <em>(i_,j_)</em>.
       */
      template<Index i_, Index j_>
      SubMatrixType& at()
      {
        static_assert(j_ == 0, "invalid sub-matrix index");
        static_assert(i_ < blocks_, "invalid sub-matrix index");
        return PowerElement<i_, SubMatrixType>::get(*this);
      }

      /** \copydoc at() */
      template<Index i_, Index j_>
      const SubMatrixType& at() const
      {
        static_assert(j_ == 0, "invalid sub-matrix index");
        static_assert(i_ < blocks_, "invalid sub-matrix index");
        return PowerElement<i_, SubMatrixType>::get(*this);
      }

      /// \cond internal
      SubMatrixType& first()
      {
        return _first;
      }

      const SubMatrixType& first() const
      {
        return _first;
      }

      RestClass& rest()
      {
        return _rest;
      }

      const RestClass& rest() const
      {
        return _rest;
      }

      Index row_blocks() const
      {
        return Index(num_row_blocks);
      }

      Index col_blocks() const
      {
        return Index(num_col_blocks);
      }
      /// \endcond

      /// Returns the total number of rows in this matrix.
      Index rows() const
      {
        return first().rows() + rest().rows();
      }

      /// Returns the total number of columns in this matrix.
      Index columns() const
      {
        return rest().columns();
      }

      /// Returns the total number of non-zeros in this matrix.
      Index used_elements() const
      {
        return first().used_elements() + rest().used_elements();
      }

      /// Returns a descriptive string for this container.
      static String name()
      {
        return String("PowerColMatrix<") + SubMatrixType::name() + "," + stringify(blocks_) + ">";
      }

      /**
       * \brief Clears this matrix.
       *
       * \param[in] value
       * The value to which the matrix' entries are to be set to.
       */
      void format(DataType value = DataType(0))
      {
        first().format(value);
        rest().format(value);
      }

      /**
       * \brief Applies this matrix onto a vector.
       *
       * This function performs
       *  \f[r \leftarrow this\cdot x \f]
       *
       * \param[out] r
       * The vector the receives the result.
       *
       * \param[in] x
       * The multiplicant vector.
       */
      template<typename Algo_>
      void apply(VectorTypeL& r, const VectorTypeR& x)
      {
        first().template apply<Algo_>(r.first(), x);
        rest().template apply<Algo_>(r.rest(), x);
      }

      /**
       * \brief Applies this matrix onto a vector.
       *
       * This function performs
       *  \f[r \leftarrow y + \alpha\cdot this\cdot x \f]
       *
       * \param[out] r
       * The vector the receives the result.
       *
       * \param[in] x
       * The multiplicant vector.
       *
       * \param[in] y
       * The summand vector
       * \param[in] alpha A scalar to scale the product with.
       */
      template<typename Algo_>
      void apply(VectorTypeL& r, const VectorTypeR& x, const VectorTypeL& y, DataType alpha = DataType(1))
      {
        first().template apply<Algo_>(r.first(), x, y.first(), alpha);
        rest().template apply<Algo_>(r.rest(), x, y.rest(), alpha);
      }

      /// Returns a new compatible L-Vector.
      VectorTypeL create_vector_l() const
      {
        return VectorTypeL(first().create_vector_l(), rest().create_vector_l());
      }

      /// Returns a new compatible R-Vector.
      VectorTypeR create_vector_r() const
      {
        return first().create_vector_r();
      }

      /// Returns the number of NNZ-elements of the selected row
      Index get_length_of_line(const Index row) const
      {
        const Index brows(this->first().rows());

        if (row < brows)
        {
          return this->first().get_length_of_line(row);
        }
        else
        {
          return this->rest().get_length_of_line(row - brows);
        }
      }

      /// \cond internal
      /// Writes the non-zero-values and matching col-indices of the selected row in allocated arrays
      void set_line(const Index row, DataType * const pval_set, IndexType * const pcol_set,
                    const Index col_start, const Index stride = 1) const
      {
        const Index brows(this->first().rows());

        if (row < brows)
        {
          this->first().set_line(row, pval_set, pcol_set, col_start, stride);
        }
        else
        {
          this->rest().set_line(row - brows, pval_set, pcol_set, col_start, stride);
        }
      }
      /// \endcond

      /**
       * \brief Convertion method
       *
       * \param[in] other The source Matrix.
       *
       * Use source matrix content as content of current matrix
       */
#ifdef FEAST_COMPILER_MICROSOFT
      template< typename SubType_>
      void convert(const PowerColMatrix<SubType_, blocks_>& other)
#else
      template <typename Mem2_, typename DT2_, typename IT2_>
      void convert(const ContainerType<Mem2_, DT2_, IT2_> & other)
#endif
      {
        CONTEXT("When converting PowerColMatrix");

        this->first().convert(other.first());
        this->rest().convert(other.rest());
      }
    };

    /// \cond internal
    template<typename SubType_>
    class PowerColMatrix<SubType_, 1>
    {
      template<typename, Index>
      friend class PowerColMatrix;

    public:
      typedef SubType_ SubMatrixType;
      typedef typename SubMatrixType::MemType MemType;
      typedef typename SubMatrixType::DataType DataType;
      typedef typename SubMatrixType::IndexType IndexType;
      /// sub-matrix layout type
      static constexpr SparseLayoutId layout_id = SubMatrixType::layout_id;
      /// Compatible L-vector type
      typedef PowerVector<typename SubMatrixType::VectorTypeL, 1> VectorTypeL;
      /// Compatible R-vector type
      typedef typename SubMatrixType::VectorTypeR VectorTypeR;
      /// Our 'base' class type
      template <typename Mem2_, typename DT2_, typename IT2_ = IndexType>
      using ContainerType = class PowerColMatrix<typename SubType_::template ContainerType<Mem2_, DT2_, IT2_>, 1>;

      static constexpr Index num_row_blocks = 1;
      static constexpr Index num_col_blocks = 1;

    protected:
      SubMatrixType _first;

      /// base-class constructor; this one is protected for a reason
      explicit PowerColMatrix(SubMatrixType&& the_first) :
        _first(the_first)
      {
      }

    public:
      /// default ctor
      PowerColMatrix()
      {
      }

      /// sub-matrix layout ctor
      explicit PowerColMatrix(const SparseLayout<MemType, IndexType, layout_id>& layout) :
        _first(layout)
      {
      }

      /// move ctor
      PowerColMatrix(PowerColMatrix&& other) :
        _first(std::move(other._first))
      {
      }

      /// move-assign operator
      PowerColMatrix& operator=(PowerColMatrix&& other)
      {
        if(this != &other)
        {
          _first = std::move(other._first);
        }
        return *this;
      }

      /// deleted copy-ctor
      PowerColMatrix(const PowerColMatrix&) = delete;
      /// deleted copy-assign operator
      PowerColMatrix& operator=(const PowerColMatrix&) = delete;

      /// virtual destructor
      virtual ~PowerColMatrix()
      {
      }

      PowerColMatrix clone() const
      {
        return PowerColMatrix(_first.clone());
      }

      template<Index i, Index j>
      SubMatrixType& at()
      {
        static_assert(i == 0, "invalid sub-matrix index");
        static_assert(j == 0, "invalid sub-matrix index");
        return _first;
      }

      template<Index i, Index j>
      const SubMatrixType& at() const
      {
        static_assert(i == 0, "invalid sub-matrix index");
        static_assert(j == 0, "invalid sub-matrix index");
        return _first;
      }

      SubMatrixType& first()
      {
        return _first;
      }

      const SubMatrixType& first() const
      {
        return _first;
      }

      Index row_blocks() const
      {
        return Index(1);
      }

      Index col_blocks() const
      {
        return Index(1);
      }

      Index rows() const
      {
        return first().rows();
      }

      Index columns() const
      {
        return first().columns();
      }

      Index used_elements() const
      {
        return first().used_elements();
      }

      void format(DataType value = DataType(0))
      {
        first().format(value);
      }

      template<typename Algo_>
      void apply(VectorTypeL& r, const VectorTypeR& x)
      {
        first().template apply<Algo_>(r.first(), x);
      }

      template<typename Algo_>
      void apply(VectorTypeL& r, const VectorTypeR& x, const VectorTypeL& y, DataType alpha = DataType(1))
      {
        first().template apply<Algo_>(r.first(), x, y.first(), alpha);
      }
      /// Returns a new compatible L-Vector.
      VectorTypeL create_vector_l() const
      {
        return VectorTypeL(first().create_vector_l());
      }

      /// Returns a new compatible R-Vector.
      VectorTypeR create_vector_r() const
      {
        return VectorTypeR(first().create_vector_r());
      }

      /// Returns the number of NNZ-elements of the selected row
      Index get_length_of_line(const Index row) const
      {
        return this->first().get_length_of_line(row);
      }

      /// Writes the non-zero-values and matching col-indices of the selected row in allocated arrays
      void set_line(const Index row, DataType * const pval_set, IndexType * const pcol_set,
                    const Index col_start, const Index stride = 1) const
      {
        this->first().set_line(row, pval_set, pcol_set, col_start, stride);
      }

      /**
       * \brief Convertion method
       *
       * \param[in] other The source Matrix.
       *
       * Use source matrix content as content of current matrix
       */
#ifdef FEAST_COMPILER_MICROSOFT
      template< typename SubType_>
      void convert(const PowerColMatrix<SubType_, Index(1)>& other)
#else
      template <typename Mem2_, typename DT2_, typename IT2_>
      void convert(const ContainerType<Mem2_, DT2_, IT2_> & other)
#endif
      {
        CONTEXT("When converting PowerColMatrix");

        this->first().convert(other.first());
      }
    };
    /// \endcond
  } // namespace LAFEM
} // namespace FEAST

#endif // KERNEL_LAFEM_POWER_COL_MATRIX_HPP
