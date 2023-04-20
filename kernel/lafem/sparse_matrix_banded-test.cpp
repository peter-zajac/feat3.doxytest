// FEAT3: Finite Element Analysis Toolbox, Version 3
// Copyright (C) 2010 - 2023 by Stefan Turek & the FEAT group
// FEAT3 is released under the GNU General Public License version 3,
// see the file 'copyright.txt' in the top level directory for details.

#include <kernel/base_header.hpp>
#include <test_system/test_system.hpp>
#include <kernel/lafem/sparse_matrix_banded.hpp>
#include <kernel/util/binary_stream.hpp>

#include <kernel/util/random.hpp>
#include <kernel/adjacency/permutation.hpp>
#include <sstream>

using namespace FEAT;
using namespace FEAT::LAFEM;
using namespace FEAT::TestSystem;


/**
 * \brief Test class for the sparse matrix banded class.
 *
 * \test test description missing
 *
 * \tparam DT_
 * description missing
 *
 * \author Christoph Lohmann
 */
template<
  typename DT_,
  typename IT_>
class SparseMatrixBandedTest
  : public UnitTest
{
public:
   SparseMatrixBandedTest(PreferredBackend backend)
    : UnitTest("SparseMatrixBandedTest", Type::Traits<DT_>::name(), Type::Traits<IT_>::name(), backend)
  {
  }

  virtual ~SparseMatrixBandedTest()
  {
  }

  typedef SparseMatrixBanded<DT_, IT_> MatrixType;

  virtual void run() const override
  {
    Random::SeedType seed(Random::SeedType(time(nullptr)));
    Random random(seed);
    std::cout << "seed: " << seed << std::endl;

    // create random matrix
    const Index tsize(100);
    const Index rows(tsize + random(Index(0), Index(20)));
    const Index columns(tsize + random(Index(0), Index(20)));

    const Index num_of_offsets(5 + random(Index(0), Index(10)));

    DenseVector<IT_, IT_> vec_offsets(num_of_offsets);
    DenseVector<DT_, IT_> vec_val(num_of_offsets * rows, DT_(1));

    // create random vector of offsets
    FEAT::Adjacency::Permutation permutation(rows + columns - 1, random);
    for (Index i(0); i < num_of_offsets; ++i)
    {
      vec_offsets(i, IT_(permutation.get_perm_pos()[i]));
    }
    std::sort(vec_offsets.elements(), vec_offsets.elements() + num_of_offsets);

    SparseMatrixBanded<DT_, IT_> a(rows, columns, vec_val, vec_offsets);

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
    SparseMatrixBanded<DT_, IT_> bl(b.layout());
    TEST_CHECK_EQUAL(bl.used_elements(), b.used_elements());
    TEST_CHECK_EQUAL(bl.size(), b.size());
    TEST_CHECK_EQUAL(bl.rows(), b.rows());
    TEST_CHECK_EQUAL(bl.columns(), b.columns());

    bl = b.layout();
    TEST_CHECK_EQUAL(bl.used_elements(), b.used_elements());
    TEST_CHECK_EQUAL(bl.size(), b.size());
    TEST_CHECK_EQUAL(bl.rows(), b.rows());
    TEST_CHECK_EQUAL(bl.columns(), b.columns());

    typename SparseLayout<IT_, SparseLayoutId::lt_banded>::template MatrixType<DT_> x(b.layout());
    // icc 14.0.2 does not understand the following line, so we need a typedef hier
    //typename decltype(b.layout())::template MatrixType<DT_> y(b.layout());
    typedef decltype(b.layout()) LayoutId;
    typename LayoutId::template MatrixType<DT_> y(b.layout());
    TEST_CHECK_EQUAL((void*)x.offsets(), (void*)b.offsets());
    TEST_CHECK_NOT_EQUAL((void*)x.val(), (void*)b.val());

    SparseMatrixBanded<DT_, IT_> z;
    z.convert(a);
    TEST_CHECK_EQUAL(a, z);

    SparseMatrixBanded<DT_, IT_> c;
    c.clone(a);
    TEST_CHECK_NOT_EQUAL((void*)c.val(), (void*)a.val());
    TEST_CHECK_EQUAL((void*)c.offsets(), (void*)a.offsets());
    c = z.clone(CloneMode::Deep);
    TEST_CHECK_NOT_EQUAL((void*)c.val(), (void*)z.val());
    TEST_CHECK_NOT_EQUAL((void*)c.offsets(), (void*)z.offsets());

    DenseVector<IT_, IT_> offsets_d(c.num_of_offsets(), c.offsets());
    DenseVector<DT_, IT_> val_d(c.num_of_offsets() * c.rows(), c.val());
    SparseMatrixBanded<DT_, IT_> d(c.rows(), c.columns(), val_d, offsets_d);
    TEST_CHECK_EQUAL(d, c);

    SparseMatrixBanded<DT_, IT_> e;
    e.convert(c);
    TEST_CHECK_EQUAL(e, c);
    e.copy(c);
    TEST_CHECK_EQUAL(e, c);
  }
};

