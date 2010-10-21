#pragma once
#ifndef KERNEL_PROCESS_GROUP_HPP
#define KERNEL_PROCESS_GROUP_HPP 1

// includes, system
#include <iostream>
#include <stdlib.h>
#include <vector>
#include <mpi.h>
#include <math.h>

// includes, Feast
#include <kernel/base_header.hpp>
#include <kernel/logger.hpp>
#include <kernel/process.hpp>
#include <kernel/graph.hpp>

/**
* \brief group of processes sharing an MPI communicator
*
* \author Hilmar Wobker
*/
class ProcessGroup
{

private:

protected:
  /* *****************
  * member variables *
  *******************/
  /// MPI group representing the processes of the group
  MPI_Group _group;

  /// communicator shared by all processes of the group
  MPI_Comm _comm;

  /// number of processes in this group
  int _num_processes;

  /// rank of this process with respect to the ProcessGroup's communicator
  int _rank;

  /**
  * \brief array of ranks the processes of this group have in the parent group
  *
  * Dimension: [#_num_processes]
  */
  // COMMENT_HILMAR, 15.9.2010:
  // Currently, this is simply a pointer to the part of the corresponding array
  // _group_ranks_world[my_group] in universe.hpp. Maybe it makes sense to copy the array and delete the one in
  // universe.hpp.

// COMMENT_HILMAR: Muessen wir das abspeichern? Kann man es nicht an die MPI Routine uebergeben und dann wieder
// wegschmeissen?
  int* _ranks_group_parent;

  /// pointer to the process group from which this process group has been spawned
  ProcessGroup* _process_group_parent;

//    /// process groups spawned from this process group
// COMMENT_HILMAR, 15.9.2010: not sure yet if this is needed
//    ProcessGroup** _process_group_child;

  /**
  * \brief ID of the process group
  *
  * When several process groups are "spawned" from one parent process group, then these process groups can be
  * distinguished via this group ID. It has to be passed to the process group constructor, it is not incremented
  * automatically.
  */
  int const _group_id;

  /// rank of the coordinator process within the group
  int const _rank_coord;

// COMMENT_HILMAR: every process group will certainly need its own MPI buffer... then activate this code.
//  /// buffer size
//  static int BUFFERSIZE_BYTES;
//
//  /// buffer for MPI communication
//  char* _buffer;


public:

  /* *************************
  * constructor & destructor *
  ***************************/
  /**
  * \brief constructor for the case the MPI_Group and the MPI_Comm already exist
  *
  * This constructor is intended for creating a process group object containing all COMM_WORLD processes. The
  * coordinator is set to rank #_num_processes-1, i.e. the last rank in the process group.
  *
  * \param[in] comm
  * communicator shared by the group processes
  *
  * \param[in] num_processes
  * number of processes in the group
  */
  ProcessGroup(
    MPI_Comm comm,
    int num_processes)
    : _comm(comm),
      _num_processes(num_processes),
      _ranks_group_parent(nullptr),
      _process_group_parent(nullptr),
      _group_id(-1),
      _rank_coord(num_processes-1)
  {
    // get MPI_Group object for the given communicator
    int mpi_error_code = MPI_Comm_group(_comm, &_group);
    MPIUtils::validate_mpi_error_code(mpi_error_code, "MPI_Comm_group");

    // and finally look up the local rank of this process w.r.t. the group's communicator
    mpi_error_code = MPI_Group_rank(_group, &_rank);
    MPIUtils::validate_mpi_error_code(mpi_error_code, "MPI_Group_rank");

    // since this is the world group of processes, the local and the global rank should be equal
    assert(Process::rank == _rank);

// COMMENT_HILMAR: every process group will certainly need its own MPI buffer... then activate this code.
//    _buffer = nullptr;
  }

