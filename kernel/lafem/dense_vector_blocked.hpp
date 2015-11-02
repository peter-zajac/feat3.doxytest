#pragma once
#ifndef KERNEL_LAFEM_DENSE_VECTOR_BLOCKED_HPP
#define KERNEL_LAFEM_DENSE_VECTOR_BLOCKED_HPP 1

// includes, FEAST
#include <kernel/base_header.hpp>
#include <kernel/lafem/forward.hpp>
#include <kernel/util/assertion.hpp>
#include <kernel/util/type_traits.hpp>
#include <kernel/util/math.hpp>
#include <kernel/lafem/container.hpp>
#include <kernel/lafem/dense_vector.hpp>
#include <kernel/lafem/vector_base.hpp>
#include <kernel/lafem/arch/sum.hpp>
#include <kernel/lafem/arch/difference.hpp>
#include <kernel/lafem/arch/dot_product.hpp>
#include <kernel/lafem/arch/norm.hpp>
#include <kernel/lafem/arch/scale.hpp>
#include <kernel/lafem/arch/axpy.hpp>
#include <kernel/lafem/arch/component_product.hpp>
#include <kernel/lafem/arch/component_invert.hpp>
#include <kernel/util/tiny_algebra.hpp>

#include <iostream>
#include <fstream>
#include <string>
#include <stdint.h>


namespace FEAST
{
  namespace LAFEM
  {
    /**
     * \brief Blocked Dense data vector class template.
     *
     * \tparam Mem_ The memory architecture to be used.
     * \tparam DT_ The datatype to be used.
     * \tparam IT_ The indextype to be used.
     * \tparam BlockSize_ The size of the represented blocks
     *
     * This class represents a vector of continuous data in memory.\n
     * Logical, the data are organized in small blocks of BlockSize_ length.\n\n
     * Data survey: \n
     * _elements[0]: raw number values \n
     * _scalar_index[0]: container size - aka block count
     *
     * Refer to \ref lafem_design for general usage informations.
     *
     * \author Dirk Ribbrock
     */
    template <typename Mem_, typename DT_, typename IT_, int BlockSize_>
    class DenseVectorBlocked : public Container<Mem_, DT_, IT_>, public VectorBase
    {
    public:
      /// Our datatype
      typedef DT_ DataType;
      /// Our indextype
      typedef IT_ IndexType;
      /// Our memory architecture type
      typedef Mem_ MemType;
      /// Our size of a single block
      static constexpr int BlockSize = BlockSize_;
      /// Our value type
      typedef Tiny::Vector<DT_, BlockSize_> ValueType;

      /// Our 'base' class type
      template <typename Mem2_, typename DT2_ = DT_, typename IT2_ = IT_>
      using ContainerType = class DenseVectorBlocked<Mem2_, DT2_, IT2_, BlockSize_>;

      /**
       * \brief Scatter-Axpy operation for DenseVectorBlocked
       *
       * \author Peter Zajac
       */
      class ScatterAxpy
      {
      public:
        typedef LAFEM::DenseVectorBlocked<Mem::Main, DT_, IT_, BlockSize_> VectorType;
        typedef Mem::Main MemType;
        typedef DT_ DataType;
        typedef IT_ IndexType;
        typedef Tiny::Vector<DT_, BlockSize_> DataTypeBlocked;

      private:
        IT_ _num_entries;
        DataTypeBlocked* _data;

      public:
        explicit ScatterAxpy(VectorType& vector) :
          _num_entries(vector.size()),
          _data(vector.elements())
        {
        }

        template<typename LocalVector_, typename Mapping_>
        void operator()(const LocalVector_& loc_vec, const Mapping_& mapping, DT_ alpha = DT_(1))
        {
          // loop over all local entries
          for(int i(0); i < mapping.get_num_local_dofs(); ++i)
          {
            // loop over all entry contributions
            for(int ic(0); ic < mapping.get_num_contribs(i); ++ic)
            {
              // update vector data
              _data[mapping.get_index(i, ic)] += (alpha * DT_(mapping.get_weight(i, ic))) * loc_vec[i];
            }
          }
        }
      }; // class ScatterAxpy

