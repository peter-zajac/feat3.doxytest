// FEAT3: Finite Element Analysis Toolbox, Version 3
// Copyright (C) 2010 - 2020 by Stefan Turek & the FEAT group
// FEAT3 is released under the GNU General Public License version 3,
// see the file 'copyright.txt' in the top level directory for details.

#pragma once
#ifndef KERNEL_GEOMETRY_MESH_NODE_HPP
#define KERNEL_GEOMETRY_MESH_NODE_HPP 1

// includes, FEAT
#include <kernel/geometry/conformal_mesh.hpp>
#include <kernel/geometry/mesh_atlas.hpp>
#include <kernel/geometry/mesh_part.hpp>
#include <kernel/geometry/macro_factory.hpp>
#include <kernel/geometry/partition_set.hpp>
#include <kernel/geometry/patch_halo_factory.hpp>
#include <kernel/geometry/patch_halo_splitter.hpp>
#include <kernel/geometry/patch_mesh_factory.hpp>
#include <kernel/geometry/patch_meshpart_factory.hpp>
#include <kernel/geometry/patch_meshpart_splitter.hpp>
#include <kernel/geometry/intern/dual_adaptor.hpp>
#include <kernel/adjacency/graph.hpp>

// includes, STL
#include <set>
#include <map>
#include <deque>
#include <vector>

namespace FEAT
{
  namespace Geometry
  {
    // forward declarations
    /// \cond internal
    namespace Intern
    {
      template<typename MeshType_>
      struct TypeConverter;

      template<int dim_>
      struct AdjacenciesFiller;
    }

    template<typename Policy_>
    class MeshPartNode DOXY({});
    /// \endcond

    /**
     * \brief Adapt mode enumeration
     */
    enum class AdaptMode
    {
      none  = 0x0,
      chart = 0x1,
      dual  = 0x2
    };

    /// \cond internal
    AdaptMode inline operator|(AdaptMode x, AdaptMode y)
    {
      return static_cast<AdaptMode>(int(x) | int(y));
    }

    AdaptMode inline operator&(AdaptMode x, AdaptMode y)
    {
      return static_cast<AdaptMode>(int(x) & int(y));
    }

    /**
     * \brief Streaming operator for AdaptModes
     */
    inline std::ostream& operator<<(std::ostream& os, AdaptMode mode)
    {
      switch(mode)
      {
        case AdaptMode::none:
          return os << "none";
        case AdaptMode::chart:
          return os << "chart";
        case AdaptMode::dual:
          return os << "dual";
        default:
          return os << "-unknown-";
      }
    }

    inline void operator<<(AdaptMode& mode, const String& mode_name)
    {
      if(mode_name == "none")
        mode = AdaptMode::none;
      else if(mode_name == "chart")
        mode = AdaptMode::chart;
      else if(mode_name == "dual")
        mode = AdaptMode::dual;
      else
        XABORTM("Unknown AdaptMode identifier string " + mode_name);
    }
    /// \endcond

    /**
     * \brief Mesh Node base class
     *
     * A MeshNode is a container for bundling a mesh with MeshParts referring to it.
     *
     * \tparam Policy_
     * Bundle of type names for meshes and partial meshes.
     *
     * \tparam MeshNodePolicy_
     * Bundle of type names for other MeshNodes containing meshes and MeshParts.
     *
     * \author Peter Zajac
     */
    template<
      typename RootMesh_,
      typename ThisMesh_>
    class MeshNode
    {
    public:
      /// the root mesh type, has to be a Mesh
      typedef RootMesh_ RootMeshType;
      /// mesh type of this node, can be a Mesh or MeshPart
      typedef ThisMesh_ MeshType;
      /// the mesh part type
      typedef MeshPart<RootMeshType> MeshPartType;
      /// the mesh part node type
      typedef MeshPartNode<RootMeshType> MeshPartNodeType;
      /// the mesh atlas type
      typedef MeshAtlas<RootMeshType> MeshAtlasType;
      /// the mesh chart type
      typedef Atlas::ChartBase<RootMeshType> MeshChartType;

    protected:
      /**
       * \brief Container class for bundling MeshPartNodes with their corresponding charts
       */
      class MeshPartNodeBin
      {
      public:
        /// This container's MeshPartNode
        MeshPartNodeType* node;
        /// Name of the chart
        String chart_name;
        /// Chart belonging to node
        const MeshChartType* chart;

