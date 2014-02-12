// includes, FEAST
#include <kernel/base_header.hpp>
#include <kernel/archs.hpp>
#include <kernel/lafem/arch/axpy.hpp>

#include <cstring>

using namespace FEAST;
using namespace FEAST::LAFEM;
using namespace FEAST::LAFEM::Arch;

template <typename DT_>
void Axpy<Mem::Main, Algo::Generic>::dv(DT_ * r, const DT_ a, const DT_ * const x, const DT_ * const y, const Index size)
{
  if (r == y)
  {
    for (Index i(0) ; i < size ; ++i)
    {
      r[i] += a * x[i];
    }
  }
  else if (r == x)
  {
    for (Index i(0) ; i < size ; ++i)
    {
      r[i] *= a;
      r[i]+= y[i];
    }
  }
  else
  {
    for (Index i(0) ; i < size ; ++i)
    {
      r[i] = a * x[i] + y[i];
    }
  }
}

template void Axpy<Mem::Main, Algo::Generic>::dv(float *, const float, const float * const, const float * const, const Index);
template void Axpy<Mem::Main, Algo::Generic>::dv(double *, const double, const double * const, const double * const, const Index);

template <typename DT_>
void Axpy<Mem::Main, Algo::Generic>::dv(DT_ * r, const DT_  * const a, const DT_ * const x, const DT_ * const y, const Index size)
{
  if (r == y)
  {
    for (Index i(0) ; i < size ; ++i)
    {
      r[i] += a[i] * x[i];
    }
  }
  else if(r == x)
  {
    for (Index i(0) ; i < size ; ++i)
    {
      r[i] *= a[i];
      r[i]+= y[i];
    }
  }
  else if(r == a)
  {
    for (Index i(0) ; i < size ; ++i)
    {
      r[i] *= x[i];
      r[i] += y[i];
    }
  }
  else
  {
    for (Index i(0) ; i < size ; ++i)
    {
      r[i] = a[i] * x[i] + y[i];
    }
  }
}

template void Axpy<Mem::Main, Algo::Generic>::dv(float *, const float * const, const float * const, const float * const, const Index);
template void Axpy<Mem::Main, Algo::Generic>::dv(double *, const double * const, const double * const, const double * const, const Index);

template <typename DT_>
void Axpy<Mem::Main, Algo::Generic>::csr(DT_ * r, const DT_ a, const DT_ * const x, const DT_ * const y, const DT_ * const val,
    const Index * const col_ind, const Index * const row_ptr, const Index rows)
{
  for (Index row(0) ; row < rows ; ++row)
  {
    DT_ sum(0);
    const Index end(row_ptr[row + 1]);
    for (Index i(row_ptr[row]) ; i < end ; ++i)
    {
      sum += val[i] * x[col_ind[i]];
    }
    r[row] = (sum * a)+ y[row];
  }
}
template void Axpy<Mem::Main, Algo::Generic>::csr(float *, const float, const float * const, const float * const, const float * const, const Index * const, const Index * const, const Index);
template void Axpy<Mem::Main, Algo::Generic>::csr(double *, const double, const double * const, const double * const, const double * const, const Index * const, const Index * const, const Index);

template <typename DT_>
void Axpy<Mem::Main, Algo::Generic>::csr(DT_ * r, const DT_ * const a, const DT_ * const x, const DT_ * const y, const DT_ * const val,
    const Index * const col_ind, const Index * const row_ptr, const Index rows)
{
  for (Index row(0) ; row < rows ; ++row)
  {
    DT_ sum(0);
    const Index end(row_ptr[row + 1]);
    for (Index i(row_ptr[row]) ; i < end ; ++i)
    {
      sum += val[i] * x[col_ind[i]];
    }
    r[row] = (sum * a[row])+ y[row];
  }
}
template void Axpy<Mem::Main, Algo::Generic>::csr(float *, const float * const, const float * const, const float * const, const float * const, const Index * const, const Index * const, const Index);
template void Axpy<Mem::Main, Algo::Generic>::csr(double *, const double * const, const double * const, const double * const, const double * const, const Index * const, const Index * const, const Index);