      /**
       * \brief Gather-Axpy operation for DenseVectorBlocked
       *
       * \author Peter Zajac
       */
      class GatherAxpy
      {
      public:
        typedef LAFEM::DenseVectorBlocked<Mem::Main, DT_, IT_, BlockSize_> VectorType;
        typedef Mem::Main MemType;
        typedef DT_ DataType;
        typedef IT_ IndexType;
        typedef Tiny::Vector<DT_, BlockSize_> DataTypeBlocked;

      private:
        IT_ _num_entries;
        const DataTypeBlocked* _data;

      public:
        explicit GatherAxpy(const VectorType& vector) :
          _num_entries(vector.size()),
          _data(vector.elements())
        {
        }

        template<typename LocalVector_, typename Mapping_>
        void operator()(LocalVector_& loc_vec, const Mapping_& mapping, DT_ alpha = DT_(1))
        {
          // loop over all local entries
          for(int i(0); i < mapping.get_num_local_dofs(); ++i)
          {
            // clear accumulation entry
            DataTypeBlocked dx(DT_(0));

            // loop over all entry contributions
            for(int ic(0); ic < mapping.get_num_contribs(i); ++ic)
            {
              // update accumulator
              dx += DT_(mapping.get_weight(i, ic)) * _data[mapping.get_index(i, ic)];
            }

            // update local vector data
            loc_vec[i] += alpha * dx;
          }
        }
      }; // class GatherAxpy

    private:
      Index & _size()
      {
        return this->_scalar_index.at(0);
      }

    public:
      /**
       * \brief Constructor
       *
       * Creates an empty non dimensional vector.
       */
      explicit DenseVectorBlocked() :
        Container<Mem_, DT_, IT_> (0)
      {
        CONTEXT("When creating DenseVectorBlocked");
      }

      /**
       * \brief Constructor
       *
       * \param[in] size_in
       * The size of the created vector. aka block count
       *
       * Creates a vector with a given block count.
       */
      explicit DenseVectorBlocked(Index size_in) :
        Container<Mem_, DT_, IT_>(size_in)
      {
        CONTEXT("When creating DenseVectorBlocked");

        this->_elements.push_back(MemoryPool<Mem_>::template allocate_memory<DT_>(raw_size()));
        this->_elements_size.push_back(raw_size());
      }

      /**
       * \brief Constructor
       *
       * \param[in] size_in
       * The size of the created vector.
       *
       * \param[in] value
       * The value, each element will be set to.
       *
       * Creates a vector with given size and value.
       */
      explicit DenseVectorBlocked(Index size_in, DT_ value) :
        Container<Mem_, DT_, IT_>(size_in)
      {
        CONTEXT("When creating DenseVectorBlocked");

        this->_elements.push_back(MemoryPool<Mem_>::template allocate_memory<DT_>(raw_size()));
        this->_elements_size.push_back(raw_size());

        MemoryPool<Mem_>::set_memory(this->_elements.at(0), value, raw_size());
      }

      /**
       * \brief Constructor
       *
       * \param[in] size_in The block count of the created vector.
       * \param[in] data An array containing the value data.
       *
       * Creates a vector with given size and given data.
       */
      explicit DenseVectorBlocked(Index size_in, DT_ * data) :
        Container<Mem_, DT_, IT_>(size_in)
      {
        CONTEXT("When creating DenseVectorBlocked");

        this->_elements.push_back(data);
        this->_elements_size.push_back(raw_size());

        for (Index i(0) ; i < this->_elements.size() ; ++i)
          MemoryPool<Mem_>::increase_memory(this->_elements.at(i));
        for (Index i(0) ; i < this->_indices.size() ; ++i)
          MemoryPool<Mem_>::increase_memory(this->_indices.at(i));
      }

      /**
       * \brief Constructor
       *
       * \param[in] other The source DenseVector.
       *
       * Creates a vector from a DenseVector source
       */
      explicit DenseVectorBlocked(const DenseVector<Mem_, DT_, IT_> & other) :
        Container<Mem_, DT_, IT_>(other.size() / Index(BlockSize_))
      {
        CONTEXT("When creating DenseVectorBlocked");
        convert(other);
      }

