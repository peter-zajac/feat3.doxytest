// FEAT3: Finite Element Analysis Toolbox, Version 3
// Copyright (C) 2010 - 2021 by Stefan Turek & the FEAT group
// FEAT3 is released under the GNU General Public License version 3,
// see the file 'copyright.txt' in the top level directory for details.

#pragma once
#ifndef KERNEL_GLOBAL_GATE_HPP
#define KERNEL_GLOBAL_GATE_HPP 1

#include <kernel/base_header.hpp>
#include <kernel/util/dist.hpp>
#include <kernel/util/exception.hpp>
#include <kernel/lafem/dense_vector.hpp>
#include <kernel/global/synch_vec.hpp>
#include <kernel/global/synch_scal.hpp>

#include <vector>

namespace FEAT
{
  /**
   * \brief Global linear algebra namespace
   */
  namespace Global
  {
    /**
     * \brief Global gate implementation
     *
     * \author Peter Zajac
     */
    template<typename LocalVector_, typename Mirror_>
    class Gate
    {
    public:
      typedef typename LocalVector_::MemType MemType;
      typedef typename LocalVector_::DataType DataType;
      typedef typename LocalVector_::IndexType IndexType;
      typedef LAFEM::DenseVector<Mem::Main, DataType, IndexType> BufferVectorType;
      typedef Mirror_ MirrorType;

      typedef std::shared_ptr<SynchScalarTicket<DataType>> ScalarTicketType;
      typedef std::shared_ptr<SynchVectorTicket<LocalVector_, Mirror_>> VectorTicketType;

    public:
      /// our communicator
      const Dist::Comm* _comm;
      /// communication ranks
      std::vector<int> _ranks;
      /// vector mirrors
      std::vector<Mirror_> _mirrors;
      /// frequency vector
      LocalVector_ _freqs;

      /// Our 'base' class type
      template <typename LocalVector2_, typename Mirror2_>
      using GateType = Gate<LocalVector2_, Mirror2_>;

      /// this typedef lets you create a gate container with new Memory, Data and Index types
      template <typename Mem2_, typename DataType2_, typename IndexType2_>
      using GateTypeByMDI = Gate<typename LocalVector_::template ContainerType<Mem2_, DataType2_, IndexType2_>, typename Mirror_::template MirrorType<Mem2_, DataType2_, IndexType2_> >;

    public:
      explicit Gate() :
        _comm(nullptr)
      {
      }

      explicit Gate(const Dist::Comm& comm) :
        _comm(&comm)
      {
      }

      Gate(Gate && other) :
        _comm(other._comm),
        _ranks(std::forward<std::vector<int>>(other._ranks)),
        _mirrors(std::forward<std::vector<Mirror_>>(other._mirrors)),
        _freqs(std::forward<LocalVector_>(other._freqs))
      {
      }

      Gate & operator=(Gate && other)
      {
        if(this == &other)
        {
          return *this;
        }

        _comm = other._comm;
        _ranks = std::forward<std::vector<int>>(other._ranks);
        _mirrors = std::forward<std::vector<Mirror_>>(other._mirrors);
        _freqs = std::forward<LocalVector_>(other._freqs);

        return *this;
      }


      virtual ~Gate()
      {
      }

      const Dist::Comm* get_comm() const
      {
        return _comm;
      }

      void set_comm(const Dist::Comm* comm_)
      {
        _comm = comm_;
      }

      template<typename LVT2_, typename MT2_>
      void convert(const Gate<LVT2_, MT2_>& other)
      {
        if((void*)this == (void*)&other)
          return;

        this->_ranks.clear();
        this->_mirrors.clear();

        this->_comm = other._comm;
        this->_ranks = other._ranks;

        for(auto& other_mirrors_i : other._mirrors)
        {
          MirrorType mirrors_i;
          mirrors_i.convert(other_mirrors_i);
          this->_mirrors.push_back(std::move(mirrors_i));
        }

        this->_freqs.convert(other._freqs);
      }

