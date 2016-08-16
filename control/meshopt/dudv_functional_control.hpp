#pragma once
#ifndef FEAT_CONTROL_MESHOPT_DUDV_FUNCTIONAL_CONTROL_HPP
#define FEAT_CONTROL_MESHOPT_DUDV_FUNCTIONAL_CONTROL_HPP 1
#include <kernel/base_header.hpp>
#include <kernel/archs.hpp>

#include <kernel/assembly/bilinear_operator_assembler.hpp>
#include <kernel/assembly/common_operators.hpp>
#include <kernel/lafem/sparse_matrix_csr.hpp>
#include <kernel/lafem/sparse_matrix_bcsr.hpp>
#include <kernel/lafem/sparse_matrix_bwrappedcsr.hpp>
#include <kernel/lafem/vector_mirror.hpp>
#include <kernel/geometry/export_vtk.hpp>
#include <kernel/global/gate.hpp>
#include <kernel/global/matrix.hpp>
#include <kernel/global/nonlinear_functional.hpp>
#include <kernel/global/vector.hpp>
#include <kernel/meshopt/dudv_functional.hpp>

#include <control/domain/domain_control.hpp>
#include <control/meshopt/meshopt_control.hpp>
#include <control/meshopt/meshopt_solver_factory.hpp>

namespace FEAT
{
  namespace Control
  {
    namespace Meshopt
    {
      template<typename>
      class DuDvFunctionalAssemblerLevel;

