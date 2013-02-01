#pragma once
#ifndef KERNEL_LAFEM_DENSE_MATRIX_HPP
#define KERNEL_LAFEM_DENSE_MATRIX_HPP 1

// includes, FEAST
#include <kernel/base_header.hpp>
#include <kernel/util/assertion.hpp>
#include <kernel/lafem/container.hpp>
#include <kernel/util/graph.hpp>

#include <vector>
#include <map>
#include <algorithm>
#include <fstream>
#include <iostream>


namespace FEAST
{
  namespace LAFEM
  {
    //forward declarations
    template <typename Mem_, typename DT_>
    class SparseMatrixELL;

    template <typename Mem_, typename DT_>
    class SparseMatrixCSR;

    /**
     * \brief Coordinate based sparse matrix.
     *
     * \tparam Mem_ The memory architecture to be used.
     * \tparam DT_ The datatype to be used.
     *
     * This class represents a sparse matrix, that stores its non zero elements alongside with its coordinates explicitly.
     *
     * \author Dirk Ribbrock
     */
    template <typename Mem_, typename DT_>
    class SparseMatrixCOO : public Container<Mem_, DT_>
    {
      private:
        /// Row count.
        Index _rows;
        /// Column count.
        Index _columns;
        /// Our non zero element.
        DT_ _zero_element;
        /// Our non zero element count.
        Index _used_elements;

        /// Our non zero values.
        DT_ * _val_ptr;
        /// Our row coordinates.
        Index * _row_ptr;
        /// Our column coordinates.
        Index * _col_ptr;

        void _read_from_m(String filename)
        {
          std::ifstream file(filename.c_str(), std::ifstream::in);
          if (! file.is_open())
            throw InternalError("Unable to open Matrix file " + filename);
          _read_from_m(file);
          file.close();
        }

        void _read_from_m(std::istream& file)
        {
          std::vector<Index> rowsv;
          std::vector<Index> colsv;
          std::vector<DT_> valsv;

          String line;
          std::getline(file, line);
          while(!file.eof())
          {
            std::getline(file, line);

            if(line.find("]", 0) < line.npos)
              break;

            if(line[line.size()-1] == ';')
              line.resize(line.size()-1);

            String::size_type begin(line.find_first_not_of(" "));
            line.erase(0, begin);
            String::size_type end(line.find_first_of(" "));
            String srow(line, 0, end);
            Index row(atol(srow.c_str()));
            --row;
            _rows = std::max(row+1, _rows);
            line.erase(0, end);

            begin = line.find_first_not_of(" ");
            line.erase(0, begin);
            end = line.find_first_of(" ");
            String scol(line, 0, end);
            Index col(atol(scol.c_str()));
            --col;
            _columns = std::max(col+1, _columns);
            line.erase(0, end);

            begin = line.find_first_not_of(" ");
            line.erase(0, begin);
            end = line.find_first_of(" ");
            String sval(line, 0, end);
            DT_ val(atof(sval.c_str()));

            rowsv.push_back(row);
            colsv.push_back(col);
            valsv.push_back(val);

          }
          this->_size = this->_rows * this->_columns;

          for (Index i(0) ; i < rowsv.size() ; ++i)
          {
            (*this)(rowsv.at(i), colsv.at(i), valsv.at(i));
          }
        }

        void _read_from_coo(String filename)
        {
          std::ifstream file(filename.c_str(), std::ifstream::in | std::ifstream::binary);
          if (! file.is_open())
            throw InternalError("Unable to open Matrix file " + filename);
          _read_from_coo(file);
          file.close();
        }