      public:
        /**
         * \brief Constructor
         *
         * \param node_
         * MeshPartNode for this container.
         *
         * \param chart_
         * Chart for this container.
         *
         */
        explicit MeshPartNodeBin(MeshPartNodeType* node_, String chart_name_, const MeshChartType* chart_) :
          node(node_),
          chart_name(chart_name_),
          chart(chart_)
        {
        }
      }; // class MeshPartNodeBin

      /// submesh node bin container type
      typedef std::map<String, MeshPartNodeBin> MeshPartNodeContainer;
      /// submesh node iterator type
      typedef typename MeshPartNodeContainer::iterator MeshPartNodeIterator;
      /// submesh node const-iterator type
      typedef typename MeshPartNodeContainer::const_iterator MeshPartNodeConstIterator;
      /// submesh node reverse-iterator type
      typedef typename MeshPartNodeContainer::reverse_iterator MeshPartNodeReverseIterator;

    protected:
      /// a pointer to the mesh of this node
      MeshType* _mesh;
      /// child submesh nodes
      MeshPartNodeContainer _mesh_part_nodes;

    protected:
      /**
       * \brief Constructor.
       *
       * \param[in] mesh
       * A pointer to the mesh for this node.
       */
      explicit MeshNode(MeshType* mesh) :
        _mesh(mesh),
        _mesh_part_nodes()
      {
      }

    public:
      /**
       * \brief Virtual destructor
       *
       * \note _mesh gets deleted because passing it to the MeshNode's constructor passes ownership to it.
       */
      virtual ~MeshNode()
      {
        // loop over all submesh nodes in reverse order and delete them
        MeshPartNodeReverseIterator it(_mesh_part_nodes.rbegin());
        MeshPartNodeReverseIterator jt(_mesh_part_nodes.rend());
        for(; it != jt; ++it)
        {
          if(it->second.node != nullptr)
          {
            delete it->second.node;
          }
        }

        // delete mesh
        if(_mesh != nullptr)
        {
          delete _mesh;
        }
      }

      /// \returns The size of dynamically allocated memory in bytes.
      std::size_t bytes() const
      {
        std::size_t s(0);
        if(_mesh != nullptr)
          s += _mesh->bytes();
        for(auto it = _mesh_part_nodes.begin(); it != _mesh_part_nodes.end(); ++it)
          s += it->second.node->bytes();
        return s;
      }

      /**
       * \brief Returns the mesh of this node.
       * \returns
       * A pointer to the mesh contained in this node.
       */
      MeshType* get_mesh()
      {
        return _mesh;
      }

      /** \copydoc get_mesh() */
      const MeshType* get_mesh() const
      {
        return _mesh;
      }

      void set_mesh(MeshType* mesh)
      {
        XASSERTM(_mesh == nullptr, "Mesh node already has a mesh");
        _mesh = mesh;
      }

      /**
       * \brief Returns the names of all mesh parts of this node.
       *
       * \param[in] no_internals
       * Specifies whether to skip all internal mesh-parts whose names begin with an underscore.
       *
       * \returns
       * A deque of all mesh-part names in this node.
       */
      std::deque<String> get_mesh_part_names(bool no_internals = false) const
      {
        std::deque<String> names;
        for(auto it = _mesh_part_nodes.begin(); it != _mesh_part_nodes.end(); ++it)
        {
          String mpname = (*it).first;
          if(!(no_internals && (mpname.front() == '_')))
            names.push_back(mpname);
        }
        return names;
      }

      /**
       * \brief Adds a new mesh-part child node.
       *
       * \param[in] part_name
       * The name of the child node.
       *
       * \param[in] mesh_part_node
       * A pointer to the mesh_part node to be added.
       *
       * \param[in] chart
       * A pointer to the chart that the subnode is to be associated with. May be \c nullptr.
       *
       * \returns
       * \p mesh_part_node if the insertion was successful, otherwise \c nullptr.
       *
       */
      MeshPartNodeType* add_mesh_part_node(
        const String& part_name,
        MeshPartNodeType* mesh_part_node,
        const String& chart_name = "",
        const MeshChartType* chart = nullptr)
      {
        if(mesh_part_node != nullptr)
        {
          if(_mesh_part_nodes.emplace(part_name, MeshPartNodeBin(mesh_part_node, chart_name, chart)).second)
          {
            return mesh_part_node;
          }
        }
        return nullptr;
      }