SparseMatrixBandedTest <float, std::uint64_t> cpu_sparse_matrix_banded_test_float_uint64(PreferredBackend::generic);
SparseMatrixBandedTest <double, std::uint64_t> cpu_sparse_matrix_banded_test_double_uint64(PreferredBackend::generic);
SparseMatrixBandedTest <float, std::uint32_t> cpu_sparse_matrix_banded_test_float_uint32(PreferredBackend::generic);
SparseMatrixBandedTest <double, std::uint32_t> cpu_sparse_matrix_banded_test_double_uint32(PreferredBackend::generic);
#ifdef FEAT_HAVE_MKL
SparseMatrixBandedTest <float, std::uint64_t> mkl_cpu_sparse_matrix_banded_test_float_uint64(PreferredBackend::mkl);
SparseMatrixBandedTest <double, std::uint64_t> mkl_cpu_sparse_matrix_banded_test_double_uint64(PreferredBackend::mkl);
#endif
#ifdef FEAT_HAVE_QUADMATH
SparseMatrixBandedTest <__float128, std::uint64_t> cpu_sparse_matrix_banded_test_float128_uint64(PreferredBackend::generic);
SparseMatrixBandedTest <__float128, std::uint32_t> cpu_sparse_matrix_banded_test_float128_uint32(PreferredBackend::generic);
#endif
#ifdef FEAT_HAVE_HALFMATH
SparseMatrixBandedTest <Half, std::uint32_t> cpu_sparse_matrix_banded_test_half_uint32(PreferredBackend::generic);
SparseMatrixBandedTest <Half, std::uint64_t> cpu_sparse_matrix_banded_test_half_uint64(PreferredBackend::generic);
#endif
#ifdef FEAT_HAVE_CUDA
SparseMatrixBandedTest <float, std::uint64_t> cuda_sparse_matrix_banded_test_float_uint64(PreferredBackend::cuda);
SparseMatrixBandedTest <double, std::uint64_t> cuda_sparse_matrix_banded_test_double_uint64(PreferredBackend::cuda);
SparseMatrixBandedTest <float, std::uint32_t> cuda_sparse_matrix_banded_test_float_uint32(PreferredBackend::cuda);
SparseMatrixBandedTest <double, std::uint32_t> cuda_sparse_matrix_banded_test_double_uint32(PreferredBackend::cuda);
#endif

template<
  typename DT_,
  typename IT_>
