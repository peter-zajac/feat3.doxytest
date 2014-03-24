// includes, FEAST
#include <kernel/base_header.hpp>
#include <kernel/archs.hpp>
#include <kernel/lafem/arch/norm.hpp>

using namespace FEAST;
using namespace FEAST::LAFEM;
using namespace FEAST::LAFEM::Arch;

template float Norm2<Mem::Main, Algo::Generic>::value(const float * const, const Index);
template double Norm2<Mem::Main, Algo::Generic>::value(const double * const, const Index);
