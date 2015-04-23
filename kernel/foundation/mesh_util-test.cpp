#include <kernel/base_header.hpp>
#include <test_system/test_system.hpp>

#include<kernel/foundation/partitioning.hpp>
#include<kernel/foundation/aura.hpp>
#include<kernel/archs.hpp>

#include<deque>

using namespace FEAST;
using namespace FEAST::TestSystem;
using namespace FEAST::Foundation;

template<typename Tag_, typename IndexType_, template<typename, typename> class OT_, typename IT_>
class MeshUtilTest2D:
  public TaggedTest<Tag_, IndexType_>
{
  public:
    MeshUtilTest2D(const std::string & tag) :
      TaggedTest<Tag_, Index>("MeshUtilTest2D<" + tag + ">")
    {
    }

    virtual void run() const
    {
      /*(0,1) (1,1)
       *  *----*
       *  |    |
       *  |    |
       *  *----*
       *(0,0) (1,0)
       */

      //create attributes for vertex coords
      OT_<Attribute<double, OT_>, std::allocator<Attribute<double, OT_> > > attrs;
      attrs.push_back(Attribute<double, OT_>()); //vertex x-coords
      attrs.push_back(Attribute<double, OT_>()); //vertex y-coords

      attrs.at(0).get_data().push_back(double(0));
      attrs.at(1).get_data().push_back(double(0));

      attrs.at(0).get_data().push_back(double(1));
      attrs.at(1).get_data().push_back(double(0));

      attrs.at(0).get_data().push_back(double(0));
      attrs.at(1).get_data().push_back(double(1));

      attrs.at(0).get_data().push_back(double(1));
      attrs.at(1).get_data().push_back(double(1));

      /*  2    3
       *  *-1--*
       *  2    |
       *  |    3
       *  *--0-*
       *  0    1
       */

      //creating foundation mesh
      Foundation::Mesh<Dim2D, Foundation::Topology<IndexType_, OT_, IT_>, OT_> m(0);
      m.add_polytope(pl_vertex);
      m.add_polytope(pl_vertex);
      m.add_polytope(pl_vertex);
      m.add_polytope(pl_vertex);

      m.add_polytope(pl_edge);
      m.add_polytope(pl_edge);
      m.add_polytope(pl_edge);
      m.add_polytope(pl_edge);

      m.add_polytope(pl_face);

      m.add_adjacency(pl_vertex, pl_edge, 0, 0);
      m.add_adjacency(pl_vertex, pl_edge, 0, 2);
      m.add_adjacency(pl_vertex, pl_face, 0, 0);

      m.add_adjacency(pl_vertex, pl_edge, 1, 0);
      m.add_adjacency(pl_vertex, pl_edge, 1, 3);
      m.add_adjacency(pl_vertex, pl_face, 1, 0);

      m.add_adjacency(pl_vertex, pl_edge, 2, 1);
      m.add_adjacency(pl_vertex, pl_edge, 2, 2);
      m.add_adjacency(pl_vertex, pl_face, 2, 0);

      m.add_adjacency(pl_vertex, pl_edge, 3, 1);
      m.add_adjacency(pl_vertex, pl_edge, 3, 3);
      m.add_adjacency(pl_vertex, pl_face, 3, 0);

      TEST_CHECK(MeshUtil::iz_property_quad(m, attrs.at(0), attrs.at(1)));

      //creating foundation mesh with disturbed F/V adj
      Foundation::Mesh<Dim2D, Foundation::Topology<IndexType_, OT_, IT_>, OT_> m2;
      m2.add_polytope(pl_vertex);
      m2.add_polytope(pl_vertex);
      m2.add_polytope(pl_vertex);
      m2.add_polytope(pl_vertex);

      m2.add_polytope(pl_edge);
      m2.add_polytope(pl_edge);
      m2.add_polytope(pl_edge);
      m2.add_polytope(pl_edge);

      m2.add_polytope(pl_face);

      m2.add_adjacency(pl_vertex, pl_edge, 0, 0);

      m2.add_adjacency(pl_vertex, pl_edge, 1, 0);
      m2.add_adjacency(pl_vertex, pl_edge, 1, 3);
      m2.add_adjacency(pl_vertex, pl_edge, 0, 2);

      m2.add_adjacency(pl_vertex, pl_edge, 2, 1);
      m2.add_adjacency(pl_vertex, pl_edge, 2, 2);

      m2.add_adjacency(pl_vertex, pl_edge, 3, 1);
      m2.add_adjacency(pl_vertex, pl_edge, 3, 3);

      m2.add_adjacency(pl_vertex, pl_face, 1, 0);
      m2.add_adjacency(pl_vertex, pl_face, 0, 0);
      m2.add_adjacency(pl_vertex, pl_face, 2, 0);
      m2.add_adjacency(pl_vertex, pl_face, 3, 0);

      TEST_CHECK(!MeshUtil::iz_property_quad(m2, attrs.at(0), attrs.at(1)));
      MeshUtil::establish_iz_property_quad(m2, attrs.at(0), attrs.at(1));
      TEST_CHECK(MeshUtil::iz_property_quad(m2, attrs.at(0), attrs.at(1)));

      //after refinement
      Foundation::Mesh<Dim2D, Foundation::Topology<IndexType_, OT_, IT_>, OT_> m_fine(m2);

      OT_<std::shared_ptr<HaloBase<Mesh<Dim2D, Topology<IndexType_, OT_, IT_>, OT_>, double, OT_> >, std::allocator<std::shared_ptr<HaloBase<Mesh<Dim2D, Topology<IndexType_, OT_, IT_>, OT_>, double, OT_> > > > halos;

      Refinement<Mem::Main,
                 mrt_standard>::execute(m_fine, &halos, attrs);
      TEST_CHECK(MeshUtil::iz_property_quad(m_fine, attrs.at(0), attrs.at(1)));
      /*std::cout << "BEFORE!" << std::endl;
      for(Index i(0) ; i < m_fine.num_polytopes(pl_face) ; ++i)
      {
        std::cout << "FACE " << i << std::endl;
        auto edges(m_fine.get_adjacent_polytopes(pl_face, pl_edge, i));
        auto outverts(m_fine.get_adjacent_polytopes(pl_face, pl_vertex, i));
        std::cout << "v_fi:" << std::endl;
        for(auto& v : outverts)
          std::cout << v << std::endl;

        for(auto& j : edges)
        {
          std::cout << "  EDGE " << j << std::endl;
          auto verts(m_fine.get_adjacent_polytopes(pl_edge, pl_vertex, j));
          for(auto& k : verts)
          {
            std::cout << "    VERTEX " << k << std::endl;
            std::cout << "      (" << attrs.at(0).at(k) << " , " << attrs.at(1).at(k) << ")" << std::endl;
          }
        }
      }*/

      //MeshUtil::establish_iz_property(m_fine, attrs.at(0), attrs.at(1));
      /*std::cout << "AFTER!" << std::endl;
      for(Index i(0) ; i < m_fine.num_polytopes(pl_face) ; ++i)
      {
        std::cout << "FACE " << i << std::endl;
        auto edges(m_fine.get_adjacent_polytopes(pl_face, pl_edge, i));
        auto outverts(m_fine.get_adjacent_polytopes(pl_face, pl_vertex, i));
        std::cout << "v_fi:" << std::endl;
        for(auto& v : outverts)
          std::cout << v << std::endl;

        for(auto& j : edges)
        {
          std::cout << "  EDGE " << j << std::endl;
          auto verts(m_fine.get_adjacent_polytopes(pl_edge, pl_vertex, j));
          for(auto& k : verts)
          {
            std::cout << "    VERTEX " << k << std::endl;
            std::cout << "      (" << attrs.at(0).at(k) << " , " << attrs.at(1).at(k) << ")" << std::endl;
          }
        }
      }*/
      TEST_CHECK(MeshUtil::iz_property_quad(m_fine, attrs.at(0), attrs.at(1)));

      //after partitioning
      Foundation::Mesh<Dim2D, Foundation::Topology<IndexType_, OT_, IT_>, OT_> m_part(m2);
      OT_<Halo<0, PLEdge, Mesh<Dim2D, Topology<IndexType_, OT_, IT_>, OT_>, double, OT_>, std::allocator<Halo<0, PLEdge, Mesh<Dim2D, Topology<IndexType_, OT_, IT_>, OT_>, double, OT_> > > boundaries;
      boundaries.push_back(Halo<0, PLEdge, Mesh<Dim2D, Topology<IndexType_, OT_, IT_>, OT_>, double, OT_>(m));
      boundaries.push_back(Halo<0, PLEdge, Mesh<Dim2D, Topology<IndexType_, OT_, IT_>, OT_>, double, OT_>(m));
      boundaries.push_back(Halo<0, PLEdge, Mesh<Dim2D, Topology<IndexType_, OT_, IT_>, OT_>, double, OT_>(m));
      boundaries.push_back(Halo<0, PLEdge, Mesh<Dim2D, Topology<IndexType_, OT_, IT_>, OT_>, double, OT_>(m));
      boundaries.at(0).push_back(0);
      boundaries.at(1).push_back(1);
      boundaries.at(2).push_back(2);
      boundaries.at(3).push_back(3);

      Index num_procs(4);
      Index rank(0);

      PData<Dim2D, Topology<IndexType_, OT_, IT_>, OT_, Mesh, double> p0(Partitioning<Tag_,
                                                                                      Dim2D,
                                                                                      0,
                                                                                      pl_vertex>::execute(m,
                                                                                                          boundaries,
                                                                                                          num_procs, rank,
                                                                                                          attrs
                                                                                                          ));


      TEST_CHECK(MeshUtil::iz_property_quad(p0.basemesh, attrs.at(0), attrs.at(1)));
      //MeshUtil::establish_iz_property(p0.basemesh, attrs.at(0), attrs.at(1));
      TEST_CHECK(MeshUtil::iz_property_quad(p0.basemesh, attrs.at(0), attrs.at(1)));
      //MeshUtil::establish_iz_property(*((Mesh<Dim2D, Topology<IndexType_, OT_, IT_>, OT_>*)(p0.submesh.get())), *( (Attribute<double, OT_>*)(p0.attrs.at(0).get()) ), *( (Attribute<double, OT_>*)(p0.attrs.at(1).get()) ));
      TEST_CHECK(MeshUtil::iz_property_quad(*((Mesh<Dim2D, Topology<IndexType_, OT_, IT_>, OT_>*)(p0.submesh.get())), *( (Attribute<double, OT_>*)(p0.attrs.at(0).get()) ), *( (Attribute<double, OT_>*)(p0.attrs.at(1).get()) )));
    }
};
MeshUtilTest2D<Mem::Main, Index, std::vector, std::vector<Index> > mu_test_cpu_v_v_2d("std::vector, std::vector");
/*MeshUtilTest2D<Mem::Main, Index, std::vector, std::deque<Index> > mu_test_cpu_v_d_2d("std::vector, std::deque");
MeshUtilTest2D<Mem::Main, Index, std::deque, std::vector<Index> > mu_test_cpu_d_v_2d("std::deque, std::vector");
MeshUtilTest2D<Mem::Main, Index, std::deque, std::deque<Index> > mu_test_cpu_d_d_2d("std::deque, std::deque");*/


