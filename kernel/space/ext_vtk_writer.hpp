// FEAT3: Finite Element Analysis Toolbox, Version 3
// Copyright (C) 2010 - 2019 by Stefan Turek & the FEAT group
// FEAT3 is released under the GNU General Public License version 3,
// see the file 'copyright.txt' in the top level directory for details.

#pragma once
#ifndef KERNEL_SPACE_EXT_VTK_WRITER_HPP
#define KERNEL_SPACE_EXT_VTK_WRITER_HPP 1

// includes, FEAT
#include <kernel/geometry/export_vtk.hpp>
#include <kernel/geometry/reference_cell_factory.hpp>
#include <kernel/space/eval_data.hpp>
#include <kernel/util/math.hpp>

// includes, system
#include <iostream>
#include <fstream>

namespace FEAT
{
  namespace Space
  {
    /// \cond internal
    namespace Intern
    {
      template<bool _enable>
      struct ExtVtkValueWriter
      {
        template<typename ExtVtkWriter_>
        static void write(const ExtVtkWriter_&, std::ostream&) {}
      };

      template<bool _enable>
      struct ExtVtkGradWriter
      {
        template<typename ExtVtkWriter_>
        static void write(const ExtVtkWriter_&, std::ostream&) {}
      };

      template<bool _enable>
      struct ExtVtkHessWriter
      {
        template<typename ExtVtkWriter_>
        static void write(const ExtVtkWriter_&, std::ostream&) {}
      };

      template<>
      struct ExtVtkValueWriter<true>
      {
        template<typename ExtVtkWriter_>
        static void write(const ExtVtkWriter_& writer, std::ostream& os)
        {
          writer.write_values(os);
        }
      };

      template<>
      struct ExtVtkGradWriter<true>
      {
        template<typename ExtVtkWriter_>
        static void write(const ExtVtkWriter_& writer, std::ostream& os)
        {
          writer.write_gradients(os);
        }
      };

      template<>
      struct ExtVtkHessWriter<true>
      {
        template<typename ExtVtkWriter_>
        static void write(const ExtVtkWriter_& writer, std::ostream& os)
        {
          writer.write_hessians(os);
        }
      };
    } // namespace Intern
    /// \endcond

    /**
     * \brief Extended VTK writer class template
     *
     * This class template implements a VTK writer which exports Finite Element functions evaluated on a locally
     * refined mesh instead of a projection in the mesh's vertices or cells.
     *
     * \author Peter Zajac
     */
    template<typename Trafo_>
    class ExtVtkWriter
    {
      friend struct Intern::ExtVtkValueWriter<true>;
      friend struct Intern::ExtVtkGradWriter<true>;
      friend struct Intern::ExtVtkHessWriter<true>;

    public:
      typedef Trafo_ TrafoType;
      typedef typename TrafoType::MeshType MeshType;
      typedef typename MeshType::ShapeType ShapeType;
      typedef typename Geometry::ConformalMesh<ShapeType> RefMeshType;

    protected:
      typedef typename TrafoType::template Evaluator<>::Type TrafoEval;

    protected:
      const TrafoType& _trafo;
      std::ofstream _ofs;
      RefMeshType* _ref_mesh;

    public:
      explicit ExtVtkWriter(const TrafoType& trafo, Index num_refines = 1) :
        _trafo(trafo),
        _ref_mesh(nullptr)
      {
        // create refined reference cell mesh
        typedef Geometry::ReferenceCellFactory<ShapeType> RefCellFactory;
        typedef Geometry::StandardRefinery<RefMeshType> RefCellRefinery;
        RefCellFactory ref_factory;
        RefMeshType* ref_mesh = new RefMeshType(ref_factory);
        for(Index i(0); i < num_refines; ++i)
        {
          RefMeshType* old_mesh = ref_mesh;
          {
            RefCellRefinery refinery(*old_mesh);
            ref_mesh = new RefMeshType(refinery);
          }
          delete old_mesh;
        }
        _ref_mesh = ref_mesh;
      }

      virtual ~ExtVtkWriter()
      {
        if(_ref_mesh != nullptr)
        {
          delete _ref_mesh;
        }
      }

