#pragma once
#ifndef KERNEL_LAFEM_POWER_FULL_MATRIX_HPP
#define KERNEL_LAFEM_POWER_FULL_MATRIX_HPP 1

// includes, FEAST
#include <kernel/lafem/power_row_matrix.hpp>
#include <kernel/lafem/power_col_matrix.hpp>
#include <kernel/lafem/power_vector.hpp>
#include <kernel/lafem/sparse_layout.hpp>

namespace FEAST
{
  namespace LAFEM
  {
    /**
     * \brief Power-Full-Matrix meta class template
     *
     * This class template implements a composition of \e m x\e n sub-matrices of the same class.
     * This can be interpreted as an m-by-n matrix of other matrices.
     *
     * \tparam SubType_
     * The type of the sub-matrix.
     *
     * \tparam width_
     * The number of sub-matrix blocks per row.
     *
     * \tparam height_
     * The number of sub-matrix blocks per column.
     *
     * \author Peter Zajac
     */
    template<
      typename SubType_,
      Index width_,
      Index height_>
    class PowerFullMatrix
    {
      /// container-class typedef
      typedef  PowerColMatrix<PowerRowMatrix<SubType_, width_>, height_> ContClass;

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
      typedef PowerVector<typename SubMatrixType::VectorTypeL, height_> VectorTypeL;
      /// Compatible R-vector type
      typedef PowerVector<typename SubMatrixType::VectorTypeR, width_> VectorTypeR;
      /// Our 'base' class type
      template <typename Mem2_, typename DT2_ = DataType, typename IT2_ = IndexType>
      using ContainerType = class PowerFullMatrix<
        typename SubType_::template ContainerType<Mem2_, DT2_, IT2_>, width_, height_>;

      /// number of row blocks (vertical size)
      static constexpr Index num_row_blocks = height_;
      /// number of column blocks (horizontal size)
      static constexpr Index num_col_blocks = width_;

    protected:
      // the container
      ContClass _container;

    protected:
      /// base-class emplacement constructor
      explicit PowerFullMatrix(ContClass&& cont) :
        _container(std::move(cont))
      {
      }

    public:
      /// default ctor
      PowerFullMatrix()
      {
      }

      /// sub-matrix layout ctor
      explicit PowerFullMatrix(const SparseLayout<MemType, IndexType, layout_id>& layout) :
        _container(layout)
      {
      }

      /// move ctor
      PowerFullMatrix(PowerFullMatrix&& other) :
        _container(std::move(other._container))
      {
      }

      /**
       * \brief Constructor
       *
       * \param[in] mode The used file format.
       * \param[in] filename The source file.
       *
       * Creates a power-full-point-matrix based on the source file.
       */
      explicit PowerFullMatrix(FileMode mode, String filename)
      {
        CONTEXT("When creating PowerFullMatrix");

        ContClass other(mode, filename);
        _container = std::move(other);
      }

      /**
       * \brief Constructor
       *
       * \param[in] mode The used file format.
       * \param[in] file The source filestream.
       *
       * Creates a power-full-matrix based on the source filestream.
       */
      explicit PowerFullMatrix(FileMode mode, std::istream& file, String directory = "")
      {
        CONTEXT("When creating PowerFullMatrix");

        ContClass other(mode, file, directory);
        _container = std::move(other);
      }

      /**
       * \brief Read in matrix from file.
       *
       * \param[in] mode The used file format.
       * \param[in] filename The file that shall be read in.
       */
      void read_from(FileMode mode, String filename)
      {
        CONTEXT("When reading in PowerFullMatrix");

        ContClass other(mode, filename);
        _container = std::move(other);
      }

      /// move-assign operator
      PowerFullMatrix& operator=(PowerFullMatrix&& other)
      {
        if(this != &other)
        {
          _container = std::move(other._container);
        }
        return *this;
      }

      /// deleted copy-ctor
      PowerFullMatrix(const PowerFullMatrix&) = delete;
      /// deleted copy-assign operator
      PowerFullMatrix& operator=(const PowerFullMatrix&) = delete;

      /// virtual destructor
      virtual ~PowerFullMatrix()
      {
      }