template<typename Tag_, typename IndexType_, template<typename, typename> class OT_, typename IT_>
class MeshUtilTest2D_triangle:
  public TaggedTest<Tag_, IndexType_>
{
  public:
    MeshUtilTest2D_triangle(const std::string & tag) :
      TaggedTest<Tag_, Index>("MeshUtilTest2D_triangle<" + tag + ">")
    {
    }

    virtual void run() const
    {
      /*(0,1)
       *  *\
       *  | \
       *  |  \
       *  |   \
       *  *----*
       *(0,0) (1,0)
       */

      //create attributes for vertex coords
      OT_<Attribute<double, OT_>, std::allocator<Attribute<double, OT_> > > attrs;
      attrs.push_back(Attribute<double, OT_>()); //vertex x-coords
      attrs.push_back(Attribute<double, OT_>()); //vertex y-coords

      attrs.at(0).get_data().push_back(double(0));
      attrs.at(1).get_data().push_back(double(0));

      attrs.at(0).get_data().push_back(double(1));
      attrs.at(1).get_data().push_back(double(0));

      attrs.at(0).get_data().push_back(double(0));
      attrs.at(1).get_data().push_back(double(1));

      /*  2
       *  *\
       *  | \
       *  2  1
       *  |   \
       *  *--0-*
       *  0    1
       */

      //creating foundation mesh
      Foundation::Mesh<Dim2D, Foundation::Topology<IndexType_, OT_, IT_>, OT_> m(0);
      m.add_polytope(pl_vertex);
      m.add_polytope(pl_vertex);
      m.add_polytope(pl_vertex);

      m.add_polytope(pl_edge);
      m.add_polytope(pl_edge);
      m.add_polytope(pl_edge);

      m.add_polytope(pl_face);

      m.add_adjacency(pl_vertex, pl_edge, 0, 0);
      m.add_adjacency(pl_vertex, pl_edge, 0, 2);
      m.add_adjacency(pl_vertex, pl_face, 0, 0);

      m.add_adjacency(pl_vertex, pl_edge, 1, 0);
      m.add_adjacency(pl_vertex, pl_edge, 1, 1);
      m.add_adjacency(pl_vertex, pl_face, 1, 0);

      m.add_adjacency(pl_vertex, pl_edge, 2, 1);
      m.add_adjacency(pl_vertex, pl_edge, 2, 2);
      m.add_adjacency(pl_vertex, pl_face, 2, 0);

      TEST_CHECK(!MeshUtil::ccw_property_triangle(m, attrs.at(0), attrs.at(1)));
      MeshUtil::establish_ccw_property_triangle(m, attrs.at(0), attrs.at(1));
      TEST_CHECK(MeshUtil::ccw_property_triangle(m, attrs.at(0), attrs.at(1)));

      //creating foundation mesh with disturbed F/V adj
      Foundation::Mesh<Dim2D, Foundation::Topology<IndexType_, OT_, IT_>, OT_> m2;
      m2.add_polytope(pl_vertex);
      m2.add_polytope(pl_vertex);
      m2.add_polytope(pl_vertex);

      m2.add_polytope(pl_edge);
      m2.add_polytope(pl_edge);
      m2.add_polytope(pl_edge);

      m2.add_polytope(pl_face);

      m2.add_adjacency(pl_vertex, pl_edge, 0, 0);
      m2.add_adjacency(pl_vertex, pl_edge, 0, 2);
      m2.add_adjacency(pl_vertex, pl_face, 0, 0);

      m2.add_adjacency(pl_vertex, pl_edge, 1, 1);
      m2.add_adjacency(pl_vertex, pl_edge, 1, 2);
      m2.add_adjacency(pl_vertex, pl_face, 1, 0);

      m2.add_adjacency(pl_vertex, pl_edge, 2, 0);
      m2.add_adjacency(pl_vertex, pl_edge, 2, 1);
      m2.add_adjacency(pl_vertex, pl_face, 2, 0);

      TEST_CHECK(!MeshUtil::ccw_property_triangle(m2, attrs.at(0), attrs.at(1)));
      MeshUtil::establish_ccw_property_triangle(m2, attrs.at(0), attrs.at(1));
      TEST_CHECK(MeshUtil::ccw_property_triangle(m2, attrs.at(0), attrs.at(1)));

    }
};
MeshUtilTest2D_triangle<Mem::Main, Index, std::vector, std::vector<Index> > mu_test_cpu_v_v_2d_tria("std::vector, std::vector");
/*MeshUtilTest2D_triangle<Mem::Main, Index, std::vector, std::deque<Index> > mu_test_cpu_v_d_2d_tria("std::vector, std::deque");
MeshUtilTest2D_triangle<Mem::Main, Index, std::deque, std::vector<Index> > mu_test_cpu_d_v_2d_tria("std::deque, std::vector");
MeshUtilTest2D_triangle<Mem::Main, Index, std::deque, std::deque<Index> > mu_test_cpu_d_d_2d_tria("std::deque, std::deque");*/



