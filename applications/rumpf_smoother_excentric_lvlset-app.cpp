#include <iostream>
#include <kernel/archs.hpp>
#include <kernel/util/math.hpp>
#include <kernel/geometry/boundary_factory.hpp>
#include <kernel/geometry/conformal_factories.hpp>
#include <kernel/meshopt/rumpf_smoother_conc.hpp>
#include <kernel/meshopt/rumpf_smoother_lvlset.hpp>
#include <kernel/meshopt/rumpf_smoother_lvlset_q1hack.hpp>
#include <kernel/meshopt/rumpf_functionals/2d_p1_d1.hpp>
#include <kernel/meshopt/rumpf_functionals/2d_p1_d2.hpp>
#include <kernel/meshopt/rumpf_functionals/2d_q1_d1.hpp>
#include <kernel/meshopt/rumpf_functionals/2d_q1_d2.hpp>
#include <kernel/meshopt/rumpf_functionals/2d_q1hack.hpp>
#include <kernel/meshopt/rumpf_functionals/lvlset.hpp>
#include <kernel/meshopt/rumpf_functionals/conc_2d_p1_d1.hpp>
#include <kernel/meshopt/rumpf_functionals/conc_2d_p1_d2.hpp>
#include <kernel/meshopt/rumpf_functionals/conc_2d_q1_d1.hpp>
#include <kernel/meshopt/rumpf_functionals/conc_2d_q1_d2.hpp>
#include <kernel/geometry/export_vtk.hpp>
#include <kernel/space/lagrange1/element.hpp>
#include <kernel/assembly/interpolator.hpp>
#include <kernel/assembly/common_functions.hpp>
#include <kernel/assembly/discrete_projector.hpp>

using namespace FEAST;

namespace FEAST
{
  namespace Assembly
  {
    namespace Common
    {
      /**
       * @brief Class Template for the gradient of DistanceFunctionSD
       *
       * \tparam ImgPointType_
       * Type for the point in the DistanceFunction
       *
       * \tparam component
       * Component of the gradient
       *
       **/
      template<typename ImgPointType_, int component>
      class DistanceFunctionSD_grad :
        public AnalyticFunction
      {
        public:
          /// Datatype from the point
          typedef typename ImgPointType_::DataType DataType;
          /** \copydoc AnalyticFunction::FunctionCapabilities */
          enum FunctionCapabilities
          {
            can_value = 1,
            can_grad = 0,
            can_hess = 0
          };

          /** \copydoc AnalyticFunction::ConfigTraits */
          template<typename Config_>
          struct ConfigTraits
          {
            /**
             * \brief Trafo configuration tag class
             *
             * \see Trafo::ConfigBase
             */
            struct TrafoConfig :
              public Trafo::ConfigBase
            {
              enum
              {
                need_img_point = 1
              };
            };
          };

          /** \copydoc AnalyticFunction::Evaluator */
          template<typename EvalTraits_>
          class Evaluator :
            public AnalyticFunction::Evaluator<EvalTraits_>
        {
          public:
            /// trafo evaluator data
            typedef typename EvalTraits_::TrafoEvaluator TrafoEvaluator;
            /// trafo data type
            typedef typename EvalTraits_::TrafoData TrafoData;
            /// coefficient data type
            typedef typename EvalTraits_::DataType DataType;
            /// value type
            typedef typename EvalTraits_::ValueType ValueType;
            /// gradient type
            typedef typename EvalTraits_::GradientType GradientType;
            /// hessian type
            typedef typename EvalTraits_::HessianType HessianType;
            /// type for the points the analytic function is evaluated at
            typedef typename EvalTraits_::ImagePointType ImgPointType;

          private:
            /// Function to evaluate
            const DistanceFunctionSD_grad& _function;

          public:
            /// Constructor
            explicit Evaluator(const DistanceFunctionSD_grad& function) :
              _function(function)
              {
              }

            ValueType value(const TrafoData& tau) const
            {
              ImgPointType tmp (tau.img_point - _function._point);
              DataType norm = tmp.norm_euclid();
              if(norm <= Math::eps<DataType>())
              {
                return DataType(0);
              }
              else
              {
                return _function._b*( tau.img_point[component] - _function._point(component))/norm;
              }
            }
        }; // class DistanceFunctionSD::Evaluator<...>

        private:
          /// The point to which the distance to is calculated
          ImgPointType_& _point;
          /// Displacement of the function
          DataType _a;
          /// Scaling factor
          DataType _b;

        public:
          /// Constructor
          explicit DistanceFunctionSD_grad(ImgPointType_& x0_, const DataType a_, const DataType b_) :
            _point(x0_),
            _a(a_),
            _b(b_)
            {
            }
          /// Sets _point to x0_
          void set_point(const ImgPointType_ x0_)
          {
            _point = x0_;
          }

      }; // class DistanceFunctionSD_grad

