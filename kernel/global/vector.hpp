// FEAT3: Finite Element Analysis Toolbox, Version 3
// Copyright (C) 2010 - 2021 by Stefan Turek & the FEAT group
// FEAT3 is released under the GNU General Public License version 3,
// see the file 'copyright.txt' in the top level directory for details.

#pragma once
#ifndef KERNEL_GLOBAL_VECTOR_HPP
#define KERNEL_GLOBAL_VECTOR_HPP 1

#include <kernel/global/gate.hpp>
#include <kernel/util/math.hpp>
#include <kernel/lafem/container.hpp> // required for LAFEM::CloneMode

namespace FEAT
{
  namespace Global
  {
    /**
     * \brief Global vector wrapper class template
     *
     * \author Peter Zajac
     */
    template<typename LocalVector_, typename Mirror_>
    class Vector
    {
    public:
      typedef Gate<LocalVector_, Mirror_> GateType;

      typedef typename LocalVector_::MemType MemType;
      typedef typename LocalVector_::DataType DataType;
      typedef typename LocalVector_::IndexType IndexType;
      typedef LocalVector_ LocalVectorType;

      /// Our 'base' class type
      template <typename LocalVector2_, typename Mirror2_ = Mirror_>
      using ContainerType = Vector<LocalVector2_, Mirror2_>;

      /// this typedef lets you create a vector container with new Memory, Datatape and Index types
      template <typename Mem2_, typename DataType2_, typename IndexType2_>
      using ContainerTypeByMDI = Vector<
        typename LocalVector_::template ContainerType<Mem2_, DataType2_, IndexType2_>,
        typename Mirror_::template MirrorType<Mem2_, DataType2_, IndexType2_> >;

    protected:
      const GateType* _gate;
      LocalVector_ _vector;

    public:
      Vector() :
        _gate(nullptr),
        _vector()
      {
      }

      template<typename... Args_>
      explicit Vector(const GateType* gate, Args_&&... args) :
        _gate(gate),
        _vector(std::forward<Args_>(args)...)
      {
      }

      LocalVector_& local()
      {
        return _vector;
      }

      const LocalVector_& local() const
      {
        return _vector;
      }

      template<typename OtherGlobalVector_>
      void convert(const GateType* gate, const OtherGlobalVector_ & other)
      {
        this->_gate = gate;
        this->_vector.convert(other.local());
      }

      const GateType* get_gate() const
      {
        return _gate;
      }

      const Dist::Comm* get_comm() const
      {
        return (_gate != nullptr ? _gate->get_comm() : nullptr);
      }

      void from_1_to_0()
      {
        if(_gate != nullptr)
        {
          _gate->from_1_to_0(this->_vector);
        }
      }

      void sync_0()
      {
        if(_gate != nullptr)
          _gate->sync_0(_vector);
      }

      auto sync_0_async() -> decltype(_gate->sync_0_async(_vector))
      //decltype(_gate->sync_0(_vector)) sync_0_async()
      {
        return _gate->sync_0_async(_vector);
      }

      void sync_1()
      {
        if(_gate != nullptr)
          _gate->sync_1(_vector);
      }

      auto sync_1_async() -> decltype(_gate->sync_1_async(_vector))
      //decltype(_gate->sync_1(_vector)) sync_1_async()
      {
        return _gate->sync_1_async(_vector);
      }

      /**
       * \brief Gets the total number of elements in this vector
       *
       * \warning In parallel, this requires communication and is very expensive, so use sparingly!
       * \note This always returns the raw (or POD - Plain Old Data) size, as everything else is ambiguous.
       *
       * \returns The number of elements
       */
      Index size() const
      {
        // Compute total number of rows
        auto vec_l = clone();
        vec_l.format(DataType(1));

        return Index(vec_l.norm2sqr());
      }

      Vector clone(LAFEM::CloneMode mode = LAFEM::CloneMode::Weak) const
      {
        return Vector(_gate, _vector.clone(mode));
      }

      void clone(const Vector& other, LAFEM::CloneMode mode = LAFEM::CloneMode::Weak)
      {
        XASSERTM(&(other.local()) != &(this->local()), "Trying to self-clone a Global::Vector!");

        this->local() = other.local().clone(mode);
      }

      void clear()
      {
        _vector.clear();
      }

      /**
       * \brief Reset all elements of the container to a given value or zero if missing.
       *
       * \param[in] alpha The value to be set (defaults to 0)
       */
      void format(DataType alpha = DataType(0))
      {
        _vector.format(alpha);
      }

