// FEAT3: Finite Element Analysis Toolbox, Version 3
// Copyright (C) 2010 - 2023 by Stefan Turek & the FEAT group
// FEAT3 is released under the GNU General Public License version 3,
// see the file 'copyright.txt' in the top level directory for details.

#pragma once
#ifndef FEAT_APPLICATIONS_CCND_SIMPLE_DOMAIN_HPP
#define FEAT_APPLICATIONS_CCND_SIMPLE_DOMAIN_HPP 1

#include "base.hpp"

#include <control/domain/parti_domain_control.hpp>
#include <kernel/assembly/domain_assembler.hpp>

namespace CCNDSimple
{
  /**
   * \brief Domain level class for CCND apps
   *
   * This class stores the mesh node, the trafo, the FE spaces as well as the domain assembler for
   * a single refinement level.
   *
   * See the base class in <control/domain/domain_level.hpp> for details.
   *
   * \author Peter Zajac
   */
  class DomainLevel :
    public Control::Domain::DomainLevel<MeshType>
  {
  public:
    /// our base class type
    typedef Control::Domain::DomainLevel<MeshType> BaseClass;

    /// our trafo
    TrafoType trafo;

    /// our velocity space: Lagrange-2
    SpaceVeloType space_velo;

    /// our pressure space: discontinuous P1
    SpacePresType space_pres;

    /// auxiliary space: Lagrange-1
    SpaceTypeQ1 space_q1;

    /// our domain assembler object
    Assembly::DomainAssembler<TrafoType> domain_asm;

  public:
    /**
     * \brief Constructor
     *
     * \param[in] lvl_idx
     * The level index of this domain level.
     *
     * \param[in] node
     * The mesh node for this domain level
     */
    explicit DomainLevel(int lvl_idx, std::unique_ptr<Geometry::RootMeshNode<MeshType>> node);

    /// destructor
    virtual ~DomainLevel();

    virtual bool add_isoparam_part(const String& part_name);

    virtual std::deque<String> add_all_isoparam_parts();
  }; // class DomainLevel<...>

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  /**
   * \brief Domain controller class for simple CCND apps
   *
   * This class creates and manages the distributed domain and mesh hierarchy
   *
   * See the base class (and its base class) in <control/domain/parti_domain_control.hpp> for details.
   *
   * \author Peter Zajac
   */
  class DomainControl :
    public Control::Domain::PartiDomainControl<DomainLevel>
  {
  public:
    /// our base class
    typedef Control::Domain::PartiDomainControl<DomainLevel> BaseClass;

    /// the names of the mesh files
    std::deque<String> mesh_file_names;

    /// the iso-parametric parts to be added
    std::deque<String> isoparam_part_names;

  public:
    /**
     * \brief Constructor
     *
     * \param[in] comm_
     * The communicator for this domain control
     */
    explicit DomainControl(const Dist::Comm& comm_);

    /// destructor
    virtual ~DomainControl();

    /// adds all supported arguments to the argument parser
    static void add_supported_args(SimpleArgParser& args);

    /// parses the arguments
    virtual bool parse_args(SimpleArgParser& args);

    /// creates the domain controller based on the arguments
    virtual void create(SimpleArgParser& args);

    /// prints info on chosen partitioning and levels after creation
    virtual void print_info();
  }; // class DomainControl
} // namespace CCNDSimple

#endif // FEAT_APPLICATIONS_CCND_SIMPLE_DOMAIN_HPP
