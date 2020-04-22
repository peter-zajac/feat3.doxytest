// FEAT3: Finite Element Analysis Toolbox, Version 3
// Copyright (C) 2010 - 2020 by Stefan Turek & the FEAT group
// FEAT3 is released under the GNU General Public License version 3,
// see the file 'copyright.txt' in the top level directory for details.

#pragma once
#ifndef KERNEL_LAFEM_SPARSE_VECTOR_HPP
#define KERNEL_LAFEM_SPARSE_VECTOR_HPP 1

// includes, FEAT
#include <kernel/base_header.hpp>
#include <kernel/util/assertion.hpp>
#include <kernel/util/type_traits.hpp>
#include <kernel/lafem/container.hpp>
#include <kernel/lafem/dense_vector.hpp>
#include <kernel/util/math.hpp>
#include <kernel/adjacency/permutation.hpp>
#include <kernel/lafem/arch/max_element.hpp>

namespace FEAT
{
  namespace LAFEM
  {
    /**
     * \brief Sparse vector class template.
     *
     * \tparam Mem_ The \ref FEAT::Mem "memory architecture" to be used.
     * \tparam DT_ The datatype to be used.
     * \tparam IT_ The indexing type to be used.
     *
     * This class represents a vector with non zero elements in a sparse layout. \n \n
     * Data survey: \n
     * _elements[0]: raw number values \n
     * _indices[0]: non zero indices \n
     * _scalar_index[0]: container size \n
     * _scalar_index[1]: non zero element count (used elements) \n
     * _scalar_index[2]: allocated elements \n
     * _scalar_index[3]: allocation size increment \n
     * _scalar_index[4]: boolean flag, if container is sorted \n
     * _scalar_dt[0]: zero element
     *
     * Refer to \ref lafem_design for general usage informations.
     *
     * \author Dirk Ribbrock
     */
    template <typename Mem_, typename DT_, typename IT_ = Index>
    class SparseVector : public Container<Mem_, DT_, IT_>
    {
    private:
      template <typename T1_, typename T2_>
      static void _insertion_sort(T1_ * key, T2_ * val1, Index size)
      {
        T1_ swap_key;
        T2_ swap1;
        for (Index i(1), j ; i < size ; ++i)
        {
          swap_key = key[i];
          swap1 = val1[i];
          j = i;
          while (j > 0 && key[j-1] > swap_key)
          {
            key[j] = key[j-1];
            val1[j] = val1[j-1];
            --j;
          }
          key[j] = swap_key;
          val1[j] = swap1;
        }
      }

      Index & _used_elements()
      {
        return this->_scalar_index.at(1);
      }

      Index & _allocated_elements()
      {
        return this->_scalar_index.at(2);
      }

      Index & _sorted()
      {
        return this->_scalar_index.at(4);
      }

    public:
      /// Our datatype
      typedef DT_ DataType;
      /// Our indextype
      typedef IT_ IndexType;
      /// Our memory architecture type
      typedef Mem_ MemType;

      /**
       * \brief Constructor
       *
       * Creates an empty non dimensional vector.
       */
      explicit SparseVector() :
        Container<Mem_, DT_, IT_> (0)
      {
        this->_scalar_index.push_back(0);
        this->_scalar_index.push_back(0);
        this->_scalar_index.push_back(Math::min<Index>(0, 1000));
        this->_scalar_index.push_back(1);
        this->_scalar_dt.push_back(DT_(0));
      }

      /**
       * \brief Constructor
       *
       * \param[in] size_in The size of the created vector.
       *
       * Creates a vector with a given size.
       */
      explicit SparseVector(Index size_in) :
        Container<Mem_, DT_, IT_>(size_in)
      {
        this->_scalar_index.push_back(0);
        this->_scalar_index.push_back(0);
        this->_scalar_index.push_back(Math::min<Index>(size_in, 1000));
        this->_scalar_index.push_back(1);
        this->_scalar_dt.push_back(DT_(0));
      }

