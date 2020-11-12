// FEAT3: Finite Element Analysis Toolbox, Version 3
// Copyright (C) 2010 - 2020 by Stefan Turek & the FEAT group
// FEAT3 is released under the GNU General Public License version 3,
// see the file 'copyright.txt' in the top level directory for details.

#include <test_system/test_system.hpp>
#include <kernel/base_header.hpp>
#include <kernel/archs.hpp>
#include <kernel/lafem/dense_vector.hpp>
#include <kernel/util/binary_stream.hpp>

#include <list>
#include <sstream>
#include <cstdio>

using namespace FEAT;
using namespace FEAT::LAFEM;
using namespace FEAT::TestSystem;

/**
 * \brief Test class for the dense vector class.
 *
 * \test test description missing
 *
 * \tparam Mem_
 * description missing
 *
 * \tparam DT_
 * description missing
 *
 * \author Dirk Ribbrock
 */
template<
  typename Mem_,
  typename DT_,
  typename IT_>
class DenseVectorTest
  : public FullTaggedTest<Mem_, DT_, IT_>
{
public:
  DenseVectorTest()
    : FullTaggedTest<Mem_, DT_, IT_>("DenseVectorTest")
  {
  }

  virtual ~DenseVectorTest()
  {
  }


  virtual void run() const override
  {
    DenseVector<Mem_, DT_, IT_> zero1;
    DenseVector<Mem::Main, DT_, IT_> zero2;
    TEST_CHECK_EQUAL(zero1, zero2);
    zero2.convert(zero1);

    if (typeid(Mem::CUDA) != typeid(Mem_))
    {
      DenseVector<Mem::Main, DT_, IT_> pinned(10, Pinning::enabled);
      pinned(5, DT_(42));
      TEST_CHECK_EQUAL(pinned(5), DT_(42));
    }

    DenseVector<Mem_, DT_, IT_> a(16, DT_(7)); //use multiple of 4 to circumanivate memory padding in MemoryPool
    TEST_CHECK_EQUAL(a.bytes(), 16 * sizeof(DT_) + 1 * sizeof(Index));
    TEST_CHECK_EQUAL(MemoryPool<Mem_>::allocated_memory(), a.bytes() - sizeof(Index));
    DenseVector<Mem_, DT_, IT_> b(16, DT_(5));
    b(7, DT_(42));
    TEST_CHECK_EQUAL(b(7), DT_(42));
    TEST_CHECK_EQUAL(b(3), DT_(5));

    DenseVector<Mem_, DT_, IT_> b_r(b, 5, 3);
    TEST_CHECK_EQUAL(b_r(0), b(0+3));
    TEST_CHECK_EQUAL(b_r(4), b(4+3));
    auto b_rc = b_r.clone();
    TEST_CHECK_EQUAL(b_rc(0), b(0+3));
    TEST_CHECK_EQUAL(b_rc(4), b(4+3));

    DenseVector<Mem_, DT_, IT_> c(b.clone());
    TEST_CHECK_EQUAL(c.size(), b.size());
    TEST_CHECK_EQUAL(c(7), b(7));
    TEST_CHECK_EQUAL(c, b);
    c.convert(b);
    TEST_CHECK_EQUAL(c.size(), b.size());
    TEST_CHECK_EQUAL(c(7), b(7));
    TEST_CHECK_EQUAL(c, b);
    DenseVector<Mem::Main, float, unsigned int> d;
    d.convert(c);
    DenseVector<Mem::Main, float, unsigned int> e;
    e.convert(b);
    TEST_CHECK_EQUAL(e.size(), d.size());
    TEST_CHECK_EQUAL(e(7), d(7));
    TEST_CHECK_EQUAL(e, d);
    e.clone(a);
    for (Index i(0) ; i < a.size() ; ++i)
      TEST_CHECK_EQUAL(DT_(e(i)), a(i));

    b.clone(a);
    TEST_CHECK_NOT_EQUAL((void*)b.elements(), (void*)a.elements());
    c.convert(a);
    TEST_CHECK_EQUAL((void*)c.elements(), (void*)a.elements());
    TEST_CHECK_EQUAL(b, c);

    DenseVector<Mem_, DT_, IT_> g(b.size(), b.elements());
    TEST_CHECK_EQUAL(g, b);
    TEST_CHECK_EQUAL((void*)g.elements(), (void*)b.elements());

    {
      EDI<Mem_, DT_> t(a.edi(2));
      t = DT_(41);
      TEST_CHECK_NOT_EQUAL(a(2), DT_(41));
      a.edi(1) = DT_(4);
      TEST_CHECK_EQUAL(a(1), DT_(4));
      a.edi(1) += DT_(4);
      TEST_CHECK_EQUAL(a(1), DT_(8));
    }
    TEST_CHECK_EQUAL(a(1), DT_(8));
    TEST_CHECK_EQUAL(a(2), DT_(41));

    DenseVector<Mem_, DT_, IT_> ap(a.clone());
    Adjacency::Permutation prm_nil;
    ap.permute(prm_nil);
    Random::SeedType seed(Random::SeedType(time(nullptr)));
    std::cout << "seed: " << seed << std::endl;
    Random rng(seed);
    Adjacency::Permutation prm_rnd(a.size(), rng);
    ap.permute(prm_rnd);
    prm_rnd = prm_rnd.inverse();
    ap.permute(prm_rnd);
    TEST_CHECK_EQUAL(ap, a);

    // random constructor check
    DT_ rnd_range[2];
    IT_ rnd_size = 1234;
    rnd_range[0] = DT_(-10);
    rnd_range[1] = DT_(+10);
    DenseVector<Mem_, DT_, IT_> rnd_vec(rng, rnd_size, rnd_range[0], rnd_range[1]);
    TEST_CHECK_EQUAL(rnd_vec.size(), rnd_size);
    DT_ rnd_max = rnd_vec.max_abs_element();
    TEST_CHECK_IN_RANGE(rnd_max, rnd_range[0], rnd_range[1]);
    rnd_vec.scale(rnd_vec, DT_(-1));
    DT_ rnd_min = -rnd_vec.max_abs_element();
    TEST_CHECK_IN_RANGE(rnd_min, rnd_range[0], rnd_range[1]);

    // new clone testing
    auto clone1 = a.clone(CloneMode::Deep);
    TEST_CHECK_EQUAL(clone1, a);
    clone1(7, DT_(132));
    TEST_CHECK_NOT_EQUAL(clone1, a);
    TEST_CHECK_NOT_EQUAL((void*)clone1.elements(), (void*)a.elements());
    DenseVector<Mem_, DT_, IT_> clone2 = clone1.clone(CloneMode::Layout);
    MemoryPool<Mem_>::set_memory(clone2.elements(), DT_(4713), clone2.size());
    TEST_CHECK_NOT_EQUAL(clone2(7), clone1(7));
    TEST_CHECK_NOT_EQUAL((void*)clone2.elements(), (void*)clone1.elements());
    DenseVector<Mem_, DT_, IT_> clone3 = clone1.clone(CloneMode::Weak);
    TEST_CHECK_EQUAL(clone3, clone1);
    clone3(7, DT_(133));
    TEST_CHECK_NOT_EQUAL(clone3, clone1);
    TEST_CHECK_NOT_EQUAL((void*)clone3.elements(), (void*)clone1.elements());
    DenseVector<Mem_, DT_, IT_> clone4 = clone1.clone(CloneMode::Shallow);
    TEST_CHECK_EQUAL(clone4, clone1);
    clone4(7, DT_(134));
    TEST_CHECK_EQUAL(clone4, clone1);
    TEST_CHECK_EQUAL((void*)clone4.elements(), (void*)clone1.elements());
    auto clone5 = a.clone(CloneMode::Allocate);
    TEST_CHECK_NOT_EQUAL((void*)clone5.elements(), (void*)a.elements());
    TEST_CHECK_EQUAL(clone5.size(), a.size());
  }
};
DenseVectorTest<Mem::Main, float, unsigned int> cpu_dense_vector_test_float_uint;
DenseVectorTest<Mem::Main, double, unsigned int> cpu_dense_vector_test_double_uint;
DenseVectorTest<Mem::Main, float, unsigned long> cpu_dense_vector_test_float_ulong;
DenseVectorTest<Mem::Main, double, unsigned long> cpu_dense_vector_test_double_ulong;
#ifdef FEAT_HAVE_QUADMATH
DenseVectorTest<Mem::Main, __float128, unsigned int> cpu_dense_vector_test_float128_uint;
DenseVectorTest<Mem::Main, __float128, unsigned long> cpu_dense_vector_test_float128_ulong;
#endif
#ifdef FEAT_HAVE_HALFMATH
DenseVectorTest<Mem::Main, half_float::half, unsigned int> cpu_dense_vector_test_half_uint;
DenseVectorTest<Mem::Main, half_float::half, unsigned long> cpu_dense_vector_test_half_ulong;
#endif
#ifdef FEAT_HAVE_CUDA
DenseVectorTest<Mem::CUDA, float, unsigned int> cuda_dense_vector_test_float_uint;
DenseVectorTest<Mem::CUDA, double, unsigned int> cuda_dense_vector_test_double_uint;
DenseVectorTest<Mem::CUDA, float, unsigned long> cuda_dense_vector_test_float_ulong;
DenseVectorTest<Mem::CUDA, double, unsigned long> cuda_dense_vector_test_double_ulong;
#endif