template<typename Tag_, typename IndexType_, template<typename, typename> class OT_, typename IT_>
class MeshUtilTest2D_triangle_fine:
  public TaggedTest<Tag_, IndexType_>
{
  public:
    MeshUtilTest2D_triangle_fine(const std::string & tag) :
      TaggedTest<Tag_, Index>("MeshUtilTest2D_triangle_fine<" + tag + ">")
    {
    }

    virtual void run() const
    {
      /*(0,1)
       *  *\
       *  | \
       *  |  \
       *  |   \
       *  *----*
       *(0,0) (1,0)
       */

      //create attributes for vertex coords
      OT_<Attribute<double, OT_>, std::allocator<Attribute<double, OT_> > > attrs;
      attrs.push_back(Attribute<double, OT_>()); //vertex x-coords
      attrs.push_back(Attribute<double, OT_>()); //vertex y-coords

      attrs.at(0).get_data().push_back(double(0));
      attrs.at(1).get_data().push_back(double(0));

      attrs.at(0).get_data().push_back(double(1));
      attrs.at(1).get_data().push_back(double(0));

      attrs.at(0).get_data().push_back(double(2));
      attrs.at(1).get_data().push_back(double(0));

      attrs.at(0).get_data().push_back(double(0));
      attrs.at(1).get_data().push_back(double(1));

      attrs.at(0).get_data().push_back(double(1));
      attrs.at(1).get_data().push_back(double(1));

      attrs.at(0).get_data().push_back(double(2));
      attrs.at(1).get_data().push_back(double(1));

      attrs.at(0).get_data().push_back(double(0));
      attrs.at(1).get_data().push_back(double(2));

      attrs.at(0).get_data().push_back(double(1));
      attrs.at(1).get_data().push_back(double(2));

      attrs.at(0).get_data().push_back(double(2));
      attrs.at(1).get_data().push_back(double(2));

      /*  2
       *  *\
       *  | \
       *  2  1
       *  |   \
       *  *--0-*
       *  0    1
       */

      //creating foundation mesh
      Foundation::Mesh<Dim2D, Foundation::Topology<IndexType_, OT_, IT_>, OT_> m(0);
      m.add_polytope(pl_vertex);
      m.add_polytope(pl_vertex);
      m.add_polytope(pl_vertex);
      m.add_polytope(pl_vertex);
      m.add_polytope(pl_vertex);
      m.add_polytope(pl_vertex);
      m.add_polytope(pl_vertex);
      m.add_polytope(pl_vertex);
      m.add_polytope(pl_vertex);

      m.add_polytope(pl_edge);
      m.add_polytope(pl_edge);
      m.add_polytope(pl_edge);
      m.add_polytope(pl_edge);
      m.add_polytope(pl_edge);
      m.add_polytope(pl_edge);
      m.add_polytope(pl_edge);
      m.add_polytope(pl_edge);
      m.add_polytope(pl_edge);
      m.add_polytope(pl_edge);
      m.add_polytope(pl_edge);
      m.add_polytope(pl_edge);
      m.add_polytope(pl_edge);
      m.add_polytope(pl_edge);
      m.add_polytope(pl_edge);
      m.add_polytope(pl_edge);

      m.add_polytope(pl_face);
      m.add_polytope(pl_face);
      m.add_polytope(pl_face);
      m.add_polytope(pl_face);
      m.add_polytope(pl_face);
      m.add_polytope(pl_face);
      m.add_polytope(pl_face);
      m.add_polytope(pl_face);

      m.add_adjacency(pl_vertex, pl_edge, 0, 0);
      m.add_adjacency(pl_vertex, pl_edge, 0, 2);
      m.add_adjacency(pl_vertex, pl_face, 0, 0);

      m.add_adjacency(pl_vertex, pl_edge, 1, 0);
      m.add_adjacency(pl_vertex, pl_edge, 1, 1);
      m.add_adjacency(pl_vertex, pl_edge, 1, 3);
      m.add_adjacency(pl_vertex, pl_edge, 1, 4);
      m.add_adjacency(pl_vertex, pl_face, 1, 0);
      m.add_adjacency(pl_vertex, pl_face, 1, 1);
      m.add_adjacency(pl_vertex, pl_face, 1, 2);

      m.add_adjacency(pl_vertex, pl_edge, 2, 1);
      m.add_adjacency(pl_vertex, pl_edge, 2, 5);
      m.add_adjacency(pl_vertex, pl_edge, 2, 6);
      m.add_adjacency(pl_vertex, pl_face, 2, 2);
      m.add_adjacency(pl_vertex, pl_face, 2, 3);

      m.add_adjacency(pl_vertex, pl_edge, 3, 2);
      m.add_adjacency(pl_vertex, pl_edge, 3, 3);
      m.add_adjacency(pl_vertex, pl_edge, 3, 7);
      m.add_adjacency(pl_vertex, pl_edge, 3, 9);
      m.add_adjacency(pl_vertex, pl_face, 3, 0);
      m.add_adjacency(pl_vertex, pl_face, 3, 1);
      m.add_adjacency(pl_vertex, pl_face, 3, 4);

      m.add_adjacency(pl_vertex, pl_edge, 4, 4);
      m.add_adjacency(pl_vertex, pl_edge, 4, 5);
      m.add_adjacency(pl_vertex, pl_edge, 4, 7);
      m.add_adjacency(pl_vertex, pl_edge, 4, 8);
      m.add_adjacency(pl_vertex, pl_edge, 4, 10);
      m.add_adjacency(pl_vertex, pl_edge, 4, 11);
      m.add_adjacency(pl_vertex, pl_face, 4, 1);
      m.add_adjacency(pl_vertex, pl_face, 4, 2);
      m.add_adjacency(pl_vertex, pl_face, 4, 3);
      m.add_adjacency(pl_vertex, pl_face, 4, 4);
      m.add_adjacency(pl_vertex, pl_face, 4, 5);
      m.add_adjacency(pl_vertex, pl_face, 4, 6);

      m.add_adjacency(pl_vertex, pl_edge, 5, 6);
      m.add_adjacency(pl_vertex, pl_edge, 5, 8);
      m.add_adjacency(pl_vertex, pl_edge, 5, 12);
      m.add_adjacency(pl_vertex, pl_edge, 5, 13);
      m.add_adjacency(pl_vertex, pl_face, 5, 3);
      m.add_adjacency(pl_vertex, pl_face, 5, 6);
      m.add_adjacency(pl_vertex, pl_face, 5, 7);

      m.add_adjacency(pl_vertex, pl_edge, 6, 9);
      m.add_adjacency(pl_vertex, pl_edge, 6, 10);
      m.add_adjacency(pl_vertex, pl_edge, 6, 14);
      m.add_adjacency(pl_vertex, pl_face, 6, 4);
      m.add_adjacency(pl_vertex, pl_face, 6, 5);

      m.add_adjacency(pl_vertex, pl_edge, 7, 11);
      m.add_adjacency(pl_vertex, pl_edge, 7, 12);
      m.add_adjacency(pl_vertex, pl_edge, 7, 14);
      m.add_adjacency(pl_vertex, pl_edge, 7, 15);
      m.add_adjacency(pl_vertex, pl_face, 7, 5);
      m.add_adjacency(pl_vertex, pl_face, 7, 6);
      m.add_adjacency(pl_vertex, pl_face, 7, 7);

      m.add_adjacency(pl_vertex, pl_edge, 8, 13);
      m.add_adjacency(pl_vertex, pl_edge, 8, 15);
      m.add_adjacency(pl_vertex, pl_face, 8, 7);

      TEST_CHECK(!MeshUtil::ccw_property_triangle(m, attrs.at(0), attrs.at(1)));
      MeshUtil::establish_ccw_property_triangle(m, attrs.at(0), attrs.at(1));
      TEST_CHECK(MeshUtil::ccw_property_triangle(m, attrs.at(0), attrs.at(1)));

    }
};
MeshUtilTest2D_triangle_fine<Mem::Main, Index, std::vector, std::vector<Index> > mu_test_cpu_v_v_2d_tria_fine("std::vector, std::vector");
/*MeshUtilTest2D_triangle<Mem::Main, Index, std::vector, std::deque<Index> > mu_test_cpu_v_d_2d_tria("std::vector, std::deque");
MeshUtilTest2D_triangle<Mem::Main, Index, std::deque, std::vector<Index> > mu_test_cpu_d_v_2d_tria("std::deque, std::vector");
MeshUtilTest2D_triangle<Mem::Main, Index, std::deque, std::deque<Index> > mu_test_cpu_d_d_2d_tria("std::deque, std::deque");*/

