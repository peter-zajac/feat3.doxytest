#pragma once
#ifndef FOUNDATION_GUARD_GLOBAL_SYNCH_SCAL_HPP
#define FOUNDATION_GUARD_GLOBAL_SYNCH_SCAL_HPP 1

#include<kernel/foundation/comm_base.hpp>
#include<kernel/foundation/communication.hpp>
#include<kernel/lafem/arch/scale.hpp>
#include<kernel/lafem/arch/component_product.hpp>
#include<kernel/lafem/arch/dot_product.hpp>
#include<kernel/util/time_stamp.hpp>
#include<kernel/util/statistics.hpp>

namespace FEAST
{
  namespace Foundation
  {
      /// \todo add communicators
      template <typename Mem_>
      struct GlobalSynchScal0
      {
        public:
#ifndef SERIAL
          template<typename DT_>
          static DT_ value(DT_& r,
                           DT_& x)
          {
            TimeStamp ts_start;

            DT_ sendbuf(x), recvbuf;

            Status stat;

            Request req;

            Comm::iallreduce(&sendbuf, Index(1), &recvbuf, req);

            Comm::wait(req, stat);

            r = recvbuf;

            TimeStamp ts_stop;
            Statistics::add_time_mpi_execute(ts_stop.elapsed(ts_start));
            return r;
          }
#else
          template<typename DT_>
          static DT_ value(DT_& r,
                           DT_& x)
          {
            r = x;
            return r;
          }
#endif
      };
  }
}


#endif