      /**
       * \brief Constructor
       *
       * \param[in] size_in The size of the created vector.
       * \param[in] elements_in A list of non zero elements.
       * \param[in] indices_in A list of non zero element indices.
       * \param[in] is_sorted Indicates, if the elements are sorted by their indices: is_sorted = true (default)
       *
       * Creates a vector with a given size.
       */
      explicit SparseVector(Index size_in, DenseVector<Mem_, DT_, IT_> & elements_in,
                            DenseVector<Mem_, IT_, IT_> & indices_in, bool is_sorted = true) :
        Container<Mem_, DT_, IT_>(size_in)
      {
        XASSERT(size_in != Index(0));
        XASSERTM(indices_in.size() == elements_in.size(), "Vector size mismatch!");

        this->_scalar_index.push_back(elements_in.size());
        this->_scalar_index.push_back(elements_in.size());
        this->_scalar_index.push_back(Math::min<Index>(size_in, 1000));
        this->_scalar_index.push_back(Index(is_sorted));
        this->_scalar_dt.push_back(DT_(0));

        this->_elements.push_back(elements_in.elements());
        this->_elements_size.push_back(elements_in.size());
        this->_indices.push_back(indices_in.elements());
        this->_indices_size.push_back(indices_in.size());

        for (Index i(0) ; i < this->_elements.size() ; ++i)
          MemoryPool<Mem_>::increase_memory(this->_elements.at(i));
        for (Index i(0) ; i < this->_indices.size() ; ++i)
          MemoryPool<Mem_>::increase_memory(this->_indices.at(i));

        this->sort();
      }

      /**
       * \brief Constructor
       *
       * \param[in] input A std::vector, containing the byte array.
       *
       * Creates a vector from the given byte array.
       */
      template <typename DT2_ = DT_, typename IT2_ = IT_>
      explicit SparseVector(std::vector<char> input) :
        Container<Mem_, DT_, IT_>(0)
      {
        deserialise<DT2_, IT2_>(input);
      }

      /**
       * \brief Move Constructor
       *
       * \param[in] other The source vector.
       *
       * Moves another vector to this vector.
       */
      SparseVector(SparseVector && other) :
        Container<Mem_, DT_, IT_>(std::forward<SparseVector>(other))
      {
      }

      /**
       * \brief Constructor
       *
       * \param[in] mode The used file format.
       * \param[in] filename The source file.
       *
       * Creates a vector based on the source file.
       */
      explicit SparseVector(FileMode mode, String filename) :
        Container<Mem_, DT_, IT_>(0)
      {
        read_from(mode, filename);
      }

      /**
       * \brief Constructor
       *
       * \param[in] mode The used file format.
       * \param[in] file The source filestream.
       *
       * Creates a vector based on the source filestream.
       */
      explicit SparseVector(FileMode mode, std::istream& file) :
        Container<Mem_, DT_, IT_>(0)
      {
        read_from(mode, file);
      }

      /**
       * \brief Assignment move operator
       *
       * \param[in] other The source vector.
       *
       * Moves another vector to the target vector.
       */
      SparseVector & operator= (SparseVector && other)
      {
        this->move(std::forward<SparseVector>(other));

        return *this;
      }

      /** \brief Clone operation
       *
       * Create a clone of this container.
       *
       * \param[in] clone_mode The actual cloning procedure.
       * \returns The created clone.
       *
       */
      SparseVector clone(CloneMode clone_mode = CloneMode::Deep) const
      {
        SparseVector t;
        t.clone(*this, clone_mode);
        return t;
      }

      /** \brief Clone operation
       *
       * Create a clone of another container.
       *
       * \param[in] other The source container to create the clone from.
       * \param[in] clone_mode The actual cloning procedure.
       *
       */
      template<typename Mem2_, typename DT2_, typename IT2_>
      void clone(const SparseVector<Mem2_, DT2_, IT2_> & other, CloneMode clone_mode = CloneMode::Deep)
      {
        Container<Mem_, DT_, IT_>::clone(other, clone_mode);
      }

