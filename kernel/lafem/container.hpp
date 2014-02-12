#pragma once
#ifndef KERNEL_LAFEM_CONTAINER_HPP
#define KERNEL_LAFEM_CONTAINER_HPP 1

// includes, FEAST
#include <kernel/base_header.hpp>
#include <kernel/lafem/memory_pool.hpp>
#include <kernel/archs.hpp>

#include <vector>
#include <limits>
#include <cmath>
#include <typeinfo>


namespace FEAST
{
  /**
   * \brief LAFEM namespace
   */
  namespace LAFEM
  {
      /**
       * Supported File modes.
       */
      enum FileMode
      {
        fm_exp = 0, /**< Exponential ascii */
        fm_dv, /**< Binary data */
        fm_m, /**< Matlab ascii */
        fm_mtx, /**< Matrix market ascii */
        fm_ell, /**< Binary ell data */
        fm_csr, /**< Binary csr data */
        fm_coo /**< Binary coo data */
      };

    /**
     * \brief Container base class.
     *
     * \tparam Mem_ The memory architecture to be used.
     * \tparam DT_ The datatype to be used.
     *
     * This is the base class of all inheritated containers. \n\n
     * Data survey: \n
     * _scalar_index[0]: container size
     *
     * \author Dirk Ribbrock
     */
    template <typename Mem_, typename DT_>
    class Container
    {
      protected:
        /// List of pointers to all datatype dependent arrays.
        std::vector<DT_*> _elements;
        /// List of pointers to all Index dependent arrays.
        std::vector<Index*> _indices;
        /// List of corresponding datatype array sizes.
        std::vector<Index> _elements_size;
        /// List of corresponding Index array sizes.
        std::vector<Index> _indices_size;
        /// List of scalars with datatype index.
        std::vector<Index> _scalar_index;
        /// List of scalars with datatype DT_
        std::vector<DT_> _scalar_dt;

        void _copy_content(const Container<Mem_, DT_> & other)
        {
          if (_elements.size() != other.get_elements().size())
            throw InternalError(__func__, __FILE__, __LINE__, "Container size missmatch!");
          if (_indices.size() != other.get_indices().size())
            throw InternalError(__func__, __FILE__, __LINE__, "Container size missmatch!");
          if (_scalar_index.size() != other.get_scalar_index().size())
            throw InternalError(__func__, __FILE__, __LINE__, "Container size missmatch!");
          if (_scalar_dt.size() != other.get_scalar_dt().size())
            throw InternalError(__func__, __FILE__, __LINE__, "Container size missmatch!");

          for (Index i(0) ; i < _elements.size() ; ++i)
          {
            if (_elements_size.at(i) != other.get_elements_size().at(i))
              throw InternalError(__func__, __FILE__, __LINE__, "Container size missmatch!");
            MemoryPool<Mem_>::template copy<DT_>(_elements.at(i), other.get_elements().at(i), _elements_size.at(i));
          }

          for (Index i(0) ; i < _indices.size() ; ++i)
          {
            if (_indices_size.at(i) != other.get_indices_size().at(i))
              throw InternalError(__func__, __FILE__, __LINE__, "Container size missmatch!");
            MemoryPool<Mem_>::template copy<Index>(_indices.at(i), other.get_indices().at(i), _indices_size.at(i));
          }

          this->_scalar_index.assign(other._scalar_index.begin(), other._scalar_index.end());
          this->_scalar_dt.assign(other._scalar_dt.begin(), other._scalar_dt.end());
        }

        template <typename Mem2_>
        void _copy_content(const Container<Mem2_, DT_> & other)
        {
          Container<Mem_, DT_> temp(other);
          this->_copy_content(temp);
        }

      public:
        /**
         * \brief Constructor
         *
         * \param[in] size The size of the created container.
         *
         * Creates a container with a given size.
         */
        Container(Index size)
        {
          CONTEXT("When creating Container");
          _scalar_index.push_back(size);
        }

        /**
         * \brief Destructor
         *
         * Destroys a container and releases all of its used arrays.
         */
        virtual ~Container()
        {
          CONTEXT("When destroying Container");

          for (Index i(0) ; i < _elements.size() ; ++i)
            MemoryPool<Mem_>::instance()->release_memory(_elements.at(i));
          for (Index i(0) ; i < _indices.size() ; ++i)
            MemoryPool<Mem_>::instance()->release_memory(_indices.at(i));
        }

        /**
         * \brief Copy Constructor
         *
         * \param[in] other The source container.
         *
         * Creates a shallow copy of a given container.
         */
        Container(const Container<Mem_, DT_> & other) :
          _elements(other._elements),
          _indices(other._indices),
          _elements_size(other._elements_size),
          _indices_size(other._indices_size),
          _scalar_index(other._scalar_index),
          _scalar_dt(other._scalar_dt)
        {
          CONTEXT("When copying Container");

          for (Index i(0) ; i < _elements.size() ; ++i)
            MemoryPool<Mem_>::instance()->increase_memory(_elements.at(i));
          for (Index i(0) ; i < _indices.size() ; ++i)
            MemoryPool<Mem_>::instance()->increase_memory(_indices.at(i));
        }

