// FEAT3: Finite Element Analysis Toolbox, Version 3
// Copyright (C) 2010 - 2021 by Stefan Turek & the FEAT group
// FEAT3 is released under the GNU General Public License version 3,
// see the file 'copyright.txt' in the top level directory for details.

#include <kernel/base_header.hpp>
#include <kernel/archs.hpp>
#include <test_system/test_system.hpp>
#include <kernel/lafem/sparse_matrix_bcsr.hpp>
#include <kernel/util/binary_stream.hpp>
#include <kernel/adjacency/cuthill_mckee.hpp>

using namespace FEAT;
using namespace FEAT::LAFEM;
using namespace FEAT::TestSystem;

/**
 * \brief Test class for the sparse matrix csr blocked class.
 *
 * \test test description missing
 *
 * \author Dirk Ribbrock
 */
template<
  typename Mem_,
  typename DT_,
  typename IT_>
class SparseMatrixBCSRTest
  : public FullTaggedTest<Mem_, DT_, IT_>
{
public:
   SparseMatrixBCSRTest()
    : FullTaggedTest<Mem_, DT_, IT_>("SparseMatrixBCSRTest")
  {
  }

  virtual ~SparseMatrixBCSRTest()
  {
  }

  void test_vector_types() const
  {
    // define a hand-full of sparse matrix BCSR
    SparseMatrixBCSR<Mem_, DT_, IT_, 3, 3> bcsr_3x3;
    SparseMatrixBCSR<Mem_, DT_, IT_, 3, 1> bcsr_3x1;
    SparseMatrixBCSR<Mem_, DT_, IT_, 1, 3> bcsr_1x3;
    SparseMatrixBCSR<Mem_, DT_, IT_, 1, 1> bcsr_1x1;

    // now create the left/right vectors
    TEST_CHECK_EQUAL(bcsr_3x3.create_vector_l().name(), "DenseVectorBlocked");
    TEST_CHECK_EQUAL(bcsr_3x3.create_vector_r().name(), "DenseVectorBlocked");
    TEST_CHECK_EQUAL(bcsr_3x1.create_vector_l().name(), "DenseVectorBlocked");
    TEST_CHECK_EQUAL(bcsr_3x1.create_vector_r().name(), "DenseVector");
    TEST_CHECK_EQUAL(bcsr_1x3.create_vector_l().name(), "DenseVector");
    TEST_CHECK_EQUAL(bcsr_1x3.create_vector_r().name(), "DenseVectorBlocked");
    TEST_CHECK_EQUAL(bcsr_1x1.create_vector_l().name(), "DenseVector");
    TEST_CHECK_EQUAL(bcsr_1x1.create_vector_r().name(), "DenseVector");
  }

  virtual void run() const override
  {
    test_vector_types();

    SparseMatrixBCSR<Mem_, DT_, IT_, 2, 3> zero1;
    SparseMatrixBCSR<Mem_, DT_, IT_, 2, 3> zero2;
    TEST_CHECK_EQUAL(zero2, zero1);
    zero2.convert(zero1);

    DenseVector<Mem_, DT_, IT_> dv1(12);
    for (Index i(0) ; i < dv1.size() ; ++i)
      dv1(i, DT_(i+1)/DT_(7*i+1));
    DenseVector<Mem_, IT_, IT_> dv2(2);
    dv2(0, IT_(0));
    dv2(1, IT_(1));
    DenseVector<Mem_, IT_, IT_> dv3(3);
    dv3(0, IT_(0));
    dv3(1, IT_(1));
    dv3(2, IT_(2));
    SparseMatrixBCSR<Mem_, DT_, IT_, 2, 3> c(2, 2, dv2, dv1, dv3);

    TEST_CHECK_EQUAL(c(1,0)(0,0), DT_(0));
    TEST_CHECK_EQUAL(c(1,1)(1,1), DT_(10+1)/DT_(7*10+1));

    SparseMatrixBCSR<Mem_, DT_, IT_, 2, 3> d;
    d.convert(c);
    TEST_CHECK_EQUAL(d.rows(), c.rows());
    TEST_CHECK_EQUAL(d.columns(), c.columns());
    TEST_CHECK_EQUAL(d.used_elements(), c.used_elements());
    TEST_CHECK_EQUAL(d, c);
    TEST_CHECK_EQUAL((void*)d.template val<Perspective::pod>(), (void*)c.template val<Perspective::pod>());
    TEST_CHECK_EQUAL((void*)d.row_ptr(), (void*)c.row_ptr());
    SparseMatrixBCSR<Mem_, DT_, IT_, 2, 3> e;
    e.clone(c);
    TEST_CHECK_EQUAL(e, c);
    TEST_CHECK_NOT_EQUAL((void*)e.template val<Perspective::pod>(), (void*)c.template val<Perspective::pod>());
    TEST_CHECK_EQUAL((void*)e.row_ptr(), (void*)c.row_ptr());
    e = c.clone(CloneMode::Deep);
    TEST_CHECK_EQUAL(e, c);
    TEST_CHECK_NOT_EQUAL((void*)e.template val<Perspective::pod>(), (void*)c.template val<Perspective::pod>());
    TEST_CHECK_NOT_EQUAL((void*)e.row_ptr(), (void*)c.row_ptr());

    SparseMatrixBCSR<Mem_, DT_, IT_, 2, 3> f(c.layout());
    TEST_CHECK_EQUAL(f.rows(), c.rows());
    TEST_CHECK_EQUAL(f.columns(), c.columns());
    TEST_CHECK_EQUAL(f.used_elements(), c.used_elements());
    TEST_CHECK_NOT_EQUAL((void*)f.template val<Perspective::pod>(), (void*)c.template val<Perspective::pod>());
    TEST_CHECK_EQUAL((void*)f.row_ptr(), (void*)c.row_ptr());

  }
};
SparseMatrixBCSRTest<Mem::Main, float, unsigned long> cpu_sparse_matrix_bcsr_test_float_ulong;
SparseMatrixBCSRTest<Mem::Main, double, unsigned long> cpu_sparse_matrix_bcsr_test_double_ulong;
SparseMatrixBCSRTest<Mem::Main, float, unsigned int> cpu_sparse_matrix_bcsr_test_float_uint;
SparseMatrixBCSRTest<Mem::Main, double, unsigned int> cpu_sparse_matrix_bcsr_test_double_uint;
#ifdef FEAT_HAVE_QUADMATH
SparseMatrixBCSRTest<Mem::Main, __float128, unsigned long> cpu_sparse_matrix_bcsr_test_float128_ulong;
SparseMatrixBCSRTest<Mem::Main, __float128, unsigned int> cpu_sparse_matrix_bcsr_test_float128_uint;
#endif
#ifdef FEAT_HAVE_CUDA
SparseMatrixBCSRTest<Mem::CUDA, float, unsigned long> cuda_sparse_matrix_bcsr_test_float_ulong;
SparseMatrixBCSRTest<Mem::CUDA, double, unsigned long> cuda_sparse_matrix_bcsr_test_double_ulong;
SparseMatrixBCSRTest<Mem::CUDA, float, unsigned int> cuda_sparse_matrix_bcsr_test_float_uint;
SparseMatrixBCSRTest<Mem::CUDA, double, unsigned int> cuda_sparse_matrix_bcsr_test_double_uint;
#endif

