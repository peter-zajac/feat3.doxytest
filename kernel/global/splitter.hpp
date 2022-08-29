// FEAT3: Finite Element Analysis Toolbox, Version 3
// Copyright (C) 2010 - 2022 by Stefan Turek & the FEAT group
// FEAT3 is released under the GNU General Public License version 3,
// see the file 'copyright.txt' in the top level directory for details.

#pragma once
#ifndef KERNEL_GLOBAL_SPLITTER_HPP
#define KERNEL_GLOBAL_SPLITTER_HPP 1

#include <kernel/base_header.hpp>
#include <kernel/lafem/base.hpp>
#include <kernel/global/gate.hpp>
#include <kernel/global/muxer.hpp>
#include <kernel/global/vector.hpp>

namespace FEAT
{
  namespace Global
  {
    /**
     * \brief Global base-mesh vector splitter (and joiner) implementation
     *
     * This class provides the functionality for the transfer of primal vectors between a
     * partitioned mesh and its underlying unpartitioned base-mesh on the same refinement level.
     * Technically, this class is merely a special case of the more general Muxer class, however,
     * this class additionally offers some direct file I/O methods. The purpose of this class is
     * to enable the user to write a solution vector, that has been computed on a partitioned mesh
     * in an MPI-parallel simulation, into a single output file on the corresponding unpartitioned
     * mesh by using the join_write_out() function, which can then be read in by another possibly
     * sequential application by the means of the standard LAFEM read_from methods. Furthermore,
     * this class also offers the vice-versa functionality, i.e. it is possible to read in a vector
     * on the unpartitioned mesh that has been written out by the standard LAFEM write_out methods
     * and split it into the local vectors on the corresponding partitioned mesh in an MPI-parallel
     * application by using the split_read_from() function. Of course, as a direct consequence,
     * it is also possible to write a partitioned vector generated on N MPI processes and then read
     * it in by an application running on a different number of MPI processes.
     *
     * \tparam LocalVector_
     * The local vector type that the splitter is to be applied onto.
     *
     * \tparam Mirror_
     * The mirror type that is to be used.
     *
     * To set up an object of this class, the following steps have to be performed:
     * -# On each process, call the set_root() function and provide the communicator as well as
     *    the rank of the remaining root process as well as the identity mirror for all DOFs.
     * -# On the root process, call the push_patch() function and provide the mirror for each
     *    corresponding child patch, which can be assembled from the 'patch:' meshparts that
     *    are (hopefully) generated by the partitioner.
     * -# On the root process, provide a template vector on the unpartitioned base-mesh space
     *    by calling the set_base_vector_template() function. Note that this step is only required
     *    if you intend to use the join() or join_write_out() functions and can be skipped if you
     *    only want to use the split() or split_read_from() functions.
     * -# Compile the splitter by calling the compile() function and supplying it with a temporary
     *    local vector.
     *
     * \author Peter Zajac
     */
    template<typename LocalVector_, typename Mirror_>
    class Splitter
    {
    public:
      /// our internal muxer type
      typedef Global::Muxer<LocalVector_, Mirror_> MuxerType;

      /// our local vector type
      typedef LocalVector_ LocalVectorType;

      /// our global vector type
      typedef Global::Vector<LocalVector_, Mirror_> GlobalVectorType;

    public:
      /// our internal muxer
      MuxerType _muxer;
      /// our base-mesh vector template; only required for join() but not for split()
      LocalVector_ _base_vector_tmpl;

    public:
      /// standard constructor
      Splitter() :
        _muxer(),
        _base_vector_tmpl()
      {
      }

      /// move-constructor
      Splitter(Splitter&& other) :
        _muxer(std::forward<MuxerType>(other._muxer)),
        _base_vector_tmpl(std::forward<LocalVector_>(other._base_vector_tmpl))
      {
      }

      /// destructor
      virtual ~Splitter()
      {
      }

      /// move-assign operator
      Splitter& operator=(Splitter&& other)
      {
        if(this == &other)
          return *this;

        _muxer = std::forward<MuxerType>(other._muxer);
        _base_vector_tmpl = std::forward<LocalVector_>(other._base_vector_tmpl);

        return *this;
      }

      /**
       * \brief Conversion function for same vector container type but with different MDI-Type
       *
       * \param[in] other
       * A \transient reference to the splitter to convert from
       */
      template<typename LVT2_, typename MT2_>
      void convert(const Splitter<LVT2_, MT2_>& other)
      {
        if((void*)this == (void*)&other)
          return;

        this->_muxer.convert(other.muxer);
        this->_base_vector_tmpl.convert(other._base_vector_tmpl);
      }

