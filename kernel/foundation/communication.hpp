#pragma once
#ifndef KERNEL_FOUNDATION_COMMUNICATION_HH
#define KERNEL_FOUNDATION_COMMUNICATION_HH 1

#ifndef FEAST_SERIAL_MODE
#include<mpi.h>
#endif
#include <kernel/foundation/base.hpp>
#include <kernel/archs.hpp>
#include <kernel/foundation/communication_error.hpp>
#include <kernel/foundation/buffer.hpp>

#include <vector>

using namespace FEAST::Archs;


namespace FEAST
{
  namespace Foundation
  {
    ///Communication modes used in Comm implementation or pass-over to backends
    enum Tier0CommModes
    {
      com_send = 0,
      com_receive,
      com_send_receive,
      com_gather,
      com_scatter,
      com_reduce_min
        //TODO...
    };

    enum Tier1CommModes
    {
      com_send_replace = 0,
      com_recv_replace,
      com_exchange,
      com_average,
      com_min,
      com_max
    };

#ifndef FEAST_SERIAL_MODE
    template <typename DT_>
      class MPIType
      {
      };

    template <>
      class MPIType<float>
      {
        public:
          static inline MPI_Datatype value()
          {
            return MPI_FLOAT;
          }
      };

    template <>
      class MPIType<double>
      {
        public:
          static inline MPI_Datatype value()
          {
            return MPI_DOUBLE;
          }
      };

    template <>
      class MPIType<unsigned long>
      {
        public:
          static inline MPI_Datatype value()
          {
            return MPI_UNSIGNED_LONG;
          }
      };

    template <>
      class MPIType<unsigned>
      {
        public:
          static inline MPI_Datatype value()
          {
            return MPI_UNSIGNED;
          }
      };

    template <>
      class MPIType<int>
      {
        public:
          static inline MPI_Datatype value()
          {
            return MPI_INT;
          }
      };
#endif

    /**
     * \brief Tier-0 Communication implementation or backend pass-over
     *
     * Foundation Tier-0 Comm protocols involve simple pointers as buffers.
     *
     * See specialisations.
     *
     * \tparam Tag_
     * backend specifier
     *
     * \author Markus Geveler
     */
    template<typename Tag_>
      class Comm
      {
      };

    ///example shared-mem exchange
    template<>
      class Comm<Archs::Serial>
      {
        public:
          template<typename DataType1_, typename DataType2_>
            static inline void send_recv(DataType1_ * sendbuf,
                                         Index num_elements_to_send,
                                         Index dest_rank,
                                         DataType2_* recvbuf,
                                         Index num_elements_to_recv,
                                         Index source_rank)
            {
              const Index send_end(num_elements_to_send);
              const Index recv_end(num_elements_to_recv);
              DataType1_ bufsend(0);
              DataType2_ bufrecv(0);

              for(Index i(0) ; i < send_end ; ++i)
              {
                bufsend = (DataType1_)recvbuf[i];
                recvbuf[i] = (DataType2_)sendbuf[i];
                recvbuf[i] = bufsend;
              }
              for(Index i(0) ; i < recv_end ; ++i)
              {
                bufrecv = (DataType2_)sendbuf[i];
                sendbuf[i] = (DataType1_)recvbuf[i];
                recvbuf[i] = bufrecv;
              }
            }

          //TODO
      };

#ifndef FEAST_SERIAL_MODE
    template<>
      class Comm<Archs::Parallel>
      {
        public:
          template<typename DataType1_, typename DataType2_>
            static inline void send_recv(DataType1_ * sendbuf,
                                         Index num_elements_to_send,
                                         Index dest_rank,
                                         DataType2_* recvbuf,
                                         Index num_elements_to_recv,
                                         Index source_rank)
            {
              MPI_Status status;

              MPI_Sendrecv(sendbuf,
                           num_elements_to_send,
                           MPIType<DataType1_>::value(),
                           dest_rank,
                           999,
                           recvbuf,
                           num_elements_to_recv,
                           MPIType<DataType2_>::value(),
                           source_rank,
                           999,
                           MPI_COMM_WORLD,
                           &status);
            }

          //TODO
      };
#endif

    ///Tier-1 implementation: Foundation Tier-1 Comm protocols use Tier-0 protocols and define how Foundation datastructures can be communicated.
    template<typename B_, Tier0CommModes c_>
    class Communicateable
    {
    };