      /**
       * \brief Conversion method
       *
       * Use source vector content as content of current vector
       *
       * \param[in] other The source container.
       *
       * \note This creates a deep copy in any case!
       *
       */
      template <typename Mem2_, typename DT2_, typename IT2_>
      void convert(const SparseVector<Mem2_, DT2_, IT2_> & other)
      {
        // clean up previous mess before creating a new vector
        this->sort();
        this->clone(other);
      }

      /**
       * \brief Get a pointer to the data array.
       *
       * \returns Pointer to the data array.
       */
      DT_ * elements()
      {
        if (sorted() == 0)
          const_cast<SparseVector *>(this)->sort();
        return this->_elements.at(0);
      }

      DT_ const * elements() const
      {
        if (sorted() == 0)
          const_cast<SparseVector *>(this)->sort();
        return this->_elements.at(0);
      }

      /**
       * \brief Get a pointer to the non zero indices array.
       *
       * \returns Pointer to the indices array.
       */
      IT_ * indices()
      {
        if (sorted() == 0)
          const_cast<SparseVector *>(this)->sort();
        return this->_indices.at(0);
      }

      IT_ const * indices() const
      {
        if (sorted() == 0)
          const_cast<SparseVector *>(this)->sort();
        return this->_indices.at(0);
      }

      /**
       * \brief Retrieve specific vector element.
       *
       * \param[in] index The index of the vector element.
       *
       * \returns Specific vector element.
       */
      const DT_ operator()(Index index) const
      {
        ASSERTM(index < this->_scalar_index.at(0), "index exceeds sparse vector size");

        if (this->_elements.size() == 0)
          return zero_element();

        if (sorted() == 0)
          const_cast<SparseVector *>(this)->sort();

        Index i(0);
        while (i < used_elements())
        {
          if (MemoryPool<Mem_>::get_element(indices(), i) >= index)
            break;
          ++i;
        }

        if (i < used_elements() && MemoryPool<Mem_>::get_element(indices(), i) == index)
          return MemoryPool<Mem_>::get_element(elements(), i);
        else
          return zero_element();
      }


      /**
       * \brief Set specific vector element.
       *
       * \param[in] index The index of the vector element.
       * \param[in] val The val to be set.
       */
      void operator()(Index index, DT_ val)
      {
        ASSERTM(index < this->_scalar_index.at(0), "index exceeds sparse vector size");

        // flag container as not sorted anymore
        // CAUTION: do not use any method triggering resorting until we are finished
        _sorted() = 0;

        // vector is empty, no arrays allocated
        if (this->_elements.size() == 0)
        {
          this->_elements.push_back(MemoryPool<Mem_>::template allocate_memory<DT_>(alloc_increment()));
          this->_elements_size.push_back(alloc_increment());
          MemoryPool<Mem_>::template set_memory<DT_>(this->_elements.back(), DT_(4711), alloc_increment());
          this->_indices.push_back(MemoryPool<Mem_>::template allocate_memory<IT_>(alloc_increment()));
          this->_indices_size.push_back(alloc_increment());
          MemoryPool<Mem_>::template set_memory<IT_>(this->_indices.back(), IT_(4711), alloc_increment());
          _allocated_elements() = alloc_increment();
          MemoryPool<Mem_>::set_memory(this->_elements.at(0), val);
          MemoryPool<Mem_>::set_memory(this->_indices.at(0), IT_(index));
          _used_elements() = 1;
        }

        // append element in already allocated arrays
        else if(_used_elements() < allocated_elements())
        {
          MemoryPool<Mem_>::set_memory(this->_elements.at(0) + _used_elements(), val);
          MemoryPool<Mem_>::set_memory(this->_indices.at(0) + _used_elements(), IT_(index));
          ++_used_elements();
        }

        // reallocate arrays, append element
        else
        {
          _allocated_elements() += alloc_increment();

          DT_ * elements_new(MemoryPool<Mem_>::template allocate_memory<DT_>(allocated_elements()));
          MemoryPool<Mem_>::template set_memory<DT_>(elements_new, DT_(4711), allocated_elements());
          IT_ * indices_new(MemoryPool<Mem_>::template allocate_memory<IT_>(allocated_elements()));
          MemoryPool<Mem_>::template set_memory<IT_>(indices_new, IT_(4711), allocated_elements());

          MemoryPool<Mem_>::copy(elements_new, this->_elements.at(0), _used_elements());
          MemoryPool<Mem_>::copy(indices_new, this->_indices.at(0), _used_elements());

          MemoryPool<Mem_>::release_memory(this->_elements.at(0));
          MemoryPool<Mem_>::release_memory(this->_indices.at(0));

          this->_elements.at(0) = elements_new;
          this->_indices.at(0) = indices_new;

          MemoryPool<Mem_>::set_memory(this->_elements.at(0) + _used_elements(), val);
          MemoryPool<Mem_>::set_memory(this->_indices.at(0) + _used_elements(), IT_(index));

          ++_used_elements();
          this->_elements_size.at(0) = allocated_elements();
          this->_indices_size.at(0) = allocated_elements();
        }
      }