      /**
       * \brief Opens a VTK files and writes basic information.
       */
      bool open(String filename)
      {
        _ofs.open(filename.c_str());
        if(!_ofs.is_open() || !_ofs.good())
          return false;

        // write VTK header
        _ofs << "# vtk DataFile Version 2.0" << std::endl;
        _ofs << "Generated by FEAT v" << version_major << "." << version_minor << "." << version_patch << std::endl;
        _ofs << "ASCII" << std::endl;

        // write mesh type
        _ofs << "DATASET UNSTRUCTURED_GRID" << std::endl;

        // write vertices
        write_vertices();

        // write indices
        write_indices();

        // write point data header
        _ofs << "POINT_DATA " << (_trafo.get_mesh().get_num_entities(ShapeType::dimension)
          * _ref_mesh->get_num_entities(0)) << std::endl;

        // okay
        return true;
      }

      /**
       * \brief Closes the VTK file.
       */
      void close()
      {
        _ofs.close();
      }

      /**
       * \brief Writes a finite element function to the VTK file.
       *
       * \param[in] name
       * The name of the variable for the VTK file.
       *
       * \param[in] space
       * A reference to the finite element space.
       *
       * \param[in] v
       * The blocked coefficient vector of the finite element function.
       */
      template<typename Space_, typename VectorType_>
      void write_values_blocked(String name, const Space_& space, const VectorType_& v)
      {
        typedef Space_ SpaceType;
        typedef typename SpaceType::template Evaluator<TrafoEval>::Type SpaceEval;
        static_assert(SpaceEval::can_value != 0, "space cannot evalute basis function values!");
        typedef typename SpaceType::DofMappingType DofMappingType;
        typedef typename SpaceEval::template ConfigTraits<SpaceTags::value> SpaceConfigTraits;
        typedef typename SpaceConfigTraits::EvalDataType SpaceData;
        static constexpr SpaceTags trafo_value_config = SpaceConfigTraits::trafo_config;
        typedef typename TrafoEval::template ConfigTraits<trafo_value_config>::EvalDataType TrafoData;
        typedef typename VectorType_::ValueType ValueType;
        Tiny::Vector<ValueType, SpaceEval::max_local_dofs> loc_vec;

        DofMappingType dof_map(space);
        TrafoEval trafo_eval(_trafo);
        SpaceEval space_eval(space);
        TrafoData trafo_data;
        SpaceData space_data;

        // write basic info
        _ofs << "VECTORS " << name << " double" << std::endl;

        // loop over all cells
        for(Index cell(trafo_eval.begin()); cell != trafo_eval.end(); ++cell)
        {
          // gather local vector
          dof_map.prepare(cell);
          for(int i(0); i < dof_map.get_num_local_dofs(); ++i)
          {
            loc_vec[i] = v(dof_map.get_index(i));
          }
          dof_map.finish();

          // prepare evaluators
          trafo_eval.prepare(cell);
          space_eval.prepare(trafo_eval);

          // loop over all points
          for(Index pt(0); pt < _ref_mesh->get_num_entities(0); ++pt)
          {
            // get domain point
            typename TrafoEval::DomainPointType dom_point;
            get_dom_point(dom_point, pt);

            // evaluate trafo and space
            trafo_eval(trafo_data, dom_point);
            space_eval(space_data, trafo_data);

            // compute value
            ValueType value(0);
            for(int i(0); i < space_eval.get_num_local_dofs(); ++i)
              value += space_data.phi[i].value * loc_vec[i];

            // write value
            for(int d(0); d < ValueType::n; ++d)
              _ofs << " " << value(d);

            for(int d(ValueType::n); d < 3; ++d)
              _ofs << " 0.0";
            _ofs << std::endl;
          }

          // finish evaluators
          space_eval.finish();
          trafo_eval.finish();
        }
      }