      /**
       * \brief Constructor
       *
       * \param[in] mode The used file format.
       * \param[in] filename The source file.
       *
       * Creates a vector from the given source file.
       */
      explicit DenseVectorBlocked(FileMode mode, String filename) :
        Container<Mem_, DT_, IT_>(0)
      {
        CONTEXT("When creating DenseVectorBlocked");

        read_from(mode, filename);
      }

      /**
       * \brief Constructor
       *
       * \param[in] mode The used file format.
       * \param[in] file The stream that is to be read from.
       *
       * Creates a vector from the given source file.
       */
      explicit DenseVectorBlocked(FileMode mode, std::istream& file) :
        Container<Mem_, DT_, IT_>(0)
      {
        CONTEXT("When creating DenseVectorBlocked");

        read_from(mode, file);
      }

      /**
       * \brief Constructor
       *
       * \param[in] std::vector<char> A std::vector, containing the byte.
       *
       * Creates a vector from the given byte array.
       */
      template <typename DT2_ = DT_, typename IT2_ = IT_>
      explicit DenseVectorBlocked(std::vector<char> input) :
        Container<Mem_, DT_, IT_>(0)
      {
        CONTEXT("When creating DenseVectorBlocked");
        deserialise<DT2_, IT2_>(input);
      }

      /**
       * \brief Move Constructor
       *
       * \param[in] other The source vector.
       *
       * Moves another vector to this vector.
       */
      DenseVectorBlocked(DenseVectorBlocked && other) :
        Container<Mem_, DT_, IT_>(std::forward<DenseVectorBlocked>(other))
      {
        CONTEXT("When moving DenseVectorBlocked");
      }

      /**
       * \brief Assignment move operator
       *
       * \param[in] other The source vector.
       *
       * Moves another vector to the target vector.
       *
       * \returns The vector moved to.
       */
      DenseVectorBlocked & operator= (DenseVectorBlocked && other)
      {
        CONTEXT("When moving DenseVectorBlocked");

        this->move(std::forward<DenseVectorBlocked>(other));

        return *this;
      }

      InsertWeakClone( DenseVectorBlocked );

      /**
       * \brief Shallow copy operation
       *
       * Create a shallow copy of itself.
       *
       */
      DenseVectorBlocked shared() const
      {
        CONTEXT("When sharing DenseVectorBlocked");
        DenseVectorBlocked r;
        r.assign(*this);
        return r;
      }

      /**
       * \brief Conversion method
       *
       * \param[in] other The source vector.
       *
       * Use source vector content as content of current vector
       */
      template <typename Mem2_, typename DT2_, typename IT2_>
      void convert(const DenseVectorBlocked<Mem2_, DT2_, IT2_, BlockSize_> & other)
      {
        CONTEXT("When converting DenseVectorBlocked");
        this->assign(other);
      }

      /**
       * \brief Conversion method
       *
       * \param[in] other The source scalar vector.
       *
       * Use source scalar vector content as content of current vector
       */
      template <typename Mem2_, typename DT2_, typename IT2_>
      void convert(const DenseVector<Mem2_, DT2_, IT2_> & other)
      {
        CONTEXT("When converting DenseVectorBlocked");

        ASSERT(other.size() % Index(BlockSize_) == 0, "Error: DenseVector cannot be partionated with given blocksize!");

        this->clear();

        this->_scalar_index.push_back(other.size() / Index(BlockSize_));

        this->_elements.push_back(other.get_elements().at(0));
        this->_elements_size.push_back(raw_size());

        for (Index i(0) ; i < this->_elements.size() ; ++i)
          MemoryPool<Mem_>::increase_memory(this->_elements.at(i));
        for (Index i(0) ; i < this->_indices.size() ; ++i)
          MemoryPool<Mem_>::increase_memory(this->_indices.at(i));
      }

      /**
       * \brief Deserialisation of complete container entity.
       *
       * \param[in] std::vector<char> A std::vector, containing the byte array.
       *
       * Recreate a complete container entity by a single binary array.
       */
      template <typename DT2_ = DT_, typename IT2_ = IT_>
      void deserialise(std::vector<char> input)
      {
        this->template _deserialise<DT2_, IT2_>(FileMode::fm_dvb, input);
      }