      /// \brief Returns the total amount of bytes allocated.
      std::size_t bytes() const
      {
        size_t temp(0);
        for (auto& i : _mirrors)
        {
          temp += i.bytes();
        }
        temp += _freqs.bytes();
        temp += _ranks.size() * sizeof(int);

        return temp;
      }

      void push(int rank, Mirror_&& mirror)
      {
        // push rank and tags
        _ranks.push_back(rank);

        // push mirror
        _mirrors.push_back(std::move(mirror));
      }

      void compile(LocalVector_&& vector)
      {
        // initialize frequency vector
        _freqs = std::move(vector);
        _freqs.format(DataType(1));

        // loop over all mirrors
        for(std::size_t i(0); i < _mirrors.size(); ++i)
        {
          // sum up number of ranks per frequency entry, listed in different mirrors
          auto temp = _mirrors.at(i).create_buffer(_freqs);
          temp.format(DataType(1));

          // gather-axpy into frequency vector
          _mirrors.at(i).scatter_axpy(_freqs, temp);
        }

        // invert frequencies
        _freqs.component_invert(_freqs);
      }

      /**
       * \brief Converts a type-1 vector into a type-0 vector.
       *
       * \param[inout] vector
       * On entry, the type-1 vector to be converted.
       * On exit, the converted type-0 vector.
       *
       * \note This function does not perform any synchronization.
       */
      void from_1_to_0(LocalVector_& vector) const
      {
        if(!_ranks.empty())
        {
          vector.component_product(vector, _freqs);
        }
      }

      /**
       * \brief Synchronizes a type-0 vector, resulting in a type-1 vector.
       *
       * \param[inout] vector
       * On entry, the type-0 vector to be synchronized.\n
       * On exit, the synchronized type-1 vector.
       */
      void sync_0(LocalVector_& vector) const
      {
        if(_ranks.empty())
          return;

        synch_vector(vector, *_comm, _ranks, _mirrors);
      }

      VectorTicketType sync_0_async(LocalVector_& vector) const
      {
        return std::make_shared<SynchVectorTicket<LocalVector_, Mirror_>>(vector, *_comm, _ranks, _mirrors);
      }

      /**
       * \brief Synchronizes a type-1 vector, resulting in a type-1 vector.
       *
       * \param[inout] vector
       * On entry, the type-1 vector to be synchronized.\n
       * On exit, the synchronized type-1 vector.
       *
       * \note
       * This function effectively applies the from_1_to_0() and sync_0()
       * functions onto the input vector.
       */
      void sync_1(LocalVector_& vector) const
      {
        if(_ranks.empty())
          return;

        from_1_to_0(vector);
        synch_vector(vector, *_comm, _ranks, _mirrors);
      }

      VectorTicketType sync_1_async(LocalVector_& vector) const
      {
        from_1_to_0(vector);
        return sync_0_async(vector);
      }

      /**
       * \brief Computes a synchronized dot-product of two type-1 vectors.
       *
       * \param[in] x, y
       * The two type-1 vector whose dot-product is to be computed.
       *
       * \returns
       * The dot-product of \p x and \p y.
       */
      DataType dot(const LocalVector_& x, const LocalVector_& y) const
      {
        // This is if there is only one process
        if(_comm == nullptr || _comm->size() == 1)
        {
          return x.dot(y);
        }
        // Even if there are no neighbors, we still need to sum up globally
        else if(_ranks.empty())
        {
          return sum(x.dot(y));
        }
        // If there are neighbors, we have to use the frequencies and sum up globally
        else
        {
          return sum(_freqs.triple_dot(x, y));
        }
      }

      ScalarTicketType dot_async(const LocalVector_& x, const LocalVector_& y, bool sqrt = false) const
      {
        return sum_async(_freqs.triple_dot(x, y), sqrt);
      }

      /**
       * \brief Computes a reduced sum over all processes.
       *
       * \param[in] x
       * The value that is to be summarized over all processes.
       *
       * \returns
       * The reduced sum of all \p x.
       */
      DataType sum(DataType x) const
      {
        return synch_scalar(x, *_comm, Dist::op_sum, false);
      }