      /**
       * \brief Adds a new mesh-part child node.
       *
       * \param[in] part_name
       * The name of the child node.
       *
       * \param[in] mesh_part
       * A pointer to the mesh_part to be added.
       *
       * \param[in] chart
       * A pointer to the chart that the subnode is to be associated with. May be \c nullptr.
       *
       * \returns
       * A pointer to the newly created mesh-part node if the insertion was successful, otherwise \c nullptr.
       */
      MeshPartNodeType* add_mesh_part(
        const String& part_name,
        MeshPartType* mesh_part,
        const String& chart_name = "",
        const MeshChartType* chart = nullptr)
      {
        MeshPartNodeType* part_node = new MeshPartNodeType(mesh_part);
        if(add_mesh_part_node(part_name, part_node, chart_name, chart) == nullptr)
        {
          delete part_node;
          return nullptr;
        }

        return part_node;
      }

      /**
       * \brief Sets the chart for a particular mesh part.
       *
       * \param[in] part_name
       * The name of the mesh part that the chart is to be assigned to.
       *
       * \param[in] chart
       * The chart that is to be assigned. May also be \c nullptr.
       *
       * \returns
       * \c true if the chart was assigned successfully or \c false if no mesh part
       * with the specified name was found.
       */
      bool set_mesh_part_chart(const String& part_name, const String& chart_name, const MeshChartType* chart)
      {
        MeshPartNodeIterator it(_mesh_part_nodes.find(part_name));
        if(it == _mesh_part_nodes.end())
          return false;
        it->second.chart_name = chart_name;
        it->second.chart = chart;
        return true;
      }

      /**
       * \brief Searches this container for a MeshPartNode
       *
       * \param[in] part_name
       * The name of the node to be found.
       *
       * \returns
       * A pointer to the mesh_part node associated with \p part_name or \c nullptr if no such node was found.
       */
      MeshPartNodeType* find_mesh_part_node(const String& part_name)
      {
        MeshPartNodeIterator it(_mesh_part_nodes.find(part_name));
        return (it != _mesh_part_nodes.end()) ? it->second.node : nullptr;
      }

      /** \copydoc find_mesh_part_node() */
      const MeshPartNodeType* find_mesh_part_node(const String& part_name) const
      {
        MeshPartNodeConstIterator it(_mesh_part_nodes.find(part_name));
        return (it != _mesh_part_nodes.end()) ? it->second.node : nullptr;
      }

      /**
       * \brief Searches this container for a MeshPart
       *
       * \param[in] part_name
       * The name of the node associated with the mesh_part to be found.
       *
       * \returns
       * A pointer to the mesh_part associated with \p part_name or \c nullptr if no such node was found.
       */
      MeshPartType* find_mesh_part(const String& part_name)
      {
        MeshPartNodeType* node = find_mesh_part_node(part_name);
        return (node != nullptr) ? node->get_mesh() : nullptr;
      }

      /** \copydoc find_mesh_part() */
      const MeshPartType* find_mesh_part(const String& part_name) const
      {
        const MeshPartNodeType* node = find_mesh_part_node(part_name);
        return (node != nullptr) ? node->get_mesh() : nullptr;
      }

      /**
       * \brief Searches for a chart belonging to a MeshPart by name
       *
       * \param[in] part_name
       * The name of the node associated with the chart to be found.
       *
       * \returns
       * A pointer to the chart associated with \p part_name of \c nullptr if no such node was found or if
       * the corresponding node did not have a chart.
       */
      const MeshChartType* find_mesh_part_chart(const String& part_name) const
      {
        MeshPartNodeConstIterator it(_mesh_part_nodes.find(part_name));
        return (it != _mesh_part_nodes.end()) ? it->second.chart : nullptr;
      }

      /**
       * \brief Searches for a chart name belonging to a MeshPart by name
       *
       * \param[in] part_name
       * The name of the node associated with the chart to be found.
       *
       * \returns
       * The name of the chart associated with \p part_name or an empty string is no
       * such node was found.
       */
      String find_mesh_part_chart_name(const String& part_name) const
      {
        MeshPartNodeConstIterator it(_mesh_part_nodes.find(part_name));
        return (it != _mesh_part_nodes.end()) ? it->second.chart_name : String();
      }

      /**
       * \brief Renames a set of mesh-parts.
       *
       * \param[in] renames
       * A map of oldname-newname pairs.
       */
      void rename_mesh_parts(const std::map<String,String>& renames)
      {
        if(renames.empty())
          return;

        MeshPartNodeContainer new_map;
        for(auto& v : _mesh_part_nodes)
        {
          auto it = renames.find(v.first);
          if(it == renames.end())
          {
            // no rename, use old name
            new_map.emplace(v.first, v.second);
          }
          else
          {
            // insert with new name
            new_map.emplace(it->second, v.second);
          }
        }
        _mesh_part_nodes = std::move(new_map);
      }