      template<int component, int derivative_component, typename ImgPointType_>
      class PlaneDistanceFunctionSD_grad :
        public AnalyticFunction
      {
        public:
          /// Datatype from the point
          typedef typename ImgPointType_::DataType DataType;
          /** \copydoc AnalyticFunction::FunctionCapabilities */
          enum FunctionCapabilities
          {
            can_value = 1,
            can_grad = 0,
            can_hess = 0
          };

          /** \copydoc AnalyticFunction::ConfigTraits */
          template<typename Config_>
          struct ConfigTraits
          {
            /**
             * \brief Trafo configuration tag class
             *
             * \see Trafo::ConfigBase
             */
            struct TrafoConfig :
              public Trafo::ConfigBase
            {
              enum
              {
                need_img_point = 1
              };
            };
          };

          /** \copydoc AnalyticFunction::Evaluator */
          template<typename EvalTraits_>
          class Evaluator :
            public AnalyticFunction::Evaluator<EvalTraits_>
        {
          public:
            /// trafo evaluator data
            typedef typename EvalTraits_::TrafoEvaluator TrafoEvaluator;
            /// trafo data type
            typedef typename EvalTraits_::TrafoData TrafoData;
            /// coefficient data type
            typedef typename EvalTraits_::DataType DataType;
            /// value type
            typedef typename EvalTraits_::ValueType ValueType;
            /// gradient type
            typedef typename EvalTraits_::GradientType GradientType;
            /// hessian type
            typedef typename EvalTraits_::HessianType HessianType;
            /// type for the points the analytic function is evaluated at
            typedef typename EvalTraits_::ImagePointType ImgPointType;

          private:
            /// Function to evaluate
            const PlaneDistanceFunctionSD_grad& _function;

          public:
            /// Constructor
            explicit Evaluator(const PlaneDistanceFunctionSD_grad& function) :
              _function(function)
              {
              }

            ValueType value(const TrafoData& DOXY(tau)) const
            {
              return _function._b*ValueType(component == derivative_component);
            }
        }; // class PlaneDistanceFunctionSD::Evaluator<...>

        private:
          /// The point to which the distance to is calculated
          ImgPointType_& _point;
          /// Scaling factor
          DataType _b;

        public:
          /// Constructor
          explicit PlaneDistanceFunctionSD_grad(ImgPointType_& x0_, const DataType b_) :
            _point(x0_),
            _b(b_)
            {
            }

      }; // class DistanceFunctionSD_grad