    ///implemented by Bufferable Foundation datastructures that can be communicated
    template<typename BufferType_>
    class Communicateable<BufferType_, com_send_receive>
    {
      public:
        virtual void send_recv(BufferType_& senddata,
                               int destrank,
                               BufferType_& recvdata,
                               int sourcerank) = 0;
    };

    template<typename T_, Tier0CommModes c_>
    class CommunicateableByAggregates
    {
    };

    ///inherited by Foundation datastructures that can be communicated but don't need to be buffered because all their aggregates already are
    template<typename T_>
    class CommunicateableByAggregates<T_, com_send_receive>
    {
      public:
        template<typename AggregateStorageType_>
        void send_recv(AggregateStorageType_& aggregates_to_communicate,
                       int destrank,
                       int sourcerank,
                       Index estimated_size_increase = 0)
        {
          for(Index i(0) ; i < aggregates_to_communicate.size() ; ++i)
          {
            typename T_::buffer_type_ sendbuf(aggregates_to_communicate.at(i).buffer());
            typename T_::buffer_type_ recvbuf(aggregates_to_communicate.at(i).buffer(estimated_size_increase));

            aggregates_to_communicate.at(i).to_buffer(sendbuf);

            aggregates_to_communicate.at(i).send_recv(sendbuf, destrank, recvbuf, sourcerank);

            aggregates_to_communicate.at(i).from_buffer(recvbuf);
          }
        }
    };

    /**
     * \brief Tier-2 Communication implementation or backend pass-over
     *
     * Foundation Tier-2 Comm protocols use Foundation datastructures to orchestrate communication.
     *
     * See specialisations.
     *
     * \author Markus Geveler
     */
    template<Tier1CommModes op_>
    class InterfacedComm
    {
    };

    template<>
    class InterfacedComm<com_exchange>
    {
      public:
        template<
          unsigned delta_,
          PolytopeLevels a_,
          typename b_,
          template<typename, typename> class c_,
          typename d_,
          template<unsigned,
                   PolytopeLevels,
                   typename,
                   template<typename, typename> class,
                   typename>
           class HaloType_,
           typename AT_>
         static void execute(const HaloType_<delta_, a_, b_, c_, d_>& interface, AT_& fct)
         {
           //acquire buffers
           std::shared_ptr<SharedArrayBase > sendbuf(BufferedSharedArray<typename AT_::data_type_>::create(interface.size()));
           std::shared_ptr<SharedArrayBase > recvbuf(BufferedSharedArray<typename AT_::data_type_>::create(interface.size()));

           //collect data
           for(Index i(0) ; i < interface.size() ; ++i)
           {
             (*((BufferedSharedArray<typename AT_::data_type_>*)(sendbuf.get())))[i] = fct.at(interface.get_element(i));
           }

           //post send_recv
           ///TODO validate via mesh reference, that polytope level is correct
#ifndef FEAST_SERIAL_MODE
          Comm<Parallel>::send_recv(((BufferedSharedArray<typename AT_::data_type_>*)(sendbuf.get()))->get(),
              interface.size(),
              interface.get_other(),
              ((BufferedSharedArray<typename AT_::data_type_>*)(recvbuf.get()))->get(),
              interface.size(),
              interface.get_other());
#else
          Comm<Serial>::send_recv(((BufferedSharedArray<typename AT_::data_type_>*)(sendbuf.get()))->get(),
              interface.size(),
              interface.get_other(),
              ((BufferedSharedArray<typename AT_::data_type_>*)(recvbuf.get()))->get(),
              interface.size(),
              interface.get_other());
#endif
          //reset values
           for(Index i(0) ; i < interface.size() ; ++i)
           {
              fct.at(interface.get_element(i)) = (*((BufferedSharedArray<typename AT_::data_type_>*)(recvbuf.get())))[i];
           }

           //buffers are destroyed automatically
         }
    };

    template<typename TopologyType_>
      struct CommStructures
      {
        CommStructures(const TopologyType_& n, const TopologyType_& p) :
          network(n),
          patch_mesh(p),
          patch_process_map(TopologyType_()),
          process_patch_map(TopologyType_())
        {
        }

        const TopologyType_& network;
        const TopologyType_& patch_mesh;
        TopologyType_ patch_process_map;
        TopologyType_ process_patch_map;
      };
  }
}
#endif