        /**
         * \brief Copy Constructor
         *
         * \param[in] other The source container.
         *
         * Creates a copy of a given container from another memory architecture.
         */
        template <typename Arch2_, typename DT2_>
        Container(const Container<Arch2_, DT2_> & other) :
          _scalar_index(other.get_scalar_index()),
          _scalar_dt(other.get_scalar_dt())
        {
          CONTEXT("When copying Container");

          if (typeid(DT_) != typeid(DT2_))
            throw InternalError(__func__, __FILE__, __LINE__, "type conversion not supported yet!");


          for (Index i(0) ; i < other.get_elements().size() ; ++i)
            this->_elements.push_back(MemoryPool<Mem_>::instance()->template allocate_memory<DT_>(other.get_elements_size().at(i)));
          for (Index i(0) ; i < other.get_indices().size() ; ++i)
            this->_indices.push_back(MemoryPool<Mem_>::instance()->template allocate_memory<Index>(other.get_indices_size().at(i)));

          for (Index i(0) ; i < other.get_elements_size().size() ; ++i)
            this->_elements_size.push_back(other.get_elements_size().at(i));
          for (Index i(0) ; i < other.get_indices_size().size() ; ++i)
            this->_indices_size.push_back(other.get_indices_size().at(i));

          for (Index i(0) ; i < (other.get_elements()).size() ; ++i)
          {
            const unsigned long size(other.get_elements_size().at(i));
            DT2_ * temp((DT2_*)::malloc(size * sizeof(DT2_)));
            MemoryPool<Arch2_>::template download<DT2_>(temp, other.get_elements().at(i), size);
            MemoryPool<Mem_>::template upload<DT_>(this->get_elements().at(i), temp, size);
            ::free(temp);
          }
          for (Index i(0) ; i < other.get_indices().size() ; ++i)
          {
            const unsigned long size(other.get_indices_size().at(i));
            Index * temp((Index*)::malloc(size * sizeof(Index)));
            MemoryPool<Arch2_>::template download<Index>(temp, other.get_indices().at(i), size);
            MemoryPool<Mem_>::template upload<Index>(this->get_indices().at(i), temp, size);
            ::free(temp);
          }
        }

        /**
         * \brief Reset all elements of the container to a given value or zero if missing.
         */
        void clear(DT_ value = 0)
        {
          CONTEXT("When clearing Container");

          for (Index i(0) ; i < _elements.size() ; ++i)
            MemoryPool<Mem_>::instance()->set_memory(_elements.at(i), value, _elements_size.at(i));
        }

        /** \brief Clone operation
         *
         * Become a deep copy of a given container.
         */
        void clone(const Container<Mem_, DT_> & other)
        {
          CONTEXT("When cloning Container");

          this->_scalar_index.assign(other._scalar_index.begin(), other._scalar_index.end());
          this->_scalar_dt.assign(other._scalar_dt.begin(), other._scalar_dt.end());
          this->_elements_size.assign(other._elements_size.begin(), other._elements_size.end());
          this->_indices_size.assign(other._indices_size.begin(), other._indices_size.end());

          for (Index i(0) ; i < this->_elements.size() ; ++i)
            MemoryPool<Mem_>::instance()->release_memory(this->_elements.at(i));
          for (Index i(0) ; i < this->_indices.size() ; ++i)
            MemoryPool<Mem_>::instance()->release_memory(this->_indices.at(i));
          this->_elements.clear();
          this->_indices.clear();

          for (Index i(0) ; i < other._elements.size() ; ++i)
          {
            this->_elements.push_back(MemoryPool<Mem_>::instance()->template allocate_memory<DT_>(this->_elements_size.at(i)));
            MemoryPool<Mem_>::template copy<DT_>(this->_elements.at(i), other._elements.at(i), this->_elements_size.at(i));
          }

          for (Index i(0) ; i < other._indices.size() ; ++i)
          {
            this->_indices.push_back(MemoryPool<Mem_>::instance()->template allocate_memory<Index>(this->_indices_size.at(i)));
            MemoryPool<Mem_>::template copy<Index>(this->_indices.at(i), other._indices.at(i), this->_indices_size.at(i));
          }
        }

        /** \brief Assignment operation
         *
         * Assign another container to the current one.
         */
        void assign(const Container<Mem_, DT_> & other)
        {
          CONTEXT("When assigning Container");

          if (this == &other)
            return;

          for (Index i(0) ; i < this->_elements.size() ; ++i)
            MemoryPool<Mem_>::instance()->release_memory(this->_elements.at(i));
          for (Index i(0) ; i < this->_indices.size() ; ++i)
            MemoryPool<Mem_>::instance()->release_memory(this->_indices.at(i));

          std::vector<DT_ *> new_elements = other.get_elements();
          std::vector<Index*> new_indices = other.get_indices();

          this->_elements.assign(new_elements.begin(), new_elements.end());
          this->_indices.assign(new_indices.begin(), new_indices.end());
          this->_elements_size.assign(other.get_elements_size().begin(), other.get_elements_size().end());
          this->_indices_size.assign(other.get_indices_size().begin(), other.get_indices_size().end());
          this->_scalar_index.assign(other.get_scalar_index().begin(), other.get_scalar_index().end());
          this->_scalar_dt.assign(other.get_scalar_dt().begin(), other.get_scalar_dt().end());

          for (Index i(0) ; i < this->_elements.size() ; ++i)
            MemoryPool<Mem_>::instance()->increase_memory(this->_elements.at(i));
          for (Index i(0) ; i < this->_indices.size() ; ++i)
            MemoryPool<Mem_>::instance()->increase_memory(this->_indices.at(i));
        }

