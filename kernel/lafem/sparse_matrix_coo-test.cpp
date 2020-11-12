// FEAT3: Finite Element Analysis Toolbox, Version 3
// Copyright (C) 2010 - 2020 by Stefan Turek & the FEAT group
// FEAT3 is released under the GNU General Public License version 3,
// see the file 'copyright.txt' in the top level directory for details.

#include <kernel/base_header.hpp>
#include <kernel/archs.hpp>
#include <test_system/test_system.hpp>
#include <kernel/lafem/sparse_matrix_coo.hpp>
#include <kernel/util/binary_stream.hpp>
#include <kernel/util/random.hpp>

using namespace FEAT;
using namespace FEAT::LAFEM;
using namespace FEAT::TestSystem;

/**
 * \brief Test class for the sparse matrix coo class.
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
class SparseMatrixCOOTest
  : public FullTaggedTest<Mem_, DT_, IT_>
{
public:
   SparseMatrixCOOTest()
    : FullTaggedTest<Mem_, DT_, IT_>("SparseMatrixCOOTest")
  {
  }

  virtual ~SparseMatrixCOOTest()
  {
  }

  virtual void run() const override
  {
    SparseMatrixCOO<Mem_, DT_, IT_> zero1;
    SparseMatrixCOO<Mem::Main, DT_, IT_> zero2;
    TEST_CHECK_EQUAL(zero1, zero2);

    SparseMatrixCOO<Mem_, DT_, IT_> a(10, 10);
    a(5,5,5);
    a(1,2,7);
    a(5,5,2);
    a(3,3,1);
    a(1,2,8);
    TEST_CHECK_EQUAL(a.used_elements(), 3ul);
    TEST_CHECK_EQUAL(a(1, 2), DT_(8));
    TEST_CHECK_EQUAL(a(5, 5), DT_(2));
    TEST_CHECK_EQUAL(a(3, 3), DT_(1));
    TEST_CHECK_EQUAL(a(1, 3), DT_(0));

    a.format();
    TEST_CHECK_EQUAL(a.used_elements(), Index(3));
    TEST_CHECK_EQUAL(a(5, 5), DT_(0));
    a(1,2,7);
    a(5,5,8);
    a(5,5,2);
    TEST_CHECK_EQUAL(a.used_elements(), 3ul);
    TEST_CHECK_EQUAL(a(1, 2), DT_(7));
    TEST_CHECK_EQUAL(a(5, 5), DT_(2));

    a.format();
    a(1,2,8);
    a(5,5,2);
    a(1,2,7);
    TEST_CHECK_EQUAL(a.used_elements(), 3ul);
    TEST_CHECK_EQUAL(a(1, 2), DT_(7));
    TEST_CHECK_EQUAL(a(5, 5), DT_(2));

    SparseMatrixCOO<Mem_, DT_, IT_> b;
    b.convert(a);
    TEST_CHECK_EQUAL(b.size(), a.size());
    TEST_CHECK_EQUAL(b.rows(), a.rows());
    TEST_CHECK_EQUAL(b.columns(), a.columns());
    TEST_CHECK_EQUAL(a(1,2), b(1,2));
    TEST_CHECK_EQUAL(a(0,2), b(0,2));
    TEST_CHECK_EQUAL(a, b);

    SparseMatrixCOO<Mem_, DT_, IT_> c(b.clone());
    TEST_CHECK_NOT_EQUAL((void*)c.val(), (void*)b.val());
    TEST_CHECK_NOT_EQUAL((void*)c.row_indices(), (void*)b.row_indices());
    c(1,2,3);
    TEST_CHECK_NOT_EQUAL(c, b);
    c.clone(b);
    TEST_CHECK_EQUAL(c, b);
    c(1,2,3);
    TEST_CHECK_NOT_EQUAL(c, b);

    c.copy(b);
    TEST_CHECK_EQUAL(c, b);
    TEST_CHECK_NOT_EQUAL((void*)c.row_indices(), (void*)b.row_indices());
    c(1,2,3);
    TEST_CHECK_NOT_EQUAL(c, b);
    TEST_CHECK_NOT_EQUAL((void*)c.val(), (void*)b.val());

    decltype(c) d(c.layout());
    TEST_CHECK_NOT_EQUAL((void*)d.row_indices(), (void*)c.row_indices());



    SparseMatrixCSR<Mem_, DT_, IT_> csr00;
    SparseMatrixCOO<Mem_, DT_, IT_> l00(csr00);
    TEST_CHECK_EQUAL(l00.size(), 0ul);
    TEST_CHECK_EQUAL(l00.rows(), 0ul);
    TEST_CHECK_EQUAL(l00.columns(), 0ul);
    TEST_CHECK_EQUAL(l00.used_elements(), 0ul);
    SparseMatrixCSR<Mem_, DT_, IT_> csr01(10, 10);
    SparseMatrixCOO<Mem_, DT_, IT_> l01(csr01);
    TEST_CHECK_EQUAL(l01.size(), 100ul);
    TEST_CHECK_EQUAL(l01.rows(), 10ul);
    TEST_CHECK_EQUAL(l01.columns(), 10ul);
    TEST_CHECK_EQUAL(l01.used_elements(), 0ul);
  }
};

SparseMatrixCOOTest<Mem::Main, float, unsigned int> sparse_matrix_coo_test_float_uint;
SparseMatrixCOOTest<Mem::Main, double, unsigned int> sparse_matrix_coo_test_double_uint;
SparseMatrixCOOTest<Mem::Main, float, unsigned long> sparse_matrix_coo_test_float_ulong;
SparseMatrixCOOTest<Mem::Main, double, unsigned long> sparse_matrix_coo_test_double_ulong;
#ifdef FEAT_HAVE_QUADMATH
SparseMatrixCOOTest<Mem::Main, __float128, unsigned int> sparse_matrix_coo_test_float128_uint;
SparseMatrixCOOTest<Mem::Main, __float128, unsigned long> sparse_matrix_coo_test_float128_ulong;
#endif
#ifdef FEAT_HAVE_CUDA
SparseMatrixCOOTest<Mem::CUDA, float, unsigned int> cuda_sparse_matrix_coo_test_float_uint;
SparseMatrixCOOTest<Mem::CUDA, double, unsigned int> cuda_sparse_matrix_coo_test_double_uint;
SparseMatrixCOOTest<Mem::CUDA, float, unsigned long> cuda_sparse_matrix_coo_test_float_ulong;
SparseMatrixCOOTest<Mem::CUDA, double, unsigned long> cuda_sparse_matrix_coo_test_double_ulong;
#endif

template<
  typename Mem_,
  typename DT_,
  typename IT_>
class SparseMatrixCOOSerializeTest
  : public FullTaggedTest<Mem_, DT_, IT_>
{
public:
   SparseMatrixCOOSerializeTest()
    : FullTaggedTest<Mem_, DT_, IT_>("SparseMatrixCOOSerializeTest")
  {
  }

  virtual ~SparseMatrixCOOSerializeTest()
  {
  }

  virtual void run() const override
  {
    SparseMatrixCOO<Mem_, DT_, IT_> f(10, 10);
    SparseMatrixCOO<Mem_, DT_, IT_> zfp_tester(10, 10);
    for (Index row(0) ; row < f.rows() ; ++row)
    {
      for (Index col(0) ; col < f.columns() ; ++col)
      {
        if(row == col)
        {
          f(row, col, DT_(2));
          zfp_tester(row, col, DT_(1.)/DT_(row * col + 1));
        }
        else if((row == col+1) || (row+1 == col))
        {
          f(row, col, DT_(-1));
          zfp_tester(row, col, DT_(1.)/DT_(row * col + 1));
        }
      }
    }

    BinaryStream bs;
    f.write_out(FileMode::fm_coo, bs);
    bs.seekg(0);
    SparseMatrixCOO<Mem_, DT_, IT_> g(FileMode::fm_coo, bs);
    TEST_CHECK_EQUAL(g, f);

    std::stringstream ts;
    f.write_out(FileMode::fm_mtx, ts);
    SparseMatrixCOO<Mem_, DT_, IT_> j(FileMode::fm_mtx, ts);
    TEST_CHECK_EQUAL(j, f);

    auto kp = f.serialize(LAFEM::SerialConfig(false, false));
    SparseMatrixCOO<Mem_, DT_, IT_> k(kp);
    TEST_CHECK_EQUAL(k, f);
#ifdef FEAT_HAVE_ZLIB
    auto zl = f.serialize(LAFEM::SerialConfig(true, false));
    SparseMatrixCOO<Mem_, DT_, IT_> zlib(zl);
    TEST_CHECK_EQUAL(zlib, f);
#endif
#ifdef FEAT_HAVE_ZFP
    auto zf = zfp_tester.serialize(LAFEM::SerialConfig(false, true, FEAT::Real(1e-7)));
    SparseMatrixCOO<Mem_, DT_, IT_> zfp(zf);
    for(Index i_ind(0) ; i_ind < zfp_tester.rows() ; ++i_ind)
    {
      for(Index j_ind(0) ; j_ind < zfp_tester.columns() ; ++j_ind)
      {
        TEST_CHECK_EQUAL_WITHIN_EPS(zfp_tester(i_ind,j_ind), zfp(i_ind,j_ind), DT_(1e-4));
      }
    }
#endif
  }
};
SparseMatrixCOOSerializeTest<Mem::Main, float, unsigned int> sparse_matrix_coo_serialize_test_float_uint;
SparseMatrixCOOSerializeTest<Mem::Main, double, unsigned int> sparse_matrix_coo_serialize_test_double_uint;
SparseMatrixCOOSerializeTest<Mem::Main, float, unsigned long> sparse_matrix_coo_serialize_test_float_ulong;
SparseMatrixCOOSerializeTest<Mem::Main, double, unsigned long> sparse_matrix_coo_serialize_test_double_ulong;
#ifdef FEAT_HAVE_CUDA
SparseMatrixCOOSerializeTest<Mem::CUDA, float, unsigned int> cuda_sparse_matrix_coo_serialize_test_float_uint;
SparseMatrixCOOSerializeTest<Mem::CUDA, double, unsigned int> cuda_sparse_matrix_coo_serialize_test_double_uint;
SparseMatrixCOOSerializeTest<Mem::CUDA, float, unsigned long> cuda_sparse_matrix_coo_serialize_test_float_ulong;
SparseMatrixCOOSerializeTest<Mem::CUDA, double, unsigned long> cuda_sparse_matrix_coo_serialize_test_double_ulong;
#endif

template<
  typename Mem_,
  typename DT_,
  typename IT_>
class SparseMatrixCOOApplyTest
  : public FullTaggedTest<Mem_, DT_, IT_>
{
public:
   SparseMatrixCOOApplyTest()
    : FullTaggedTest<Mem_, DT_, IT_>("SparseMatrixCOOApplyTest")
  {
  }

  virtual ~SparseMatrixCOOApplyTest()
  {
  }

  virtual void run() const override
  {
    DT_ s(DT_(4711.1));
    for (Index size(1) ; size < Index(1e3) ; size*=2)
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
      SparseMatrixCOO<Mem_,DT_, IT_> a;
      a.convert(a_local);

      DenseVector<Mem_, DT_, IT_> r(size);

      // apply-test for alpha = 0.0
      a.apply(r, x, y, DT_(0.0));
      for (Index i(0) ; i < size ; ++i)
        TEST_CHECK_EQUAL_WITHIN_EPS(y(i), r(i), DT_(1e-2));

      // apply-test for alpha = -1.0
      a.apply(r, x, y, DT_(-1.0));
      a.apply(ref, x);
      ref.scale(ref, DT_(-1.0));
      ref.axpy(ref, y);

      for (Index i(0) ; i < size ; ++i)
        TEST_CHECK_EQUAL_WITHIN_EPS(ref(i), r(i), DT_(1e-2));

      // apply-test for alpha = 4711.1
      //r.axpy(s, a, x, y);
      a.apply(r, x, y, s);
      result_local.copy(r);

      //ref.template product_matvec(a, x);
      a.apply(ref, x);
      ref.scale(ref, s);
      ref.axpy(ref, y);
      ref_local.copy(ref);

      for (Index i(0) ; i < size ; ++i)
        TEST_CHECK_EQUAL_WITHIN_EPS(result_local(i), ref_local(i), DT_(5e-2));

      a.apply(r, x);
      result_local.copy(r);

      ref_local.format();
      for (Index i(0) ; i < a_local.used_elements() ; ++i)
      {
        DT_ t = ref_local(a_local.row_indices()[i]);
        t += a_local.val()[i] * x_local(a_local.column_indices()[i]);
        ref_local(a_local.row_indices()[i], t);
      }
      for (Index i(0) ; i < size ; ++i)
        TEST_CHECK_EQUAL_WITHIN_EPS(result_local(i), ref_local(i), DT_(5e-2));
    }
  }
};

SparseMatrixCOOApplyTest<Mem::Main, float, unsigned int> sm_coo_apply_test_float_uint;
SparseMatrixCOOApplyTest<Mem::Main, double, unsigned int> sm_coo_apply_test_double_uint;
SparseMatrixCOOApplyTest<Mem::Main, float, unsigned long> sm_coo_apply_test_float_ulong;
SparseMatrixCOOApplyTest<Mem::Main, double, unsigned long> sm_coo_apply_test_double_ulong;
#ifdef FEAT_HAVE_QUADMATH
SparseMatrixCOOApplyTest<Mem::Main, __float128, unsigned int> sm_coo_apply_test_float128_uint;
SparseMatrixCOOApplyTest<Mem::Main, __float128, unsigned long> sm_coo_apply_test_float128_ulong;
#endif


template<
  typename Mem_,
  typename DT_,
  typename IT_>
class SparseMatrixCOOScaleTest
  : public FullTaggedTest<Mem_, DT_, IT_>
{
public:
   SparseMatrixCOOScaleTest()
    : FullTaggedTest<Mem_, DT_, IT_>("SparseMatrixCOOScaleTest")
  {
  }

  virtual ~SparseMatrixCOOScaleTest()
  {
  }

  virtual void run() const override
  {
    for (Index size(2) ; size < Index(3e2) ; size*=2)
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

      SparseMatrixCOO<Mem_, DT_, IT_> a;
      a.convert(a_local);
      SparseMatrixCOO<Mem_, DT_, IT_> b;
      b.clone(a);

      b.scale(a, s);
      TEST_CHECK_EQUAL(b, ref_local);

      a.scale(a, s);
      TEST_CHECK_EQUAL(a, ref_local);
    }
  }
};

SparseMatrixCOOScaleTest<Mem::Main, float, unsigned int> sm_coo_scale_test_float_uint;
SparseMatrixCOOScaleTest<Mem::Main, double, unsigned int> sm_coo_scale_test_double_uint;
SparseMatrixCOOScaleTest<Mem::Main, float, unsigned long> sm_coo_scale_test_float_ulong;
SparseMatrixCOOScaleTest<Mem::Main, double, unsigned long> sm_coo_scale_test_double_ulong;
#ifdef FEAT_HAVE_QUADMATH
SparseMatrixCOOScaleTest<Mem::Main, __float128, unsigned int> sm_coo_scale_test_float128_uint;
SparseMatrixCOOScaleTest<Mem::Main, __float128, unsigned long> sm_coo_scale_test_float128_ulong;
#endif
#ifdef FEAT_HAVE_CUDA
SparseMatrixCOOScaleTest<Mem::CUDA, float, unsigned int> cuda_sm_coo_scale_test_float_uint;
SparseMatrixCOOScaleTest<Mem::CUDA, double, unsigned int> cuda_sm_coo_scale_test_double_uint;
SparseMatrixCOOScaleTest<Mem::CUDA, float, unsigned long> cuda_sm_coo_scale_test_float_ulong;
SparseMatrixCOOScaleTest<Mem::CUDA, double, unsigned long> cuda_sm_coo_scale_test_double_ulong;
#endif


template<
  typename Mem_,
  typename DT_,
  typename IT_>
class SparseMatrixCOOScaleRowColTest
  : public FullTaggedTest<Mem_, DT_, IT_>
{
public:
   SparseMatrixCOOScaleRowColTest()
    : FullTaggedTest<Mem_, DT_, IT_>("SparseMatrixCOOScaleRowColTest")
  {
  }

  virtual ~SparseMatrixCOOScaleRowColTest()
  {
  }

  virtual void run() const override
  {
    Index size(123);
    //for (Index size(2) ; size < Index(3e2) ; size*=10)
    {
      const DT_ pi(Math::pi<DT_>());
      const DT_ eps(Math::pow(Math::eps<DT_>(), DT_(0.8)));

      SparseMatrixCOO<Mem::Main, DT_, IT_> a(size, size + 2);
      for (Index row(0) ; row < a.rows() ; ++row)
      {
        for (Index col(0) ; col < a.columns() ; ++col)
        {
          if(row == col)
            a(row, col, DT_(2));
          else if((row == col+1) || (row+1 == col))
            a(row, col, DT_(-1));
        }
      }

      SparseMatrixCOO<Mem_, DT_, IT_> b(a.clone());

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

SparseMatrixCOOScaleRowColTest<Mem::Main, float, unsigned int> sm_coo_scale_row_col_test_float_uint;
SparseMatrixCOOScaleRowColTest<Mem::Main, double, unsigned int> sm_coo_scale_row_col_test_double_uint;
SparseMatrixCOOScaleRowColTest<Mem::Main, float, unsigned long> sm_coo_scale_row_col_test_float_ulong;
SparseMatrixCOOScaleRowColTest<Mem::Main, double, unsigned long> sm_coo_scale_row_col_test_double_ulong;
#ifdef FEAT_HAVE_QUADMATH
SparseMatrixCOOScaleRowColTest<Mem::Main, __float128, unsigned int> sm_coo_scale_row_col_test_float128_uint;
SparseMatrixCOOScaleRowColTest<Mem::Main, __float128, unsigned long> sm_coo_scale_row_col_test_float128_ulong;
#endif
// #ifdef FEAT_HAVE_CUDA
// SparseMatrixCOOScaleRowColTest<Mem::CUDA, float, unsigned int> cuda_sm_coo_scale_row_col_test_float_uint;
// SparseMatrixCOOScaleRowColTest<Mem::CUDA, double, unsigned int> cuda_sm_coo_scale_row_col_test_double_uint;
// SparseMatrixCOOScaleRowColTest<Mem::CUDA, float, unsigned long> cuda_sm_coo_scale_row_col_test_float_ulong;
// SparseMatrixCOOScaleRowColTest<Mem::CUDA, double, unsigned long> cuda_sm_coo_scale_row_col_test_double_ulong;
// #endif


/**
 * \brief Test class for the transposition of a SparseMatrixCOO
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
class SparseMatrixCOOTranspositionTest
  : public FullTaggedTest<Mem_, DT_, IT_>
{

public:
  typedef SparseMatrixCOO<Mem_, DT_, IT_> MatrixType;

   SparseMatrixCOOTranspositionTest()
    : FullTaggedTest<Mem_, DT_, IT_>("SparseMatrixCOOTranspositionTest")
  {
  }

  virtual ~SparseMatrixCOOTranspositionTest()
  {
  }

  virtual void run() const override
  {
    Index size(31);
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

SparseMatrixCOOTranspositionTest<Mem::Main, float, unsigned int> sm_coo_transposition_test_float_uint;
SparseMatrixCOOTranspositionTest<Mem::Main, double, unsigned int> sm_coo_transposition_test_double_uint;
SparseMatrixCOOTranspositionTest<Mem::Main, float, unsigned long> sm_coo_transposition_test_float_ulong;
SparseMatrixCOOTranspositionTest<Mem::Main, double, unsigned long> sm_coo_transposition_test_double_ulong;
#ifdef FEAT_HAVE_QUADMATH
SparseMatrixCOOTranspositionTest<Mem::Main, __float128, unsigned int> sm_coo_transposition_test_float128_uint;
SparseMatrixCOOTranspositionTest<Mem::Main, __float128, unsigned long> sm_coo_transposition_test_float128_ulong;
#endif
#ifdef FEAT_HAVE_CUDA
SparseMatrixCOOTranspositionTest<Mem::CUDA, float, unsigned int> cuda_sm_coo_transposition_test_float_uint;
SparseMatrixCOOTranspositionTest<Mem::CUDA, double, unsigned int> cuda_sm_coo_transposition_test_double_uint;
SparseMatrixCOOTranspositionTest<Mem::CUDA, float, unsigned long> cuda_sm_coo_transposition_test_float_ulong;
SparseMatrixCOOTranspositionTest<Mem::CUDA, double, unsigned long> cuda_sm_coo_transposition_test_double_ulong;
#endif


template<
  typename Mem_,
  typename DT_,
  typename IT_>
class SparseMatrixCOOAxpyTest
  : public FullTaggedTest<Mem_, DT_, IT_>
{
public:
   SparseMatrixCOOAxpyTest()
    : FullTaggedTest<Mem_, DT_, IT_>("SparseMatrixCOOAxpyTest")
  {
  }

  virtual ~SparseMatrixCOOAxpyTest()
  {
  }

  virtual void run() const override
  {
    for (Index size(2) ; size < Index(3e2) ; size*=2)
    {
      DT_ s(DT_(4.321));

      SparseMatrixCOO<Mem::Main, DT_, IT_> a_local(size, size + 2);
      SparseMatrixCOO<Mem::Main, DT_, IT_> b_local(size, size + 2);
      SparseMatrixCOO<Mem::Main, DT_, IT_> ref_local(size, size + 2);
      for (Index row(0) ; row < a_local.rows() ; ++row)
      {
        for (Index col(0) ; col < a_local.columns() ; ++col)
        {
          if(row == col)
            a_local(row, col, DT_(2));
          else if((row == col+1) || (row+1 == col))
            a_local(row, col, DT_(-1));

          if((row == col+1) || (row+1 == col) || row==col)
          {
            b_local(row, col, DT_((row+col) % 15));
            ref_local(row, col, a_local(row, col) * s + b_local(row, col));
          }
        }
      }

      SparseMatrixCOO<Mem_, DT_, IT_> ref;
      ref.convert(ref_local);
      SparseMatrixCOO<Mem_, DT_, IT_> a;
      a.convert(a_local);
      SparseMatrixCOO<Mem_, DT_, IT_> b;
      b.convert(b_local);
      SparseMatrixCOO<Mem_, DT_, IT_> c;

      c.clone(a);
      c.axpy(c, b, s);
      TEST_CHECK_EQUAL(c, ref);

      c.clone(b);
      c.axpy(a, c, s);
      TEST_CHECK_EQUAL(c, ref);

      c.axpy(a, b, s);
      TEST_CHECK_EQUAL(c, ref);

      s = DT_(0);
      ref.clone(b);
      c.axpy(a, b, s);
      TEST_CHECK_EQUAL(c, ref);

      s = DT_(1);
      for (Index i(0) ; i < c.used_elements() ; ++i)
      {
        ref_local.val()[i] = a_local.val()[i] + b_local.val()[i];
      }
      ref.convert(ref_local);
      c.axpy(a, b, s);
      TEST_CHECK_EQUAL(c, ref);

      s = DT_(-1);
      for (Index i(0) ; i < c.used_elements() ; ++i)
      {
        ref_local.val()[i] = b_local.val()[i] - a_local.val()[i];
      }
      ref.convert(ref_local);
      c.axpy(a, b, s);
      TEST_CHECK_EQUAL(c, ref);
    }
  }
};

SparseMatrixCOOAxpyTest<Mem::Main, float, unsigned int> sm_coo_axpy_test_float_uint;
SparseMatrixCOOAxpyTest<Mem::Main, double, unsigned int> sm_coo_axpy_test_double_uint;
SparseMatrixCOOAxpyTest<Mem::Main, float, unsigned long> sm_coo_axpy_test_float_ulong;
SparseMatrixCOOAxpyTest<Mem::Main, double, unsigned long> sm_coo_axpy_test_double_ulong;
#ifdef FEAT_HAVE_QUADMATH
SparseMatrixCOOAxpyTest<Mem::Main, __float128, unsigned int> sm_coo_axpy_test_float128_uint;
SparseMatrixCOOAxpyTest<Mem::Main, __float128, unsigned long> sm_coo_axpy_test_float128_ulong;
#endif
#ifdef FEAT_HAVE_CUDA
SparseMatrixCOOAxpyTest<Mem::CUDA, float, unsigned int> cuda_sm_coo_axpy_test_float_uint;
SparseMatrixCOOAxpyTest<Mem::CUDA, double, unsigned int> cuda_sm_coo_axpy_test_double_uint;
SparseMatrixCOOAxpyTest<Mem::CUDA, float, unsigned long> cuda_sm_coo_axpy_test_float_ulong;
SparseMatrixCOOAxpyTest<Mem::CUDA, double, unsigned long> cuda_sm_coo_axpy_test_double_ulong;
#endif


template<
  typename Mem_,
  typename DT_,
  typename IT_>
class SparseMatrixCOOFrobeniusTest
  : public FullTaggedTest<Mem_, DT_, IT_>
{
public:
   SparseMatrixCOOFrobeniusTest()
    : FullTaggedTest<Mem_, DT_, IT_>("SparseMatrixCOOFrobeniusTest")
  {
  }

  virtual ~SparseMatrixCOOFrobeniusTest()
  {
  }

  virtual void run() const override
  {
    for (Index size(2) ; size < Index(3e2) ; size*=2)
    {
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

      SparseMatrixCOO<Mem_, DT_, IT_> a;
      a.convert(a_local);

      DenseVector<Mem_, DT_, IT_> refv(a.used_elements(), a.val());
      DT_ ref = refv.norm2();
      DT_ c = a.norm_frobenius();
      TEST_CHECK_EQUAL(c, ref);
    }
  }
};

SparseMatrixCOOFrobeniusTest<Mem::Main, float, unsigned int> sm_coo_frobenius_test_float_uint;
SparseMatrixCOOFrobeniusTest<Mem::Main, double, unsigned int> sm_coo_frobenius_test_double_uint;
SparseMatrixCOOFrobeniusTest<Mem::Main, float, unsigned long> sm_coo_frobenius_test_float_ulong;
SparseMatrixCOOFrobeniusTest<Mem::Main, double, unsigned long> sm_coo_frobenius_test_double_ulong;
#ifdef FEAT_HAVE_QUADMATH
SparseMatrixCOOFrobeniusTest<Mem::Main, __float128, unsigned int> sm_coo_frobenius_test_float128_uint;
SparseMatrixCOOFrobeniusTest<Mem::Main, __float128, unsigned long> sm_coo_frobenius_test_float128_ulong;
#endif
#ifdef FEAT_HAVE_CUDA
SparseMatrixCOOFrobeniusTest<Mem::CUDA, float, unsigned int> cuda_sm_coo_frobenius_test_float_uint;
SparseMatrixCOOFrobeniusTest<Mem::CUDA, double, unsigned int> cuda_sm_coo_frobenius_test_double_uint;
SparseMatrixCOOFrobeniusTest<Mem::CUDA, float, unsigned long> cuda_sm_coo_frobenius_test_float_ulong;
SparseMatrixCOOFrobeniusTest<Mem::CUDA, double, unsigned long> cuda_sm_coo_frobenius_test_double_ulong;
#endif
