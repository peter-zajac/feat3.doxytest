// FEAT3: Finite Element Analysis Toolbox, Version 3
// Copyright (C) 2010 - 2020 by Stefan Turek & the FEAT group
// FEAT3 is released under the GNU General Public License version 3,
// see the file 'copyright.txt' in the top level directory for details.

#include <kernel/base_header.hpp>
#include <kernel/lafem/dense_vector.hpp>
#include <kernel/lafem/arch/axpy.hpp>
#include <kernel/util/type_traits.hpp>
#include <benchmarks/benchmark.hpp>
#include <kernel/util/runtime.hpp>

#include <iostream>

using namespace FEAT;
using namespace FEAT::LAFEM;
using namespace FEAT::Benchmark;

template<typename Algo_, typename DT_, typename IT_>
struct AxpyBench;

template<typename DT_, typename IT_>
struct AxpyBench<Algo::Generic, DT_, IT_>
{
  static void f(const DenseVector<Mem::Main, DT_, IT_> & x, DenseVector<Mem::Main, DT_, IT_> & y, DT_ s)
  {
    Arch::Axpy<Mem::Main>::dv_generic(y.elements(), s, x.elements(), y.elements(), x.size());
  }
};

template<typename DT_, typename IT_>
struct AxpyBench<Algo::MKL, DT_, IT_>
{
  static void f(const DenseVector<Mem::Main, DT_, IT_> & x, DenseVector<Mem::Main, DT_, IT_> & y, DT_ s)
  {
    Arch::Axpy<Mem::Main>::dv_mkl(y.elements(), s, x.elements(), y.elements(), x.size());
  }
};

template<typename DT_, typename IT_>
struct AxpyBench<Algo::CUDA, DT_, IT_>
{
  static void f(const DenseVector<Mem::CUDA, DT_, IT_> & x, DenseVector<Mem::CUDA, DT_, IT_> & y, DT_ s)
  {
    Arch::Axpy<Mem::CUDA>::dv(y.elements(), s, x.elements(), y.elements(), x.size());
  }
};


template <typename Algo_, typename VT_>
void run()
{
  typedef typename VT_::DataType DT_;
  typedef typename VT_::IndexType IT_;
  typedef typename VT_::MemType Mem_;

  Index size(50000000ul);
  std::cout<<Mem_::name()<<" "<<Algo_::name()<<" "<<Type::Traits<DT_>::name()<<" "<<Type::Traits<IT_>::name()<<std::endl;
  std::cout<<"vector size: "<<size<<std::endl;
  DenseVector<Mem_, DT_, IT_> x(size, DT_(1.234));
  DenseVector<Mem_, DT_, IT_> y(size, DT_(4711));
  DT_ s(23);

  double flops = double(size);
  flops *= 2;

  double bytes = double(size);
  bytes *= 3;
  bytes *= sizeof(DT_);

  auto func = [&] () { AxpyBench<Algo_, DT_, IT_>::f(x, y, s); };
  run_bench<Mem_>(func, flops, bytes);
}

int main(int argc, char ** argv)
{
  Runtime::initialize(argc, argv);
#ifdef FEAT_HAVE_CUDA
  run<Algo::CUDA, DenseVector<Mem::CUDA, float, Index> >();
  run<Algo::CUDA, DenseVector<Mem::CUDA, double, Index> >();
#endif
  run<Algo::Generic, DenseVector<Mem::Main, float, Index> >();
  run<Algo::Generic, DenseVector<Mem::Main, double, Index> >();
#ifdef FEAT_HAVE_MKL
  run<Algo::MKL, DenseVector<Mem::Main, float, Index> >();
  run<Algo::MKL, DenseVector<Mem::Main, double, Index> >();
#endif
  Runtime::finalize();
}