      /**
       * \brief Adapts this mesh node.
       *
       * This function loops over all MeshPart nodes and uses their associated charts (if given)
       * to adapt the mesh in this node.
       *
       * \param[in] recursive
       * If set to \c true, all mesh_part nodes are adapted prior to adapting this node.
       */
      void adapt(bool recursive = true)
      {
        // loop over all mesh_part nodes
        MeshPartNodeIterator it(_mesh_part_nodes.begin());
        MeshPartNodeIterator jt(_mesh_part_nodes.end());
        for(; it != jt; ++it)
        {
          // adapt child node
          if(recursive)
          {
            it->second.node->adapt(true);
          }

          // adapt this node if a chart is given
          if(it->second.chart != nullptr)
          {
            // we need to check whether the mesh part really exists
            // because it may be nullptr in a parallel run, which
            // indicates that the current patch is not adjacent to
            // the boundary represented by the mesh part.
            const MeshPartType* mesh_part = it->second.node->get_mesh();
            if(mesh_part != nullptr)
              it->second.chart->adapt(*_mesh, *mesh_part);
          }
        }
      }

      /**
       * \brief Adapts this mesh node.
       *
       * This function adapts this node by a specific chart whose name is given.
       *
       * \param[in] part_name
       * The name of the mesh_part node that is to be used for adaption.
       *
       * \param[in] recursive
       * If set to \c true, the mesh_part node associated with \p part_name will be adapted prior to adapting this node.
       *
       * \returns
       * \c true if this node was adapted successfully or \c false if no node is associated with \p part_name or if
       * the node did not contain any chart.
       */
      bool adapt_by_name(const String& part_name, bool recursive = false)
      {
        // Try to find the corresponding mesh_part node
        MeshPartNodeIterator it(_mesh_part_nodes.find(part_name));
        // Do not proceed if the mesh_part node does not exist or exists but is empty on the current patch
        if(it == _mesh_part_nodes.end() || it->second.node->get_mesh() == nullptr)
          return false;

        // adapt child node
        if(recursive)
        {
          it->second.node->adapt(true);
        }

        // Adapt this node if we have a chart
        if(it->second.chart != nullptr)
        {
          it->second.chart->adapt(*_mesh, *(it->second.node->get_mesh()));
          return true;
        }

        // no chart associated
        return false;
      }

      /**
       * \brief Returns the name of the class.
       * \returns
       * The name of the class as a String.
       */
      static String name()
      {
        return "MeshNode<...>";
      }

    protected:
      /**
       * \brief Refines all child nodes of this node.
       *
       * \param[in,out] refined_node
       * A reference to the node generated by refining this node.
       */
      void refine_children(MeshNode& refined_node) const
      {
        // refine mesh parts
        refine_mesh_parts(refined_node);
      }

      /**
       * \brief Refines all child MeshPart nodes of this node.
       *
       * \param[in,out] refined_node
       * A reference to the node generated by refining this node.
       */
      void refine_mesh_parts(MeshNode& refined_node) const
      {
        MeshPartNodeConstIterator it(_mesh_part_nodes.begin());
        MeshPartNodeConstIterator jt(_mesh_part_nodes.end());

        for(; it != jt; ++it)
        {
          refined_node.add_mesh_part_node(it->first, it->second.node->refine(*_mesh), it->second.chart_name, it->second.chart);
        }
      }
    }; // class MeshNode

    /* ***************************************************************************************** */