        void _read_from_coo(std::istream& file)
        {
          uint64_t rows;
          uint64_t columns;
          uint64_t elements;
          file.read((char *)&rows, sizeof(uint64_t));
          file.read((char *)&columns, sizeof(uint64_t));
          file.read((char *)&elements, sizeof(uint64_t));

          this->_size = Index(rows * columns);
          _rows = Index(rows);
          _columns = Index(columns);
          _zero_element = DT_(0);
          _used_elements = elements;

          uint64_t * crow_ptr = new uint64_t[elements];
          file.read((char *)crow_ptr, (elements) * sizeof(uint64_t));
          _row_ptr = (Index*)MemoryPool<Mem::Main>::instance()->allocate_memory((elements) * sizeof(Index));
          for (Index i(0) ; i < elements ; ++i)
            _row_ptr[i] = Index(crow_ptr[i]);
          delete[] crow_ptr;

          uint64_t * ccol_ptr = new uint64_t[elements];
          file.read((char *)ccol_ptr, (elements) * sizeof(uint64_t));
          _col_ptr = (Index*)MemoryPool<Mem::Main>::instance()->allocate_memory((elements) * sizeof(Index));
          for (Index i(0) ; i < elements ; ++i)
            _col_ptr[i] = Index(ccol_ptr[i]);
          delete[] ccol_ptr;

          double * cval = new double[elements];
          file.read((char *)cval, elements * sizeof(double));
          _val_ptr = (DT_*)MemoryPool<Mem::Main>::instance()->allocate_memory((elements) * sizeof(DT_));
          for (Index i(0) ; i < elements ; ++i)
            _val_ptr[i] = DT_(cval[i]);
          delete[] cval;

          this->_elements.push_back((DT_*)MemoryPool<Mem_>::instance()->allocate_memory(elements * sizeof(DT_)));
          this->_elements_size.push_back(elements);
          this->_indices.push_back((Index*)MemoryPool<Mem_>::instance()->allocate_memory((elements)* sizeof(Index)));
          this->_indices_size.push_back(elements);
          this->_indices.push_back((Index*)MemoryPool<Mem_>::instance()->allocate_memory((elements)* sizeof(Index)));
          this->_indices_size.push_back(elements);

          MemoryPool<Mem_>::upload(this->get_elements().at(0), _val_ptr, elements * sizeof(DT_));
          MemoryPool<Mem_>::upload(this->get_indices().at(0), _row_ptr, elements * sizeof(Index));
          MemoryPool<Mem_>::upload(this->get_indices().at(1), _col_ptr, elements * sizeof(Index));
          MemoryPool<Mem::Main>::instance()->release_memory(_val_ptr);
          MemoryPool<Mem::Main>::instance()->release_memory(_row_ptr);
          MemoryPool<Mem::Main>::instance()->release_memory(_col_ptr);

          _row_ptr = this->_indices.at(0);
          _col_ptr = this->_indices.at(1);
          _val_ptr = this->_elements.at(0);
        }

      public:
        /// Our datatype
        typedef DT_ DataType;
        /// Our memory architecture type
        typedef Mem_ MemType;

        /**
         * \brief Constructor
         *
         * Creates an empty non dimensional matrix.
         */
        explicit SparseMatrixCOO() :
          Container<Mem_, DT_> (0),
          _rows(0),
          _columns(0),
          _zero_element(DT_(0)),
          _used_elements(0),
          _val_ptr(0),
          _row_ptr(0),
          _col_ptr(0)
        {
          CONTEXT("When creating SparseMatrixCOO");
        }

        /**
         * \brief Constructor
         *
         * \param[in] rows The row count of the created matrix.
         * \param[in] columns The column count of the created matrix.
         *
         * Creates a matrix with given dimensions.
         */
        explicit SparseMatrixCOO(Index rows, Index columns) :
          Container<Mem_, DT_>(rows * columns),
          _rows(rows),
          _columns(columns),
          _zero_element(DT_(0)),
          _used_elements(0),
          _val_ptr(0),
          _row_ptr(0),
          _col_ptr(0)
        {
          CONTEXT("When creating SparseMatrixCOO");
        }

        /**
         * \brief Constructor
         *
         * \param[in] other The source ell matrix.
         *
         * Creates a matrix from the given source matrix.
         */
        template <typename Mem2_>
        explicit SparseMatrixCOO(const SparseMatrixELL<Mem2_, DT_> & other) :
          Container<Mem_, DT_>(other.rows() * other.columns()),
          _rows(other.rows()),
          _columns(other.columns()),
          _zero_element(DT_(0)),
          _used_elements(other.used_elements()),
          _val_ptr(0),
          _row_ptr(0),
          _col_ptr(0)
        {
          CONTEXT("When creating SparseMatrixCOO from SparseMatrixELL");

          SparseMatrixELL<Mem::Main, DT_> cother(other);

          SparseMatrixCOO<Mem::Main, DT_> coo(_rows, _columns);

          for (Index i(0) ; i < other.num_cols_per_row() * other.stride() ; ++i)
          {
            if (cother.Ax()[i] != DT_(0))
            {
              coo(i%other.stride(), cother.Aj()[i], cother.Ax()[i]);
            }
          }

          this->_elements.push_back((DT_*)MemoryPool<Mem_>::instance()->allocate_memory(_used_elements * sizeof(DT_)));
          this->_elements_size.push_back(_used_elements);
          this->_indices.push_back((Index*)MemoryPool<Mem_>::instance()->allocate_memory(_used_elements * sizeof(Index)));
          this->_indices_size.push_back(_used_elements);
          this->_indices.push_back((Index*)MemoryPool<Mem_>::instance()->allocate_memory(_used_elements * sizeof(Index)));
          this->_indices_size.push_back(_used_elements);
          _val_ptr = this->_elements.at(0);
          _row_ptr = this->_indices.at(0);
          _col_ptr = this->_indices.at(1);

          MemoryPool<Mem_>::upload(_val_ptr, coo.val(), _used_elements * sizeof(DT_));
          MemoryPool<Mem_>::upload(_row_ptr, coo.row(), _used_elements * sizeof(Index));
          MemoryPool<Mem_>::upload(_col_ptr, coo.column(), _used_elements * sizeof(Index));
        }

