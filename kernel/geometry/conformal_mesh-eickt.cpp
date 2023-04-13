// FEAT3: Finite Element Analysis Toolbox, Version 3
// Copyright (C) 2010 - 2023 by Stefan Turek & the FEAT group
// FEAT3 is released under the GNU General Public License version 3,
// see the file 'copyright.txt' in the top level directory for details.

// includes, FEAT
#include <kernel/geometry/conformal_mesh.hpp>

namespace FEAT
{
  namespace Geometry
  {
    template class ConformalMesh<Shape::Simplex<2>, 2, Real>;
    template class ConformalMesh<Shape::Simplex<3>, 3, Real>;
    template class ConformalMesh<Shape::Hypercube<2>, 2, Real>;
    template class ConformalMesh<Shape::Hypercube<3>, 3, Real>;

    template class StandardRefinery<ConformalMesh<Shape::Simplex<2>, 2, Real>>;
    template class StandardRefinery<ConformalMesh<Shape::Simplex<3>, 3, Real>>;
    template class StandardRefinery<ConformalMesh<Shape::Hypercube<2>, 2, Real>>;
    template class StandardRefinery<ConformalMesh<Shape::Hypercube<3>, 3, Real>>;
  } // namespace Geometry
} // namespace FEAT