        /** \brief Assignment operation
         *
         * Assigns a container from another memory architecture to the current one.
         */
        template <typename Mem2_, typename DT2_>
        void assign(const Container<Mem2_, DT_> & other)
        {
          CONTEXT("When assigning Container");

          if (typeid(DT_) != typeid(DT2_))
            throw InternalError(__func__, __FILE__, __LINE__, "type conversion not supported yet!");

          for (Index i(0) ; i < this->_elements.size() ; ++i)
            MemoryPool<Mem_>::instance()->release_memory(this->_elements.at(i));
          for (Index i(0) ; i < this->_indices.size() ; ++i)
            MemoryPool<Mem_>::instance()->release_memory(this->_indices.at(i));

          this->_elements.clear();
          this->_indices.clear();
          this->_elements_size.clear();
          this->_indices_size.clear();
          this->_scalar_index.clear();
          this->_scalar_dt.clear();

          this->_elements_size.assign(other.get_elements_size().begin(), other.get_elements_size().end());
          this->_indices_size.assign(other.get_indices_size().begin(), other.get_indices_size().end());
          this->_scalar_index.assign(other.get_scalar_index().begin(), other.get_scalar_index().end());
          this->_scalar_dt.assign(other.get_scalar_dt().begin(), other.get_scalar_dt().end());


          for (Index i(0) ; i < this->_elements_size.size() ; ++i)
          {
            const unsigned long size(this->_elements_size.at(i));
            this->_elements.push_back(MemoryPool<Mem_>::instance()->template allocate_memory<DT_>(size));
            DT2_ * temp((DT2_*)::malloc(size * sizeof(DT2_)));
            MemoryPool<Mem2_>::template download<DT2_>(temp, other.get_elements().at(0), size);
            MemoryPool<Mem_>::template upload<DT_>(this->_elements.at(0), temp, size);
            ::free(temp);
          }

          for (Index i(0) ; i < this->_indices_size.size() ; ++i)
          {
            const unsigned long size(this->_indices_size.at(i));
            this->_indices.push_back(MemoryPool<Mem_>::instance()->template allocate_memory<Index>(size));
            Index * temp((Index*)::malloc(size * sizeof(Index)));
            MemoryPool<Mem2_>::template download<Index>(temp, other.get_indices().at(0), size);
            MemoryPool<Mem_>::template upload<Index>(this->_indices.at(0), temp, size);
            ::free(temp);
          }

        }


        /**
         * \brief Returns a list of all data arrays.
         *
         * \returns A list of all data arrays.
         */
        const std::vector<DT_*> & get_elements() const
        {
          return _elements;
        }

        /**
         * \brief Returns a list of all Index arrays.
         *
         * \returns A list of all Index arrays.
         */
        const std::vector<Index*> & get_indices() const
        {
          return _indices;
        }

        /**
         * \brief Returns a list of all data array sizes.
         *
         * \returns A list of all data array sizes.
         */
        const std::vector<Index> & get_elements_size() const
        {
          return _elements_size;
        }

        /**
         * \brief Returns a list of all Index array sizes.
         *
         * \returns A list of all Index array sizes.
         */
        const std::vector<Index> & get_indices_size() const
        {
          return _indices_size;
        }

        /**
         * \brief Returns a list of all scalar values with datatype index.
         *
         * \returns A list of all scalars with datatype index.
         */
        const std::vector<Index> & get_scalar_index() const
        {
          return _scalar_index;
        }

        /**
         * \brief Returns a list of all scalar values with datatype dt.
         *
         * \returns A list of all scalars with datatype dt.
         */
        const std::vector<DT_> & get_scalar_dt() const
        {
          return _scalar_dt;
        }

        /**
         * \brief Returns the containers size.
         *
         * \returns The containers size.
         */
        Index size() const
        {
          if (_scalar_index.size() > 0)
            return _scalar_index.at(0);
          else
            return Index(0);
        }

        /**
         * \brief Returns the number of effective stored elements.
         *
         * \returns The number of data values.
         */
        virtual Index used_elements() const
        {
          return this->size();
        }

        /**
         * \brief Returns a descriptive string.
         *
         * \returns A string describing the container.
         */
        static String type_name()
        {
          return "Container";
        }
    };

  } // namespace LAFEM
} // namespace FEAST

#endif // KERNEL_LAFEM_CONTAINER_HPP