        /**
         * \brief Constructor
         *
         * \param[in] other The source csr matrix.
         *
         * Creates a matrix from the given source matrix.
         */
        template <typename Mem2_>
        explicit SparseMatrixCOO(const SparseMatrixCSR<Mem2_, DT_> & other) :
          Container<Mem_, DT_>(other.rows() * other.columns()),
          _rows(other.rows()),
          _columns(other.columns()),
          _zero_element(DT_(0)),
          _used_elements(other.used_elements()),
          _val_ptr(0),
          _row_ptr(0),
          _col_ptr(0)
        {
          CONTEXT("When creating SparseMatrixCOO from SparseMatrixCSR");

          SparseMatrixCSR<Mem::Main, DT_> cother(other);

          SparseMatrixCOO<Mem::Main, DT_> coo(_rows, _columns);

          for (Index row(0) ; row < _rows ; ++row)
          {
            const Index end(cother.row_ptr_end()[row]);
            for (Index i(cother.row_ptr()[row]) ; i < end ; ++i)
            {
              if (cother.val()[i] != DT_(0))
              {
                coo(row, cother.col_ind()[i], cother.val()[i]);
              }
            }
          }

          this->_elements.push_back((DT_*)MemoryPool<Mem_>::instance()->allocate_memory(_used_elements * sizeof(DT_)));
          this->_elements_size.push_back(_used_elements);
          this->_indices.push_back((Index*)MemoryPool<Mem_>::instance()->allocate_memory(_used_elements * sizeof(Index)));
          this->_indices_size.push_back(_used_elements);
          this->_indices.push_back((Index*)MemoryPool<Mem_>::instance()->allocate_memory(_used_elements * sizeof(Index)));
          this->_indices_size.push_back(_used_elements);
          _val_ptr = this->_elements.at(0);
          _row_ptr = this->_indices.at(0);
          _col_ptr = this->_indices.at(1);

          MemoryPool<Mem_>::upload(_val_ptr, coo.val(), _used_elements * sizeof(DT_));
          MemoryPool<Mem_>::upload(_row_ptr, coo.row(), _used_elements * sizeof(Index));
          MemoryPool<Mem_>::upload(_col_ptr, coo.column(), _used_elements * sizeof(Index));
        }

        /**
         * \brief Constructor
         *
         * \param[in] graph The Graph, the matrix will be created from.
         *
         * Creates a matrix from a given graph.
         */
        explicit SparseMatrixCOO(const Graph & graph) :
          Container<Mem_, DT_>(graph.get_num_nodes_domain() * graph.get_num_nodes_image()),
          _zero_element(DT_(0)),
          _used_elements(graph.get_num_indices()),
          _val_ptr(0),
          _row_ptr(0),
          _col_ptr(0)
        {
          CONTEXT("When creating SparseMatrixCOO");

          this->_rows = graph.get_num_nodes_domain();
          this->_columns = graph.get_num_nodes_image();

          for (Index i(0) ; i < this->_rows ; ++i)
          {
            typename Graph::ImageIterator it(graph.image_begin(i));
            typename Graph::ImageIterator jt(graph.image_end(i));
            for( ; it != jt ; ++it)
            {
              (*this)(i, *it, DT_(0));
            }
          }
        }

        /**
         * \brief Constructor
         *
         * \param[in] mode The used file format.
         * \param[in] filename The source file to be read in.
         *
         * Creates a matrix based on the source file.
         */
        explicit SparseMatrixCOO(FileMode mode, String filename) :
          Container<Mem_, DT_>(0),
          _rows(0),
          _columns(0),
          _zero_element(DT_(0)),
          _val_ptr(0),
          _row_ptr(0),
          _col_ptr(0)
        {
          CONTEXT("When creating SparseMatrixCOO");

          switch(mode)
          {
            case fm_m:
              _read_from_m(filename);
              break;
            case fm_coo:
              _read_from_coo(filename);
              break;
            default:
              throw InternalError("Filemode not supported!");
          }
        }