template<
  typename Mem_,
  typename DT_,
  typename IT_>
class SparseMatrixBCSRSerializeTest
  : public FullTaggedTest<Mem_, DT_, IT_>
{
public:
   SparseMatrixBCSRSerializeTest()
    : FullTaggedTest<Mem_, DT_, IT_>("SparseMatrixBCSRSerializeTest")
  {
  }

  virtual ~SparseMatrixBCSRSerializeTest()
  {
  }
  virtual void run() const override
  {
    DenseVector<Mem_, DT_, IT_> dv1(12);
    for (Index i(0) ; i < dv1.size() ; ++i)
      dv1(i, DT_(i+1)/DT_(7*i+1));
    DenseVector<Mem_, IT_, IT_> dv2(2);
    dv2(0, IT_(0));
    dv2(1, IT_(1));
    DenseVector<Mem_, IT_, IT_> dv3(3);
    dv3(0, IT_(0));
    dv3(1, IT_(1));
    dv3(2, IT_(2));
    SparseMatrixBCSR<Mem_, DT_, IT_, 2, 3> c(2, 2, dv2, dv1, dv3);

    BinaryStream bs;
    c.write_out(FileMode::fm_bcsr, bs);
    bs.seekg(0);
    SparseMatrixBCSR<Mem_, DT_, IT_, 2, 3> g(FileMode::fm_bcsr, bs);
    TEST_CHECK_EQUAL(g, c);

    /*std::stringstream ts;
    f.write_out(FileMode::fm_mtx, ts);
    SparseMatrixCSR<Mem::Main, DT_, IT_> j(FileMode::fm_mtx, ts);
    TEST_CHECK_EQUAL(j, f);

    std::stringstream ts2;
    f.write_out_mtx(ts2, true);
    SparseMatrixCSR<Mem::Main, DT_, IT_> j2(FileMode::fm_mtx, ts2);
    TEST_CHECK_EQUAL(j2, f);*/

    auto kp = c.serialize(LAFEM::SerialConfig(false, false));
    SparseMatrixBCSR<Mem_, DT_, IT_, 2, 3> k(kp);
    TEST_CHECK_EQUAL(k, c);
#ifdef FEAT_HAVE_ZLIB
    auto zl = c.serialize(LAFEM::SerialConfig(true, false));
    SparseMatrixBCSR<Mem_, DT_, IT_, 2, 3> zlib(zl);
    TEST_CHECK_EQUAL(k, c);
#endif
#ifdef FEAT_HAVE_ZFP
    auto zf = c.serialize(LAFEM::SerialConfig(false, true, FEAT::Real(1e-7)));
    SparseMatrixBCSR<Mem_, DT_, IT_, 2, 3> zfp(zf);
    for(Index i(0) ; i < c.rows() ; ++i)
    {
      for(Index j(0) ; j < c.columns() ; ++j)
      {
        for(int q(0) ; q < c(i,j).m ; ++q)
        {
          for(int w(0) ; w < c(i,j).n ; ++w)
          {
            TEST_CHECK_EQUAL_WITHIN_EPS(c(i,j)(q,w), zfp(i,j)(q,w), DT_(1e-5));
          }
        }
      }
    }
#endif
  }
};