      void sort()
      {
        if (sorted() == 0)
        {
          //first of all, mark vector as sorted, because otherwise we would call ourselves inifite times
          // CAUTION: do not use any method triggering resorting until we are finished
          _sorted() = 1;

          // check if there is anything to be sorted
          if(_used_elements() == Index(0))
            return;

          IT_ * pindices;
          DT_ * pelements;
          if (typeid(Mem_) == typeid(Mem::Main))
          {
            pindices = this->_indices.at(0);
            pelements = this->_elements.at(0);
          }
          else
          {
            pindices = new IT_[_allocated_elements()];
            pelements = new DT_[_allocated_elements()];
            MemoryPool<Mem_>::download(pindices, this->_indices.at(0), _allocated_elements());
            MemoryPool<Mem_>::download(pelements, this->_elements.at(0), _allocated_elements());
          }

          _insertion_sort(pindices, pelements, _used_elements());

          // find and mark duplicate entries
          for (Index i(1) ; i < _used_elements() ; ++i)
          {
            if (pindices[i-1] == pindices[i])
            {
              pindices[i-1] = std::numeric_limits<IT_>::max();
            }
          }

          // sort out marked duplicated elements
          _insertion_sort(pindices, pelements, _used_elements());
          Index junk(0);
          while (pindices[_used_elements() - 1 - junk] == std::numeric_limits<IT_>::max() && junk < _used_elements())
            ++junk;
          _used_elements() -= junk;

          if (typeid(Mem_) != typeid(Mem::Main))
          {
            MemoryPool<Mem_>::upload(this->_indices.at(0), pindices, _allocated_elements());
            MemoryPool<Mem_>::upload(this->_elements.at(0), pelements, _allocated_elements());
            delete[] pindices;
            delete[] pelements;
          }
        }
      }

      /**
       * \brief Retrieve the absolute maximum value of this vector.
       *
       * \return The largest absolute value.
       */
      DT_ max_element() const
      {
        TimeStamp ts_start;

        Index max_index = Arch::MaxElement<Mem_>::value(this->elements(), this->used_elements());
        DT_ result = Math::abs((*this)(max_index));

        TimeStamp ts_stop;
        Statistics::add_time_reduction(ts_stop.elapsed(ts_start));

        return result;
      }

      /**
       * \brief Deserialisation of complete container entity.
       *
       * \param[in] input A std::vector, containing the byte array.
       *
       * Recreate a complete container entity by a single binary array.
       */
      template <typename DT2_ = DT_, typename IT2_ = IT_>
      void deserialise(std::vector<char> input)
      {
        this->template _deserialise<DT2_, IT2_>(FileMode::fm_sv, input);
      }

      /**
       * \brief Serialisation of complete container entity.
       *
       * Serialize a complete container entity into a single binary array.
       *
       * See \ref FEAT::LAFEM::Container::_serialise for details.
       */
      template <typename DT2_ = DT_, typename IT2_ = IT_>
      std::vector<char> serialise()
      {
        return this->template _serialise<DT2_, IT2_>(FileMode::fm_sv);
      }

