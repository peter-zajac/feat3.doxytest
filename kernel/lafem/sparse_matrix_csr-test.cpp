#include <kernel/base_header.hpp>
#include <kernel/archs.hpp>
#include <test_system/test_system.hpp>
#include <kernel/lafem/sparse_matrix_coo.hpp>
#include <kernel/lafem/sparse_matrix_csr.hpp>
#include <kernel/util/binary_stream.hpp>
#include <kernel/util/random.hpp>
#include <kernel/adjacency/cuthill_mckee.hpp>

#include <sstream>

using namespace FEAST;
using namespace FEAST::LAFEM;
using namespace FEAST::TestSystem;

/**
 * \brief Test class for the sparse matrix csr class.
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
class SparseMatrixCSRTest
  : public FullTaggedTest<Mem_, DT_, IT_>
{
public:
  SparseMatrixCSRTest()
    : FullTaggedTest<Mem_, DT_, IT_>("SparseMatrixCSRTest")
  {
  }

  virtual void run() const
  {
    SparseMatrixCSR<Mem_, DT_, IT_> zero1;
    SparseMatrixCSR<Mem::Main, DT_, IT_> zero2;
    TEST_CHECK_EQUAL(zero1, zero2);

    SparseMatrixCOO<Mem::Main, DT_, IT_> a(10, 10);
    a(1,2,7);
    a.format();
    a(1,2,7);
    a(5,5,2);
    a(5,7,3);
    a(5,2,4);
    SparseMatrixCSR<Mem_, DT_, IT_> b(a);
    TEST_CHECK_EQUAL(b.used_elements(), a.used_elements());
    TEST_CHECK_EQUAL(b.size(), a.size());
    TEST_CHECK_EQUAL(b.rows(), a.rows());
    TEST_CHECK_EQUAL(b.columns(), a.columns());
    TEST_CHECK_EQUAL(b(1, 2), a(1, 2));
    TEST_CHECK_EQUAL(b(5, 5), a(5, 5));
    TEST_CHECK_EQUAL(b(5, 2), a(5, 2));
    TEST_CHECK_EQUAL(b(1, 1), a(1, 1));

    Index bandw, bandw_idx;
    b.bandwidth_row(bandw, bandw_idx);
    TEST_CHECK_EQUAL(bandw, Index(6));
    TEST_CHECK_EQUAL(bandw_idx, Index(5));
    b.bandwidth_column(bandw, bandw_idx);
    TEST_CHECK_EQUAL(bandw, Index(5));
    TEST_CHECK_EQUAL(bandw_idx, Index(2));

    Index radius, radius_idx;
    b.radius_row(radius, radius_idx);
    TEST_CHECK_EQUAL(radius, Index(3));
    TEST_CHECK_EQUAL(radius_idx, Index(5));
    b.radius_column(radius, radius_idx);
    TEST_CHECK_EQUAL(radius, Index(3));
    TEST_CHECK_EQUAL(radius_idx, Index(2));

    SparseMatrixCSR<Mem_, DT_, IT_> bl(b.layout());
    TEST_CHECK_EQUAL(bl.used_elements(), b.used_elements());
    TEST_CHECK_EQUAL(bl.size(), b.size());
    TEST_CHECK_EQUAL(bl.rows(), b.rows());
    TEST_CHECK_EQUAL(bl.columns(), b.columns());

    bl = b.layout();
    TEST_CHECK_EQUAL(bl.used_elements(), b.used_elements());
    TEST_CHECK_EQUAL(bl.size(), b.size());
    TEST_CHECK_EQUAL(bl.rows(), b.rows());
    TEST_CHECK_EQUAL(bl.columns(), b.columns());

    typename SparseLayout<Mem_, IT_, SparseLayoutId::lt_csr>::template MatrixType<DT_> x(b.layout());
    // icc 14.0.2 does not understand the following line, so we need a typedef hier
    //typename decltype(b.layout())::template MatrixType<DT_> y(b.layout());
    typedef decltype(b.layout()) LayoutId;
    typename LayoutId::template MatrixType<DT_> y(b.layout());
    TEST_CHECK_EQUAL((void*)x.row_ptr(), (void*)b.row_ptr());
    TEST_CHECK_NOT_EQUAL((void*)x.val(), (void*)b.val());


    SparseMatrixCSR<Mem_, DT_, IT_> z;
    z.convert(b);
    TEST_CHECK_EQUAL(z.used_elements(), 4ul);
    TEST_CHECK_EQUAL(z.size(), a.size());
    TEST_CHECK_EQUAL(z.rows(), a.rows());
    TEST_CHECK_EQUAL(z.columns(), a.columns());
    TEST_CHECK_EQUAL(z(1, 2), a(1, 2));
    TEST_CHECK_EQUAL(z(5, 5), a(5, 5));

    SparseMatrixCSR<Mem_, DT_, IT_> c;
    c.clone(b);
    TEST_CHECK_NOT_EQUAL((void*)c.val(), (void*)b.val());
    TEST_CHECK_EQUAL((void*)c.col_ind(), (void*)b.col_ind());
    c = b.clone(CloneMode::Deep);
    TEST_CHECK_NOT_EQUAL((void*)c.val(), (void*)b.val());
    TEST_CHECK_NOT_EQUAL((void*)c.col_ind(), (void*)b.col_ind());
    c = b.shared();
    TEST_CHECK_EQUAL((void*)c.val(), (void*)b.val());
    TEST_CHECK_EQUAL((void*)c.col_ind(), (void*)b.col_ind());

    DenseVector<Mem_, IT_, IT_> col_ind(c.used_elements(), c.col_ind());
    DenseVector<Mem_, DT_, IT_> val(c.used_elements(), c.val());
    DenseVector<Mem_, IT_, IT_> row_ptr(c.rows() + 1, c.row_ptr());
    SparseMatrixCSR<Mem_, DT_, IT_> d(c.rows(), c.columns(), col_ind, val, row_ptr);
    TEST_CHECK_EQUAL(d, c);

    SparseMatrixCSR<Mem::Main, DT_, IT_> e;
    e.convert(c);
    TEST_CHECK_EQUAL(e, c);
    e.copy(c);
    TEST_CHECK_EQUAL(e, c);
    e.clone(c);
    b.clone(e);
    TEST_CHECK_EQUAL(b, c);

    SparseMatrixCOO<Mem::Main, DT_, IT_> fcoo(10, 10);
    for (Index row(0) ; row < fcoo.rows() ; ++row)
    {
      for (Index col(0) ; col < fcoo.columns() ; ++col)
      {
        if(row == col)
          fcoo(row, col, DT_(2));
        else if((row == col+1) || (row+1 == col))
          fcoo(row, col, DT_(-1));
      }
    }
    SparseMatrixCSR<Mem_, DT_, IT_> f(fcoo);

    BinaryStream bs;
    f.write_out(FileMode::fm_csr, bs);
    bs.seekg(0);
    SparseMatrixCSR<Mem_, DT_, IT_> g(FileMode::fm_csr, bs);
    TEST_CHECK_EQUAL(g, f);

    std::stringstream ts;
    f.write_out(FileMode::fm_mtx, ts);
    SparseMatrixCSR<Mem::Main, DT_, IT_> j(FileMode::fm_mtx, ts);
    TEST_CHECK_EQUAL(j, f);

    std::stringstream ts2;
    f.write_out_mtx(ts2, true);
    SparseMatrixCSR<Mem::Main, DT_, IT_> j2(FileMode::fm_mtx, ts2);
    TEST_CHECK_EQUAL(j2, f);

    auto kp = f.serialise();
    SparseMatrixCSR<Mem_, DT_, IT_> k(kp);
    TEST_CHECK_EQUAL(k, f);

    // new clone testing
    auto clone1 = b.clone(CloneMode::Deep);
    TEST_CHECK_EQUAL(clone1, b);
    Util::MemoryPool<Mem_>::set_memory(clone1.val() + 1, DT_(132));
    TEST_CHECK_NOT_EQUAL(clone1, b);
    TEST_CHECK_NOT_EQUAL((void*)clone1.val(), (void*)b.val());
    TEST_CHECK_NOT_EQUAL((void*)clone1.row_ptr(), (void*)b.row_ptr());
    auto clone2 = clone1.clone(CloneMode::Layout);
    Util::MemoryPool<Mem_>::set_memory(clone2.val(), DT_(4713), clone2.used_elements());
    TEST_CHECK_NOT_EQUAL(clone2(5, 5), clone1(5, 5));
    TEST_CHECK_NOT_EQUAL((void*)clone2.val(), (void*)clone1.val());
    TEST_CHECK_EQUAL((void*)clone2.row_ptr(), (void*)clone1.row_ptr());
    auto clone3 = clone1.clone(CloneMode::Weak);
    TEST_CHECK_EQUAL(clone3, clone1);
    Util::MemoryPool<Mem_>::set_memory(clone3.val() + 1, DT_(133));
    TEST_CHECK_NOT_EQUAL(clone3, clone1);
    TEST_CHECK_NOT_EQUAL((void*)clone3.val(), (void*)clone1.val());
    TEST_CHECK_EQUAL((void*)clone3.row_ptr(), (void*)clone1.row_ptr());
    auto clone4 = clone1.clone(CloneMode::Shallow);
    TEST_CHECK_EQUAL(clone4, clone1);
    Util::MemoryPool<Mem_>::set_memory(clone4.val() + 1, DT_(134));
    TEST_CHECK_EQUAL(clone4, clone1);
    TEST_CHECK_EQUAL((void*)clone4.val(), (void*)clone1.val());
    TEST_CHECK_EQUAL((void*)clone4.row_ptr(), (void*)clone1.row_ptr());
  }
};

SparseMatrixCSRTest<Mem::Main, float, unsigned long> cpu_sparse_matrix_csr_test_float_ulong;
SparseMatrixCSRTest<Mem::Main, double, unsigned long> cpu_sparse_matrix_csr_test_double_ulong;
SparseMatrixCSRTest<Mem::Main, float, unsigned int> cpu_sparse_matrix_csr_test_float_uint;
SparseMatrixCSRTest<Mem::Main, double, unsigned int> cpu_sparse_matrix_csr_test_double_uint;
#ifdef FEAST_HAVE_QUADMATH
SparseMatrixCSRTest<Mem::Main, __float128, unsigned long> cpu_sparse_matrix_csr_test_float128_ulong;
SparseMatrixCSRTest<Mem::Main, __float128, unsigned int> cpu_sparse_matrix_csr_test_float128_uint;
#endif
#ifdef FEAST_BACKENDS_CUDA
SparseMatrixCSRTest<Mem::CUDA, float, unsigned long> cuda_sparse_matrix_csr_test_float_ulong;
SparseMatrixCSRTest<Mem::CUDA, double, unsigned long> cuda_sparse_matrix_csr_test_double_ulong;
SparseMatrixCSRTest<Mem::CUDA, float, unsigned int> cuda_sparse_matrix_csr_test_float_uint;
SparseMatrixCSRTest<Mem::CUDA, double, unsigned int> cuda_sparse_matrix_csr_test_double_uint;
#endif


template<
  typename Mem_,
  typename DT_,
  typename IT_>
class SparseMatrixCSRApplyTest
  : public FullTaggedTest<Mem_, DT_, IT_>
{
public:
  SparseMatrixCSRApplyTest()
    : FullTaggedTest<Mem_, DT_, IT_>("SparseMatrixCSRApplyTest")
  {
  }

  virtual void run() const
  {
    DT_ s(DT_(4711.1));
    for (Index size(1) ; size < 1e3 ; size*=2)
    {
      SparseMatrixCOO<Mem::Main, DT_, IT_> a_local(size, size);
      DenseVector<Mem::Main, DT_, IT_> x_local(size);
      DenseVector<Mem::Main, DT_, IT_> y_local(size);
      DenseVector<Mem::Main, DT_, IT_> ref_local(size);
      DenseVector<Mem_, DT_, IT_> ref(size);
      DenseVector<Mem::Main, DT_, IT_> result_local(size);
      for (Index i(0) ; i < size ; ++i)
      {
        x_local(i, DT_(i % 100 * DT_(1.234)));
        y_local(i, DT_(2 - DT_(i % 42)));
      }
      DenseVector<Mem_, DT_, IT_> x(size);
      x.copy(x_local);
      DenseVector<Mem_, DT_, IT_> y(size);
      y.copy(y_local);

      Index ue(0);
      for (Index row(0) ; row < a_local.rows() ; ++row)
      {
        for (Index col(0) ; col < a_local.columns() ; ++col)
        {
          if(row == col)
          {
            a_local(row, col, DT_(2));
            ++ue;
          }
          else if((row == col+1) || (row+1 == col))
          {
            a_local(row, col, DT_(-1));
            ++ue;
          }
        }
      }
      SparseMatrixCSR<Mem_,DT_, IT_> a(a_local);

      DenseVector<Mem_, DT_, IT_> r(size);

      // apply-test for alpha = 0.0
      a.apply(r, x, y, DT_(0.0));
      for (Index i(0) ; i < size ; ++i)
        TEST_CHECK_EQUAL_WITHIN_EPS(y(i), r(i), 1e-2);

      // apply-test for alpha = -1.0
      a.apply(r, x, y, DT_(-1.0));
      a.apply(ref, x);
      ref.scale(ref, DT_(-1.0));
      ref.axpy(ref, y);

      for (Index i(0) ; i < size ; ++i)
        TEST_CHECK_EQUAL_WITHIN_EPS(ref(i), r(i), 1e-2);

      // apply-test for alpha = 4711.1
      //r.axpy(s, a, x, y);
      a.apply(r, x, y, s);
      result_local.copy(r);

      //ref.product_matvec(a, x);
      a.apply(ref, x);
      ref.scale(ref, s);
      ref.axpy(ref, y);
      ref_local.copy(ref);

      for (Index i(0) ; i < size ; ++i)
        TEST_CHECK_EQUAL_WITHIN_EPS(result_local(i), ref_local(i), 1e-2);

      a.apply(r, x);
      result_local.copy(r);
      a.apply(ref, x);
      ref_local.copy(ref);
      for (Index i(0) ; i < size ; ++i)
        TEST_CHECK_EQUAL_WITHIN_EPS(result_local(i), ref_local(i), 1e-2);
    }
  }
};

SparseMatrixCSRApplyTest<Mem::Main, float, unsigned long> sm_csr_apply_test_float_ulong;
SparseMatrixCSRApplyTest<Mem::Main, double, unsigned long> sm_csr_apply_test_double_ulong;
SparseMatrixCSRApplyTest<Mem::Main, float, unsigned int> sm_csr_apply_test_float_uint;
SparseMatrixCSRApplyTest<Mem::Main, double, unsigned int> sm_csr_apply_test_double_uint;
#ifdef FEAST_HAVE_QUADMATH
SparseMatrixCSRApplyTest<Mem::Main, __float128, unsigned long> sm_csr_apply_test_float128_ulong;
SparseMatrixCSRApplyTest<Mem::Main, __float128, unsigned int> sm_csr_apply_test_float128_uint;
#endif
#ifdef FEAST_BACKENDS_CUDA
SparseMatrixCSRApplyTest<Mem::CUDA, float, unsigned long> cuda_sm_csr_apply_test_float_ulong;
SparseMatrixCSRApplyTest<Mem::CUDA, double, unsigned long> cuda_sm_csr_apply_test_double_ulong;
SparseMatrixCSRApplyTest<Mem::CUDA, float, unsigned int> cuda_sm_csr_apply_test_float_uint;
SparseMatrixCSRApplyTest<Mem::CUDA, double, unsigned int> cuda_sm_csr_apply_test_double_uint;
#endif


template<
  typename Mem_,
  typename DT_,
  typename IT_>
class SparseMatrixCSRScaleTest
  : public FullTaggedTest<Mem_, DT_, IT_>
{
public:
  SparseMatrixCSRScaleTest()
    : FullTaggedTest<Mem_, DT_, IT_>("SparseMatrixCSRScaleTest")
  {
  }

  virtual void run() const
  {
    for (Index size(2) ; size < 3e2 ; size*=2)
    {
      DT_ s(DT_(4.321));

      SparseMatrixCOO<Mem::Main, DT_, IT_> a_local(size, size + 2);
      SparseMatrixCOO<Mem::Main, DT_, IT_> ref_local(size, size + 2);
      for (Index row(0) ; row < a_local.rows() ; ++row)
      {
        for (Index col(0) ; col < a_local.columns() ; ++col)
        {
          if(row == col)
            a_local(row, col, DT_(2));
          else if((row == col+1) || (row+1 == col))
            a_local(row, col, DT_(-1));

          if(row == col)
            ref_local(row, col, DT_(2) * s);
          else if((row == col+1) || (row+1 == col))
            ref_local(row, col, DT_(-1) * s);
        }
      }

      SparseMatrixCSR<Mem_, DT_, IT_> ref(ref_local);
      SparseMatrixCSR<Mem_, DT_, IT_> a(a_local);
      SparseMatrixCSR<Mem_, DT_, IT_> b;
      b.clone(a);

      b.scale(a, s);
      TEST_CHECK_EQUAL(b, ref);

      a.scale(a, s);
      TEST_CHECK_EQUAL(a, ref);
    }
  }
};

SparseMatrixCSRScaleTest<Mem::Main, float, unsigned int> sm_csr_scale_test_float_uint;
SparseMatrixCSRScaleTest<Mem::Main, double, unsigned int> sm_csr_scale_test_double_uint;
SparseMatrixCSRScaleTest<Mem::Main, float, unsigned long> sm_csr_scale_test_float_ulong;
SparseMatrixCSRScaleTest<Mem::Main, double, unsigned long> sm_csr_scale_test_double_ulong;
#ifdef FEAST_HAVE_QUADMATH
SparseMatrixCSRScaleTest<Mem::Main, __float128, unsigned int> sm_csr_scale_test_float128_uint;
SparseMatrixCSRScaleTest<Mem::Main, __float128, unsigned long> sm_csr_scale_test_float128_ulong;
#endif
#ifdef FEAST_BACKENDS_CUDA
SparseMatrixCSRScaleTest<Mem::CUDA, float, unsigned int> cuda_sm_csr_scale_test_float_uint;
SparseMatrixCSRScaleTest<Mem::CUDA, double, unsigned int> cuda_sm_csr_scale_test_double_uint;
SparseMatrixCSRScaleTest<Mem::CUDA, float, unsigned long> cuda_sm_csr_scale_test_float_ulong;
SparseMatrixCSRScaleTest<Mem::CUDA, double, unsigned long> cuda_sm_csr_scale_test_double_ulong;
#endif


template<
  typename Mem_,
  typename DT_,
  typename IT_>
class SparseMatrixCSRScaleRowColTest
  : public FullTaggedTest<Mem_, DT_, IT_>
{
public:
  SparseMatrixCSRScaleRowColTest()
    : FullTaggedTest<Mem_, DT_, IT_>("SparseMatrixCSRScaleRowColTest")
  {
  }

  virtual void run() const
  {
    for (Index size(2) ; size < 3e2 ; size*=3)
    {
      const DT_ pi(Math::pi<DT_>());
      const DT_ eps(Math::pow(Math::eps<DT_>(), DT_(0.8)));

      SparseMatrixCOO<Mem::Main, DT_, IT_> a_local(size, size + 2);
      for (Index row(0) ; row < a_local.rows() ; ++row)
      {
        for (Index col(0) ; col < a_local.columns() ; ++col)
        {
          if(row == col)
            a_local(row, col, DT_(2));
          else if((row == col+1) || (row+1 == col))
            a_local(row, col, DT_(-1));
        }
      }

      SparseMatrixCSR<Mem_, DT_, IT_> a(a_local);
      SparseMatrixCSR<Mem_, DT_, IT_> b(a.clone());

      // Scale rows
      DenseVector<Mem_, DT_, IT_> s1(a.rows());
      for (Index i(0); i < s1.size(); ++i)
      {
        s1(i, pi * (i % 3 + 1) - DT_(5.21) + DT_(i));
      }
      b.scale_rows(b, s1);
      for (Index row(0) ; row < a.rows() ; ++row)
      {
        for (Index col(0) ; col < a.columns() ; ++col)
        {
          TEST_CHECK_EQUAL_WITHIN_EPS(b(row, col), a(row, col) * s1(row), eps);
        }
      }

      // Scale rows
      DenseVector<Mem_, DT_, IT_> s2(a.columns());
      for (Index i(0); i < s2.size(); ++i)
      {
        s2(i, pi * (i % 3 + 1) - DT_(5.21) + DT_(i));
      }
      b.scale_cols(a, s2);
      for (Index row(0) ; row < a.rows() ; ++row)
      {
        for (Index col(0) ; col < a.columns() ; ++col)
        {
          TEST_CHECK_EQUAL_WITHIN_EPS(b(row, col), a(row, col) * s2(col), eps);
        }
      }
    }
  }
};

SparseMatrixCSRScaleRowColTest<Mem::Main, float, unsigned int> sm_csr_scale_row_col_test_float_uint;
SparseMatrixCSRScaleRowColTest<Mem::Main, double, unsigned int> sm_csr_scale_row_col_test_double_uint;
SparseMatrixCSRScaleRowColTest<Mem::Main, float, unsigned long> sm_csr_scale_row_col_test_float_ulong;
SparseMatrixCSRScaleRowColTest<Mem::Main, double, unsigned long> sm_csr_scale_row_col_test_double_ulong;
#ifdef FEAST_HAVE_QUADMATH
SparseMatrixCSRScaleRowColTest<Mem::Main, __float128, unsigned int> sm_csr_scale_row_col_test_float128_uint;
SparseMatrixCSRScaleRowColTest<Mem::Main, __float128, unsigned long> sm_csr_scale_row_col_test_float128_ulong;
#endif
// #ifdef FEAST_BACKENDS_CUDA
// SparseMatrixCSRScaleRowColTest<Mem::CUDA, float, unsigned int> cuda_sm_csr_scale_row_col_test_float_uint;
// SparseMatrixCSRScaleRowColTest<Mem::CUDA, double, unsigned int> cuda_sm_csr_scale_row_col_test_double_uint;
// SparseMatrixCSRScaleRowColTest<Mem::CUDA, float, unsigned long> cuda_sm_csr_scale_row_col_test_float_ulong;
// SparseMatrixCSRScaleRowColTest<Mem::CUDA, double, unsigned long> cuda_sm_csr_scale_row_col_test_double_ulong;
// #endif


/**
 * \brief Test class for the transposition of a SparseMatrixCSR
 *
 * \test test description missing
 *
 * \tparam Mem_
 * description missing
 *
 * \tparam DT_
 * description missing
 *
 * \tparam IT_
 * description missing
 *
 * \author Christoph Lohmann
 */