SparseMatrixBCSRSerializeTest<Mem::Main, float, unsigned long> cpu_sparse_matrix_bcsr_serialize_test_float_ulong;
SparseMatrixBCSRSerializeTest<Mem::Main, double, unsigned long> cpu_sparse_matrix_bcsr_serialize_test_double_ulong;
SparseMatrixBCSRSerializeTest<Mem::Main, float, unsigned int> cpu_sparse_matrix_bcsr_serialize_test_float_uint;
SparseMatrixBCSRSerializeTest<Mem::Main, double, unsigned int> cpu_sparse_matrix_bcsr_serialize_test_double_uint;
#ifdef FEAT_HAVE_CUDA
SparseMatrixBCSRSerializeTest<Mem::CUDA, float, unsigned long> cuda_sparse_matrix_bcsr_serialize_test_float_ulong;
SparseMatrixBCSRSerializeTest<Mem::CUDA, double, unsigned long> cuda_sparse_matrix_bcsr_serialize_test_double_ulong;
SparseMatrixBCSRSerializeTest<Mem::CUDA, float, unsigned int> cuda_sparse_matrix_bcsr_serialize_test_float_uint;
SparseMatrixBCSRSerializeTest<Mem::CUDA, double, unsigned int> cuda_sparse_matrix_bcsr_serialize_test_double_uint;
#endif
/**
 * \brief Test class for the sparse matrix csr blocked apply method.
 *
 * \test test description missing
 *
 * \author Dirk Ribbrock
 */
template<
  typename Mem_,
  typename DT_,
  typename IT_>
class SparseMatrixBCSRApplyTest
  : public FullTaggedTest<Mem_, DT_, IT_>
{
public:
   SparseMatrixBCSRApplyTest()
    : FullTaggedTest<Mem_, DT_, IT_>("SparseMatrixBCSRApplyTest")
  {
  }

  virtual ~SparseMatrixBCSRApplyTest()
  {
  }

  virtual void run() const override
  {
    DenseVector<Mem_, DT_, IT_> dv1(12);
    for (Index i(0) ; i < dv1.size() ; ++i)
      dv1(i, DT_(i+1));
    DenseVector<Mem_, IT_, IT_> dv2(2);
    dv2(0, IT_(0));
    dv2(1, IT_(1));
    DenseVector<Mem_, IT_, IT_> dv3(3);
    dv3(0, IT_(0));
    dv3(1, IT_(1));
    dv3(2, IT_(2));
    SparseMatrixBCSR<Mem_, DT_, IT_, 2, 3> c(2, 2, dv2, dv1, dv3);

    DenseVector<Mem_, DT_, IT_> x(c.template columns<Perspective::pod>());
    DenseVector<Mem_, DT_, IT_> y(c.template rows<Perspective::pod>());
    DenseVector<Mem_, DT_, IT_> r(c.template rows<Perspective::pod>());
    DenseVector<Mem_, DT_, IT_> ref(c.template rows<Perspective::pod>());
    for (Index i(0) ; i < x.size() ; ++i)
    {
      x(i, DT_(i));
    }
    for (Index i(0) ; i < r.size() ; ++i)
    {
      r(i, DT_(4711));
      ref(i, DT_(4711));
      y(i, DT_(i % 100));
    }
    DenseVectorBlocked<Mem_, DT_, IT_, 3> xb(x);
    DenseVectorBlocked<Mem_, DT_, IT_, 2> yb(y);
    DenseVectorBlocked<Mem_, DT_, IT_, 2> rb(r);

    SparseMatrixCSR<Mem_, DT_, IT_> csr;
    csr.convert(c);
    csr.apply(ref, x);

    c.apply(r, x);

    TEST_CHECK_EQUAL(r, ref);

    c.apply(rb, x);
    r.convert(rb);
    TEST_CHECK_EQUAL(r, ref);

    c.apply(r, xb);
    TEST_CHECK_EQUAL(r, ref);

    c.apply(rb, xb);
    r.convert(rb);
    TEST_CHECK_EQUAL(r, ref);

    //defect
    DT_ alpha(-1);
    csr.apply(ref, x, y, alpha);
    c.apply(r, x, y, alpha);
    TEST_CHECK_EQUAL(r, ref);

    // &r == &y
    r.copy(y);
    c.apply(r, x, r, alpha);
    TEST_CHECK_EQUAL(r, ref);

    c.apply(rb, x, yb, alpha);
    r.convert(rb);
    TEST_CHECK_EQUAL(r, ref);

    c.apply(r, xb, y, alpha);
    TEST_CHECK_EQUAL(r, ref);

    c.apply(rb, xb, yb, alpha);
    r.convert(rb);
    TEST_CHECK_EQUAL(r, ref);


    //axpy
    alpha = DT_(1.234);
    csr.apply(ref, x, y, alpha);
    c.apply(r, x, y, alpha);
    TEST_CHECK_EQUAL(r, ref);

    // &r == &y
    r.copy(y);
    c.apply(r, x, r, alpha);
    TEST_CHECK_EQUAL(r, ref);

    c.apply(rb, x, yb, alpha);
    r.convert(rb);
    TEST_CHECK_EQUAL(r, ref);

    c.apply(r, xb, y, alpha);
    TEST_CHECK_EQUAL(r, ref);

    c.apply(rb, xb, y, alpha);
    r.convert(rb);
    TEST_CHECK_EQUAL(r, ref);

    c.apply(rb, xb, yb, alpha);
    r.convert(rb);
    TEST_CHECK_EQUAL(r, ref);
  }
};
SparseMatrixBCSRApplyTest<Mem::Main, float, unsigned long> cpu_sparse_matrix_bcsr_apply_test_float_ulong;
SparseMatrixBCSRApplyTest<Mem::Main, double, unsigned long> cpu_sparse_matrix_bcsr_apply_test_double_ulong;
SparseMatrixBCSRApplyTest<Mem::Main, float, unsigned int> cpu_sparse_matrix_bcsr_apply_test_float_uint;
SparseMatrixBCSRApplyTest<Mem::Main, double, unsigned int> cpu_sparse_matrix_bcsr_apply_test_double_uint;
#ifdef FEAT_HAVE_QUADMATH
SparseMatrixBCSRApplyTest<Mem::Main, __float128, unsigned long> cpu_sparse_matrix_bcsr_apply_test_float128_ulong;
SparseMatrixBCSRApplyTest<Mem::Main, __float128, unsigned int> cpu_sparse_matrix_bcsr_apply_test_float128_uint;
#endif