template <typename DT_>
void Axpy<Mem::Main, Algo::Generic>::ell(DT_ * r, const DT_ a, const DT_ * const x, const DT_ * const y, const DT_ * const Ax, const Index * const Aj, const Index * const Arl, const Index stride, const Index rows)
{
  for (Index row(0) ; row < rows ; ++row)
  {
    const Index * tAj(Aj);
    const DT_ * tAx(Ax);
    DT_ sum(0);
    tAj += row;
    tAx += row;

    const Index max(Arl[row]);
    for(Index n(0); n < max ; n++)
    {
      const DT_ A_ij = *tAx;

      const Index col = *tAj;
      sum += A_ij * x[col];

      tAj += stride;
      tAx += stride;
    }
    r[row] = (sum * a) + y[row];
  }
}
template void Axpy<Mem::Main, Algo::Generic>::ell(float *, const float, const float * const, const float * const, const float * const, const Index * const, const Index * const, const Index, const Index);
template void Axpy<Mem::Main, Algo::Generic>::ell(double *, const double, const double * const, const double * const, const double * const, const Index * const, const Index * const, const Index, const Index);

template <typename DT_>
void Axpy<Mem::Main, Algo::Generic>::ell(DT_ * r, const DT_ * const a, const DT_ * const x, const DT_ * const y, const DT_ * const Ax, const Index * const Aj, const Index * const Arl, const Index stride, const Index rows)
{
  for (Index row(0) ; row < rows ; ++row)
  {
    const Index * tAj(Aj);
    const DT_ * tAx(Ax);
    DT_ sum(0);
    tAj += row;
    tAx += row;

    const Index max(Arl[row]);
    for(Index n(0); n < max ; n++)
    {
      const DT_ A_ij = *tAx;

      const Index col = *tAj;
      sum += A_ij * x[col];

      tAj += stride;
      tAx += stride;
    }
    r[row] = (sum * a[row]) + y[row];
  }
}
template void Axpy<Mem::Main, Algo::Generic>::ell(float *, const float * const, const float * const, const float * const, const float * const, const Index * const, const Index * const, const Index, const Index);
template void Axpy<Mem::Main, Algo::Generic>::ell(double *, const double * const, const double * const, const double * const, const double * const, const Index * const, const Index * const, const Index, const Index);

template <typename DT_>
void Axpy<Mem::Main, Algo::Generic>::coo(DT_ * r, const DT_ a, const DT_ * const x, const DT_ * const y, const DT_ * const val,
    const Index * const row_ptr, const Index * const col_ptr, const Index rows, const Index used_elements)
{
  Index iter(0);
  for (Index row(0); row < rows; ++row)
  {
    DT_ sum(DT_(0));
    while (row_ptr[iter] == row && iter < used_elements)
    {
      sum += val[iter] * x[col_ptr[iter]];
      ++iter;
    }
    r[row] = a * sum + y[row];
  }
}
template void Axpy<Mem::Main, Algo::Generic>::coo(float *, const float, const float * const, const float * const, const float * const, const Index * const, const Index * const, const Index, const Index);
template void Axpy<Mem::Main, Algo::Generic>::coo(double *, const double, const double * const, const double * const, const double * const, const Index * const, const Index * const, const Index, const Index);

template <typename DT_>
void Axpy<Mem::Main, Algo::Generic>::coo(DT_ * r, const DT_ * const a, const DT_ * const x, const DT_ * const y, const DT_ * const val,
    const Index * const row_ptr, const Index * const col_ptr, const Index rows, const Index used_elements)
{
  Index iter(0);
  for (Index row(0); row < rows; ++row)
  {
    DT_ sum(DT_(0));
    while (row_ptr[iter] == row && iter < used_elements)
    {
      sum += val[iter] * x[col_ptr[iter]];
      ++iter;
    }
    r[row] = a[row] * sum + y[row];
  }
}
template void Axpy<Mem::Main, Algo::Generic>::coo(float *, const float * const, const float * const, const float * const, const float * const, const Index * const, const Index * const, const Index, const Index);
template void Axpy<Mem::Main, Algo::Generic>::coo(double *, const double * const, const double * const, const double * const, const double * const, const Index * const, const Index * const, const Index, const Index);
