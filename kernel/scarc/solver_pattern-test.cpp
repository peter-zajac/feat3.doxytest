#include <kernel/base_header.hpp>
#include <test_system/test_system.hpp>

#include <kernel/scarc/solver_functor.hpp>
#include <kernel/scarc/solver_pattern.hpp>
#include <kernel/util/cpp11_smart_pointer.hpp>
#include <kernel/archs.hpp>
#include<deque>

using namespace FEAST;
using namespace FEAST::TestSystem;
using namespace FEAST::Foundation;
using namespace FEAST::LAFEM;
using namespace FEAST::ScaRC;


template<typename Tag_, typename Algo_, typename DataType_>
class SolverPatternTest:
  public TaggedTest<Tag_, DataType_>
{
  public:
    SolverPatternTest(const std::string & tag) :
      TaggedTest<Tag_, DataType_>("SolverPatternTest<" + tag + ">")
    {
    }

    virtual void run() const
    {
      DenseVector<Tag_, DataType_> x(100, DataType_(1)), b(100, DataType_(1));
      SparseMatrixCOO<Mem::Main, DataType_> T(100, 100);
      for(Index i(0) ; i < 100 ; ++i)
        T(i, i, DataType_(1));
      SparseMatrixCSR<Tag_, DataType_> A(T);

      PreconditionedSolverData<> data(A, A, x, b,
                        SolverPatternGeneration<Richardson, Algo_>::min_num_temp_vectors(),
                        SolverPatternGeneration<Richardson, Algo_>::min_num_temp_scalars());
      std::shared_ptr<SolverFunctorBase<DenseVector<Tag_, DataType_> > > solver(SolverPatternGeneration<Richardson, Algo_>::execute(data, 20, 1e-8));
      TEST_CHECK_EQUAL(solver->type_name(), "CompoundSolverFunctor[DefectFunctor, NormFunctor2, IterateFunctor[CompoundSolverFunctor[ProductFunctor, SumFunctor, DefectFunctor, NormFunctor2, DivFunctor]]]");
      solver->execute();

      //---------------------------------------------------------------------------------------------------------------------------------------------
      DenseVector<Tag_, DataType_> dummy;
      PreconditionedSolverData<> data3(A, A, x, b,
                         SolverPatternGeneration<RichardsonLayer, Algo_>::min_num_temp_vectors(),
                         SolverPatternGeneration<RichardsonLayer, Algo_>::min_num_temp_scalars());
      std::shared_ptr<SolverFunctorBase<DenseVector<Tag_, DataType_> > > solver3(SolverPatternGeneration<RichardsonLayer, Algo_>::execute(data3, dummy, 20, 1e-8));
      TEST_CHECK_EQUAL(solver3->type_name(), "CompoundSolverFunctor[DefectFunctor, NormFunctor2, IterateFunctor[CompoundSolverFunctor[ProductFunctor, SumFunctor, DefectFunctor, NormFunctor2, DivFunctor]]]");
      TEST_CHECK_THROWS(solver3->execute(), ScaRCError);

    }
};
SolverPatternTest<Mem::Main, Algo::Generic,  double> sf_cpu_double("StorageType: std::vector, DataType: double");