template<
  typename Mem_,
  typename DT_,
  typename IT_>
class DenseVectorSerializeTest
  : public FullTaggedTest<Mem_, DT_, IT_>
{
public:
  DenseVectorSerializeTest()
    : FullTaggedTest<Mem_, DT_, IT_>("DenseVectorSerializeTest")
  {
  }

  virtual ~DenseVectorSerializeTest()
  {
  }


  virtual void run() const override
  {
    Index io_vector_size = 1234;

    DenseVector<Mem_, DT_, IT_> k(io_vector_size);
    for (Index i(0) ; i < k.size() ; ++i)
      k(i, DT_(i) / DT_(12));

    {
      std::stringstream mts;
      k.write_out(FileMode::fm_mtx, mts);
      DenseVector<Mem_, DT_, IT_> l(FileMode::fm_mtx, mts);
      for (Index i(0) ; i < k.size() ; ++i)
        TEST_CHECK_EQUAL_WITHIN_EPS(l(i), k(i), DT_(1e-4));
    }

    {
      std::stringstream ts;
      k.write_out(FileMode::fm_exp, ts);
      DenseVector<Mem_, DT_, IT_> m(FileMode::fm_exp, ts);
      for (Index i(0) ; i < k.size() ; ++i)
        TEST_CHECK_EQUAL_WITHIN_EPS(m(i), k(i), DT_(1e-4));
    }

    {
      BinaryStream bs;
      k.write_out(FileMode::fm_dv, bs);
      bs.seekg(0);
      DenseVector<Mem_, DT_, IT_> n(FileMode::fm_dv, bs);
      for (Index i(0) ; i < k.size() ; ++i)
        TEST_CHECK_EQUAL_WITHIN_EPS(n(i), k(i), DT_(1e-5));
    }

    {
      auto op = k.serialize(LAFEM::SerialConfig(false,false));
      DenseVector<Mem_, DT_, IT_> o(op);
      for (Index i(0) ; i < k.size() ; ++i)
        TEST_CHECK_EQUAL_WITHIN_EPS(o(i), k(i), DT_(1e-5));
#ifdef FEAT_HAVE_ZLIB
      auto zb = k.serialize(LAFEM::SerialConfig(true,false));
      DenseVector<Mem_, DT_, IT_> zlib(zb);
      for (Index i(0) ; i < k.size() ; ++i)
        TEST_CHECK_EQUAL_WITHIN_EPS(zlib(i), k(i), DT_(1e-5));
#endif
#ifdef FEAT_HAVE_ZFP
      auto zp = k.serialize(LAFEM::SerialConfig(false, true, FEAT::Real(1e-5)));
      DenseVector<Mem_, DT_, IT_> zfp(zp);
      for (Index i(0) ; i < k.size() ; ++i)
        TEST_CHECK_EQUAL_WITHIN_EPS(zfp(i), k(i), DT_(1e-5));
#endif
    }
  }
};