      /**
       * \brief Read in vector from file.
       *
       * \param[in] mode The used file format.
       * \param[in] filename The file that shall be read in.
       */
      void read_from(FileMode mode, String filename)
      {
        std::ifstream file(filename.c_str(), std::ifstream::in);
        if (! file.is_open())
          XABORTM("Unable to open Vector file " + filename);
        read_from(mode, file);
        file.close();
      }

      /**
       * \brief Read in vector from stream.
       *
       * \param[in] mode The used file format.
       * \param[in] file The stream that shall be read in.
       */
      void read_from(FileMode mode, std::istream& file)
      {
        switch(mode)
        {
          case FileMode::fm_mtx:
          {
            this->clear();
            this->_scalar_index.push_back(0);
            this->_scalar_index.push_back(0);
            this->_scalar_index.push_back(Math::min<Index>(0, 1000));
            this->_scalar_index.push_back(1);
            this->_scalar_dt.push_back(DT_(0));

            Index rows;
            Index nnz;
            String line;
            std::getline(file, line);
            if (line.find("%%MatrixMarket matrix coordinate real general") == String::npos)
            {
              XABORTM("Input-file is not a compatible mtx-file");
            }
            while(!file.eof())
            {
              std::getline(file,line);
              if (file.eof())
                XABORTM("Input-file is empty");

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
                XABORTM("Input-file is no sparse-vector-file");

              begin = line.find_first_not_of(" ");
              line.erase(0, begin);
              end = line.find_first_of(" ");
              String snnz(line, 0, end);
              nnz = (Index)atol(snnz.c_str());
              line.erase(0, end);
            }

            DenseVector<Mem::Main, IT_, IT_> ind(nnz);
            DenseVector<Mem::Main, DT_, IT_> val(nnz);

            IT_ * pind(ind.elements());
            DT_ * pval(val.elements());

            while(!file.eof())
            {
              std::getline(file, line);
              if (file.eof())
                break;

              String::size_type begin(line.find_first_not_of(" "));
              line.erase(0, begin);
              String::size_type end(line.find_first_of(" "));
              String srow(line, 0, end);
              IT_ row((IT_)atol(srow.c_str()));
              --row;
              line.erase(0, end);

              begin = line.find_first_not_of(" ");
              line.erase(0, begin);
              end = line.find_first_of(" ");
              line.erase(0, end);

              begin = line.find_first_not_of(" ");
              line.erase(0, begin);
              end = line.find_first_of(" ");
              String sval(line, 0, end);
              DT_ tval((DT_)atof(sval.c_str()));

              *pval = tval;
              *pind = row;
              ++pval;
              ++pind;
            }
            SparseVector<Mem::Main, DT_, IT_> tmp(rows, val, ind, false);
            this->assign(tmp);
            break;
          }
          default:
            XABORTM("Filemode not supported!");
        }
      }

      /**
       * \brief Write out vector to file.
       *
       * \param[in] mode The used file format.
       * \param[in] filename The file where the matrix shall be stored.
       */
      void write_out(FileMode mode, String filename) const
      {
        std::ofstream file(filename.c_str(), std::ofstream::out);
        if (! file.is_open())
          XABORTM("Unable to open Vector file " + filename);
        write_out(mode, file);
        file.close();
      }

      /**
       * \brief Write out vector to file.
       *
       * \param[in] mode The used file format.
       * \param[in] file The stream that shall be written to.
       */
      void write_out(FileMode mode, std::ostream& file) const
      {
        switch(mode)
        {
          case FileMode::fm_mtx:
          {
            SparseVector<Mem::Main, DT_, IT_> temp;
            temp.convert(*this);

            file << "%%MatrixMarket matrix coordinate real general" << std::endl;
            file << temp.size() << " " << 1 << " " << temp.used_elements() << std::endl;

            const Index u_elem(temp.used_elements());
            const IT_ * pind(temp.indices());
            const DT_ * pval(temp.elements());
            for (Index i(0) ; i < u_elem ; ++i, ++pind, ++pval)
            {
              file << *pind+1 << " " << 1 << " " << std::scientific << *pval << std::endl;
            }
            break;
          }
          default:
            XABORTM("Filemode not supported!");
        }
      }