template<
  typename Mem_,
  typename DT_,
  typename IT_>
class SparseMatrixCSRTranspositionTest
  : public FullTaggedTest<Mem_, DT_, IT_>
{

public:
  typedef SparseMatrixCSR<Mem_, DT_, IT_> MatrixType;

  SparseMatrixCSRTranspositionTest()
    : FullTaggedTest<Mem_, DT_, IT_>("SparseMatrixCSRTranspositionTest")
  {
  }

  virtual void run() const
  {
    for (Index size(2) ; size < 3e2 ; size*=4)
    {
      SparseMatrixCOO<Mem::Main, DT_, IT_> a_local(size, size + 2);

      for (Index row(0) ; row < a_local.rows() ; ++row)
      {
        for (Index col(0) ; col < a_local.columns() ; ++col)
        {
          if(row == col)
            a_local(row, col, DT_(2));
          else if(row == col+1)
            a_local(row, col, DT_(-1));
          else if(row+1 == col)
            a_local(row, col, DT_(-3));
        }
      }
      MatrixType a;
      a.convert(a_local);

      MatrixType b;
      b.transpose(a);

      for (Index i(0) ; i < a.rows() ; ++i)
      {
        for (Index j(0) ; j < a.columns() ; ++j)
        {
          TEST_CHECK_EQUAL(b(j, i), a(i, j));
        }
      }

      b = b.transpose();

      TEST_CHECK_EQUAL(a, b);
    }
  }
};