      /**
       * \brief Serialisation of complete container entity.
       *
       * \param[in] mode FileMode enum, describing the actual container specialisation.
       * \param[out] std::vector<char> A std::vector, containing the byte array.
       *
       * Serialize a complete container entity into a single binary array.
       *
       * See \ref FEAST::LAFEM::Container::_serialise for details.
       */
      template <typename DT2_ = DT_, typename IT2_ = IT_>
      std::vector<char> serialise()
      {
        return this->template _serialise<DT2_, IT2_>(FileMode::fm_dvb);
      }

      /**
       * \brief Get a pointer to the data array.
       *
       * \returns Pointer to the data array.
       */
      Tiny::Vector<DT_, BlockSize_> * elements()
      {
        return (Tiny::Vector<DT_, BlockSize_>*)this->_elements.at(0);
      }

      /// \copydoc elements()
      /// const version.
      Tiny::Vector<DT_, BlockSize_> const * elements() const
      {
        return (Tiny::Vector<DT_, BlockSize_>*)this->_elements.at(0);
      }

      /**
       * \brief Get a pointer to the raw data array.
       *
       * \returns Pointer to the raw data array.
       */
      DT_ * raw_elements()
      {
        return this->_elements.at(0);
      }

      /// \copydoc raw_elements()
      /// const version.
      DT_ const * raw_elements() const
      {
        return this->_elements.at(0);
      }

      /// The raw number of elements of type DT_
      Index raw_size() const
      {
        return this->size() * Index(BlockSize_);
      }

      /**
       * \brief Retrieve specific vector element.
       *
       * \param[in] index The index of the vector element.
       *
       * \returns Specific Tiny::Vector element.
       */
      const Tiny::Vector<DT_, BlockSize_> operator()(Index index) const
      {
        CONTEXT("When retrieving DenseVectorBlocked element");

        ASSERT(index < this->size(), "Error: " + stringify(index) + " exceeds dense vector blocked size " + stringify(this->size()) + " !");
        Tiny::Vector<DT_, BlockSize_> t;
        MemoryPool<Mem_>::download(t.v, this->_elements.at(0) + index * Index(BlockSize_), Index(BlockSize_));
        return t;
      }

      /**
       * \brief Set specific vector element.
       *
       * \param[in] index The index of the vector element.
       * \param[in] value The value to be set.
       */
      void operator()(Index index, const Tiny::Vector<DT_, BlockSize_> & value)
      {
        CONTEXT("When setting DenseVectorBlocked element");

        ASSERT(index < this->size(), "Error: " + stringify(index) + " exceeds dense vector blocked size " + stringify(this->size()) + " !");
        MemoryPool<Mem_>::upload(this->_elements.at(0) + index * Index(BlockSize_), value.v, Index(BlockSize_));
      }

      /**
       * \brief Returns a descriptive string.
       *
       * \returns A string describing the container.
       */
      static String name()
      {
        return "DenseVectorBlocked";
      }

      /**
       * \brief Performs \f$this \leftarrow x\f$.
       *
       * \param[in] x The vector to be copied.
       */
      void copy(const DenseVectorBlocked & x)
      {
        this->_copy_content(x);
      }

      /**
       * \brief Performs \f$this \leftarrow x\f$.
       *
       * \param[in] x The vector to be copied.
       */
      template <typename Mem2_>
      void copy(const DenseVectorBlocked<Mem2_, DT_, IT_, BlockSize_> & x)
      {
        this->_copy_content(x);
      }

      /**
       * \brief Read in vector from file.
       *
       * \param[in] mode The used file format.
       * \param[in] filename The file that shall be read in.
       */
      void read_from(FileMode mode, String filename)
      {
        CONTEXT("When reading in DenseVectorBlocked");

        switch(mode)
        {
        case FileMode::fm_mtx:
          read_from_mtx(filename);
          break;
        case FileMode::fm_exp:
          read_from_exp(filename);
          break;
        case FileMode::fm_dvb:
          read_from_dvb(filename);
          break;
        default:
          throw InternalError(__func__, __FILE__, __LINE__, "Filemode not supported!");
        }
      }