    /**
     * \brief MeshPart mesh tree node class template
     *
     * This class template implements a mesh tree node containing a MeshPart.
     *
     * \author Peter Zajac
     */
    template<typename RootMesh_>
    class MeshPartNode
      : public MeshNode<RootMesh_, MeshPart<RootMesh_>>
    {
    public:
      /// base class typedef
      typedef MeshNode<RootMesh_, MeshPart<RootMesh_>> BaseClass;

      /// this mesh type
      typedef typename BaseClass::MeshType MeshType;
      /// mesh part type
      typedef typename BaseClass::MeshPartType MeshPartType;
      /// mesh atlas type
      typedef typename BaseClass::MeshAtlasType MeshAtlasType;
      /// mesh chart type
      typedef typename BaseClass::MeshChartType MeshChartType;

    public:
      /**
       * \brief Constructor.
       *
       * \param[in] mesh_part
       * A pointer to the MeshPart for this node.
       */
      explicit MeshPartNode(MeshPartType* mesh_part) :
        BaseClass(mesh_part)
      {
      }

      /// virtual destructor
      virtual ~MeshPartNode()
      {
      }

      /**
       * \brief Refines this node and its sub-tree.
       *
       * \param[in] parent
       * A reference to the parent mesh/cell subset of this node's cell subset.
       *
       * \returns
       * A pointer to a MeshPartNode containing the refined cell subset tree.
       */
      template<typename ParentType_>
      MeshPartNode* refine(const ParentType_& parent) const
      {
        // the mesh part may be a nullptr; in this case also return a node containing a nullptr
        if(this->_mesh == nullptr)
        {
          return new MeshPartNode(nullptr);
        }

        // create a refinery
        StandardRefinery<MeshPartType> refinery(*this->_mesh, parent);

        // create a new MeshPartNode
        MeshPartNode* fine_node = new MeshPartNode(new MeshPartType(refinery));

        // refine our children
        refine_mesh_parts(*fine_node);

        // okay
        return fine_node;
      }

      /**
       * \brief Returns the name of the class.
       * \returns
       * The name of the class as a String.
       */
      static String name()
      {
        return "MeshPartNode<...>";
      }

    protected:
      /**
       * \brief Refines this node's child nodes.
       *
       * \note This function is called by this node's refine() function, therefore there is no need
       * for the user to call this function.
       *
       * \param[in,out] refined_node
       * A reference to the node generated from refinement of this node.
       */
      void refine_mesh_parts(MeshPartNode& refined_node) const
      {
        typename BaseClass::MeshPartNodeConstIterator it(this->_mesh_part_nodes.begin());
        typename BaseClass::MeshPartNodeConstIterator jt(this->_mesh_part_nodes.end());
        for(; it != jt; ++it)
        {
          refined_node.add_mesh_part_node(it->first, it->second.node->refine(*this->_mesh));
        }
      }

    }; // class MeshPartNode<...>

    /* ***************************************************************************************** */

