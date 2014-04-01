#include <test_system/test_system.hpp>
#include <kernel/geometry/test_aux/index_calculator_meshes.hpp>
#include <kernel/geometry/test_aux/tetris_hexa.hpp>
#include <kernel/geometry/test_aux/standard_tetra.hpp>

using namespace FEAST;
using namespace FEAST::TestSystem;
using namespace FEAST::Geometry;
using namespace FEAST::Geometry::TestAux;

/**
 * \brief Test class for the IndexCalculator class template and the IndexTree class template.
 *
 * \test Tests the IndexCalculator class template an the IndexTree class template.
 *
 * \author Constantin Christof
 */

class IndexCalculatorTest
  : public TestSystem::TaggedTest<Archs::None, Archs::None>
{
public:
  IndexCalculatorTest() :
    TestSystem::TaggedTest<Archs::None, Archs::None>("index_calculator-test")
  {
  }

  // run the tests
  virtual void run() const
  {
    hexa_index_tree_test();
    tetra_index_tree_test();
    hexa_index_calculator_test();
    tetra_index_calculator_test();
  }

  void hexa_index_tree_test() const
  {
    // create mesh and index-trees
    HexaMesh* mesh = create_tetris_mesh_3d();
    HEdgeIndexTree* edge_tree;
    QuadIndexTree* quad_tree;

    try
    {
      // typedef index set type
      typedef IndexSet<2> IndexSetTypeEV;
      typedef IndexSet<4> IndexSetTypeQV;

      // fetch the quad-vertex- and the edge-vertex-index set
      const IndexSetTypeEV& index_set_e_v = mesh->get_index_set<1,0>();
      const IndexSetTypeQV& index_set_q_v = mesh->get_index_set<2,0>();

      // create index-tree objects
      Index vertex_number = mesh->get_num_entities(0);
      edge_tree = new HEdgeIndexTree(vertex_number);
      quad_tree = new QuadIndexTree(vertex_number);

      // parsing
      edge_tree->parse(index_set_e_v);
      quad_tree->parse(index_set_q_v);

      // check if everything is right
      validate_hypercube_edge_index_set(*edge_tree);
      validate_hypercube_quad_index_set(*quad_tree);

      // clean up
      delete mesh;
      delete edge_tree;
      delete quad_tree;
    }

    catch(const String& msg)
    {
      TEST_CHECK_MSG(false, msg);
    }
  } // hexa_index_tree_test()

  void tetra_index_tree_test() const
  {
    // create mesh
    TetraMesh* mesh = create_big_tetra_mesh_3d();
    SEdgeIndexTree* edge_tree = nullptr;
    TriaIndexTree* tria_tree = nullptr;

    try
    {
      // typedef index set type
      typedef IndexSet<2> IndexSetTypeEV;
      typedef IndexSet<3> IndexSetTypeTV;

      // fetch the triangle-vertex- and the edge-vertex index set
      const IndexSetTypeEV& index_set_e_v = mesh->get_index_set<1,0>();
      const IndexSetTypeTV& index_set_t_v = mesh->get_index_set<2,0>();

      // create index-tree objects
      Index vertex_number = mesh->get_num_entities(0);
      edge_tree = new SEdgeIndexTree(vertex_number);
      tria_tree = new TriaIndexTree(vertex_number);

      // parsing
      edge_tree->parse(index_set_e_v);
      tria_tree->parse(index_set_t_v);

      // checking
      validate_simplex_edge_index_set(*edge_tree);
      validate_simplex_triangle_index_set(*tria_tree);

      // clean up
      delete mesh;
      delete edge_tree;
      delete tria_tree;
    }

    catch(const String& msg)
    {
      TEST_CHECK_MSG(false, msg);
    }
  } // simplex_index_tree_test()

  void hexa_index_calculator_test() const
  {
    // typedef calculators
    typedef IndexCalculator<Shape::Hypercube<2>, 1> Quad_Edge_Calc;
    typedef IndexCalculator<Shape::Hypercube<3>, 1> Cube_Edge_Calc;
    typedef IndexCalculator<Shape::Hypercube<3>, 2> Cube_Quad_Calc;

    // create mesh
    HexaMesh* mesh = create_big_tetris_mesh_3d();
    HEdgeIndexTree* edge_tree = nullptr;
    QuadIndexTree* quad_tree = nullptr;

    try
    {
      // typedef index set types
      typedef IndexSet<2> IndexSetTypeEV;

      typedef IndexSet<4> IndexSetTypeQV;
      typedef IndexSet<4> IndexSetTypeQE;

      typedef IndexSet<8> IndexSetTypeCV;
      typedef IndexSet<12> IndexSetTypeCE;

      typedef IndexSet<6> IndexSetTypeCQ;

      // fetch the index sets
      const IndexSetTypeEV& index_set_e_v = mesh->get_index_set<1,0>();

      const IndexSetTypeQV& index_set_q_v = mesh->get_index_set<2,0>();
      IndexSetTypeQE& index_set_q_e = mesh->get_index_set<2,1>();

      const IndexSetTypeCV& index_set_c_v = mesh->get_index_set<3,0>();
      IndexSetTypeCE& index_set_c_e = mesh->get_index_set<3,1>();

      IndexSetTypeCQ& index_set_c_q = mesh->get_index_set<3,2>();

      // create index-tree objects
      Index vertex_number = mesh->get_num_entities(0);
      edge_tree = new HEdgeIndexTree(vertex_number);
      quad_tree = new QuadIndexTree(vertex_number);

      // parsing
      edge_tree->parse(index_set_e_v);
      quad_tree->parse(index_set_q_v);

      if(!Quad_Edge_Calc::compute(*edge_tree, index_set_q_v, index_set_q_e))
        throw String("Failed to compute edges-at-quad index set");
      if(!Cube_Edge_Calc::compute(*edge_tree, index_set_c_v, index_set_c_e))
        throw String("Failed to compute edges-at-hexa index set");
      if(!Cube_Quad_Calc::compute(*quad_tree, index_set_c_v, index_set_c_q))
        throw String("Failed to compute quads-at-hexa index set");

      // check if everything is right
      validate_refined_tetris_mesh_3d(*mesh);

      // clean up
      delete mesh;
      delete edge_tree;
      delete quad_tree;
    }

    catch(const String& msg)
    {
      TEST_CHECK_MSG(false, msg);
    }
  } // hexa_index_calculator_test()

  void tetra_index_calculator_test() const
  {
    typedef IndexCalculator<Shape::Simplex<2>, 1> Tria_Edge_Calc;
    typedef IndexCalculator<Shape::Simplex<3>, 1> Tetra_Edge_Calc;
    typedef IndexCalculator<Shape::Simplex<3>, 2> Tetra_Tria_Calc;

    // create mesh
    TetraMesh* mesh = create_really_big_tetra_mesh_3d();
    SEdgeIndexTree* edge_tree = nullptr;
    TriaIndexTree* tria_tree = nullptr;

    try
    {

      // typedef index set types
      typedef IndexSet<2> IndexSetTypeEV;

      typedef IndexSet<3> IndexSetTypeTV;
      typedef IndexSet<3> IndexSetTypeTE;

      typedef IndexSet<4> IndexSetTypeSV;
      typedef IndexSet<6> IndexSetTypeSE;

      typedef IndexSet<4> IndexSetTypeST;

      // fetch the index sets
      const IndexSetTypeEV& index_set_e_v = mesh->get_index_set<1,0>();

      const IndexSetTypeTV& index_set_t_v = mesh->get_index_set<2,0>();
      IndexSetTypeTE& index_set_t_e = mesh->get_index_set<2,1>();

      const IndexSetTypeSV& index_set_s_v = mesh->get_index_set<3,0>();
      IndexSetTypeSE& index_set_s_e = mesh->get_index_set<3,1>();

      IndexSetTypeST& index_set_s_t = mesh->get_index_set<3,2>();

      // create index-calculator objects
      Index vertex_number = mesh->get_num_entities(0);
      edge_tree = new SEdgeIndexTree(vertex_number);
      tria_tree = new TriaIndexTree(vertex_number);

      // parsing
      edge_tree->parse(index_set_e_v);
      tria_tree->parse(index_set_t_v);

      // compute index sets
      if(!Tria_Edge_Calc::compute(*edge_tree, index_set_t_v, index_set_t_e))
        throw String("Failed to compute edges-at-tria index set");
      if(!Tetra_Edge_Calc::compute(*edge_tree, index_set_s_v, index_set_s_e))
        throw String("Failed to compute edges-at-tetra index set");
      if(!Tetra_Tria_Calc::compute(*tria_tree, index_set_s_v, index_set_s_t))
        throw String("Failed to compute trias-at-tetra index set");

      // check if everything is right
      validate_refined_big_tetra_mesh_3d(*mesh);

      // clean up
      delete mesh;
      delete edge_tree;
      delete tria_tree;
    }

    catch(const String& msg)
    {
      TEST_CHECK_MSG(false, msg);
    }
  } // tetra_index_calculator_test()

} index_calculator_test;