        /**
         * \brief Constructor
         *
         * \param[in] mode The used file format.
         * \param[in] file The stream that is to be read from.
         *
         * Creates a matrix based on the source file.
         */
        explicit SparseMatrixCOO(FileMode mode, std::istream& file) :
          Container<Mem_, DT_>(0),
          _rows(0),
          _columns(0),
          _zero_element(DT_(0)),
          _val_ptr(0),
          _row_ptr(0),
          _col_ptr(0)
        {
          CONTEXT("When creating SparseMatrixCOO");

          switch(mode)
          {
            case fm_m:
              _read_from_m(file);
              break;
            case fm_coo:
              _read_from_coo(file);
              break;
            default:
              throw InternalError("Filemode not supported!");
          }
        }

        /**
         * \brief Copy Constructor
         *
         * \param[in] other The source matrix.
         *
         * Creates a shallow copy of a given matrix.
         */
        SparseMatrixCOO(const SparseMatrixCOO<Mem_, DT_> & other) :
          Container<Mem_, DT_>(other),
          _rows(other._rows),
          _columns(other._columns),
          _zero_element(other._zero_element),
          _used_elements(other.used_elements())
        {
          CONTEXT("When copying SparseMatrixCOO");

          this->_val_ptr = this->_elements.at(0);
          this->_row_ptr = this->_indices.at(0);
          this->_col_ptr = this->_indices.at(1);
        }

        /**
         * \brief Copy Constructor
         *
         * \param[in] other The source matrix.
         *
         * Creates a copy of a given matrix from another memory architecture.
         */
        template <typename Arch2_, typename DT2_>
        SparseMatrixCOO(const SparseMatrixCOO<Arch2_, DT2_> & other) :
          Container<Mem_, DT_>(other),
          _rows(other.rows()),
          _columns(other.columns()),
          _zero_element(other.zero_element()),
          _used_elements(other.used_elements())
        {
          CONTEXT("When copying SparseMatrixCOO");

          this->_val_ptr = this->_elements.at(0);
          this->_row_ptr = this->_indices.at(0);
          this->_col_ptr = this->_indices.at(1);
        }

        /** \brief Clone operation
         *
         * Creates a deep copy of this matrix.
         */
        SparseMatrixCOO<Mem_, DT_> clone()
        {
          CONTEXT("When cloning SparseMatrixCOO");

          SparseMatrixCOO<Mem_, DT_> t(this->_rows, this->_columns);
          t._elements.push_back((DT_*)MemoryPool<Mem_>::instance()->allocate_memory(_used_elements * sizeof(DT_)));
          t._elements_size.push_back(_used_elements);
          t._indices.push_back((Index*)MemoryPool<Mem_>::instance()->allocate_memory(_used_elements * sizeof(Index)));
          t._indices_size.push_back(_used_elements);
          t._indices.push_back((Index*)MemoryPool<Mem_>::instance()->allocate_memory(_used_elements * sizeof(Index)));
          t._indices_size.push_back(_used_elements);
          t._val_ptr = t._elements.at(0);
          t._row_ptr = t._indices.at(0);
          t._col_ptr = t._indices.at(1);
          t._used_elements = _used_elements;

          MemoryPool<Mem_>::copy(t._elements.at(0), _val_ptr, _used_elements * sizeof(DT_));
          MemoryPool<Mem_>::copy(t._indices.at(0), _row_ptr, _used_elements * sizeof(Index));
          MemoryPool<Mem_>::copy(t._indices.at(1), _col_ptr, _used_elements * sizeof(Index));
          return t;
        }

        /**
         * \brief Assignment operator
         *
         * \param[in] other The source matrix.
         *
         * Assigns another matrix to the target matrix.
         */
        SparseMatrixCOO<Mem_, DT_> & operator= (const SparseMatrixCOO<Mem_, DT_> & other)
        {
          CONTEXT("When assigning SparseMatrixCOO");

          if (this == &other)
            return *this;

          this->_size = other.size();
          this->_rows = other.rows();
          this->_columns = other.columns();
          this->_used_elements = other.used_elements();
          this->_zero_element = other._zero_element;

          for (Index i(0) ; i < this->_elements.size() ; ++i)
            MemoryPool<Mem_>::instance()->release_memory(this->_elements.at(i));
          for (Index i(0) ; i < this->_indices.size() ; ++i)
            MemoryPool<Mem_>::instance()->release_memory(this->_indices.at(i));

          this->_elements.clear();
          this->_indices.clear();
          this->_elements_size.clear();
          this->_indices_size.clear();

          std::vector<DT_ *> new_elements = other.get_elements();
          std::vector<Index *> new_indices = other.get_indices();

          this->_elements.assign(new_elements.begin(), new_elements.end());
          this->_indices.assign(new_indices.begin(), new_indices.end());
          this->_elements_size.assign(other.get_elements_size().begin(), other.get_elements_size().end());
          this->_indices_size.assign(other.get_indices_size().begin(), other.get_indices_size().end());

          this->_val_ptr = this->_elements.at(0);
          this->_row_ptr = this->_indices.at(0);
          this->_col_ptr = this->_indices.at(1);

          for (Index i(0) ; i < this->_elements.size() ; ++i)
            MemoryPool<Mem_>::instance()->increase_memory(this->_elements.at(i));
          for (Index i(0) ; i < this->_indices.size() ; ++i)
            MemoryPool<Mem_>::instance()->increase_memory(this->_indices.at(i));

          return *this;
        }

