#include <kernel/base_header.hpp>
#include <kernel/archs.hpp>
#include <test_system/test_system.hpp>
#include <kernel/lafem/sparse_matrix_banded.hpp>
#include <kernel/util/binary_stream.hpp>

#include <kernel/util/random.hpp>
#include <kernel/adjacency/permutation.hpp>
#include <sstream>

using namespace FEAST;
using namespace FEAST::LAFEM;
using namespace FEAST::TestSystem;


/**
 * \brief Test class for the sparse matrix banded class.
 *
 * \test test description missing
 *
 * \tparam Mem_
 * description missing
 *
 * \tparam DT_
 * description missing
 *
 * \author Christoph Lohmann
 */
template<
  typename Mem_,
  typename Algo_,
  typename DT_,
  typename IT_>
class SparseMatrixBandedTest
  : public FullTaggedTest<Mem_, Algo_, DT_, IT_>
{
public:
  SparseMatrixBandedTest()
    : FullTaggedTest<Mem_, Algo_, DT_, IT_>("SparseMatrixBandedTest")
  {
  }

  typedef SparseMatrixBanded<Mem_, DT_, IT_> MatrixType;

  virtual void run() const
  {
    Random::SeedType seed(Random::SeedType(time(NULL)));
    Random random(seed);
    std::cout << "seed: " << seed << std::endl;

    // create random matrix
    const Index tsize(100);
    const Index rows(tsize + random(Index(0), Index(20)));
    const Index columns(tsize + random(Index(0), Index(20)));

    const Index num_of_offsets(5 + random(Index(0), Index(10)));

    DenseVector<Mem::Main, IT_, IT_> tvec_offsets(num_of_offsets);
    DenseVector<Mem_, DT_, IT_> vec_val(num_of_offsets * rows, DT_(1));

    // create random vector of offsets
    FEAST::Adjacency::Permutation permutation(rows + columns - 1, random);
    for (Index i(0); i < num_of_offsets; ++i)
    {
      tvec_offsets(i, IT_(permutation.get_perm_pos()[i]));
    }
    std::sort(tvec_offsets.elements(), tvec_offsets.elements() + num_of_offsets);
    DenseVector<Mem_, IT_, IT_> vec_offsets;
    vec_offsets.convert(tvec_offsets);

    SparseMatrixBanded<Mem_, DT_, IT_> a(rows, columns, vec_val, vec_offsets);

    // calculate number of used elements
    Index nnz(0);
    for (Index i(0); i < a.rows(); ++i)
    {
      for (Index j(0); j < a.columns(); ++j)
      {
        nnz += Index(a(i, j));
      }
    }

    TEST_CHECK_EQUAL(a.used_elements(), nnz);
    TEST_CHECK_EQUAL(a.size(), rows * columns);
    TEST_CHECK_EQUAL(a.rows(), rows);
    TEST_CHECK_EQUAL(a.columns(), columns);

    auto b = a.clone();
    SparseMatrixBanded<Mem_, DT_, IT_> bl(b.layout());
    TEST_CHECK_EQUAL(bl.used_elements(), b.used_elements());
    TEST_CHECK_EQUAL(bl.size(), b.size());
    TEST_CHECK_EQUAL(bl.rows(), b.rows());
    TEST_CHECK_EQUAL(bl.columns(), b.columns());

    bl = b.layout();
    TEST_CHECK_EQUAL(bl.used_elements(), b.used_elements());
    TEST_CHECK_EQUAL(bl.size(), b.size());
    TEST_CHECK_EQUAL(bl.rows(), b.rows());
    TEST_CHECK_EQUAL(bl.columns(), b.columns());

    typename SparseLayout<Mem_, IT_, SparseLayoutId::lt_banded>::template MatrixType<DT_> x(b.layout());
    // icc 14.0.2 does not understand the following line, so we need a typedef hier
    //typename decltype(b.layout())::template MatrixType<DT_> y(b.layout());
    typedef decltype(b.layout()) LayoutId;
    typename LayoutId::template MatrixType<DT_> y(b.layout());
    TEST_CHECK_EQUAL((void*)x.offsets(), (void*)b.offsets());
    TEST_CHECK_NOT_EQUAL((void*)x.val(), (void*)b.val());

    SparseMatrixBanded<Mem_, DT_, IT_> z;
    z.convert(a);
    TEST_CHECK_EQUAL(a, z);

    SparseMatrixBanded<Mem_, DT_, IT_> c;
    c.clone(a);
    TEST_CHECK_NOT_EQUAL((void*)c.val(), (void*)a.val());
    TEST_CHECK_EQUAL((void*)c.offsets(), (void*)a.offsets());
    c = z.clone(CloneMode::Deep);
    TEST_CHECK_NOT_EQUAL((void*)c.val(), (void*)z.val());
    TEST_CHECK_NOT_EQUAL((void*)c.offsets(), (void*)z.offsets());
    c = z.shared();
    TEST_CHECK_EQUAL((void*)c.val(), (void*)z.val());
    TEST_CHECK_EQUAL((void*)c.offsets(), (void*)z.offsets());

    DenseVector<Mem_, IT_, IT_> offsets_d(c.num_of_offsets(), c.offsets());
    DenseVector<Mem_, DT_, IT_> val_d(c.num_of_offsets() * c.rows(), c.val());
    SparseMatrixBanded<Mem_, DT_, IT_> d(c.rows(), c.columns(), val_d, offsets_d);
    TEST_CHECK_EQUAL(d, c);

    SparseMatrixBanded<Mem::Main, DT_, IT_> e;
    e.convert(c);
    TEST_CHECK_EQUAL(e, c);
    e.copy(c);
    TEST_CHECK_EQUAL(e, c);

    BinaryStream bs;
    c.write_out(FileMode::fm_bm, bs);
    bs.seekg(0);
    SparseMatrixBanded<Mem_, DT_, IT_> f(FileMode::fm_bm, bs);
    TEST_CHECK_EQUAL(f, c);

    auto kp = c.serialise();
    SparseMatrixBanded<Mem_, DT_, IT_> k(kp);
    TEST_CHECK_EQUAL(k, c);
  }
};