class SparseMatrixBandedSerializeTest
  : public UnitTest
{
public:
  SparseMatrixBandedSerializeTest(PreferredBackend backend)
    : UnitTest("SparseMatrixBandedSerializeTest", Type::Traits<DT_>::name(), Type::Traits<IT_>::name(), backend)
  {
  }

  virtual ~SparseMatrixBandedSerializeTest()
  {
  }

  typedef SparseMatrixBanded<DT_, IT_> MatrixType;

  virtual void run() const override
  {
    Random::SeedType seed(Random::SeedType(time(nullptr)));
    Random random(seed);
    std::cout << "seed: " << seed << std::endl;

    // create random matrix
    const Index tsize(100);
    const Index rows(tsize + random(Index(0), Index(20)));
    const Index columns(tsize + random(Index(0), Index(20)));

    const Index num_of_offsets(5 + random(Index(0), Index(10)));

    DenseVector<IT_, IT_> vec_offsets(num_of_offsets);
    DenseVector<DT_, IT_> vec_val(num_of_offsets * rows, DT_(1));

    // create random vector of offsets
    FEAT::Adjacency::Permutation permutation(rows + columns - 1, random);
    for (Index i(0); i < num_of_offsets; ++i)
    {
      vec_offsets(i, IT_(permutation.get_perm_pos()[i]));
    }
    std::sort(vec_offsets.elements(), vec_offsets.elements() + num_of_offsets);

    SparseMatrixBanded<DT_, IT_> c(rows, columns, vec_val, vec_offsets);

    BinaryStream bs;
    c.write_out(FileMode::fm_bm, bs);
    bs.seekg(0);
    SparseMatrixBanded<DT_, IT_> f(FileMode::fm_bm, bs);
    TEST_CHECK_EQUAL(f, c);

    auto kp = c.serialize(LAFEM::SerialConfig(false, false));
    SparseMatrixBanded<DT_, IT_> k(kp);
    TEST_CHECK_EQUAL(k, c);

#ifdef FEAT_HAVE_ZLIB
    auto zl = c.serialize(LAFEM::SerialConfig(true, false));
    SparseMatrixBanded<DT_, IT_> zlib(zl);
    TEST_CHECK_EQUAL(zlib, c);
#endif
#ifdef FEAT_HAVE_ZFP
    auto zf = c.serialize(LAFEM::SerialConfig(false, true, FEAT::Real(1e-7)));
    SparseMatrixBanded<DT_, IT_> zfp(zf);
    for(Index i(0); i < c.rows() ; ++i)
    {
      for(Index j(0) ; j < c.columns() ; ++j)
      {
        TEST_CHECK_EQUAL_WITHIN_EPS(c(i,j), zfp(i,j), DT_(1e-5));
      }
    }
#endif
  }
};
SparseMatrixBandedSerializeTest <float, std::uint64_t> cpu_sparse_matrix_banded_serialize_test_float_uint64(PreferredBackend::generic);
SparseMatrixBandedSerializeTest <double, std::uint64_t> cpu_sparse_matrix_banded_serialize_test_double_uint64(PreferredBackend::generic);
SparseMatrixBandedSerializeTest <float, std::uint32_t> cpu_sparse_matrix_banded_serialize_test_float_uint32(PreferredBackend::generic);
SparseMatrixBandedSerializeTest <double, std::uint32_t> cpu_sparse_matrix_banded_serialize_test_double_uint32(PreferredBackend::generic);
#ifdef FEAT_HAVE_MKL
SparseMatrixBandedSerializeTest <float, std::uint64_t> mkl_cpu_sparse_matrix_banded_serialize_test_float_uint64(PreferredBackend::mkl);
SparseMatrixBandedSerializeTest <double, std::uint64_t> mkl_cpu_sparse_matrix_banded_serialize_test_double_uint64(PreferredBackend::mkl);
#endif
#ifdef FEAT_HAVE_QUADMATH
SparseMatrixBandedSerializeTest <__float128, std::uint64_t> cpu_sparse_matrix_banded_serialize_test_float128_uint64(PreferredBackend::generic);
SparseMatrixBandedSerializeTest <__float128, std::uint32_t> cpu_sparse_matrix_banded_serialize_test_float128_uint32(PreferredBackend::generic);
#endif
#ifdef FEAT_HAVE_HALFMATH
SparseMatrixBandedSerializeTest <Half, std::uint32_t> cpu_sparse_matrix_banded_serialize_test_half_uint32(PreferredBackend::generic);
SparseMatrixBandedSerializeTest <Half, std::uint64_t> cpu_sparse_matrix_banded_serialize_test_half_uint64(PreferredBackend::generic);
#endif
#ifdef FEAT_HAVE_CUDA
SparseMatrixBandedSerializeTest <float, std::uint64_t> cuda_sparse_matrix_banded_serialize_test_float_uint64(PreferredBackend::cuda);
SparseMatrixBandedSerializeTest <double, std::uint64_t> cuda_sparse_matrix_banded_serialize_test_double_uint64(PreferredBackend::cuda);
SparseMatrixBandedSerializeTest <float, std::uint32_t> cuda_sparse_matrix_banded_serialize_test_float_uint32(PreferredBackend::cuda);
SparseMatrixBandedSerializeTest <double, std::uint32_t> cuda_sparse_matrix_banded_serialize_test_double_uint32(PreferredBackend::cuda);
#endif