        /**
         * \brief Assignment operator
         *
         * \param[in] other The source matrix.
         *
         * Assigns a matrix from another memory architecture to the target matrix.
         */
        template <typename Mem2_, typename DT2_>
        SparseMatrixCOO<Mem_, DT_> & operator= (const SparseMatrixCOO<Mem2_, DT2_> & other)
        {
          CONTEXT("When assigning SparseMatrixCOO");

          this->_size = other.size();
          this->_rows = other.rows();
          this->_columns = other.columns();
          this->_used_elements = other.used_elements();
          this->_zero_element = other.zero_element();

          for (Index i(0) ; i < this->_elements.size() ; ++i)
            MemoryPool<Mem_>::instance()->release_memory(this->_elements.at(i));
          for (Index i(0) ; i < this->_indices.size() ; ++i)
            MemoryPool<Mem_>::instance()->release_memory(this->_indices.at(i));

          this->_elements.clear();
          this->_indices.clear();
          this->_elements_size.clear();
          this->_indices_size.clear();


          this->_elements.push_back((DT_*)MemoryPool<Mem_>::instance()->allocate_memory(_used_elements * sizeof(DT_)));
          this->_elements_size.push_back(_used_elements);
          this->_indices.push_back((Index*)MemoryPool<Mem_>::instance()->allocate_memory(_used_elements * sizeof(Index)));
          this->_indices_size.push_back(_used_elements);
          this->_indices.push_back((Index*)MemoryPool<Mem_>::instance()->allocate_memory(_used_elements * sizeof(Index)));
          this->_indices_size.push_back(_used_elements);

          this->_val_ptr = this->_elements.at(0);
          this->_row_ptr = this->_indices.at(0);
          this->_col_ptr = this->_indices.at(1);

          Index src_size(other.get_elements_size().at(0) * sizeof(DT2_));
          Index dest_size(other.get_elements_size().at(0) * sizeof(DT_));
          void * temp(::malloc(src_size));
          MemoryPool<Mem2_>::download(temp, other.get_elements().at(0), src_size);
          MemoryPool<Mem_>::upload(this->get_elements().at(0), temp, dest_size);
          ::free(temp);

          src_size = (other.get_indices_size().at(0) * sizeof(Index));
          dest_size = (other.get_indices_size().at(0) * sizeof(Index));
          temp = (::malloc(src_size));
          MemoryPool<Mem2_>::download(temp, other.get_indices().at(0), src_size);
          MemoryPool<Mem_>::upload(this->get_indices().at(0), temp, dest_size);
          ::free(temp);

          src_size = (other.get_indices_size().at(1) * sizeof(Index));
          dest_size = (other.get_indices_size().at(1) * sizeof(Index));
          temp = (::malloc(src_size));
          MemoryPool<Mem2_>::download(temp, other.get_indices().at(1), src_size);
          MemoryPool<Mem_>::upload(this->get_indices().at(1), temp, dest_size);
          ::free(temp);

          return *this;
        }

        /**
         * \brief Write out matrix to file.
         *
         * \param[in] mode The used file format.
         * \param[in] filename The file where the matrix shall be stored.
         */
        void write_out(FileMode mode, String filename) const
        {
          CONTEXT("When writing out SparseMatrixCOO");

          switch(mode)
          {
            case fm_coo:
              write_out_coo(filename);
              break;
            case fm_m:
              write_out_m(filename);
            default:
                throw InternalError("Filemode not supported!");
          }
        }

        /**
         * \brief Write out matrix to file.
         *
         * \param[in] mode The used file format.
         * \param[in] file The stream that shall be written to.
         */
        void write_out(FileMode mode, std::ostream& file) const
        {
          CONTEXT("When writing out SparseMatrixCOO");

          switch(mode)
          {
            case fm_coo:
              write_out_coo(file);
              break;
            case fm_m:
              write_out_m(file);
              break;
            default:
                throw InternalError("Filemode not supported!");
          }
        }