/**
 * \brief Test class for the sparse matrix csr blocked apply method.
 *
 * \test test description missing
 *
 * \author Dirk Ribbrock
 */
template<
  typename Mem_,
  typename DT_,
  typename IT_>
class SparseMatrixBCSRApplySquareTest
  : public FullTaggedTest<Mem_, DT_, IT_>
{
public:
   SparseMatrixBCSRApplySquareTest()
    : FullTaggedTest<Mem_, DT_, IT_>("SparseMatrixBCSRApplySquareTest")
  {
  }

  virtual ~SparseMatrixBCSRApplySquareTest()
  {
  }

  virtual void run() const override
  {
    DenseVector<Mem_, DT_, IT_> dv1(18);
    for (Index i(0) ; i < dv1.size() ; ++i)
      dv1(i, DT_(i+1));
    DenseVector<Mem_, IT_, IT_> dv2(2);
    dv2(0, IT_(0));
    dv2(1, IT_(1));
    DenseVector<Mem_, IT_, IT_> dv3(3);
    dv3(0, IT_(0));
    dv3(1, IT_(1));
    dv3(2, IT_(2));
    SparseMatrixBCSR<Mem_, DT_, IT_, 3, 3> c(2, 2, dv2, dv1, dv3);

    DenseVector<Mem_, DT_, IT_> x(c.template columns<Perspective::pod>());
    DenseVector<Mem_, DT_, IT_> y(c.template rows<Perspective::pod>());
    DenseVector<Mem_, DT_, IT_> r(c.template rows<Perspective::pod>());
    DenseVector<Mem_, DT_, IT_> ref(c.template rows<Perspective::pod>());
    for (Index i(0) ; i < x.size() ; ++i)
    {
      x(i, DT_(i));
    }
    for (Index i(0) ; i < r.size() ; ++i)
    {
      r(i, DT_(4711));
      ref(i, DT_(4711));
      y(i, DT_(i % 100));
    }
    DenseVectorBlocked<Mem_, DT_, IT_, 3> xb(x);
    DenseVectorBlocked<Mem_, DT_, IT_, 3> yb(y);
    DenseVectorBlocked<Mem_, DT_, IT_, 3> rb(r);

    SparseMatrixCSR<Mem_, DT_, IT_> csr;
    csr.convert(c);
    csr.apply(ref, x);

    c.apply(r, x);

    TEST_CHECK_EQUAL(r, ref);

    c.apply(rb, x);
    r.convert(rb);
    TEST_CHECK_EQUAL(r, ref);

    c.apply(r, xb);
    TEST_CHECK_EQUAL(r, ref);

    c.apply(rb, xb);
    r.convert(rb);
    TEST_CHECK_EQUAL(r, ref);

    // defect
    DT_ alpha(-1);
    csr.apply(ref, x, y, alpha);
    c.apply(r, x, y, alpha);
    TEST_CHECK_EQUAL(r, ref);

    // &r == &y
    r.copy(y);
    c.apply(r, x, r, alpha);
    TEST_CHECK_EQUAL(r, ref);

    c.apply(rb, x, yb, alpha);
    r.convert(rb);
    TEST_CHECK_EQUAL(r, ref);

    c.apply(r, xb, y, alpha);
    TEST_CHECK_EQUAL(r, ref);

    c.apply(rb, xb, yb, alpha);
    r.convert(rb);
    TEST_CHECK_EQUAL(r, ref);

    // axpy
    alpha = DT_(1.234);
    csr.apply(ref, x, y, alpha);
    c.apply(r, x, y, alpha);
    for (Index i(0) ; i < r.size() ; ++i)
      TEST_CHECK_EQUAL_WITHIN_EPS(r(i), ref(i), DT_(1e-3));

    // &r == &y
    r.copy(y);
    c.apply(r, x, y, alpha);
    for (Index i(0) ; i < r.size() ; ++i)
      TEST_CHECK_EQUAL_WITHIN_EPS(r(i), ref(i), DT_(1e-3));

    c.apply(rb, x, yb, alpha);
    r.convert(rb);
    for (Index i(0) ; i < r.size() ; ++i)
      TEST_CHECK_EQUAL_WITHIN_EPS(r(i), ref(i), DT_(1e-3));

    c.apply(r, xb, y, alpha);
    for (Index i(0) ; i < r.size() ; ++i)
      TEST_CHECK_EQUAL_WITHIN_EPS(r(i), ref(i), DT_(1e-3));

    c.apply(rb, xb, yb, alpha);
    r.convert(rb);
    for (Index i(0) ; i < r.size() ; ++i)
      TEST_CHECK_EQUAL_WITHIN_EPS(r(i), ref(i), DT_(1e-3));

    c.apply(rb, xb, y, alpha);
    r.convert(rb);
    for (Index i(0) ; i < r.size() ; ++i)
      TEST_CHECK_EQUAL_WITHIN_EPS(r(i), ref(i), DT_(1e-3));
  }
};
SparseMatrixBCSRApplySquareTest<Mem::Main, float, unsigned long> cpu_sparse_matrix_bcsr_apply_square_test_float_ulong;
SparseMatrixBCSRApplySquareTest<Mem::Main, double, unsigned long> cpu_sparse_matrix_bcsr_apply_square_test_double_ulong;
SparseMatrixBCSRApplySquareTest<Mem::Main, float, unsigned int> cpu_sparse_matrix_bcsr_apply_square_test_float_uint;
SparseMatrixBCSRApplySquareTest<Mem::Main, double, unsigned int> cpu_sparse_matrix_bcsr_apply_square_test_double_uint;
#ifdef FEAT_HAVE_QUADMATH
SparseMatrixBCSRApplySquareTest<Mem::Main, __float128, unsigned long> cpu_sparse_matrix_bcsr_apply_square_test_float128_ulong;
SparseMatrixBCSRApplySquareTest<Mem::Main, __float128, unsigned int> cpu_sparse_matrix_bcsr_apply_square_test_float128_uint;
#endif
#ifdef FEAT_HAVE_CUDA
SparseMatrixBCSRApplySquareTest<Mem::CUDA, float, unsigned int> gpu_sparse_matrix_bcsr_apply_square_test_float_uint;
SparseMatrixBCSRApplySquareTest<Mem::CUDA, double, unsigned int> gpu_sparse_matrix_bcsr_apply_square_test_double_uint;
SparseMatrixBCSRApplySquareTest<Mem::CUDA, float, unsigned long> gpu_sparse_matrix_bcsr_apply_square_test_float_ulong;
SparseMatrixBCSRApplySquareTest<Mem::CUDA, double, unsigned long> gpu_sparse_matrix_bcsr_apply_square_test_double_ulong;
#endif