template<
  typename DT_,
  typename IT_>
class SparseMatrixBandedApplyTest
  : public UnitTest
{
private:
  const Index _opt;
public:
  SparseMatrixBandedApplyTest(PreferredBackend backend)
    : UnitTest("SparseMatrixBandedApplyTest", Type::Traits<DT_>::name(), Type::Traits<IT_>::name(), backend), _opt(0)
  {
  }

  explicit SparseMatrixBandedApplyTest(PreferredBackend backend, const Index opt)
    : UnitTest("SparseMatrixBandedApplyTest: "
                                            + stringify(opt) + " offsets", Type::Traits<DT_>::name(), Type::Traits<IT_>::name(), backend), _opt(opt)
  {
  }

  virtual ~SparseMatrixBandedApplyTest()
  {
  }

  typedef SparseMatrixBanded<DT_, IT_> MatrixType;

  virtual void run() const override
  {
    DT_ eps(Math::pow(Math::eps<DT_>(), DT_(0.5)));

    Random::SeedType seed(Random::SeedType(time(nullptr)));
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

    DenseVector<IT_, IT_> vec_offsets(num_of_offsets);
    DenseVector<DT_, IT_> vec_val(num_of_offsets * rows);

    // create random vector of offsets
    FEAT::Adjacency::Permutation permutation(rows + columns - 1, random);
    for (Index i(0); i < num_of_offsets; ++i)
    {
      vec_offsets(i, IT_(permutation.get_perm_pos()[i]));
    }
    std::sort(vec_offsets.elements(), vec_offsets.elements() + num_of_offsets);

    // fill data-array
    for (Index i(0); i < vec_val.size(); ++i)
    {
      vec_val(i, random(DT_(0), DT_(10)));
    }

    // create test-matrix
    SparseMatrixBanded<DT_, IT_> sys(rows, columns, vec_val, vec_offsets);

    auto x(sys.create_vector_r());
    DenseVector<DT_, IT_> r(rows, DT_(0));
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

    sys.apply(r, x);

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
      if (Math::abs(ref1(i)) > eps)
        TEST_CHECK_EQUAL_WITHIN_EPS(r(i), ref1(i), eps);
    }

    for (Index i(0); i < r.size(); ++i)
    {
      ref2(i, ref1(i) + Math::cos(DT_(i)));
    }

    sys.apply(r, x, ref2, DT_(-1.0));

    // check, if the result is correct
    for (Index i(0) ; i < r.size() ; ++i)
    {
      if (Math::abs(ref1(i)) > eps)
        TEST_CHECK_EQUAL_WITHIN_EPS(Math::cos(DT_(i)), r(i), eps);
    }

    for (Index i(0); i < r.size(); ++i)
    {
      ref2(i, ref2(i) * s);
    }

    sys.apply(r, x, ref2, -s);

    // check, if the result is correct
    for (Index i(0) ; i < r.size() ; ++i)
    {
      if (Math::abs(ref1(i)) > eps)
        TEST_CHECK_EQUAL_WITHIN_EPS(Math::cos(DT_(i)) * s, r(i), eps);
    }
  }
};

