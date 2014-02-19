#include <test_system/test_system.hpp>
#include <kernel/base_header.hpp>
#include <kernel/archs.hpp>
#include <kernel/lafem/dense_vector.hpp>
#include <kernel/lafem/power_vector.hpp>
#include <kernel/lafem/tuple_vector.hpp>
#include <kernel/lafem/unit_filter.hpp>
#include <kernel/lafem/mean_filter.hpp>
#include <kernel/lafem/power_filter.hpp>
#include <kernel/lafem/tuple_filter.hpp>

using namespace FEAST;
using namespace FEAST::LAFEM;
using namespace FEAST::TestSystem;

template<typename Algo_, typename DataType_>
class MetaFilterTest :
  public TaggedTest<typename Algo_::MemType, DataType_, Algo_>
{
public:
  typedef Algo_ AlgoType;
  typedef typename AlgoType::MemType MemType;
  typedef DataType_ DataType;

  typedef DenseVector<MemType, DataType> ScalarVector;
  typedef PowerVector<ScalarVector, 2> PowerVector2;
  typedef TupleVector<PowerVector2, ScalarVector> MetaVector;

  typedef UnitFilter<MemType, DataType> ScalarFilter1;
  typedef MeanFilter<MemType, DataType> ScalarFilter2;
  typedef PowerFilter<ScalarFilter1, 2> PowerFilter2;
  typedef TupleFilter<PowerFilter2, ScalarFilter2> MetaFilter;

  MetaFilterTest() : TaggedTest<typename Algo_::MemType, DataType_, Algo_>("MetaFilterTest") {}

  static MetaFilter gen_filter(Index m)
  {
    // create a unit-filter
    UnitFilter<Mem::Main, DataType> unit_filter(2);
    Index* idx(unit_filter.get_indices());
    DataType* fv(unit_filter.get_values());
    idx[0] = 0;
    idx[1] = m-1;
    fv[0] = DataType(1);
    fv[1] = DataType(5);

    // create vectors for mean-filter
    DenseVector<Mem::Main, DataType> mfv(m, DataType(1)), mfw(m, DataType(0));
    DataType* fw(mfw.elements());
    for(Index i(0); i < m; ++i)
      fw[i] = DataType(i+1);

    // create a mean-filter
    MeanFilter<Mem::Main, DataType> mean_filter(std::move(mfv), std::move(mfw), DataType(((m+1)*(m+2))/2));

    // create a power-filer
    PowerFilter2 power_filter;
    power_filter.template at<Index(0)>() = unit_filter.clone();
    power_filter.template at<Index(1)>() = unit_filter.clone();

    // return the tuple-filter
    return MetaFilter(std::move(power_filter), std::move(mean_filter));
  }

  static MetaVector gen_vector(Index m)
  {
    PowerVector2 vec;
    vec.template at<Index(0)>() = ScalarVector(m, DataType(2));
    vec.template at<Index(1)>() = ScalarVector(m, DataType(3));

    return MetaVector(std::move(vec), ScalarVector(m, DataType(1)));
  }

  static MetaVector gen_vector_sol(Index m)
  {
    DenseVector<Mem::Main, DataType> vx(m, DataType(2));
    DenseVector<Mem::Main, DataType> vy(m, DataType(3));
    DenseVector<Mem::Main, DataType> vz(m, DataType(2) / DataType(7));

    DataType* fx(vx.elements());
    DataType* fy(vy.elements());
    fx[0] = fy[0] = DataType(1);
    fx[m-1] = fy[m-1] = DataType(5);

    // create a power-vector
    PowerVector2 vec;
    vec.template at<Index(0)>() = ScalarVector(vx);
    vec.template at<Index(1)>() = ScalarVector(vy);

    return MetaVector(std::move(vec), ScalarVector(vz));
  }

  static MetaVector gen_vector_def(Index m)
  {
    DenseVector<Mem::Main, DataType> vx(m, DataType(2));
    DenseVector<Mem::Main, DataType> vy(m, DataType(3));
    DenseVector<Mem::Main, DataType> vz(m, DataType(0));

    DataType* fx(vx.elements());
    DataType* fy(vy.elements());
    DataType* fz(vz.elements());
    fx[0] = fy[0] = fx[m-1] = fy[m-1] = DataType(0);
    for(Index i(0); i < m; ++i)
    {
      fz[i] = DataType_(32 - 10*int(i)) / DataType_(42);
    }

    // create a power-vector
    PowerVector2 vec;
    vec.template at<Index(0)>() = ScalarVector(vx);
    vec.template at<Index(1)>() = ScalarVector(vy);

    return MetaVector(std::move(vec), ScalarVector(vz));
  }
  virtual void run() const
  {
    const DataType tol(Math::pow(Math::eps<DataType>(), DataType(0.7)));

    const Index m(5);

    // create a power-filter
    MetaFilter filter(gen_filter(m));

    // generate two input vector
    MetaVector vec_sol(gen_vector(m));
    MetaVector vec_def(vec_sol.clone());

    // appy sol filter
    filter.template filter_sol<AlgoType>(vec_sol);
    filter.template filter_def<AlgoType>(vec_def);

    // generate ref vectors
    const MetaVector ref_sol(gen_vector_sol(m));
    const MetaVector ref_def(gen_vector_def(m));

    // subtract reference
    vec_sol.template axpy<AlgoType>(ref_sol, vec_sol, -DataType(1));
    vec_def.template axpy<AlgoType>(ref_def, vec_def, -DataType(1));

    // check norm
    TEST_CHECK_EQUAL_WITHIN_EPS(vec_sol.template norm2<AlgoType>(), DataType(0), tol);
    TEST_CHECK_EQUAL_WITHIN_EPS(vec_def.template norm2<AlgoType>(), DataType(0), tol);
  }
};

MetaFilterTest<Algo::Generic, double> meta_filter_test_generic_double;
MetaFilterTest<Algo::Generic, float> meta_filter_test_generic_float;