        /**
         * \brief Write out matrix to coo binary file.
         *
         * \param[in] filename The file where the matrix shall be stored.
         */
        void write_out_coo(String filename) const
        {
          std::ofstream file(filename.c_str(), std::ofstream::out | std::ofstream::binary);
          if (! file.is_open())
            throw InternalError("Unable to open Matrix file " + filename);
          write_out_coo(file);
          file.close();
        }

        /**
         * \brief Write out matrix to coo binary file.
         *
         * \param[in] file The stream that shall be written to.
         */
        void write_out_coo(std::ostream& file) const
        {
          if (typeid(DT_) != typeid(double))
            std::cout<<"Warning: You are writing out an coo matrix with less than double precission!"<<std::endl;

          Index * row_ptr = (Index*)MemoryPool<Mem::Main>::instance()->allocate_memory(this->_indices_size.at(0) * sizeof(Index));
          MemoryPool<Mem_>::download(row_ptr, _row_ptr, this->_indices_size.at(0) * sizeof(Index));
          uint64_t * crow_ptr = new uint64_t[this->_indices_size.at(0)];
          for (Index i(0) ; i < this->_indices_size.at(0) ; ++i)
            crow_ptr[i] = row_ptr[i];
          MemoryPool<Mem::Main>::instance()->release_memory(row_ptr);

          Index * col_ptr = (Index*)MemoryPool<Mem::Main>::instance()->allocate_memory(this->_indices_size.at(1) * sizeof(Index));
          MemoryPool<Mem_>::download(col_ptr, _col_ptr, this->_indices_size.at(1) * sizeof(Index));
          uint64_t * ccol_ptr = new uint64_t[this->_indices_size.at(1)];
          for (Index i(0) ; i < this->_indices_size.at(1) ; ++i)
            ccol_ptr[i] = col_ptr[i];
          MemoryPool<Mem::Main>::instance()->release_memory(col_ptr);

          DT_ * val = (DT_*)MemoryPool<Mem::Main>::instance()->allocate_memory(this->_elements_size.at(0) * sizeof(DT_));
          MemoryPool<Mem_>::download(val, _val_ptr, this->_elements_size.at(0) * sizeof(DT_));
          double * cval = new double[this->_elements_size.at(0)];
          for (Index i(0) ; i < this->_elements_size.at(0) ; ++i)
            cval[i] = val[i];
          MemoryPool<Mem::Main>::instance()->release_memory(val);

          uint64_t rows(_rows);
          uint64_t columns(_columns);
          uint64_t elements(this->_indices_size.at(0));
          file.write((const char *)&rows, sizeof(uint64_t));
          file.write((const char *)&columns, sizeof(uint64_t));
          file.write((const char *)&elements, sizeof(uint64_t));
          file.write((const char *)crow_ptr, elements * sizeof(uint64_t));
          file.write((const char *)ccol_ptr, elements * sizeof(uint64_t));
          file.write((const char *)cval, elements * sizeof(double));

          delete[] crow_ptr;
          delete[] ccol_ptr;
          delete[] cval;
        }

        /**
         * \brief Write out matrix to matlab m file.
         *
         * \param[in] filename The file where the matrix shall be stored.
         */
        void write_out_m(String filename) const
        {
          std::ofstream file(filename.c_str(), std::ofstream::out);
          if (! file.is_open())
            throw InternalError("Unable to open Matrix file " + filename);
          write_out_m(file);
          file.close();
        }

        /**
         * \brief Write out matrix to matlab m file.
         *
         * \param[in] file The stream that shall be written to.
         */
        void write_out_m(std::ostream& file) const
        {
          SparseMatrixCOO<Mem::Main, DT_> temp(*this);

          file << "data = [" << std::endl;
          for (Index i(0) ; i < _used_elements ; ++i)
          {
            file << temp.row()[i] + 1 << " " << temp.column()[i] + 1 << " " << std::scientific << (double)temp.val()[i] << ";" << std::endl;
          }
          file << "];" << std::endl;
          file << "mat=sparse(data(:,1),data(:,2),data(:,3));";
        }