SparseMatrixBandedApplyTest <float, std::uint64_t> cpu_sparse_matrix_banded_apply_test_float_uint64(PreferredBackend::generic);
SparseMatrixBandedApplyTest <double, std::uint64_t> cpu_sparse_matrix_banded_apply_test_double_uint64(PreferredBackend::generic);
SparseMatrixBandedApplyTest <float, std::uint32_t> cpu_sparse_matrix_banded_apply_test_float_uint32(PreferredBackend::generic);
SparseMatrixBandedApplyTest <double, std::uint32_t> cpu_sparse_matrix_banded_apply_test_double_uint32(PreferredBackend::generic);

SparseMatrixBandedApplyTest <float, std::uint64_t> cpu_sparse_matrix_banded_apply_test_float_uint64_9(PreferredBackend::generic,9);
SparseMatrixBandedApplyTest <double, std::uint64_t> cpu_sparse_matrix_banded_apply_test_double_uint64_9(PreferredBackend::generic,9);
SparseMatrixBandedApplyTest <float, std::uint32_t> cpu_sparse_matrix_banded_apply_test_float_uint32_9(PreferredBackend::generic,9);
SparseMatrixBandedApplyTest <double, std::uint32_t> cpu_sparse_matrix_banded_apply_test_double_uint32_9(PreferredBackend::generic,9);
#ifdef FEAT_HAVE_MKL
SparseMatrixBandedApplyTest <float, std::uint64_t> mkl_cpu_sparse_matrix_banded_apply_test_float_uint64(PreferredBackend::mkl);
SparseMatrixBandedApplyTest <double, std::uint64_t> mkl_cpu_sparse_matrix_banded_apply_test_double_uint64(PreferredBackend::mkl);
#endif
#ifdef FEAT_HAVE_QUADMATH
SparseMatrixBandedApplyTest <__float128, std::uint64_t> cpu_sparse_matrix_banded_apply_test_float128_uint64(PreferredBackend::generic);
SparseMatrixBandedApplyTest <__float128, std::uint32_t> cpu_sparse_matrix_banded_apply_test_float128_uint32(PreferredBackend::generic);
#endif
#ifdef FEAT_HAVE_HALFMATH
SparseMatrixBandedApplyTest <Half, std::uint32_t> cpu_sparse_matrix_banded_apply_test_half_uint32(PreferredBackend::generic);
SparseMatrixBandedApplyTest <Half, std::uint64_t> cpu_sparse_matrix_banded_apply_test_half_uint64(PreferredBackend::generic);
#endif
#ifdef FEAT_HAVE_CUDA
SparseMatrixBandedApplyTest <float, std::uint64_t> cuda_sparse_matrix_banded_apply_test_float_uint64(PreferredBackend::cuda);
SparseMatrixBandedApplyTest <double, std::uint64_t> cuda_sparse_matrix_banded_apply_test_double_uint64(PreferredBackend::cuda);
SparseMatrixBandedApplyTest <float, std::uint32_t> cuda_sparse_matrix_banded_apply_test_float_uint32(PreferredBackend::cuda);
SparseMatrixBandedApplyTest <double, std::uint32_t> cuda_sparse_matrix_banded_apply_test_double_uint32(PreferredBackend::cuda);
#endif

template<
  typename DT_,
  typename IT_>
