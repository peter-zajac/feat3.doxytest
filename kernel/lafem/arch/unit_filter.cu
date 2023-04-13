// FEAT3: Finite Element Analysis Toolbox, Version 3
// Copyright (C) 2010 - 2021 by Stefan Turek & the FEAT group
// FEAT3 is released under the GNU General Public License version 3,
// see the file 'copyright.txt' in the top level directory for details.

// includes, FEAT
#include <kernel/base_header.hpp>
#include <kernel/lafem/arch/unit_filter.hpp>
#include <kernel/util/exception.hpp>
#include <kernel/util/memory_pool.hpp>

/// \cond internal
namespace FEAT
{
  namespace LAFEM
  {
    namespace Intern
    {
      template <typename DT_, typename IT_>
      __global__ void cuda_unit_filter_rhs(DT_ * v, const DT_ * sv_elements, const IT_ * sv_indices, const Index ue)
      {
        Index idx = threadIdx.x + blockDim.x * blockIdx.x;
        if (idx >= ue)
          return;
        v[sv_indices[idx]] = sv_elements[idx];
      }

      template <typename DT_, typename IT_>
      __global__ void cuda_unit_filter_def(DT_ * v, const IT_ * sv_indices, const Index ue)
      {
        Index idx = threadIdx.x + blockDim.x * blockIdx.x;
        if (idx >= ue)
          return;
        v[sv_indices[idx]] = DT_(0);
      }
    }
  }
}


using namespace FEAT;
using namespace FEAT::LAFEM;
using namespace FEAT::LAFEM::Arch;

template <typename DT_, typename IT_>
void UnitFilter::filter_rhs_cuda(DT_ * v, const DT_ * const sv_elements, const IT_ * const sv_indices, const Index ue)
{
  Index blocksize = Util::cuda_blocksize_misc;
  dim3 grid;
  dim3 block;
  block.x = blocksize;
  grid.x = (unsigned)ceil((ue)/(double)(block.x));

  FEAT::LAFEM::Intern::cuda_unit_filter_rhs<<<grid, block>>>(v, sv_elements, sv_indices, ue);
#ifdef FEAT_DEBUG_MODE
  cudaDeviceSynchronize();
  cudaError_t last_error(cudaGetLastError());
  if (cudaSuccess != last_error)
    throw InternalError(__func__, __FILE__, __LINE__, "CUDA error occurred in execution!\n" + stringify(cudaGetErrorString(last_error)));
#endif
}

template void UnitFilter::filter_rhs_cuda(float *, const float * const, const unsigned long * const, const Index);
template void UnitFilter::filter_rhs_cuda(double *, const double * const, const unsigned long * const, const Index);
template void UnitFilter::filter_rhs_cuda(float *, const float * const, const unsigned int * const, const Index);
template void UnitFilter::filter_rhs_cuda(double *, const double * const, const unsigned int * const, const Index);

template <typename DT_, typename IT_>
void UnitFilter::filter_def_cuda(DT_ * v, const IT_ * const sv_indices, const Index ue)
{
  Index blocksize = Util::cuda_blocksize_misc;
  dim3 grid;
  dim3 block;
  block.x = blocksize;
  grid.x = (unsigned)ceil((ue)/(double)(block.x));

  FEAT::LAFEM::Intern::cuda_unit_filter_def<<<grid, block>>>(v, sv_indices, ue);
#ifdef FEAT_DEBUG_MODE
  cudaDeviceSynchronize();
  cudaError_t last_error(cudaGetLastError());
  if (cudaSuccess != last_error)
    throw InternalError(__func__, __FILE__, __LINE__, "CUDA error occurred in execution!\n" + stringify(cudaGetErrorString(last_error)));
#endif
}

template void UnitFilter::filter_def_cuda(float *, const unsigned long * const, const Index);
template void UnitFilter::filter_def_cuda(double *, const unsigned long * const, const Index);
template void UnitFilter::filter_def_cuda(float *, const unsigned int * const, const Index);
template void UnitFilter::filter_def_cuda(double *, const unsigned int * const, const Index);

/// \endcond