        /**
         * \brief Set specific matrix element.
         *
         * \param[in] row The row of the matrix element.
         * \param[in] col The column of the matrix element.
         * \param[in] value The value to be set.
         */
        void operator()(Index row, Index col, DT_ val)
        {
          CONTEXT("When setting SparseMatrixCOO element");

          ASSERT(row < this->_rows, "Error: " + stringify(row) + " exceeds sparse matrix coo row size " + stringify(this->_rows) + " !");
          ASSERT(col < this->_columns, "Error: " + stringify(col) + " exceeds sparse matrix coo column size " + stringify(this->_columns) + " !");

          if (this->_elements.size() == 0)
          {
            this->_elements.push_back((DT_*)MemoryPool<Mem_>::instance()->allocate_memory(sizeof(DT_)));
            this->_elements_size.push_back(1);
            this->_indices.push_back((Index*)MemoryPool<Mem_>::instance()->allocate_memory(sizeof(Index)));
            this->_indices_size.push_back(1);
            this->_indices.push_back((Index*)MemoryPool<Mem_>::instance()->allocate_memory(sizeof(Index)));
            this->_indices_size.push_back(1);

            this->_val_ptr = this->_elements.at(0);
            this->_row_ptr = this->_indices.at(0);
            this->_col_ptr = this->_indices.at(1);

            MemoryPool<Mem_>::modify_element(_val_ptr, 0, val);
            MemoryPool<Mem_>::modify_element(_row_ptr, 0, row);
            MemoryPool<Mem_>::modify_element(_col_ptr, 0, col);

            _used_elements = 1;
            return;
          }

          Index i(0);
          while (i < _used_elements)
          {
            if (MemoryPool<Mem_>::get_element(_row_ptr, i) >= row)
              break;
            ++i;
          }

          while (i < _used_elements)
          {
            if (MemoryPool<Mem_>::get_element(_col_ptr,i) >= col || MemoryPool<Mem_>::get_element(_row_ptr, i) > row)
              break;
            ++i;
          }

          if(i < _used_elements && MemoryPool<Mem_>::get_element(_row_ptr, i) == row && MemoryPool<Mem_>::get_element(_col_ptr, i) == col)
          {
            MemoryPool<Mem_>::modify_element(_val_ptr, i, val);
            return;
          }
          else
          {
            DT_ * t_val((DT_*)MemoryPool<Mem_>::instance()->allocate_memory((_used_elements + 1) * sizeof(DT_)));
            Index * t_row((Index*)MemoryPool<Mem_>::instance()->allocate_memory((_used_elements + 1) * sizeof(Index)));
            Index * t_col((Index*)MemoryPool<Mem_>::instance()->allocate_memory((_used_elements + 1) * sizeof(Index)));

            if (i > 0)
            {
              MemoryPool<Mem_>::copy(t_val, _val_ptr, (i) * sizeof(DT_));
              MemoryPool<Mem_>::copy(t_row, _row_ptr, (i) * sizeof(Index));
              MemoryPool<Mem_>::copy(t_col, _col_ptr, (i) * sizeof(Index));
            }

            MemoryPool<Mem_>::modify_element(t_val, i, val);
            MemoryPool<Mem_>::modify_element(t_row, i, row);
            MemoryPool<Mem_>::modify_element(t_col, i, col);

            MemoryPool<Mem_>::copy(t_val + i + 1, _val_ptr + i, (_used_elements - i) * sizeof(DT_));
            MemoryPool<Mem_>::copy(t_row + i + 1, _row_ptr + i, (_used_elements - i) * sizeof(Index));
            MemoryPool<Mem_>::copy(t_col+ i + 1, _col_ptr + i, (_used_elements - i) * sizeof(Index));

            MemoryPool<Mem_>::instance()->release_memory(_val_ptr);
            MemoryPool<Mem_>::instance()->release_memory(_row_ptr);
            MemoryPool<Mem_>::instance()->release_memory(_col_ptr);

            this->_elements.at(0) = t_val;
            this->_indices.at(0) = t_row;
            this->_indices.at(1) = t_col;
            this->_val_ptr = this->_elements.at(0);
            this->_row_ptr = this->_indices.at(0);
            this->_col_ptr = this->_indices.at(1);
            ++_used_elements;

            this->_elements_size.at(0) = _used_elements;
            this->_indices_size.at(0) = _used_elements;
            this->_indices_size.at(1) = _used_elements;
          }
        }

        /**
         * \brief Retrieve specific matrix element.
         *
         * \param[in] row The row of the matrix element.
         * \param[in] col The column of the matrix element.
         *
         * \returns Specific matrix element.
         */
        const DT_ operator()(Index row, Index col) const
        {
          CONTEXT("When retrieving SparseMatrixCOO element");

          ASSERT(row < this->_rows, "Error: " + stringify(row) + " exceeds sparse matrix coo row size " + stringify(this->_rows) + " !");
          ASSERT(col < this->_columns, "Error: " + stringify(col) + " exceeds sparse matrix coo column size " + stringify(this->_columns) + " !");

          if (this->_elements.size() == 0)
            return _zero_element;

          Index i(0);
          while (i < _used_elements)
          {
            if (MemoryPool<Mem_>::get_element(_row_ptr, i) >= row)
              break;
            ++i;
          }

          while (i < _used_elements)
          {
            if (MemoryPool<Mem_>::get_element(_col_ptr,i) >= col || MemoryPool<Mem_>::get_element(_row_ptr, i) > row)
              break;
            ++i;
          }
          if (i == _used_elements)
            return _zero_element;

          if(MemoryPool<Mem_>::get_element(_row_ptr, i) == row && MemoryPool<Mem_>::get_element(_col_ptr, i) == col)
          {
            return MemoryPool<Mem_>::get_element(_val_ptr, i);
          }
          else
            return _zero_element;
        }