      /**
       * \brief Control class for DuDvFunctionals
       *
       * \tparam Mem_
       * The memory architecture of the local DuDvFunctional
       *
       * \tparam DT_
       * The floating point precision for the solver
       *
       * \tparam IT_
       * Index type
       *
       * \tparam DomainControl_
       * The domain control type this is based on
       *
       * \tparam Trafo_
       * The mesh's underlying transformation. At the time of writing, there is only Trafo::Standard, which means
       * P1/Q1 transformation.
       *
       * \author Jordi Paul
       *
       */
      template<typename Mem_, typename DT_, typename IT_, typename DomainControl_, typename Trafo_>
      class DuDvFunctionalControl
      : public MeshoptControlBase<DomainControl_, Trafo_>
      {
        public:
          /// Our memory architecture
          typedef Mem_ MemType;
          /// The floating point type
          typedef DT_ DataType;
          /// The index type
          typedef IT_ IndexType;
          /// The transformation we solve for
          typedef Trafo_ TrafoType;
          /// The type of the domain control
          typedef DomainControl_ DomainControlType;
          /// The underlying mesh type
          typedef typename DomainControl_::MeshType MeshType;
          /// The floating point type the mesh's coordinates use
          typedef typename MeshType::CoordType CoordType;
          /// The FE space the transformation lives in
          typedef typename FEAT::Meshopt::Intern::TrafoFE<Trafo_>::Space TrafoSpace;

          /// Our base class
          typedef MeshoptControlBase<DomainControl_, Trafo_> BaseClass;

          /// Template-alias away the Trafo so the SystemLevel can take it as a template template parameter
          template<typename A, typename B, typename C>
          using OperatorType =  FEAT::Meshopt::DuDvFunctional<A, B, C, Trafo_>;
          /// Linear system of equations on one refinement level
          typedef MeshoptSystemLevel<Mem_, DT_, IT_, OperatorType, Global::Matrix> SystemLevelType;

          /// Inter-level transfer matrix
          typedef LAFEM::SparseMatrixBWrappedCSR<Mem_, DT_, IT_, MeshType::world_dim> TransferMatrixType;
          /// Type to do all inter level information transfer
          typedef MeshoptTransferLevel<SystemLevelType, TransferMatrixType> TransferLevelType;

          typedef typename DomainControl_::LayerType DomainLayerType;
          typedef typename DomainControl_::LevelType DomainLevelType;
          typedef DuDvFunctionalAssemblerLevel<TrafoSpace> AssemblerLevelType;

          typedef typename SystemLevelType::GlobalSystemVectorL GlobalSystemVectorL;
          typedef typename SystemLevelType::GlobalSystemVectorR GlobalSystemVectorR;

          /// For every level of refinement we have one assembler level
          std::deque<AssemblerLevelType*> _assembler_levels;
          /// For every level of refinement, we have one system level
          std::deque<SystemLevelType*> _system_levels;
          /// Two subsequent levels can communicate through their transfer level
          std::deque<TransferLevelType*> _transfer_levels;

        public:
          /// Number of refinement levels
          const Index num_levels;
          /// Solver configuration
          PropertyMap& solver_config;
          /// The name of the section from solver_config we want to use
          String solver_name;
          /// The solver
          std::shared_ptr<Solver::SolverBase<GlobalSystemVectorR>> solver;
          /// Whether to reassemble the system matrix in every call of optimise
          const bool fixed_reference_domain;

          explicit DuDvFunctionalControl(
            DomainControl_& dom_ctrl, const std::deque<String>& dirichlet_list, const std::deque<String>& slip_list,
            const String& solver_name_, PropertyMap& solver_config_, bool fixed_reference_domain_):
            _assembler_levels(),
            _system_levels(),
            _transfer_levels(),
            num_levels(dom_ctrl.get_levels().size()),
            solver_config(solver_config_),
            solver_name(solver_name_),
            solver(nullptr),
            fixed_reference_domain(fixed_reference_domain_)
            {
              XASSERT(num_levels > Index(0));

              const DomainLayerType& layer = *dom_ctrl.get_layers().back();
              const std::deque<DomainLevelType*>& domain_levels = dom_ctrl.get_levels();

              for(Index i(0); i < num_levels; ++i)
              {
                _assembler_levels.push_back(new AssemblerLevelType(*domain_levels.at(i), dirichlet_list, slip_list));
                _system_levels.push_back(new SystemLevelType(dirichlet_list, slip_list,
                  domain_levels.at(i)->get_mesh_node(),
                  &(_assembler_levels.at(i)->trafo_space),
                  &(_assembler_levels.at(i)->dirichlet_asm),
                  &(_assembler_levels.at(i)->slip_asm)));

                // This assembles the system matrix symbolically
                (*(_system_levels.at(i)->op_sys)).init();

                if(i > 0)
                  _transfer_levels.push_back(new TransferLevelType(*_system_levels.at(i-1), *_system_levels.at(i)));
              }

              for(Index i(0); i < num_levels; ++i)
                _assembler_levels.at(i)->assemble_gates(layer, *_system_levels.at(i));

              for(Index i(0); (i+1) < num_levels; ++i)
                _assembler_levels.at(i+1)->assemble_system_transfer(*_transfer_levels.at(i), *_assembler_levels.at(i));

              for(Index i(0); i < num_levels; ++i)
                _assembler_levels.at(i)->assemble_system_matrix(*_system_levels.at(i));

              // create our solver
              solver = Control::MeshoptSolverFactory::create_linear_solver
                (_system_levels, _transfer_levels, &solver_config, solver_name);
              // initialise
              solver->init();
            }

          /// Explicitly delete empty default constructor
          DuDvFunctionalControl() = delete;
          /// Explicitly delete move constructor
          DuDvFunctionalControl(DuDvFunctionalControl&&) = delete;

          /**
           * \brief Virtual destructor
           */
          virtual ~DuDvFunctionalControl()
          {
            while(!_assembler_levels.empty())
            {
              delete _assembler_levels.back();
              _assembler_levels.pop_back();
            }

            while(!_system_levels.empty())
            {
              delete _system_levels.back();
              _system_levels.pop_back();
            }

            while(!_transfer_levels.empty())
            {
              delete _transfer_levels.back();
              _transfer_levels.pop_back();
            }

            solver->done();
          }

          /// \copydoc BaseClass::name()
          virtual String name() const override
          {
            return "DuDvFunctionalControl<>";
          }

          /// \copydoc BaseClass::print()
          virtual void print() const override
          {
            Util::mpi_cout(name()+" settings:\n");
            Util::mpi_cout_pad_line("Domain level min",_assembler_levels.front()->domain_level.get_level_index());
            Util::mpi_cout_pad_line("Domain level max",_assembler_levels.back()->domain_level.get_level_index());
            Util::mpi_cout_pad_line("Fixed reference domain",fixed_reference_domain);
            for(const auto& it : get_dirichlet_boundaries())
              Util::mpi_cout_pad_line("Displacement BC on",it);
            for(const auto& it : get_slip_boundaries())
              Util::mpi_cout_pad_line("Unilateral BC of place on",it);
            Util::mpi_cout_pad_line("DoF",_system_levels.back()->op_sys.columns());

            FEAT::Statistics::expression_target = name();
            try
            {
              Util::mpi_cout_pad_line("Solver",FEAT::Statistics::get_formatted_solver_tree().trim() + "\n");
            }
            catch(std::exception& e)
            {
            }

          }

          /// \copydoc BaseClass::compute_cell_size_defect()
          virtual CoordType compute_cell_size_defect(CoordType& lambda_min, CoordType& lambda_max,
              CoordType& vol_min, CoordType& vol_max) const override
          {
            return (*(_system_levels.back()->op_sys)).
              compute_cell_size_defect(lambda_min, lambda_max, vol_min, vol_max);
          }

          /// \copydoc BaseClass::get_coords()
          virtual typename SystemLevelType::GlobalCoordsBuffer& get_coords() override
          {
            return _system_levels.back()->coords_buffer;
          }

          /// \copydoc BaseClass::buffer_to_mesh()
          virtual void buffer_to_mesh() override
          {
            // Write finest level
            (*(_system_levels.back()->op_sys)).buffer_to_mesh();

            // Get the coords buffer on the finest level
            const auto& coords_buffer_loc = (*(_system_levels.back()->op_sys)).get_coords();

            // Transfer fine coords buffer to coarser levels and perform buffer_to_mesh
            for(size_t level(num_levels-1); level > 0; )
            {
              --level;
              Index ndofs(_assembler_levels.at(level)->trafo_space.get_num_dofs());

              // At this point, what we really need is a primal restriction operator that restricts the FE function
              // representing the coordinate distribution to the coarser level. This is very simple for continuous
              // Lagrange elements (just discard the additional information from the fine level), but not clear in
              // the generic case. So we use an evil hack here:
              // Because of the underlying two level ordering, we just need to copy the first ndofs entries from
              // the fine level vector.
              typename SystemLevelType::LocalCoordsBuffer
                vec_level(coords_buffer_loc, ndofs, Index(0));

              (*(_system_levels.at(level)->op_sys)).get_coords().copy(vec_level);
              (*(_system_levels.at(level)->op_sys)).buffer_to_mesh();
            }
          }

          /// \copydoc BaseClass::mesh_to_buffer()
          virtual void mesh_to_buffer() override
          {
            // Write finest level
            (*(_system_levels.back()->op_sys)).mesh_to_buffer();

            // Get the coords buffer on the finest level
            const auto& coords_buffer_loc = *(_system_levels.back()->coords_buffer);

            // Transfer fine coords buffer to coarser levels and perform buffer_to_mesh
            for(size_t level(num_levels-1); level > 0; )
            {
              --level;
              Index ndofs(_assembler_levels.at(level)->trafo_space.get_num_dofs());

              // At this point, what we really need is a primal restriction operator that restricts the FE function
              // representing the coordinate distribution to the coarser level. This is very simple for continuous
              // Lagrange elements (just discard the additional information from the fine level), but not clear in
              // the generic case. So we use an evil hack here:
              // Because of the underlying two level ordering, we just need to copy the first ndofs entries from
              // the fine level vector.
              typename SystemLevelType::LocalCoordsBuffer
                vec_level(coords_buffer_loc, ndofs, Index(0));

              (*(_system_levels.at(level)->op_sys)).get_coords().copy(vec_level);
            }

          }

          /// \copydoc BaseClass::get_dirichlet_boundaries()
          virtual std::deque<String> get_dirichlet_boundaries() const override
          {
            std::deque<String> dirichlet_boundaries;

            for(const auto& it:_assembler_levels.back()->dirichlet_asm)
              dirichlet_boundaries.push_back(it.first);

            return dirichlet_boundaries;
          }

          /// \copydoc BaseClass::get_slip_boundaries()
          virtual std::deque<String> get_slip_boundaries() const override
          {
            std::deque<String> slip_boundaries;

            for(const auto& it:_assembler_levels.back()->slip_asm)
              slip_boundaries.push_back(it.first);

            return slip_boundaries;
          }

          /**
           * \brief Numerically assembles the functional for evaluation
           *
           * This is mainly for using this functional as a preconditioner for the HyperelasticityFunctional so it
           * can be called separately.
           *
           */
          virtual void init_numeric()
          {
            if(!fixed_reference_domain)
            {
              for(size_t lvl(0); lvl < size_t(num_levels); ++lvl)
                _assembler_levels.at(lvl)->assemble_system_matrix(*(_system_levels.at(lvl)));
            }
          }

          /// \copydoc BaseClass::prepare()
          virtual void prepare(const GlobalSystemVectorR& vec_state) override
          {
            typename SystemLevelType::LocalCoordsBuffer vec_buf;
            vec_buf.convert(*vec_state);

            for(size_t level(num_levels); level > 0; )
            {
              --level;
              Index ndofs(_assembler_levels.at(level)->trafo_space.get_num_dofs());

              // At this point, what we really need is a primal restriction operator that restricts the FE function
              // representing the coordinate distribution to the coarser level. This is very simple for continuous
              // Lagrange elements (just discard the additional information from the fine level), but not clear in
              // the generic case. So we use an evil hack here:
              // Because of the underlying two level ordering, we just need to copy the first ndofs entries from
              // the fine level vector.
              typename SystemLevelType::GlobalCoordsBuffer
                global_vec_level( &(_system_levels.at(level)->gate_sys), vec_buf, ndofs, Index(0));

              (*(_system_levels.at(level)->op_sys)).prepare(vec_buf, *(_system_levels.at(level)->filter_sys));

              _assembler_levels.at(level)->assemble_system_filter(*(_system_levels.at(level)), global_vec_level);
            }

          }

          /**
           * \brief Applies the inverse of this functional's gradient to a right hand side
           *
           * \param[in,out] vec_sol
           * Initial guess, gets overwritten with the solution.
           *
           * \param[in] vec_rhs
           * Right hand side.
           *
           * This is for using this functional as a preconditioner i.e. for the HyperelasticityFunctional
           */
          virtual Solver::Status apply(GlobalSystemVectorR& vec_sol, const GlobalSystemVectorL& vec_rhs)
          {
            // Get our global system matrix and filter
            typename SystemLevelType::GlobalQualityFunctional& op_sys = (*_system_levels.back()).op_sys;
            typename SystemLevelType::GlobalSystemFilter& filter_sys = (*_system_levels.back()).filter_sys;

            return Solver::solve(*solver, vec_sol, vec_rhs, op_sys, filter_sys);
          }

          /// \copydoc BaseClass()::optimise()
          virtual void optimise() override
          {
            // Reassemble the system matrix
            init_numeric();

            // fetch our finest levels
            //DomainLevelType& the_domain_level = *domain_levels.back();
            SystemLevelType& the_system_level = *_system_levels.back();
            AssemblerLevelType& the_asm_level = *_assembler_levels.back();

            // Create our RHS and SOL vectors
            GlobalSystemVectorR vec_rhs = the_asm_level.assemble_rhs_vector(the_system_level);
            GlobalSystemVectorL vec_sol = the_asm_level.assemble_sol_vector(the_system_level);

            // Let it be knownst to Statitics that it was Us who called the solver
            FEAT::Statistics::expression_target = name();

            // solve
            this->apply(vec_sol, vec_rhs);

            // Write the solution to the control object's buffer and the buffer to mesh
            typename SystemLevelType::LocalCoordsBuffer vec_buf;
            vec_buf.convert(*vec_sol);
            (*(the_system_level.coords_buffer)).copy(vec_buf);

            buffer_to_mesh();

            // Call prepare again to project the slip boundaries
            prepare(vec_sol);

          }

      };

