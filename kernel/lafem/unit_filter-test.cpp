// FEAT3: Finite Element Analysis Toolbox, Version 3
// Copyright (C) 2010 - 2021 by Stefan Turek & the FEAT group
// FEAT3 is released under the GNU General Public License version 3,
// see the file 'copyright.txt' in the top level directory for details.

#include <test_system/test_system.hpp>
#include <kernel/base_header.hpp>
#include <kernel/lafem/dense_vector.hpp>
#include <kernel/lafem/sparse_matrix_csr.hpp>
#include <kernel/lafem/unit_filter.hpp>

using namespace FEAT;
using namespace FEAT::LAFEM;
using namespace FEAT::TestSystem;

/**
 * \brief Test class for UnitFilter class template
 *
 * \author Peter Zajac
 */
template<
  typename DT_,
  typename IT_>
class UnitFilterVectorTest
  : public UnitTest
{
  typedef DenseVector<DT_, IT_> VectorType;
  typedef DenseVector<IT_, IT_> IVectorType;
  typedef UnitFilter<DT_, IT_> FilterType;
public:
  UnitFilterVectorTest(PreferredBackend backend)
    : UnitTest("UnitFilterVectorTest", Type::Traits<DT_>::name(), Type::Traits<IT_>::name(), backend)
  {
  }

  virtual ~UnitFilterVectorTest()
  {
  }

  virtual void run() const override
  {
    const DT_ tol = Math::pow(Math::eps<DT_>(), DT_(0.9));

    const Index n = 7;
    VectorType a1(n, DT_(1));
    VectorType a2(n, DT_(1));
    VectorType ar(n, DT_(1));
    VectorType b1(n, DT_(1));
    VectorType b2(n, DT_(1));
    VectorType br(n, DT_(1));

    // modify reference results
    ar(0, DT_(0));
    ar(2, DT_(0));
    ar(6, DT_(0));
    br(0, DT_(3));
    br(2, DT_(5));
    br(6, DT_(9));

    // create filter
    FilterType filter(n);
    filter.add(IT_(0), DT_(3));
    filter.add(IT_(2), DT_(5));
    filter.add(IT_(6), DT_(9));

    // check sizes
    TEST_CHECK_EQUAL(filter.size(), n);
    TEST_CHECK_EQUAL(filter.used_elements(), Index(3));

    FilterType filter2;
    filter2.convert(filter);
    TEST_CHECK_EQUAL(filter2.get_filter_vector()(2), filter.get_filter_vector()(2));
    filter2.clone(filter);
    TEST_CHECK_EQUAL(filter2.get_filter_vector()(2), filter.get_filter_vector()(2));

    // apply the filter
    filter.filter_def(a1);
    filter.filter_cor(a2);
    filter.filter_rhs(b1);
    filter.filter_sol(b2);

    // subtract reference results
    a1.axpy(ar, a1, -DT_(1));
    a2.axpy(ar, a2, -DT_(1));
    b1.axpy(br, b1, -DT_(1));
    b2.axpy(br, b2, -DT_(1));

    // check results
    TEST_CHECK_EQUAL_WITHIN_EPS(a1.norm2(), DT_(0), tol);
    TEST_CHECK_EQUAL_WITHIN_EPS(a2.norm2(), DT_(0), tol);
    TEST_CHECK_EQUAL_WITHIN_EPS(b1.norm2(), DT_(0), tol);
    TEST_CHECK_EQUAL_WITHIN_EPS(b2.norm2(), DT_(0), tol);
  }
};

UnitFilterVectorTest<float, unsigned int> unit_filter_vector_test_generic_float_uint(PreferredBackend::generic);
UnitFilterVectorTest<double, unsigned int> unit_filter_vector_test_generic_double_uint(PreferredBackend::generic);
UnitFilterVectorTest<float, unsigned long> unit_filter_vector_test_generic_float_ulong(PreferredBackend::generic);
UnitFilterVectorTest<double, unsigned long> unit_filter_vector_test_generic_double_ulong(PreferredBackend::generic);
#ifdef FEAT_HAVE_MKL
UnitFilterVectorTest<float, unsigned long> mkl_unit_filter_vector_test_float_ulong(PreferredBackend::mkl);
UnitFilterVectorTest<double, unsigned long> mkl_unit_filter_vector_test_double_ulong(PreferredBackend::mkl);
#endif
#ifdef FEAT_HAVE_QUADMATH
UnitFilterVectorTest<__float128, unsigned long> unit_filter_vector_test_float128_ulong(PreferredBackend::generic);
UnitFilterVectorTest<__float128, unsigned int> unit_filter_vector_test_float128_uint(PreferredBackend::generic);
#endif
#ifdef FEAT_HAVE_HALFMATH
UnitFilterVectorTest<Half, unsigned int> unit_filter_vector_test_half_uint(PreferredBackend::generic);
UnitFilterVectorTest<Half, unsigned long> unit_filter_vector_test_half_ulong(PreferredBackend::generic);
#endif
#ifdef FEAT_HAVE_CUDA
UnitFilterVectorTest<float, unsigned int> unit_filter_vector_test_cuda_float_uint(PreferredBackend::cuda);
UnitFilterVectorTest<double, unsigned int> unit_filter_vector_test_cuda_double_uint(PreferredBackend::cuda);
UnitFilterVectorTest<float, unsigned long> unit_filter_vector_test_cuda_float_ulong(PreferredBackend::cuda);
UnitFilterVectorTest<double, unsigned long> unit_filter_vector_test_cuda_double_ulong(PreferredBackend::cuda);
#endif