    /**
     * \brief Root mesh node class template
     *
     * This class template is used for the root node of a mesh tree.
     *
     * \author Peter Zajac
     */
    template<typename RootMesh_>
    class RootMeshNode
      : public MeshNode<RootMesh_, RootMesh_>
    {
    public:
      /// base class typedef
      typedef MeshNode<RootMesh_, RootMesh_> BaseClass;

      /// this mesh type
      typedef typename BaseClass::MeshType MeshType;
      /// mesh part type
      typedef typename BaseClass::MeshPartType MeshPartType;
      /// mesh atlas type
      typedef typename BaseClass::MeshAtlasType MeshAtlasType;
      /// mesh chart type
      typedef typename BaseClass::MeshChartType MeshChartType;

    protected:
      /// our atlas
      MeshAtlasType* _atlas;
      /// a map of our halo mesh-parts
      std::map<int, MeshPartType*> _halos;
      /// a map of our patch mesh-parts
      std::map<int, MeshPartType*> _patches;

    public:
      /**
       * \brief Constructor.
       *
       * \param[in] mesh
       * A pointer to the mesh for this node.
       *
       * \param[in] atlas
       * A pointer to the atlas for this mesh tree.
       */
      explicit RootMeshNode(MeshType* mesh, MeshAtlasType* atlas = nullptr) :
        BaseClass(mesh),
        _atlas(atlas),
        _halos(),
        _patches()
      {
      }

      /**
       * \brief Virtual destructor
       *
       * \note _atlas does not get deleted because it is not owned by the RootMeshNode, as several MeshNodes may refer
       * to the same atlas.
       */
      virtual ~RootMeshNode()
      {
        for(auto& x : _patches)
          delete x.second;
        for(auto& x : _halos)
          delete x.second;
      }

      const MeshAtlasType* get_atlas() const
      {
        return _atlas;
      }

      /// \returns The size of dynamically allocated memory in bytes.
      std::size_t bytes() const
      {
        std::size_t s = BaseClass::bytes();
        for(const auto& x : _halos)
          s += x.second->bytes();
        for(const auto& x : _patches)
          s += x.second->bytes();
        return s;
      }

      void add_halo(int rank, MeshPartType* halo_part)
      {
        XASSERT(halo_part != nullptr);
        _halos.emplace(rank, halo_part);
      }

      const MeshPartType* get_halo(int rank) const
      {
        auto it = _halos.find(rank);
        return (it != _halos.end() ? it->second : nullptr);
      }

      const std::map<int, MeshPartType*>& get_halo_map() const
      {
        return this->_halos;
      }

      void clear_halos()
      {
        for(auto& x : _halos)
          delete x.second;
        _halos.clear();
      }

      /**
       * \brief Renames the halo meshparts.
       *
       * This function can be used to rename the halo meshparts when the rank ordering in the
       * communicator has changed.
       *
       * \param[in] ranks
       * An map of oldrank-newrank pairs.
       */
      void rename_halos(const std::map<int,int>& ranks)
      {
        if(ranks.empty())
          return;

        std::map<int, MeshPartType*> new_halos;

        for(auto& v : _halos)
        {
          auto it = ranks.find(v.first);
          if(it == ranks.end())
            new_halos.emplace(v.first, v.second);
          else
            new_halos.emplace(it->second, v.second);
        }

        _halos = std::move(new_halos);
      }

      void add_patch(int rank, MeshPartType* patch_part)
      {
        XASSERT(patch_part != nullptr);
        _patches.emplace(rank, patch_part);
      }

      const MeshPartType* get_patch(int rank) const
      {
        auto it = _patches.find(rank);
        return (it != _patches.end() ? it->second : nullptr);
      }

      const std::map<int, MeshPartType*>& get_patch_map() const
      {
        return this->_patches;
      }

      void clear_patches()
      {
        for(auto& x : _patches)
          delete x.second;
        _patches.clear();
      }

      /**
       * \brief Creates mesh-parts for all base cells of this root mesh.
       *
       * This function must be called for the unrefined base root mesh, if one
       * intends to use the Assembly::BaseSplitter class for the redistribution
       * of data in dynamic load balancing.
       */
      void create_base_splitting()
      {
        // make sure we have a mesh
        XASSERT(this->_mesh != nullptr);

        // get number of cells
        Index num_cells = this->_mesh->get_num_entities(MeshType::shape_dim);

        // loop over all cell indices
        for(Index cell(0); cell < num_cells; ++cell)
        {
          // create a macro factory
          MacroFactory<MeshPartType> factory(*this->_mesh, cell);
          // create a new mesh part and add to this mesh node
          this->add_mesh_part("_base:" + stringify(cell), new MeshPartType(factory));
        }
      }

      /**
       * \brief Refines this node and its sub-tree.
       *
       * \param[in] adapt_mode
       * Mode for adaption, defaults to chart.
       *
       * \returns
       * A pointer to a RootMeshNode containing the refined mesh tree.
       */
      RootMeshNode* refine(AdaptMode adapt_mode = AdaptMode::chart) const
      {
        // create a refinery
        StandardRefinery<MeshType> refinery(*this->_mesh);

        // create a new root mesh node
        RootMeshNode* fine_node = new RootMeshNode(new MeshType(refinery), this->_atlas);

        // refine our children
        this->refine_children(*fine_node);

        // refine our halos
        for(const auto& v : _halos)
        {
          StandardRefinery<MeshPartType> halo_refinery(*v.second, *this->_mesh);
          fine_node->add_halo(v.first, new MeshPartType(halo_refinery));
        }

        // refine our patch mesh-parts
        for(const auto& v : _patches)
        {
          StandardRefinery<MeshPartType> patch_refinery(*v.second, *this->_mesh);
          fine_node->add_patch(v.first, new MeshPartType(patch_refinery));
        }

        // adapt by chart?
        if((adapt_mode & AdaptMode::chart) != AdaptMode::none)
        {
          fine_node->adapt(true);
        }

        // adapt dual?
        if((adapt_mode & AdaptMode::dual) != AdaptMode::none)
        {
          Intern::DualAdaptor<MeshType>::adapt(*fine_node->get_mesh(), *this->get_mesh());
        }

        // okay
        return fine_node;
      }

      /**
       * \brief Extracts a patch from the root mesh as a new mesh node
       *
       * This function also computes the communication neighbour ranks.
       *
       * \param[out] comm_ranks
       * The communication neighbour ranks vector for this process.
       *
       * \param[in] elems_at_rank
       * The elements-at-rank graph representing the partitioning
       * from which the patch is to be extracted.
       *
       * \param[in] rank
       * The rank of the patch to be created.
       *
       * \returns
       * A new mesh node representing the extracted patch.
       */
      RootMeshNode* extract_patch(
        std::vector<int>& comm_ranks,
        const Adjacency::Graph& elems_at_rank,
        const int rank)
      {
        // get dimensions of graph
        const Index num_elems = elems_at_rank.get_num_nodes_image();

        // get our mesh
        const MeshType* base_root_mesh = this->get_mesh();
        XASSERTM(base_root_mesh != nullptr, "mesh node has no mesh");

        // validate element count
        XASSERTM(num_elems == base_root_mesh->get_num_elements(), "mesh vs partition: element count mismatch");

        // transpose the partitioning graph here, as we will need it multiple times
        const Adjacency::Graph ranks_at_elem(Adjacency::RenderType::transpose, elems_at_rank);

        // Step 1: compute neighbour ranks
        {
          // get vertices-at-element
          const auto& verts_at_elem = base_root_mesh->template get_index_set<MeshType::shape_dim, 0>();

          // Note:
          // In the following, we are building the ranks-at-rank adjacency graph in an apparently
          // cumbersome manner in five steps (including the ranks-at-elem graph assembly above).
          // Let "R>E" be the elements-at-rank graph, that is given as a parameter to this function,
          // and let "E>V" be the vertices-at-element graph, then we perform the following steps:
          //
          // 1: compute ranks-at-element by transposition:   E>R := (R>E)^T
          // 2: compute elements-at-vertex by transposition: V>E := (E>V)^T
          // 3: compute ranks-at-vertex by composition:      V>R := (V>E) * (E>R)
          // 4: compute vertices-at-rank by transposition:   R>V := (V>R)^T
          // 5: compute ranks-at-rank by composition:        R>R := (R>V) * (V>R)
          //
          // As said before, this looks as if it is more complicated than it has to be, because
          // one might also try the following three-step approach (which was, in fact, implemented
          // here before):
          //
          // 1. compute vertices-at-rank by composition:     R>V := (R>E) * (E>V)
          // 2. compute ranks-at-vertex by transposition:    V>R := (R>V)^T
          // 3: compute ranks-at-rank by composition:        R>R := (R>V) * (V>R)
          //
          // The million dollar question is: why don't use the 3-step approach?
          // Answer: Assume that we have N elements in the base-mesh and P processes, then it
          // can be shown that the first step "R>V := (R>E) * (E>V)" has a runtime of O(N^2/P),
          // which results in quadratic runtime in N unless N = O(P). So the practically important
          // scenario of "many elements on few processors" lead to this bottleneck. However, it is
          // worth mentioning that the 3-step approach has linear runtime in the case N = O(P).
          // On the other hand, the 5-step approach implemented here can be shown to have
          // linear runtime in both N and P in any relevant case, which is the reason why it
          // was chosen here.

          // build elements-at-vertex graph
          Adjacency::Graph elems_at_vert(Adjacency::RenderType::transpose, verts_at_elem);

          // build ranks-at-vertex graph and transpose it
          Adjacency::Graph ranks_at_vert(Adjacency::RenderType::injectify, elems_at_vert, ranks_at_elem);
          Adjacency::Graph verts_at_rank(Adjacency::RenderType::transpose, ranks_at_vert);

          // build ranks-at-rank (via vertices) graph
          Adjacency::Graph ranks_at_rank(Adjacency::RenderType::injectify, verts_at_rank, ranks_at_vert);

          // build comm neighbour ranks vector
          for(auto it = ranks_at_rank.image_begin(Index(rank)); it != ranks_at_rank.image_end(Index(rank)); ++it)
          {
            if(int(*it) != rank)
              comm_ranks.push_back(int(*it));
          }
        }

        // Step 2: create mesh part of the patch
        MeshPartType* patch_mesh_part = nullptr;
        {
          // create a factory for our partition
          PatchMeshPartFactory<MeshType> part_factory(Index(rank), elems_at_rank);

          // ensure that the partition is not empty
          if(part_factory.empty())
          {
            String msg("Rank ");
            msg += stringify(rank);
            msg += " received empty patch from partitioner!";
            XABORTM(msg);
          }

          // create patch mesh part
          patch_mesh_part = new MeshPartType(part_factory);
          patch_mesh_part->template deduct_target_sets_from_top<MeshType::shape_dim>(
            base_root_mesh->get_index_set_holder());

          // add this to out patch map
          this->add_patch(rank, patch_mesh_part);
        }

        // Step 3: Create root mesh of partition by using PatchMeshFactory
        MeshType* patch_root_mesh = nullptr;
        {
          // create patch root mesh
          PatchMeshFactory<MeshType> patch_factory(*base_root_mesh, *patch_mesh_part);
          patch_root_mesh = new MeshType(patch_factory);
        }

        // Step 4: Create root mesh node for mesh
        RootMeshNode* patch_node = new RootMeshNode(patch_root_mesh, this->_atlas);

        // Step 5: intersect boundary and other base mesh parts
        {
          // create mesh part splitter
          PatchMeshPartSplitter<MeshType> part_splitter(*base_root_mesh, *patch_mesh_part);

          // get all mesh part names
          std::deque<String> part_names = this->get_mesh_part_names();

          // loop over all base-mesh mesh parts
          for(auto it = part_names.begin(); it != part_names.end(); ++it)
          {
            // get base mesh part node
            auto* base_part_node = this->find_mesh_part_node(*it);
            XASSERTM(base_part_node != nullptr, String("base-mesh part '") + (*it) + "' not found!");

            // our split mesh part
            MeshPartType* split_part = nullptr;

            // get base-mesh part
            MeshPartType* base_part = base_part_node->get_mesh();

            // build
            if((base_part != nullptr) && part_splitter.build(*base_part))
            {
              // create our mesh part
              split_part = new MeshPartType(part_splitter);
            }

            // Insert patch mesh part
            patch_node->add_mesh_part(*it, split_part, this->find_mesh_part_chart_name(*it), this->find_mesh_part_chart(*it));
          }
        }

        // Step 6: Create halos
        {
          // create halo factory
          PatchHaloFactory<MeshType> halo_factory(ranks_at_elem, *base_root_mesh, *patch_mesh_part);

          // loop over all comm ranks
          for(auto it = comm_ranks.begin(); it != comm_ranks.end(); ++it)
          {
            // build halo
            halo_factory.build(Index(*it));

            // create halo mesh part and add to our map
            patch_node->add_halo(*it, new MeshPartType(halo_factory));
          }
        }

        // return patch mesh part
        return patch_node;
      }

      /**
       * \brief Extracts a patch from a manual partition
       *
       * This function also computes the communication ranks.
       *
       * \param[out] comm_ranks
       * The communication ranks vector for this rank.
       *
       * \param[in] partition
       * The partition from which the patch is to be extracted.
       *
       * \param[in] rank
       * The rank of the patch to be created.
       *
       * \returns
       * A new mesh node representing the extracted patch.
       */
      RootMeshNode* extract_patch(
        std::vector<int>& comm_ranks,
        const Partition& partition,
        const int rank)
      {
        return extract_patch(comm_ranks, partition.get_patches(), rank);
      }

      void create_patch_meshpart(const Adjacency::Graph& elems_at_rank, const int rank)
      {
        // create a factory for our partition
        PatchMeshPartFactory<MeshType> part_factory(Index(rank), elems_at_rank);

        // create patch mesh part
        MeshPartType* patch_mesh_part = new MeshPartType(part_factory);
        patch_mesh_part->template deduct_target_sets_from_top<MeshType::shape_dim>(
          this->get_mesh()->get_index_set_holder());

        // add patch meshpart to this node
        this->add_patch(rank, patch_mesh_part);
      }

      /**
       * \brief Returns the name of the class.
       * \returns
       * The name of the class as a String.
       */
      static String name()
      {
        return "RootMeshNode<...>";
      }
    }; // class RootMeshNode

#ifdef FEAT_EICKT
    extern template class MeshNode<ConformalMesh<Shape::Simplex<2>, 2, Real>, ConformalMesh<Shape::Simplex<2>, 2, Real>>;
    extern template class MeshNode<ConformalMesh<Shape::Simplex<3>, 3, Real>, ConformalMesh<Shape::Simplex<3>, 3, Real>>;
    extern template class MeshNode<ConformalMesh<Shape::Hypercube<2>, 2, Real>, ConformalMesh<Shape::Hypercube<2>, 2, Real>>;
    extern template class MeshNode<ConformalMesh<Shape::Hypercube<3>, 3, Real>, ConformalMesh<Shape::Hypercube<3>, 3, Real>>;

