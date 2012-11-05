#pragma once
#ifndef KERNEL_ASSEMBLY_ASM_TRAITS_HPP
#define KERNEL_ASSEMBLY_ASM_TRAITS_HPP 1

// includes, FEAST
#include <kernel/assembly/base.hpp>
#include <kernel/assembly/local_system_data.hpp>
#include <kernel/cubature/dynamic_factory.hpp>

namespace FEAST
{
  namespace Assembly
  {
    /// \cond internal
    namespace Intern
    {
      template<typename Space_, typename DataType_>
      struct EvalPolicyFetcher
      {
        typedef typename Space_::TrafoType TrafoType;
        typedef typename TrafoType::ShapeType ShapeType;
        typedef typename TrafoType::template Evaluator<ShapeType, DataType_>::Type TrafoEvalType;
        typedef typename TrafoEvalType::EvalPolicy EvalPolicy;
      };

      template<typename TrafoEvaluator_>
      class CubatureTraits
      {
      public:
        /// evaluation policy
        typedef typename TrafoEvaluator_::EvalPolicy EvalPolicy;

        /// data type
        typedef typename EvalPolicy::DataType DataType;

        /// shape type
        typedef typename EvalPolicy::ShapeType ShapeType;

        /// domain coordinate type
        typedef typename EvalPolicy::DomainCoordType DomainCoordType;

        /// domain point type
        typedef typename EvalPolicy::DomainPointType DomainPointType;

        /// cubature rule type
        typedef Cubature::Rule<ShapeType, DataType, DomainCoordType, DomainPointType> RuleType;

        /// cubature dynamic factory type
        typedef typename Cubature::DynamicFactorySelect<RuleType>::Type DynamicFactoryType;
      };
    }
    /// \endcond