SparseMatrixBandedTest<Mem::Main, NotSet, float, unsigned long> cpu_sparse_matrix_banded_test_float_ulong;
SparseMatrixBandedTest<Mem::Main, NotSet, double, unsigned long> cpu_sparse_matrix_banded_test_double_ulong;
SparseMatrixBandedTest<Mem::Main, NotSet, float, unsigned int> cpu_sparse_matrix_banded_test_float_uint;
SparseMatrixBandedTest<Mem::Main, NotSet, double, unsigned int> cpu_sparse_matrix_banded_test_double_uint;
#ifdef FEAST_BACKENDS_CUDA
SparseMatrixBandedTest<Mem::CUDA, NotSet, float, unsigned long> cuda_sparse_matrix_banded_test_float_ulong;
SparseMatrixBandedTest<Mem::CUDA, NotSet, double, unsigned long> cuda_sparse_matrix_banded_test_double_ulong;
SparseMatrixBandedTest<Mem::CUDA, NotSet, float, unsigned int> cuda_sparse_matrix_banded_test_float_uint;
SparseMatrixBandedTest<Mem::CUDA, NotSet, double, unsigned int> cuda_sparse_matrix_banded_test_double_uint;
#endif


template<
  typename Mem_,
  typename Algo_,
  typename DT_,
  typename IT_>