      /**
       * \brief Conversion function for different vector container type
       *
       * This function (re)creates this splitter from another gate with a different vector type, but the same mirrors.
       * This can be used to create a splitter using DenseVectorBlocked from a splitter using DenseVector or vice versa.
       *
       * \param[in] other
       * A (transient) reference to the splitter using the other vector type to create this splitter from.
       *
       * \param[in] vector
       * A temporary vector allocated to the correct size that is to be used for internal initialization.
       *
       * \param[in] mode
       * The clone-mode to be used for cloning the mirrors. Defaults to shallow clone.
       *
       * \attention
       * This function does not (because it can not) convert the base-mesh vector template, so this has to be
       * provided afterwards by calling the set_base_vector_template() function.
       */
      template<typename LVT2_>
      void convert(const Splitter<LVT2_, Mirror_>& other, LocalVector_&& vector, LAFEM::CloneMode mode = LAFEM::CloneMode::Shallow)
      {
        if((void*)this == (void*)&other)
          return;

        this->_muxer.convert(other._muxer, std::forward<LocalVector_>(vector), mode);
        this->_base_vector_tmpl.clear(); // needs to be supplied later
      }

      /// Returns the internal data size in bytes.
      std::size_t bytes() const
      {
        return _muxer.bytes() + _base_vector_tmpl.bytes();
      }

      /**
       * \brief Checks whether this is the root process.
       *
       * The root process is the process that receives the joined local vector and
       * which is responsible for writing out and reading in the files.
       *
       * \returns \c true, if this is the root process, otherwise \c false.
       */
      bool is_root() const
      {
        return (_muxer.get_sibling_comm() == nullptr) || _muxer.is_parent();
      }

      /**
       * \brief Checks whether there is only one process in the communicator.
       *
       * \returns \c true, if there is only one process, otherwise \c false.
       */
      bool is_single() const
      {
        return (_muxer.get_sibling_comm() == nullptr) || (_muxer.get_sibling_comm()->size() == 1);
      }

      /**
       * \brief Sets the root communicator.
       *
       * \param[in] comm
       * The communicator.
       *
       * \param[in] root_rank
       * The rank of the root process in the comm.
       *
       * \param[in] root_mirror
       * The root mirror.
       */
      void set_root(const Dist::Comm* comm, int root_rank, Mirror_&& root_mirror)
      {
        this->_muxer.set_parent(comm, root_rank, std::forward<Mirror_>(root_mirror));
      }

      /**
       * \brief Adds a child rank and mirror for the parent process.
       *
       * \param[in] patch_mirror
       * The mirror of the patch of this process.
       */
      void push_patch(Mirror_&& patch_mirror)
      {
        _muxer.push_child(std::forward<Mirror_>(patch_mirror));
      }

      /**
       * \brief Sets the base-vector template on the root process.
       *
       * \note
       * Calling this function is only required on the root process if one wants to make use of
       * the join() or join_write_out() functions and can be skipped on all other processes.
       *
       * \param[in] vector_template
       * A base-mesh vector that is allocated to correct size(s) and which is used as a template
       * for the creation of temporary base-mesh vectors. Its numerical contents are ignored.
       */
      void set_base_vector_template(LocalVector_&& vector_template)
      {
        _base_vector_tmpl = std::forward<LocalVector_>(vector_template);
      }

      /**
       * \brief Compiles the muxer.
       *
       * \param[in] vec_tmp_
       * A \transient reference to a temporary vector for internal use.
       */
      void compile(const LocalVector_& vec_tmp_)
      {
        _muxer.compile(vec_tmp_);
      }

      /**
       * \brief Creates a new base-mesh vector
       *
       * \returns A new vector for the base-mesh space, which is allocated to correct size(s),
       * but its numerical contents are uninitialized.
       */
      LocalVector_ create_base_vector() const
      {
        XASSERT(!this->_base_vector_tmpl.empty());
        return this->_base_vector_tmpl.clone(LAFEM::CloneMode::Layout);
      }

      /**
       * \brief Joins a global vector into a base-mesh local vector on the root process
       *
       * \param[in] vector
       * A \transient reference to the global vector that is to be joined.
       *
       * \returns
       * A base-mesh local vector that represents the joined input global vector.
       *
       * \attention
       * A base-mesh vector template must have been given to this object on the root process
       * by calling the set_base_vector_template() before calling this function.
       */
      LocalVector_ join(const Global::Vector<LocalVector_, Mirror_>& vector) const
      {
        // are there no other processes involved here?
        if(is_single())
          return vector.local().clone();

        // make sure the root rank has a base-vector template
        if(is_root())
        {
          XASSERTM(_base_vector_tmpl.size() > Index(0), "root process must have a base vector template");
        }

        // create an base-mesh vector (may be empty on all processes except for parent)
        LocalVector_ vec_base(this->_base_vector_tmpl.clone(LAFEM::CloneMode::Layout));

        // actual join
        join(vec_base, vector);

        // return local vector
        return vec_base;
      }

