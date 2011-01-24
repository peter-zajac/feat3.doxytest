// includes, system
#include <iostream> // for std::ostream
#include <cstdlib> // for exit()

// includes, FEAST
#include <kernel/base_header.hpp>
#include <kernel/base_mesh_3d.hpp>
//#include <kernel/base_mesh_cell.hpp>

using namespace FEAST;
using namespace BaseMesh;

#define WDIM 3
#define SDIM 3

int main (int argc, char **argv)
{
  BaseMesh3D<WDIM> bm;
  bm.print(std::cout);

  // validate base mesh
  bm.validate();

  // subdivide cell 0
  std::cout << "Subdividing cell 0" << std::endl;
  SubdivisionData<3, WDIM, SDIM> subdiv_data;
  bm.cell(0)->subdivide(subdiv_data);
  bm.add_created_items(subdiv_data);
  bm.print(std::cout);

  // validate base mesh
  bm.validate();

  std::cout << "!!! Neighbourhood update after subdivision not implemented yet!!!" << std::endl;
  std::cout << "!!! Neighbourhood update after subdivision not implemented yet!!!" << std::endl;
  std::cout << "!!! DTORS not checked yet! Possible memory holes! Not 'valgrinded' yet !!!" << std::endl;
  std::cout << "!!! DTORS not checked yet! Possible memory holes! Not 'valgrinded' yet !!!" << std::endl;
}