class SparseMatrixBandedApplyTest
  : public FullTaggedTest<Mem_, Algo_, DT_, IT_>
{
private:
  const Index _opt;
public:
  SparseMatrixBandedApplyTest()
    : FullTaggedTest<Mem_, Algo_, DT_, IT_>("SparseMatrixBandedApplyTest"), _opt(0)
  {
  }

  SparseMatrixBandedApplyTest(const Index opt)
    : FullTaggedTest<Mem_, Algo_, DT_, IT_>("SparseMatrixBandedApplyTest: "
                                            + stringify(opt) + " offsets"), _opt(opt)
  {
  }

  typedef SparseMatrixBanded<Mem_, DT_, IT_> MatrixType;

  virtual void run() const
  {
    DT_ eps(Math::pow(Math::eps<DT_>(), DT_(0.5)));

    Random::SeedType seed(Random::SeedType(time(NULL)));
    Random random(seed);
    std::cout << "seed: " << seed << std::endl;

    DT_ s(DT_(4.321));

    // create random matrix
    const Index tsize(100);
    const Index rows(tsize + random(Index(0), Index(20)));
    const Index columns(tsize + random(Index(0), Index(20)));

    Index num_of_offsets;
    if (_opt == 0)
      num_of_offsets = 5 + random(Index(0), Index(10));
    else
      num_of_offsets = _opt;

    DenseVector<Mem::Main, IT_, IT_> tvec_offsets(num_of_offsets);
    DenseVector<Mem_, DT_, IT_> vec_val(num_of_offsets * rows);

    // create random vector of offsets
    FEAST::Adjacency::Permutation permutation(rows + columns - 1, random);
    for (Index i(0); i < num_of_offsets; ++i)
    {
      tvec_offsets(i, IT_(permutation.get_perm_pos()[i]));
    }
    std::sort(tvec_offsets.elements(), tvec_offsets.elements() + num_of_offsets);
    DenseVector<Mem_, IT_, IT_> vec_offsets;
    vec_offsets.convert(tvec_offsets);

    // fill data-array
    for (Index i(0); i < vec_val.size(); ++i)
    {
      vec_val(i, random(DT_(0), DT_(10)));
    }

    // create test-matrix
    SparseMatrixBanded<Mem_, DT_, IT_> sys(rows, columns, vec_val, vec_offsets);

    auto x(sys.create_vector_r());
    DenseVector<Mem_, DT_, IT_> r(rows, DT_(0));
    auto ref1(sys.create_vector_l());
    auto ref2(sys.create_vector_l());

    for (Index i(0); i < x.size(); ++i)
    {
      x(i, random(DT_(-1), DT_(1)));
    }

    for (Index i(0); i < ref1.size(); ++i)
    {
      ref1(i, 0);
    }

    sys.template apply<Algo_>(r, x);

    for(Index i(0); i < sys.rows(); ++i)
    {
      for(Index j(0); j < sys.columns(); ++j)
      {
        ref1(i, ref1(i) + sys(i, j) * x(j));
      }
    }

    // check, if the result is correct
    for (Index i(0) ; i < r.size() ; ++i)
    {
      if (fabs(ref1(i)) > eps)
        TEST_CHECK_EQUAL_WITHIN_EPS(r(i), ref1(i), eps);
    }

    for (Index i(0); i < r.size(); ++i)
    {
      ref2(i, ref1(i) + Math::cos(DT_(i)));
    }

    sys.template apply<Algo_>(r, x, ref2, DT_(-1.0));

    // check, if the result is correct
    for (Index i(0) ; i < r.size() ; ++i)
    {
      if (fabs(ref1(i)) > eps)
        TEST_CHECK_EQUAL_WITHIN_EPS(Math::cos(DT_(i)), r(i), eps);
    }

    for (Index i(0); i < r.size(); ++i)
    {
      ref2(i, ref2(i) * s);
    }

    sys.template apply<Algo_>(r, x, ref2, -s);

    // check, if the result is correct
    for (Index i(0) ; i < r.size() ; ++i)
    {
      if (fabs(ref1(i)) > eps)
        TEST_CHECK_EQUAL_WITHIN_EPS(Math::cos(DT_(i)) * s, r(i), eps);
    }
}
};

SparseMatrixBandedApplyTest<Mem::Main, Algo::Generic, float, unsigned long> cpu_sparse_matrix_banded_apply_test_float_ulong;
SparseMatrixBandedApplyTest<Mem::Main, Algo::Generic, double, unsigned long> cpu_sparse_matrix_banded_apply_test_double_ulong;
SparseMatrixBandedApplyTest<Mem::Main, Algo::Generic, float, unsigned int> cpu_sparse_matrix_banded_apply_test_float_uint;
SparseMatrixBandedApplyTest<Mem::Main, Algo::Generic, double, unsigned int> cpu_sparse_matrix_banded_apply_test_double_uint;

SparseMatrixBandedApplyTest<Mem::Main, Algo::Generic, float, unsigned long> cpu_sparse_matrix_banded_apply_test_float_ulong_9(9);
SparseMatrixBandedApplyTest<Mem::Main, Algo::Generic, double, unsigned long> cpu_sparse_matrix_banded_apply_test_double_ulong_9(9);
SparseMatrixBandedApplyTest<Mem::Main, Algo::Generic, float, unsigned int> cpu_sparse_matrix_banded_apply_test_float_uint_9(9);
SparseMatrixBandedApplyTest<Mem::Main, Algo::Generic, double, unsigned int> cpu_sparse_matrix_banded_apply_test_double_uint_9(9);
// #ifdef FEAST_BACKENDS_MKL
// SparseMatrixBandedApplyTest<Mem::Main, Algo::MKL, float, unsigned long> mkl_sparse_matrix_banded_apply_test_float_ulong;
// SparseMatrixBandedApplyTest<Mem::Main, Algo::MKL, double, unsigned long> mkl_sparse_matrix_banded_apply_test_double_ulong;
// #endif
#ifdef FEAST_BACKENDS_CUDA
SparseMatrixBandedApplyTest<Mem::CUDA, Algo::CUDA, float, unsigned long> cuda_sparse_matrix_banded_apply_test_float_ulong;
SparseMatrixBandedApplyTest<Mem::CUDA, Algo::CUDA, double, unsigned long> cuda_sparse_matrix_banded_apply_test_double_ulong;
SparseMatrixBandedApplyTest<Mem::CUDA, Algo::CUDA, float, unsigned int> cuda_sparse_matrix_banded_apply_test_float_uint;
SparseMatrixBandedApplyTest<Mem::CUDA, Algo::CUDA, double, unsigned int> cuda_sparse_matrix_banded_apply_test_double_uint;
#endif