      /**
       * \brief Read in vector from stream.
       *
       * \param[in] mode The used file format.
       * \param[in] file The stream that shall be read in.
       */
      void read_from(FileMode mode, std::istream& file)
      {
        CONTEXT("When reading in DenseVectorBlocked");

        switch(mode)
        {
        case FileMode::fm_mtx:
          read_from_mtx(file);
          break;
        case FileMode::fm_exp:
          read_from_exp(file);
          break;
        case FileMode::fm_dvb:
          read_from_dvb(file);
          break;
        default:
          throw InternalError(__func__, __FILE__, __LINE__, "Filemode not supported!");
        }
      }

      /**
       * \brief Read in vector from MatrixMarket mtx file.
       *
       * \param[in] filename The file that shall be read in.
       */
      void read_from_mtx(String filename)
      {
        std::ifstream file(filename.c_str(), std::ifstream::in);
        if (! file.is_open())
          throw InternalError(__func__, __FILE__, __LINE__, "Unable to open Vector file " + filename);
        read_from_mtx(file);
        file.close();
      }

      /**
       * \brief Read in vector from MatrixMarket mtx stream.
       *
       * \param[in] file The stream that shall be read in.
       */
      void read_from_mtx(std::istream& file)
      {
        this->clear();
        this->_scalar_index.push_back(0);

        Index rows;
        String line;
        std::getline(file, line);
        if (line.find("%%MatrixMarket matrix array real general") == String::npos)
        {
          throw InternalError(__func__, __FILE__, __LINE__, "Input-file is not a compatible mtx-vector-file");
        }
        while(!file.eof())
        {
          std::getline(file,line);
          if (file.eof())
            throw InternalError(__func__, __FILE__, __LINE__, "Input-file is empty");

          String::size_type begin(line.find_first_not_of(" "));
          if (line.at(begin) != '%')
            break;
        }
        {
          String::size_type begin(line.find_first_not_of(" "));
          line.erase(0, begin);
          String::size_type end(line.find_first_of(" "));
          String srows(line, 0, end);
          rows = (Index)atol(srows.c_str());
          line.erase(0, end);

          begin = line.find_first_not_of(" ");
          line.erase(0, begin);
          end = line.find_first_of(" ");
          String scols(line, 0, end);
          Index cols((Index)atol(scols.c_str()));
          line.erase(0, end);
          if (cols != 1)
            throw InternalError(__func__, __FILE__, __LINE__, "Input-file is no dense-vector-file");
        }

        DenseVectorBlocked<Mem::Main, DT_, IT_, BlockSize_> tmp(rows / BlockSize_);
        DT_ * pval(tmp.raw_elements());

        while(!file.eof())
        {
          std::getline(file, line);
          if (file.eof())
            break;

          String::size_type begin(line.find_first_not_of(" "));
          line.erase(0, begin);
          String::size_type end(line.find_first_of(" "));
          String sval(line, 0, end);
          DT_ tval((DT_)atof(sval.c_str()));

          *pval = tval;
          ++pval;
        }
        this->assign(tmp);
      }

      /**
       * \brief Read in vector from ASCII file.
       *
       * \param[in] filename The file that shall be read in.
       */
      void read_from_exp(String filename)
      {
        std::ifstream file(filename.c_str(), std::ifstream::in);
        if (! file.is_open())
          throw InternalError(__func__, __FILE__, __LINE__, "Unable to open Vector file " + filename);
        read_from_exp(file);
        file.close();
      }

