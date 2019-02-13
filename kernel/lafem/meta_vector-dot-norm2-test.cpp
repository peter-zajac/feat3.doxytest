// FEAT3: Finite Element Analysis Toolbox, Version 3
// Copyright (C) 2010 - 2021 by Stefan Turek & the FEAT group
// FEAT3 is released under the GNU General Public License version 3,
// see the file 'copyright.txt' in the top level directory for details.

#include <test_system/test_system.hpp>
#include <kernel/lafem/meta_vector_test_base.hpp>

using namespace FEAT;
using namespace FEAT::LAFEM;
using namespace FEAT::TestSystem;

/**
 * \brief Meta-Vector 'dot' and 'norm2' test class
 *
 * \test The 'dot' and 'norm2' operations of the DenseVector, PowerVector and TupleVector class templates.
 *
 * \author Peter Zajac
 */
template<
  typename DataType_,
  typename IndexType_>
class MetaVectorDotNorm2Test
  : public MetaVectorTestBase<DataType_, IndexType_>
{
public:
  typedef DataType_ DataType;
  typedef MetaVectorTestBase<DataType_, IndexType_> BaseClass;
  typedef typename BaseClass::MetaVector MetaVector;

   MetaVectorDotNorm2Test(PreferredBackend backend) :
    BaseClass("MetaVectorDotNorm2Test", Type::Traits<DataType>::name(), Type::Traits<IndexType_>::name(), backend)
  {
  }

  virtual ~MetaVectorDotNorm2Test()
  {
  }

  using BaseClass::fx00;
  using BaseClass::fx01;
  using BaseClass::fx1;
  using BaseClass::fy00;
  using BaseClass::fy01;
  using BaseClass::fy1;

  virtual void run() const override
  {
    const DataType tol = Math::pow(Math::eps<DataType>(), DataType(0.6));

    const Index n00 = 5;
    const Index n01 = 10;
    const Index n1 = 7;

    MetaVector x(this->gen_vector_x(n00, n01, n1));
    MetaVector y(this->gen_vector_y(n00, n01, n1));

    // compute x*x and x*y
    DataType x_dot_y(DataType(0));
    DataType x_dot_x(DataType(0));

    // compute reference results
    for(Index i(0); i < n00; ++i)
    {
      x_dot_y += fx00(i) * fy00(i);
      x_dot_x += Math::sqr(fx00(i));
    }
    for(Index i(0); i < n01; ++i)
    {
      x_dot_y += fx01(i) * fy01(i);
      x_dot_x += Math::sqr(fx01(i));
    }
    for(Index i(0); i < n1; ++i)
    {
      x_dot_y += fx1(i) * fy1(i);
      x_dot_x += Math::sqr(fx1(i));
    }
    DataType x_norm2(Math::sqrt(x_dot_x));

    // test x*y
    TEST_CHECK_EQUAL_WITHIN_EPS(x.dot(y), x_dot_y, tol);

    // test x*x
    TEST_CHECK_EQUAL_WITHIN_EPS(x.dot(x), x_dot_x, tol);

    // test norm2(x)
    TEST_CHECK_EQUAL_WITHIN_EPS(x.norm2(), x_norm2, tol);
  }
};

MetaVectorDotNorm2Test<float, unsigned int> meta_vector_dot_norm2_test_generic_float_uint(PreferredBackend::generic);
MetaVectorDotNorm2Test<double, unsigned int> meta_vector_dot_norm2_test_generic_double_uint(PreferredBackend::generic);
MetaVectorDotNorm2Test<float, unsigned long> meta_vector_dot_norm2_test_generic_float_ulong(PreferredBackend::generic);
MetaVectorDotNorm2Test<double, unsigned long> meta_vector_dot_norm2_test_generic_double_ulong(PreferredBackend::generic);
#ifdef FEAT_HAVE_MKL
MetaVectorDotNorm2Test<float, unsigned long> mkl_meta_vector_dot_norm2_test_float_ulong(PreferredBackend::mkl);
MetaVectorDotNorm2Test<double, unsigned long> mkl_meta_vector_dot_norm2_test_double_ulong(PreferredBackend::mkl);
#endif
#ifdef FEAT_HAVE_QUADMATH
MetaVectorDotNorm2Test<__float128, unsigned int> meta_vector_dot_norm2_test_generic_float128_uint(PreferredBackend::generic);
MetaVectorDotNorm2Test<__float128, unsigned long> meta_vector_dot_norm2_test_generic_float128_ulong(PreferredBackend::generic);
#endif
#ifdef FEAT_HAVE_HALFMATH
MetaVectorDotNorm2Test<Half, unsigned int> meta_vector_dot_norm2_test_half_uint(PreferredBackend::generic);
MetaVectorDotNorm2Test<Half, unsigned long> meta_vector_dot_norm2_test_half_ulong(PreferredBackend::generic);
#endif
#ifdef FEAT_HAVE_CUDA
MetaVectorDotNorm2Test<float, unsigned int> meta_vector_dot_norm2_test_cuda_float_uint(PreferredBackend::cuda);
MetaVectorDotNorm2Test<double, unsigned int> meta_vector_dot_norm2_test_cuda_double_uint(PreferredBackend::cuda);
MetaVectorDotNorm2Test<float, unsigned long> meta_vector_dot_norm2_test_cuda_float_ulong(PreferredBackend::cuda);
MetaVectorDotNorm2Test<double, unsigned long> meta_vector_dot_norm2_test_cuda_double_ulong(PreferredBackend::cuda);
#endif