DenseVectorSerializeTest<Mem::Main, float, unsigned int> cpu_dense_vector_serialize_test_float_uint;
DenseVectorSerializeTest<Mem::Main, double, unsigned int> cpu_dense_vector_serialize_test_double_uint;
DenseVectorSerializeTest<Mem::Main, float, unsigned long> cpu_dense_vector_serialize_test_float_ulong;
DenseVectorSerializeTest<Mem::Main, double, unsigned long> cpu_dense_vector_serialize_test_double_ulong;
#ifdef FEAT_HAVE_CUDA
DenseVectorSerializeTest<Mem::CUDA, float, unsigned int> cuda_dense_vector_serialize_test_float_uint;
DenseVectorSerializeTest<Mem::CUDA, double, unsigned int> cuda_dense_vector_serialize_test_double_uint;
DenseVectorSerializeTest<Mem::CUDA, float, unsigned long> cuda_dense_vector_serialize_test_float_ulong;
DenseVectorSerializeTest<Mem::CUDA, double, unsigned long> cuda_dense_vector_serialize_test_double_ulong;
#endif

template<
  typename Mem_,
  typename DT_,
  typename IT_>
class DenseVectorAxpyTest
  : public FullTaggedTest<Mem_, DT_, IT_>
{
public:
  DenseVectorAxpyTest()
    : FullTaggedTest<Mem_, DT_, IT_>("DenseVectorAxpyTest")
  {
  }

  virtual ~DenseVectorAxpyTest()
  {
  }

  virtual void run() const override
  {
    DT_ s(DT_(4711.1));
    for (Index size(1) ; size < Index(1e3) ; size*=2)
    {
      DenseVector<Mem::Main, DT_, IT_> a_local(size);
      DenseVector<Mem::Main, DT_, IT_> b_local(size);
      DenseVector<Mem::Main, DT_, IT_> ref(size);
      DenseVector<Mem::Main, DT_, IT_> result_local(size);
      for (Index i(0) ; i < size ; ++i)
      {
        a_local(i, DT_(i % 100 * DT_(1.234)));
        b_local(i, DT_(2 - DT_(i % 42)));
        ref(i, s * a_local(i) + b_local(i));
      }
      DenseVector<Mem_, DT_, IT_> a(size);
      a.copy(a_local);
      DenseVector<Mem_, DT_, IT_> b(size);
      b.copy(b_local);

      DenseVector<Mem_, DT_, IT_> c(size);
      c.axpy(a, b, s);
      result_local.copy(c);
      for (Index i(0) ; i < size ; ++i)
        TEST_CHECK_EQUAL_WITHIN_EPS(result_local(i), ref(i), DT_(1e-2));

      a.axpy(a, b, s);
      result_local.copy(a);
      for (Index i(0) ; i < size ; ++i)
        TEST_CHECK_EQUAL_WITHIN_EPS(result_local(i), ref(i), DT_(1e-2));

      a.copy(a_local);
      b.axpy(a, b, s);
      result_local.copy(b);
      for (Index i(0) ; i < size ; ++i)
        TEST_CHECK_EQUAL_WITHIN_EPS(result_local(i), ref(i), DT_(1e-2));
    }
  }
};
DenseVectorAxpyTest<Mem::Main, float, unsigned int> dv_axpy_test_float_uint;
DenseVectorAxpyTest<Mem::Main, double, unsigned int> dv_axpy_test_double_uint;
DenseVectorAxpyTest<Mem::Main, float, unsigned long> dv_axpy_test_float_ulong;
DenseVectorAxpyTest<Mem::Main, double, unsigned long> dv_axpy_test_double_ulong;
#ifdef FEAT_HAVE_QUADMATH
DenseVectorAxpyTest<Mem::Main, __float128, unsigned int> dv_axpy_test_float128_uint;
DenseVectorAxpyTest<Mem::Main, __float128, unsigned long> dv_axpy_test_float128_ulong;
#endif
#ifdef FEAT_HAVE_CUDA
DenseVectorAxpyTest<Mem::CUDA, float, unsigned int> cuda_dv_axpy_test_float_uint;
DenseVectorAxpyTest<Mem::CUDA, double, unsigned int> cuda_dv_axpy_test_double_uint;
DenseVectorAxpyTest<Mem::CUDA, float, unsigned long> cuda_dv_axpy_test_float_ulong;
DenseVectorAxpyTest<Mem::CUDA, double, unsigned long> cuda_dv_axpy_test_double_ulong;
#endif