template<typename Tag_, typename IndexType_, template<typename, typename> class OT_, typename IT_>
class MeshUtilTest3D:
  public TaggedTest<Tag_, IndexType_>
{
  public:
    MeshUtilTest3D(const std::string & tag) :
      TaggedTest<Tag_, Index>("MeshUtilTest3D<" + tag + ">")
    {
    }

    virtual void run() const
    {
      /*(-0.5,1,1.5) (0.5,1,2)
       *  *----*
       *  |    |
       *  |    |
       *  *----*
       *(0,0,0) (1,0,0.5)
       */
      //create attributes for vertex coords
      OT_<Attribute<double, OT_>, std::allocator<Attribute<double, OT_> > > attrs;
      attrs.push_back(Attribute<double, OT_>()); //vertex x-coords
      attrs.push_back(Attribute<double, OT_>()); //vertex y-coords
      attrs.push_back(Attribute<double, OT_>()); //vertex z-coords

      attrs.at(0).get_data().push_back(double(0));
      attrs.at(1).get_data().push_back(double(0));
      attrs.at(2).get_data().push_back(double(0));

      attrs.at(0).get_data().push_back(double(1));
      attrs.at(1).get_data().push_back(double(0));
      attrs.at(2).get_data().push_back(double(0.5));

      attrs.at(0).get_data().push_back(double(-0.5));
      attrs.at(1).get_data().push_back(double(1));
      attrs.at(2).get_data().push_back(double(1.5));

      attrs.at(0).get_data().push_back(double(0.5));
      attrs.at(1).get_data().push_back(double(1));
      attrs.at(2).get_data().push_back(double(2));

      /*  2    3
       *  *-1--*
       *  2    |
       *  |    3
       *  *--0-*
       *  0    1
       */

      //creating foundation mesh
      Foundation::Mesh<Dim3D, Foundation::Topology<IndexType_, OT_, IT_>, OT_> m(0);
      m.add_polytope(pl_vertex);
      m.add_polytope(pl_vertex);
      m.add_polytope(pl_vertex);
      m.add_polytope(pl_vertex);

      m.add_polytope(pl_edge);
      m.add_polytope(pl_edge);
      m.add_polytope(pl_edge);
      m.add_polytope(pl_edge);
      m.add_polytope(pl_face);

      m.add_adjacency(pl_vertex, pl_edge, 0, 0);
      m.add_adjacency(pl_vertex, pl_edge, 0, 2);
      m.add_adjacency(pl_vertex, pl_face, 0, 0);

      m.add_adjacency(pl_vertex, pl_edge, 1, 0);
      m.add_adjacency(pl_vertex, pl_edge, 1, 3);
      m.add_adjacency(pl_vertex, pl_face, 1, 0);

      m.add_adjacency(pl_vertex, pl_edge, 2, 1);
      m.add_adjacency(pl_vertex, pl_edge, 2, 2);
      m.add_adjacency(pl_vertex, pl_face, 2, 0);

      m.add_adjacency(pl_vertex, pl_edge, 3, 1);
      m.add_adjacency(pl_vertex, pl_edge, 3, 3);
      m.add_adjacency(pl_vertex, pl_face, 3, 0);

      Foundation::Mesh<Dim3D, Foundation::Topology<IndexType_, OT_, IT_>, OT_> m2;
      m2.add_polytope(pl_vertex);
      m2.add_polytope(pl_vertex);
      m2.add_polytope(pl_vertex);
      m2.add_polytope(pl_vertex);

      m2.add_polytope(pl_edge);
      m2.add_polytope(pl_edge);
      m2.add_polytope(pl_edge);
      m2.add_polytope(pl_edge);

      m2.add_polytope(pl_face);

      m2.add_adjacency(pl_vertex, pl_edge, 0, 0);

      m2.add_adjacency(pl_vertex, pl_edge, 1, 0);
      m2.add_adjacency(pl_vertex, pl_edge, 1, 3);
      m2.add_adjacency(pl_vertex, pl_edge, 0, 2);

      m2.add_adjacency(pl_vertex, pl_edge, 2, 1);
      m2.add_adjacency(pl_vertex, pl_edge, 2, 2);

      m2.add_adjacency(pl_vertex, pl_edge, 3, 1);
      m2.add_adjacency(pl_vertex, pl_edge, 3, 3);

      m2.add_adjacency(pl_vertex, pl_face, 1, 0);
      m2.add_adjacency(pl_vertex, pl_face, 0, 0);
      m2.add_adjacency(pl_vertex, pl_face, 2, 0);
      m2.add_adjacency(pl_vertex, pl_face, 3, 0);

      TEST_CHECK(MeshUtil::iz_property_quad(m, attrs.at(0), attrs.at(1), attrs.at(2)));

      TEST_CHECK(!MeshUtil::iz_property_quad(m2, attrs.at(0), attrs.at(1), attrs.at(2)));
      MeshUtil::establish_iz_property_quad(m2, attrs.at(0), attrs.at(1), attrs.at(2));
      TEST_CHECK(MeshUtil::iz_property_quad(m2, attrs.at(0), attrs.at(1), attrs.at(2)));

      //after refinement
      Foundation::Mesh<Dim3D, Foundation::Topology<IndexType_, OT_, IT_>, OT_> m_fine(m2);

      OT_<std::shared_ptr<HaloBase<Mesh<Dim3D, Topology<IndexType_, OT_, IT_>, OT_>, double, OT_> >, std::allocator<std::shared_ptr<HaloBase<Mesh<Dim3D, Topology<IndexType_, OT_, IT_>, OT_>, double, OT_> > > > halos;

      Refinement<Mem::Main,
                 mrt_standard>::execute(m_fine, &halos, attrs);
      MeshUtil::establish_iz_property_quad(m_fine, attrs.at(0), attrs.at(1), attrs.at(2));
      TEST_CHECK(MeshUtil::iz_property_quad(m_fine, attrs.at(0), attrs.at(1), attrs.at(2)));
      /*std::cout << "BEFORE!" << std::endl;
      for(Index i(0) ; i < m_fine.num_polytopes(pl_face) ; ++i)
      {
        std::cout << "FACE " << i << std::endl;
        auto edges(m_fine.get_adjacent_polytopes(pl_face, pl_edge, i));
        auto outverts(m_fine.get_adjacent_polytopes(pl_face, pl_vertex, i));
        std::cout << "v_fi:" << std::endl;
        for(auto& v : outverts)
          std::cout << v << std::endl;

        for(auto& j : edges)
        {
          std::cout << "  EDGE " << j << std::endl;
          auto verts(m_fine.get_adjacent_polytopes(pl_edge, pl_vertex, j));
          for(auto& k : verts)
          {
            std::cout << "    VERTEX " << k << std::endl;
            std::cout << "      (" << attrs.at(0).at(k) << " , " << attrs.at(1).at(k) << ")" << std::endl;
          }
        }
      }*/

      //MeshUtil::establish_iz_property(m_fine, attrs.at(0), attrs.at(1));
      /*std::cout << "AFTER!" << std::endl;
      for(Index i(0) ; i < m_fine.num_polytopes(pl_face) ; ++i)
      {
        std::cout << "FACE " << i << std::endl;
        auto edges(m_fine.get_adjacent_polytopes(pl_face, pl_edge, i));
        auto outverts(m_fine.get_adjacent_polytopes(pl_face, pl_vertex, i));
        std::cout << "v_fi:" << std::endl;
        for(auto& v : outverts)
          std::cout << v << std::endl;

        for(auto& j : edges)
        {
          std::cout << "  EDGE " << j << std::endl;
          auto verts(m_fine.get_adjacent_polytopes(pl_edge, pl_vertex, j));
          for(auto& k : verts)
          {
            std::cout << "    VERTEX " << k << std::endl;
            std::cout << "      (" << attrs.at(0).at(k) << " , " << attrs.at(1).at(k) << ")" << std::endl;
          }
        }
      }*/
/*      TEST_CHECK(MeshUtil::iz_property(m_fine, attrs.at(0), attrs.at(1), attrs.at(2)));

      //after partitioning
      Foundation::Mesh<Dim3D, Foundation::Topology<IndexType_, OT_, IT_>, OT_> m_part(m2);
      OT_<Halo<0, PLFace, Mesh<Dim3D, Topology<IndexType_, OT_, IT_>, OT_>, double, OT_>, std::allocator<Halo<0, PLFace, Mesh<Dim3D, Topology<IndexType_, OT_, IT_>, OT_>, double, OT_> > > boundaries;
      boundaries.push_back(Halo<0, PLFace, Mesh<Dim3D, Topology<IndexType_, OT_, IT_>, OT_>, double, OT_>(m));
      boundaries.push_back(Halo<0, PLFace, Mesh<Dim3D, Topology<IndexType_, OT_, IT_>, OT_>, double, OT_>(m));
      boundaries.push_back(Halo<0, PLFace, Mesh<Dim3D, Topology<IndexType_, OT_, IT_>, OT_>, double, OT_>(m));
      boundaries.push_back(Halo<0, PLFace, Mesh<Dim3D, Topology<IndexType_, OT_, IT_>, OT_>, double, OT_>(m));
      boundaries.at(0).push_back(0);
      boundaries.at(1).push_back(1);
      boundaries.at(2).push_back(2);
      boundaries.at(3).push_back(3);

      Index num_procs(4);
      Index rank(0);

      PData<Dim3D, Topology<IndexType_, OT_, IT_>, OT_, Mesh, double> p0(Partitioning<Tag_,
                                                                                      Dim3D,
                                                                                      0,
                                                                                      pl_vertex>::execute(m,
                                                                                                          boundaries,
                                                                                                          num_procs, rank,
                                                                                                          attrs
                                                                                                          ));


      TEST_CHECK(MeshUtil::iz_property(p0.basemesh, attrs.at(0), attrs.at(1)));
      //MeshUtil::establish_iz_property(p0.basemesh, attrs.at(0), attrs.at(1));
      TEST_CHECK(MeshUtil::iz_property(p0.basemesh, attrs.at(0), attrs.at(1)));
      //MeshUtil::establish_iz_property(*((Mesh<Dim2D, Topology<IndexType_, OT_, IT_>, OT_>*)(p0.submesh.get())), *( (Attribute<double, OT_>*)(p0.attrs.at(0).get()) ), *( (Attribute<double, OT_>*)(p0.attrs.at(1).get()) ));
      TEST_CHECK(MeshUtil::iz_property(*((Mesh<Dim2D, Topology<IndexType_, OT_, IT_>, OT_>*)(p0.submesh.get())), *( (Attribute<double, OT_>*)(p0.attrs.at(0).get()) ), *( (Attribute<double, OT_>*)(p0.attrs.at(1).get()) )));*/
    }
};
MeshUtilTest3D<Mem::Main, Index, std::vector, std::vector<Index> > mu_test_cpu_v_v_3d("std::vector, std::vector");
/*MeshUtilTest3D<Mem::Main, Index, std::vector, std::deque<Index> > mu_test_cpu_v_d_3d("std::vector, std::deque");
MeshUtilTest3D<Mem::Main, Index, std::deque, std::vector<Index> > mu_test_cpu_d_v_3d("std::deque, std::vector");
MeshUtilTest3D<Mem::Main, Index, std::deque, std::deque<Index> > mu_test_cpu_d_d_3d("std::deque, std::deque");*/