class SparseMatrixBandedScaleTest
  : public UnitTest
{
public:
  SparseMatrixBandedScaleTest(PreferredBackend backend)
    : UnitTest("SparseMatrixBandedScaleTest", Type::Traits<DT_>::name(), Type::Traits<IT_>::name(), backend)
  {
  }

  virtual ~SparseMatrixBandedScaleTest()
  {
  }

  typedef SparseMatrixBanded<DT_, IT_> MatrixType;

  virtual void run() const override
  {
    Random::SeedType seed(Random::SeedType(time(nullptr)));
    Random random(seed);
    std::cout << "seed: " << seed << std::endl;

    DT_ s(DT_(4.321));

    // create random matrix
    const Index tsize(100);
    const Index rows(tsize + random(Index(0), Index(20)));
    const Index columns(tsize + random(Index(0), Index(20)));

    const Index num_of_offsets(5 + random(Index(0), Index(10)));

    DenseVector<IT_, IT_> vec_offsets(num_of_offsets);
    DenseVector<DT_, IT_> vec_val_a(num_of_offsets * rows);
    DenseVector<DT_, IT_> vec_val_ref(num_of_offsets * rows, DT_(1));

    // create random vector of offsets
    FEAT::Adjacency::Permutation permutation(rows + columns - 1, random);
    for (Index i(0); i < num_of_offsets; ++i)
    {
      vec_offsets(i, IT_(permutation.get_perm_pos()[i]));
    }
    std::sort(vec_offsets.elements(), vec_offsets.elements() + num_of_offsets);

    // fill data-array
    for (Index i(0); i < vec_val_a.size(); ++i)
    {
      vec_val_a(i, random(DT_(0), DT_(10)));
      vec_val_ref(i, vec_val_a(i) * s);
    }

    // create test-matrix
    SparseMatrixBanded<DT_, IT_> ref(rows, columns, vec_val_ref, vec_offsets);
    SparseMatrixBanded<DT_, IT_> a(rows, columns, vec_val_a, vec_offsets);
    SparseMatrixBanded<DT_, IT_> b;
    b.clone(a);

    b.scale(a, s);
    TEST_CHECK_EQUAL(b, ref);

    a.scale(a, s);
    TEST_CHECK_EQUAL(a, ref);
  }
};

SparseMatrixBandedScaleTest <float, std::uint64_t> cpu_sparse_matrix_banded_scale_test_float_uint64(PreferredBackend::generic);
SparseMatrixBandedScaleTest <double, std::uint64_t> cpu_sparse_matrix_banded_scale_test_double_uint64(PreferredBackend::generic);
SparseMatrixBandedScaleTest <float, std::uint32_t> cpu_sparse_matrix_banded_scale_test_float_uint32(PreferredBackend::generic);
SparseMatrixBandedScaleTest <double, std::uint32_t> cpu_sparse_matrix_banded_scale_test_double_uint32(PreferredBackend::generic);
#ifdef FEAT_HAVE_MKL
SparseMatrixBandedScaleTest <float, std::uint64_t> mkl_cpu_sparse_matrix_banded_scale_test_float_uint64(PreferredBackend::mkl);
SparseMatrixBandedScaleTest <double, std::uint64_t> mkl_cpu_sparse_matrix_banded_scale_test_double_uint64(PreferredBackend::mkl);
#endif
#ifdef FEAT_HAVE_QUADMATH
SparseMatrixBandedScaleTest <__float128, std::uint64_t> cpu_sparse_matrix_banded_scale_test_float128_uint64(PreferredBackend::generic);
SparseMatrixBandedScaleTest <__float128, std::uint32_t> cpu_sparse_matrix_banded_scale_test_float128_uint32(PreferredBackend::generic);
#endif
#ifdef FEAT_HAVE_HALFMATH
SparseMatrixBandedScaleTest <Half, std::uint32_t> cpu_sparse_matrix_banded_scale_test_half_uint32(PreferredBackend::generic);
SparseMatrixBandedScaleTest <Half, std::uint64_t> cpu_sparse_matrix_banded_scale_test_half_uint64(PreferredBackend::generic);
#endif
#ifdef FEAT_HAVE_CUDA
SparseMatrixBandedScaleTest <float, std::uint64_t> cuda_sparse_matrix_banded_scale_test_float_uint64(PreferredBackend::cuda);
SparseMatrixBandedScaleTest <double, std::uint64_t> cuda_sparse_matrix_banded_scale_test_double_uint64(PreferredBackend::cuda);
SparseMatrixBandedScaleTest <float, std::uint32_t> cuda_sparse_matrix_banded_scale_test_float_uint32(PreferredBackend::cuda);
SparseMatrixBandedScaleTest <double, std::uint32_t> cuda_sparse_matrix_banded_scale_test_double_uint32(PreferredBackend::cuda);
#endif