template<
  typename Mem_,
  typename DT_,
  typename IT_>
class DenseVectorDotTest
  : public FullTaggedTest<Mem_, DT_, IT_>
{
public:
  DenseVectorDotTest()
    : FullTaggedTest<Mem_, DT_, IT_>("DenseVectorDotTest")
  {
  }

  virtual ~DenseVectorDotTest()
  {
  }

  virtual void run() const override
  {
    const DT_ eps = Math::pow(Math::eps<DT_>(), DT_(0.8));

    for (Index size(1) ; size < Index(1e3) ; size*=2)
    {
      DenseVector<Mem::Main, DT_, IT_> a_local(size);
      DenseVector<Mem::Main, DT_, IT_> b_local(size);
      const DT_ den(DT_(1) / DT_(size));
      for (Index i(0) ; i < size ; ++i)
      {
        a_local(i, DT_(i+1) * den);    // a[i] = (i+1) / n
        b_local(i, DT_(1) / DT_(i+1)); // b[i] = 1 / (i+1)
      }

      DenseVector<Mem_, DT_, IT_> a;
      a.convert(a_local);
      DenseVector<Mem_, DT_, IT_> b;
      b.convert(b_local);

      // a*b = 1
      DT_ ref(DT_(1));
      DT_ c  = a.dot(b);
      TEST_CHECK_EQUAL_WITHIN_EPS(c, ref, eps);
      c = b.dot(a);
      TEST_CHECK_EQUAL_WITHIN_EPS(c, ref, eps);
      c = b.dot(b);
      ref = b.norm2();
      ref *= ref;
      TEST_CHECK_EQUAL_WITHIN_EPS(c, ref, eps);
    }
  }
};
DenseVectorDotTest<Mem::Main, float, unsigned int> dv_dot_product_test_float_uint;
DenseVectorDotTest<Mem::Main, double, unsigned int> dv_dot_product_test_double_uint;
DenseVectorDotTest<Mem::Main, float, unsigned long> dv_dot_product_test_float_ulong;
DenseVectorDotTest<Mem::Main, double, unsigned long> dv_dot_product_test_double_ulong;
#ifdef FEAT_HAVE_QUADMATH
DenseVectorDotTest<Mem::Main, __float128, unsigned int> dv_dot_product_test_float128_uint;
DenseVectorDotTest<Mem::Main, __float128, unsigned long> dv_dot_product_test_float128_ulong;
#endif
#ifdef FEAT_HAVE_CUDA
DenseVectorDotTest<Mem::CUDA, float, unsigned int> cuda_dv_dot_product_test_float_uint;
DenseVectorDotTest<Mem::CUDA, double, unsigned int> cuda_dv_dot_product_test_double_uint;
DenseVectorDotTest<Mem::CUDA, float, unsigned long> cuda_dv_dot_product_test_float_ulong;
DenseVectorDotTest<Mem::CUDA, double, unsigned long> cuda_dv_dot_product_test_double_ulong;
#endif


/**
 * \brief DenseVector triple_dot test class
 *
 * \test The triple_dot routines
 *
 * \author Jordi Paul
 *
 **/

template<
  typename Mem_,
  typename DT_,
  typename IT_>
class DenseVectorTripleDotTest
  : public FullTaggedTest<Mem_, DT_, IT_>
{
public:
  DenseVectorTripleDotTest()
    : FullTaggedTest<Mem_, DT_, IT_>("DenseVectorTripleDotTest")
  {
  }

  virtual ~DenseVectorTripleDotTest()
  {
  }


  virtual void run() const override
  {
    const DT_ eps = Math::pow(Math::eps<DT_>(), DT_(0.7));

    for (Index size(1) ; size < Index(1e3) ; size*=2)
    {
      DenseVector<Mem::Main, DT_, IT_> a_local(size);
      DenseVector<Mem::Main, DT_, IT_> b_local(size);
      DenseVector<Mem::Main, DT_, IT_> c_local(size);

      const DT_ den( DT_(1) / Math::sqrt(DT_(size)) );

      for (Index i(0) ; i < size ; ++i)
      {
        a_local(i, DT_(i+1) * den);    // a[i] = (i+1) / n
        b_local(i, DT_(1) / DT_(i+1)); // b[i] = 1 / (i+1)
        c_local(i, den);
      }

      DenseVector<Mem_, DT_, IT_> a;
      a.convert(a_local);

      DenseVector<Mem_, DT_, IT_> b;
      b.convert(b_local);

      DenseVector<Mem_, DT_, IT_> c;
      c.convert(c_local);

      // a^T diag(c) b = 1
      DT_ ref(DT_(1));

      DT_ res  = a.triple_dot(b,c);
      TEST_CHECK_EQUAL_WITHIN_EPS(res, ref, eps);
      res  = a.triple_dot(c,b);
      TEST_CHECK_EQUAL_WITHIN_EPS(res, ref, eps);

      res = b.triple_dot(a,c);
      TEST_CHECK_EQUAL_WITHIN_EPS(res, ref, eps);
      res = b.triple_dot(c,a);
      TEST_CHECK_EQUAL_WITHIN_EPS(res, ref, eps);

      res = c.triple_dot(a,b);
      TEST_CHECK_EQUAL_WITHIN_EPS(res, ref, eps);
      res = c.triple_dot(b,a);
      TEST_CHECK_EQUAL_WITHIN_EPS(res, ref, eps);

    }
  }
};
DenseVectorTripleDotTest<Mem::Main, float, unsigned int> dv_triple_dot_product_test_float_uint;
DenseVectorTripleDotTest<Mem::Main, double, unsigned int> dv_triple_dot_product_test_double_uint;
DenseVectorTripleDotTest<Mem::Main, float, unsigned long> dv_triple_dot_product_test_float_ulong;
DenseVectorTripleDotTest<Mem::Main, double, unsigned long> dv_triple_dot_product_test_double_ulong;
#ifdef FEAT_HAVE_QUADMATH
DenseVectorTripleDotTest<Mem::Main, __float128, unsigned int> dv_triple_dot_product_test_float128_uint;
DenseVectorTripleDotTest<Mem::Main, __float128, unsigned long> dv_triple_dot_product_test_float128_ulong;
#endif
#ifdef FEAT_HAVE_CUDA
DenseVectorTripleDotTest<Mem::CUDA, float, unsigned int> cuda_dv_triple_dot_product_test_float_uint;
DenseVectorTripleDotTest<Mem::CUDA, double, unsigned int> cuda_dv_triple_dot_product_test_double_uint;
DenseVectorTripleDotTest<Mem::CUDA, float, unsigned long> cuda_dv_triple_dot_product_test_float_ulong;
DenseVectorTripleDotTest<Mem::CUDA, double, unsigned long> cuda_dv_triple_dot_product_test_double_ulong;
#endif