template<
  typename Mem_,
  typename Algo_,
  typename DT_,
  typename IT_>
class SparseMatrixBandedScaleTest
  : public FullTaggedTest<Mem_, Algo_, DT_, IT_>
{
public:
  SparseMatrixBandedScaleTest()
    : FullTaggedTest<Mem_, Algo_, DT_, IT_>("SparseMatrixBandedScaleTest")
  {
  }

  typedef SparseMatrixBanded<Mem_, DT_, IT_> MatrixType;

  virtual void run() const
  {
    Random::SeedType seed(Random::SeedType(time(NULL)));
    Random random(seed);
    std::cout << "seed: " << seed << std::endl;

    DT_ s(DT_(4.321));

    // create random matrix
    const Index tsize(100);
    const Index rows(tsize + random(Index(0), Index(20)));
    const Index columns(tsize + random(Index(0), Index(20)));

    const Index num_of_offsets(5 + random(Index(0), Index(10)));

    DenseVector<Mem::Main, IT_, IT_> tvec_offsets(num_of_offsets);
    DenseVector<Mem_, DT_, IT_> vec_val_a(num_of_offsets * rows);
    DenseVector<Mem_, DT_, IT_> vec_val_ref(num_of_offsets * rows, DT_(1));

    // create random vector of offsets
    FEAST::Adjacency::Permutation permutation(rows + columns - 1, random);
    for (Index i(0); i < num_of_offsets; ++i)
    {
      tvec_offsets(i, IT_(permutation.get_perm_pos()[i]));
    }
    std::sort(tvec_offsets.elements(), tvec_offsets.elements() + num_of_offsets);
    DenseVector<Mem_, IT_, IT_> vec_offsets;
    vec_offsets.convert(tvec_offsets);

    // fill data-array
    for (Index i(0); i < vec_val_a.size(); ++i)
    {
      vec_val_a(i, random(DT_(0), DT_(10)));
      vec_val_ref(i, vec_val_a(i) * s);
    }

    // create test-matrix
    SparseMatrixBanded<Mem_, DT_, IT_> ref(rows, columns, vec_val_ref, vec_offsets);
    SparseMatrixBanded<Mem_, DT_, IT_> a(rows, columns, vec_val_a, vec_offsets);
    SparseMatrixBanded<Mem_, DT_, IT_> b;
    b.clone(a);

    b.template scale<Algo_>(a, s);
    TEST_CHECK_EQUAL(b, ref);

    a.template scale<Algo_>(a, s);
    TEST_CHECK_EQUAL(a, ref);
  }
};

SparseMatrixBandedScaleTest<Mem::Main, Algo::Generic, float, unsigned long> cpu_sparse_matrix_banded_scale_test_float_ulong;
SparseMatrixBandedScaleTest<Mem::Main, Algo::Generic, double, unsigned long> cpu_sparse_matrix_banded_scale_test_double_ulong;
SparseMatrixBandedScaleTest<Mem::Main, Algo::Generic, float, unsigned int> cpu_sparse_matrix_banded_scale_test_float_uint;
SparseMatrixBandedScaleTest<Mem::Main, Algo::Generic, double, unsigned int> cpu_sparse_matrix_banded_scale_test_double_uint;
#ifdef FEAST_BACKENDS_MKL
SparseMatrixBandedScaleTest<Mem::Main, Algo::MKL, float, unsigned long> mkl_sparse_matrix_banded_scale_test_float_ulong;
SparseMatrixBandedScaleTest<Mem::Main, Algo::MKL, double, unsigned long> mkl_sparse_matrix_banded_scale_test_double_ulong;
#endif
#ifdef FEAST_BACKENDS_CUDA
SparseMatrixBandedScaleTest<Mem::CUDA, Algo::CUDA, float, unsigned long> cuda_sparse_matrix_banded_scale_test_float_ulong;
SparseMatrixBandedScaleTest<Mem::CUDA, Algo::CUDA, double, unsigned long> cuda_sparse_matrix_banded_scale_test_double_ulong;
SparseMatrixBandedScaleTest<Mem::CUDA, Algo::CUDA, float, unsigned int> cuda_sparse_matrix_banded_scale_test_float_uint;
SparseMatrixBandedScaleTest<Mem::CUDA, Algo::CUDA, double, unsigned int> cuda_sparse_matrix_banded_scale_test_double_uint;
#endif