SparseMatrixCSRTranspositionTest<Mem::Main, float, unsigned int> sm_csr_transposition_test_float_uint;
SparseMatrixCSRTranspositionTest<Mem::Main, double, unsigned int> sm_csr_transposition_test_double_uint;
SparseMatrixCSRTranspositionTest<Mem::Main, float, unsigned long> sm_csr_transposition_test_float_ulong;
SparseMatrixCSRTranspositionTest<Mem::Main, double, unsigned long> sm_csr_transposition_test_double_ulong;
#ifdef FEAST_HAVE_QUADMATH
SparseMatrixCSRTranspositionTest<Mem::Main, __float128, unsigned int> sm_csr_transposition_test_float128_uint;
SparseMatrixCSRTranspositionTest<Mem::Main, __float128, unsigned long> sm_csr_transposition_test_float128_ulong;
#endif
#ifdef FEAST_BACKENDS_CUDA
SparseMatrixCSRTranspositionTest<Mem::CUDA, float, unsigned int> cuda_sm_csr_transposition_test_float_uint;
SparseMatrixCSRTranspositionTest<Mem::CUDA, double, unsigned int> cuda_sm_csr_transposition_test_double_uint;
SparseMatrixCSRTranspositionTest<Mem::CUDA, float, unsigned long> cuda_sm_csr_transposition_test_float_ulong;
SparseMatrixCSRTranspositionTest<Mem::CUDA, double, unsigned long> cuda_sm_csr_transposition_test_double_ulong;
#endif

