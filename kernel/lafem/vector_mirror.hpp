// FEAT3: Finite Element Analysis Toolbox, Version 3
// Copyright (C) 2010 - 2021 by Stefan Turek & the FEAT group
// FEAT3 is released under the GNU General Public License version 3,
// see the file 'copyright.txt' in the top level directory for details.

#pragma once
#ifndef KERNEL_LAFEM_VECTOR_MIRROR_HPP
#define KERNEL_LAFEM_VECTOR_MIRROR_HPP 1

// includes, FEAT
#include <kernel/lafem/container.hpp>
#include <kernel/lafem/dense_vector.hpp>
#include <kernel/lafem/dense_vector_blocked.hpp>
#include <kernel/lafem/sparse_vector.hpp>
#include <kernel/lafem/sparse_vector_blocked.hpp>
#include <kernel/lafem/sparse_matrix_csr.hpp>
#include <kernel/lafem/arch/mirror.hpp>

namespace FEAT
{
  namespace LAFEM
  {
    /**
     * \brief Handles vector prolongation, restriction and serialization
     *
     * \tparam Mem_
     * Memory architecture
     *
     * \tparam DT_
     * Data type in the vector
     *
     * \tparam IT_
     * Type for indexing
     *
     * A Mirror handles the restriction of a given vector to a subvector of itself and the serialization into
     * buffer vector (which always is a LAFEM::DenseVector), as well as the prolongation of such buffer vectors.
     * The buffer vectors then can be communicated via MPI etc.
     *
     * This can be used for restricting a vector associated with a Mesh to a subset of that Mesh, i.e. identified by
     * a MeshPart. The standard use case is that we have an FE coefficient vector living on a patch that we want to
     * communicate. For this, we need all coefficients (= vector entries) that contribute to other patches, and
     * these are identified by a MeshPart called Halo. The mirror is constructed from the restriction (gather) and
     * prolongation (scatter) matrices representing the adjacency structure of the underlying FE space and thus the
     * coefficients.
     *
     * Data survey: \n
     * _indices[0]: non zero indices \n
     * _scalar_index[0]: container size \n
     * _scalar_index[1]: non zero element count (used elements) \n
     *
     * \author Peter Zajac
     * \author Jordi Paul
     */
    template<typename Mem_, typename DT_, typename IT_>
    class VectorMirror :
      public Container<Mem_, DT_, IT_>
    {
    public:
      /// our base class
      typedef Container<Mem_, DT_, IT_> BaseClass;
      /// memory typedef
      typedef Mem_ MemType;
      /// data-type typedef
      typedef DT_ DataType;
      /// index-type typedef
      typedef IT_ IndexType;

      /// Our 'base' class type
      template <typename Mem2_, typename DT2_ = DT_, typename IT2_ = IT_>
      using MirrorType = VectorMirror<Mem2_, DT2_, IT2_>;

      /// this typedef lets you create a mirror with new Memory, Data and Index types
      template <typename Mem2_, typename DataType2_, typename IndexType2_>
      using MirrorTypeByMDI = MirrorType<Mem2_, DataType2_, IndexType2_>;

      /// ImageIterator for Adjactor interface implementation
      typedef IT_* ImageIterator;

    public:
      /// default constructor
      VectorMirror() :
        BaseClass(0)
      {
        this->_scalar_index.push_back(0); // number of indices
      }

      /**
       * \brief Constructor
       *
       * This constructor initializes the mirror dimensions and allocates the internal
       * indices array. This array is uninitialized and has to be filled after construction.
       *
       * \param[in] size_in
       * Specifies the size of the mirror, i.e. the length
       * of the vectors that this mirror can be applied to.
       *
       * \param[in] num_idx
       * The number of indices in the mirror, i.e. the length
       * of the buffers that this mirror can be applied to.
       */
      explicit VectorMirror(Index size_in, Index num_idx) :
        BaseClass(size_in)
      {
        this->_scalar_index.push_back(num_idx); // number of indices
        if(num_idx > Index(0))
        {
          this->_indices.push_back(MemoryPool<MemType>::template allocate_memory<IndexType>(num_idx));
          this->_indices_size.push_back(num_idx);
        }
      }

      /// move-ctor
      VectorMirror(VectorMirror&& other) :
        BaseClass(std::forward<BaseClass>(other))
      {
      }

      /// move-assign operator
      VectorMirror& operator=(VectorMirror&& other)
      {
        if(this == &other)
          return *this;

        this->move(std::forward<BaseClass>(other));

        return *this;
      }

      /**
       * \brief Creates and returns an identity mirror
       *
       * \param[in] size_in
       * The desired size of the mirror to be created
       *
       * \returns The new identity mirror
       */
      static VectorMirror make_identity(IndexType size_in)
      {
        VectorMirror mir(size_in, size_in);
        IndexType* idx = mir.indices();
        for(IndexType i(0); i < size_in; ++i)
          idx[i] = i;
        return mir;
      }

      /**
       * \brief Clone operation
       *
       * Create a clone of this container.
       *
       * \param[in] clone_mode The actual cloning procedure.
       */
      VectorMirror clone(CloneMode clone_mode = CloneMode::Weak) const
      {
        VectorMirror t;
        t.clone(*this, clone_mode);
        return t;
      }

      /**
       * \brief Clone operation
       *
       * Create a clone of another container.
       *
       * \param[in] other The source container to create the clone from.
       * \param[in] clone_mode The actual cloning procedure.
       */
      template<typename Mem2_, typename DT2_, typename IT2_>
      void clone(const VectorMirror<Mem2_, DT2_, IT2_> & other, CloneMode clone_mode = CloneMode::Weak)
      {
        Container<Mem_, DT_, IT_>::clone(other, clone_mode);
      }

      /**
       * \brief Conversion method
       *
       * \param[in] other
       * The source mirror.
       */
      template<typename Mem2_, typename DT2_, typename IT2_>
      void convert(const VectorMirror<Mem2_, DT2_, IT2_>& other)
      {
        this->assign(other);
      }

      /**
       * \brief Returns the number of indices in the mirror.
       *
       * \return The number of indices in the mirror.
       */
      Index num_indices() const
      {
        return this->_scalar_index.empty() ? Index(0) : this->_scalar_index.at(1);
      }

      /**
       * \brief Checks whether the mirror is empty.
       *
       * \returns \c true, if there are no indices in the mirror, otherwise \c false.
       */
      bool empty() const
      {
        return (this->num_indices() == Index(0));
      }

      /**
       * \brief Get a pointer to the non zero indices array.
       *
       * \returns Pointer to the indices array.
       */
      IT_* indices()
      {
        return this->_indices.empty() ? nullptr : this->_indices.at(0);
      }

      /** \copydoc indices() */
      const IT_* indices() const
      {
        return this->_indices.empty() ? nullptr : this->_indices.at(0);
      }

      /**
       * \brief Computes the required buffer size for a DenseVector.
       *
       * \param[in] vector
       * The vector whose buffer size is to be computed.
       */
      template<typename Mem2_, typename DT2_, typename IT2_>
      Index buffer_size(const DenseVector<Mem2_, DT2_, IT2_>& DOXY(vector)) const
      {
        return num_indices();
      }

      /**
       * \brief Computes the required buffer size for a DenseVectorBlocked.
       *
       * \param[in] vector
       * The vector whose buffer size is to be computed.
       */
      template<typename Mem2_, typename DT2_, typename IT2_, int block_size_>
      Index buffer_size(const DenseVectorBlocked<Mem2_, DT2_, IT2_, block_size_>& DOXY(vector)) const
      {
        return num_indices()*Index(block_size_);
      }

      /**
       * \brief Computes the required buffer size for a SparseVector.
       *
       * \param[in] vector
       * The vector whose buffer size is to be computed.
       */
      template<typename Mem2_, typename DT2_, typename IT2_>
      Index buffer_size(const SparseVector<Mem2_, DT2_, IT2_>& DOXY(vector)) const
      {
        return num_indices();
      }

      /**
       * \brief Computes the required buffer size for a SparseVectorBlocked.
       *
       * \param[in] vector
       * The vector whose buffer size is to be computed.
       */
      template<typename Mem2_, typename DT2_, typename IT2_, int block_size_>
      Index buffer_size(const SparseVectorBlocked<Mem2_, DT2_, IT2_, block_size_>& DOXY(vector)) const
      {
        return num_indices()*Index(block_size_);
      }

      /**
       * \brief Creates a new buffer vector for a vector.
       *
       * \param[in] vector
       * The vector for which the buffer is to be created.
       */
      template<typename Vector_>
      DenseVector<MemType, DataType, IndexType> create_buffer(const Vector_& vector) const
      {
        return DenseVector<MemType, DataType, IndexType>(buffer_size(vector), Pinning::disabled);
      }

      /**
       * \brief Gathers the buffer entries from a DenseVector.
       *
       * \param[in,out] buffer
       * A reference to the buffer.
       *
       * \param[in] vector
       * A vector whose entries are to be gathered.
       *
       * \param[in] buffer_offset
       * The offset within the buffer.
       */
      void gather(
        LAFEM::DenseVector<MemType, DataType, IndexType>& buffer,
        const LAFEM::DenseVector<MemType, DataType, IndexType>& vector,
        const Index buffer_offset = Index(0)) const
      {
        XASSERT(buffer_offset + this->num_indices() <= buffer.size());
        XASSERTM(this->size() == vector.size(), "size mismatch between mirror and vector");

        if(this->empty())
          return;

        LAFEM::Arch::Mirror<MemType>::gather_dv(
          buffer_offset, this->num_indices(), this->indices(), buffer.elements(), vector.elements());
      }

      /**
       * \brief Scatters the buffer entries onto a DenseVector.
       *
       * \param[in,out] vector
       * A reference to the vector.
       *
       * \param[in] buffer
       * A reference to the buffer whose entries are to be scattered.
       *
       * \param[in] alpha
       * A scaling factor for the operation.
       *
       * \param[in] buffer_offset
       * The offset within the buffer.
       */
      void scatter_axpy(
        LAFEM::DenseVector<MemType, DataType, IndexType>& vector,
        const LAFEM::DenseVector<MemType, DataType, IndexType>& buffer,
        const DataType alpha = DataType(1),
        const Index buffer_offset = Index(0)) const
      {
        XASSERT(buffer_offset + this->num_indices() <= buffer.size());
        XASSERTM(this->size() == vector.size(), "size mismatch between mirror and vector");

        if(this->empty())
          return;

        LAFEM::Arch::Mirror<MemType>::scatter_dv(
          buffer_offset, this->num_indices(), this->indices(), buffer.elements(), vector.elements(), alpha);
      }

      /**
       * \brief Gathers the buffer entries from a DenseVectorBlocked.
       *
       * \param[in,out] buffer
       * A reference to a buffer vector.
       *
       * \param[in] vector
       * A vector whose entries are to be gathered.
       *
       * \param[in] buffer_offset
       * The offset within the buffer vector.
       */
      template<int block_size_>
      void gather(
        LAFEM::DenseVector<MemType, DataType, IndexType>& buffer,
        const LAFEM::DenseVectorBlocked<MemType, DataType, IndexType, block_size_>& vector,
        const Index buffer_offset = Index(0)) const
      {
        XASSERT(buffer_offset + Index(block_size_)*this->num_indices() <= buffer.size());
        XASSERTM(this->size() == vector.size(), "size mismatch between mirror and vector");

        if(this->empty())
          return;

        LAFEM::Arch::Mirror<MemType>::gather_dvb(
          Index(block_size_), buffer_offset, this->num_indices(), this->indices(),
          buffer.elements(), vector.template elements<Perspective::pod>());
      }

      /**
       * \brief Scatters the buffer entries onto a DenseVectorBlocked.
       *
       * \param[in,out] vector
       * A reference to the vector.
       *
       * \param[in] buffer
       * A reference to the buffer whose entries are to be scattered.
       *
       * \param[in] alpha
       * A scaling factor for the operation.
       *
       * \param[in] buffer_offset
       * The offset within the buffer.
       */
      template<int block_size_>
      void scatter_axpy(
        LAFEM::DenseVectorBlocked<MemType, DataType, IndexType, block_size_>& vector,
        const LAFEM::DenseVector<MemType, DataType, IndexType>& buffer,
        const DataType alpha = DataType(1),
        const Index buffer_offset = Index(0)) const
      {
        XASSERT(buffer_offset + Index(block_size_)*this->num_indices() <= buffer.size());
        XASSERTM(this->size() == vector.size(), "size mismatch between mirror and vector");

        if(this->empty())
          return;

        LAFEM::Arch::Mirror<MemType>::scatter_dvb(
          Index(block_size_), buffer_offset, this->num_indices(), this->indices(),
          buffer.elements(), vector.template elements<Perspective::pod>(), alpha);
      }

      /**
       * \brief Gathers the buffer entries from a SparseVector.
       *
       * \param[in,out] buffer
       * A reference to a buffer vector.
       *
       * \param[in] vector
       * A vector whose entries are to be gathered.
       *
       * \param[in] buffer_offset
       * The offset within the buffer vector.
       */
      void gather(
        LAFEM::DenseVector<MemType, DataType, IndexType>& buffer,
        const LAFEM::SparseVector<MemType, DataType, IndexType>& vector,
        const Index buffer_offset = Index(0)) const
      {
        XASSERT(buffer_offset + this->num_indices() <= buffer.size());
        XASSERTM(this->size() == vector.size(), "size mismatch between mirror and vector");

        if(this->empty())
          return;

        LAFEM::Arch::Mirror<MemType>::gather_sv(
          buffer_offset, this->num_indices(), this->indices(), buffer.elements(),
          vector.used_elements(), vector.elements(), vector.indices());
      }

      /**
       * \brief Scatters the buffer entries onto a SparseVector.
       *
       * \param[in,out] vector
       * A reference to the vector.
       *
       * \param[in] buffer
       * A reference to the buffer whose entries are to be scattered.
       *
       * \param[in] alpha
       * A scaling factor for the operation.
       *
       * \param[in] buffer_offset
       * The offset within the buffer.
       */
      void scatter_axpy(
        LAFEM::SparseVector<MemType, DataType, IndexType>& vector,
        const LAFEM::DenseVector<MemType, DataType, IndexType>& buffer,
        const DataType alpha = DataType(1),
        const Index buffer_offset = Index(0)) const
      {
        XASSERT(buffer_offset + this->num_indices() <= buffer.size());
        XASSERTM(this->size() == vector.size(), "size mismatch between mirror and vector");

        if(this->empty())
          return;

        LAFEM::Arch::Mirror<MemType>::scatter_sv(
          buffer_offset, this->num_indices(), this->indices(), buffer.elements(),
          vector.used_elements(), vector.elements(), vector.indices(), alpha);
      }

      /**
       * \brief Gathers the buffer entries from a SparseVector.
       *
       * \param[in,out] buffer
       * A reference to a buffer vector.
       *
       * \param[in] vector
       * A vector whose entries are to be gathered.
       *
       * \param[in] buffer_offset
       * The offset within the buffer vector.
       */
      template<int block_size_>
      void gather(
        LAFEM::DenseVector<MemType, DataType, IndexType>& buffer,
        const LAFEM::SparseVectorBlocked<MemType, DataType, IndexType, block_size_>& vector,
        const Index buffer_offset = Index(0)) const
      {
        XASSERT(buffer_offset + Index(block_size_)*this->num_indices() <= buffer.size());
        XASSERTM(this->size() == vector.size(), "size mismatch between mirror and vector");

        if(this->empty())
          return;

        LAFEM::Arch::Mirror<MemType>::gather_svb(
          Index(block_size_), buffer_offset, this->num_indices(), this->indices(),buffer.elements(),
          vector.used_elements(), vector.template elements<Perspective::pod>(), vector.indices());
      }

      /**
       * \brief Scatters the buffer entries onto a SparseVectorBlocked.
       *
       * \param[in,out] vector
       * A reference to the vector.
       *
       * \param[in] buffer
       * A reference to the buffer whose entries are to be scattered.
       *
       * \param[in] alpha
       * A scaling factor for the operation.
       *
       * \param[in] buffer_offset
       * The offset within the buffer.
       */
      template<int block_size_>
      void scatter_axpy(
        LAFEM::SparseVectorBlocked<MemType, DataType, IndexType, block_size_>& vector,
        const LAFEM::DenseVector<MemType, DataType, IndexType>& buffer,
        const DataType alpha = DataType(1),
        const Index buffer_offset = Index(0)) const
      {
        XASSERT(buffer_offset + Index(block_size_)*this->num_indices() <= buffer.size());
        XASSERTM(this->size() == vector.size(), "size mismatch between mirror and vector");

        if(this->empty())
          return;

        LAFEM::Arch::Mirror<MemType>::scatter_svb(
          Index(block_size_), buffer_offset, this->num_indices(), this->indices(), buffer.elements(),
          vector.used_elements(), vector.template elements<Perspective::pod>(), vector.indices(), alpha);
      }

      friend std::ostream & operator<< (std::ostream & lhs, const VectorMirror & b)
      {
        Index n = b.num_indices();
        const IT_* idx = b.indices();

        lhs << "[";
        for (Index i(0) ; i < n ; ++i)
        {
          lhs << "  " << idx[i];
        }
        lhs << "]";

        return lhs;
      }

      /* ******************************************************************* */
      /*  A D J A C T O R   I N T E R F A C E   I M P L E M E N T A T I O N  */
      /* ******************************************************************* */
    public:
      /** \copydoc Adjactor::get_num_nodes_domain() */
      Index get_num_nodes_domain() const
      {
        return this->num_indices();
      }

      /** \copydoc Adjactor::get_num_nodes_image() */
      Index get_num_nodes_image() const
      {
        return this->size();
      }

      /** \copydoc Adjactor::image_begin() */
      ImageIterator image_begin(Index domain_node) const
      {
        XASSERTM(domain_node < num_indices(), "Domain node index out of range");
        return &this->_indices.at(0)[domain_node];
      }

      /** \copydoc Adjactor::image_end() */
      ImageIterator image_end(Index domain_node) const
      {
        XASSERTM(domain_node < num_indices(), "Domain node index out of range");
        return &this->_indices.at(0)[domain_node+1];
      }
    }; // class VectorMirror<...>
  } // namespace LAFEM
} // namespace FEAT

#endif // KERNEL_LAFEM_VECTOR_MIRROR_HPP