      /**
       * \brief Function representing the gradient of the minimum of two analytic functions
       *
       * This is needed i.e. if there are several objects implicitly defined by their zero level sets. The class
       * in general supports function values, gradients and hessians for all dimensions, depending on the two analytic
       * functions supporting these.
       *
       * \Warning As min is non differentiable in general, Bad Things(TM) may happen when computing the gradient
       * and/or hessian where the function values are nearly identical.
       *
       * \tparam AnalyticFunctionType1
       * Type for the first AnalyticFunction
       *
       * \tparam AnalyticFunctionType2
       * Type for the second AnalyticFunction
       *
       * \author Jordi Paul
       */
      template<typename AnalyticFunctionType1, typename AnalyticFunctionType2, typename AnalyticFunctionType1Der, typename AnalyticFunctionType2Der>
      class MinOfTwoFunctionsDer :
        public AnalyticFunction
      {
        private:
          /// The first AnalyticFunction
          const AnalyticFunctionType1& _f1;
          /// The second AnalyticFunction
          const AnalyticFunctionType2& _f2;
          /// The first AnalyticFunctions derivative
          const AnalyticFunctionType1Der& _f1_der;
          /// The second AnalyticFunctions derivative
          const AnalyticFunctionType2Der& _f2_der;

        public:
          /// Can compute function values if both AnalyticFunctions can do that
          static constexpr bool can_value = (AnalyticFunctionType1Der::can_value && AnalyticFunctionType2Der::can_value);
          /// Can compute the function gradient if both AnalyticFunctions can do that
          static constexpr bool can_grad = (AnalyticFunctionType1Der::can_grad && AnalyticFunctionType2Der::can_grad);
          /// Can compute the function hessian if both AnalyticFunctions can do that
          static constexpr bool can_hess = (AnalyticFunctionType1Der::can_hess && AnalyticFunctionType2Der::can_hess);

          /** \copydoc AnalyticFunction::ConfigTraits */
          template<typename Config_>
          struct ConfigTraits
          {
            /// TrafoConfig of the first AnalyticFunction
            typedef typename AnalyticFunctionType1Der::template ConfigTraits<Config_>::TrafoConfig TrafoConfig1;
            /// TrafoConfig of the second AnalyticFunction
            typedef typename AnalyticFunctionType2Der::template ConfigTraits<Config_>::TrafoConfig TrafoConfig2;

            /**
             * \brief Trafo configuration tag class
             *
             * \see Trafo::ConfigBase
             *
             * A quantity (i.e. the Jacobian matrix) is needed if any of the functions needs it.
             */
            typedef Trafo::ConfigOr<TrafoConfig1, TrafoConfig2> TrafoConfig;
          };

          /** \copydoc AnalyticFunction::Evaluator */
          template<typename EvalTraits_>
          class Evaluator :
            public AnalyticFunction::Evaluator<EvalTraits_>
        {
          public:
            /// trafo evaluator data
            typedef typename EvalTraits_::TrafoEvaluator TrafoEvaluator;
            /// trafo data type
            typedef typename EvalTraits_::TrafoData TrafoData;
            /// coefficient data type
            typedef typename EvalTraits_::DataType DataType;
            /// value type
            typedef typename EvalTraits_::ValueType ValueType;
            /// gradient type
            typedef typename EvalTraits_::GradientType GradientType;
            /// hessian type
            typedef typename EvalTraits_::HessianType HessianType;
            /// type for the points the analytic function is evaluated at
            typedef typename EvalTraits_::ImagePointType ImgPointType;

          private:
            /// Function to evaluate
            const MinOfTwoFunctionsDer& _function;
            typename AnalyticFunctionType1::template Evaluator<EvalTraits_> _f1_eval;
            typename AnalyticFunctionType2::template Evaluator<EvalTraits_> _f2_eval;
            typename AnalyticFunctionType1Der::template Evaluator<EvalTraits_> _f1_der_eval;
            typename AnalyticFunctionType2Der::template Evaluator<EvalTraits_> _f2_der_eval;

          public:
            /// Constructor
            explicit Evaluator(const MinOfTwoFunctionsDer& function) :
              _function(function),
              _f1_eval(function._f1),
              _f2_eval(function._f2),
              _f1_der_eval(function._f1_der),
              _f2_der_eval(function._f2_der)
              {
              }

            ValueType value(const TrafoData& tau) const
            {
              ValueType fval1 = _f1_eval.value(tau);
              ValueType fval2 = _f2_eval.value(tau);
              if(Math::abs(fval1-fval2) < Math::eps<DataType>()) return ValueType(0);
              return fval1 < fval2 ? _f1_der_eval.value(tau) : _f2_der_eval.value(tau);
            }

            GradientType gradient(const TrafoData& tau) const
            {
              ValueType fval1 = _f1_eval.value(tau);
              ValueType fval2 = _f2_eval.value(tau);
              if(Math::abs(fval1-fval2) < Math::eps<DataType>()) return GradientType(0);
              return fval1 < fval2 ? _f1_der_eval.gradient(tau) : _f2_der_eval.gradient(tau);
            }

            HessianType hessian(const TrafoData& tau) const
            {
              ValueType fval1 = _f1_eval.value(tau);
              ValueType fval2 = _f2_eval.value(tau);
              if(Math::abs(fval1 - fval2) < Math::eps<DataType>()) return HessianType(0);
              return fval1 < fval2 ? _f1_der_eval.hessian(tau) : _f2_der_eval.hessian(tau);
            }

        }; // class MinOfTwoFunctions::Evaluator<...>

        public:
          /// Constructor
          explicit MinOfTwoFunctionsDer(const AnalyticFunctionType1& f1_, const AnalyticFunctionType2& f2_,
          const AnalyticFunctionType1Der& f1_der_, const AnalyticFunctionType2Der& f2_der_) :
            _f1(f1_),
            _f2(f2_),
            _f1_der(f1_der_),
            _f2_der(f2_der_)
            {
            }

      }; // class MinOfTwoFunctionsDer

    } // namespace Common
  } // namespace Assembly
} // namespace FEAST