template<
  typename Mem_,
  typename DT_,
  typename IT_>
class SparseMatrixCSRPermuteTest
  : public FullTaggedTest<Mem_, DT_, IT_>
{
public:
  SparseMatrixCSRPermuteTest()
    : FullTaggedTest<Mem_, DT_, IT_>("SparseMatrixCSRPermuteTest")
  {
  }

  virtual void run() const
  {
    for (Index size(25) ; size < 1e3 ; size*=2)
    {
      SparseMatrixCOO<Mem::Main, DT_, IT_> a_local(size, size);
      DenseVector<Mem::Main, DT_, IT_> x_local(size);
      for (Index i(0) ; i < size ; ++i)
      {
        x_local(i, DT_(i % 100 * DT_(1.234)));
      }
      DenseVector<Mem_, DT_, IT_> x(size);
      x.copy(x_local);

      for (Index row(0) ; row < a_local.rows() ; ++row)
      {
        for (Index col(0) ; col < a_local.columns() ; ++col)
        {
          if(row == col)
          {
            a_local(row, col, DT_(2));
          }
          else if((row == col+7) || (row+7 == col))
          {
            a_local(row, col, DT_(-1));
          }
          else if((row == col+15) || (row+15 == col))
          {
            a_local(row, col, DT_(-2));
          }
          else if((row == col+a_local.columns()/2) || (row+a_local.rows()/2 == col))
          {
            a_local(row, col, DT_(1));
          }
        }
      }
      SparseMatrixCSR<Mem_, DT_, IT_> a(a_local);

      DenseVector<Mem_, DT_, IT_> r(size);
      a.apply(r, x);
      DT_ ref_norm = r.norm2();

      auto a_backup = a.clone(CloneMode::Deep);

      SparseMatrixCSR<Mem::Main, DT_, IT_> a_main;
      a_main.convert(a);
      Adjacency::Graph graph(Adjacency::rt_as_is, a_main);

      Adjacency::Permutation perm = Adjacency::CuthillMcKee::compute(graph, true, Adjacency::CuthillMcKee::root_minimum_degree, Adjacency::CuthillMcKee::sort_desc);

      a.permute(perm, perm);
      x.permute(perm);

      a.apply(r, x);
      DT_ norm = r.norm2();
      TEST_CHECK_EQUAL_WITHIN_EPS(norm, ref_norm, 1e-3);

      a = a_backup.clone(CloneMode::Deep);
      auto perm_inv = perm.inverse();
      a.permute(perm_inv, perm);
      a.permute(perm, perm_inv);
      TEST_CHECK_EQUAL(a, a_backup);
    }
  }
};