    /**
      * \brief Common single-space assembly traits class template
      *
      * This class template takes care of defining the necessary classes for assembly with one single
      * finite element space.
      *
      * This class can e.g. be used as a base class for
      * - assembly of a linear functional, e.g. a right-hand-side vector
      * - assembly of a bilinear operator with identical test- and trial-spaces
      * - assembly of a trilinear operator with identical test-, trial- and multiplier-spaces
      * - post-processing of a primal vector, e.g. L2- or H1-error calculation
      *
      * \tparam DataType_
      * The data type that is used to be for the assembly.
      *
      * \tparam Space_
      * The finite element space that is to be used as a (test- and trial-) space.
      *
      * \tparam TrafoConfig_
      * A trafo config class defining additional trafo requirements, e.g. from a (bi)linear functor.
      *
      * \tparam SpaceConfig_
      * A space config class defining additional space requirements, e.g. from a (bi)linear functor.
      *
      * \author Peter Zajac
      */
    template<
      typename DataType_,
      typename Space_,
      typename TrafoConfig_ = Trafo::ConfigBase,
      typename SpaceConfig_ = Space::ConfigBase>
    class AsmTraits1 :
      public Intern::EvalPolicyFetcher<Space_, DataType_>::EvalPolicy
    {
    public:
      /// data type
      typedef DataType_ DataType;
      /// space type
      typedef Space_ SpaceType;
      /// test-space type
      typedef SpaceType TestSpaceType;
      /// trial-space type
      typedef SpaceType TrialSpaceType;
      /// multiplier space type
      typedef SpaceType MultSpaceType;

      /// trafo type
      typedef typename SpaceType::TrafoType TrafoType;
      /// shape type
      typedef typename TrafoType::ShapeType ShapeType;
      /// mesh type
      typedef typename TrafoType::MeshType MeshType;

      // define test- and trial-space configs
      typedef SpaceConfig_ SpaceConfig;
      typedef SpaceConfig TestConfig;
      typedef SpaceConfig TrialConfig;
      typedef SpaceConfig MultConfig;

      // now fetch the trafo config from the space
      typedef typename SpaceType::template TrafoConfig<SpaceConfig> SpaceTrafoConfig;
      typedef SpaceTrafoConfig TestTrafoConfig;
      typedef SpaceTrafoConfig TrialTrafoConfig;
      typedef SpaceTrafoConfig MultTrafoConfig;

      /// assembly trafo config: derive from user-defined trafo config
      struct AsmTrafoConfig :
        public TrafoConfig_
      {
        /// dummy enumeration
        enum
        {
          /// we need jacobian determinants for integration
          need_jac_det = 1
        };
      };

      /// trafo config: combine space and assembly trafo configs
      typedef Trafo::ConfigOr<AsmTrafoConfig, SpaceTrafoConfig> TrafoConfig;

      /// trafo evaluator types
      typedef typename TrafoType::template Evaluator<ShapeType, DataType>::Type TrafoEvaluator;

      /// space evaluator types
      typedef typename SpaceType::template Evaluator<TrafoEvaluator>::Type SpaceEvaluator;
      typedef SpaceEvaluator TestEvaluator;
      typedef SpaceEvaluator TrialEvaluator;
      typedef SpaceEvaluator MultEvaluator;

      /// trafo evaluation data type
      typedef Trafo::EvalData<TrafoEvaluator, TrafoConfig> TrafoEvalData;
      typedef TrafoEvalData TrafoData;

      /// space evaluation data types
      typedef Space::EvalData<SpaceEvaluator, SpaceConfig> SpaceEvalData;
      typedef SpaceEvalData TestEvalData;
      typedef SpaceEvalData TrialEvalData;
      typedef SpaceEvalData MultEvalData;

      /// basis function data types
      typedef Space::FuncData<SpaceEvaluator, SpaceConfig> SpaceFuncData;
      typedef SpaceFuncData FuncData;
      typedef SpaceFuncData TestFuncData;
      typedef SpaceFuncData TrialFuncData;
      typedef SpaceFuncData MultFuncData;

      /// dof-mapping types
      typedef typename SpaceType::DofMappingType DofMapping;
      typedef DofMapping TestDofMapping;
      typedef DofMapping TrialDofMapping;
      typedef DofMapping MultDofMapping;

      /// local vector type
      typedef Tiny::Vector<DataType, SpaceEvaluator::max_local_dofs> LocalVectorType;
      typedef LocalVectorType LocalTestVectorType;
      typedef LocalVectorType LocalTrialVectorType;
      typedef LocalVectorType LocalMultVectorType;

      /// local vector data type
      typedef LocalVectorData<LocalVectorType, DofMapping> LocalVectorDataType;

      /// local matrix type
      typedef Tiny::Matrix<DataType, SpaceEvaluator::max_local_dofs, SpaceEvaluator::max_local_dofs> LocalMatrixType;

      /// local matrix data type
      typedef LocalMatrixData<LocalMatrixType, DofMapping, DofMapping> LocalMatrixDataType;

      /// cubature rule type
      typedef typename Intern::CubatureTraits<TrafoEvaluator>::RuleType CubatureRuleType;

      /// cubature factory type
      typedef typename Intern::CubatureTraits<TrafoEvaluator>::DynamicFactoryType CubatureDynamicFactoryType;

    }; // class AsmTraits1

