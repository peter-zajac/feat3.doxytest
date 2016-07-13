// includes, FEAT
#include <kernel/base_header.hpp>
#include <kernel/archs.hpp>
#include <kernel/lafem/arch/defect.hpp>
#include <kernel/lafem/arch/difference.hpp>
#include <kernel/lafem/arch/product_matvec.hpp>
#include <kernel/util/exception.hpp>
#include <kernel/util/memory_pool.hpp>

#include "cusparse_v2.h"

namespace FEAT
{
  namespace LAFEM
  {
    namespace Intern
    {
      template <typename DT_>
      __global__ void cuda_defect_csr(DT_ * r, const DT_ * rhs, const DT_ * b, const DT_ * val, const unsigned long * col_ind,
                                      const unsigned long * row_ptr, const Index count)
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
        r[idx] = rhs[idx] - sum;
      }

      template <typename DT_, typename IT_>
      __global__ void cuda_defect_ell(DT_ * r, const DT_ * rhs, const DT_ * x, const DT_ * val, const IT_ * col_ind,
                                      const IT_ * cs, const IT_ * cl, const Index rows, const Index C)
      {
        const Index idx = threadIdx.x + blockDim.x * blockIdx.x;
        if (idx >= rows)
          return;


        DT_ sum(0);
        const Index chunk(idx / C);
        const Index local_row(idx % C);
        const Index chunk_end(cs[chunk+1]);

        for (Index pcol(cs[chunk] + local_row) ; pcol < chunk_end ; pcol+=C)
        {
          sum += val[pcol] * x[col_ind[pcol]];
        }
        r[idx] = rhs[idx] - sum;

      }

      template <typename DT_, typename IT_>
      __global__ void cuda_defect_banded(DT_ * r, const DT_ * rhs, const DT_ * x, const DT_ * val, const IT_ * offsets, const Index num_of_offsets, const Index rows, const Index columns)
      {
        Index idx = threadIdx.x + blockDim.x * blockIdx.x;
        if (idx >= rows)
          return;

        const Index k1(rows - 1);
        const Index k2(rows + columns - 1);

        Index start(0);

        while (k1 > offsets[start] + idx)
        {
          ++start;
        }

        Index end(start);

        while (end < num_of_offsets && idx + offsets[end] < k2)
        {
          ++end;
        }

        DT_ sum(DT_(0.0));
        for (Index diag(start); diag < end; ++diag)
        {
          sum += val[rows * diag + idx] * x[idx + offsets[diag] - rows + 1];
        }
        r[idx] = rhs[idx] - sum;
      }

      void cusparse_defect_csr(cusparseOperation_t trans,
                                       int m, int n, int nnz,
                                       const float * alpha, const cusparseMatDescr_t descrA,
                                       const float * csrVal, const int * csrRowPtr, const int *csrColInd,
                                       const float * x, const float * beta, float * y)
      {
        cusparseStatus_t status;
        status = cusparseScsrmv(Util::Intern::cusparse_handle, trans, m, n, nnz, alpha, descrA, csrVal, csrRowPtr,
                       csrColInd, x, beta, y);
        if (status != CUSPARSE_STATUS_SUCCESS)
          throw InternalError(__func__, __FILE__, __LINE__, "cusparsecsrmv failed with status code: " + stringify(status));
      }

      void cusparse_defect_csr(cusparseOperation_t trans,
                                       int m, int n, int nnz,
                                       const double * alpha, const cusparseMatDescr_t descrA,
                                       const double * csrVal, const int * csrRowPtr, const int *csrColInd,
                                       const double * x, const double * beta, double * y)
      {
        cusparseStatus_t status;
        status = cusparseDcsrmv(Util::Intern::cusparse_handle, trans, m, n, nnz, alpha, descrA, csrVal, csrRowPtr,
                       csrColInd, x, beta, y);
        if (status != CUSPARSE_STATUS_SUCCESS)
          throw InternalError(__func__, __FILE__, __LINE__, "cusparsecsrmv failed with status code: " + stringify(status));
      }

