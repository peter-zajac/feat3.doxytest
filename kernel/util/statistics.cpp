#include <kernel/util/statistics.hpp>
#include <kernel/util/function_scheduler.hpp>

using namespace FEAT;

// static member initialisation
Index Statistics::_flops = Index(0);
KahanAccumulation Statistics::_time_reduction;
KahanAccumulation Statistics::_time_spmv;
KahanAccumulation Statistics::_time_axpy;
KahanAccumulation Statistics::_time_precon;
KahanAccumulation Statistics::_time_mpi_execute;
KahanAccumulation Statistics::_time_mpi_wait;
std::map<FEAT::String, SolverStatistics> Statistics::_solver_statistics;
double Statistics::toe_partition;
double Statistics::toe_assembly;
double Statistics::toe_solve;

void Statistics::write_out_solver_statistics_scheduled(Index rank, size_t la_bytes, size_t domain_bytes, size_t mpi_bytes, Index cells, Index dofs, Index nzes, String filename)
{
  auto func = [&] () { write_out_solver_statistics(rank, la_bytes, domain_bytes, mpi_bytes, cells, dofs, nzes, filename); };
  Util::schedule_function(func, Util::ScheduleMode::clustered);
}