SparseMatrixCSRPermuteTest<Mem::Main, float, unsigned long> sm_csr_permute_test_float_ulong;
SparseMatrixCSRPermuteTest<Mem::Main, double, unsigned long> sm_csr_permute_test_double_ulong;
SparseMatrixCSRPermuteTest<Mem::Main, float, unsigned int> sm_csr_permute_test_float_uint;
SparseMatrixCSRPermuteTest<Mem::Main, double, unsigned int> sm_csr_permute_test_double_uint;
#ifdef FEAST_HAVE_QUADMATH
SparseMatrixCSRPermuteTest<Mem::Main, __float128, unsigned long> sm_csr_permute_test_float128_ulong;
SparseMatrixCSRPermuteTest<Mem::Main, __float128, unsigned int> sm_csr_permute_test_float128_uint;
#endif
#ifdef FEAST_BACKENDS_CUDA
SparseMatrixCSRPermuteTest<Mem::CUDA, float, unsigned long> cuda_sm_csr_permute_test_float_ulong;
SparseMatrixCSRPermuteTest<Mem::CUDA, double, unsigned long> cuda_sm_csr_permute_test_double_ulong;
SparseMatrixCSRPermuteTest<Mem::CUDA, float, unsigned int> cuda_sm_csr_permute_test_float_uint;
SparseMatrixCSRPermuteTest<Mem::CUDA, double, unsigned int> cuda_sm_csr_permute_test_double_uint;
#endif