  /**
  * \brief constructor for the case the MPI_Group and the corresponding communicator have to be created
  *
  * This constructor can be used for splitting the complete set of COMM_WORLD processes into subgroups for performing
  * two or more completely separated tasks (e.g., for multiphysics).
  * The coordinator of this process group is set to rank #_num_processes-1, i.e. the last rank in the process group.
  */
  ProcessGroup(
    int num_processes,
    int ranks_group_parent[],
    ProcessGroup* process_group_parent,
    int const group_id)
    : _num_processes(num_processes),
      _ranks_group_parent(ranks_group_parent),
      _process_group_parent(process_group_parent),
      _group_id(group_id),
      _rank_coord(num_processes-1)
  {
    int mpi_error_code = MPI_Group_incl(_process_group_parent->_group, _num_processes,
                                        ranks_group_parent, &_group);
    MPIUtils::validate_mpi_error_code(mpi_error_code, "MPI_Group_incl");

    // Create the group communicator for, among others, collective operations.
    // It is essential that *all* processes in MPI_COMM_WORLD participate since MPI_COMM_WORLD
    // is a parent sub-universe, i.e., something to spawn from
    mpi_error_code = MPI_Comm_create(_process_group_parent->_comm, _group, &_comm);
    MPIUtils::validate_mpi_error_code(mpi_error_code, "MPI_Comm_create");

    // and finally look up the local rank of this process w.r.t. the group's communicator
    mpi_error_code = MPI_Group_rank(_group, &_rank);
    MPIUtils::validate_mpi_error_code(mpi_error_code, "MPI_Group_rank");

// COMMENT_HILMAR: every process group will certainly need its own MPI buffer... then activate this code.
//    // allocate communication buffers
//    int size_of_char;
//    mpi_error_code = MPI_Pack_size(1, MPI_CHAR, MPI_COMM_WORLD, &size_of_char);
//    MPIUtils::validate_mpi_error_code(mpi_error_code, "MPI_Pack_size");
//
//    if (size_of_char != 1)
//    {
//      // In the unlikely case that a char is not 1 byte, determine the size of the buffer. The -0.001 is a precaution
//      // for the case the division results in 42.00000000001 instead of 42.0.
//      int buffer_size = ceil(BUFFERSIZE_BYTES/size_of_char - 0.001);
// //    std::cout << "buffer size: " << StringUtils::stringify(buffer_size) <<  std::endl;
//      _buffer = new char[buffer_size];
//    }
//    else
//    {
//      // otherwise, array size (in bytes) and number of array entries are equal
//      _buffer = new char[BUFFERSIZE_BYTES];
//    }

  }

  /* ***********
  * destructor *
  *************/
  /// destructor
  ~ProcessGroup()
  {
// COMMENT_HILMAR: every process group will certainly need its own MPI buffer... then activate this code.
//    if (_buffer != nullptr)
//    {
//      delete [] _buffer;
//    }
  }

  /* ******************
  * getters & setters *
  ********************/
  /**
  * \brief getter for the MPI group
  *
  * Return a reference in order to avoid making a copy. Make this return reference constant so that the user
  * cannot change the object.
  *
  * \return reference to MPI_Group #_group
  */
  inline const MPI_Group& group() const
  {
    return _group;
  }

  /**
  * \brief getter for the MPI communicator
  *
  * Return a reference in order to avoid making a copy. Make this return reference constant so that the user
  * cannot change the object.
  *
  * \return reference to MPI_Comm #_comm
  */
  inline const MPI_Comm& comm() const
  {
    return _comm;
  }

  /**
  * \brief getter for the number of processes
  *
  * \return number of processes #_num_processes
  */
  inline int num_processes() const
  {
    return _num_processes;
  }

  /**
  * \brief getter for the pointer to the parent process group
  *
  * \return parent process group pointer #_process_group_parent
  */
  inline ProcessGroup* process_group_parent() const
  {
    return _process_group_parent;
  }

  /**
  * \brief getter for group ID
  *
  * \return group ID #_group_id
  */
  inline int group_id() const
  {
    return _group_id;
  }

  /**
  * \brief getter for the rank w.r.t. the group communicator this process belongs to
  *
  * \return rank #_rank w.r.t. process group communicator
  */
  inline int rank() const
  {
    return _rank;
  }

  /**
  * \brief getter for the rank w.r.t. the group communicator this process belongs to
  *
  * \return rank #_rank w.r.t. process group communicator
  */
  inline int rank_coord() const
  {
    return _rank_coord;
  }