      void cusparse_defect_csrb(cusparseDirection_t dir, cusparseOperation_t trans,
                                       int m, int n, int nnz,
                                       const float * alpha, const cusparseMatDescr_t descrA,
                                       const float * csrVal, const int * csrRowPtr, const int *csrColInd,
                                       int block_dim,
                                       const float * x, const float * beta, float * y)
      {
        cusparseStatus_t status;
        status = cusparseSbsrmv(Util::Intern::cusparse_handle, dir, trans, m, n, nnz, alpha, descrA, csrVal, csrRowPtr,
                       csrColInd, block_dim, x, beta, y);
        if (status != CUSPARSE_STATUS_SUCCESS)
          throw InternalError(__func__, __FILE__, __LINE__, "cusparsebsrmv failed with status code: " + stringify(status));
      }

      void cusparse_defect_csrb(cusparseDirection_t dir, cusparseOperation_t trans,
                                       int m, int n, int nnz,
                                       const double * alpha, const cusparseMatDescr_t descrA,
                                       const double * csrVal, const int * csrRowPtr, const int *csrColInd,
                                       int block_dim,
                                       const double * x, const double * beta, double * y)
      {
        cusparseStatus_t status;
        status = cusparseDbsrmv(Util::Intern::cusparse_handle, dir, trans, m, n, nnz, alpha, descrA, csrVal, csrRowPtr,
                       csrColInd, block_dim, x, beta, y);
        if (status != CUSPARSE_STATUS_SUCCESS)
          throw InternalError(__func__, __FILE__, __LINE__, "cusparsebsrmv failed with status code: " + stringify(status));
      }

      void cublas_defect_dense(cublasOperation_t trans,
                                       int m, int n,
                                       const float * alpha,
                                       const float * val,
                                       const float * x, const float * beta, float * y)
      {
        cublasStatus_t status;
        status = cublasSgemv(Util::Intern::cublas_handle, trans, n, m, alpha, val, n, x, 1, beta, y, 1);
        if (status != CUBLAS_STATUS_SUCCESS)
          throw InternalError(__func__, __FILE__, __LINE__, "cublasSgemv failed with status code: " + stringify(status));
      }

      void cublas_defect_dense(cublasOperation_t trans,
                                       int m, int n,
                                       const double * alpha,
                                       const double * val,
                                       const double * x, const double * beta, double * y)
      {
        cublasStatus_t status;
        status = cublasDgemv(Util::Intern::cublas_handle, trans, n, m, alpha, val, n, x, 1, beta, y, 1);
        if (status != CUBLAS_STATUS_SUCCESS)
          throw InternalError(__func__, __FILE__, __LINE__, "cublasDgemv failed with status code: " + stringify(status));
      }
    }
  }
}


using namespace FEAT;
using namespace FEAT::LAFEM;
using namespace FEAT::LAFEM::Arch;

template <typename DT_>
void Defect<Mem::CUDA>::csr(DT_ * r, const DT_ * const rhs, const DT_ * const val, const unsigned long * const col_ind, const unsigned long * const row_ptr, const DT_ * const x, const Index rows, const Index columns, const Index used_elements)
{
  Index blocksize = MemoryPool<Mem::CUDA>::blocksize_spmv;
  dim3 grid;
  dim3 block;
  block.x = blocksize;
  grid.x = (unsigned)ceil((rows)/(double)(block.x));

  FEAT::LAFEM::Intern::cuda_defect_csr<<<grid, block>>>(r, rhs, x, val, col_ind, row_ptr, rows);
#ifdef FEAT_DEBUG_MODE
  cudaDeviceSynchronize();
  cudaError_t last_error(cudaGetLastError());
  if (cudaSuccess != last_error)
    throw InternalError(__func__, __FILE__, __LINE__, "CUDA error occured in execution!\n" + stringify(cudaGetErrorString(last_error)));
#endif
}
template void Defect<Mem::CUDA>::csr(float *, const float * const, const float * const, const unsigned long * const, const unsigned long * const, const float * const, const Index, const Index, const Index);
template void Defect<Mem::CUDA>::csr(double *, const double * const, const double * const, const unsigned long * const, const unsigned long * const, const double * const, const Index, const Index, const Index);

