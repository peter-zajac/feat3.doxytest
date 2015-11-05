#include <kernel/base_header.hpp>
#include <kernel/archs.hpp>
#include <test_system/test_system.hpp>
#include <kernel/lafem/sparse_matrix_bcsr.hpp>
#include <kernel/util/binary_stream.hpp>

using namespace FEAST;
using namespace FEAST::LAFEM;
using namespace FEAST::TestSystem;

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

  virtual void run() const
  {
    test_vector_types();

    SparseMatrixBCSR<Mem_, DT_, IT_, 2, 3> a;
    SparseMatrixBCSR<Mem_, DT_, IT_, 2, 3> b;
    TEST_CHECK_EQUAL(b, a);

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

    TEST_CHECK_EQUAL(c(1,0)(0,0), DT_(0));
    TEST_CHECK_EQUAL(c(1,1)(1,1), DT_(11));

    SparseMatrixBCSR<Mem_, DT_, IT_, 2, 3> d;
    d.convert(c);
    TEST_CHECK_EQUAL(d.rows(), c.rows());
    TEST_CHECK_EQUAL(d.columns(), c.columns());
    TEST_CHECK_EQUAL(d.used_elements(), c.used_elements());
    TEST_CHECK_EQUAL(d, c);
    TEST_CHECK_EQUAL((void*)d.raw_val(), (void*)c.raw_val());
    TEST_CHECK_EQUAL((void*)d.row_ptr(), (void*)c.row_ptr());
    SparseMatrixBCSR<Mem_, DT_, IT_, 2, 3> e;
    e.clone(c);
    TEST_CHECK_EQUAL(e, c);
    TEST_CHECK_NOT_EQUAL((void*)e.raw_val(), (void*)c.raw_val());
    TEST_CHECK_EQUAL((void*)e.row_ptr(), (void*)c.row_ptr());
    e = c.clone(CloneMode::Deep);
    TEST_CHECK_EQUAL(e, c);
    TEST_CHECK_NOT_EQUAL((void*)e.raw_val(), (void*)c.raw_val());
    TEST_CHECK_NOT_EQUAL((void*)e.row_ptr(), (void*)c.row_ptr());
    e = c.shared();
    TEST_CHECK_EQUAL(e, c);
    TEST_CHECK_EQUAL((void*)e.raw_val(), (void*)c.raw_val());
    TEST_CHECK_EQUAL((void*)e.row_ptr(), (void*)c.row_ptr());

    SparseMatrixBCSR<Mem_, DT_, IT_, 2, 3> f(c.layout());
    TEST_CHECK_EQUAL(f.rows(), c.rows());
    TEST_CHECK_EQUAL(f.columns(), c.columns());
    TEST_CHECK_EQUAL(f.used_elements(), c.used_elements());
    TEST_CHECK_NOT_EQUAL((void*)f.raw_val(), (void*)c.raw_val());
    TEST_CHECK_EQUAL((void*)f.row_ptr(), (void*)c.row_ptr());

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

    auto kp = c.serialise();
    SparseMatrixBCSR<Mem_, DT_, IT_, 2, 3> k(kp);
    TEST_CHECK_EQUAL(k, c);
  }
};
SparseMatrixBCSRTest<Mem::Main, float, unsigned long> cpu_sparse_matrix_bcsr_test_float_ulong;
SparseMatrixBCSRTest<Mem::Main, double, unsigned long> cpu_sparse_matrix_bcsr_test_double_ulong;
SparseMatrixBCSRTest<Mem::Main, float, unsigned int> cpu_sparse_matrix_bcsr_test_float_uint;
SparseMatrixBCSRTest<Mem::Main, double, unsigned int> cpu_sparse_matrix_bcsr_test_double_uint;
#ifdef FEAST_BACKENDS_CUDA
SparseMatrixBCSRTest<Mem::CUDA, float, unsigned long> cuda_sparse_matrix_bcsr_test_float_ulong;
SparseMatrixBCSRTest<Mem::CUDA, double, unsigned long> cuda_sparse_matrix_bcsr_test_double_ulong;
SparseMatrixBCSRTest<Mem::CUDA, float, unsigned int> cuda_sparse_matrix_bcsr_test_float_uint;
SparseMatrixBCSRTest<Mem::CUDA, double, unsigned int> cuda_sparse_matrix_bcsr_test_double_uint;
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

  virtual void run() const
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

    DenseVector<Mem_, DT_, IT_> x(c.raw_columns());
    DenseVector<Mem_, DT_, IT_> y(c.raw_rows());
    DenseVector<Mem_, DT_, IT_> r(c.raw_rows());
    DenseVector<Mem_, DT_, IT_> ref(c.raw_rows());
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

    DT_ alpha(-1);
    csr.apply(ref, x, y, alpha);
    c.apply(r, x, y, alpha);
    TEST_CHECK_EQUAL(r, ref);

    c.apply(rb, x, yb, alpha);
    r.convert(rb);
    TEST_CHECK_EQUAL(r, ref);

    c.apply(r, xb, y, alpha);
    TEST_CHECK_EQUAL(r, ref);

    c.apply(rb, xb, yb, alpha);
    r.convert(rb);
    TEST_CHECK_EQUAL(r, ref);

    alpha = DT_(1.234);
    csr.apply(ref, x, y, alpha);
    c.apply(r, x, y, alpha);
    TEST_CHECK_EQUAL(r, ref);

    c.apply(rb, x, yb, alpha);
    r.convert(rb);
    TEST_CHECK_EQUAL(r, ref);

    c.apply(r, xb, y, alpha);
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

  virtual void run() const
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
    TEST_CHECK_EQUAL(diag.raw_elements()[0], 1);
    TEST_CHECK_EQUAL(diag.raw_elements()[1], 5);
    TEST_CHECK_EQUAL(diag.raw_elements()[2], 9);
    TEST_CHECK_EQUAL(diag.raw_elements()[3], 10);
    TEST_CHECK_EQUAL(diag.raw_elements()[4], 14);
    TEST_CHECK_EQUAL(diag.raw_elements()[5], 18);
  }
};
SparseMatrixBCSRDiagTest<Mem::Main, float, unsigned long> cpu_sparse_matrix_bcsr_diag_test_float_ulong;
SparseMatrixBCSRDiagTest<Mem::Main, double, unsigned long> cpu_sparse_matrix_bcsr_diag_test_double_ulong;
SparseMatrixBCSRDiagTest<Mem::Main, float, unsigned int> cpu_sparse_matrix_bcsr_diag_test_float_uint;
SparseMatrixBCSRDiagTest<Mem::Main, double, unsigned int> cpu_sparse_matrix_bcsr_diag_test_double_uint;