/**
 * \brief Meta vector triple_dot and triple_dot_i test class
 *
 * \test The triple_dot and triple_dot_i routines of the PowerVector and TupleVector class templates.
 *
 * \author Jordi Paul
 */
template<
  typename DataType_,
  typename IndexType_>
class MetaVectorTripleDotTest
  : public MetaVectorTestBase<DataType_, IndexType_>
{
public:
  typedef DataType_ DataType;
  typedef MetaVectorTestBase<DataType_, IndexType_> BaseClass;
  typedef typename BaseClass::MetaVector MetaVector;

   MetaVectorTripleDotTest(PreferredBackend backend) :
    BaseClass("MetaVectorTripleDotTest", Type::Traits<DataType>::name(), Type::Traits<IndexType_>::name(), backend)
  {
  }

  virtual ~MetaVectorTripleDotTest()
  {
  }

  using BaseClass::fx00;
  using BaseClass::fx01;
  using BaseClass::fx1;
  using BaseClass::fy00;
  using BaseClass::fy01;
  using BaseClass::fy1;
  using BaseClass::fz00;
  using BaseClass::fz01;
  using BaseClass::fz1;

  virtual void run() const override
  {
    const DataType tol = Math::pow(Math::eps<DataType>(), DataType(0.6));

    const Index n00 = 7;
    const Index n01 = 10;
    const Index n1 = 5;

    MetaVector x(this->gen_vector_x(n00, n01, n1));
    MetaVector y(this->gen_vector_y(n00, n01, n1));
    MetaVector z(this->gen_vector_z(n00, n01, n1));

    // compute x^T diag(z) y and x^T diag(1/z_ii) y
    DataType tdot(DataType(0));

    // compute reference results
    for(Index i(0); i < n00; ++i)
    {
      tdot += fx00(i) * fy00(i) * fz00(i);
    }
    for(Index i(0); i < n01; ++i)
    {
      tdot += fx01(i) * fy01(i) * fz01(i);
    }
    for(Index i(0); i < n1; ++i)
    {
      tdot += fx1(i) * fy1(i) * fz1(i);
    }

    // test y^T diag(x) z
    TEST_CHECK_EQUAL_WITHIN_EPS(x.triple_dot(y, z), tdot, tol);
    TEST_CHECK_EQUAL_WITHIN_EPS(x.triple_dot(z, y), tdot, tol);

    // test x^T diag(y) z
    TEST_CHECK_EQUAL_WITHIN_EPS(y.triple_dot(x, z), tdot, tol);
    TEST_CHECK_EQUAL_WITHIN_EPS(y.triple_dot(z, x), tdot, tol);

    // test x^T diag(z) y
    TEST_CHECK_EQUAL_WITHIN_EPS(z.triple_dot(x, y), tdot, tol);
    TEST_CHECK_EQUAL_WITHIN_EPS(z.triple_dot(y, x), tdot, tol);
  }
};

MetaVectorTripleDotTest<float, unsigned int> meta_vector_triple_dot_test_generic_float_uint(PreferredBackend::generic);
MetaVectorTripleDotTest<double, unsigned int> meta_vector_triple_dot_test_generic_double_uint(PreferredBackend::generic);
MetaVectorTripleDotTest<float, unsigned long> meta_vector_triple_dot_test_generic_float_ulong(PreferredBackend::generic);
MetaVectorTripleDotTest<double, unsigned long> meta_vector_triple_dot_test_generic_double_ulong(PreferredBackend::generic);
#ifdef FEAT_HAVE_MKL
MetaVectorTripleDotTest<float, unsigned long> mkl_meta_vector_triple_dot_test_float_ulong(PreferredBackend::mkl);
MetaVectorTripleDotTest<double, unsigned long> mkl_meta_vector_triple_dot_test_double_ulong(PreferredBackend::mkl);
#endif
#ifdef FEAT_HAVE_QUADMATH
MetaVectorTripleDotTest<__float128, unsigned int> meta_vector_triple_dot_test_generic_float128_uint(PreferredBackend::generic);
MetaVectorTripleDotTest<__float128, unsigned long> meta_vector_triple_dot_test_generic_float128_ulong(PreferredBackend::generic);
#endif
#ifdef FEAT_HAVE_HALFMATH
MetaVectorTripleDotTest<Half, unsigned int> meta_vector_triple_dot_test_half_uint(PreferredBackend::generic);
MetaVectorTripleDotTest<Half, unsigned long> meta_vector_triple_dot_test_half_ulong(PreferredBackend::generic);
#endif
#ifdef FEAT_HAVE_CUDA
MetaVectorTripleDotTest<float, unsigned int> meta_vector_triple_dot_test_cuda_float_uint(PreferredBackend::cuda);
MetaVectorTripleDotTest<double, unsigned int> meta_vector_triple_dot_test_cuda_double_uint(PreferredBackend::cuda);
MetaVectorTripleDotTest<float, unsigned long> meta_vector_triple_dot_test_cuda_float_ulong(PreferredBackend::cuda);
MetaVectorTripleDotTest<double, unsigned long> meta_vector_triple_dot_test_cuda_double_ulong(PreferredBackend::cuda);
#endif
