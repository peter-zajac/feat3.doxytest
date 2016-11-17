// includes, FEAT
#include <kernel/base_header.hpp>
#include <kernel/archs.hpp>
#include <kernel/lafem/arch/apply.hpp>

#include <cstring>

using namespace FEAT;
using namespace FEAT::LAFEM;
using namespace FEAT::LAFEM::Arch;

template void Apply<Mem::Main>::banded_generic(float *, const float, const float * const, const float, const float * const, const float * const, const unsigned long * const, const Index, const Index, const Index);
template void Apply<Mem::Main>::banded_generic(double *, const double, const double * const, const double, const double * const, const double * const, const unsigned long * const, const Index, const Index, const Index);
template void Apply<Mem::Main>::banded_generic(float *, const float, const float * const, const float, const float * const, const float * const, const unsigned int * const, const Index, const Index, const Index);
template void Apply<Mem::Main>::banded_generic(double *, const double, const double * const, const double, const double * const, const double * const, const unsigned int * const, const Index, const Index, const Index);