  /* *****************
  * member functions *
  *******************/
  /**
  * \brief checks whether this process is the coordinator of the process group
  *
  * \return true if this process is the coordinator of the process group, otherwise false
  */
  inline bool is_coordinator() const
  {
    return _rank == _rank_coord;
  }

  /**
  * \brief sending individual messages to the master
  *
  * With this function the processes of the process group can send individual log messages to the master. The
  * coordinator of the process group collects all these log messages in the order of process group ranks and sends
  * them as one MPI message to the master. Depending on the target the master then displays these messages on the
  * screen or writes them to the log file (one line per message). This function has to be called by \em all processes
  * of the process group! Note no further information will be appended to the messages (like rank of the process from
  * which the messages comes). Such infomration has to be coded directly into the message string.
  *
  * \param[in] message
  * string representing the log message
  *
  * \param[in] target
  * output target Logger::SCREEN, Logger::FILE or Logger::SCREEN_FILE (default: Logger::SCREEN_FILE)
  *
  * \sa Logger::log_master_array()
  *
  * \author Hilmar Wobker
  */
  void log_indiv_master(std::string message, Logger::target target = Logger::SCREEN_FILE)
  {

    // add 1 to the message length since string::c_str() adds the null termination symbol to the resulting char array
    int length = message.length() + 1;

    if (!is_coordinator())
    {
      /* ***************************************
      * code for all non-coordinator processes *
      *****************************************/

      // coordinator process gathers the lengths of the messages
      MPI_Gather(&length, 1, MPI_INT, nullptr, 0, MPI_DATATYPE_NULL, _rank_coord, _comm);

      // coordinator process gathers the messages
      // (Here, it is necessary to cast away the const'ness of string::c_str() since the MPI routine expects a
      // non-const send buffer.)
      MPI_Gatherv(const_cast<char *>(message.c_str()), length, MPI_CHAR, nullptr, nullptr, nullptr,
                  MPI_DATATYPE_NULL, _rank_coord, _comm);
// COMMENT_HILMAR: not sure, if it is actually legitimate to simply cast the const away or if one has to manually copy
// the string like this:
//    char bla[length];
//    strcpy(bla, message.c_str());
//    MPI_Gatherv(bla, length, MPI_CHAR, nullptr, nullptr, nullptr, MPI_DATATYPE_NULL, _rank_coord, _comm);
// With gcc 4.4.0 and intel 11.1 it works for tiny test problems. (If the function call is changed, then also
// change the call on coordinator side some lines below.)
    }
    else
    {
      /* ********************************
      * code for the coordinator process *
      **********************************/

      // receive buffer for storing the lengths of the messages
      int msg_lengths[_num_processes];
      // array for storing the start positions of the single messages in the receive buffer
      int msg_start_pos[_num_processes];

      // gather the lengths of the messages from the other processes
      MPI_Gather(&length, 1, MPI_INT, msg_lengths, 1, MPI_INT, _rank_coord, _comm);

      // set start positions of the single messages in the receive buffer
      msg_start_pos[0] = 0;
      for(int i(1) ; i < _num_processes ; ++i)
      {
        msg_start_pos[i] = msg_start_pos[i-1] + msg_lengths[i-1];
      }

      // determine total length of the messages and allocate receive buffer accordingly
      int total_length(msg_start_pos[_num_processes-1] + msg_lengths[_num_processes-1]);

      // receive buffer for (consecutively) storing the messages
      char messages[total_length];

//      // debug output
//      for(int i(0) ; i < _num_processes ; ++i)
//      {
//        std::cout << "local process " << StringUtils::stringify(i) << ": length of string = "
//                  << StringUtils::stringify(msg_lengths[i]) << std::endl;
//      }
//      std::cout << "Total length: " << StringUtils::stringify(total_length) << std::endl;

      // gather the messages from the other processes.
      MPI_Gatherv(const_cast<char *>(message.c_str()), length, MPI_CHAR, messages, msg_lengths, msg_start_pos,
                  MPI_CHAR, _rank_coord, _comm);

//      // debug output
//      // output the strings collected from all processes by using corresponding offsets in the
//      // char array (pointer arithmetic)
//      for(int i(0) ; i < _num_processes ; ++i)
//      {
//        std::cout << "Process " << Process::rank << " writes: "
//                  << std::string(messages + msg_start_pos[i], msg_lengths[i]) << std::endl;
//      }

      // now send everything to the master
      Logger::log_master_array(_num_processes, msg_lengths, total_length, messages, target);

    }
  } // log_indiv_master
}; // class ProcessGroup

