// FEAT3: Finite Element Analysis Toolbox, Version 3
// Copyright (C) 2010 - 2023 by Stefan Turek & the FEAT group
// FEAT3 is released under the GNU General Public License version 3,
// see the file 'copyright.txt' in the top level directory for details.

#include <test_system/test_system.hpp>
#include <kernel/base_header.hpp>
#include <kernel/lafem/sparse_vector.hpp>

using namespace FEAT;
using namespace FEAT::LAFEM;
using namespace FEAT::TestSystem;

/**
 * \brief Test class for the sparse vector class.
 *
 * \test test description missing
 *
 * \tparam DT_
 * description missing
 *
 * \author Dirk Ribbrock
 */
template<
  typename DT_,
  typename IT_>
class SparseVectorTest
  : public UnitTest
{
public:
  SparseVectorTest(PreferredBackend backend)
    : UnitTest("SparseVectorTest", Type::Traits<DT_>::name(), Type::Traits<IT_>::name(), backend)
  {
  }

  virtual ~SparseVectorTest()
  {
  }

  virtual void run() const override
  {
    SparseVector<DT_, IT_> zero1;
    SparseVector<DT_, IT_> zero2;
    TEST_CHECK_EQUAL(zero1, zero2);

    SparseVector<DT_, IT_> a(10);
    a(3, DT_(7));
    a(3, DT_(3));
    a(6, DT_(1));
    a(5, DT_(6));
    a(6, DT_(8));
    TEST_CHECK_EQUAL(a.used_elements(), Index(3));
    TEST_CHECK_EQUAL(a(3), DT_(3));
    TEST_CHECK_EQUAL(a(2), DT_(0));
    TEST_CHECK_EQUAL(a(5), DT_(6));
    TEST_CHECK_EQUAL(a(6), DT_(8));

    Random::SeedType seed(Random::SeedType(time(nullptr)));
    std::cout << "seed: " << seed << std::endl;
    Random rng(seed);
    Adjacency::Permutation prm_rnd(a.size(), rng);
    SparseVector<DT_, IT_> ap(a.clone());
    ap.permute(prm_rnd);
    prm_rnd = prm_rnd.inverse();
    ap.permute(prm_rnd);
    TEST_CHECK_EQUAL(ap, a);
    TEST_CHECK_EQUAL(ap.used_elements(), Index(3));

    SparseVector<DT_, IT_> b;
    b.convert(a);
    TEST_CHECK_EQUAL(a, b);
    b(6, DT_(1));
    TEST_CHECK_NOT_EQUAL(a, b);
    b.clone(a);
    b(6, DT_(3));
    TEST_CHECK_NOT_EQUAL(a, b);
    TEST_CHECK_NOT_EQUAL((void*)a.elements(), (void*)b.elements());
    TEST_CHECK_NOT_EQUAL((void*)a.indices(), (void*)b.indices());
    b = a.clone();
    TEST_CHECK_NOT_EQUAL((void*)a.elements(), (void*)b.elements());
    TEST_CHECK_NOT_EQUAL((void*)a.indices(), (void*)b.indices());

    SparseVector<float, unsigned int> c;
    c.convert(a);
    SparseVector<float, unsigned int> d;
    d.clone(c);
    SparseVector<float, unsigned int> e;
    e.convert(a);
    TEST_CHECK_EQUAL(d, e);
    c(6, DT_(1));
    TEST_CHECK_NOT_EQUAL(c, e);

    a.format();
    TEST_CHECK_EQUAL(a.used_elements(), Index(3));
    TEST_CHECK_EQUAL(a(2), DT_(0));
    TEST_CHECK_EQUAL(a(3), DT_(0));

    //increase vector size above alloc_increment
    SparseVector<DT_, IT_> p(3001);
    for (Index i(1) ; i <= p.size() ; ++i)
    {
      p(p.size() - i, DT_(i));
    }
  }
};
SparseVectorTest<float, unsigned int> dv_test_float_uint(PreferredBackend::generic);
SparseVectorTest<double, unsigned int> cpu_sparse_vector_test_double_uint(PreferredBackend::generic);
SparseVectorTest<float, unsigned long> cpu_sparse_vector_test_float_ulong(PreferredBackend::generic);
SparseVectorTest<double, unsigned long> cpu_sparse_vector_test_double_ulong(PreferredBackend::generic);
#ifdef FEAT_HAVE_MKL
SparseVectorTest<float, unsigned long> mkl_cpu_sparse_vector_test_float_ulong(PreferredBackend::mkl);
SparseVectorTest<double, unsigned long> mkl_cpu_sparse_vector_test_double_ulong(PreferredBackend::mkl);
#endif
#ifdef FEAT_HAVE_QUADMATH
SparseVectorTest<__float128, unsigned long> cpu_sparse_vector_test_float128_ulong(PreferredBackend::generic);
SparseVectorTest<__float128, unsigned int> cpu_sparse_vector_test_float128_uint(PreferredBackend::generic);
#endif
#ifdef FEAT_HAVE_HALFMATH
SparseVectorTest<Half, unsigned int> sparse_vector_test_half_uint(PreferredBackend::generic);
SparseVectorTest<Half, unsigned long> sparse_vector_test_half_ulong(PreferredBackend::generic);
#endif
#ifdef FEAT_HAVE_CUDA
SparseVectorTest<float, unsigned int> cuda_sparse_vector_test_float_uint(PreferredBackend::cuda);
SparseVectorTest<double, unsigned int> cuda_sparse_vector_test_double_uint(PreferredBackend::cuda);
SparseVectorTest<float, unsigned long> cuda_sparse_vector_test_float_ulong(PreferredBackend::cuda);
SparseVectorTest<double, unsigned long> cuda_sparse_vector_test_double_ulong(PreferredBackend::cuda);
#endif