    /**
      * \brief Common test-/trial-space assembly traits class template
      *
      * This class template takes care of defining the necessary classes for assembly with a combination of different
      * test- and trial-spaces using the same transformation.
      *
      * This class can e.g. be used as a base class for
      * - assembly of a bilinear operator with different test- and trial-spaces
      * - assembly of a trilinear operator with different test- and trial-spaces but identical trial- and
      *   multiplier-spaces
      *
      * \tparam DataType_
      * The data type that is used to be for the assembly.
      *
      * \tparam TestSpace_
      * The finite element space that is to be used as the test-space.
      *
      * \tparam TrialSpace_
      * The finite element space that is to be used as the trial-space. Must be defined on the same  trafo object as
      * \p TestSpace_.
      *
      * \tparam TrafoConfig_
      * A trafo config class defining additional trafo requirements, e.g. from a (bi)linear functor.
      *
      * \tparam TestConfig_, TrialConfig_
      * Two space config classes defining additional test- and trial-space requirements, e.g. from a (bi)linear functor.
      *
      * \author Peter Zajac
      */
    template<
      typename DataType_,
      typename TestSpace_,
      typename TrialSpace,
      typename TrafoConfig_ = Trafo::ConfigBase,
      typename TestConfig_ = Space::ConfigBase,
      typename TrialConfig_ = Space::ConfigBase>
    class AsmTraits2 :
      public Intern::EvalPolicyFetcher<TestSpace_, DataType_>::EvalPolicy
    {
    public:
      /// data type
      typedef DataType_ DataType;
      /// test-space type
      typedef TestSpace_ TestSpaceType;
      /// trial-space type
      typedef TrialSpace TrialSpaceType;
      /// mult-space type
      typedef TrialSpaceType MultSpaceType;

    protected:
      /// trafo type
      typedef typename TestSpaceType::TrafoType TrafoType;
      /// shape type
      typedef typename TrafoType::ShapeType ShapeType;
      /// mesh type
      typedef typename TrafoType::MeshType MeshType;

      // define test- and trial-space configs
      typedef TestConfig_ TestConfig;
      typedef TrialConfig_ TrialConfig;
      typedef TrialConfig MultConfig;

      // now fetch the trafo configs from the spaces
      typedef typename TestSpaceType::template TrafoConfig<TestConfig> TestTrafoConfig;
      typedef typename TrialSpaceType::template TrafoConfig<TrialConfig> TrialTrafoConfig;
      typedef TrialTrafoConfig MultTrafoConfig;

      // combine the space trafo configurations
      typedef Trafo::ConfigOr<TestTrafoConfig, TrialTrafoConfig> SpaceTrafoConfig;

      /// assembly trafo config: derive from  user-defined trafo config
      struct AsmTrafoConfig :
        public TrafoConfig_
      {
        /// dummy enumeration
        enum
        {
          /// we need jacobian determinants for integration
          need_jac_det = 1
        };
      };

      /// trafo config: combine space and assembly trafo configs
      typedef Trafo::ConfigOr<AsmTrafoConfig, SpaceTrafoConfig> TrafoConfig;

      /// trafo evaluator types
      typedef typename TrafoType::template Evaluator<ShapeType, DataType>::Type TrafoEvaluator;

      /// space evaluator types
      typedef typename TestSpaceType::template Evaluator<TrafoEvaluator>::Type TestEvaluator;
      typedef typename TrialSpaceType::template Evaluator<TrafoEvaluator>::Type TrialEvaluator;
      typedef TrialEvaluator MultEvaluator;

      /// trafo evaluation data type
      typedef Trafo::EvalData<TrafoEvaluator, TrafoConfig> TrafoEvalData;
      typedef TrafoEvalData TrafoData;

      /// space evaluation data types
      typedef Space::EvalData<TestEvaluator, TestConfig> TestEvalData;
      typedef Space::EvalData<TrialEvaluator, TrialConfig> TrialEvalData;
      typedef TrialEvalData MultEvalData;

      /// basis function data types
      typedef Space::FuncData<TestEvaluator, TestConfig> TestFuncData;
      typedef Space::FuncData<TrialEvaluator, TrialConfig> TrialFuncData;
      typedef TrialFuncData MultFuncData;

      /// dof-mapping types
      typedef typename TestSpaceType::DofMappingType TestDofMapping;
      typedef typename TrialSpaceType::DofMappingType TrialDofMapping;
      typedef TrialDofMapping MultDofMapping;

      /// local vector type
      typedef Tiny::Vector<DataType, TestEvaluator::max_local_dofs> LocalTestVectorType;
      typedef Tiny::Vector<DataType, TrialEvaluator::max_local_dofs> LocalTrialVectorType;
      typedef LocalTrialVectorType LocalMultVectorType;

      /// local matrix type
      typedef Tiny::Matrix<DataType, TestEvaluator::max_local_dofs, TrialEvaluator::max_local_dofs> LocalMatrixType;

      /// local matrix data type
      typedef LocalMatrixData<LocalMatrixType, TestDofMapping, TrialDofMapping> LocalMatrixDataType;

      /// cubature rule type
      typedef typename Intern::CubatureTraits<TrafoEvaluator>::RuleType CubatureRuleType;

      /// cubature factory type
      typedef typename Intern::CubatureTraits<TrafoEvaluator>::DynamicFactoryType CubatureDynamicFactoryType;

    }; // class AsmTraits2
  } // namespace Assembly
} // namespace FEAST

#endif // KERNEL_ASSEMBLY_ASM_TRAITS_HPP