      /**
       * \brief Read in vector from ASCII stream.
       *
       * \param[in] file The stream that shall be read in.
       */
      void read_from_exp(std::istream& file)
      {
        this->clear();
        this->_scalar_index.push_back(0);

        std::vector<DT_> data;

        while(!file.eof())
        {
          std::string line;
          std::getline(file, line);
          if(line.find("#", 0) < line.npos)
            continue;
          if(file.eof())
            break;

          std::string n_z_s;

          std::string::size_type first_digit(line.find_first_not_of(" "));
          line.erase(0, first_digit);
          std::string::size_type eol(line.length());
          for(unsigned long i(0) ; i < eol ; ++i)
          {
            n_z_s.append(1, line[i]);
          }

          DT_ n_z((DT_)atof(n_z_s.c_str()));

          data.push_back(n_z);

        }

        _size() = Index(data.size()) / Index(BlockSize_);
        this->_elements.push_back(MemoryPool<Mem_>::template allocate_memory<DT_>(Index(data.size())));
        this->_elements_size.push_back(Index(data.size()));
        MemoryPool<Mem_>::template upload<DT_>(this->_elements.at(0), &data[0], Index(data.size()));
      }

      /**
       * \brief Read in vector from binary file.
       *
       * \param[in] filename The file that shall be read in.
       */
      void read_from_dvb(String filename)
      {
        std::ifstream file(filename.c_str(), std::ifstream::in | std::ifstream::binary);
        if (! file.is_open())
          throw InternalError(__func__, __FILE__, __LINE__, "Unable to open Vector file " + filename);
        read_from_dvb(file);
        file.close();
      }

      /**
       * \brief Read in vector from binary stream.
       *
       * \param[in] file The stream that shall be read in.
       */
      void read_from_dvb(std::istream& file)
      {
        this->clear();
        this->_scalar_index.push_back(0);
        this->_scalar_index.push_back(0);

        this->template _deserialise<double, uint64_t>(FileMode::fm_dvb, file);
      }

      /**
       * \brief Write out vector to file.
       *
       * \param[in] mode The used file format.
       * \param[in] filename The file where the vector shall be stored.
       */
      void write_out(FileMode mode, String filename) const
      {
        CONTEXT("When writing out DenseVectorBlocked");

        switch(mode)
        {
        case FileMode::fm_mtx:
          write_out_mtx(filename);
          break;
        case FileMode::fm_exp:
          write_out_exp(filename);
          break;
        case FileMode::fm_dvb:
          write_out_dvb(filename);
          break;
        default:
          throw InternalError(__func__, __FILE__, __LINE__, "Filemode not supported!");
        }
      }

      /**
       * \brief Write out vector to file.
       *
       * \param[in] mode The used file format.
       * \param[in] file The stream that shall be written to.
       */
      void write_out(FileMode mode, std::ostream& file) const
      {
        CONTEXT("When writing out DenseVectorBlocked");

        switch(mode)
        {
        case FileMode::fm_mtx:
          write_out_mtx(file);
          break;
        case FileMode::fm_exp:
          write_out_exp(file);
          break;
        case FileMode::fm_dvb:
          write_out_dvb(file);
          break;
        default:
          throw InternalError(__func__, __FILE__, __LINE__, "Filemode not supported!");
        }
      }

      /**
       * \brief Write out vector to MatrixMarket mtx file.
       *
       * \param[in] filename The file where the vector shall be stored.
       */
      void write_out_mtx(String filename) const
      {
        std::ofstream file(filename.c_str(), std::ofstream::out);
        if (! file.is_open())
          throw InternalError(__func__, __FILE__, __LINE__, "Unable to open Vector file " + filename);
        write_out_mtx(file);
        file.close();
      }

      /**
       * \brief Write out vector to MatrixMarket mtx file.
       *
       * \param[in] file The stream that shall be written to.
       */
      void write_out_mtx(std::ostream& file) const
      {
        DenseVectorBlocked<Mem::Main, DT_, IT_, BlockSize_> temp;
        temp.convert(*this);

        const Index tsize(temp.raw_size());
        file << "%%MatrixMarket matrix array real general" << std::endl;
        file << tsize << " " << 1 << std::endl;

        const DT_ * pval(temp.raw_elements());
        for (Index i(0) ; i < tsize ; ++i, ++pval)
        {
          file << std::scientific << *pval << std::endl;
        }
      }

      /**
       * \brief Write out vector to file.
       *
       * \param[in] filename The file where the vector shall be stored.
       */
      void write_out_exp(String filename) const
      {
        std::ofstream file(filename.c_str(), std::ofstream::out);
        if (! file.is_open())
          throw InternalError(__func__, __FILE__, __LINE__, "Unable to open Vector file " + filename);
        write_out_exp(file);
        file.close();
      }