    extern template class MeshNode<ConformalMesh<Shape::Simplex<2>, 2, Real>, MeshPart<ConformalMesh<Shape::Simplex<2>, 2, Real>>>;
    extern template class MeshNode<ConformalMesh<Shape::Simplex<3>, 3, Real>, MeshPart<ConformalMesh<Shape::Simplex<3>, 3, Real>>>;
    extern template class MeshNode<ConformalMesh<Shape::Hypercube<2>, 2, Real>, MeshPart<ConformalMesh<Shape::Hypercube<2>, 2, Real>>>;
    extern template class MeshNode<ConformalMesh<Shape::Hypercube<3>, 3, Real>, MeshPart<ConformalMesh<Shape::Hypercube<3>, 3, Real>>>;

    extern template class RootMeshNode<ConformalMesh<Shape::Simplex<2>, 2, Real>>;
    extern template class RootMeshNode<ConformalMesh<Shape::Simplex<3>, 3, Real>>;
    extern template class RootMeshNode<ConformalMesh<Shape::Hypercube<2>, 2, Real>>;
    extern template class RootMeshNode<ConformalMesh<Shape::Hypercube<3>, 3, Real>>;
#endif // FEAT_EICKT
  } // namespace Geometry
} // namespace FEAT

#endif // KERNEL_GEOMETRY_MESH_NODE_HPP