template<
  typename DT_,
  typename IT_>
class SparseVectorSerializeTest
  : public UnitTest
{
public:
  SparseVectorSerializeTest(PreferredBackend backend)
    : UnitTest("SparseVectorSerializeTest", Type::Traits<DT_>::name(), Type::Traits<IT_>::name(), backend)
  {
  }

  virtual ~SparseVectorSerializeTest()
  {
  }

  virtual void run() const override
  {
    SparseVector<DT_, IT_> a(10);
    a(3, DT_(7));
    a(3, DT_(3));
    a(6, DT_(1));
    a(5, DT_(6));
    a(6, DT_(8));

    std::stringstream ts;
    a.write_out(FileMode::fm_mtx, ts);
    SparseVector<DT_, IT_> j(FileMode::fm_mtx, ts);
    TEST_CHECK_EQUAL(j, a);

    BinaryStream bs;
    a.write_out(FileMode::fm_sv, bs);
    bs.seekg(0);
    SparseVector<DT_, IT_> bin(FileMode::fm_sv, bs);
    TEST_CHECK_EQUAL(bin, a);

    auto op = a.serialize(LAFEM::SerialConfig(false, false));
    SparseVector<DT_, IT_> o(op);
    for (Index i(0) ; i < a.size() ; ++i)
      TEST_CHECK_EQUAL_WITHIN_EPS(o(i), a(i), DT_(1e-5));
#ifdef FEAT_HAVE_ZLIB
    auto zl = a.serialize(LAFEM::SerialConfig(true, false));
    SparseVector<DT_, IT_> zlib(zl);
    for (Index i(0) ; i < a.size() ; ++i)
      TEST_CHECK_EQUAL_WITHIN_EPS(zlib(i), a(i), DT_(1e-5));
#endif
#ifdef FEAT_HAVE_ZFP
    auto zf = a.serialize(LAFEM::SerialConfig(false, true, FEAT::Real(1e-7)));
    SparseVector<DT_, IT_> zfp(zf);
    for (Index i(0) ; i < a.size() ; ++i)
      TEST_CHECK_EQUAL_WITHIN_EPS(zfp(i), a(i), DT_(1e-4));
#endif
  }
};
SparseVectorSerializeTest<float, unsigned int> cpu_sparse_vector_serialize_test_float_uint(PreferredBackend::generic);
SparseVectorSerializeTest<double, unsigned int> cpu_sparse_vector_serialize_test_double_uint(PreferredBackend::generic);
SparseVectorSerializeTest<float, unsigned long> cpu_sparse_vector_serialize_test_float_ulong(PreferredBackend::generic);
SparseVectorSerializeTest<double, unsigned long> cpu_sparse_vector_serialize_test_double_ulong(PreferredBackend::generic);
#ifdef FEAT_HAVE_MKL
SparseVectorSerializeTest<float, unsigned long> mkl_cpu_sparse_vector_serialize_test_float_ulong(PreferredBackend::mkl);
SparseVectorSerializeTest<double, unsigned long> mkl_cpu_sparse_vector_serialize_test_double_ulong(PreferredBackend::mkl);
#endif
#ifdef FEAT_HAVE_QUADMATH
SparseVectorSerializeTest<__float128, unsigned long> cpu_sparse_vector_serialize_test_float128_ulong(PreferredBackend::generic);
SparseVectorSerializeTest<__float128, unsigned int> cpu_sparse_vector_serialize_test_float128_uint(PreferredBackend::generic);
#endif
#ifdef FEAT_HAVE_HALFMATH
SparseVectorSerializeTest<Half, unsigned int> sparse_vector_serialize_test_half_uint(PreferredBackend::generic);
SparseVectorSerializeTest<Half, unsigned long> sparse_vector_serialize_test_half_ulong(PreferredBackend::generic);
#endif
#ifdef FEAT_HAVE_CUDA
SparseVectorSerializeTest<float, unsigned int> cuda_sparse_vector_serialize_test_float_uint(PreferredBackend::cuda);
SparseVectorSerializeTest<double, unsigned int> cuda_sparse_vector_serialize_test_double_uint(PreferredBackend::cuda);
SparseVectorSerializeTest<float, unsigned long> cuda_sparse_vector_serialize_test_float_ulong(PreferredBackend::cuda);
SparseVectorSerializeTest<double, unsigned long> cuda_sparse_vector_serialize_test_double_ulong(PreferredBackend::cuda);
#endif