template <typename DT_>
void Defect<Mem::CUDA>::csr(DT_ * r, const DT_ * const rhs, const DT_ * const val, const unsigned int * const col_ind, const unsigned int * const row_ptr, const DT_ * const x, const Index rows, const Index columns, const Index used_elements)
{
  const DT_ a(-1.);
  if (r == rhs)
  {
    cusparseMatDescr_t descr=0;
    cusparseCreateMatDescr(&descr);
    cusparseSetMatType(descr, CUSPARSE_MATRIX_TYPE_GENERAL);
    cusparseSetMatIndexBase(descr, CUSPARSE_INDEX_BASE_ZERO);

    DT_ one(1);
    FEAT::LAFEM::Intern::cusparse_defect_csr(CUSPARSE_OPERATION_NON_TRANSPOSE, (int)rows, (int)columns, (int)used_elements, &a, descr, val, (int*)row_ptr, (int*)col_ind, x, &one, r);

    cusparseDestroyMatDescr(descr);
  }
  else
  {
    cudaMemcpy(r, rhs, rows * sizeof(DT_), cudaMemcpyDeviceToDevice);

    cusparseMatDescr_t descr=0;
    cusparseCreateMatDescr(&descr);
    cusparseSetMatType(descr, CUSPARSE_MATRIX_TYPE_GENERAL);
    cusparseSetMatIndexBase(descr, CUSPARSE_INDEX_BASE_ZERO);

    DT_ one(1);
    FEAT::LAFEM::Intern::cusparse_defect_csr(CUSPARSE_OPERATION_NON_TRANSPOSE, (int)rows, (int)columns, (int)used_elements, &a, descr, val, (int*)row_ptr, (int*)col_ind, x, &one, r);

    cusparseDestroyMatDescr(descr);
  }

#ifdef FEAT_DEBUG_MODE
  cudaDeviceSynchronize();
  cudaError_t last_error(cudaGetLastError());
  if (cudaSuccess != last_error)
    throw InternalError(__func__, __FILE__, __LINE__, "CUDA error occured in execution!\n" + stringify(cudaGetErrorString(last_error)));
#endif
}
template void Defect<Mem::CUDA>::csr(float *, const float * const, const float * const, const unsigned int * const, const unsigned int * const, const float * const, const Index, const Index, const Index);
template void Defect<Mem::CUDA>::csr(double *, const double * const, const double * const, const unsigned int * const, const unsigned int * const, const double * const, const Index, const Index, const Index);

template <typename DT_>
void Defect<Mem::CUDA>::csrb_intern(DT_ * r, const DT_ * const rhs, const DT_ * const val, const unsigned int * const col_ind, const unsigned int * const row_ptr, const DT_ * const x, const Index rows, const Index columns, const Index used_elements, const int blocksize)
{
  const DT_ a(-1.);
  if (r == rhs)
  {
    cusparseMatDescr_t descr=0;
    cusparseCreateMatDescr(&descr);
    cusparseSetMatType(descr, CUSPARSE_MATRIX_TYPE_GENERAL);
    cusparseSetMatIndexBase(descr, CUSPARSE_INDEX_BASE_ZERO);

    DT_ one(1);
    FEAT::LAFEM::Intern::cusparse_defect_csrb(CUSPARSE_DIRECTION_ROW, CUSPARSE_OPERATION_NON_TRANSPOSE, (int)rows, (int)columns, (int)used_elements, &a, descr, val, (int*)row_ptr, (int*)col_ind,
        blocksize, x, &one, r);

    cusparseDestroyMatDescr(descr);
  }
  else
  {
    cudaMemcpy(r, rhs, rows * blocksize * sizeof(DT_), cudaMemcpyDeviceToDevice);

    cusparseMatDescr_t descr=0;
    cusparseCreateMatDescr(&descr);
    cusparseSetMatType(descr, CUSPARSE_MATRIX_TYPE_GENERAL);
    cusparseSetMatIndexBase(descr, CUSPARSE_INDEX_BASE_ZERO);

    DT_ one(1);
    FEAT::LAFEM::Intern::cusparse_defect_csrb(CUSPARSE_DIRECTION_ROW, CUSPARSE_OPERATION_NON_TRANSPOSE, (int)rows, (int)columns, (int)used_elements, &a, descr, val, (int*)row_ptr, (int*)col_ind,
        blocksize, x, &one, r);

    cusparseDestroyMatDescr(descr);
  }

#ifdef FEAT_DEBUG_MODE
  cudaDeviceSynchronize();
  cudaError_t last_error(cudaGetLastError());
  if (cudaSuccess != last_error)
    throw InternalError(__func__, __FILE__, __LINE__, "CUDA error occured in execution!\n" + stringify(cudaGetErrorString(last_error)));
#endif
}
template void Defect<Mem::CUDA>::csrb_intern(float *, const float * const, const float * const, const unsigned int * const, const unsigned int * const, const float * const, const Index, const Index, const Index, const int);
template void Defect<Mem::CUDA>::csrb_intern(double *, const double * const, const double * const, const unsigned int * const, const unsigned int * const, const double * const, const Index, const Index, const Index, const int);