      /**
       * \brief Writes a finite element function to the VTK file.
       *
       * \param[in] name
       * The name of the variable for the VTK file.
       *
       * \param[in] space
       * A reference to the finite element space.
       *
       * \param[in] data
       * An array representing the coefficient vector of the finite element function.
       */
      template<typename Space_, typename T_>
      void write_values(String name, const Space_& space, const T_* data)
      {
        XASSERT(data != nullptr);
        typedef Space_ SpaceType;
        typedef typename SpaceType::template Evaluator<TrafoEval>::Type SpaceEval;
        static_assert(*(SpaceEval::eval_caps & SpaceTags::value), "space cannot evalute basis function values!");
        typedef typename SpaceType::DofMappingType DofMappingType;
        typedef typename SpaceEval::template ConfigTraits<SpaceTags::value> SpaceConfigTraits;
        typedef typename SpaceConfigTraits::EvalDataType SpaceData;
        static constexpr TrafoTags trafo_value_config = SpaceConfigTraits::trafo_config;
        typedef typename TrafoEval::template ConfigTraits<trafo_value_config>::EvalDataType TrafoData;
        typedef typename SpaceEval::DataType DataType;
        Tiny::Vector<DataType, SpaceEval::max_local_dofs> loc_vec;

        DofMappingType dof_map(space);
        TrafoEval trafo_eval(_trafo);
        SpaceEval space_eval(space);
        TrafoData trafo_data;
        SpaceData space_data;

        // write basic info
        _ofs << "SCALARS " << name << " double 1" << std::endl;
        _ofs << "LOOKUP_TABLE default" << std::endl;

        // loop over all cells
        for(Index cell(trafo_eval.begin()); cell != trafo_eval.end(); ++cell)
        {
          // gather local vector
          dof_map.prepare(cell);
          for(int i(0); i < dof_map.get_num_local_dofs(); ++i)
          {
            loc_vec[i] = DataType(data[dof_map.get_index(i)]);
          }
          dof_map.finish();

          // prepare evaluators
          trafo_eval.prepare(cell);
          space_eval.prepare(trafo_eval);

          // loop over all points
          for(Index pt(0); pt < _ref_mesh->get_num_entities(0); ++pt)
          {
            // get domain point
            typename TrafoEval::DomainPointType dom_point;
            get_dom_point(dom_point, pt);

            // evaluate trafo and space
            trafo_eval(trafo_data, dom_point);
            space_eval(space_data, trafo_data);

            // compute value
            DataType value(0);
            for(int i(0); i < space_eval.get_num_local_dofs(); ++i)
              value += loc_vec[i] * space_data.phi[i].value;

            // write value
            _ofs << value << std::endl;
          }

          // finish evaluators
          space_eval.finish();
          trafo_eval.finish();
        }
      }

      /**
       * \brief Writes a finite element function gradient field to the VTK file.
       *
       * \param[in] name
       * The name of the variable for the VTK file.
       *
       * \param[in] space
       * A reference to the finite element space.
       *
       * \param[in] data
       * An array representing the coefficient vector of the finite element function.
       */
      template<typename Space_, typename T_>
      void write_gradients(String name, const Space_& space, const T_* data)
      {
        XASSERT(data != nullptr);
        typedef Space_ SpaceType;
        typedef typename SpaceType::template Evaluator<TrafoEval>::Type SpaceEval;
        static_assert(*(SpaceEval::eval_caps & SpaceTags::grad), "space cannot evalute basis function gradients!");
        typedef typename SpaceType::DofMappingType DofMappingType;
        typedef typename SpaceEval::template ConfigTraits<SpaceTags::grad> SpaceConfigTraits;
        typedef typename SpaceConfigTraits::EvalDataType SpaceData;
        static constexpr TrafoTags trafo_grad_config = SpaceConfigTraits::trafo_config;
        typedef typename TrafoEval::template ConfigTraits<trafo_grad_config>::EvalDataType TrafoData;
        typedef typename SpaceEval::DataType DataType;
        typedef typename SpaceEval::SpaceEvalTraits SpaceEvalTraits;
        typedef typename SpaceEvalTraits::BasisGradientType BasisGradientType;
        Tiny::Vector<DataType, SpaceEval::max_local_dofs> loc_vec;

        DofMappingType dof_map(space);
        TrafoEval trafo_eval(_trafo);
        SpaceEval space_eval(space);
        TrafoData trafo_data;
        SpaceData space_data;

        // write basic info
        _ofs << "VECTORS " << name << " double" << std::endl;

        // loop over all cells
        for(Index cell(trafo_eval.begin()); cell != trafo_eval.end(); ++cell)
        {
          // gather local vector
          dof_map.prepare(cell);
          for(int i(0); i < dof_map.get_num_local_dofs(); ++i)
          {
            loc_vec[i] = DataType(data[dof_map.get_index(i)]);
          }
          dof_map.finish();

          // prepare evaluators
          trafo_eval.prepare(cell);
          space_eval.prepare(trafo_eval);

          // loop over all points
          for(Index pt(0); pt < _ref_mesh->get_num_entities(0); ++pt)
          {
            // get domain point
            typename TrafoEval::DomainPointType dom_point;
            get_dom_point(dom_point, pt);

            // evaluate trafo and space
            trafo_eval(trafo_data, dom_point);
            space_eval(space_data, trafo_data);

            // compute value
            BasisGradientType grad;
            grad.format();
            for(int i(0); i < space_eval.get_num_local_dofs(); ++i)
              grad += loc_vec[i] * space_data.phi[i].grad;

            // write value
            _ofs << grad[0];
            for(int i(1); i < ShapeType::dimension; ++i)
              _ofs << " " << grad[i];
            for(int i(ShapeType::dimension); i < 3; ++i)
              _ofs << " 0.0";
            _ofs << std::endl;
          }

          // finish evaluators
          space_eval.finish();
          trafo_eval.finish();
        }

      }