      /**
       * \brief Write out matrix to file.
       *
       * \param[in] mode The used file format.
       * \param[in] filename The file where the matrix shall be stored.
       */
      void write_out(FileMode mode, String filename) const
      {
        CONTEXT("When writing out PowerFullMatrix");

        _container.write_out(mode, filename);
      }

      /**
       * \brief Creates and returns a deep copy of this matrix.
       */
      PowerFullMatrix clone() const
      {
        return PowerFullMatrix(_container.clone());
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
        static_assert(i_ < height_, "invalid sub-matrix index");
        static_assert(j_ < width_, "invalid sub-matrix index");
        return _container.template at<i_, Index(0)>().template at<Index(0),j_>();
      }

      /** \copydoc at() */
      template<Index i_, Index j_>
      const SubMatrixType& at() const
      {
        static_assert(i_ < height_, "invalid sub-matrix index");
        static_assert(j_ < width_, "invalid sub-matrix index");
        return _container.template at<i_, Index(0)>().template at<Index(0),j_>();
      }

      /// \cond internal
      Index row_blocks() const
      {
        return Index(num_row_blocks);
      }

      Index col_blocks() const
      {
        return Index(num_col_blocks);
      }

      Index get_length_of_line(const Index row) const
      {
        return _container.get_length_of_line(row);
      }

      void set_line(const Index row, DataType * const pval_set, IndexType * const pcol_set,
                    const Index col_start, const Index stride = 1) const
      {
        _container.set_line(row, pval_set, pcol_set, col_start, stride);
      }
      /// \endcond

      VectorTypeL create_vector_l() const
      {
        return _container.create_vector_l();
      }

      VectorTypeR create_vector_r() const
      {
        return _container.create_vector_r();
      }

      /// Returns the total number of rows in this matrix.
      Index rows() const
      {
        return _container.rows();
      }

      /// Returns the total number of columns in this matrix.
      Index columns() const
      {
        return _container.columns();
      }

      /// Returns the total number of non-zeros in this matrix.
      Index used_elements() const
      {
        return _container.used_elements();
      }

      /// Returns a descriptive string for this container.
      static String name()
      {
        return String("PowerFullMatrix<") + SubMatrixType::name() + "," + stringify(width_) + "," + stringify(height_) + ">";
      }

      void format(DataType value = DataType(0))
      {
        _container.format(value);
      }

      void apply(VectorTypeL& r, const VectorTypeR& x) const
      {
        _container.apply(r, x);
      }

      void apply(DenseVector<MemType, DataType, IndexType>& r, const DenseVector<MemType, DataType, IndexType>& x) const
      {
        _container.apply(r, x);
      }

      void apply(VectorTypeL& r, const VectorTypeR& x, const VectorTypeL& y, DataType alpha = DataType(1)) const
      {
        _container.apply(r, x, y, alpha);
      }

      void apply(DenseVector<MemType, DataType, IndexType>& r, const DenseVector<MemType, DataType, IndexType>& x,
                 const DenseVector<MemType, DataType, IndexType>& y, DataType alpha = DataType(1)) const
      {
        _container.apply(r, x, y, alpha);
      }

#ifdef FEAST_COMPILER_MICROSOFT
      template< typename SubType_>
      void convert(const PowerFullMatrix<SubType_, width_, height_>& other)
#else
      template <typename Mem2_, typename DT2_, typename IT2_>
      void convert(const ContainerType<Mem2_, DT2_, IT2_> & other)
#endif
      {
        CONTEXT("When converting PowerFullMatrix");
        _container.convert(other._container);
      }

      /**
       * \brief PowerFullMatrix comparison operator
       *
       * \param[in] a A matrix to compare with.
       * \param[in] b A matrix to compare with.
       */
#ifdef FEAST_COMPILER_MICROSOFT
      template< typename SubType_>
      friend bool operator== (const PowerFullMatrix & a, const PowerFullMatrix<SubType_, width_, height_> & b)
#else
      template <typename Mem2_>
      friend bool operator== (const PowerFullMatrix & a, const ContainerType<Mem2_> & b)
#endif
      {
        CONTEXT("When comparing PowerFullMatrices");

        return a._container == b._container;
      }
    }; // class PowerFullMatrix
  } // namespace LAFEM
} // namespace FEAST

#endif // KERNEL_LAFEM_POWER_FULL_MATRIX_HPP