      /**
       * \copydoc Control::Meshopt::MeshoptAssemblerLevel
       */
      template<typename Space_>
      class DuDvFunctionalAssemblerLevel : public MeshoptAssemblerLevel<Space_>
      {
        public:
          /// Our baseclass
          typedef Control::Meshopt::MeshoptAssemblerLevel<Space_> BaseClass;
          /// Type of the underlying transformation
          typedef typename BaseClass::TrafoType TrafoType;
          /// The shape dimension of the mesh's cells
          static constexpr int shape_dim = BaseClass::MeshType::ShapeType::dimension;

          // Copy baseclass constructors
          using BaseClass::BaseClass;

          /**
           * \brief Assembles the system matrix on the given system level
           *
           * This is just a wrapper around the local functional's routine
           */
          template<typename SystemLevel_>
          void assemble_system_matrix(SystemLevel_& sys_level)
          {
            (*(sys_level.op_sys)).assemble_system_matrix();
          }

          /**
           * \brief Assembles the initial guess on the given system level
           *
           * This just copies the mesh's vertex coordinates and filters them.
           */
          template<typename SystemLevel_>
          typename SystemLevel_::GlobalSystemVectorR assemble_sol_vector(SystemLevel_& sys_level)
          {
            XASSERTM(!(*sys_level.op_sys).empty(), "assemble_sol_vector for empty operator");
            typename SystemLevel_::GlobalSystemVectorR vec_sol(sys_level.op_sys.create_vector_r());
            vec_sol.clone(sys_level.coords_buffer, LAFEM::CloneMode::Deep);

            sys_level.filter_sys.filter_sol(vec_sol);

            return vec_sol;
          }
      }; // class DuDvFunctionalAssemblerLevel

    } // namespace Meshopt
  } // namespace Control
} // namespace FEAT

#endif// FEAT_CONTROL_MESHOPT_DUDV_FUNCTIONAL_CONTROL_HPP