      /**
       * \brief Reset all elements of the container to random values.
       *
       * \param[in] rng The random number generator.
       * \param[in] min Lower rng bound.
       * \param[in] max Upper rng bound.
       */
      void format(Random & rng, DataType min, DataType max)
      {
        _vector.format(rng, min, max);
        sync_1();
      }

      void copy(const Vector& x)
      {
        // avoid self-copy
        if(this != &x)
        {
          _vector.copy(x.local());
        }
      }

      void axpy(const Vector& x, const Vector& y, const DataType alpha = DataType(1))
      {
        _vector.axpy(x.local(), y.local(), alpha);
      }

      void scale(const Vector& x, const DataType alpha)
      {
        _vector.scale(x.local(), alpha);
      }

      DataType dot(const Vector& x) const
      {
        if(_gate != nullptr)
          return _gate->dot(_vector, x.local());
        return _vector.dot(x.local());
      }

      std::shared_ptr<SynchScalarTicket<DataType>> dot_async(const Vector& x) const
      {
        return _gate->dot_async(_vector, x.local());
      }

      DataType norm2sqr() const
      {
        return dot(*this);
      }

      std::shared_ptr<SynchScalarTicket<DataType>> norm2sqr_async() const
      {
        return dot_async(*this);
      }

      /**
       * \brief Computes a reduced 2-norm over all processes.
       *
       * This function is equivalent to the call
       *    Math::sqrt(this->sum(x*x))
       *
       * \param[in] x
       * The value that is to be summarised over all processes.
       *
       * \returns
       * The reduced 2-norm of all \p x.
       */
      DataType norm2() const
      {
        return Math::sqrt(norm2sqr());
      }

      std::shared_ptr<SynchScalarTicket<DataType>> norm2_async() const
      {
        return _gate->dot_async(_vector, _vector, true);
      }

      void component_invert(const Vector& x, const DataType alpha = DataType(1))
      {
        _vector.component_invert(x.local(), alpha);
      }

      void component_product(const Vector& x, const Vector& y)
      {
        _vector.component_product(x.local(), y.local());
      }

      /**
       * \brief Retrieve the absolute maximum value of this vector.
       *
       * \return The largest absolute value.
       */
      DataType max_abs_element() const
      {
        if (_gate != nullptr)
          return _gate->max_abs_element(_vector);
        return _vector.max_abs_element();
      }

      std::shared_ptr<SynchScalarTicket<DataType>> max_abs_element_async() const
      {
        return _gate->max_abs_element_async(_vector);
      }

      /**
       * \brief Retrieve the absolute minimum value of this vector.
       *
       * \return The smallest absolute value.
       */
      DataType min_abs_element() const
      {
        if (_gate != nullptr)
          return _gate->min_abs_element(_vector);
        return _vector.min_abs_element();
      }

      std::shared_ptr<SynchScalarTicket<DataType>> min_abs_element_async() const
      {
        return _gate->min_abs_element_async(_vector);
      }

      /**
       * \brief Retrieve the maximum value of this vector.
       *
       * \return The largest value.
       */
      DataType max_element() const
      {
        if (_gate != nullptr)
          return _gate->max_element(_vector);
        return _vector.max_element();
      }

      std::shared_ptr<SynchScalarTicket<DataType>> max_element_async() const
      {
        return _gate->max_element_async(_vector);
      }

      /**
       * \brief Retrieve the minimum value of this vector.
       *
       * \return The smallest value.
       */
      DataType min_element() const
      {
        if (_gate != nullptr)
          return _gate->min_element(_vector);
        return _vector.min_element();
      }

      std::shared_ptr<SynchScalarTicket<DataType>> min_element_async() const
      {
        return _gate->min_element_async(_vector);
      }

      /// \copydoc FEAT::Control::Checkpointable::get_checkpoint_size()
      std::uint64_t get_checkpoint_size(LAFEM::SerialConfig& config)
      {
        return _vector.get_checkpoint_size(config);
      }

      /// \copydoc FEAT::Control::Checkpointable::restore_from_checkpoint_data(std::vector<char>&)
      void restore_from_checkpoint_data(std::vector<char>& data)
      {
        _vector.restore_from_checkpoint_data(data);
      }

      /// \copydoc FEAT::Control::Checkpointable::set_checkpoint_data(std::vector<char>&)
      std::uint64_t set_checkpoint_data(std::vector<char>& data, LAFEM::SerialConfig& config)
      {
        return _vector.set_checkpoint_data(data, config);
      }
    };
  } // namespace Global
} // namespace FEAT

#endif // KERNEL_GLOBAL_VECTOR_HPP
