// FEAT3: Finite Element Analysis Toolbox, Version 3
// Copyright (C) 2010 - 2021 by Stefan Turek & the FEAT group
// FEAT3 is released under the GNU General Public License version 3,
// see the file 'copyright.txt' in the top level directory for details.

// includes, FEAT
#include <kernel/base_header.hpp>
#include <kernel/archs.hpp>
#include <kernel/lafem/arch/scale_row_col.hpp>
#include <kernel/util/exception.hpp>
#include <kernel/util/memory_pool.hpp>

namespace FEAT
{
  namespace LAFEM
  {
    namespace Intern
    {
      template <typename DT_, typename IT_>
      __global__ void cuda_scale_rows_csr(DT_ * r, const DT_ * b, const DT_ * val, const IT_ * col_ind,
                                          const IT_ * row_ptr, const Index count)
      {
        Index idx = threadIdx.x + blockDim.x * blockIdx.x;
        if (idx >= count)
          return;

        const Index end(row_ptr[idx + 1]);
        for (Index i(row_ptr[idx]) ; i < end ; ++i)
        {
          r[i] = val[i] * b[idx];
        }
      }

      template <typename DT_, typename IT_>
      __global__ void cuda_scale_cols_csr(DT_ * r, const DT_ * b, const DT_ * val, const IT_ * col_ind,
                                          const IT_ * row_ptr, const Index count)
      {
        Index idx = threadIdx.x + blockDim.x * blockIdx.x;
        if (idx >= count)
          return;

        DT_ sum(0);
        const Index end(row_ptr[idx + 1]);
        for (Index i(row_ptr[idx]) ; i < end ; ++i)
        {
          sum += val[i] * b[col_ind[i]];
        }
      }


      template <typename DT_, typename IT_>
      __global__ void cuda_scale_rows_ell(DT_ * r, const DT_ * const a, const IT_ * const col_ind, const IT_ * const cs,
                                          const IT_ * const cl, const IT_ * const /*rl*/, const DT_ * const x, const Index C, const Index rows)
      {
        Index idx = threadIdx.x + blockDim.x * blockIdx.x;
        if (idx >= rows)
          return;

        const Index chunk(idx / C);
        const Index local_row(idx % C);
        const Index chunk_end(cs[chunk+1]);

        for (Index pcol(cs[chunk] + local_row) ; pcol < chunk_end ; pcol+=C)
        {
          r[pcol] = a[pcol] * x[idx];
        }
      }

      template <typename DT_, typename IT_>
      __global__ void cuda_scale_cols_ell(DT_ * r, const DT_ * const a, const IT_ * const col_ind, const IT_ * const cs,
                                          const IT_ * const cl, const IT_ * const /*rl*/, const DT_ * const x, const Index C, const Index rows)
      {
        Index idx = threadIdx.x + blockDim.x * blockIdx.x;
        if (idx >= rows)
          return;

        const Index chunk(idx / C);
        const Index local_row(idx % C);
        const Index chunk_end(cs[chunk+1]);

        for (Index pcol(cs[chunk] + local_row) ; pcol < chunk_end ; pcol+=C)
        {
          r[pcol] = a[pcol] * x[col_ind[pcol]];
        }
      }
    }
  }
}


using namespace FEAT;
using namespace FEAT::LAFEM;
using namespace FEAT::LAFEM::Arch;

template <typename DT_, typename IT_>
void ScaleRows<Mem::CUDA>::csr(DT_ * r, const DT_ * const val, const IT_ * const col_ind, const IT_ * const row_ptr, const DT_ * const x, const Index rows, const Index columns, const Index used_elements)
{
  Index blocksize = MemoryPool<Mem::CUDA>::blocksize_axpy;
  dim3 grid;
  dim3 block;
  block.x = blocksize;
  grid.x = (unsigned)ceil((rows)/(double)(block.x));

  FEAT::LAFEM::Intern::cuda_scale_rows_csr<<<grid, block>>>(r, x, val, col_ind, row_ptr, rows);
#ifdef FEAT_DEBUG_MODE
  cudaDeviceSynchronize();
  cudaError_t last_error(cudaGetLastError());
  if (cudaSuccess != last_error)
    throw InternalError(__func__, __FILE__, __LINE__, "CUDA error occurred in execution!\n" + stringify(cudaGetErrorString(last_error)));
#endif
}
template void ScaleRows<Mem::CUDA>::csr(float *, const float * const, const unsigned long * const, const unsigned long * const, const float * const, const Index, const Index, const Index);
template void ScaleRows<Mem::CUDA>::csr(double *, const double * const, const unsigned long * const, const unsigned long * const, const double * const, const Index, const Index, const Index);
template void ScaleRows<Mem::CUDA>::csr(float *, const float * const, const unsigned int * const, const unsigned int * const, const float * const, const Index, const Index, const Index);
template void ScaleRows<Mem::CUDA>::csr(double *, const double * const, const unsigned int * const, const unsigned int * const, const double * const, const Index, const Index, const Index);