template<
  typename Mem_,
  typename DT_,
  typename IT_>
class DenseVectorComponentProductTest
  : public FullTaggedTest<Mem_, DT_, IT_>
{
public:
  DenseVectorComponentProductTest()
    : FullTaggedTest<Mem_, DT_, IT_>("DenseVectorComponentProductTest")
  {
  }

  virtual ~DenseVectorComponentProductTest()
  {
  }

  void run1() const
  {
    for (Index size(1) ; size < Index(1e3) ; size*=2)
    {
      DenseVector<Mem::Main, DT_, IT_> a_local(size);
      DenseVector<Mem::Main, DT_, IT_> b_local(size);
      DenseVector<Mem::Main, DT_, IT_> ref(size);
      DenseVector<Mem::Main, DT_, IT_> ref2(size);
      DenseVector<Mem::Main, DT_, IT_> result_local(size);
      for (Index i(0) ; i < size ; ++i)
      {
        a_local(i, DT_(DT_(i) * DT_(1.234)));
        b_local(i, DT_(size*2 - i));
        ref(i, a_local(i) * b_local(i));
        ref2(i, a_local(i) * a_local(i));
      }

      DenseVector<Mem_, DT_, IT_> a(size);
      a.copy(a_local);
      DenseVector<Mem_, DT_, IT_> b(size);
      b.copy(b_local);
      DenseVector<Mem_, DT_, IT_> c(size);

      c.component_product(a, b);
      result_local.copy(c);
      TEST_CHECK_EQUAL(result_local, ref);

      a.component_product(a, b);
      result_local.copy(a);
      TEST_CHECK_EQUAL(result_local, ref);

      a.copy(a_local);
      b.component_product(a, b);
      result_local.copy(b);
      TEST_CHECK_EQUAL(result_local, ref);

      b.copy(b_local);
      a.component_product(a, a);
      result_local.copy(a);
      TEST_CHECK_EQUAL(result_local, ref2);
    }
  }

  virtual void run() const override
  {
    run1();
  }
};
DenseVectorComponentProductTest<Mem::Main, float, unsigned int> dv_component_product_test_float_uint;
DenseVectorComponentProductTest<Mem::Main, double, unsigned int> dv_component_product_test_double_uint;
DenseVectorComponentProductTest<Mem::Main, float, unsigned long> dv_component_product_test_float_ulong;
DenseVectorComponentProductTest<Mem::Main, double, unsigned long> dv_component_product_test_double_ulong;
#ifdef FEAT_HAVE_QUADMATH
DenseVectorComponentProductTest<Mem::Main, __float128, unsigned int> dv_component_product_test_float128_uint;
DenseVectorComponentProductTest<Mem::Main, __float128, unsigned long> dv_component_product_test_float128_ulong;
#endif
#ifdef FEAT_HAVE_CUDA
DenseVectorComponentProductTest<Mem::CUDA, float, unsigned int> cuda_dv_component_product_test_float_uint;
DenseVectorComponentProductTest<Mem::CUDA, double, unsigned int> cuda_dv_component_product_test_double_uint;
DenseVectorComponentProductTest<Mem::CUDA, float, unsigned long> cuda_dv_component_product_test_float_ulong;
DenseVectorComponentProductTest<Mem::CUDA, double, unsigned long> cuda_dv_component_product_test_double_ulong;
#endif


template<
  typename Mem_,
  typename DT_,
  typename IT_>
