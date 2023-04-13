// FEAT3: Finite Element Analysis Toolbox, Version 3
// Copyright (C) 2010 - 2023 by Stefan Turek & the FEAT group
// FEAT3 is released under the GNU General Public License version 3,
// see the file 'copyright.txt' in the top level directory for details.

#pragma once
#ifndef CONTROL_DOMAIN_DOMAIN_LEVEL_HPP
#define CONTROL_DOMAIN_DOMAIN_LEVEL_HPP 1

#include <kernel/base_header.hpp>
#include <kernel/geometry/mesh_node.hpp>
#include <kernel/assembly/domain_assembler.hpp>

namespace FEAT
{
  namespace Control
  {
    namespace Domain
    {
      template<typename Mesh_>
      class DomainLevel
      {
      public:
        typedef Mesh_ MeshType;
        typedef typename MeshType::ShapeType ShapeType;
        typedef Geometry::MeshPart<Mesh_> PartType;
        typedef Geometry::RootMeshNode<MeshType> MeshNodeType;

      protected:
        int _level_index;
        std::unique_ptr<MeshNodeType> _mesh_node;

      public:
        explicit DomainLevel(int _lvl_idx, std::unique_ptr<MeshNodeType> node) :
          _level_index(_lvl_idx),
          _mesh_node(std::move(node))
        {
          // note: we have to check _mesh_node instead of node here,
          // because the unique_ptr has already been moved...
          XASSERT(_mesh_node.get() != nullptr);
        }

        DomainLevel(DomainLevel&& other) :
          _level_index(_level_index),
          _mesh_node(std::forward<std::unique_ptr<MeshNodeType>>(other._mesh_node))
        {
        }

        virtual ~DomainLevel()
        {
        }

        std::size_t bytes() const
        {
          return _mesh_node->bytes();
        }

        int get_level_index() const
        {
          return _level_index;
        }

        MeshNodeType* get_mesh_node()
        {
          return _mesh_node.get();
        }

        const MeshNodeType* get_mesh_node() const
        {
          return _mesh_node.get();
        }

        MeshType& get_mesh()
        {
          return *_mesh_node->get_mesh();
        }

        const MeshType& get_mesh() const
        {
          return *_mesh_node->get_mesh();
        }

        const PartType* find_halo_part(int rank) const
        {
          return this->_mesh_node->get_halo(rank);
        }

        const PartType* find_patch_part(int rank) const
        {
          return this->_mesh_node->get_patch(rank);
        }
      }; // class DomainLevel<...>

      template<typename Mesh_, typename Trafo_, typename Space_>
      class SimpleDomainLevel :
        public Domain::DomainLevel<Mesh_>
      {
      public:
        typedef Domain::DomainLevel<Mesh_> BaseClass;

        typedef Mesh_ MeshType;
        typedef Trafo_ TrafoType;
        typedef Space_ SpaceType;

        TrafoType trafo;
        SpaceType space;

        Assembly::DomainAssembler<TrafoType> domain_asm;

      public:
        explicit SimpleDomainLevel(int lvl_idx, std::unique_ptr<Geometry::RootMeshNode<MeshType>> node) :
          BaseClass(lvl_idx, std::move(node)),
          trafo(BaseClass::get_mesh()),
          space(trafo),
          domain_asm(trafo)
        {
        }
      }; // class SimpleDomainLevel<...>

      template<typename Mesh_, typename Trafo_, typename SpaceVelo_, typename SpacePres_>
      class StokesDomainLevel :
        public Domain::DomainLevel<Mesh_>
      {
      public:
        typedef Domain::DomainLevel<Mesh_> BaseClass;

        typedef Mesh_ MeshType;
        typedef Trafo_ TrafoType;
        typedef SpaceVelo_ SpaceVeloType;
        typedef SpacePres_ SpacePresType;

        TrafoType trafo;
        SpaceVeloType space_velo;
        SpacePresType space_pres;

        Assembly::DomainAssembler<TrafoType> domain_asm;

      public:
        explicit StokesDomainLevel(int lvl_idx, std::unique_ptr<Geometry::RootMeshNode<MeshType>> node) :
          BaseClass(lvl_idx, std::move(node)),
          trafo(BaseClass::get_mesh()),
          space_velo(trafo),
          space_pres(trafo),
          domain_asm(trafo)
        {
        }
      }; // class StokesDomainLevel<...>

      template<typename Mesh_, typename Trafo_, typename SpaceVelo_, typename SpacePres_, typename SpaceStress_>
      class Stokes3FieldDomainLevel :
        public Domain::DomainLevel<Mesh_>
      {
      public:
        typedef Domain::DomainLevel<Mesh_> BaseClass;

        typedef Mesh_ MeshType;
        typedef Trafo_ TrafoType;
        typedef SpaceVelo_ SpaceVeloType;
        typedef SpacePres_ SpacePresType;
        typedef SpaceStress_ SpaceStressType;

        TrafoType trafo;
        SpaceVeloType space_velo;
        SpacePresType space_pres;
        SpaceStressType space_stress;

        Assembly::DomainAssembler<TrafoType> domain_asm;

      public:
        explicit Stokes3FieldDomainLevel(int lvl_idx, std::unique_ptr<Geometry::RootMeshNode<MeshType>> node) :
          BaseClass(lvl_idx, std::move(node)),
          trafo(BaseClass::get_mesh()),
          space_velo(trafo),
          space_pres(trafo),
          space_stress(trafo),
          domain_asm(trafo)
        {
        }
      }; // class Stokes3FieldDomainLevel<...>
    } // namespace Domain
  } // namespace Control
} // namespace FEAT

#endif // CONTROL_DOMAIN_DOMAIN_LEVEL_HPP
