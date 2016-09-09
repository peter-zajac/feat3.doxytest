#pragma once
#ifndef CONTROL_DOMAIN_PARTI_DOMAIN_CONTROL_HPP
#define CONTROL_DOMAIN_PARTI_DOMAIN_CONTROL_HPP 1

#include <kernel/base_header.hpp>

#include <kernel/util/comm_base.hpp>
#include <kernel/util/dist_file_io.hpp>
#include <kernel/util/runtime.hpp>
#include <kernel/util/mpi_cout.hpp>
#include <kernel/util/simple_arg_parser.hpp>
#include <kernel/foundation/pexecutor.hpp>
#include <kernel/foundation/pgraph.hpp>
#include <kernel/foundation/psynch.hpp>
#include <kernel/geometry/mesh_file_reader.hpp>
#include <kernel/geometry/mesh_node.hpp>
#include <kernel/geometry/partition_set.hpp>

#include <control/domain/domain_control.hpp>

namespace FEAT
{
  namespace Control
  {
    namespace Domain
    {
      /**
       * \brief Partitioned Domain Control
       *
       * \author Peter Zajac
       */
      template<typename MeshType_>
      class PartiDomainControl :
        public Control::Domain::DomainControl<MeshType_>
      {
      public:
        /// Our base class
        typedef Control::Domain::DomainControl<MeshType_> BaseClass;
        /// our domain level type
        typedef typename BaseClass::LevelType LevelType;
        /// our domain layer type
        typedef typename BaseClass::LayerType LayerType;
        /// our atlas type
        typedef typename BaseClass::AtlasType AtlasType;
        /// our root mesh node type
        typedef Geometry::RootMeshNode<MeshType_> MeshNodeType;

      protected:
        /// the adapt mode for refinement
        Geometry::AdaptMode _adapt_mode;
        /// specifies whether a mesh file has already been read
        bool _have_read_mesh;
        /// specifies whether the base-mesh has been partitioned
        bool _have_partition;
        /// specifies whether a mesh hierarchy has been created.
        bool _have_hierarchy;
        /// our base-mesh level
        int _base_mesh_level;
        /// allow manual partitioner?
        bool _allow_parti_manual;
        /// allow parmetis partitioner?
        bool _allow_parti_parmetis;
        /// allow fallback partitioner?
        bool _allow_parti_fallback;
        /// desired name of manual partition
        String _manual_parti_name;
        /// minimum number of elements per rank for automatic partitioning
        Index _min_elems_per_rank;
        /// our partition set for manual partitioning
        Geometry::PartitionSet _part_set;
        /// our base-mesh node
        MeshNodeType* _base_mesh_node;
        /// our patch-mesh node
        MeshNodeType* _patch_mesh_node;

      public:
        /// default constructor
        PartiDomainControl() :
          _adapt_mode(Geometry::AdaptMode::chart),
          _have_read_mesh(false),
          _have_partition(false),
          _have_hierarchy(false),
          _base_mesh_level(0),
          _allow_parti_manual(true),
          _allow_parti_parmetis(true),
          _allow_parti_fallback(true),
          _min_elems_per_rank(4),
          _base_mesh_node(nullptr),
          _patch_mesh_node(nullptr)
        {
        }

        /// virtual destructor
        virtual ~PartiDomainControl()
        {
          if(_patch_mesh_node != nullptr)
            delete _patch_mesh_node;
          if(_base_mesh_node != nullptr)
            delete _base_mesh_node;
        }

        /**
         * \brief Adds the supported arguments to an argument parser.
         *
         * This function adds all supported options, which can be parsed by calling
         * the #parse_args() function, to a SimpleArgParser object.
         *
         * \param[in] args
         * The argument parser.
         */
        static void add_supported_args(SimpleArgParser& args)
        {
          args.support("parti-type");
          args.support("parti-name");
          args.support("parti-rank-elems");
        }

        /**
         * \brief Parses the partitioner options from an argument parser.
         *
         * \param[in] args
         * The parser that is to be used.
         *
         * \returns
         * \c true, if the parsing was successful, or \c false,
         * if at least one option was invalid.
         */
        bool parse_args(SimpleArgParser& args)
        {
          // try to parse --parti-type
          {
            auto it = args.query("parti-type");
            if(it != nullptr)
            {
              _allow_parti_manual = _allow_parti_parmetis = _allow_parti_fallback = false;
              for(const auto& t : it->second)
              {
                if(t == "manual")
                  _allow_parti_manual = true;
                else if(t == "parmetis")
                  _allow_parti_parmetis = true;
                else if(t == "fallback")
                  _allow_parti_fallback = true;
                else
                {
                  if(Util::Comm::rank() == Index(0))
                  {
                    std::cerr << "ERROR: unknown partitioner type '" << t << "'" << std::endl;
                    return false;
                  }
                }
              }
            }
          }

          // parse --parti-name
          args.parse("parti-name", _manual_parti_name);

          // parse --parti-rank-elems
          args.parse("parti-rank-elems", _min_elems_per_rank);

          // okay
          return true;
        }

        /**
         * \brief Sets the adapt-mode for refinement
         *
         * \param[in] adapt_mode
         * The adapt-mode that is to be used.
         */
        void set_adapt_mode(Geometry::AdaptMode adapt_mode)
        {
          _adapt_mode = adapt_mode;
        }

        /**
         * \brief Sets the minimum number of elements per rank for automatic partitioning
         *
         * \param[in] min_elems_per_rank
         * The minimum number of elements per rank.
         */
        void set_min_rank_elems(Index min_elems_per_rank)
        {
          XASSERTM(min_elems_per_rank > Index(0), "element count must be > 0");
          _min_elems_per_rank = min_elems_per_rank;
        }

        /**
         * \brief Sets the name of the desired manual partition
         *
         * \param[in] name
         * The desired name of the manual partition.
         *
         * \note
         * If \p name is an empty string, then all manual partitions are considered
         * as candidates independent of their names.
         */
        void set_manual_parti_name(const String& name)
        {
          _manual_parti_name = name;
        }

        /**
         * \brief Sets whether the manual partitioning strategy is allowed.
         *
         * \param[in] allow
         * Specifies whether manual partitioning is allowed.
         */
        void set_allow_parti_manual(bool allow)
        {
          _allow_parti_manual = allow;
        }

        /**
         * \brief Sets whether the ParMETIS partitioning strategy is allowed.
         *
         * \param[in] allow
         * Specifies whether ParMETIS partitioning is allowed.
         */
        void set_allow_parti_parmetis(bool allow)
        {
          _allow_parti_parmetis = allow;
        }

        /**
         * \brief Sets whether the fallback partitioning strategy is allowed.
         *
         * \param[in] allow
         * Specifies whether fallback partitioning is allowed.
         */
        void set_allow_parti_fallback(bool allow)
        {
          _allow_parti_fallback = allow;
        }

        /**
         * \brief Reads the base-mesh from a mesh reader object
         *
         * \param[in] mesh_reader
         * The reader object to read the base-mesh from.
         */
        void read_mesh(Geometry::MeshFileReader& mesh_reader)
        {
          // make sure we did not already read a mesh
          XASSERTM(!_have_read_mesh, "domain control has already read a mesh");

          // make sure we don't have a base-mesh node
          XASSERTM(_base_mesh_node == nullptr, "domain control already has a base mesh");

          // create a new atlas unless we already have one
          if(this->_atlas == nullptr)
            this->_atlas = new AtlasType();

          // create a base-mesh node
          _base_mesh_node = new MeshNodeType(nullptr, this->_atlas);

          // try to parse the atlas, the base-mesh and the partition set
          mesh_reader.parse(*_base_mesh_node, *this->_atlas, &_part_set);

          // remember that we have read a mesh
          _have_read_mesh = true;
        }

        /**
         * \brief Reads the base-mesh from an input stream
         *
         * This function uses the Geometry::MeshFileReader to parse the input stream.
         *
         * \param[in] is
         * The input stream from which to read the base-mesh.
         */
        void read_mesh(std::istream& is)
        {
          Geometry::MeshFileReader mesh_reader(is);
          read_mesh(mesh_reader);
        }

        /**
         * \brief Reads the base-mesh from a file
         *
         * This function uses the Geometry::MeshFileReader to parse the input file.
         *
         * \note
         * This function makes use of the DistFileIO functionality to parse
         * the input file efficiently in an MPI-based run.
         *
         * \param[in] filename
         * The name of the input file.
         */
        void read_mesh(const String& filename)
        {
          std::stringstream stream;
          DistFileIO::read_common(stream, filename);
          read_mesh(stream);
        }

        /**
         * \brief Reads the base-mesh from a file sequence
         *
         * This function uses the Geometry::MeshFileReader to parse the input files.
         *
         * \note
         * This function makes use of the DistFileIO functionality to parse
         * the input files efficiently in an MPI-based run.
         *
         * \param[in] filenames
         * A deque of the input filenames.
         */
        void read_mesh(const std::deque<String>& filenames)
        {
          const std::size_t num_files = filenames.size();

          // create a deque of streams
          std::deque<std::stringstream> streams(num_files);

          // create a mesh file reader
          Geometry::MeshFileReader mesh_reader;

          // read all files
          for(std::size_t i(0); i < num_files; ++i)
          {
            // read the stream
            DistFileIO::read_common(streams.at(i), filenames.at(i));

            // add to mesh reader
            mesh_reader.add_stream(streams.at(i));
          }

          // finally, read the mesh
          read_mesh(mesh_reader);
        }

        /**
         * \brief Tries to create a suitable partitioning of the base-mesh.
         *
         * This function make uses of several strategies to find a suitable
         * partitioning of the base-mesh matching the number of processes
         * in a MPI-run.
         *
         * \attention
         * This functions assumes that a valid base-mesh node has been already
         * read in from a (sequence of) input file(s) by using one of the
         * #read_mesh() functions.
         *
         * This function applies the following partitioning strategies in
         * the following order:
         *
         * 1. This function checks if there is exactly 1 process in the MPI
         *    world and, if so, uses the whole base-mesh as the one and only
         *    patch.
         *
         * 2. Unless deactivated by the user, this function tries to find
         *    a suitable manual partitioning in the set of partitionings that
         *    have been parsed from the input file.
         *
         * 3. Unless deactivated by the user, this function tries to call
         *    the ParMETIS partitioner to obtain a partitioning.\n
         *    This works only if FEAT has been configured to build and link
         *    against the ParMETIS third-party library, of course.
         *
         * 4. Unless deactivated by the user, this function tries to call
         *    the "fallback" partitioner to obtain a partitioning.
         *
         * If none of the above strategies yield a suitable partitioning,
         * this function throws an InternalError.
         *
         * \note
         * This function (more precisely: one of its sub-functions) may
         * automatically refine the base-mesh to higher level to ensure
         * that the base-mesh has a sufficient number of elements so that
         * a partitioning strategy can yield something useful.
         *
         * \note
         * In a non-MPI run, this function does nothing.
         */
        void create_partition()
        {
          // do we have just 1 process?
          if(_create_partition_single_patch())
          {
            _have_partition = true;
            return;
          }
#ifdef FEAT_HAVE_MPI
          // let's see if we have a matching manual partition
          if(_allow_parti_manual && _create_partition_auto_manual())
          {
            _have_partition = true;
            return;
          }
          // next, let's give ParMETIS a try
          if(_allow_parti_parmetis && _create_partition_parmetis())
          {
            _have_partition = true;
            return;
          }
          // finally, launch the fallback partitioner
          if(_allow_parti_fallback && _create_partition_fallback())
          {
            _have_partition = true;
            return;
          }
#endif // FEAT_HAVE_MPI
          // we should not arrive here
          throw InternalError(__func__, __FILE__, __LINE__, "Failed to create a partition");
        }

        /**
         * \brief Creates a mesh hierarchy
         *
         * \param[in] lvl_max
         * The desired maximum refinement level.
         *
         * \param[in] lvl_min
         * The desired minimum refinement level.
         *
         * \attention
         * This function assumes that the base-mesh has already been read in
         * and (in a MPI-run) that a patch has been created by calling the
         * #create_partition() function.
         *
         * \attention
         * Note that both \p lvl_max and \p lvl_min are merely \b hints,
         * i.e. the resulting hierarchy \b may represent a different level
         * interval. This can happen if the partitioner already had to refine
         * the input base-mesh to obtain a suitable partitioning.
         */
        void create_hierarchy(int lvl_max, int lvl_min)
        {
          XASSERTM(!_have_hierarchy, "domain control already has a hierarchy");

#ifdef FEAT_HAVE_MPI
          XASSERTM(_have_partition, "domain needs to be partitioned before creating a hierarchy");
#endif

          // adjust lvl_max and lvl_min
          lvl_max = Math::max(lvl_max, 0);
          if(lvl_min < 0)
            lvl_min = Math::max(lvl_max + lvl_min + 1, 0);
          else
            lvl_min = Math::min(lvl_min, lvl_max);

          MeshNodeType* mesh_node = nullptr;

#ifdef FEAT_HAVE_MPI
          // create hierarchy from our patch mesh
          XASSERTM(_patch_mesh_node != nullptr, "domain control does not have a patch mesh");
          mesh_node = _patch_mesh_node;
          _patch_mesh_node = nullptr;
#else
          // create hierarchy from our base mesh
          XASSERTM(_base_mesh_node != nullptr, "domain control does not have a base mesh");
          mesh_node = _base_mesh_node;
          _base_mesh_node = nullptr;
#endif // FEAT_HAVE_MPI

          // refine up to desired minimum level
          int lvl(_base_mesh_level);
          for(; lvl < lvl_min; ++lvl)
          {
            MeshNodeType* coarse_node = mesh_node;
            mesh_node = coarse_node->refine(_adapt_mode);
            delete coarse_node;
          }

          // add coarse mesh node
          this->_levels.push_back(new LevelType(lvl, mesh_node));

          // refine up to desired maximum level
          for(; lvl < lvl_max; ++lvl)
          {
            MeshNodeType* coarse_node = mesh_node;
            mesh_node = coarse_node->refine(_adapt_mode);
            this->_levels.push_back(new LevelType(lvl+1, mesh_node));
          }

          // okay, we're done here
          _have_hierarchy = true;
        }

      protected:
        /**
         * \brief Auxiliary function: refines the base-mesh up to a specified level.
         *
         * \param[in] level
         * The level unto which to refine to.
         */
        void _refine_base_mesh_to_level(int level)
        {
          XASSERT(_base_mesh_node != nullptr);

          for(; _base_mesh_level < level; ++_base_mesh_level)
          {
            MeshNodeType* coarse_node = _base_mesh_node;
            _base_mesh_node = coarse_node->refine(_adapt_mode);
            delete coarse_node;
          }
        }

        /**
         * \brief Auxiliary function: refines the base-mesh until it has a minimum number of elements
         *
         * \param[in] min_elements
         * The minimum number of elements that the refined base-mesh needs to have.
         */
        void _refine_base_mesh_to_min_elems(Index min_elements)
        {
          XASSERT(_base_mesh_node != nullptr);

          Index num_elements = _base_mesh_node->get_mesh()->get_num_elements();
          for(; num_elements < min_elements; ++_base_mesh_level)
          {
            MeshNodeType* coarse_node = _base_mesh_node;
            _base_mesh_node = coarse_node->refine(_adapt_mode);
            num_elements = _base_mesh_node->get_mesh()->get_num_elements();
            delete coarse_node;
          }
        }

        /**
         * \brief Creates a patch from the base-mesh
         *
         * This function "converts" the base-mesh node into a patch-mesh node.
         * This function can be used if the MPI-world consists of only 1 process.
         *
         * \returns
         * \c true, if the MPI-world contains only 1 process and the base-mesh node
         * has been converted to a patch-mesh node, otherwise \c false.
         */
        bool _create_partition_single_patch()
        {
          XASSERT(_base_mesh_node != nullptr);

#ifdef FEAT_HAVE_MPI
          if(Util::Comm::size() != Index(1))
            return false;

          std::cout << "Using base-mesh as patch, because there is only one process..." << std::endl;

          // copy our base mesh
          _patch_mesh_node = _base_mesh_node;
          _base_mesh_node = nullptr;
#endif // FEAT_HAVE_MPI

          // comm ranks and tags
          std::vector<Index> ranks, ctags;

          // push layer
          this->_layers.push_back(new LayerType(std::move(ranks), std::move(ctags)));

          // okay
          return true;
        }

        /**
         * \brief Creates a patch from a manual partitioning.
         *
         * This function tries to find a manual partitioning, i.e. a partitioning that has
         * been parsed from the input mesh files, that matches the number of processes in
         * the MPI-world.
         *
         * \note
         * If necessary, this function will automatically refine the base-mesh.
         *
         * \returns
         * \c true, if a suitable manual partitioning was found and the patch-mesh for the
         * current rank has been created successfully, otherwise \c false.
         */
        bool _create_partition_auto_manual()
        {
          // get rank and nprocs
          Index rank = Util::Comm::rank();
          Index nprocs = Util::Comm::size();

          //if(rank == Index(0))
          //  std::cout << "Searching for suitable manual partition..." << std::endl;

          // check whether we have a suitable partition
          const Geometry::Partition* part = _part_set.find_partition(nprocs, _manual_parti_name);
          if(part == nullptr)
          {
            // no suitable partition found
            if(rank == Index(0))
            {
              if(_manual_parti_name.empty())
                std::cout << "No suitable manual partition available" << std::endl;
              else
                std::cout << "No suitable manual partition with name '" << _manual_parti_name << "' available" << std::endl;
            }
            return false;
          }

          if(rank == Index(0))
            std::cout << "Found manual partition '" << part->get_name() << "' for base-mesh level " << part->get_level() << std::endl;

          // refine up to required base-mesh level
          this->_refine_base_mesh_to_level(part->get_level());

          // comm ranks and tags
          std::vector<Index> ranks, ctags;

          // extract our patch
          _patch_mesh_node = _base_mesh_node->extract_patch(ranks, ctags, *part, rank);

          // >>>>> DEBUG >>>>>
          /*
          std::cout << "RANKS[" << rank << "]:";
          for(auto& i : ranks)
            std::cout << ' ' << i;
          std::cout << std::endl;
          std::cout << "CTAGS[" << rank << "]:";
          for(auto& i : ctags)
            std::cout << ' ' << i;
          std::cout << std::endl;
          */
          // <<<<< DEBUG <<<<<

          // push layer
          this->_layers.push_back(new LayerType(std::move(ranks), std::move(ctags)));

          // okay
          return true;
        }

        /**
         * \brief Creates a patch by using the ParMETIS partitioner.
         *
         * This function uses the ParMETIS partitioner to obtain a partitioning
         * and creates a patch-mesh node for the current process.
         *
         * \note
         * If necessary, this function will automatically refine the base-mesh.
         *
         * \note
         * This function always returns \c false if FEAT has not been configured
         * to link against the ParMETIS third-party library.
         *
         * \returns
         * \c true, if ParMETIS created a valid partitioning of the (possibly
         * refined) base-mesh and the patch-mesh for the current process has
         * been created successfully, otherwise \c false.
         */
        bool _create_partition_parmetis()
        {
#ifdef FEAT_HAVE_PARMETIS
          return _create_partition_foundation<Foundation::PExecutorParmetis<Foundation::ParmetisModePartKway>>("ParMETIS");
#else
          return false;
#endif
        }

        /**
         * \brief Creates a patch by using the "fallback" partitioner.
         *
         * This function uses the "fallback" partitioner to obtain a partitioning
         * and creates a patch-mesh node for the current process.
         *
         * \note
         * If necessary, this function will automatically refine the base-mesh.
         *
         * \returns
         * \c true, if a valid partitioning of the (possibly refined) base-mesh and the
         * patch-mesh for the current process have been created successfully, otherwise \c false.
         */
        bool _create_partition_fallback()
        {
#ifdef FEAT_HAVE_MPI
          return _create_partition_foundation<Foundation::PExecutorFallback<double, Index>>("Fallback");
#else
          return false;
#endif
        }

#ifdef FEAT_HAVE_MPI
        /**
         * \brief Creates a patch by using a foundation partitioner.
         *
         * This function uses one of the foundation partitioners to obtain
         * a partitioning and creates a patch-mesh node for the current process.
         *
         * \note
         * If necessary, this function will automatically refine the base-mesh.
         *
         * \tparam PExecutorT_
         * The foundation partitioner class to be used.
         *
         * \param[in] parti_name
         * The name of the partitioner. This is only used for console output.
         *
         * \returns
         * \c true, if a valid partitioning of the (possibly refined) base-mesh and the
         * patch-mesh for the current process have been created successfully, otherwise \c false.
         */
        template<typename PExecutorT_>
        bool _create_partition_foundation(const String& parti_name)
        {
          typedef PExecutorT_ PartT;

          const Index rank = Util::Comm::rank();
          const Index nprocs = Util::Comm::size();

          // refine base-mesh if necessary
          this->_refine_base_mesh_to_min_elems(_min_elems_per_rank * nprocs);

          if(rank == Index(0))
            std::cout << "Running " << parti_name << " partitioner on level " << _base_mesh_level << "..." << std::endl;

          // get our base-mesh
          const MeshType_& base_root_mesh = *(this->_base_mesh_node->get_mesh());

          // get number of elements
          const Index num_global_elements(base_root_mesh.get_num_entities(MeshType_::shape_dim));

          // allocate graph
          typename PartT::PGraphT global_dual(base_root_mesh, num_global_elements, Util::Communicator(MPI_COMM_WORLD));

          // local input for k-way partitioning
          auto local_dual(global_dual.create_local());

          auto part(PartT::part(*((typename PartT::PGraphT*)local_dual.get())));

          auto synched_part(Foundation::PSynch<PartT>::exec(part, typename PartT::IndexType(num_global_elements)));

          PartT::fill_comm_structs_global(synched_part, global_dual);

          // get ranks-at-element graph
          Adjacency::Graph ranks_at_elem(synched_part.rank_at_element());

          /// \todo ensure that each rank has at least one element.

          // get comm ranks and tags
          std::vector<Index> ranks(synched_part.get_comm_ranks());
          std::vector<Index> ctags(synched_part.get_comm_tags());

          // extract our patch
          _patch_mesh_node = this->_base_mesh_node->extract_patch(rank, ranks_at_elem, ranks);

          // push layer
          this->_layers.push_back(new LayerType(std::move(ranks), std::move(ctags)));

          // okay
          return true;
        }
#endif // FEAT_HAVE_MPI
      }; // class PartiDomainControl<...>
    } // namespace Domain
  } // namespace Control
} // namespace FEAT

#endif // CONTROL_DOMAIN_PARTI_DOMAIN_CONTROL_HPP