class DenseVectorScaleTest
  : public FullTaggedTest<Mem_, DT_, IT_>
{
public:
  DenseVectorScaleTest()
    : FullTaggedTest<Mem_, DT_, IT_>("DenseVectorScaleTest")
  {
  }

  virtual ~DenseVectorScaleTest()
  {
  }

  virtual void run() const override
  {
    for (Index size(1) ; size < Index(1e3) ; size*=2)
    {
      DT_ s(DT_(4.321));
      DenseVector<Mem::Main, DT_, IT_> a_local(size);
      DenseVector<Mem::Main, DT_, IT_> ref(size);
      DenseVector<Mem::Main, DT_, IT_> result_local(size);
      for (Index i(0) ; i < size ; ++i)
      {
        a_local(i, DT_(DT_(i) * DT_(1.234)));
        ref(i, a_local(i) * s);
      }

      DenseVector<Mem_, DT_, IT_> a(size);
      a.copy(a_local);
      DenseVector<Mem_, DT_, IT_> b(size);

      b.scale(a, s);
      result_local.copy(b);
      TEST_CHECK_EQUAL(result_local, ref);

      a.scale(a, s);
      result_local.copy(a);
      TEST_CHECK_EQUAL(result_local, ref);
    }
  }
};
DenseVectorScaleTest<Mem::Main, float, unsigned int> dv_scale_test_float_uint;
DenseVectorScaleTest<Mem::Main, double, unsigned int> dv_scale_test_double_uint;
DenseVectorScaleTest<Mem::Main, float, unsigned long> dv_scale_test_float_ulong;
DenseVectorScaleTest<Mem::Main, double, unsigned long> dv_scale_test_double_ulong;
#ifdef FEAT_HAVE_QUADMATH
DenseVectorScaleTest<Mem::Main, __float128, unsigned int> dv_scale_test_float128_uint;
DenseVectorScaleTest<Mem::Main, __float128, unsigned long> dv_scale_test_float128_ulong;
#endif
#ifdef FEAT_HAVE_CUDA
DenseVectorScaleTest<Mem::CUDA, float, unsigned int> cuda_dv_scale_test_float_uint;
DenseVectorScaleTest<Mem::CUDA, double, unsigned int> cuda_dv_scale_test_double_uint;
DenseVectorScaleTest<Mem::CUDA, float, unsigned long> cuda_dv_scale_test_float_ulong;
DenseVectorScaleTest<Mem::CUDA, double, unsigned long> cuda_dv_scale_test_double_ulong;
#endif


template<
  typename Mem_,
  typename DT_,
  typename IT_>
class DenseVectorNorm2Test
  : public FullTaggedTest<Mem_, DT_, IT_>
{
public:
  DenseVectorNorm2Test()
    : FullTaggedTest<Mem_, DT_, IT_>("DenseVectorNorm2Test")
  {
  }

  virtual ~DenseVectorNorm2Test()
  {
  }

  virtual void run() const override
  {
    const DT_ eps = Math::pow(Math::eps<DT_>(), DT_(0.8));

    for (Index size(1) ; size < Index(1e3) ; size*=2)
    {
      DenseVector<Mem::Main, DT_, IT_> a_local(size);
      for (Index i(0) ; i < size ; ++i)
      {
        // a[i] = 1/sqrt(2^i) = (1/2)^(i/2)
        a_local(i, Math::pow(DT_(0.5), DT_(0.5) * DT_(i)));
      }

      // ||a||_2 = sqrt(2 - 2^{1-n})
      const DT_ ref(Math::sqrt(DT_(2) - Math::pow(DT_(0.5), DT_(size-1))));

      DenseVector<Mem_, DT_, IT_> a;
      a.convert(a_local);
      DT_ c = a.norm2();
      TEST_CHECK_EQUAL_WITHIN_EPS(c, ref, eps);

      c = a.norm2sqr();
      TEST_CHECK_EQUAL_WITHIN_EPS(c, ref*ref, eps);
    }
  }
};
DenseVectorNorm2Test<Mem::Main, float, unsigned int> dv_norm2_test_float_uint;
DenseVectorNorm2Test<Mem::Main, double, unsigned int> dv_norm2_test_double_uint;
DenseVectorNorm2Test<Mem::Main, float, unsigned long> dv_norm2_test_float_ulong;
DenseVectorNorm2Test<Mem::Main, double, unsigned long> dv_norm2_test_double_ulong;
#ifdef FEAT_HAVE_QUADMATH
DenseVectorNorm2Test<Mem::Main, __float128, unsigned int> dv_norm2_test_float128_uint;
DenseVectorNorm2Test<Mem::Main, __float128, unsigned long> dv_norm2_test_float128_ulong;
#endif
#ifdef FEAT_HAVE_CUDA
DenseVectorNorm2Test<Mem::CUDA, float, unsigned int> cuda_dv_norm2_test_float_uint;
DenseVectorNorm2Test<Mem::CUDA, double, unsigned int> cuda_dv_norm2_test_double_uint;
DenseVectorNorm2Test<Mem::CUDA, float, unsigned long> cuda_dv_norm2_test_float_ulong;
DenseVectorNorm2Test<Mem::CUDA, double, unsigned long> cuda_dv_norm2_test_double_ulong;
#endif

template<
  typename Mem_,
  typename DT_,
  typename IT_>