template<
  typename Mem_,
  typename DT_,
  typename IT_>
class SparseMatrixBCSRDiagTest
  : public FullTaggedTest<Mem_, DT_, IT_>
{
public:
   SparseMatrixBCSRDiagTest()
    : FullTaggedTest<Mem_, DT_, IT_>("SparseMatrixBCSRDiagTest")
  {
  }

  virtual ~SparseMatrixBCSRDiagTest()
  {
  }

  virtual void run() const override
  {
    DenseVector<Mem_, DT_, IT_> dv1(18);
    for (Index i(0) ; i < dv1.size() ; ++i)
      dv1(i, DT_(i+1));
    DenseVector<Mem_, IT_, IT_> dv2(3);
    dv2(0, IT_(0));
    dv2(1, IT_(1));
    dv2(2, IT_(2));
    DenseVector<Mem_, IT_, IT_> dv3(3);
    dv3(0, IT_(0));
    dv3(1, IT_(1));
    dv3(2, IT_(2));
    SparseMatrixBCSR<Mem_, DT_, IT_, 3, 3> smb(2, 2, dv2, dv1, dv3);

    auto diag = smb.extract_diag();
    typedef decltype(diag) DiagType;
    typename DiagType::template ContainerTypeByMDI<Mem::Main, DT_, IT_> diag_main;
    diag_main.convert(diag);
    TEST_CHECK_EQUAL(diag_main.template elements<Perspective::pod>()[0], 1);
    TEST_CHECK_EQUAL(diag_main.template elements<Perspective::pod>()[1], 5);
    TEST_CHECK_EQUAL(diag_main.template elements<Perspective::pod>()[2], 9);
    TEST_CHECK_EQUAL(diag_main.template elements<Perspective::pod>()[3], 10);
    TEST_CHECK_EQUAL(diag_main.template elements<Perspective::pod>()[4], 14);
    TEST_CHECK_EQUAL(diag_main.template elements<Perspective::pod>()[5], 18);
  }
};
SparseMatrixBCSRDiagTest<Mem::Main, float, unsigned long> cpu_sparse_matrix_bcsr_diag_test_float_ulong;
SparseMatrixBCSRDiagTest<Mem::Main, double, unsigned long> cpu_sparse_matrix_bcsr_diag_test_double_ulong;
SparseMatrixBCSRDiagTest<Mem::Main, float, unsigned int> cpu_sparse_matrix_bcsr_diag_test_float_uint;
SparseMatrixBCSRDiagTest<Mem::Main, double, unsigned int> cpu_sparse_matrix_bcsr_diag_test_double_uint;
#ifdef FEAT_HAVE_QUADMATH
SparseMatrixBCSRDiagTest<Mem::Main, __float128, unsigned long> cpu_sparse_matrix_bcsr_diag_test_float128_ulong;
SparseMatrixBCSRDiagTest<Mem::Main, __float128, unsigned int> cpu_sparse_matrix_bcsr_diag_test_float128_uint;
#endif
#ifdef FEAT_HAVE_CUDA
SparseMatrixBCSRDiagTest<Mem::CUDA, float, unsigned long> cpu_sparse_matrix_bcsr_diag_test_float_ulong_cuda;
SparseMatrixBCSRDiagTest<Mem::CUDA, double, unsigned long> cpu_sparse_matrix_bcsr_diag_test_double_ulong_cuda;
SparseMatrixBCSRDiagTest<Mem::CUDA, float, unsigned int> cpu_sparse_matrix_bcsr_diag_test_float_uint_cuda;
SparseMatrixBCSRDiagTest<Mem::CUDA, double, unsigned int> cpu_sparse_matrix_bcsr_diag_test_double_uint_cuda;
#endif