// COMMENT_HILMAR: every process group will certainly need its own MPI buffer... then activate this code.
// // initialisation of static members
// COMMENT_HILMAR: Use some arbitrary size for the time being. This has to be parameterised somehow...
//int ProcessGroup::BUFFERSIZE_BYTES = 4194304;





/**
* \brief class describing a work group, i.e. a set of worker processes sharing the same MPI communicator and performing
*        the same compute task
*
* A WorkGroup object is part of a ProcessSubgroup. It describes a set of compute processes sharing the MPI communicator.
*
* The member of the work group living on this process is called "worker" (which is not realised via an extra class.)
*
* Communication between different work groups is done via the enclosing ProcessGroup communicator.
*
* For an example concerning WorkGroup and ProcessSubgroup creation, see the description of class ProcessSubgroup.
*
* \author Hilmar Wobker
* \author Dominik Goeddeke
*
*/
class WorkGroup
  : public ProcessGroup
{

private:

  /* *****************
  * member variables *
  *******************/
//  /**
//  * \brief additional communicator representing an optimised process topology
//  *
//  * This communicator results from a call to MPI_Dist_graph_create(...), which remaps the processes of this work group
//  * to optimise communication patterns.
//  */
// COMMENT_HILMAR: Everything that has to do with comm_opt is deactivated for the time being (see comment in routine
//    set_graph_distributed.
//  MPI_Comm _comm_opt;

  /**
  * \brief local portions of a distributed graph
  *
  * This object is sent to this process by the coordinator of the parent process group. It contains the parent process
  * group ranks of the neighbours of this worker. It can be used to create an MPI topology graph via
  * MPI_Dist_graph_create().
  */
  GraphDistributed* _graph_distributed;

  /**
  * \brief vector of process group ranks of members of the "next finer" work group this worker has to exchange
  *        data with
  *
  * Since those workers live in a different work group, this worker has to communicate with them via the
  * process group communicator. Note that one of those workers may have the same process group rank (i.e., live on the
  * same MPI_COMM_WORLD process) as this worker (hence, no MPI communication is necessary in this case).
  */
  std::vector<int> _ranks_finer; // COMMENT_HILMAR: find a better variable name!

  /**
  * \brief process group rank of the worker in the "next coarser" work group this worker has to exchange data with
  *        (this is always only one!)
  *
  * Since the worker lives in a different work group, this worker has to communicate with it via the process
  * group communicator. Note that the worker may have the same process group rank (i.e., live on the same
  * MPI_COMM_WORLD process) as this worker (hence, no MPI communication is necessary in this case).
  */
  int _rank_coarser; // COMMENT_HILMAR: find a better variable name!

// COMMENT_HILMAR, 20.10.2010:
// Bei den letzten beiden Variablen muss man abfragen, ob der entsprechende worker auf demselben Prozess lebt,
// denn MPI_send/recv mit source=destination ist unsafe. Dazuein  Zitat aus dem MPI2.2-Standard, Seite 31:
//   Source = destination is allowed, that is, a process can send a message to itself.
//   (However, it is unsafe to do so with the blocking send and receive operations described above,
//   since this may lead to deadlock. See Section 3.5.)
// Fuer den Fall, dass source und destination worker auf dem selben Prozess leben, muss kopiert anstatt kommuniziert
// werden. Dafuer muss eine entsprechende Funktion vorgesehen werden, und die beiden muessen sich
// datenstruktur-technisch "kennen". Das heisst, man wird in diesem Fall wohl noch einen Pointer auf die entsprechende
// WorkGroup abspeichern muessen... na, mal sehen, wenn's an die Details geht.

public:

  /* *************************
  * constructor & destructor *
  ***************************/
  /// constructor
  WorkGroup(
    const int num_processes,
    int ranks_group_parent[],
    ProcessGroup* process_group_parent,
    const int group_id)
    : ProcessGroup(num_processes, ranks_group_parent, process_group_parent, group_id),
//      _comm_opt(MPI_COMM_NULL),
      _graph_distributed(nullptr),
      _ranks_finer(nullptr)
  {
    // debugging output
    /* ******************************
    * test the logger functionality *
    ********************************/
//    // let the coordinator of the subgroup trigger some common messages
//    if(is_coordinator())
//    {
//      std::string s("I have COMM_WORLD rank " + StringUtils::stringify(Process::rank)
//                    + " and I am the coordinator of work group " + StringUtils::stringify(_group_id));
// //      Logger::log_master("Hello, master screen! " + s, Logger::SCREEN);
//      Logger::log_master("Hello, master file! " + s, Logger::FILE);
// //      Logger::log_master("Hello, master screen and file! " + s, Logger::SCREEN_FILE);
// //      Logger::log_master("Hello, default master screen and file! " + s);
//    }
    // write some individual messages to screen and file
    std::string s("I have COMM_WORLD rank " + StringUtils::stringify(Process::rank)
                  + " and group rank " + StringUtils::stringify(_rank)
                  + " in work group " + StringUtils::stringify(_group_id) + ".");
    if(is_coordinator())
    {
      s += " I am the coordinator!";
    }
//    log_indiv_master("Hello, master screen! " + s, Logger::SCREEN);
    log_indiv_master("Hello, master file! " + s, Logger::FILE);
//    log_indiv_master("Hello, master screen and file! " + s, Logger::SCREEN_FILE);
//    log_indiv_master("Hello, master default screen and file! " + s);

    // end of debugging output
  } // constructor


  /// destructor
  ~WorkGroup()
  {
    if(_graph_distributed != nullptr)
    {
      delete _graph_distributed;
    }
  }


  /* ******************
  * getters & setters *
  ********************/
//  /**
//  * \brief getter for the optimised MPI communicator
//  *
//  * Return a reference in order to avoid making a copy. Make this return reference constant so that the user
//  * cannot change the object.
//  *
//  * \return reference to MPI_Comm #_comm
//  */
//  inline const MPI_Comm& comm_opt() const
//  {
//    return _comm_opt;
//  }

  /* *****************
  * member functions *
  *******************/
  /**
  * \brief sets local graph portions of a distributed graph
  *
  * COMMENT_HILMAR: By now, the weights and info objects are empty. Does it make sense to, e.g., use different weights
  * for edge neighbours on the one hand and diagonal neighbours on the other hand? Should diagonal neighbours appear
  * in the graph topology at all? Or should we only take the edge neighbours here?
  */
  void set_graph_distributed(
    int const num_neighbours,
    int* edges)
  {
    _graph_distributed = new GraphDistributed(num_neighbours, edges);

// COMMENT_HILMAR, 14.10.2010: The plan was to use MPI_Dist_graph_create(...) to create the MPI graph topology.
//   Unfortunately, this function has only been introduced with MPI-2.2 and is not yet implemented in OpenMPI yet.
//   The only alternative that already exists, MPI_Graph_create(...), requires that all processes know the complete
//   graph (and not only the coordinator process). IMHO, it doesn't make sense to further pursue this MPI process
//   topology topic in the moment. Let us stick with the standard MPI communicators and let us see what the ITMC
//   will say.
//
//    int sources[1];
//    int degrees[1];
//    sources[0] = _rank;
//    degrees[0] = _graph_distributed->num_neighbours();
//    MPI_Dist_graph_create(_comm, 1, sources, degrees, _graph_distributed->neighbours(), nullptr,
//                          MPI_INFO_NULL, true, _comm_opt());
  }
};