template<
  typename Mem_,
  typename DT_,
  typename IT_>
class SparseMatrixCSRDiagTest
  : public FullTaggedTest<Mem_, DT_, IT_>
{
public:
  SparseMatrixCSRDiagTest()
    : FullTaggedTest<Mem_, DT_, IT_>("SparseMatrixCSRDiagTest")
  {
  }

  virtual void run() const
  {
    for (Index size(2) ; size < 3e2 ; size*=2)
    {
      SparseMatrixCOO<Mem::Main, DT_, IT_> a_local(size, size);
      for (Index row(0) ; row < a_local.rows() ; ++row)
      {
        for (Index col(0) ; col < a_local.columns() ; ++col)
        {
          if(row == col)
            a_local(row, col, DT_(DT_(col % 100) / DT_(2)));
          else if((row == col+1) || (row+1 == col))
            a_local(row, col, DT_(-1));
        }
      }

      SparseMatrixCSR<Mem_, DT_, IT_> a(a_local);

      auto ref = a.create_vector_l();
      auto ref_local = a_local.create_vector_l();
      for (Index i(0) ; i < a_local.rows() ; ++i)
      {
        ref_local(i, a_local(i, i));
      }
      ref.convert(ref_local);

      auto diag = a.extract_diag();
      TEST_CHECK_EQUAL(diag, ref);
    }
  }
};

