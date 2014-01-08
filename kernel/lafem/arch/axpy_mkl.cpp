// includes, FEAST
#include <kernel/lafem/arch/axpy.hpp>

#include <cstring>

#include <mkl.h>

using namespace FEAST;
using namespace FEAST::LAFEM;
using namespace FEAST::LAFEM::Arch;

void Axpy<Mem::Main, Algo::MKL>::dv(float * r, const float a, const float * const x, const float * const y, const Index size)
{
  if (r == y)
  {
    cblas_saxpy((MKL_INT)size, a, x, 1, r, 1);
  }
  else if (r == x)
  {
    float * t = new float[size];
    memcpy(t, x, sizeof(float) * size);
    memcpy(r, y, sizeof(float) * size);
    cblas_saxpy((MKL_INT)size, a, t, 1, r, 1);
    delete[] t;
  }
  else
  {
    memcpy(r, y, sizeof(float) * size);
    cblas_saxpy((MKL_INT)size, a, x, 1, r, 1);
  }
}

void Axpy<Mem::Main, Algo::MKL>::dv(double * r, const double a, const double * const x, const double * const y, const Index size)
{
  if (r == y)
  {
    cblas_daxpy((MKL_INT)size, a, x, 1, r, 1);
  }
  else if (r == x)
  {
    double * t = new double[size];
    memcpy(t, x, sizeof(double) * size);
    memcpy(r, y, sizeof(double) * size);
    cblas_daxpy((MKL_INT)size, a, t, 1, r, 1);
    delete[] t;
  }
  else
  {
    memcpy(r, y, sizeof(double) * size);
    cblas_daxpy((MKL_INT)size, a, x, 1, r, 1);
  }
}

void Axpy<Mem::Main, Algo::MKL>::csr(float * r, const float a, const float * const x, const float * const y, const float * const val, const Index * const col_ind, const Index * const row_ptr, const Index rows)
{
  MKL_INT mrows = (MKL_INT)rows;
  char trans = 'N';
  mkl_cspblas_scsrgemv(&trans, &mrows, (float *)val, (MKL_INT*)row_ptr, (MKL_INT*)col_ind, (float *)x, r);

  if (r == y)
  {
    cblas_saxpy((MKL_INT)mrows, a, x, 1, r, 1);
  }
  else if (r == x)
  {
    float * t = new float[rows];
    memcpy(t, x, sizeof(float) * rows);
    memcpy(r, y, sizeof(float) * rows);
    cblas_saxpy(mrows, a, t, 1, r, 1);
    delete[] t;
  }
  else
  {
    memcpy(r, y, sizeof(float) * rows);
    cblas_saxpy(mrows, a, x, 1, r, 1);
  }
}

void Axpy<Mem::Main, Algo::MKL>::csr(double * r, const double a, const double * const x, const double * const y, const double * const val, const Index * const col_ind, const Index * const row_ptr, const Index rows)
{
  MKL_INT mrows = (MKL_INT)rows;
  char trans = 'N';
  mkl_cspblas_dcsrgemv(&trans, &mrows, (double *)val, (MKL_INT*)row_ptr, (MKL_INT*)col_ind, (double *)x, r);

  if (r == y)
  {
    cblas_daxpy((MKL_INT)mrows, a, x, 1, r, 1);
  }
  else if (r == x)
  {
    double * t = new double[rows];
    memcpy(t, x, sizeof(double) * rows);
    memcpy(r, y, sizeof(double) * rows);
    cblas_daxpy(mrows, a, t, 1, r, 1);
    delete[] t;
  }
  else
  {
    memcpy(r, y, sizeof(double) * rows);
    cblas_daxpy(mrows, a, x, 1, r, 1);
  }
}

void Axpy<Mem::Main, Algo::MKL>::coo(float * r, const float a, const float * const x, const float * const y, const float * const val, const Index * const row_ptr, const Index * const col_ptr, const Index rows, const Index used_elements)
{
  MKL_INT mrows = (MKL_INT)rows;
  char trans = 'N';
  MKL_INT ue = (MKL_INT)used_elements;
  mkl_cspblas_scoogemv(&trans, &mrows, (float *)val, (MKL_INT*)row_ptr, (MKL_INT*)col_ptr, &ue, (float *)x, r);

  if (r == y)
  {
    cblas_saxpy((MKL_INT)mrows, a, x, 1, r, 1);
  }
  else if (r == x)
  {
    float * t = new float[rows];
    memcpy(t, x, sizeof(float) * rows);
    memcpy(r, y, sizeof(float) * rows);
    cblas_saxpy(mrows, a, t, 1, r, 1);
    delete[] t;
  }
  else
  {
    memcpy(r, y, sizeof(float) * rows);
    cblas_saxpy(mrows, a, x, 1, r, 1);
  }
}

void Axpy<Mem::Main, Algo::MKL>::coo(double * r, const double a, const double * const x, const double * const y, const double * const val, const Index * const row_ptr, const Index * const col_ptr, const Index rows, const Index used_elements)
{
  MKL_INT mrows = (MKL_INT)rows;
  char trans = 'N';
  MKL_INT ue = (MKL_INT)used_elements;
  mkl_cspblas_dcoogemv(&trans, &mrows, (double *)val, (MKL_INT*)row_ptr, (MKL_INT*)col_ptr, &ue, (double *)x, r);

  if (r == y)
  {
    cblas_daxpy((MKL_INT)mrows, a, x, 1, r, 1);
  }
  else if (r == x)
  {
    double * t = new double[rows];
    memcpy(t, x, sizeof(double) * rows);
    memcpy(r, y, sizeof(double) * rows);
    cblas_daxpy(mrows, a, t, 1, r, 1);
    delete[] t;
  }
  else
  {
    memcpy(r, y, sizeof(double) * rows);
    cblas_daxpy(mrows, a, x, 1, r, 1);
  }
}