      /**
       * \brief Writes a finite element function Hessian field to the VTK file.
       *
       * \param[in] name
       * The name of the variable for the VTK file.
       *
       * \param[in] space
       * A reference to the finite element space.
       *
       * \param[in] data
       * An array representing the coefficient vector of the finite element function.
       */
      template<typename Space_, typename T_>
      void write_hessians(String name, const Space_& space, const T_* data)
      {
        XASSERT(data != nullptr);
        typedef Space_ SpaceType;
        typedef typename SpaceType::template Evaluator<TrafoEval>::Type SpaceEval;
        static_assert(*(SpaceEval::eval_caps & SpaceTags::hess), "space cannot evalute basis function hessians!");
        typedef typename SpaceType::DofMappingType DofMappingType;
        typedef typename SpaceEval::template ConfigTraits<SpaceTags::hess> SpaceConfigTraits;
        typedef typename SpaceConfigTraits::EvalDataType SpaceData;
        static constexpr TrafoTags trafo_hess_config = SpaceConfigTraits::trafo_config;
        typedef typename TrafoEval::template ConfigTraits<trafo_hess_config>::EvalDataType TrafoData;
        typedef typename SpaceEval::DataType DataType;
        typedef typename SpaceEval::SpaceEvalTraits SpaceEvalTraits;
        typedef typename SpaceEvalTraits::BasisHessianType BasisHessianType;
        Tiny::Vector<DataType, SpaceEval::max_local_dofs> loc_vec;

        DofMappingType dof_map(space);
        TrafoEval trafo_eval(_trafo);
        SpaceEval space_eval(space);
        TrafoData trafo_data;
        SpaceData space_data;

        // write basic info
        _ofs << "TENSORS " << name << " double" << std::endl;

        // loop over all cells
        for(Index cell(trafo_eval.begin()); cell != trafo_eval.end(); ++cell)
        {
          // gather local vector
          dof_map.prepare(cell);
          for(int i(0); i < dof_map.get_num_local_dofs(); ++i)
          {
            loc_vec[i] = DataType(data[dof_map.get_index(i)]);
          }
          dof_map.finish();

          // prepare evaluators
          trafo_eval.prepare(cell);
          space_eval.prepare(trafo_eval);

          // loop over all points
          for(Index pt(0); pt < _ref_mesh->get_num_entities(0); ++pt)
          {
            // get domain point
            typename TrafoEval::DomainPointType dom_point;
            get_dom_point(dom_point, pt);

            // evaluate trafo and space
            trafo_eval(trafo_data, dom_point);
            space_eval(space_data, trafo_data);

            // compute value
            BasisHessianType hess;
            hess.format();
            for(int i(0); i < space_eval.get_num_local_dofs(); ++i)
              hess += loc_vec[i] * space_data.phi[i].hess;

            // write value
            for(int i(0); i < ShapeType::dimension; ++i)
            {
              _ofs << hess(i,0);
              for(int j(1); j < ShapeType::dimension; ++j)
                _ofs << " " << hess(i,j);
              for(int j(ShapeType::dimension); j < 3; ++j)
                _ofs << " 0.0";
              _ofs << std::endl;
            }
            for(int i(ShapeType::dimension); i < 3; ++i)
            {
              _ofs << "0.0";
              for(int j(1); j < 3; ++j)
                _ofs << " 0.0";
              _ofs << std::endl;
            }
          }

          // finish evaluators
          space_eval.finish();
          trafo_eval.finish();
        }
      }