      /**
       * \brief Retrieve non zero element count.
       *
       * \returns Non zero element count.
       */
      template <Perspective = Perspective::native>
      Index used_elements() const
      {
        if (sorted() == 0)
          const_cast<SparseVector *>(this)->sort();
        return this->_scalar_index.at(1);
      }

      /**
       * \brief Retrieve non zero element.
       *
       * \returns Zero element.
       */
      DT_ zero_element() const
      {
        return this->_scalar_dt.at(0);
      }

      /**
       * \brief Retrieve amount of allocated elements.
       *
       * \return Allocated element count.
       */
      Index allocated_elements() const
      {
        return this->_scalar_index.at(2);
      }

      /**
       * \brief Retrieve allocation incrementation value.
       *
       * \return Allocation increment.
       */
      Index alloc_increment() const
      {
        return this->_scalar_index.at(3);
      }

      /**
       * \brief Retrieve status of element sorting.
       *
       * \return Sorting status.
       */
      Index sorted() const
      {
        return this->_scalar_index.at(4);
      }

      /// Permutate vector according to the given Permutation
      void permute(Adjacency::Permutation & perm)
      {
        if (perm.size() == 0)
          return;

        XASSERTM(perm.size() == this->size(), "Container size does not match permutation size");

        SparseVector<Mem::Main, DT_, IT_> local;
        local.convert(*this);
        SparseVector<Mem::Main, DT_, IT_> target(this->size());

        auto inv = perm.inverse();
        const Index * const inv_pos(inv.get_perm_pos());
        for (Index i(0) ; i < local.used_elements() ; ++i)
        {
          const Index col = local.indices()[i];
          target(inv_pos[col], local(col));
        }

        target.sort();
        this->assign(target);
      }

      /**
       * \brief Returns a descriptive string.
       *
       * \returns A string describing the container.
       */
      static String name()
      {
        return "SparseVector";
      }


      /**
       * \brief SparseVector comparison operator
       *
       * \param[in] a A vector to compare with.
       * \param[in] b A vector to compare with.
       */
      template <typename Mem2_> friend bool operator== (const SparseVector & a, const SparseVector<Mem2_, DT_, IT_> & b)
      {
        if (a.size() != b.size())
          return false;
        if (a.get_elements().size() != b.get_elements().size())
          return false;
        if (a.get_indices().size() != b.get_indices().size())
          return false;

        for (Index i(0) ; i < a.size() ; ++i)
          if (a(i) != b(i))
            return false;

        return true;
      }

      /**
       * \brief SparseVector streaming operator
       *
       * \param[in] lhs The target stream.
       * \param[in] b The vector to be streamed.
       */
      friend std::ostream & operator<< (std::ostream & lhs, const SparseVector & b)
      {
        lhs << "[";
        for (Index i(0) ; i < b.size() ; ++i)
        {
          lhs << "  " << b(i);
        }
        lhs << "]";

        return lhs;
      }
    }; // class SparseVector<...>

#ifdef FEAT_EICKT
    extern template class SparseVector<Mem::Main, float, unsigned int>;
    extern template class SparseVector<Mem::Main, double, unsigned int>;
    extern template class SparseVector<Mem::Main, float, unsigned long>;
    extern template class SparseVector<Mem::Main, double, unsigned long>;
#ifdef FEAT_HAVE_CUDA
    extern template class SparseVector<Mem::CUDA, float, unsigned int>;
    extern template class SparseVector<Mem::CUDA, double, unsigned int>;
    extern template class SparseVector<Mem::CUDA, float, unsigned long>;
    extern template class SparseVector<Mem::CUDA, double, unsigned long>;
#endif
#endif


  } // namespace LAFEM
} // namespace FEAT

#endif // KERNEL_LAFEM_SPARSE_VECTOR_HPP