class DenseVectorComponentInvertTest
  : public FullTaggedTest<Mem_, DT_, IT_>
{
public:
  typedef DenseVector<Mem_, DT_, IT_> VectorType;

  DenseVectorComponentInvertTest()
    : FullTaggedTest<Mem_, DT_, IT_>("DenseVectorComponentInvertTest")
  {
  }

  virtual ~DenseVectorComponentInvertTest()
  {
  }

  virtual void run() const override
  {
    const DT_ eps = Math::pow(Math::eps<DT_>(), DT_(0.8));
    const DT_ alpha(Math::pi<DT_>());

    for (Index size(1) ; size < Index(1e3) ; size*=2)
    {
      // create a vector
      DenseVector<Mem::Main, DT_, IT_> tvec(size);
      for (Index i(0); i < size; ++i)
      {
        tvec(i, DT_(7.63) * DT_(i % 3 + 1) - DT_(9.3));
      }
      VectorType vec;
      vec.convert(tvec);

      VectorType vec2(vec.clone());
      vec2.component_invert(vec2, alpha);
      vec2.component_product(vec2, vec);
      for (Index i(0); i < size; ++i)
      {
        TEST_CHECK_EQUAL_WITHIN_EPS(vec2(i), alpha, eps);
      }

      VectorType vec3(size);
      vec3.component_invert(vec);
      for (Index i(0); i < size; ++i)
      {
        TEST_CHECK_EQUAL_WITHIN_EPS(vec3(i), DT_(1.0) / vec(i), eps);
      }
    }
  }
};

DenseVectorComponentInvertTest<Mem::Main, float, Index> dv_component_invert_test_float;
DenseVectorComponentInvertTest<Mem::Main, double, Index> dv_component_invert_test_double;
#ifdef FEAT_HAVE_QUADMATH
DenseVectorComponentInvertTest<Mem::Main, __float128, Index> dv_component_invert_test_float128;
#endif
#ifdef FEAT_HAVE_CUDA
DenseVectorComponentInvertTest<Mem::CUDA, float, Index> cuda_dv_component_invert_test_float;
DenseVectorComponentInvertTest<Mem::CUDA, double, Index> cuda_dv_component_invert_test_double;
#endif


template<
  typename Mem_,
  typename DT_,
  typename IT_>
class DenseVectorMaxAbsElementTest
  : public FullTaggedTest<Mem_, DT_, IT_>
{
public:
  DenseVectorMaxAbsElementTest()
    : FullTaggedTest<Mem_, DT_, IT_>("DenseVectorMaxAbsElementTest")
  {
  }

  virtual ~DenseVectorMaxAbsElementTest()
  {
  }

  virtual void run() const override
  {
    for (Index size(1) ; size < Index(1e3) ; size*=2)
    {
      DenseVector<Mem::Main, DT_, IT_> a_local(size);
      for (Index i(0) ; i < size ; ++i)
      {
        a_local(i, DT_(i) * (i%2 == 0 ? DT_(1) : DT_(-1)));
      }

      DenseVector<Mem_, DT_, IT_> a;
      a.convert(a_local);
      Random::SeedType seed(Random::SeedType(time(nullptr)));
      std::cout << "seed: " << seed << std::endl;
      Random rng(seed);
      Adjacency::Permutation prm_rnd(a.size(), rng);
      a.permute(prm_rnd);

      DT_ max = a.max_abs_element();

      TEST_CHECK_EQUAL(max, DT_(size-1));
    }
  }
};
DenseVectorMaxAbsElementTest<Mem::Main, float, unsigned int> dv_max_abs_element_test_float_uint;
DenseVectorMaxAbsElementTest<Mem::Main, double, unsigned int> dv_max_abs_element_test_double_uint;
DenseVectorMaxAbsElementTest<Mem::Main, float, unsigned long> dv_max_abs_element_test_float_ulong;
DenseVectorMaxAbsElementTest<Mem::Main, double, unsigned long> dv_max_abs_element_test_double_ulong;
#ifdef FEAT_HAVE_QUADMATH
DenseVectorMaxAbsElementTest<Mem::Main, __float128, unsigned int> dv_max_abs_element_test_float128_uint;
DenseVectorMaxAbsElementTest<Mem::Main, __float128, unsigned long> dv_max_abs_element_test_float128_ulong;
#endif
#ifdef FEAT_HAVE_CUDA
DenseVectorMaxAbsElementTest<Mem::CUDA, float, unsigned int> cuda_dv_max_abs_element_test_float_uint;
DenseVectorMaxAbsElementTest<Mem::CUDA, double, unsigned int> cuda_dv_max_abs_element_test_double_uint;
DenseVectorMaxAbsElementTest<Mem::CUDA, float, unsigned long> cuda_dv_max_abs_element_test_float_ulong;
DenseVectorMaxAbsElementTest<Mem::CUDA, double, unsigned long> cuda_dv_max_abs_element_test_double_ulong;
#endif

template<
  typename Mem_,
  typename DT_,
  typename IT_>