/**
 * \brief Test class for the sparse matrix csr blocked scale method.
 *
 * \test test description missing
 *
 * \author Dirk Ribbrock
 */
template<
  typename Mem_,
  typename DT_,
  typename IT_>
class SparseMatrixBCSRScaleTest
  : public FullTaggedTest<Mem_, DT_, IT_>
{
public:
   SparseMatrixBCSRScaleTest()
    : FullTaggedTest<Mem_, DT_, IT_>("SparseMatrixBCSRScaleTest")
  {
  }

  virtual ~SparseMatrixBCSRScaleTest()
  {
  }

  virtual void run() const override
  {
    DenseVector<Mem_, DT_, IT_> dv1(12);
    for (Index i(0) ; i < dv1.size() ; ++i)
    {
      dv1(i, DT_(i+1));
    }
    DenseVector<Mem_, IT_, IT_> dv2(2);
    dv2(0, IT_(0));
    dv2(1, IT_(1));
    DenseVector<Mem_, IT_, IT_> dv3(3);
    dv3(0, IT_(0));
    dv3(1, IT_(1));
    dv3(2, IT_(2));
    SparseMatrixBCSR<Mem_, DT_, IT_, 2, 3> a(2, 2, dv2, dv1, dv3);
    SparseMatrixBCSR<Mem_, DT_, IT_, 2, 3> c(a.layout());

    DT_ scal = DT_(4711);

    SparseMatrixCSR<Mem_, DT_, IT_> a_s;
    a_s.convert(a);
    SparseMatrixCSR<Mem_, DT_, IT_> ref(a_s.layout());
    SparseMatrixCSR<Mem_, DT_, IT_> result_s;
    ref.scale(a_s, scal);

    c.scale(a, scal);
    result_s.convert(c);
    TEST_CHECK_EQUAL(result_s, ref);

    a.scale(a, scal);
    result_s.convert(a);
    TEST_CHECK_EQUAL(result_s, ref);
  }
};
SparseMatrixBCSRScaleTest<Mem::Main, float, unsigned long> cpu_sparse_matrix_bcsr_scale_test_float_ulong;
SparseMatrixBCSRScaleTest<Mem::Main, double, unsigned long> cpu_sparse_matrix_bcsr_scale_test_double_ulong;
SparseMatrixBCSRScaleTest<Mem::Main, float, unsigned int> cpu_sparse_matrix_bcsr_scale_test_float_uint;
SparseMatrixBCSRScaleTest<Mem::Main, double, unsigned int> cpu_sparse_matrix_bcsr_scale_test_double_uint;
#ifdef FEAT_HAVE_QUADMATH
SparseMatrixBCSRScaleTest<Mem::Main, __float128, unsigned long> cpu_sparse_matrix_bcsr_scale_test_float128_ulong;
SparseMatrixBCSRScaleTest<Mem::Main, __float128, unsigned int> cpu_sparse_matrix_bcsr_scale_test_float128_uint;
#endif
#ifdef FEAT_HAVE_CUDA
SparseMatrixBCSRScaleTest<Mem::CUDA, float, unsigned long> cuda_sparse_matrix_bcsr_scale_test_float_ulong;
SparseMatrixBCSRScaleTest<Mem::CUDA, double, unsigned long> cuda_sparse_matrix_bcsr_scale_test_double_ulong;
SparseMatrixBCSRScaleTest<Mem::CUDA, float, unsigned int> cuda_sparse_matrix_bcsr_scale_test_float_uint;
SparseMatrixBCSRScaleTest<Mem::CUDA, double, unsigned int> cuda_sparse_matrix_bcsr_scale_test_double_uint;
#endif


/**
 * \brief Test class for the sparse matrix csr blocked norm method.
 *
 * \test test description missing
 *
 * \author Dirk Ribbrock
 */
template<
  typename Mem_,
  typename DT_,
  typename IT_>
class SparseMatrixBCSRNormTest
  : public FullTaggedTest<Mem_, DT_, IT_>
{
public:
   SparseMatrixBCSRNormTest()
    : FullTaggedTest<Mem_, DT_, IT_>("SparseMatrixBCSRNormTest")
  {
  }

  virtual ~SparseMatrixBCSRNormTest()
  {
  }

  virtual void run() const override
  {
    DenseVector<Mem_, DT_, IT_> dv1(12);
    for (Index i(0) ; i < dv1.size() ; ++i)
    {
      dv1(i, DT_(i+1));
    }
    DenseVector<Mem_, IT_, IT_> dv2(2);
    dv2(0, IT_(0));
    dv2(1, IT_(1));
    DenseVector<Mem_, IT_, IT_> dv3(3);
    dv3(0, IT_(0));
    dv3(1, IT_(1));
    dv3(2, IT_(2));
    SparseMatrixBCSR<Mem_, DT_, IT_, 2, 3> a(2, 2, dv2, dv1, dv3);

    SparseMatrixCSR<Mem_, DT_, IT_> a_s;
    a_s.convert(a);
    DT_ ref = a_s.norm_frobenius();

    DT_ result = a.norm_frobenius();
    TEST_CHECK_EQUAL(result, ref);
  }
};
SparseMatrixBCSRNormTest<Mem::Main, float, unsigned long> cpu_sparse_matrix_bcsr_norm_test_float_ulong;
SparseMatrixBCSRNormTest<Mem::Main, double, unsigned long> cpu_sparse_matrix_bcsr_norm_test_double_ulong;
SparseMatrixBCSRNormTest<Mem::Main, float, unsigned int> cpu_sparse_matrix_bcsr_norm_test_float_uint;
SparseMatrixBCSRNormTest<Mem::Main, double, unsigned int> cpu_sparse_matrix_bcsr_norm_test_double_uint;
#ifdef FEAT_HAVE_QUADMATH
SparseMatrixBCSRNormTest<Mem::Main, __float128, unsigned long> cpu_sparse_matrix_bcsr_norm_test_float128_ulong;
SparseMatrixBCSRNormTest<Mem::Main, __float128, unsigned int> cpu_sparse_matrix_bcsr_norm_test_float128_uint;
#endif
#ifdef FEAT_HAVE_CUDA
SparseMatrixBCSRNormTest<Mem::CUDA, float, unsigned long> cuda_sparse_matrix_bcsr_norm_test_float_ulong;
SparseMatrixBCSRNormTest<Mem::CUDA, double, unsigned long> cuda_sparse_matrix_bcsr_norm_test_double_ulong;
SparseMatrixBCSRNormTest<Mem::CUDA, float, unsigned int> cuda_sparse_matrix_bcsr_norm_test_float_uint;
SparseMatrixBCSRNormTest<Mem::CUDA, double, unsigned int> cuda_sparse_matrix_bcsr_norm_test_double_uint;
#endif