      /**
       * \brief Write out vector to file.
       *
       * \param[in] file The stream that shall be written to.
       */
      void write_out_exp(std::ostream& file) const
      {
        DT_ * temp = MemoryPool<Mem::Main>::template allocate_memory<DT_>((this->raw_size()));
        MemoryPool<Mem_>::template download<DT_>(temp, this->raw_elements(), this->raw_size());

        for (Index i(0) ; i < this->raw_size() ; ++i)
        {
          file << std::scientific << temp[i] << std::endl;
        }

        MemoryPool<Mem::Main>::release_memory(temp);
      }

      /**
       * \brief Write out vector to file.
       *
       * \param[in] filename The file where the vector shall be stored.
       */
      void write_out_dvb(String filename) const
      {
        std::ofstream file(filename.c_str(), std::ofstream::out | std::ofstream::binary);
        if (! file.is_open())
          throw InternalError(__func__, __FILE__, __LINE__, "Unable to open Vector file " + filename);
        write_out_dvb(file);
        file.close();
      }

      /**
       * \brief Write out vector to file.
       *
       * \param[in] file The stream that shall be written to.
       */
      void write_out_dvb(std::ostream& file) const
      {
        if (! std::is_same<DT_, double>::value)
          std::cout<<"Warning: You are writing out a dense vector that is not double precision!"<<std::endl;

        this->template _serialise<double, uint64_t>(FileMode::fm_dvb, file);
      }

      ///@name Linear algebra operations
      ///@{
      /**
       * \brief Calculate \f$this \leftarrow \alpha x + y\f$
       *
       * \param[in] x The first summand vector to be scaled.
       * \param[in] y The second summand vector
       * \param[in] alpha A scalar to multiply x with.
       */
      void axpy(
                const DenseVectorBlocked & x,
                const DenseVectorBlocked & y,
                const DT_ alpha = DT_(1))
      {
        if (x.size() != y.size())
          throw InternalError(__func__, __FILE__, __LINE__, "Vector size does not match!");
        if (x.size() != this->size())
          throw InternalError(__func__, __FILE__, __LINE__, "Vector size does not match!");

        // check for special cases
        // r <- x + y
        if(Math::abs(alpha - DT_(1)) < Math::eps<DT_>())
          Arch::Sum<Mem_>::value(raw_elements(), x.raw_elements(), y.raw_elements(), this->raw_size());
        // r <- y - x
        else if(Math::abs(alpha + DT_(1)) < Math::eps<DT_>())
          Arch::Difference<Mem_>::value(raw_elements(), y.raw_elements(), x.raw_elements(), this->raw_size());
        // r <- y
        else if(Math::abs(alpha) < Math::eps<DT_>())
          this->copy(y);
        // r <- y + alpha*x
        else
          Arch::Axpy<Mem_>::dv(raw_elements(), alpha, x.raw_elements(), y.raw_elements(), this->raw_size());
      }

      /**
       * \brief Calculate \f$this_i \leftarrow x_i \cdot y_i\f$
       *
       * \param[in] x The first factor.
       * \param[in] y The second factor.
       */
      void component_product(const DenseVectorBlocked & x, const DenseVectorBlocked & y)
      {
        if (this->size() != x.size())
          throw InternalError(__func__, __FILE__, __LINE__, "Vector size does not match!");
        if (this->size() != y.size())
          throw InternalError(__func__, __FILE__, __LINE__, "Vector size does not match!");

        Arch::ComponentProduct<Mem_>::value(raw_elements(), x.raw_elements(), y.raw_elements(), this->raw_size());
      }

      /**
       * \brief Performs \f$ this_i \leftarrow \alpha / x_i \f$
       *
       * \param[in] x
       * The vector whose components serve as denominators.
       *
       * \param[in] alpha
       * The nominator.
       */
      void component_invert(const DenseVectorBlocked & x, const DT_ alpha = DT_(1))
      {
        if (this->size() != x.size())
          throw InternalError(__func__, __FILE__, __LINE__, "Vector size does not match!");

        Arch::ComponentInvert<Mem_>::value(this->raw_elements(), x.raw_elements(), alpha, this->raw_size());
      }