      ScalarTicketType sum_async(DataType x, bool sqrt = false) const
      {
        return std::make_shared<SynchScalarTicket<DataType>>(x, *_comm, Dist::op_sum, sqrt);
      }

      /**
       * \brief Computes the minimum of a scalar variable over all processes.
       *
       * \param[in] x
       * What we want to compute the minimum over all processes of.
       *
       * \returns
       * The minimum of all \p x.
       */
      DataType min(DataType x) const
      {
        return synch_scalar(x, *_comm, Dist::op_min, false);
      }

      ScalarTicketType min_async(DataType x) const
      {
        return std::make_shared<SynchScalarTicket<DataType>>(x, *_comm, Dist::op_min, false);
      }

      /**
       * \brief Computes the maximum of a scalar variable over all processes.
       *
       * \param[in] x
       * What we want to compute the maximum over all processes of.
       *
       * \returns
       * The maximum of all \p x.
       */
      DataType max(DataType x) const
      {
        return synch_scalar(x, *_comm, Dist::op_max, false);
      }

      ScalarTicketType max_async(DataType x) const
      {
        return std::make_shared<SynchScalarTicket<DataType>>(x, *_comm, Dist::op_max, false);
      }

      /**
       * \brief Computes a reduced 2-norm over all processes.
       *
       * This function is equivalent to the call
       *    Math::sqrt(this->sum(x*x))
       *
       * \param[in] x
       * The value that is to be summarized over all processes.
       *
       * \returns
       * The reduced 2-norm of all \p x.
       */
      DataType norm2(DataType x) const
      {
        return synch_scalar(x*x, *_comm, Dist::op_sum, true);
      }

      ScalarTicketType norm2_async(DataType x) const
      {
        return std::make_shared<SynchScalarTicket<DataType>>(x*x, *_comm, Dist::op_sum, true);
      }

      /**
       * \brief Retrieve the absolute maximum value of this vector.
       *
       * \return The largest absolute value.
       */
      DataType max_abs_element(const LocalVector_ & x) const
      {
        return synch_scalar(x.max_abs_element(), *_comm, Dist::op_max);
      }

      ScalarTicketType max_abs_element_async(const LocalVector_ & x) const
      {
        return std::make_shared<SynchScalarTicket<DataType>>(x.max_abs_element(), *_comm, Dist::op_max);
      }

      /**
       * \brief Retrieve the absolute minimum value of this vector.
       *
       * \return The smallest absolute value.
       */
      DataType min_abs_element(const LocalVector_ & x) const
      {
        return synch_scalar(x.min_abs_element(), *_comm, Dist::op_min);
      }

      ScalarTicketType min_abs_element_async(const LocalVector_ & x) const
      {
        return std::make_shared<SynchScalarTicket<DataType>>(x.min_abs_element(), *_comm, Dist::op_min);
      }

      /**
       * \brief Retrieve the maximum value of this vector.
       *
       * \return The largest value.
       */
      DataType max_element(const LocalVector_ & x) const
      {
        return synch_scalar(x.max_element(), *_comm, Dist::op_max);
      }

      ScalarTicketType max_element_async(const LocalVector_ & x) const
      {
        return std::make_shared<SynchScalarTicket<DataType>>(x.max_element(), *_comm, Dist::op_max);
      }

      /**
       * \brief Retrieve the minimum value of this vector.
       *
       * \return The smallest value.
       */
      DataType min_element(const LocalVector_ & x) const
      {
        return synch_scalar(x.min_element(), *_comm, Dist::op_max);
      }

      ScalarTicketType min_element_async(const LocalVector_ & x) const
      {
        return std::make_shared<SynchScalarTicket<DataType>>(x.min_element(), *_comm, Dist::op_max);
      }
    }; // class Gate<...>
  } // namespace Global
} // namespace FEAT

#endif // KERNEL_GLOBAL_GATE_HPP