/**
 * \brief Test class for UnitFilter class template
 *
 * \author Peter Zajac
 */
template<
  typename DT_,
  typename IT_>
class UnitFilterMatrixTest
  : public UnitTest
{
  typedef DenseVector<DT_, IT_> VectorType;
  typedef DenseVector<IT_, IT_> IVectorType;
  typedef UnitFilter<DT_, IT_> FilterType;
public:
  UnitFilterMatrixTest(PreferredBackend backend)
    : UnitTest("UnitFilterMatrixTest", Type::Traits<DT_>::name(), Type::Traits<IT_>::name(), backend)
  {
  }

  virtual ~UnitFilterMatrixTest()
  {
  }

  virtual void run() const override
  {
    const DT_ tol = Math::pow(Math::eps<DT_>(), DT_(0.9));

    typedef SparseMatrixCSR<DT_, IT_> MatrixType;
    IVectorType row_ptr(IT_(8));
    IVectorType col_idx(IT_(18));

    row_ptr(IT_(0), IT_(0));
    row_ptr(IT_(1), IT_(2));
    row_ptr(IT_(2), IT_(4));
    row_ptr(IT_(3), IT_(7));
    row_ptr(IT_(4), IT_(10));
    row_ptr(IT_(5), IT_(13));
    row_ptr(IT_(6), IT_(15));
    row_ptr(IT_(7), IT_(18));
    col_idx(IT_( 0), IT_(0));
    col_idx(IT_( 1), IT_(2));
    col_idx(IT_( 2), IT_(1));
    col_idx(IT_( 3), IT_(5));
    col_idx(IT_( 4), IT_(0));
    col_idx(IT_( 5), IT_(2));
    col_idx(IT_( 6), IT_(4));
    col_idx(IT_( 7), IT_(1));
    col_idx(IT_( 8), IT_(3));
    col_idx(IT_( 9), IT_(4));
    col_idx(IT_(10), IT_(0));
    col_idx(IT_(11), IT_(4));
    col_idx(IT_(12), IT_(6));
    col_idx(IT_(13), IT_(2));
    col_idx(IT_(14), IT_(5));
    col_idx(IT_(15), IT_(1));
    col_idx(IT_(16), IT_(4));
    col_idx(IT_(17), IT_(6));

    VectorType a_data(IT_(18), DT_(7));
    VectorType b_data(IT_(18), DT_(7));
    b_data(IT_( 0), DT_(1));
    b_data(IT_( 1), DT_(0));
    b_data(IT_( 4), DT_(0));
    b_data(IT_( 5), DT_(1));
    b_data(IT_( 6), DT_(0));
    b_data(IT_(15), DT_(0));
    b_data(IT_(16), DT_(0));
    b_data(IT_(17), DT_(1));

    // create a CSR matrix
    MatrixType matrix_a(Index(7), Index(7), col_idx, a_data, row_ptr);
    MatrixType matrix_b(Index(7), Index(7), col_idx, b_data, row_ptr);

    // create filter
    FilterType filter(IT_(7));
    filter.add(IT_(0), DT_(3));
    filter.add(IT_(2), DT_(5));
    filter.add(IT_(6), DT_(9));

    // apply filter onto a
    filter.filter_mat(matrix_a);

    // subtract reference
    matrix_a.axpy(matrix_b, matrix_a, -DT_(1));

    // check difference
    TEST_CHECK_EQUAL_WITHIN_EPS(matrix_a.norm_frobenius(), DT_(0), tol);
  }
};

UnitFilterMatrixTest<float, unsigned long> unit_filter_matrix_test_generic_float_ulong(PreferredBackend::generic);
UnitFilterMatrixTest<double, unsigned long> unit_filter_matrix_test_generic_double_ulong(PreferredBackend::generic);
UnitFilterMatrixTest<float, unsigned int> unit_filter_matrix_test_generic_float_uint(PreferredBackend::generic);
UnitFilterMatrixTest<double, unsigned int> unit_filter_matrix_test_generic_double_uint(PreferredBackend::generic);
#ifdef FEAT_HAVE_MKL
UnitFilterMatrixTest<float, unsigned long> mkl_unit_filter_matrix_test_float_ulong(PreferredBackend::mkl);
UnitFilterMatrixTest<double, unsigned long> mkl_unit_filter_matrix_test_double_ulong(PreferredBackend::mkl);
#endif
#ifdef FEAT_HAVE_QUADMATH
UnitFilterMatrixTest<__float128, unsigned long> unit_filter_matrix_test_float128_ulong(PreferredBackend::generic);
UnitFilterMatrixTest<__float128, unsigned int> unit_filter_matrix_test_float128_uint(PreferredBackend::generic);
#endif
#ifdef FEAT_HAVE_HALFMATH
UnitFilterMatrixTest<Half, unsigned int> unit_filter_matrix_test_half_uint(PreferredBackend::generic);
UnitFilterMatrixTest<Half, unsigned long> unit_filter_matrix_test_half_ulong(PreferredBackend::generic);
#endif
#ifdef FEAT_HAVE_CUDA
UnitFilterMatrixTest<float, unsigned long> cuda_unit_filter_matrix_test_float_ulong(PreferredBackend::cuda);
UnitFilterMatrixTest<double, unsigned long> cuda_unit_filter_matrix_test_double_ulong(PreferredBackend::cuda);
UnitFilterMatrixTest<float, unsigned int> cuda_unit_filter_matrix_test_float_uint(PreferredBackend::cuda);
UnitFilterMatrixTest<double, unsigned int> cuda_unit_filter_matrix_test_double_uint(PreferredBackend::cuda);
#endif
