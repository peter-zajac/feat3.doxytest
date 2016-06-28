#pragma once
#ifndef KERNEL_GEOMETRY_ATLAS_SPHERE_HPP
#define KERNEL_GEOMETRY_ATLAS_SPHERE_HPP 1

#include <kernel/geometry/atlas/chart.hpp>

namespace FEAT
{
  namespace Geometry
  {
    namespace Atlas
    {
      /// Sphere chart traits
      struct SphereTraits
      {
        /// we support explicit map
        static constexpr bool is_explicit = false;
        /// we support implicit projection
        static constexpr bool is_implicit = true;
        /// this is a 2D object
        static constexpr int world_dim = 3;
        /// we have 1D parameters
        static constexpr int param_dim = 1;
      };

      /**
       * \brief Sphere chart class template.
       *
       * This class represents a 3D sphere characterised by a midpoint and a radius.
       *
       * This chart implements only the implicit chart interface.
       *
       * \tparam Mesh_
       * The type of the mesh to be parameterised by this chart.
       *
       * \author Peter Zajac
       */
      template<typename Mesh_>
      class Sphere :
        public ChartCRTP<Sphere<Mesh_>, Mesh_, SphereTraits>
      {
      public:
        /// CRTP base class
        typedef ChartCRTP<Sphere<Mesh_>, Mesh_, SphereTraits> BaseClass;
        /// Floating point type
        typedef typename BaseClass::CoordType CoordType;
        /// Vector type for world points, aka image points
        typedef typename BaseClass::WorldPoint WorldPoint;
        /// Vector type for parameter points, aka domain points
        typedef typename BaseClass::ParamPoint ParamPoint;

      protected:
        /// the sphere's midpoint
        WorldPoint _midpoint;
        /// the sphere's radius
        CoordType _radius;

      public:
        /**
         * \brief Constructor
         *
         * \param[in] mid_x, mid_y, mid_z
         * The coordinates of the sphere midpoint.
         *
         * \param[in] radius
         * The radius of the sphere. Must be positive.
         */
        explicit Sphere(CoordType mid_x, CoordType mid_y, CoordType mid_z, CoordType radius) :
          _radius(radius)
        {
          XASSERTM(radius > CoordType(0), "invalid Sphere radius");
          _midpoint[0] = mid_x;
          _midpoint[1] = mid_y;
          _midpoint[2] = mid_z;
        }

        /** \copydoc ChartBase::get_type() */
        virtual String get_type() const override
        {
          return "sphere";
        }

        /// \copydoc ChartBase::move_by()
        virtual void move_by(const WorldPoint& translation) override
        {
            _midpoint += translation;
        }

        /// \copydoc ChartBase::rotate()
        virtual void rotate(const WorldPoint& centre, const WorldPoint& angles)
        {
          // This is the 3x3 matrix representing the turning by the angle angles(0)
          Tiny::Matrix<CoordType, 3, 3> rot(CoordType(0));

          CoordType c0 = Math::cos(angles(0));
          CoordType c1 = Math::cos(angles(1));
          CoordType c2 = Math::cos(angles(2));

          CoordType s0 = Math::sin(angles(0));
          CoordType s1 = Math::sin(angles(1));
          CoordType s2 = Math::sin(angles(2));

          rot(0,0) = c1*c2;
          rot(0,1) = c0*s2 + s0*s1*c2;
          rot(0,2) = s0*s2 - c0*s1*c2;

          rot(1,0) = -c1*s2;
          rot(1,1) = c0*c2 - s0*s1*s2;
          rot(1,2) = s0*c2 + c0*s1*s2;

          rot(2,0) = s1;
          rot(2,1) = -s0*c1;
          rot(2,2) = c0*c1;

          WorldPoint tmp0(CoordType(0));
          WorldPoint tmp1(CoordType(0));

          // Translate the point to the centre of rotation
          tmp0 = _midpoint - centre;
          // Rotate
          tmp1.set_vec_mat_mult(tmp0, rot);
          // Translate back
          _midpoint = centre + tmp1;
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
          WorldPoint grad_dist(point - _midpoint);
          CoordType distance(grad_dist.norm_euclid());

          if(distance < Math::eps<CoordType>())
          {
            grad_dist(0) = _radius;
            point += grad_dist;
          }
          else
          {
            point = _midpoint + (_radius / distance)*grad_dist;
          }
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
        CoordType compute_dist(const WorldPoint& point) const
        {
          return Math::abs(compute_signed_dist(point));
        }

        /// \copydoc ChartBase::dist()
        CoordType compute_dist(const WorldPoint& point, WorldPoint& grad_dist) const
        {
          WorldPoint projected(point);
          project_point(projected);

          CoordType my_dist(_radius - (point - _midpoint).norm_euclid());

          grad_dist = (point - projected);
          grad_dist.normalise();

          return Math::abs(my_dist);
        }

        /// \copydoc ChartBase::signed_dist()
        CoordType compute_signed_dist(const WorldPoint& point) const
        {
          return (point - _midpoint).norm_euclid() - _radius;
        }

        /// \copydoc ChartBase::signed_dist()
        CoordType compute_signed_dist(const WorldPoint& point, WorldPoint& grad_dist) const
        {
          WorldPoint projected(point);
          project_point(projected);

          CoordType my_dist(_radius - (point - _midpoint).norm_euclid());

          grad_dist = (point - projected);
          grad_dist.normalise();
          grad_dist *= Math::signum(my_dist);

          return my_dist;
        }

        /** \copydoc ChartBase::write */
        virtual void write(std::ostream& os, const String& sindent) const override
        {
          os << sindent << "<Sphere";
          os << " radius=\"" << this->_radius << "\"";
          os << " midpoint=\"" << this->_midpoint[0] << " " << this->_midpoint[1] << " " << this->_midpoint[2] << "\"";
          os << " />" << std::endl;
        }
      };

      template<typename Mesh_, typename ChartReturn_ = ChartBase<Mesh_>>
      class SphereChartParser :
        public Xml::MarkupParser
      {
      private:
        typedef Sphere<Mesh_> ChartType;
        typedef typename ChartType::CoordType CoordType;
        ChartReturn_*& _chart;

      public:
        explicit SphereChartParser(ChartReturn_*& chart) :
          _chart(chart)
        {
        }

        virtual bool attribs(std::map<String,bool>& attrs) const override
        {
          attrs.emplace("radius", true);
          attrs.emplace("midpoint", true);
          return true;
        }

        virtual void create(
          int iline,
          const String& sline,
          const String&,
          const std::map<String, String>& attrs,
          bool) override
        {
          CoordType radius = CoordType(0);
          CoordType mid_x = CoordType(0);
          CoordType mid_y = CoordType(0);
          CoordType mid_z = CoordType(0);

          // try to parse the radius
          if(!attrs.find("radius")->second.parse(radius))
            throw Xml::GrammarError(iline, sline, "Failed to parse sphere radius");
          if(radius < CoordType(1E-5))
            throw Xml::GrammarError(iline, sline, "Invalid sphere radius");

          // try to parse midpoind
          std::deque<String> mids;
          attrs.find("midpoint")->second.split_by_charset(mids);
          if(mids.size() != std::size_t(3))
            throw Xml::GrammarError(iline, sline, "Invalid sphere midpoint string");
          if(!mids.front().parse(mid_x) || !mids.at(1).parse(mid_y) || !mids.back().parse(mid_z))
            throw Xml::GrammarError(iline, sline, "'Failed to parse sphere midpoint");

          _chart = new ChartType(mid_x, mid_y, mid_z, radius);
        }

        virtual void close(int, const String&) override
        {
        }

        virtual bool content(int, const String&) override
        {
          return false;
        }

        virtual std::shared_ptr<Xml::MarkupParser> markup(int, const String&, const String&) override
        {
          return nullptr;
        }
      }; // class SphereChartParser<...>
    } // namespace Atlas
  } // namespace Geometry
} // namespace FEAT
#endif // KERNEL_GEOMETRY_ATLAS_SPHERE_HPP