class DenseVectorMinAbsElementTest
  : public FullTaggedTest<Mem_, DT_, IT_>
{
public:
  DenseVectorMinAbsElementTest()
    : FullTaggedTest<Mem_, DT_, IT_>("DenseVectorMinAbsElementTest")
  {
  }

  virtual ~DenseVectorMinAbsElementTest()
  {
  }

  virtual void run() const override
  {
    for (Index size(1) ; size < Index(1e3) ; size*=2)
    {
      DenseVector<Mem::Main, DT_, IT_> a_local(size);
      for (Index i(0) ; i < size ; ++i)
      {
        a_local(i, DT_(i) * (i%2 == 0 ? DT_(1) : DT_(-1)));
      }

      DenseVector<Mem_, DT_, IT_> a;
      a.convert(a_local);
      Random::SeedType seed(Random::SeedType(time(nullptr)));
      std::cout << "seed: " << seed << std::endl;
      Random rng(seed);
      Adjacency::Permutation prm_rnd(a.size(), rng);
      a.permute(prm_rnd);

      DT_ min = a.min_abs_element();

      TEST_CHECK_EQUAL(min, DT_(0));
    }
  }
};
DenseVectorMinAbsElementTest<Mem::Main, float, unsigned int> dv_min_abs_element_test_float_uint;
DenseVectorMinAbsElementTest<Mem::Main, double, unsigned int> dv_min_abs_element_test_double_uint;
DenseVectorMinAbsElementTest<Mem::Main, float, unsigned long> dv_min_abs_element_test_float_ulong;
DenseVectorMinAbsElementTest<Mem::Main, double, unsigned long> dv_min_abs_element_test_double_ulong;
#ifdef FEAT_HAVE_QUADMATH
DenseVectorMinAbsElementTest<Mem::Main, __float128, unsigned int> dv_min_abs_element_test_float128_uint;
DenseVectorMinAbsElementTest<Mem::Main, __float128, unsigned long> dv_min_abs_element_test_float128_ulong;
#endif
#ifdef FEAT_HAVE_CUDA
DenseVectorMinAbsElementTest<Mem::CUDA, float, unsigned int> cuda_dv_min_abs_element_test_float_uint;
DenseVectorMinAbsElementTest<Mem::CUDA, double, unsigned int> cuda_dv_min_abs_element_test_double_uint;
DenseVectorMinAbsElementTest<Mem::CUDA, float, unsigned long> cuda_dv_min_abs_element_test_float_ulong;
DenseVectorMinAbsElementTest<Mem::CUDA, double, unsigned long> cuda_dv_min_abs_element_test_double_ulong;
#endif


template<
  typename Mem_,
  typename DT_,
  typename IT_>
class DenseVectorMaxElementTest
  : public FullTaggedTest<Mem_, DT_, IT_>
{
public:
  DenseVectorMaxElementTest()
    : FullTaggedTest<Mem_, DT_, IT_>("DenseVectorMaxElementTest")
  {
  }

  virtual ~DenseVectorMaxElementTest()
  {
  }

  virtual void run() const override
  {
    for (Index size(5) ; size < Index(1e3) ; size*=2)
    {
      DenseVector<Mem::Main, DT_, IT_> a_local(size);
      for (Index i(0) ; i < size ; ++i)
      {
        a_local(i, DT_(i));
      }
      a_local(0, DT_(-5));

      DenseVector<Mem_, DT_, IT_> a;
      a.convert(a_local);
      Random::SeedType seed(Random::SeedType(time(nullptr)));
      std::cout << "seed: " << seed << std::endl;
      Random rng(seed);
      Adjacency::Permutation prm_rnd(a.size(), rng);
      a.permute(prm_rnd);

      DT_ max = a.max_element();

      TEST_CHECK_EQUAL(max, DT_(size-1));
    }
  }
};
DenseVectorMaxElementTest<Mem::Main, float, unsigned int> dv_max_element_test_float_uint;
DenseVectorMaxElementTest<Mem::Main, double, unsigned int> dv_max_element_test_double_uint;
DenseVectorMaxElementTest<Mem::Main, float, unsigned long> dv_max_element_test_float_ulong;
DenseVectorMaxElementTest<Mem::Main, double, unsigned long> dv_max_element_test_double_ulong;
#ifdef FEAT_HAVE_QUADMATH
DenseVectorMaxElementTest<Mem::Main, __float128, unsigned int> dv_max_element_test_float128_uint;
DenseVectorMaxElementTest<Mem::Main, __float128, unsigned long> dv_max_element_test_float128_ulong;
#endif


template<
  typename Mem_,
  typename DT_,
  typename IT_>
class DenseVectorMinElementTest
  : public FullTaggedTest<Mem_, DT_, IT_>
{
public:
  DenseVectorMinElementTest()
    : FullTaggedTest<Mem_, DT_, IT_>("DenseVectorMinElementTest")
  {
  }

  virtual ~DenseVectorMinElementTest()
  {
  }

  virtual void run() const override
  {
    for (Index size(5) ; size < Index(1e3) ; size*=2)
    {
      DenseVector<Mem::Main, DT_, IT_> a_local(size);
      for (Index i(0) ; i < size ; ++i)
      {
        a_local(i, DT_(DT_(i) - DT_(3)));
      }

      DenseVector<Mem_, DT_, IT_> a;
      a.convert(a_local);
      Random::SeedType seed(Random::SeedType(time(nullptr)));
      std::cout << "seed: " << seed << std::endl;
      Random rng(seed);
      Adjacency::Permutation prm_rnd(a.size(), rng);
      a.permute(prm_rnd);

      DT_ min = a.min_element();

      TEST_CHECK_EQUAL(min, DT_(-3.));
    }
  }
};
DenseVectorMinElementTest<Mem::Main, float, unsigned int> dv_min_element_test_float_uint;
DenseVectorMinElementTest<Mem::Main, double, unsigned int> dv_min_element_test_double_uint;
DenseVectorMinElementTest<Mem::Main, float, unsigned long> dv_min_element_test_float_ulong;
DenseVectorMinElementTest<Mem::Main, double, unsigned long> dv_min_element_test_double_ulong;
#ifdef FEAT_HAVE_QUADMATH
DenseVectorMinElementTest<Mem::Main, __float128, unsigned int> dv_min_element_test_float128_uint;
DenseVectorMinElementTest<Mem::Main, __float128, unsigned long> dv_min_element_test_float128_ulong;
#endif