template<typename PointType, typename DataType>
void centre_point_outer(PointType& my_point, DataType time)
{
  my_point.v[0] = DataType(0.5) - DataType(0.125)*Math::cos(DataType(2)*Math::pi<DataType>()*DataType(time));
  my_point.v[1] = DataType(0.5) - DataType(0.125)*Math::sin(DataType(2)*Math::pi<DataType>()*DataType(time));
}

template<typename PointType, typename DataType>
void centre_point_inner(PointType& my_point, DataType time)
{
  my_point.v[0] = DataType(0.5) - DataType(0.1875)*Math::cos(DataType(2)*Math::pi<DataType>()*DataType(time));
  my_point.v[1] = DataType(0.5) - DataType(0.1875)*Math::sin(DataType(2)*Math::pi<DataType>()*DataType(time));
}

/**
 * \brief Wrapper struct as functions do not seem to agree with template template parameters
 **/
template
<
  typename DT_,
  typename ShapeType_,
  template<typename ... > class RumpfSmootherType_,
  template<typename, typename> class FunctionalType_,
  template<typename, typename> class LevelsetFunctionalType_
> struct LevelsetApp
{
  /**
   * @brief Runs mesh smoother stuff
   *
   **/
  static void run(Index level, DT_ deltat)
  {
    /// Precision for meshes etc, everything else uses the same data type
    typedef DT_ DataType;
    /// Rumpf Smoothers are implemented for Mem::Main only
    typedef Mem::Main MemType;
    /// So we use Index
    typedef Index IndexType;
    /// Shape of the mesh cells
    typedef ShapeType_ ShapeType;
    /// The full mesh type
    typedef Geometry::ConformalMesh<ShapeType, ShapeType::dimension,ShapeType::dimension, DataType> MeshType;
    /// The transformation
    typedef Trafo::Standard::Mapping<MeshType> TrafoType;
    /// The Rumpf functional variant
    typedef FunctionalType_<DataType, ShapeType> FunctionalType;
    /// The levelset part of the Rumpf functional
    typedef LevelsetFunctionalType_<DataType, ShapeType> LevelsetFunctionalType;

    /// Type for points in the domain
    typedef Tiny::Vector<DataType,2,2> ImgPointType;

    typedef Assembly::Common::DistanceFunctionSD<ImgPointType> AnalyticFunctionType1;
    typedef Assembly::Common::DistanceFunctionSD_grad<ImgPointType, 0> AnalyticFunction1Grad0Type;
    typedef Assembly::Common::DistanceFunctionSD_grad<ImgPointType, 1> AnalyticFunction1Grad1Type;

    typedef Assembly::Common::MinOfTwoFunctions<AnalyticFunctionType1, AnalyticFunctionType1> AnalyticFunctionType;
    typedef Assembly::Common::MinOfTwoFunctionsDer<AnalyticFunctionType1, AnalyticFunctionType1, AnalyticFunction1Grad0Type, AnalyticFunction1Grad0Type> AnalyticFunctionGrad0Type;
    typedef Assembly::Common::MinOfTwoFunctionsDer<AnalyticFunctionType1, AnalyticFunctionType1, AnalyticFunction1Grad1Type, AnalyticFunction1Grad1Type> AnalyticFunctionGrad1Type;

    // Reference point for the rotation
    ImgPointType x0(DataType(0));

    centre_point_outer(x0,DataType(0));
    DataType scaling_outer(-1);
    DataType radius_outer(0.35);

    // Analytic function for the distance to the outer circle's boundary
    AnalyticFunctionType1 outer(x0, radius_outer, scaling_outer);
    AnalyticFunction1Grad0Type outer_grad0(x0, radius_outer, scaling_outer);
    AnalyticFunction1Grad1Type outer_grad1(x0, radius_outer, scaling_outer);

    DataType scaling_inner(-scaling_outer);
    DataType radius_inner(-scaling_inner*0.275);
    centre_point_inner(x0,DataType(0));

    // Analytic function for the distance to the inner circle's boundary
    AnalyticFunctionType1 inner(x0, radius_inner, scaling_inner);
    AnalyticFunction1Grad0Type inner_grad0(x0, radius_inner, scaling_inner);
    AnalyticFunction1Grad1Type inner_grad1(x0, radius_inner, scaling_inner);

    AnalyticFunctionType analytic_lvlset(inner, outer);
    AnalyticFunctionGrad0Type analytic_lvlset_grad0(inner, outer, inner_grad0, outer_grad0);
    AnalyticFunctionGrad1Type analytic_lvlset_grad1(inner, outer, inner_grad1, outer_grad1);

    // Now we can define the whole smoother
    typedef RumpfSmootherType_
    <
      AnalyticFunctionType,
      AnalyticFunctionGrad0Type,
      AnalyticFunctionGrad1Type,
      TrafoType,
      FunctionalType,
      LevelsetFunctionalType
    > RumpfSmootherType;

    // Factory to build a unit cube mesh
    Geometry::RefineFactory<MeshType,Geometry::UnitCubeFactory> mesh_factory(level);
    // Create the mesh
    MeshType mesh(mesh_factory);
    // Create the transformation
    TrafoType trafo(mesh);

    // Parameters for the functional
    DataType fac_norm = DataType(1e0),fac_det = DataType(1e0),fac_cof = DataType(0), fac_reg(DataType(1e-8));
    // Create the functional
    FunctionalType my_functional(fac_norm, fac_det, fac_cof, fac_reg);
    // Parameters for the Rumpf smoother
    bool align_to_lvlset(false);
    bool r_adaptivity(true);
    DataType r_adapt_reg = DataType(1e-2), r_adapt_pow = DataType(0.5);
    // Create levelset part of the functional
    LevelsetFunctionalType my_levelset_functional;

    // The smoother in all its template glory
    RumpfSmootherType rumpflpumpfl(trafo, my_functional, my_levelset_functional, align_to_lvlset, r_adaptivity,
    analytic_lvlset, analytic_lvlset_grad0, analytic_lvlset_grad1);
    // Set the r-adaptivity parameters
    rumpflpumpfl.set_r_adapt_params(r_adapt_reg,r_adapt_pow);

    // Print lotsa information
    std::cout << __func__ << " at refinement level " << level << std::endl;
    rumpflpumpfl.print();

    rumpflpumpfl.init();

    // Arrays for saving the contributions of the different Rumpf functional parts
    DataType* func_norm(new DataType[mesh.get_num_entities(MeshType::shape_dim)]);
    DataType* func_det(new DataType[mesh.get_num_entities(MeshType::shape_dim)]);
    DataType* func_rec_det(new DataType[mesh.get_num_entities(MeshType::shape_dim)]);
    DataType* func_lvlset(new DataType[mesh.get_num_entities(MeshType::shape_dim)]);

    // Evaluates the levelset function and its gradient
    rumpflpumpfl.prepare();
    // Compute initial functional value
    DataType fval(0);
    fval = rumpflpumpfl.compute_functional(func_norm, func_det, func_rec_det, func_lvlset);
    std::cout << "fval pre optimisation = " << scientify(fval) << " cell size quality indicator: " << scientify(rumpflpumpfl.cell_size_quality()) << std::endl;
    // Compute initial functional gradient
    rumpflpumpfl.compute_gradient();

    // Filename for vtk files
    std::string filename;

    // Write initial state to file
    Geometry::ExportVTK<MeshType> writer_pre_initial(mesh);
    writer_pre_initial.add_scalar_cell("norm", func_norm);
    writer_pre_initial.add_scalar_cell("det", func_det);
    writer_pre_initial.add_scalar_cell("rec_det", func_rec_det);
    writer_pre_initial.add_scalar_cell("lambda", rumpflpumpfl._lambda.elements() );
    writer_pre_initial.add_scalar_vertex("levelset", rumpflpumpfl._lvlset_vtx_vec.elements());
    writer_pre_initial.add_field_cell_blocked_vector("h", rumpflpumpfl._h);
    writer_pre_initial.add_field_vertex_blocked_vector("grad", rumpflpumpfl._grad);
    writer_pre_initial.add_field_vertex("lvlset_grad", rumpflpumpfl._lvlset_grad_vtx_vec[0].elements(), rumpflpumpfl._lvlset_grad_vtx_vec[1].elements());
    writer_pre_initial.add_scalar_cell("levelset_constraint", func_lvlset );
    writer_pre_initial.write("pre_initial");

    // Optimise the mesh
    rumpflpumpfl.optimise();

    fval = rumpflpumpfl.compute_functional(func_norm, func_det, func_rec_det, func_lvlset);
    std::cout << "fval post optimisation = " << scientify(fval) << " cell size quality indicator: " << scientify(rumpflpumpfl.cell_size_quality()) << std::endl;

    // Call prepare() again because the mesh changed due to the optimisation and it was not called again after the
    // last iteration
    rumpflpumpfl.prepare();
    rumpflpumpfl.compute_gradient();

    // Write optimised initial mesh
    Geometry::ExportVTK<MeshType> writer_post_initial(mesh);
    writer_post_initial.add_scalar_cell("norm", func_norm);
    writer_post_initial.add_scalar_cell("det", func_det);
    writer_post_initial.add_scalar_cell("rec_det", func_rec_det);
    writer_post_initial.add_scalar_cell("lambda", rumpflpumpfl._lambda.elements() );
    writer_post_initial.add_field_cell_blocked_vector("h", rumpflpumpfl._h );
    writer_post_initial.add_field_vertex_blocked_vector("grad", rumpflpumpfl._grad);
    writer_post_initial.add_scalar_vertex("levelset", rumpflpumpfl._lvlset_vtx_vec.elements());
    writer_post_initial.add_field_vertex("lvlset_grad", rumpflpumpfl._lvlset_grad_vtx_vec[0].elements(), rumpflpumpfl._lvlset_grad_vtx_vec[1].elements());
    writer_post_initial.add_scalar_cell("levelset_constraint", func_lvlset );
    writer_post_initial.write("post_initial");

    DataType time(0);
    Index n(0);

    // Old mesh coordinates for computing the mesh velocity
    LAFEM::DenseVectorBlocked<MemType, DataType, IndexType, MeshType::world_dim> coords_old(mesh.get_num_entities(0),DataType(0));
    LAFEM::DenseVectorBlocked<MemType, DataType, IndexType, MeshType::world_dim> mesh_velocity(mesh.get_num_entities(0), DataType(0));

    while(time < DataType(1))
    {
      std::cout << "timestep " << n << std::endl;
      time+= deltat;

      // Save old vertex coordinates
      coords_old.clone(rumpflpumpfl._coords);

      // Update levelset function
      centre_point_outer(x0,time);
      outer.set_point(x0);
      outer_grad0.set_point(x0);
      outer_grad1.set_point(x0);

      // Update reference point
      centre_point_inner(x0,time);
      inner.set_point(x0);
      inner_grad0.set_point(x0);
      inner_grad1.set_point(x0);

      // Compute functional value and gradient
      fval = rumpflpumpfl.compute_functional(func_norm, func_det, func_rec_det, func_lvlset);
      rumpflpumpfl.compute_gradient();
      std::cout << "fval pre optimisation = " << scientify(fval) << " cell size quality indicator: " <<
        scientify(rumpflpumpfl.cell_size_quality()) << std::endl;

      // Write pre-optimisation mesh
      filename = "pre_" + stringify(n);
      Geometry::ExportVTK<MeshType> writer_pre(mesh);

      writer_pre.add_scalar_cell("norm", func_norm);
      writer_pre.add_scalar_cell("det", func_det);
      writer_pre.add_scalar_cell("rec_det", func_rec_det);
      writer_pre.add_scalar_cell("lambda", rumpflpumpfl._lambda.elements() );
      writer_pre.add_field_cell_blocked_vector("h", rumpflpumpfl._h);
      writer_pre.add_field_vertex_blocked_vector("grad", rumpflpumpfl._grad);
      writer_pre.add_scalar_vertex("levelset", rumpflpumpfl._lvlset_vtx_vec.elements());
      writer_pre.add_field_vertex("lvlset_grad", rumpflpumpfl._lvlset_grad_vtx_vec[0].elements(),
      rumpflpumpfl._lvlset_grad_vtx_vec[1].elements());
      writer_pre.add_scalar_cell("levelset_constraint", func_lvlset );
      writer_pre.write(filename);

      // Optimise the mesh
      rumpflpumpfl.optimise();

      // Compute grid velocity
      DataType max_mesh_velocity(-1e10);
      DataType ideltat = DataType(1)/deltat;
      for(Index i(0); i < mesh.get_num_entities(0); ++i)
      {
        mesh_velocity(i, ideltat*(rumpflpumpfl._coords(i) - coords_old(i)));

        DataType my_mesh_velocity(mesh_velocity(i).norm_euclid());

        if(my_mesh_velocity > max_mesh_velocity)
          max_mesh_velocity = my_mesh_velocity;
      }
      std::cout << "max mesh velocity = " << scientify(max_mesh_velocity) << std::endl;

      // Write post-optimisation mesh
      fval = rumpflpumpfl.compute_functional(func_norm,func_det,func_rec_det,func_lvlset);
      rumpflpumpfl.compute_gradient();
      std::cout << "fval post optimisation = " << scientify(fval) << " cell size quality indicator: "
      << scientify(rumpflpumpfl.cell_size_quality()) << std::endl;

      filename = "post_" + stringify(n);
      Geometry::ExportVTK<MeshType> writer_post(mesh);
      writer_post.add_scalar_cell("norm", func_norm);
      writer_post.add_scalar_cell("det", func_det);
      writer_post.add_scalar_cell("rec_det", func_rec_det);
      writer_post.add_scalar_cell("lambda", rumpflpumpfl._lambda.elements() );
      writer_post.add_field_cell_blocked_vector("h", rumpflpumpfl._h );
      writer_post.add_field_vertex_blocked_vector("grad", rumpflpumpfl._grad);
      writer_post.add_scalar_vertex("levelset", rumpflpumpfl._lvlset_vtx_vec.elements());
      writer_post.add_field_vertex("lvlset_grad", rumpflpumpfl._lvlset_grad_vtx_vec[0].elements(),
      rumpflpumpfl._lvlset_grad_vtx_vec[1].elements());
      writer_post.add_scalar_cell("levelset_constraint", func_lvlset );
      writer_post.add_field_vertex_blocked_vector("mesh_velocity", mesh_velocity);
      writer_post.write(filename);

      n++;

    }

    delete func_norm;
    delete func_det;
    delete func_rec_det;
    delete func_lvlset;

  }

}; // struct LevelsetApp

template<typename A, typename B, typename C, typename D, typename E, typename F>
using MySmoother = Meshopt::RumpfSmootherLevelsetConcAnalytic<A, B, C, D, E, F>;

//template<typename A, typename B, typename C, typename D, typename E, typename F>
//using MySmootherQ1Hack = Meshopt::RumpfSmootherLevelsetAnalyticQ1Hack<A, B, C, D, E, F>;

template<typename A, typename B>
using MyFunctional= Meshopt::RumpfFunctionalConc<A, B>;

//template<typename A, typename B>
//using MyFunctionalQ1Hack = Meshopt::RumpfFunctionalQ1Hack<A, B, Geometry::RumpfFunctional>;

int main()
{

  typedef double DataType;

  // Refinement level
  Index level(4);
  // Timestep size
  DataType deltat(DataType(0.025));

  LevelsetApp<DataType, Shape::Simplex<2>, MySmoother, MyFunctional, Meshopt::RumpfFunctionalLevelset>::run(level, deltat);
  return 0;
}