      /**
       * \brief Calculate \f$this \leftarrow \alpha x \f$
       *
       * \param[in] x The vector to be scaled.
       * \param[in] alpha A scalar to scale x with.
       */
      void scale(const DenseVectorBlocked & x, const DT_ alpha)
      {
        if (x.size() != this->size())
          throw InternalError(__func__, __FILE__, __LINE__, "Vector size does not match!");

        Arch::Scale<Mem_>::value(raw_elements(), x.raw_elements(), alpha, this->raw_size());
      }

      /**
       * \brief Calculate \f$this \leftarrow this \cdot x\f$
       *
       * \param[in] x The other vector.
       *
       * \return The calculated dot product.
       */
      DataType dot(const DenseVectorBlocked & x) const
      {
        if (x.size() != this->size())
          throw InternalError(__func__, __FILE__, __LINE__, "Vector size does not match!");

        return Arch::DotProduct<Mem_>::value(raw_elements(), x.raw_elements(), this->raw_size());
      }

      /**
       * \brief Calculates and returns the euclid norm of this vector.
       *
       * \return The calculated norm.
       */
      DT_ norm2() const
      {
        return Arch::Norm2<Mem_>::value(raw_elements(), this->raw_size());
      }

      /**
       * \brief Calculates and returns the squared euclid norm of this vector.
       *
       * \return The calculated norm.
       */
      DT_ norm2sqr() const
      {
        // fallback
        return Math::sqr(this->norm2());
      }
      ///@}

      /**
       * \brief DenseVectorBlocked comparison operator
       *
       * \param[in] a A vector to compare with.
       * \param[in] b A vector to compare with.
       */
      template <typename Mem2_> friend bool operator== (const DenseVectorBlocked & a, const DenseVectorBlocked<Mem2_, DT_, IT_, BlockSize_> & b)
      {
        CONTEXT("When comparing DenseVectorBlockeds");

        if (a.size() != b.size())
          return false;
        if (a.get_elements().size() != b.get_elements().size())
          return false;
        if (a.get_indices().size() != b.get_indices().size())
          return false;

        if(a.size() == 0 && b.size() == 0 && a.get_elements().size() == 0 && b.get_elements().size() == 0)
          return true;

        bool ret(true);

        DT_ * ta;
        DT_ * tb;

        if(std::is_same<Mem::Main, Mem_>::value)
          ta = (DT_*)a.elements();
        else
        {
          ta = new DT_[a.raw_size()];
          MemoryPool<Mem_>::template download<DT_>(ta, a.raw_elements(), a.raw_size());
        }
        if(std::is_same<Mem::Main, Mem2_>::value)
          tb = (DT_*)b.elements();
        else
        {
          tb = new DT_[b.raw_size()];
          MemoryPool<Mem2_>::template download<DT_>(tb, b.raw_elements(), b.raw_size());
        }

        for (Index i(0) ; i < a.raw_size() ; ++i)
          if (ta[i] != tb[i])
          {
            ret = false;
            break;
          }

        if(! std::is_same<Mem::Main, Mem_>::value)
          delete[] ta;
        if(! std::is_same<Mem::Main, Mem2_>::value)
          delete[] tb;

        return ret;
      }

      /**
       * \brief DenseVectorBlocked streaming operator
       *
       * \param[in] lhs The target stream.
       * \param[in] b The vector to be streamed.
       */
      friend std::ostream & operator<< (std::ostream & lhs, const DenseVectorBlocked & b)
      {
        lhs << "[";
        for (Index i(0) ; i < b.size() ; ++i)
        {
          Tiny::Vector<DT_, BlockSize_> t = b(i);
          for (int j(0) ; j < BlockSize_ ; ++j)
            lhs << "  " << t[j];
        }
        lhs << "]";

        return lhs;
      }
    }; // class DenseVectorBlocked<...>


  } // namespace LAFEM
} // namespace FEAST

#endif // KERNEL_LAFEM_DENSE_VECTOR_BLOCKED_HPP