template <typename DT_, typename IT_>
void ScaleCols<Mem::CUDA>::csr(DT_ * r, const DT_ * const val, const IT_ * const col_ind, const IT_ * const row_ptr, const DT_ * const x, const Index rows, const Index columns, const Index used_elements)
{
  Index blocksize = MemoryPool<Mem::CUDA>::blocksize_axpy;
  dim3 grid;
  dim3 block;
  block.x = blocksize;
  grid.x = (unsigned)ceil((rows)/(double)(block.x));

  FEAT::LAFEM::Intern::cuda_scale_cols_csr<<<grid, block>>>(r, x, val, col_ind, row_ptr, rows);
#ifdef FEAT_DEBUG_MODE
  cudaDeviceSynchronize();
  cudaError_t last_error(cudaGetLastError());
  if (cudaSuccess != last_error)
    throw InternalError(__func__, __FILE__, __LINE__, "CUDA error occurred in execution!\n" + stringify(cudaGetErrorString(last_error)));
#endif
}
template void ScaleCols<Mem::CUDA>::csr(float *, const float * const, const unsigned long * const, const unsigned long * const, const float * const, const Index, const Index, const Index);
template void ScaleCols<Mem::CUDA>::csr(double *, const double * const, const unsigned long * const, const unsigned long * const, const double * const, const Index, const Index, const Index);
template void ScaleCols<Mem::CUDA>::csr(float *, const float * const, const unsigned int * const, const unsigned int * const, const float * const, const Index, const Index, const Index);
template void ScaleCols<Mem::CUDA>::csr(double *, const double * const, const unsigned int * const, const unsigned int * const, const double * const, const Index, const Index, const Index);


template <typename DT_, typename IT_>
void ScaleRows<Mem::CUDA>::ell(DT_ * r, const DT_ * const a, const IT_ * const col_ind, const IT_ * const cs,
                                           const IT_ * const cl, const IT_ * const rl, const DT_ * const x, const Index C, const Index rows)
{
  Index blocksize = MemoryPool<Mem::CUDA>::blocksize_axpy;
  dim3 grid;
  dim3 block;
  block.x = blocksize;
  grid.x = (unsigned)ceil((rows)/(double)(block.x));

  FEAT::LAFEM::Intern::cuda_scale_rows_ell<<<grid, block>>>(r, a, col_ind, cs, cl, rl, x, C, rows);
#ifdef FEAT_DEBUG_MODE
  cudaDeviceSynchronize();
  cudaError_t last_error(cudaGetLastError());
  if (cudaSuccess != last_error)
    throw InternalError(__func__, __FILE__, __LINE__, "CUDA error occurred in execution!\n" + stringify(cudaGetErrorString(last_error)));
#endif
}
template void ScaleRows<Mem::CUDA>::ell(float *, const float * const, const unsigned long * const, const unsigned long * const, const unsigned long * const, const unsigned long * const, const float * const, const Index, const Index);
template void ScaleRows<Mem::CUDA>::ell(double *, const double * const, const unsigned long * const, const unsigned long * const, const unsigned long * const, const unsigned long * const, const double * const, const Index, const Index);
template void ScaleRows<Mem::CUDA>::ell(float *, const float * const, const unsigned int * const, const unsigned int * const, const unsigned int * const, const unsigned int * const, const float * const, const Index, const Index);
template void ScaleRows<Mem::CUDA>::ell(double *, const double * const, const unsigned int * const, const unsigned int * const, const unsigned int * const, const unsigned int * const, const double * const, const Index, const Index);

template <typename DT_, typename IT_>
void ScaleCols<Mem::CUDA>::ell(DT_ * r, const DT_ * const a, const IT_ * const col_ind, const IT_ * const cs,
                                           const IT_ * const cl, const IT_ * const rl, const DT_ * const x, const Index C, const Index rows)
{
  Index blocksize = MemoryPool<Mem::CUDA>::blocksize_axpy;
  dim3 grid;
  dim3 block;
  block.x = blocksize;
  grid.x = (unsigned)ceil((rows)/(double)(block.x));

  FEAT::LAFEM::Intern::cuda_scale_cols_ell<<<grid, block>>>(r, a, col_ind, cs, cl, rl, x, C, rows);
#ifdef FEAT_DEBUG_MODE
  cudaDeviceSynchronize();
  cudaError_t last_error(cudaGetLastError());
  if (cudaSuccess != last_error)
    throw InternalError(__func__, __FILE__, __LINE__, "CUDA error occurred in execution!\n" + stringify(cudaGetErrorString(last_error)));
#endif
}
template void ScaleCols<Mem::CUDA>::ell(float *, const float * const, const unsigned long * const, const unsigned long * const, const unsigned long * const, const unsigned long * const, const float * const, const Index, const Index);
template void ScaleCols<Mem::CUDA>::ell(double *, const double * const, const unsigned long * const, const unsigned long * const, const unsigned long * const, const unsigned long * const, const double * const, const Index, const Index);
template void ScaleCols<Mem::CUDA>::ell(float *, const float * const, const unsigned int * const, const unsigned int * const, const unsigned int * const, const unsigned int * const, const float * const, const Index, const Index);
template void ScaleCols<Mem::CUDA>::ell(double *, const double * const, const unsigned int * const, const unsigned int * const, const unsigned int * const, const unsigned int * const, const double * const, const Index, const Index);