/**
 * \brief Test class for the sparse matrix csr blocked axpy method.
 *
 * \test test description missing
 *
 * \author Dirk Ribbrock
 */
template<
  typename Mem_,
  typename DT_,
  typename IT_>
class SparseMatrixBCSRAxpyTest
  : public FullTaggedTest<Mem_, DT_, IT_>
{
public:
   SparseMatrixBCSRAxpyTest()
    : FullTaggedTest<Mem_, DT_, IT_>("SparseMatrixBCSRAxpyTest")
  {
  }

  virtual ~SparseMatrixBCSRAxpyTest()
  {
  }

  virtual void run() const override
  {
    DenseVector<Mem_, DT_, IT_> dv1(12);
    for (Index i(0) ; i < dv1.size() ; ++i)
    {
      dv1(i, DT_(i+1));
    }
    DenseVector<Mem_, IT_, IT_> dv2(2);
    dv2(0, IT_(0));
    dv2(1, IT_(1));
    DenseVector<Mem_, IT_, IT_> dv3(3);
    dv3(0, IT_(0));
    dv3(1, IT_(1));
    dv3(2, IT_(2));
    SparseMatrixBCSR<Mem_, DT_, IT_, 2, 3> a(2, 2, dv2, dv1, dv3);
    DenseVector<Mem_, DT_, IT_> dv4(12);
    for (Index i(0) ; i < dv4.size() ; ++i)
    {
      dv4(i, DT_(i-1));
    }
    SparseMatrixBCSR<Mem_, DT_, IT_, 2, 3> b(2, 2, dv2, dv4, dv3);
    SparseMatrixBCSR<Mem_, DT_, IT_, 2, 3> c(a.layout());
    SparseMatrixBCSR<Mem_, DT_, IT_, 2, 3> ref(a.layout());

    DT_ scal = DT_(1.234);

    SparseMatrixBCSR<Mem::Main, DT_, IT_, 2, 3> ref_local;
    ref_local.convert(ref);
    for(Index i(0) ; i < ref_local.template used_elements<Perspective::pod>() ; ++i)
    {
      ref_local.template val<Perspective::pod>()[i] = scal * dv1(i) + dv4(i);
    }
    ref.convert(ref_local);

    c.axpy(a, b, scal);
    TEST_CHECK_EQUAL(c, ref);

    c.copy(b);
    c.axpy(a, c, scal);
    TEST_CHECK_EQUAL(c, ref);

    c.copy(a);
    c.axpy(c, b, scal);
    TEST_CHECK_EQUAL(c, ref);

    scal = DT_(0);
    ref.clone(b);
    c.axpy(a, b, scal);
    TEST_CHECK_EQUAL(c, ref);

    scal = DT_(1);
    for(Index i(0) ; i < ref_local.template used_elements<Perspective::pod>() ; ++i)
    {
      ref_local.template val<Perspective::pod>()[i] = dv1(i) + dv4(i);
    }
    ref.convert(ref_local);
    c.axpy(a, b, scal);
    TEST_CHECK_EQUAL(c, ref);

    scal = DT_(-1);
    for(Index i(0) ; i < ref_local.template used_elements<Perspective::pod>() ; ++i)
    {
      ref_local.template val<Perspective::pod>()[i] = dv4(i) - dv1(i);
    }
    ref.convert(ref_local);
    c.axpy(a, b, scal);
    TEST_CHECK_EQUAL(c, ref);
  }
};
SparseMatrixBCSRAxpyTest<Mem::Main, float, unsigned long> cpu_sparse_matrix_bcsr_axpy_test_float_ulong;
SparseMatrixBCSRAxpyTest<Mem::Main, double, unsigned long> cpu_sparse_matrix_bcsr_axpy_test_double_ulong;
SparseMatrixBCSRAxpyTest<Mem::Main, float, unsigned int> cpu_sparse_matrix_bcsr_axpy_test_float_uint;
SparseMatrixBCSRAxpyTest<Mem::Main, double, unsigned int> cpu_sparse_matrix_bcsr_axpy_test_double_uint;
#ifdef FEAT_HAVE_QUADMATH
SparseMatrixBCSRAxpyTest<Mem::Main, __float128, unsigned long> cpu_sparse_matrix_bcsr_axpy_test_float128_ulong;
SparseMatrixBCSRAxpyTest<Mem::Main, __float128, unsigned int> cpu_sparse_matrix_bcsr_axpy_test_float128_uint;
#endif
#ifdef FEAT_HAVE_CUDA
SparseMatrixBCSRAxpyTest<Mem::CUDA, float, unsigned long> cuda_sparse_matrix_bcsr_axpy_test_float_ulong;
SparseMatrixBCSRAxpyTest<Mem::CUDA, double, unsigned long> cuda_sparse_matrix_bcsr_axpy_test_double_ulong;
SparseMatrixBCSRAxpyTest<Mem::CUDA, float, unsigned int> cuda_sparse_matrix_bcsr_axpy_test_float_uint;
SparseMatrixBCSRAxpyTest<Mem::CUDA, double, unsigned int> cuda_sparse_matrix_bcsr_axpy_test_double_uint;
#endif

