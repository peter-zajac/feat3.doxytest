// includes, FEAST
#include <kernel/base_header.hpp>
#include <kernel/archs.hpp>
#include <kernel/lafem/arch/sum.hpp>

namespace FEAST
{
  namespace LAFEM
  {
    namespace Intern
    {
      template <typename DT_>
      __global__ void cuda_sum(DT_ * r, const DT_ * x, const DT_ * y, const Index count)
      {
        Index idx = threadIdx.x + blockDim.x * blockIdx.x;
        if (idx >= count)
          return;
        r[idx] = x[idx] + y[idx];
      }
    }
  }
}


using namespace FEAST;
using namespace FEAST::LAFEM;
using namespace FEAST::LAFEM::Arch;

template <typename DT_>
void Sum<Mem::CUDA, Algo::CUDA>::value(DT_ * r, const DT_ * const x, const DT_ * const y, const Index size)
{
  Index blocksize(128);
  dim3 grid;
  dim3 block;
  block.x = blocksize;
  grid.x = (unsigned)ceil((size)/(double)(block.x));

  FEAST::LAFEM::Intern::cuda_sum<<<grid, block>>>(r, x, y, size);
}

template void Sum<Mem::CUDA, Algo::CUDA>::value(float *, const float * const, const float * const, const Index);
template void Sum<Mem::CUDA, Algo::CUDA>::value(double *, const double * const, const double * const, const Index);
