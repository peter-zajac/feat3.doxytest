#pragma once
#ifndef KERNEL_GEOMETRY_LAPLACE_SMOOTHER_HPP
#define KERNEL_GEOMETRY_LAPLACE_SMOOTHER_HPP 1

#include <kernel/archs.hpp>

#include <kernel/assembly/bilinear_operator_assembler.hpp> // for BilinearOperatorAssembler
#include <kernel/assembly/common_operators.hpp>            // for LaplaceOperator
#include <kernel/assembly/symbolic_assembler.hpp>          // for SymbolicMatrixAssembler
#include <kernel/assembly/dirichlet_assembler.hpp>         // for DirichletAssembler
#include <kernel/cubature/dynamic_factory.hpp>             // for DynamicFactory
#include <kernel/geometry/conformal_mesh.hpp>
#include <kernel/trafo/standard/mapping.hpp>
#include <kernel/lafem/dense_vector.hpp>
#include <kernel/lafem/preconditioner.hpp>
#include <kernel/lafem/bicgstab.hpp>
#include <kernel/lafem/sparse_matrix_csr.hpp>
#include <kernel/lafem/unit_filter.hpp>
#include <kernel/geometry/mesh_smoother/mesh_smoother.hpp>
#include <kernel/space/lagrange1/element.hpp>

namespace FEAST
{
  namespace Geometry
  {

    /**
     * \brief Mesh optimiser based on minimisation of harmonic energy
     *
     * \tparam TrafoType_
     * Type of the underlying transformation.
     *
     * \tparam DataType_
     * Our datatype.
     *
     * \tparam MemType_
     * Memory architecture.
     *
     * \author Jordi Paul
     *
     */
    template<typename TrafoType_, typename DataType_, typename MemType_>
    class LaplaceSmoother :
      public MeshSmoother<TrafoType_, DataType_, MemType_>
    {
      public:
        /// Type for the transformation
        typedef TrafoType_ TrafoType;
        /// Our datatype
        typedef DataType_ DataType;
        /// Memory architecture
        typedef MemType_ MemType;

        /// Our base class
        typedef MeshSmoother<TrafoType, DataType, MemType> BaseClass;

        /// The mesh the transformation is defined on
        typedef typename TrafoType::MeshType MeshType;
        /// ShapeType of said mesh
        typedef typename MeshType::ShapeType ShapeType;

        /// FE space the transformation lives in
        typedef Space::Lagrange1::Element<TrafoType> SpaceType;

        /// Type for the vectors to hold coordinates etc.
        typedef LAFEM::DenseVector<MemType, DataType> VectorType;
        typedef LAFEM::SparseMatrixCSR<MemType, DataType> MatrixType;
        typedef LAFEM::UnitFilter<MemType, DataType> FilterType;

        typedef Algo::Generic AlgoType;

      protected:
        SpaceType _trafo_space;
        MatrixType _sys_matrix;
        VectorType _vec_rhs;
        Cubature::DynamicFactory _cubature_factory;
        FilterType _filter;
        Assembly::DirichletAssembler<SpaceType> _dirichlet_asm;

      public:
        /// Constructor
        explicit LaplaceSmoother(TrafoType& trafo_) :
          BaseClass(trafo_),
          _trafo_space(trafo_),
          _sys_matrix(),
          _vec_rhs(trafo_.get_mesh().get_num_entities(0)),
          _cubature_factory("auto-degree:5"),
          _filter(trafo_.get_mesh().get_num_entities(0)),
          _dirichlet_asm(_trafo_space)
      {
        /// Type for the boundary mesh
        typedef typename Geometry::CellSubSet<ShapeType> BoundaryType;
        /// Factory for the boundary mesh
        typedef typename Geometry::BoundaryFactory<MeshType> BoundaryFactoryType;


        // Get the boundary set
        BoundaryFactoryType boundary_factory(this->_mesh);
        BoundaryType boundary(boundary_factory);

        _dirichlet_asm.add_cell_set(boundary);

      }

        virtual ~LaplaceSmoother()
        {
        }

        /// \brief Initialises parts of the MeshSmoother not set in in the constructor
        virtual void init() override
        {
          BaseClass::init();
          Assembly::SymbolicMatrixAssembler<>::assemble1(_sys_matrix, _trafo_space);
        }

        /// \brief Optimises the mesh according to the criteria implemented in the mesh smoother.
        virtual void optimise() override
        {
          prepare();
          // Create a dummy preconditioner
          LAFEM::NonePreconditioner<AlgoType, MatrixType, VectorType> precond;
          for(Index d(0); d < this->_world_dim; ++d)
          {
            _dirichlet_asm.assemble(_filter,this->_coords[d]);

            _filter.template filter_mat<AlgoType>(_sys_matrix);

            _vec_rhs.format();
            _filter.template filter_rhs<AlgoType>(_vec_rhs);
//            _filter.template filter_sol<AlgoType>(this->_coords[d]);

            // Fire up the BiCGStab solver
            LAFEM::BiCGStab<AlgoType>::value(
              this->_coords[d],    // the initial solution vector
              _sys_matrix,     // the system matrix
              _vec_rhs,    // the right-hand-side vector
              precond,    // the dummy preconditioner
              1000,        // maximum number of iterations
              1E-8        // relative tolerance to achieve
              );
          }
          this->set_coords();

          return;
        }

        /// \copydoc MeshSmoother::prepare()
        virtual void prepare() override
        {
          _sys_matrix.format();
          Assembly::Common::LaplaceOperator laplace_operator;
          Assembly::BilinearOperatorAssembler::assemble_matrix1(
            _sys_matrix,           // the matrix that receives the assembled operator
            laplace_operator, // the operator that is to be assembled
            _trafo_space,            // the finite element space in use
            _cubature_factory  // the cubature factory to be used for integration
            );


          return;
        }



    }; // class LaplaceSmoother

  } // namespace Geometry
} // namespace FEAST
#endif // KERNEL_GEOMETRY_LAPLACE_SMOOTHER_HPP