SparseMatrixCSRDiagTest<Mem::Main, float, unsigned int> sm_csr_diag_test_float_uint;
SparseMatrixCSRDiagTest<Mem::Main, double, unsigned int> sm_csr_diag_test_double_uint;
SparseMatrixCSRDiagTest<Mem::Main, float, unsigned long> sm_csr_diag_test_float_ulong;
SparseMatrixCSRDiagTest<Mem::Main, double, unsigned long> sm_csr_diag_test_double_ulong;
#ifdef FEAST_HAVE_QUADMATH
SparseMatrixCSRDiagTest<Mem::Main, __float128, unsigned int> sm_csr_diag_test_float128_uint;
SparseMatrixCSRDiagTest<Mem::Main, __float128, unsigned long> sm_csr_diag_test_float128_ulong;
#endif
#ifdef FEAST_BACKENDS_CUDA
SparseMatrixCSRDiagTest<Mem::CUDA, float, unsigned int> cuda_sm_csr_diag_test_float_uint;
SparseMatrixCSRDiagTest<Mem::CUDA, double, unsigned int> cuda_sm_csr_diag_test_double_uint;
SparseMatrixCSRDiagTest<Mem::CUDA, float, unsigned long> cuda_sm_csr_diag_test_float_ulong;
SparseMatrixCSRDiagTest<Mem::CUDA, double, unsigned long> cuda_sm_csr_diag_test_double_ulong;
#endif