template<typename Tag_, typename IndexType_, template<typename, typename> class OT_, typename IT_>
class MeshUtilTest3D_hexa:
  public TaggedTest<Tag_, IndexType_>
{
  public:
    MeshUtilTest3D_hexa(const std::string & tag) :
      TaggedTest<Tag_, Index>("MeshUtilTest3D_hexa<" + tag + ">")
    {
    }

    virtual void run() const
    {
      /*
       * (0,1,1)  (1,1,1)
       *      *----*
       *     /    /|
       *(0,1,0)(1,1,0)
       *   *----*  *(1,0,1)
       *   | /  | /
       *   |/   |/
       *   *----*
       *(0,0,0) (1,0,0)
       */

      //create attributes for vertex coords
      OT_<Attribute<double, OT_>, std::allocator<Attribute<double, OT_> > > attrs;
      attrs.push_back(Attribute<double, OT_>()); //vertex x-coords
      attrs.push_back(Attribute<double, OT_>()); //vertex y-coords
      attrs.push_back(Attribute<double, OT_>()); //vertex z-coords

      attrs.at(0).get_data().push_back(double(0));
      attrs.at(1).get_data().push_back(double(0));
      attrs.at(2).get_data().push_back(double(0));

      attrs.at(0).get_data().push_back(double(1));
      attrs.at(1).get_data().push_back(double(0));
      attrs.at(2).get_data().push_back(double(0));

      attrs.at(0).get_data().push_back(double(0));
      attrs.at(1).get_data().push_back(double(1));
      attrs.at(2).get_data().push_back(double(0));

      attrs.at(0).get_data().push_back(double(1));
      attrs.at(1).get_data().push_back(double(1));
      attrs.at(2).get_data().push_back(double(0));

      attrs.at(0).get_data().push_back(double(1));
      attrs.at(1).get_data().push_back(double(0));
      attrs.at(2).get_data().push_back(double(1));

      attrs.at(0).get_data().push_back(double(1));
      attrs.at(1).get_data().push_back(double(1));
      attrs.at(2).get_data().push_back(double(1));

      attrs.at(0).get_data().push_back(double(0));
      attrs.at(1).get_data().push_back(double(0));
      attrs.at(2).get_data().push_back(double(1));

      attrs.at(0).get_data().push_back(double(0));
      attrs.at(1).get_data().push_back(double(1));
      attrs.at(2).get_data().push_back(double(1));

      /*  2    3
       *  *-1--*
       *  2    |
       *  |    3
       *  *--0-*
       *  0    1
       */

      //creating foundation mesh
      Foundation::Mesh<Dim3D, Foundation::Topology<IndexType_, OT_, IT_>, OT_> m(0);
      m.add_polytope(pl_vertex);
      m.add_polytope(pl_vertex);
      m.add_polytope(pl_vertex);
      m.add_polytope(pl_vertex);
      m.add_polytope(pl_vertex);
      m.add_polytope(pl_vertex);
      m.add_polytope(pl_vertex);
      m.add_polytope(pl_vertex);

      m.add_polytope(pl_edge);
      m.add_polytope(pl_edge);
      m.add_polytope(pl_edge);
      m.add_polytope(pl_edge);
      m.add_polytope(pl_edge);
      m.add_polytope(pl_edge);
      m.add_polytope(pl_edge);
      m.add_polytope(pl_edge);
      m.add_polytope(pl_edge);
      m.add_polytope(pl_edge);
      m.add_polytope(pl_edge);
      m.add_polytope(pl_edge);

      m.add_polytope(pl_face);
      m.add_polytope(pl_face);
      m.add_polytope(pl_face);
      m.add_polytope(pl_face);
      m.add_polytope(pl_face);
      m.add_polytope(pl_face);

      m.add_polytope(pl_polyhedron);

      //vertex 0
      m.add_adjacency(pl_vertex, pl_edge, 0, 0);
      m.add_adjacency(pl_vertex, pl_edge, 0, 2);
      m.add_adjacency(pl_vertex, pl_edge, 0, 10);

      m.add_adjacency(pl_vertex, pl_face, 0, 0);
      m.add_adjacency(pl_vertex, pl_face, 0, 3);
      m.add_adjacency(pl_vertex, pl_face, 0, 4);

      m.add_adjacency(pl_vertex, pl_polyhedron, 0, 0);

      //vertex 1
      m.add_adjacency(pl_vertex, pl_edge, 1, 0);
      m.add_adjacency(pl_vertex, pl_edge, 1, 3);
      m.add_adjacency(pl_vertex, pl_edge, 1, 4);

      m.add_adjacency(pl_vertex, pl_face, 1, 0);
      m.add_adjacency(pl_vertex, pl_face, 1, 2);
      m.add_adjacency(pl_vertex, pl_face, 1, 4);

      m.add_adjacency(pl_vertex, pl_polyhedron, 1, 0);

      //vertex 2
      m.add_adjacency(pl_vertex, pl_edge, 2, 1);
      m.add_adjacency(pl_vertex, pl_edge, 2, 2);
      m.add_adjacency(pl_vertex, pl_edge, 2, 11);

      m.add_adjacency(pl_vertex, pl_face, 2, 0);
      m.add_adjacency(pl_vertex, pl_face, 2, 3);
      m.add_adjacency(pl_vertex, pl_face, 2, 5);

      m.add_adjacency(pl_vertex, pl_polyhedron, 2, 0);

      //vertex 3
      m.add_adjacency(pl_vertex, pl_edge, 3, 1);
      m.add_adjacency(pl_vertex, pl_edge, 3, 3);
      m.add_adjacency(pl_vertex, pl_edge, 3, 5);

      m.add_adjacency(pl_vertex, pl_face, 3, 2);
      m.add_adjacency(pl_vertex, pl_face, 3, 0);
      m.add_adjacency(pl_vertex, pl_face, 3, 5);

      m.add_adjacency(pl_vertex, pl_polyhedron, 3, 0);

      //vertex 4
      m.add_adjacency(pl_vertex, pl_edge, 4, 4);
      m.add_adjacency(pl_vertex, pl_edge, 4, 6);
      m.add_adjacency(pl_vertex, pl_edge, 4, 7);

      m.add_adjacency(pl_vertex, pl_face, 4, 1);
      m.add_adjacency(pl_vertex, pl_face, 4, 2);
      m.add_adjacency(pl_vertex, pl_face, 4, 4);

      m.add_adjacency(pl_vertex, pl_polyhedron, 4, 0);

      //vertex 5
      m.add_adjacency(pl_vertex, pl_edge, 5, 5);
      m.add_adjacency(pl_vertex, pl_edge, 5, 6);
      m.add_adjacency(pl_vertex, pl_edge, 5, 8);

      m.add_adjacency(pl_vertex, pl_face, 5, 1);
      m.add_adjacency(pl_vertex, pl_face, 5, 2);
      m.add_adjacency(pl_vertex, pl_face, 5, 5);

      m.add_adjacency(pl_vertex, pl_polyhedron, 5, 0);

      //vertex 6
      m.add_adjacency(pl_vertex, pl_edge, 6, 7);
      m.add_adjacency(pl_vertex, pl_edge, 6, 9);
      m.add_adjacency(pl_vertex, pl_edge, 6, 10);

      m.add_adjacency(pl_vertex, pl_face, 6, 1);
      m.add_adjacency(pl_vertex, pl_face, 6, 3);
      m.add_adjacency(pl_vertex, pl_face, 6, 4);

      m.add_adjacency(pl_vertex, pl_polyhedron, 6, 0);

      //vertex 7
      m.add_adjacency(pl_vertex, pl_edge, 7, 8);
      m.add_adjacency(pl_vertex, pl_edge, 7, 9);
      m.add_adjacency(pl_vertex, pl_edge, 7, 11);

      m.add_adjacency(pl_vertex, pl_face, 7, 1);
      m.add_adjacency(pl_vertex, pl_face, 7, 3);
      m.add_adjacency(pl_vertex, pl_face, 7, 5);

      m.add_adjacency(pl_vertex, pl_polyhedron, 7, 0);

      TEST_CHECK(!MeshUtil::iz_property_hexa(m, attrs.at(0), attrs.at(1), attrs.at(2)));
      MeshUtil::establish_iz_property_hexa(m, attrs.at(0), attrs.at(1), attrs.at(2));
      TEST_CHECK(MeshUtil::iz_property_hexa(m, attrs.at(0), attrs.at(1), attrs.at(2)));

      //after refinement
      Foundation::Mesh<Dim3D, Foundation::Topology<IndexType_, OT_, IT_>, OT_> m_fine(m);

      OT_<std::shared_ptr<HaloBase<Mesh<Dim3D, Topology<IndexType_, OT_, IT_>, OT_>, double, OT_> >, std::allocator<std::shared_ptr<HaloBase<Mesh<Dim3D, Topology<IndexType_, OT_, IT_>, OT_>, double, OT_> > > > halos;

      Refinement<Mem::Main,
                 mrt_standard>::execute(m_fine, &halos, attrs);

      MeshUtil::establish_iz_property_hexa(m_fine, attrs.at(0), attrs.at(1), attrs.at(2));
      TEST_CHECK(MeshUtil::iz_property_hexa(m_fine, attrs.at(0), attrs.at(1), attrs.at(2)));
      /*std::cout << "BEFORE!" << std::endl;
      for(Index i(0) ; i < m_fine.num_polytopes(pl_face) ; ++i)
      {
        std::cout << "FACE " << i << std::endl;
        auto edges(m_fine.get_adjacent_polytopes(pl_face, pl_edge, i));
        auto outverts(m_fine.get_adjacent_polytopes(pl_face, pl_vertex, i));
        std::cout << "v_fi:" << std::endl;
        for(auto& v : outverts)
          std::cout << v << std::endl;

        for(auto& j : edges)
        {
          std::cout << "  EDGE " << j << std::endl;
          auto verts(m_fine.get_adjacent_polytopes(pl_edge, pl_vertex, j));
          for(auto& k : verts)
          {
            std::cout << "    VERTEX " << k << std::endl;
            std::cout << "      (" << attrs.at(0).at(k) << " , " << attrs.at(1).at(k) << ")" << std::endl;
          }
        }
      }*/

      //MeshUtil::establish_iz_property_quad(m_fine, attrs.at(0), attrs.at(1));
      /*std::cout << "AFTER!" << std::endl;
      for(Index i(0) ; i < m_fine.num_polytopes(pl_face) ; ++i)
      {
        std::cout << "FACE " << i << std::endl;
        auto edges(m_fine.get_adjacent_polytopes(pl_face, pl_edge, i));
        auto outverts(m_fine.get_adjacent_polytopes(pl_face, pl_vertex, i));
        std::cout << "v_fi:" << std::endl;
        for(auto& v : outverts)
          std::cout << v << std::endl;

        for(auto& j : edges)
        {
          std::cout << "  EDGE " << j << std::endl;
          auto verts(m_fine.get_adjacent_polytopes(pl_edge, pl_vertex, j));
          for(auto& k : verts)
          {
            std::cout << "    VERTEX " << k << std::endl;
            std::cout << "      (" << attrs.at(0).at(k) << " , " << attrs.at(1).at(k) << ")" << std::endl;
          }
        }
      }*/
/*      TEST_CHECK(MeshUtil::iz_property_quad(m_fine, attrs.at(0), attrs.at(1), attrs.at(2)));

      //after partitioning
      Foundation::Mesh<Dim3D, Foundation::Topology<IndexType_, OT_, IT_>, OT_> m_part(m2);
      OT_<Halo<0, PLFace, Mesh<Dim3D, Topology<IndexType_, OT_, IT_>, OT_>, double, OT_>, std::allocator<Halo<0, PLFace, Mesh<Dim3D, Topology<IndexType_, OT_, IT_>, OT_>, double, OT_> > > boundaries;
      boundaries.push_back(Halo<0, PLFace, Mesh<Dim3D, Topology<IndexType_, OT_, IT_>, OT_>, double, OT_>(m));
      boundaries.push_back(Halo<0, PLFace, Mesh<Dim3D, Topology<IndexType_, OT_, IT_>, OT_>, double, OT_>(m));
      boundaries.push_back(Halo<0, PLFace, Mesh<Dim3D, Topology<IndexType_, OT_, IT_>, OT_>, double, OT_>(m));
      boundaries.push_back(Halo<0, PLFace, Mesh<Dim3D, Topology<IndexType_, OT_, IT_>, OT_>, double, OT_>(m));
      boundaries.at(0).push_back(0);
      boundaries.at(1).push_back(1);
      boundaries.at(2).push_back(2);
      boundaries.at(3).push_back(3);

      Index num_procs(4);
      Index rank(0);

      PData<Dim3D, Topology<IndexType_, OT_, IT_>, OT_, Mesh, double> p0(Partitioning<Tag_,
                                                                                      Dim3D,
                                                                                      0,
                                                                                      pl_vertex>::execute(m,
                                                                                                          boundaries,
                                                                                                          num_procs, rank,
                                                                                                          attrs
                                                                                                          ));


      TEST_CHECK(MeshUtil::iz_property_quad(p0.basemesh, attrs.at(0), attrs.at(1)));
      //MeshUtil::establish_iz_property_quad(p0.basemesh, attrs.at(0), attrs.at(1));
      TEST_CHECK(MeshUtil::iz_property_quad(p0.basemesh, attrs.at(0), attrs.at(1)));
      //MeshUtil::establish_iz_property_quad(*((Mesh<Dim2D, Topology<IndexType_, OT_, IT_>, OT_>*)(p0.submesh.get())), *( (Attribute<double, OT_>*)(p0.attrs.at(0).get()) ), *( (Attribute<double, OT_>*)(p0.attrs.at(1).get()) ));
      TEST_CHECK(MeshUtil::iz_property_quad(*((Mesh<Dim2D, Topology<IndexType_, OT_, IT_>, OT_>*)(p0.submesh.get())), *( (Attribute<double, OT_>*)(p0.attrs.at(0).get()) ), *( (Attribute<double, OT_>*)(p0.attrs.at(1).get()) )));*/
    }
};
MeshUtilTest3D_hexa<Mem::Main, Index, std::vector, std::vector<Index> > mu_test_cpu_v_v_3d_hexa("std::vector, std::vector");
/*MeshUtilTest3D_hexa<Mem::Main, Index, std::vector, std::deque<Index> > mu_test_cpu_v_d_3d_hexa("std::vector, std::deque");
MeshUtilTest3D_hexa<Mem::Main, Index, std::deque, std::vector<Index> > mu_test_cpu_d_v_3d_hexa("std::deque, std::vector");
MeshUtilTest3D_hexa<Mem::Main, Index, std::deque, std::deque<Index> > mu_test_cpu_d_d_3d_hexa("std::deque, std::deque");*/