template <typename DT_, typename IT_>
void Defect<Mem::CUDA>::ell(DT_ * r, const DT_ * const rhs, const DT_ * const val, const IT_ * const col_ind, const IT_ * const cs, const IT_ * const cl, const DT_ * const x, const Index C, const Index rows)
{
  Index blocksize = MemoryPool<Mem::CUDA>::blocksize_spmv;
  dim3 grid;
  dim3 block;
  block.x = blocksize;
  grid.x = (unsigned)ceil((rows)/(double)(block.x));

  FEAT::LAFEM::Intern::cuda_defect_ell<<<grid, block>>>(r, rhs, x, val, col_ind, cs, cl, rows, C);
#ifdef FEAT_DEBUG_MODE
  cudaDeviceSynchronize();
  cudaError_t last_error(cudaGetLastError());
  if (cudaSuccess != last_error)
    throw InternalError(__func__, __FILE__, __LINE__, "CUDA error occured in execution!\n" + stringify(cudaGetErrorString(last_error)));
#endif
}
template void Defect<Mem::CUDA>::ell(float *, const float * const, const float * const, const unsigned long * const, const unsigned long * const, const unsigned long * const, const float * const, const Index, const Index);
template void Defect<Mem::CUDA>::ell(double *, const double * const, const double * const, const unsigned long * const, const unsigned long * const, const unsigned long * const, const double * const, const Index, const Index);
template void Defect<Mem::CUDA>::ell(float *, const float * const, const float * const, const unsigned int * const, const unsigned int * const, const unsigned int * const, const float * const, const Index, const Index);
template void Defect<Mem::CUDA>::ell(double *, const double * const, const double * const, const unsigned int * const, const unsigned int * const, const unsigned int * const, const double * const, const Index, const Index);

template <typename DT_, typename IT_>
void Defect<Mem::CUDA>::banded(DT_ * r, const DT_ * const rhs, const DT_ * const val, const IT_ * const offsets, const DT_ * const x, const Index num_of_offsets, const Index rows, const Index columns)
{
  Index blocksize(128);
  dim3 grid;
  dim3 block;
  block.x = blocksize;
  grid.x = (unsigned)ceil((rows)/(double)(block.x));

  FEAT::LAFEM::Intern::cuda_defect_banded<<<grid, block>>>(r, rhs, x, val, offsets, num_of_offsets, rows, columns);
}
template void Defect<Mem::CUDA>::banded(float *, const float *, const float * const, const unsigned long * const, const float * const, const Index, const Index, const Index);
template void Defect<Mem::CUDA>::banded(double *, const double *, const double * const, const unsigned long * const, const double * const, const Index, const Index, const Index);
template void Defect<Mem::CUDA>::banded(float *, const float *, const float * const, const unsigned int * const, const float * const, const Index, const Index, const Index);
template void Defect<Mem::CUDA>::banded(double *, const double *, const double * const, const unsigned int * const, const double * const, const Index, const Index, const Index);

template <typename DT_>
void Defect<Mem::CUDA>::dense(DT_ * r, const DT_ * const rhs, const DT_ * const val, const DT_ * const x, const Index rows, const Index columns)
{
  const DT_ a(-1.);
  if (r == rhs)
  {
    DT_ one(1);
    FEAT::LAFEM::Intern::cublas_defect_dense(CUBLAS_OP_N, (int)rows, (int)columns, &a, val, x, &one, r);
  }
  else
  {
    cudaMemcpy(r, rhs, rows * sizeof(DT_), cudaMemcpyDeviceToDevice);
    DT_ one(1);
    FEAT::LAFEM::Intern::cublas_defect_dense(CUBLAS_OP_T, (int)rows, (int)columns, &a, val, x, &one, r);
  }

#ifdef FEAT_DEBUG_MODE
  cudaDeviceSynchronize();
  cudaError_t last_error(cudaGetLastError());
  if (cudaSuccess != last_error)
    throw InternalError(__func__, __FILE__, __LINE__, "CUDA error occured in execution!\n" + stringify(cudaGetErrorString(last_error)));
#endif
}
template void Defect<Mem::CUDA>::dense(float *, const float * const, const float * const, const float * const, const Index, const Index);
template void Defect<Mem::CUDA>::dense(double *, const double * const, const double * const, const double * const, const Index, const Index);
