#include <kernel/base_header.hpp>
#include <kernel/archs.hpp>
#include <test_system/test_system.hpp>
#include <kernel/lafem/dense_vector.hpp>
#include <kernel/lafem/algorithm.hpp>

using namespace FEAST;
using namespace FEAST::LAFEM;
using namespace FEAST::TestSystem;

template<
  typename Arch_,
  typename Algo_,
  typename DT_>
class DVDifferenceTest
  : public TaggedTest<Arch_, DT_, Algo_>
{

public:

  DVDifferenceTest()
    : TaggedTest<Arch_, DT_, Algo_>("dv_difference_test")
  {
  }

  virtual void run() const
  {
    for (Index size(1) ; size < 1e5 ; size*=2)
    {
      DenseVector<Mem::Main, DT_> a_local(size);
      DenseVector<Mem::Main, DT_> b_local(size);
      DenseVector<Mem::Main, DT_> ref(size);
      DenseVector<Mem::Main, DT_> result_local(size);
      for (Index i(0) ; i < size ; ++i)
      {
        a_local(i, DT_(i * DT_(1.234)));
        b_local(i, DT_(size*2 - i));
        ref(i, a_local(i) - b_local(i));
      }

      DenseVector<Arch_, DT_> a(size);
      copy(a, a_local);
      DenseVector<Arch_, DT_> b(size);
      copy(b, b_local);
      DenseVector<Arch_, DT_> c(size);

      c.template difference<Algo_>(a, b);
      copy(result_local, c);
      TEST_CHECK_EQUAL(result_local, ref);

      c.template apply<Algo_>(b, a, DT_(-1));
      copy(result_local, c);
      TEST_CHECK_EQUAL(result_local, ref);

      a.template difference<Algo_>(a, b);
      copy(result_local, a);
      TEST_CHECK_EQUAL(result_local, ref);

      copy(a, a_local);
      b.template difference<Algo_>(a, b);
      copy(result_local, b);
      TEST_CHECK_EQUAL(result_local, ref);
    }
  }
};
DVDifferenceTest<Mem::Main, Algo::Generic, float> dv_difference_test_float;
DVDifferenceTest<Mem::Main, Algo::Generic, double> dv_difference_test_double;
#ifdef FEAST_GMP
DVDifferenceTest<Mem::Main, Algo::Generic, mpf_class> dv_difference_test_mpf_class;
#endif
#ifdef FEAST_BACKENDS_MKL
DVDifferenceTest<Mem::Main, Algo::MKL, float> mkl_dv_difference_test_float;
DVDifferenceTest<Mem::Main, Algo::MKL, double> mkl_dv_difference_test_double;
#endif
#ifdef FEAST_BACKENDS_CUDA
DVDifferenceTest<Mem::CUDA, Algo::CUDA, float> cuda_dv_difference_test_float;
DVDifferenceTest<Mem::CUDA, Algo::CUDA, double> cuda_dv_difference_test_double;
#endif