template<typename Tag_, typename IndexType_, template<typename, typename> class OT_, typename IT_>
class MeshUtilTest3D_tetra:
  public TaggedTest<Tag_, IndexType_>
{
  public:
    MeshUtilTest3D_tetra(const std::string & tag) :
      TaggedTest<Tag_, Index>("MeshUtilTest3D_tetra<" + tag + ">")
    {
    }

    virtual void run() const
    {
      /*
       * (0,1,1)  (1,1,1)
       *      *----*
       *     /    /|
       *(0,1,0)(1,1,0)
       *   *----*  *(1,0,1)
       *   | /  | /
       *   |/   |/
       *   *----*
       *(0,0,0) (1,0,0)
       */

      //create attributes for vertex coords
      OT_<Attribute<double, OT_>, std::allocator<Attribute<double, OT_> > > attrs;
      attrs.push_back(Attribute<double, OT_>()); //vertex x-coords
      attrs.push_back(Attribute<double, OT_>()); //vertex y-coords
      attrs.push_back(Attribute<double, OT_>()); //vertex z-coords

      attrs.at(0).get_data().push_back(double(0));
      attrs.at(1).get_data().push_back(double(0));
      attrs.at(2).get_data().push_back(double(0));

      attrs.at(0).get_data().push_back(double(1));
      attrs.at(1).get_data().push_back(double(0));
      attrs.at(2).get_data().push_back(double(-sqrt(3)));

      attrs.at(0).get_data().push_back(double(2));
      attrs.at(1).get_data().push_back(double(0));
      attrs.at(2).get_data().push_back(double(0));

      attrs.at(0).get_data().push_back(double(1));
      attrs.at(1).get_data().push_back(double(2*sqrt(2/3)));
      attrs.at(2).get_data().push_back(double(-sqrt(3)/3));

      attrs.at(0).get_data().push_back(double(0.5));
      attrs.at(1).get_data().push_back(double(0));
      attrs.at(2).get_data().push_back(double(-sqrt(3)/2));

      attrs.at(0).get_data().push_back(double(1));
      attrs.at(1).get_data().push_back(double(0));
      attrs.at(2).get_data().push_back(double(1));

      attrs.at(0).get_data().push_back(double(0.5));
      attrs.at(1).get_data().push_back(double(sqrt(2/3)));
      attrs.at(2).get_data().push_back(double(-sqrt(3)/6));

      attrs.at(0).get_data().push_back(double(1.5));
      attrs.at(1).get_data().push_back(double(0));
      attrs.at(2).get_data().push_back(double(-sqrt(3)/2));

      attrs.at(0).get_data().push_back(double(1));
      attrs.at(1).get_data().push_back(double(sqrt(2/3)));
      attrs.at(2).get_data().push_back(double(-sqrt(3)/3*2));

      attrs.at(0).get_data().push_back(double(1.5));
      attrs.at(1).get_data().push_back(double(sqrt(2/3)));
      attrs.at(2).get_data().push_back(double(-sqrt(3)/6));

      attrs.at(0).get_data().push_back(double(1));
      attrs.at(1).get_data().push_back(double(sqrt(2/3)));
      attrs.at(2).get_data().push_back(double(-sqrt(3)/3));

      /*  2    3
       *  *-1--*
       *  2    |
       *  |    3
       *  *--0-*
       *  0    1
       */

      //creating foundation mesh
      Foundation::Mesh<Dim3D, Foundation::Topology<IndexType_, OT_, IT_>, OT_> m(0);
      m.add_polytope(pl_vertex);
      m.add_polytope(pl_vertex);
      m.add_polytope(pl_vertex);
      m.add_polytope(pl_vertex);

      m.add_polytope(pl_edge);
      m.add_polytope(pl_edge);
      m.add_polytope(pl_edge);
      m.add_polytope(pl_edge);
      m.add_polytope(pl_edge);
      m.add_polytope(pl_edge);

      m.add_polytope(pl_face);
      m.add_polytope(pl_face);
      m.add_polytope(pl_face);
      m.add_polytope(pl_face);

      m.add_polytope(pl_polyhedron);

      m.add_adjacency(pl_vertex, pl_edge, 1, 0);
      m.add_adjacency(pl_vertex, pl_edge, 1, 3);
      m.add_adjacency(pl_vertex, pl_edge, 1, 4);

      m.add_adjacency(pl_vertex, pl_face, 1, 0);
      m.add_adjacency(pl_vertex, pl_face, 1, 2);
      m.add_adjacency(pl_vertex, pl_face, 1, 3);

      m.add_adjacency(pl_vertex, pl_polyhedron, 1, 0);

      m.add_adjacency(pl_vertex, pl_edge, 0, 0);
      m.add_adjacency(pl_vertex, pl_edge, 0, 1);
      m.add_adjacency(pl_vertex, pl_edge, 0, 2);

      m.add_adjacency(pl_vertex, pl_face, 0, 3);
      m.add_adjacency(pl_vertex, pl_face, 0, 1);

      m.add_adjacency(pl_vertex, pl_polyhedron, 0, 0);

      m.add_adjacency(pl_vertex, pl_edge, 2, 1);
      m.add_adjacency(pl_vertex, pl_edge, 2, 3);
      m.add_adjacency(pl_vertex, pl_edge, 2, 5);

      m.add_adjacency(pl_vertex, pl_face, 2, 0);
      m.add_adjacency(pl_vertex, pl_face, 2, 1);
      m.add_adjacency(pl_vertex, pl_face, 2, 3);

      m.add_adjacency(pl_vertex, pl_polyhedron, 2, 0);

      m.add_adjacency(pl_vertex, pl_edge, 3, 2);
      m.add_adjacency(pl_vertex, pl_edge, 3, 4);
      m.add_adjacency(pl_vertex, pl_edge, 3, 5);

      m.add_adjacency(pl_vertex, pl_face, 3, 0);
      m.add_adjacency(pl_vertex, pl_face, 3, 1);
      m.add_adjacency(pl_vertex, pl_face, 3, 2);

      m.add_adjacency(pl_vertex, pl_face, 0, 2);

      m.add_adjacency(pl_vertex, pl_polyhedron, 3, 0);

      TEST_CHECK(!MeshUtil::property_tetra(m));
      MeshUtil::establish_property_tetra(m);
      TEST_CHECK(MeshUtil::property_tetra(m));

      //creating foundation mesh
      Foundation::Mesh<Dim3D, Foundation::Topology<IndexType_, OT_, IT_>, OT_> m_fine(0);
      m_fine.add_polytope(pl_vertex);
      m_fine.add_polytope(pl_vertex);
      m_fine.add_polytope(pl_vertex);
      m_fine.add_polytope(pl_vertex);
      m_fine.add_polytope(pl_vertex);
      m_fine.add_polytope(pl_vertex);
      m_fine.add_polytope(pl_vertex);
      m_fine.add_polytope(pl_vertex);
      m_fine.add_polytope(pl_vertex);
      m_fine.add_polytope(pl_vertex);
      m_fine.add_polytope(pl_vertex);

      m_fine.add_polytope(pl_edge);
      m_fine.add_polytope(pl_edge);
      m_fine.add_polytope(pl_edge);
      m_fine.add_polytope(pl_edge);
      m_fine.add_polytope(pl_edge);
      m_fine.add_polytope(pl_edge);
      m_fine.add_polytope(pl_edge);
      m_fine.add_polytope(pl_edge);
      m_fine.add_polytope(pl_edge);
      m_fine.add_polytope(pl_edge);
      m_fine.add_polytope(pl_edge);
      m_fine.add_polytope(pl_edge);
      m_fine.add_polytope(pl_edge);
      m_fine.add_polytope(pl_edge);
      m_fine.add_polytope(pl_edge);
      m_fine.add_polytope(pl_edge);
      m_fine.add_polytope(pl_edge);
      m_fine.add_polytope(pl_edge);
      m_fine.add_polytope(pl_edge);
      m_fine.add_polytope(pl_edge);
      m_fine.add_polytope(pl_edge);
      m_fine.add_polytope(pl_edge);
      m_fine.add_polytope(pl_edge);
      m_fine.add_polytope(pl_edge);
      m_fine.add_polytope(pl_edge);
      m_fine.add_polytope(pl_edge);
      m_fine.add_polytope(pl_edge);
      m_fine.add_polytope(pl_edge);
      m_fine.add_polytope(pl_edge);
      m_fine.add_polytope(pl_edge);

      m_fine.add_polytope(pl_face);
      m_fine.add_polytope(pl_face);
      m_fine.add_polytope(pl_face);
      m_fine.add_polytope(pl_face);
      m_fine.add_polytope(pl_face);
      m_fine.add_polytope(pl_face);
      m_fine.add_polytope(pl_face);
      m_fine.add_polytope(pl_face);
      m_fine.add_polytope(pl_face);
      m_fine.add_polytope(pl_face);
      m_fine.add_polytope(pl_face);
      m_fine.add_polytope(pl_face);
      m_fine.add_polytope(pl_face);
      m_fine.add_polytope(pl_face);
      m_fine.add_polytope(pl_face);
      m_fine.add_polytope(pl_face);
      m_fine.add_polytope(pl_face);
      m_fine.add_polytope(pl_face);
      m_fine.add_polytope(pl_face);
      m_fine.add_polytope(pl_face);
      m_fine.add_polytope(pl_face);
      m_fine.add_polytope(pl_face);
      m_fine.add_polytope(pl_face);
      m_fine.add_polytope(pl_face);
      m_fine.add_polytope(pl_face);
      m_fine.add_polytope(pl_face);
      m_fine.add_polytope(pl_face);
      m_fine.add_polytope(pl_face);
      m_fine.add_polytope(pl_face);
      m_fine.add_polytope(pl_face);
      m_fine.add_polytope(pl_face);
      m_fine.add_polytope(pl_face);

      m_fine.add_polytope(pl_polyhedron);
      m_fine.add_polytope(pl_polyhedron);
      m_fine.add_polytope(pl_polyhedron);
      m_fine.add_polytope(pl_polyhedron);
      m_fine.add_polytope(pl_polyhedron);
      m_fine.add_polytope(pl_polyhedron);
      m_fine.add_polytope(pl_polyhedron);
      m_fine.add_polytope(pl_polyhedron);
      m_fine.add_polytope(pl_polyhedron);
      m_fine.add_polytope(pl_polyhedron);
      m_fine.add_polytope(pl_polyhedron);
      m_fine.add_polytope(pl_polyhedron);

      //vertex 0
      m_fine.add_adjacency(pl_vertex, pl_edge, 0, 0);
      m_fine.add_adjacency(pl_vertex, pl_edge, 0, 2);
      m_fine.add_adjacency(pl_vertex, pl_edge, 0, 4);

      m_fine.add_adjacency(pl_vertex, pl_face, 0, 4);
      m_fine.add_adjacency(pl_vertex, pl_face, 0, 8);
      m_fine.add_adjacency(pl_vertex, pl_face, 0, 12);

      m_fine.add_adjacency(pl_vertex, pl_polyhedron, 0, 0);

      //vertex 1
      m_fine.add_adjacency(pl_vertex, pl_edge, 1, 1);
      m_fine.add_adjacency(pl_vertex, pl_edge, 1, 6);
      m_fine.add_adjacency(pl_vertex, pl_edge, 1, 8);

      m_fine.add_adjacency(pl_vertex, pl_face, 1, 0);
      m_fine.add_adjacency(pl_vertex, pl_face, 1, 9);
      m_fine.add_adjacency(pl_vertex, pl_face, 1, 13);

      m_fine.add_adjacency(pl_vertex, pl_polyhedron, 1, 2);

      //vertex 2
      m_fine.add_adjacency(pl_vertex, pl_edge, 2, 3);
      m_fine.add_adjacency(pl_vertex, pl_edge, 2, 7);
      m_fine.add_adjacency(pl_vertex, pl_edge, 2, 10);

      m_fine.add_adjacency(pl_vertex, pl_face, 2, 1);
      m_fine.add_adjacency(pl_vertex, pl_face, 2, 5);
      m_fine.add_adjacency(pl_vertex, pl_face, 2, 14);

      m_fine.add_adjacency(pl_vertex, pl_polyhedron, 2, 4);

      //vertex 3
      m_fine.add_adjacency(pl_vertex, pl_edge, 3, 5);
      m_fine.add_adjacency(pl_vertex, pl_edge, 3, 9);
      m_fine.add_adjacency(pl_vertex, pl_edge, 3, 11);

      m_fine.add_adjacency(pl_vertex, pl_face, 3, 2);
      m_fine.add_adjacency(pl_vertex, pl_face, 3, 6);
      m_fine.add_adjacency(pl_vertex, pl_face, 3, 10);

      m_fine.add_adjacency(pl_vertex, pl_polyhedron, 3, 6);

      //vertex 4
      m_fine.add_adjacency(pl_vertex, pl_face, 4, 15);
      m_fine.add_adjacency(pl_vertex, pl_face, 4, 11);
      m_fine.add_adjacency(pl_vertex, pl_face, 4, 13);
      m_fine.add_adjacency(pl_vertex, pl_face, 4, 9);
      m_fine.add_adjacency(pl_vertex, pl_face, 4, 23);
      m_fine.add_adjacency(pl_vertex, pl_face, 4, 22);
      m_fine.add_adjacency(pl_vertex, pl_face, 4, 17);
      m_fine.add_adjacency(pl_vertex, pl_face, 4, 8);
      m_fine.add_adjacency(pl_vertex, pl_face, 4, 12);
      m_fine.add_adjacency(pl_vertex, pl_face, 4, 16);
      m_fine.add_adjacency(pl_vertex, pl_face, 4, 21);
      m_fine.add_adjacency(pl_vertex, pl_face, 4, 20);

      m_fine.add_adjacency(pl_vertex, pl_polyhedron, 4, 0);
      m_fine.add_adjacency(pl_vertex, pl_polyhedron, 4, 1);
      m_fine.add_adjacency(pl_vertex, pl_polyhedron, 4, 2);
      m_fine.add_adjacency(pl_vertex, pl_polyhedron, 4, 3);
      m_fine.add_adjacency(pl_vertex, pl_polyhedron, 4, 10);
      m_fine.add_adjacency(pl_vertex, pl_polyhedron, 4, 11);


      //vertex 5
      m_fine.add_adjacency(pl_vertex, pl_edge, 5, 2);
      m_fine.add_adjacency(pl_vertex, pl_edge, 5, 15);
      m_fine.add_adjacency(pl_vertex, pl_edge, 5, 21);
      m_fine.add_adjacency(pl_vertex, pl_edge, 5, 25);
      m_fine.add_adjacency(pl_vertex, pl_edge, 5, 23);
      m_fine.add_adjacency(pl_vertex, pl_edge, 5, 16);
      m_fine.add_adjacency(pl_vertex, pl_edge, 5, 3);

      m_fine.add_adjacency(pl_vertex, pl_face, 5, 15);
      m_fine.add_adjacency(pl_vertex, pl_face, 5, 7);
      m_fine.add_adjacency(pl_vertex, pl_face, 5, 14);
      m_fine.add_adjacency(pl_vertex, pl_face, 5, 5);
      m_fine.add_adjacency(pl_vertex, pl_face, 5, 18);
      m_fine.add_adjacency(pl_vertex, pl_face, 5, 26);
      m_fine.add_adjacency(pl_vertex, pl_face, 5, 12);
      m_fine.add_adjacency(pl_vertex, pl_face, 5, 4);
      m_fine.add_adjacency(pl_vertex, pl_face, 5, 24);
      m_fine.add_adjacency(pl_vertex, pl_face, 5, 20);
      m_fine.add_adjacency(pl_vertex, pl_face, 5, 25);
      m_fine.add_adjacency(pl_vertex, pl_face, 5, 16);

      m_fine.add_adjacency(pl_vertex, pl_polyhedron, 5, 0);
      m_fine.add_adjacency(pl_vertex, pl_polyhedron, 5, 1);
      m_fine.add_adjacency(pl_vertex, pl_polyhedron, 5, 4);
      m_fine.add_adjacency(pl_vertex, pl_polyhedron, 5, 5);
      m_fine.add_adjacency(pl_vertex, pl_polyhedron, 5, 9);
      m_fine.add_adjacency(pl_vertex, pl_polyhedron, 5, 11);

      //vertex 6
      m_fine.add_adjacency(pl_vertex, pl_edge, 6, 4);
      m_fine.add_adjacency(pl_vertex, pl_edge, 6, 15);
      m_fine.add_adjacency(pl_vertex, pl_edge, 6, 18);
      m_fine.add_adjacency(pl_vertex, pl_edge, 6, 26);
      m_fine.add_adjacency(pl_vertex, pl_edge, 6, 5);
      m_fine.add_adjacency(pl_vertex, pl_edge, 6, 17);
      m_fine.add_adjacency(pl_vertex, pl_edge, 6, 20);

      m_fine.add_adjacency(pl_vertex, pl_face, 6, 11);
      m_fine.add_adjacency(pl_vertex, pl_face, 6, 7);
      m_fine.add_adjacency(pl_vertex, pl_face, 6, 28);
      m_fine.add_adjacency(pl_vertex, pl_face, 6, 27);
      m_fine.add_adjacency(pl_vertex, pl_face, 6, 6);
      m_fine.add_adjacency(pl_vertex, pl_face, 6, 19);
      m_fine.add_adjacency(pl_vertex, pl_face, 6, 10);
      m_fine.add_adjacency(pl_vertex, pl_face, 6, 8);
      m_fine.add_adjacency(pl_vertex, pl_face, 6, 4);
      m_fine.add_adjacency(pl_vertex, pl_face, 6, 24);
      m_fine.add_adjacency(pl_vertex, pl_face, 6, 21);
      m_fine.add_adjacency(pl_vertex, pl_face, 6, 16);

      m_fine.add_adjacency(pl_vertex, pl_polyhedron, 6, 0);
      m_fine.add_adjacency(pl_vertex, pl_polyhedron, 6, 1);
      m_fine.add_adjacency(pl_vertex, pl_polyhedron, 6, 6);
      m_fine.add_adjacency(pl_vertex, pl_polyhedron, 6, 7);
      m_fine.add_adjacency(pl_vertex, pl_polyhedron, 6, 9);
      m_fine.add_adjacency(pl_vertex, pl_polyhedron, 6, 10);

      //vertex 7
      m_fine.add_adjacency(pl_vertex, pl_edge, 7, 22);
      m_fine.add_adjacency(pl_vertex, pl_edge, 7, 12);
      m_fine.add_adjacency(pl_vertex, pl_edge, 7, 27);
      m_fine.add_adjacency(pl_vertex, pl_edge, 7, 6);
      m_fine.add_adjacency(pl_vertex, pl_edge, 7, 23);
      m_fine.add_adjacency(pl_vertex, pl_edge, 7, 13);
      m_fine.add_adjacency(pl_vertex, pl_edge, 7, 7);

      m_fine.add_adjacency(pl_vertex, pl_face, 7, 15);
      m_fine.add_adjacency(pl_vertex, pl_face, 7, 3);
      m_fine.add_adjacency(pl_vertex, pl_face, 7, 14);
      m_fine.add_adjacency(pl_vertex, pl_face, 7, 1);
      m_fine.add_adjacency(pl_vertex, pl_face, 7, 25);
      m_fine.add_adjacency(pl_vertex, pl_face, 7, 30);
      m_fine.add_adjacency(pl_vertex, pl_face, 7, 18);
      m_fine.add_adjacency(pl_vertex, pl_face, 7, 13);
      m_fine.add_adjacency(pl_vertex, pl_face, 7, 0);
      m_fine.add_adjacency(pl_vertex, pl_face, 7, 29);
      m_fine.add_adjacency(pl_vertex, pl_face, 7, 22);
      m_fine.add_adjacency(pl_vertex, pl_face, 7, 17);

      m_fine.add_adjacency(pl_vertex, pl_polyhedron, 7, 2);
      m_fine.add_adjacency(pl_vertex, pl_polyhedron, 7, 3);
      m_fine.add_adjacency(pl_vertex, pl_polyhedron, 7, 4);
      m_fine.add_adjacency(pl_vertex, pl_polyhedron, 7, 5);
      m_fine.add_adjacency(pl_vertex, pl_polyhedron, 7, 8);
      m_fine.add_adjacency(pl_vertex, pl_polyhedron, 7, 11);

      //vertex 8
      m_fine.add_adjacency(pl_vertex, pl_edge, 8, 8);
      m_fine.add_adjacency(pl_vertex, pl_edge, 8, 9);
      m_fine.add_adjacency(pl_vertex, pl_edge, 8, 12);
      m_fine.add_adjacency(pl_vertex, pl_edge, 8, 14);
      m_fine.add_adjacency(pl_vertex, pl_edge, 8, 19);
      m_fine.add_adjacency(pl_vertex, pl_edge, 8, 20);
      m_fine.add_adjacency(pl_vertex, pl_edge, 8, 28);

      m_fine.add_adjacency(pl_vertex, pl_face, 8, 11);
      m_fine.add_adjacency(pl_vertex, pl_face, 8, 27);
      m_fine.add_adjacency(pl_vertex, pl_face, 8, 31);
      m_fine.add_adjacency(pl_vertex, pl_face, 8, 3);
      m_fine.add_adjacency(pl_vertex, pl_face, 8, 19);
      m_fine.add_adjacency(pl_vertex, pl_face, 8, 10);
      m_fine.add_adjacency(pl_vertex, pl_face, 8, 2);
      m_fine.add_adjacency(pl_vertex, pl_face, 8, 9);
      m_fine.add_adjacency(pl_vertex, pl_face, 8, 0);
      m_fine.add_adjacency(pl_vertex, pl_face, 8, 29);
      m_fine.add_adjacency(pl_vertex, pl_face, 8, 23);
      m_fine.add_adjacency(pl_vertex, pl_face, 8, 17);

      m_fine.add_adjacency(pl_vertex, pl_polyhedron, 8, 2);
      m_fine.add_adjacency(pl_vertex, pl_polyhedron, 8, 3);
      m_fine.add_adjacency(pl_vertex, pl_polyhedron, 8, 6);
      m_fine.add_adjacency(pl_vertex, pl_polyhedron, 8, 8);
      m_fine.add_adjacency(pl_vertex, pl_polyhedron, 8, 7);
      m_fine.add_adjacency(pl_vertex, pl_polyhedron, 8, 10);

      //vertex 10
      m_fine.add_adjacency(pl_vertex, pl_edge, 10, 24);
      m_fine.add_adjacency(pl_vertex, pl_edge, 10, 25);
      m_fine.add_adjacency(pl_vertex, pl_edge, 10, 26);
      m_fine.add_adjacency(pl_vertex, pl_edge, 10, 27);
      m_fine.add_adjacency(pl_vertex, pl_edge, 10, 28);
      m_fine.add_adjacency(pl_vertex, pl_edge, 10, 29);

      m_fine.add_adjacency(pl_vertex, pl_face, 10, 30);
      m_fine.add_adjacency(pl_vertex, pl_face, 10, 29);
      m_fine.add_adjacency(pl_vertex, pl_face, 10, 31);
      m_fine.add_adjacency(pl_vertex, pl_face, 10, 28);
      m_fine.add_adjacency(pl_vertex, pl_face, 10, 26);
      m_fine.add_adjacency(pl_vertex, pl_face, 10, 24);
      m_fine.add_adjacency(pl_vertex, pl_face, 10, 21);
      m_fine.add_adjacency(pl_vertex, pl_face, 10, 27);
      m_fine.add_adjacency(pl_vertex, pl_face, 10, 23);
      m_fine.add_adjacency(pl_vertex, pl_face, 10, 25);
      m_fine.add_adjacency(pl_vertex, pl_face, 10, 22);
      m_fine.add_adjacency(pl_vertex, pl_face, 10, 20);

      m_fine.add_adjacency(pl_vertex, pl_polyhedron, 10, 1);
      m_fine.add_adjacency(pl_vertex, pl_polyhedron, 10, 3);
      m_fine.add_adjacency(pl_vertex, pl_polyhedron, 10, 5);
      m_fine.add_adjacency(pl_vertex, pl_polyhedron, 10, 7);
      m_fine.add_adjacency(pl_vertex, pl_polyhedron, 10, 8);
      m_fine.add_adjacency(pl_vertex, pl_polyhedron, 10, 9);
      m_fine.add_adjacency(pl_vertex, pl_polyhedron, 10, 10);
      m_fine.add_adjacency(pl_vertex, pl_polyhedron, 10, 11);

      //vertex 9
      m_fine.add_adjacency(pl_vertex, pl_edge, 9, 10);
      m_fine.add_adjacency(pl_vertex, pl_edge, 9, 11);
      m_fine.add_adjacency(pl_vertex, pl_edge, 9, 13);
      m_fine.add_adjacency(pl_vertex, pl_edge, 9, 14);
      m_fine.add_adjacency(pl_vertex, pl_edge, 9, 16);
      m_fine.add_adjacency(pl_vertex, pl_edge, 9, 17);
      m_fine.add_adjacency(pl_vertex, pl_edge, 9, 29);

      m_fine.add_adjacency(pl_vertex, pl_face, 9, 7);
      m_fine.add_adjacency(pl_vertex, pl_face, 9, 28);
      m_fine.add_adjacency(pl_vertex, pl_face, 9, 31);
      m_fine.add_adjacency(pl_vertex, pl_face, 9, 3);
      m_fine.add_adjacency(pl_vertex, pl_face, 9, 19);
      m_fine.add_adjacency(pl_vertex, pl_face, 9, 6);
      m_fine.add_adjacency(pl_vertex, pl_face, 9, 2);
      m_fine.add_adjacency(pl_vertex, pl_face, 9, 5);
      m_fine.add_adjacency(pl_vertex, pl_face, 9, 1);
      m_fine.add_adjacency(pl_vertex, pl_face, 9, 30);
      m_fine.add_adjacency(pl_vertex, pl_face, 9, 26);
      m_fine.add_adjacency(pl_vertex, pl_face, 9, 18);

      m_fine.add_adjacency(pl_vertex, pl_polyhedron, 9, 4);
      m_fine.add_adjacency(pl_vertex, pl_polyhedron, 9, 5);
      m_fine.add_adjacency(pl_vertex, pl_polyhedron, 9, 6);
      m_fine.add_adjacency(pl_vertex, pl_polyhedron, 9, 7);
      m_fine.add_adjacency(pl_vertex, pl_polyhedron, 9, 8);
      m_fine.add_adjacency(pl_vertex, pl_polyhedron, 9, 9);

      m_fine.add_adjacency(pl_vertex, pl_edge, 4, 0);
      m_fine.add_adjacency(pl_vertex, pl_edge, 4, 21);
      m_fine.add_adjacency(pl_vertex, pl_edge, 4, 18);
      m_fine.add_adjacency(pl_vertex, pl_edge, 4, 24);
      m_fine.add_adjacency(pl_vertex, pl_edge, 4, 19);
      m_fine.add_adjacency(pl_vertex, pl_edge, 4, 22);
      m_fine.add_adjacency(pl_vertex, pl_edge, 4, 1);

      TEST_CHECK(!MeshUtil::property_tetra(m_fine));
      MeshUtil::establish_property_tetra(m_fine);
      TEST_CHECK(MeshUtil::property_tetra(m_fine));
    }
};
MeshUtilTest3D_tetra<Mem::Main, Index, std::vector, std::vector<Index> > mu_test_cpu_v_v_3d_tetra("std::vector, std::vector");
/*MeshUtilTest3D_tetra<Mem::Main, Index, std::vector, std::deque<Index> > mu_test_cpu_v_d_3d_tetra("std::vector, std::deque");
MeshUtilTest3D_tetra<Mem::Main, Index, std::deque, std::vector<Index> > mu_test_cpu_d_v_3d_tetra("std::deque, std::vector");
MeshUtilTest3D_tetra<Mem::Main, Index, std::deque, std::deque<Index> > mu_test_cpu_d_d_3d_tetra("std::deque, std::deque");*/
