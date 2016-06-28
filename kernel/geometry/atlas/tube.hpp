#pragma once
#ifndef KERNEL_GEOMETRY_ATLAS_TUBE_HPP
#define KERNEL_GEOMETRY_ATLAS_TUBE_HPP 1

#include <kernel/geometry/atlas/chart.hpp>

namespace FEAT
{
  namespace Geometry
  {
    namespace Atlas
    {
      /// Tube chart traits
      struct TubeTraits
      {
        /// we support explicit map
        static constexpr bool is_explicit = false;
        /// we support implicit projection
        static constexpr bool is_implicit = true;
        /// this is a 2D object
        static constexpr int world_dim = 3;
        /// we have 1D parameters
        static constexpr int param_dim = 2;
      };

      /**
       * \brief Tube chart class template
       *
       * This class represents a 3D tube (aka the curved boundary part of a cylinder)
       * characterised by a midpoint, a rotation axis and a radius.
       *
       * This chart implements only the implicit chart interface.
       *
       * \tparam Mesh_
       * The type of the mesh to be parameterised by this chart.
       *
       * \author Peter Zajac
       */
      template<typename Mesh_>
      class Tube :
        public ChartCRTP<Tube<Mesh_>, Mesh_, TubeTraits>
      {
      public:
        /// CRTP base class
        typedef ChartCRTP<Tube<Mesh_>, Mesh_, TubeTraits> BaseClass;
        /// Floating point type for coordinates
        typedef typename BaseClass::CoordType DataType;
        /// Vector type for world points aka. image points
        typedef typename BaseClass::WorldPoint WorldPoint;
        /// Vector type for parameter points aka. domain points
        typedef typename BaseClass::ParamPoint ParamPoint;

      protected:
        /// the tube's midpoint
        WorldPoint _midpoint;
        /// the tube's rotation axis
        WorldPoint _rot_axis;
        /// the tube's rotation axis inverse length
        DataType _rot_scale;
        /// the tube's radius
        DataType _radius;

      public:
        /**
         * \brief Constructor
         *
         * \param[in] mid_x, mid_y, mid_z
         * The coordinates of the tube midpoint.
         *
         * \param[in] axis_x, axis_y, axis_z
         * The components of the rotation axis vector. At least one of those three parameters must
         * be non-zero, but the axis vector need not be normalised.
         *
         * \param[in] radius
         * The radius of the tube. Must be positive.
         */
        explicit Tube(
          DataType mid_x, DataType mid_y, DataType mid_z,
          DataType axis_x, DataType axis_y, DataType axis_z,
          DataType radius) :
          _radius(radius)
        {
          XASSERTM(radius > DataType(0), "invalid Tube radius");
          _midpoint[0] = mid_x;
          _midpoint[1] = mid_y;
          _midpoint[2] = mid_z;
          _rot_axis[0] = axis_x;
          _rot_axis[1] = axis_y;
          _rot_axis[2] = axis_z;
          // get inverse rotation axis length
          _rot_scale = (DataType(1) / Math::sqr(_rot_axis.norm_euclid()));
        }

        /** \copydoc ChartBase::get_type() */
        virtual String get_type() const override
        {
          return "tube";
        }

        /// \copydoc ChartBase::move_by()
        void move_me_by(const WorldPoint& translation)
        {
            _midpoint += translation;
        }

        /// \copydoc ChartBase::rotate()
        virtual void rotate(const WorldPoint& centre, const WorldPoint& angles)
        {
          // This is the 3x3 matrix representing the turning by the angle angles(0)
          Tiny::Matrix<DataType, 3, 3> rot(DataType(0));

          DataType c0 = Math::cos(angles(0));
          DataType c1 = Math::cos(angles(1));
          DataType c2 = Math::cos(angles(2));

          DataType s0 = Math::sin(angles(0));
          DataType s1 = Math::sin(angles(1));
          DataType s2 = Math::sin(angles(2));

          rot(0,0) = c1*c2;
          rot(0,1) = c0*s2 + s0*s1*c2;
          rot(0,2) = s0*s2 - c0*s1*c2;

          rot(1,0) = -c1*s2;
          rot(1,1) = c0*c2 - s0*s1*s2;
          rot(1,2) = s0*c2 + c0*s1*s2;

          rot(2,0) = s1;
          rot(2,1) = -s0*c1;
          rot(2,2) = c0*c1;

          WorldPoint tmp0(DataType(0));
          WorldPoint tmp1(DataType(0));

          // Translate the point to the centre of rotation
          tmp0 = _midpoint - centre;
          // Rotate
          tmp1.set_vec_mat_mult(tmp0, rot);
          // Translate back
          _midpoint = centre + tmp1;

          // Rotate the rotation axis (duh!)
          tmp0 = _rot_axis;
          _rot_axisset_vec_mat_mult(tmp0, rot);
        }

        /**
         * \brief Projects a single world point
         *
         * \param[in,out] point
         * The world point to be projected
         *
         */
        void project_point(WorldPoint& point) const
        {
          // subtract tube midpoint
          point -= _midpoint;
          // project point onto axis
          WorldPoint axis_point = (_rot_scale * Tiny::dot(_rot_axis, point)) * _rot_axis;
          // compute difference of point and axis point
          WorldPoint diff_point = point - axis_point;
          // compute projected point
          point = (_midpoint + axis_point) + diff_point * (_radius / diff_point.norm_euclid());
        }

        /**
         * \brief Projects all mesh points identified by a meshpart
         *
         * \param[in,out] mesh
         * The mesh whose points will be projected
         *
         * \param[in] meshpart
         * The MeshPart identifying the point to be projected
         *
         */
        void project_meshpart(Mesh_& mesh, const MeshPart<Mesh_>& meshpart) const
        {
          auto& vtx = mesh.get_vertex_set();
          const auto& target_vtx = meshpart.template get_target_set<0>();

          for(Index i(0); i < meshpart.get_num_entities(0); ++i)
          {
            project_point(reinterpret_cast<WorldPoint&>(vtx[target_vtx[i]]));
          }
        }

        /// \copydoc ChartBase::dist()
        DataType compute_dist(const WorldPoint& point) const
        {
          WorldPoint projected(point);
          project_point(projected);
          return (projected - point).norm_euclid();
        }

        /** \copydoc ChartBase::write */
        virtual void write(std::ostream& os, const String& sindent) const override
        {
          os << sindent << "<Tube";
          os << " radius=\"" << this->_radius << "\"";
          os << " midpoint=\"" << this->_midpoint[0] << " " << this->_midpoint[1] << " " << this->_midpoint[2] << "\"";
          os << " axis=\"" << this->_rot_axis[0] << " " << this->_rot_axis[1] << " " << this->_rot_axis[2] << "\"";
          os << " />" << std::endl;
        }

      }; // class Tube<...>
    } // namespace Atlas
  } // namespace Geometry
} // namespace FEAT
#endif // KERNEL_GEOMETRY_ATLAS_TUBE_HPP
