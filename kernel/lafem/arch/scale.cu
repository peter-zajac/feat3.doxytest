// includes, FEAST
#include <kernel/lafem/arch/scale.hpp>

namespace FEAST
{
  namespace LAFEM
  {
    namespace Intern
    {
      template <typename DT_>
      __global__ void cuda_scale(DT_ * r, const DT_ * x, const DT_ s, const Index count)
      {
        Index idx = threadIdx.x + blockDim.x * blockIdx.x;
        if (idx >= count)
          return;
        r[idx] = x[idx] * s;
      }
    }
  }
}


using namespace FEAST;
using namespace FEAST::LAFEM;
using namespace FEAST::LAFEM::Arch;

template <typename DT_>
void Scale<Mem::CUDA, Algo::CUDA>::value(DT_ * r, const DT_ * const x, const DT_ s, const Index size)
{
  Index blocksize(128);
  dim3 grid;
  dim3 block;
  block.x = blocksize;
  grid.x = (unsigned)ceil((size)/(double)(block.x));

  FEAST::LAFEM::Intern::cuda_scale<<<grid, block>>>(r, x, s, size);
}
template void Scale<Mem::CUDA, Algo::CUDA>::value(float *, const float * const, const float, const Index);
template void Scale<Mem::CUDA, Algo::CUDA>::value(double *, const double * const, const double, const Index);