/**
* \brief class describing a subgroup of a process group, consisting of some compute processes and eventually one
*        extra coordinator process, sharing the same MPI communicator
*
* ProcessSubgroup objects are created by the load balancer. They consist of n compute processes and eventually one extra
* process which is the coordinator of the parent process group. The latter is only the case if the coordinator is not
* part of the compute processes anyway. In both cases (containing an extra coordinator process or not), a
* ProcessSubgroup creates a WorkGroup object, which either consists of exactly the same processes as this
* ProcessSubgroup (in the case there is no extra coordinator process) or of the n compute processes only (excluding the
* extra coordinator process). Each ProcessSubgroup has its own MPI communicator. Since the coordinator of the parent
* process group is part of this communicator, all necessary information (mesh, graph, ...) can be transferred
* efficiently via collective communication routines. The extra WorkGroup object (excluding the eventual coordinator
* process) with its own communicator is necessary to efficiently perform collective communication during the actual
* computation (scalar products, norms, etc).
*
* Example:
* The process group of a load balancer consists of six processes, the sixth one being the coordinator of the process
* group. There is no dedicated load balancing process. The coordinator process (rank 5) reads the mesh and the solver
* configuration and decides that the coarse grid problem is to be treated by two compute processes (process group ranks
* 0 and 1) and the fine grid problems by six compute processes (ranks 0-5). Then two ProcessSubgroup objects are
* created: The first one (for the coarse grid problem) consists of the two compute processes with ranks 0 and 1 and the
* coordinator of the parent process group (rank 5). The other ProcessSubgroup (for the fine grid problem) object
* consists of all six processes (ranks 0-5). Here, the coordinator of the parent process group (rank 5) is also a
* compute process. Both ProcessSubgroups then create a WorkGroup object. The coarse grid work group consists of the
* two compute process (ranks 0 and 1) and thus differs from its ProcessSubgroup, while the fine grid work group contains
* exactly the same six processes as its ProcessSubgroup.
*
*    processes parent ProcessGroup:   0 1 2 3 4 5
*    coarse grid ProcessSubgroup:     x x       x
*    coarse grid WorkGroup:           x x
*    fine grid ProcessSubgroup:       x x x x x x
*    fine grid WorkGroup:             x x x x x x
*
* If the parent ProcessGroup contains seven processes with the seventh process being a dedicated load balancer and the
* coordinator at the same time, the ProcessSubgroups and WorkGroups look like this:
*
*    processes parent ProcessGroup:   0 1 2 3 4 5 6
*    coarse grid ProcessSubgroup:     x x         x
*    coarse grid WorkGroup:           x x
*    fine grid ProcessSubgroup:       x x x x x x x
*    fine grid WorkGroup:             x x x x x x
*
*
* \author Hilmar Wobker
* \author Dominik Goeddeke
*
*/
class ProcessSubgroup
  : public ProcessGroup
{

private:

  /* *****************
  * member variables *
  *******************/
  /// flag whether this group contains the coordinator of the parent process group as an extra process
  bool _contains_extra_coord;

  /**
  * \brief work group containing the containing only the real compute processes of this ProcessSubgroup
  *
  * If #_contains_extra_coord == false, then the work group contains all processes of this ProcessGroup, otherwise it
  * it contains only the compute processes excluding the the extra coordinator process. The parent process group of
  * the work group is not this ProcessSubgroup, but the parent group of this ProcessSubgroup. That is why, when
  * creating the work group, the dummy calls of MPI_Comm_create(...) by the remaining processes of the parent process
  * group are performed in the load balancer class and not here.
  */
  WorkGroup* _work_group;

public:

  /* *************************
  * constructor & destructor *
  ***************************/
  /// constructor
  ProcessSubgroup(
    const int num_processes,
    int ranks_group_parent[],
    ProcessGroup* process_group_parent,
    const int group_id,
    bool contains_extra_coord)
    : ProcessGroup(num_processes, ranks_group_parent, process_group_parent, group_id),
      _contains_extra_coord(contains_extra_coord),
      _work_group(nullptr)
  {
    /* ******************************
    * test the logger functionality *
    ********************************/
    // debugging output
//    // let the coordinator of the subgroup trigger some common messages
//    if(is_coordinator())
//    {
//      std::string s("I have COMM_WORLD rank " + StringUtils::stringify(Process::rank)
//                    + " and I am the coordinator of process subgroup " + StringUtils::stringify(_group_id));
// //      Logger::log_master("Hello, master screen! " + s, Logger::SCREEN);
//      Logger::log_master("Hello, master file! " + s, Logger::FILE);
// //      Logger::log_master("Hello, master screen and file! " + s, Logger::SCREEN_FILE);
// //      Logger::log_master("Hello, default master screen and file! " + s);
//    }

    // write some individual messages to screen and file
    std::string s("I have COMM_WORLD rank " + StringUtils::stringify(Process::rank)
                  + " and group rank " + StringUtils::stringify(_rank)
                  + " in process subgroup " + StringUtils::stringify(_group_id) + ".");
    if(is_coordinator())
    {
      s += " I am the coordinator!";
    }
    // vary the lengths of the messages a little bit
    for(int i(0) ; i < _rank+1 ; ++i)
    {
      s += " R";
    }
    for(int i(0) ; i < _group_id+1 ; ++i)
    {
      s += " G";
    }
//    log_indiv_master("Hello, master screen! " + s, Logger::SCREEN);
    log_indiv_master("Hello, master file! " + s, Logger::FILE);
//    log_indiv_master("Hello, master screen and file! " + s, Logger::SCREEN_FILE);
//    log_indiv_master("Hello, master default screen and file! " + s);

    // end of debugging output

    // create work group consisting of the real compute processes only (the dummy calls of MPI_Comm_create(...) by the
    // remaining processes of the parent process group are performed in the load balancer class and not here)
    if (!_contains_extra_coord)
    {
      // in the case there is no extra coordinator process, the work group of compute processes contains all processes
      // of this subgroup
// BRAL: Wenn wir beschliessen, dass _ranks_group_parent nicht abgespeichert werden muss, dann muessen wir hier nicht
// kopieren!
      int* work_group_ranks = new int[_num_processes];
      for(int i(0) ; i < _num_processes ; ++i)
      {
        work_group_ranks[i] = ranks_group_parent[i];
      }
      _work_group = new WorkGroup(_num_processes, work_group_ranks, _process_group_parent, _group_id + 42);
    }
    else
    {
      // otherwise, the work group contains all processes of this subgroup except the last one (the extra coordinator)
// BRAL: Wenn wir beschliessen, dass _ranks_group_parent nicht abgespeichert werden muss, dann muessen wir hier nicht
// kopieren. Die Methode MPI_Group_incl(...) bekommt dann das Array ranks_group_parent[] durchgereicht, und
// wegen _num_processes-1 wird nur auf die ersten _num_processes-1 Positionen zugegriffen.
      int* work_group_ranks = new int[_num_processes-1];
      for(int i(0) ; i < _num_processes - 1 ; ++i)
      {
        work_group_ranks[i] = ranks_group_parent[i];
      }
      if(!is_coordinator())
      {
        _work_group = new WorkGroup(_num_processes - 1, work_group_ranks, _process_group_parent, _group_id + 666);
      }
      else
      {
        // *All* processes of the parent MPI group have to call the MPI_Comm_create() routine (otherwise the forking
        // will deadlock), so let the coordinator call the routine with MPI_GROUP_EMPTY and dummy communicator.
        MPI_Comm dummy_comm;
        int mpi_error_code = MPI_Comm_create(_process_group_parent->comm(), MPI_GROUP_EMPTY, &dummy_comm);
        MPIUtils::validate_mpi_error_code(mpi_error_code, "MPI_Comm_create");
        _work_group = nullptr;
      }
    }
  } // constructor

  /// destructor
  ~ProcessSubgroup()
  {
    if(_work_group != nullptr)
    {
      delete _work_group;
    }
  }

  /**
  * \brief getter for the flag whether this subgroup contains an extra coordinator process
  *
  * \return pointer the flag #_contains_extra_coord
  */
  inline bool contains_extra_coord() const
  {
    return _contains_extra_coord;
  }

  /**
  * \brief getter for the compute work group
  *
  * \return pointer to the work group #_work_group
  */
  inline WorkGroup* work_group() const
  {
    return _work_group;
  }
};

#endif // guard KERNEL_PROCESS_GROUP_HPP
