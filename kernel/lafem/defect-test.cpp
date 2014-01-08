#include <kernel/base_header.hpp>
#include <kernel/archs.hpp>
#include <kernel/util/math.hpp>
#include <test_system/test_system.hpp>
#include <kernel/lafem/dense_vector.hpp>
#include <kernel/lafem/sparse_matrix_csr.hpp>
#include <kernel/lafem/sparse_matrix_coo.hpp>

using namespace FEAST;
using namespace FEAST::LAFEM;
using namespace FEAST::TestSystem;

template<
  typename Arch_,
  typename Algo_,
  typename DT_,
  typename SM_>
class DefectTest
  : public TaggedTest<Arch_, DT_, Algo_>
{

public:

  DefectTest()
    : TaggedTest<Arch_, DT_, Algo_>("defect_test: " + SM_::type_name())
  {
  }

  virtual void run() const
  {
    const DT_ eps = Math::pow(Math::Limits<DT_>::epsilon(), DT_(0.8));
    const DT_ pi = Math::Limits<DT_>::pi();

    for (Index size(2) ; size < 3e2 ; size*=2)
    {
      SparseMatrixCOO<Mem::Main, DT_> a_local(size, size + 2);
      DenseVector<Mem::Main, DT_> b_local(size + 2);
      DenseVector<Mem::Main, DT_> rhs_local(size);
      DenseVector<Mem::Main, DT_> ref(size);
      DenseVector<Mem::Main, DT_> result_local(size);
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
      for (Index i(0) ; i < size ; ++i)
      {
        b_local(i, Math::sin(pi * DT_(i) / DT_(size-1)));
        rhs_local(i, b_local(i) - DT_(5));
      }

      SM_ a(a_local);
      DenseVector<Arch_, DT_> b(size + 2);
      copy(b, b_local);
      DenseVector<Arch_, DT_> rhs(size);
      copy(rhs, rhs_local);
      DenseVector<Arch_, DT_> c(size, 4711);

      c.template defect<Algo_>(rhs, a, b);
      copy(result_local, c);

      DT_ dev(DT_(0));
      for (Index i(0) ; i < size ; ++i)
      {
        DT_ sum(result_local(i) - rhs_local(i));
        for (Index j(0) ; j < a_local.columns() ; ++j)
          sum += a_local(i, j) * b_local(j);
        dev = Math::max(dev, Math::abs(sum));
      }

      TEST_CHECK(dev <= eps);
    }
  }
};
DefectTest<Mem::Main, Algo::Generic, float, SparseMatrixCSR<Mem::Main, float> > csr_defect_test_float;
DefectTest<Mem::Main, Algo::Generic, double, SparseMatrixCSR<Mem::Main, double> > csr_defect_test_double;
#ifdef FEAST_GMP
//DefectTest<Mem::Main, Algo::Generic, mpf_class, SparseMatrixCSR<Mem::Main, mpf_class> > csr_defect_test_mpf_class;
#endif
#ifdef FEAST_BACKENDS_MKL
DefectTest<Mem::Main, Algo::MKL, float, SparseMatrixCSR<Mem::Main, float> > mkl_csr_defect_test_float;
DefectTest<Mem::Main, Algo::MKL, double, SparseMatrixCSR<Mem::Main, double> > mkl_csr_defect_test_double;
#endif
#ifdef FEAST_BACKENDS_CUDA
DefectTest<Mem::CUDA, Algo::CUDA, float, SparseMatrixCSR<Mem::CUDA, float> > cuda_csr_defect_test_float;
DefectTest<Mem::CUDA, Algo::CUDA, double, SparseMatrixCSR<Mem::CUDA, double> > cuda_csr_defect_test_double;
#endif
DefectTest<Mem::Main, Algo::Generic, float, SparseMatrixELL<Mem::Main, float> > ell_defect_test_float;
DefectTest<Mem::Main, Algo::Generic, double, SparseMatrixELL<Mem::Main, double> > ell_defect_test_double;
#ifdef FEAST_GMP
//DefectTest<Mem::Main, Algo::Generic, mpf_class, SparseMatrixELL<Mem::Main, mpf_class> > ell_defect_test_mpf_class;
#endif
#ifdef FEAST_BACKENDS_CUDA
DefectTest<Mem::CUDA, Algo::CUDA, float, SparseMatrixELL<Mem::CUDA, float> > cuda_ell_defect_test_float;
DefectTest<Mem::CUDA, Algo::CUDA, double, SparseMatrixELL<Mem::CUDA, double> > cuda_ell_defect_test_double;
#endif
DefectTest<Mem::Main, Algo::Generic, float, SparseMatrixCOO<Mem::Main, float> > coo_defect_test_float;
DefectTest<Mem::Main, Algo::Generic, double, SparseMatrixCOO<Mem::Main, double> > coo_defect_test_double;
#ifdef FEAST_GMP
//DefectTest<Mem::Main, Algo::Generic, mpf_class, SparseMatrixCOO<Mem::Main, mpf_class> > coo_defect_test_mpf_class;
#endif
#ifdef FEAST_BACKENDS_MKL
DefectTest<Mem::Main, Algo::MKL, float, SparseMatrixCOO<Mem::Main, float> > mkl_coo_defect_test_float;
DefectTest<Mem::Main, Algo::MKL, double, SparseMatrixCOO<Mem::Main, double> > mkl_coo_defect_test_double;
#endif