    protected:
      void write_vertices()
      {
        // fetch index set and vertex set
        const TrafoType& trafo = _trafo;
        const MeshType& mesh = trafo.get_mesh();

        // write vertex set
        const Index num_cells = mesh.get_num_entities(ShapeType::dimension);
        const Index num_ref_verts = num_cells * _ref_mesh->get_num_entities(0);
        _ofs << "POINTS " << num_ref_verts << " double" << std::endl;

        // create trafo evaluator and data
        typedef typename TrafoEval::DomainPointType DomainPointType;
        typedef typename TrafoEval::template ConfigTraits<TrafoTags::img_point>::EvalDataType TrafoData;
        DomainPointType dom_point;
        TrafoEval trafo_eval(trafo);
        TrafoData trafo_data;

        // loop over all cells
        for(Index cell(trafo_eval.begin()); cell != trafo_eval.end(); ++cell)
        {
          // prepare trafo
          trafo_eval.prepare(cell);

          // loop over all reference vertices
          for(Index i(0); i < _ref_mesh->get_num_entities(0); ++i)
          {
            // get reference vertex
            get_dom_point(dom_point, i);

            // apply trafo
            trafo_eval(trafo_data, dom_point);

            // write image coords
            _ofs << trafo_data.img_point[0];
            for(int j(1); j < ShapeType::dimension; ++j)
              _ofs << " " << trafo_data.img_point[j];
            for(int j(ShapeType::dimension); j < 3; ++j)
              _ofs << " 0.0";
            _ofs << std::endl;
          }

          // finish trafo
          trafo_eval.finish();
        }
      }

      void write_indices()
      {
        // fetch index set and vertex set
        const TrafoType& trafo = _trafo;
        const MeshType& mesh = trafo.get_mesh();

        // write index set
        const Index num_cells = mesh.get_num_entities(ShapeType::dimension);
        const Index num_ref_verts = _ref_mesh->get_num_entities(0);
        const Index num_ref_cells = _ref_mesh->get_num_entities(ShapeType::dimension);
        const typename RefMeshType::template IndexSet<ShapeType::dimension,0>::Type& idx =
          _ref_mesh->template get_index_set<ShapeType::dimension, 0>();
        int num_idx = idx.get_num_indices();

        typedef Geometry::Intern::VTKShape<ShapeType> VTKHelperType;

        _ofs << "CELLS " << (num_cells*num_ref_cells) << " " << (Index(num_idx+1)*num_cells*num_ref_cells) << std::endl;

        // loop over all mesh cells
        for(Index cell(0); cell < num_cells; ++cell)
        {
          // compute vertex offset
          Index voff = num_ref_verts * cell;
          for(Index i(0); i < num_ref_cells; ++i)
          {
            _ofs << num_idx;
            for(int j(0); j < num_idx; ++j)
            {
              _ofs << " " << voff + idx(i,VTKHelperType::map(j));
            }
            _ofs << std::endl;
          }
        }

        // write cell types
        _ofs << "CELL_TYPES " << (num_cells*num_ref_cells) << std::endl;

        // loop over all mesh cells
        for(Index cell(0); cell < num_cells*num_ref_cells; ++cell)
        {
          _ofs << VTKHelperType::type << std::endl;
        }
      }

      void get_dom_point(typename TrafoEval::DomainPointType& dom_point, Index i) const
      {
        // get reference mesh vertex set
        const typename RefMeshType::VertexSetType& ref_vtx = _ref_mesh->get_vertex_set();

        // set domain point
        for(int j(0); j < ShapeType::dimension; ++j)
          dom_point[j] = ref_vtx[i][j];
      }
    };
  } // namespace Space
} // namespace FEAT

#endif // KERNEL_SPACE_EXT_VTK_WRITER_HPP