/**
 * \brief Test class for the sparse matrix csr blocked permute method.
 *
 * \test test description missing
 *
 * \author Dirk Ribbrock
 */
template<
  typename Mem_,
  typename DT_,
  typename IT_>
class SparseMatrixBCSRPermuteTest
  : public FullTaggedTest<Mem_, DT_, IT_>
{
public:
   SparseMatrixBCSRPermuteTest()
    : FullTaggedTest<Mem_, DT_, IT_>("SparseMatrixBCSRPermuteTest")
  {
  }

  virtual ~SparseMatrixBCSRPermuteTest()
  {
  }

  virtual void run() const override
  {
    DenseVector<Mem_, DT_, IT_> dv1(18);
    for (Index i(0) ; i < dv1.size() ; ++i)
      dv1(i, DT_(i+1));
    DenseVector<Mem_, IT_, IT_> dv2(2);
    dv2(0, IT_(0));
    dv2(1, IT_(1));
    DenseVector<Mem_, IT_, IT_> dv3(3);
    dv3(0, IT_(0));
    dv3(1, IT_(1));
    dv3(2, IT_(2));
    SparseMatrixBCSR<Mem_, DT_, IT_, 3, 3> a(2, 2, dv2, dv1, dv3);

    DenseVector<Mem_, DT_, IT_> x(a.template columns<Perspective::pod>());
    DenseVector<Mem_, DT_, IT_> r(a.template rows<Perspective::pod>());
    for (Index i(0) ; i < x.size() ; ++i)
    {
      x(i, DT_(i));
    }
    DenseVectorBlocked<Mem_, DT_, IT_, 3> xb(x);
    DenseVectorBlocked<Mem_, DT_, IT_, 3> rb(r);

    /////////////////////////

    std::cout<<xb<<std::endl;
    a.apply(rb, xb);
    DT_ ref_norm = rb.norm2();

    auto a_backup = a.clone(CloneMode::Deep);

    Random::SeedType seed(Random::SeedType(time(nullptr)));
    std::cout << "seed: " << seed << std::endl;
    Random rng(seed);
    Adjacency::Permutation perm(a.rows(), rng);

    a.permute(perm, perm);
    xb.permute(perm);

    std::cout<<xb<<std::endl;
    a.apply(rb, xb);
    DT_ norm = r.norm2();
    TEST_CHECK_EQUAL_WITHIN_EPS(norm, ref_norm, DT_(1e-4));

    a = a_backup.clone(CloneMode::Deep);
    auto perm_inv = perm.inverse();
    a.permute(perm_inv, perm);
    a.permute(perm, perm_inv);
    TEST_CHECK_EQUAL(a, a_backup);
  }
};
SparseMatrixBCSRPermuteTest<Mem::Main, float, unsigned long> cpu_sparse_matrix_bcsr_permute_test_float_ulong;
SparseMatrixBCSRPermuteTest<Mem::Main, double, unsigned long> cpu_sparse_matrix_bcsr_permute_test_double_ulong;
SparseMatrixBCSRPermuteTest<Mem::Main, float, unsigned int> cpu_sparse_matrix_bcsr_permute_test_float_uint;
SparseMatrixBCSRPermuteTest<Mem::Main, double, unsigned int> cpu_sparse_matrix_bcsr_permute_test_double_uint;
#ifdef FEAT_HAVE_QUADMATH
SparseMatrixBCSRPermuteTest<Mem::Main, __float128, unsigned long> cpu_sparse_matrix_bcsr_permute_test_float128_ulong;
SparseMatrixBCSRPermuteTest<Mem::Main, __float128, unsigned int> cpu_sparse_matrix_bcsr_permute_test_float128_uint;
#endif
#ifdef FEAT_HAVE_CUDA
SparseMatrixBCSRPermuteTest<Mem::CUDA, float, unsigned long> cpu_sparse_matrix_bcsr_permute_test_float_ulong_cuda;
SparseMatrixBCSRPermuteTest<Mem::CUDA, double, unsigned long> cpu_sparse_matrix_bcsr_permute_test_double_ulong_cuda;
SparseMatrixBCSRPermuteTest<Mem::CUDA, float, unsigned int> cpu_sparse_matrix_bcsr_permute_test_float_uint_cuda;
SparseMatrixBCSRPermuteTest<Mem::CUDA, double, unsigned int> cpu_sparse_matrix_bcsr_permute_test_double_uint_cuda;
#endif