      /**
       * \brief Joins a global vector into a base-mesh local vector on the root process
       *
       * \param[inout] vec_base
       * A \transient reference to a base-mesh local vector that receives the joined global vector.
       * Must be allocated to correct size(s), but its numerical contents are ignored.
       *
       * \param[in] vector
       * A \transient reference to the type-1 global vector that is to be joined.
       *
       * \note
       * It is not necessary to have supplied a base-mesh vector template by calling the
       * set_base_vector_template() before calling this function, because the base-mesh vector
       * is supplied here as a function parameter.
       */
      void join(LocalVector_& vec_base, const Global::Vector<LocalVector_, Mirror_>& vector) const
      {
        // are there no other processes involved here?
        if(is_single())
        {
          vec_base.copy(vector.local());
          return;
        }

        // Note: the 'join' of the muxer class works with type-0 vectors, so we need to create a
        // temporary type-0 conversion of our input vector here, which is assumed to be a type-1
        // vector. In theory, we also should apply a 'sync_0' on the returned vector, but since
        // its a 'local' vector on the whole base-mesh domain, it is redundant and it also does
        // not make sense due to the lack of a gate on the base-mesh, i.e. on the base-mesh
        // type-0 and type-1 are the same thing.

        // convert input vector to type 0
        GlobalVectorType vector_0 = vector.clone(LAFEM::CloneMode::Deep);
        vector_0.from_1_to_0();

        // is this the muxer parent or only a child process?
        if(_muxer.is_parent())
        {
          _muxer.join(vector_0.local(), vec_base);
        }
        else
        {
          XASSERT(_muxer.is_child());
          _muxer.join_send(vector_0.local());
        }
      }

      /**
       * \brief Splits a base-mesh local vector on the root process into a global vector
       *
       * \param[inout] vector
       * A \transient reference to the type-1 global vector that receives the split base-mesh vector.
       * Must be allocated to correct size(s), but its numerical contents are ignored.
       *
       * \param[in] vec_base
       * A \transient reference to a base-mesh local vector that is to be split.
       */
      void split(Global::Vector<LocalVector_, Mirror_>& vector, const LocalVector_& vec_base) const
      {
        // call patch-local version
        split(vector.local(), vec_base);
      }

      /**
      * \brief Splits a base-mesh local vector on the root process into a patch-local vector
      *
      * \param[inout] vector
      * A \transient reference to the type-1 patch-local vector that receives the split base-mesh vector.
      * Must be allocated to correct size(s), but its numerical contents are ignored.
      *
      * \param[in] vec_base
      * A \transient reference to a base-mesh local vector that is to be split.
      */
      void split(LocalVector_& vector, const LocalVector_& vec_base) const
      {
        // are there no other processes involved here?
        if(is_single())
        {
          // simply copy the local vector contents
          vector.copy(vec_base);
          return;
        }

        // do we have a base muxer?
        if(_muxer.is_parent())
        {
          _muxer.split(vector, vec_base);
        }
        else
        {
          XASSERT(_muxer.is_child());
          _muxer.split_recv(vector);
        }
      }

      /**
       * \brief Joins a global vector into a base-mesh local vector and writes it to a file
       *
       * \param[in] vector
       * A \transient reference to the type-1 global vector that is to be joined.
       *
       * \param[in] filename
       * The filename of the output file that is to be written.
       *
       * \param[in] mode
       * The LAFEM::FileMode of the output file, defaults to binary format.
       *
       * \attention
       * A base-mesh vector template must have been given to this object on the root process
       * by calling the set_base_vector_template() before calling this function.
       */
      void join_write_out(const Global::Vector<LocalVector_, Mirror_>& vector, const String& filename,
        LAFEM::FileMode mode = LAFEM::FileMode::fm_binary)
      {
        // actual join
        LocalVector_ vec_base(join(vector));

        // if we're the parent (rank 0), then we do the actual write out
        if(this->is_root())
        {
          vec_base.write_out(mode, filename);
        }
      }

      /**
       * \brief Reads in a base-mesh local vector and splits it into a global vector.
       *
       * \param[inout] vector
       * A \transient reference to the type-1 global vector that is to be joined.
       * Must be allocated to correct size(s), but its numerical contents are ignored.
       *
       * \param[in] filename
       * The filename of the input file that is to be read in.
       *
       * \param[in] mode
       * The LAFEM::FileMode of the input file, defaults to binary format.
       */
      void split_read_from(Global::Vector<LocalVector_, Mirror_>& vector, const String& filename,
        LAFEM::FileMode mode = LAFEM::FileMode::fm_binary)
      {
        split_read_from(vector.local(), filename, mode);
      }

      /**
       * \brief Reads in a base-mesh local vector and splits it into a patch-local vector.
       *
       * \param[inout] vector
       * A \transient reference to the type-1 patch-local vector that receives the split base-mesh vector.
       * Must be allocated to correct size(s), but its numerical contents are ignored.
       *
       * \param[in] filename
       * The filename of the input file that is to be read in.
       *
       * \param[in] mode
       * The LAFEM::FileMode of the input file, defaults to binary format.
       */
      void split_read_from(LocalVector_& vector, const String& filename,
        LAFEM::FileMode mode = LAFEM::FileMode::fm_binary)
      {
        // create an empty base-mesh vector
        LocalVector_ vec_base;

        // if we're the parent (rank 0), then we do the actual read from
        if(this->is_root())
        {
          // read the actual vector
          vec_base.read_from(mode, filename);
        }

        // split the vector
        split(vector, vec_base);
      }
    }; // class Splitter<...>
  } // namespace Global
} // namespace FEAT

#endif // KERNEL_GLOBAL_SPLITTER_HPP