        /**
         * \brief Reset all elements to zero.
         */
        void clear()
        {
          CONTEXT("When clearing SparseMatrixCOO");
          MemoryPool<Mem_>::instance()->release_memory(_val_ptr);
          MemoryPool<Mem_>::instance()->release_memory(_row_ptr);
          MemoryPool<Mem_>::instance()->release_memory(_col_ptr);

          this->_elements.clear();
          this->_indices.clear();
          this->_elements_size.clear();
          this->_indices_size.clear();
          this->_val_ptr = 0;
          this->_row_ptr = 0;
          this->_col_ptr = 0;
          _used_elements = 0;
        }

        /**
         * \brief Retrieve matrix row count.
         *
         * \returns Matrix row count.
         */
        const Index & rows() const
        {
          return this->_rows;
        }

        /**
         * \brief Retrieve matrix column count.
         *
         * \returns Matrix column count.
         */
        const Index & columns() const
        {
          return this->_columns;
        }

        /**
         * \brief Retrieve non zero element count.
         *
         * \returns Non zero element count.
         */
        const Index & used_elements() const
        {
          return this->_used_elements;
        }

        /**
         * \brief Retrieve non zero element array.
         *
         * \returns Non zero element array.
         */
        DT_ * val() const
        {
          return _val_ptr;
        }

        /**
         * \brief Retrieve row coordinate array.
         *
         * \returns Row coordinate array.
         */
        Index * row() const
        {
          return _row_ptr;
        }

        /**
         * \brief Retrieve columns coordinate array.
         *
         * \returns Column coordinate array.
         */
        Index * column() const
        {
          return _col_ptr;
        }

        /**
         * \brief Retrieve non zero element.
         *
         * \returns Non zero element.
         */
        const DT_ zero_element() const
        {
          return _zero_element;
        }

        /**
         * \brief Returns a descriptive string.
         *
         * \returns A string describing the container.
         */
        static String type_name()
        {
          return "SparseMatrixCOO";
        }
    };

    /**
     * \brief SparseMatrixCOO comparison operator
     *
     * \param[in] a A matrix to compare with.
     * \param[in] b A matrix to compare with.
     */
    template <typename Mem_, typename Mem2_, typename DT_> bool operator== (const SparseMatrixCOO<Mem_, DT_> & a, const SparseMatrixCOO<Mem2_, DT_> & b)
    {
      CONTEXT("When comparing SparseMatrixCOOs");

      if (a.rows() != b.rows())
        return false;
      if (a.columns() != b.columns())
        return false;
      if (a.used_elements() != b.used_elements())
        return false;
      if (a.zero_element() != b.zero_element())
        return false;

      for (Index i(0) ; i < a.used_elements() ; ++i)
      {
        if (MemoryPool<Mem_>::get_element(a.val(), i) != MemoryPool<Mem2_>::get_element(b.val(), i))
            return false;
        if (MemoryPool<Mem_>::get_element(a.row(), i) != MemoryPool<Mem2_>::get_element(b.row(), i))
            return false;
        if (MemoryPool<Mem_>::get_element(a.column(), i) != MemoryPool<Mem2_>::get_element(b.column(), i))
            return false;
      }

      return true;
    }

    /**
     * \brief SparseMatrixCOO streaming operator
     *
     * \param[in] lhs The target stream.
     * \param[in] b The matrix to be streamed.
     */
    template <typename Mem_, typename DT_>
    std::ostream &
    operator<< (std::ostream & lhs, const SparseMatrixCOO<Mem_, DT_> & b)
    {
      lhs << "[" << std::endl;
      for (Index i(0) ; i < b.rows() ; ++i)
      {
        lhs << "[";
        for (Index j(0) ; j < b.columns() ; ++j)
        {
          lhs << "  " << b(i, j);
        }
        lhs << "]" << std::endl;
      }
      lhs << "]" << std::endl;

      return lhs;
    }
  } // namespace LAFEM
} // namespace FEAST

#endif // KERNEL_LAFEM_DENSE_VECTOR_HPP
