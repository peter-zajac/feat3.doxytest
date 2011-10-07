/* GENERAL_REMARK_BY_HILMAR:
 * See COMMENT_HILMAR below.
 * Generally, Peter wanted to take a deeper look at the base mesh implementation.
 *
 * HILMAR WON'T TOUCH THIS FILE ANYMORE! Please remove this comment-block as soon as possible... :-)
 */
#pragma once
#ifndef KERNEL_BASE_MESH_CELL_2D_QUAD_HPP
#define KERNEL_BASE_MESH_CELL_2D_QUAD_HPP 1

// includes, FEAST
#include <kernel/base_header.hpp>
#include <kernel/util/exception.hpp>
#include <kernel/util/assertion.hpp>
#include <kernel/error_handler.hpp>
#include <kernel/base_mesh/vertex.hpp>
#include <kernel/base_mesh/cell.hpp>
#include <kernel/base_mesh/cell_data_validation.hpp>
#include <kernel/base_mesh/cell_1d_edge.hpp>

// includes, system
#include <iostream> // for std::ostream
#include <typeinfo>  // for typeid()

namespace FEAST
{
  /// BaseMesh namespace comprising base mesh specific code
  namespace BaseMesh
  {
    /**
    * \brief 2D base mesh cell of type quad
    *
    * numbering scheme:
    *
    *           e1
    *      v2---------v3
    *      |           |
    *      |           |
    *    e2|           |e3
    *      |           |
    *      |           |
    *      v0---------v1
    *           e0
    *
    * vertices of standard quad [0,1]x[0,1]:
    *   v0: (0, 0)
    *   v1: (1, 0)
    *   v2: (0, 1)
    *   v3: (1, 1)
    * edges:
    *   e0: (v0,v1)
    *   e1: (v2,v3)
    *   e2: (v0,v2)
    *   e3: (v1,v3)
    *
    * "Orientation in the quad" means that edge vertices are traversed by increasing local vertex index. So, this must
    * not be mixed up with the standard way of defining orientation (i.e., running counter-clockwise through the quad).
    *
    * \tparam space_dim_
    * space dimension (must be <= world_dim_; it is < world_dim_, e.g., when doing FE on 2D surfaces in a 3D world)
    *
    * \tparam world_dim_
    * world dimension (determines the number of coordinates)
    *
    * \author Hilmar Wobker
    */
// COMMENT_HILMAR: Um Code-Redundanz zu vermeiden, koennten wir ueberlegen, eine weitere Klasse Cell2D einzufuehren,
// die von Cell<2, space_dim_, world_dim_> erbt, und von der dann wiederum Quad und Tri erben. Darin koennte
// man zum Beispiel die Funktion _determine_edge_orientation() implementieren.
    template<
      unsigned char space_dim_,
      unsigned char world_dim_>
    class Quad
      : public Cell<2, space_dim_, world_dim_>
    {
      /// shortcut to save typing of template parameters
      typedef Vertex<world_dim_> Vertex_;

      /// shortcut to save typing of template parameters
      typedef Edge<space_dim_, world_dim_> Edge_;

      /// shortcut to save typing of template parameters
      typedef Cell<1, space_dim_, world_dim_> Cell_1D_;


    private:

      /* *****************
      * member variables *
      *******************/
      /// vertices of the quad
      Vertex_* _vertices[4];

      /// edges of the quad
      Cell_1D_* _edges[4];

      /// stores whether the edge orientations in edge- and quad structure coincide
      bool _edge_has_correct_orientation[4];


      /* **********
      * functions *
      ************/
      /**
      * \brief returns index (w.r.t. to quad numbering) of start vertex (iv=0) or end vertex (iv=1) of edge iedge
      *
      * \param[in] iedge
      * index of the edge whose start or end vertex is inquired
      *
      * \param[in] iv
      * decides whether start vertex (iv = 0) or end vertex (iv = 1) is inquired
      *
      * \return index of start or end vertex
      */
      inline unsigned char _edge_vertex(unsigned char iedge, unsigned char iv) const
      {
        CONTEXT("BaseMesh::Quad::_edge_vertex()");
        ASSERT(iedge < num_edges(), "Index " + stringify(iedge) + " must not exceed number of edges "
               + stringify(num_edges()) + ".");
        ASSERT(iv < 2, "Edge vertex index " + stringify(iv) + " must not be larger than 1.");
        // the index is inquired from the fixed numbering scheme stored in Numbering::quad_edge_vertices
        return Numbering::quad_edge_vertices[iedge][iv];
      }


      /**
      * \brief Determines orientation of the edges and tests whether edges are set up correctly.
      *
      * This function stores whether the orientation of the edges coincide with their orientation within the quad.
      * At the same time, it tests whether edge vertices are set correctly.
      */
      inline void _determine_edge_orientation()
      {
        CONTEXT("BaseMesh::Quad::_determine_edge_orientation()");
        try
        {
          for(unsigned char iedge(0) ; iedge < num_edges() ; ++iedge)
          {
            if(vertex(_edge_vertex(iedge,0)) == edge(iedge)->vertex(0))
            {
              ASSERT(vertex(_edge_vertex(iedge,1)) == edge(iedge)->vertex(1), "Vertex pointers are not equal.");
              _edge_has_correct_orientation[iedge] = true;
            }
            else if (vertex(_edge_vertex(iedge,0)) == edge(iedge)->vertex(1))
            {
              ASSERT(vertex(_edge_vertex(iedge,1)) == edge(iedge)->vertex(0), "Vertex pointers are not equal.");
              _edge_has_correct_orientation[iedge] = false;
            }
            else
            {
              throw InternalError("Something is wrong with the numbering of the " + stringify((int)iedge)
                                  + "-th edge with index " + edge(iedge)->print_index() + " in quad "
                                  + this->print_index() + ".");
            }
          }
        }
        catch(Exception& e)
        {
          ErrorHandler::exception_occured(e);
        }
      }


    public:

      /**
      * \brief CTOR
      *
      * \param[in] v0, v1, v2, v3
      * vertices the quad is built of
      *
      * \param[in] e0, e1, e2, e3
      * edges the quad is built of
      *
      * \param[in] ref_level
      * refinement level the quad is created on
      */
      Quad(
        Vertex_* v0, Vertex_* v1, Vertex_* v2, Vertex_* v3,
        Cell_1D_* e0, Cell_1D_* e1, Cell_1D_* e2, Cell_1D_* e3,
        unsigned char ref_level)
        : Cell<2, space_dim_, world_dim_>(ref_level)
      {
        CONTEXT("BaseMesh::Quad::Quad()");
        _vertices[0] = v0;
        _vertices[1] = v1;
        _vertices[2] = v2;
        _vertices[3] = v3;
        _edges[0] = e0;
        _edges[1] = e1;
        _edges[2] = e2;
        _edges[3] = e3;
        // assure that the edges are in fact of type Edge<space_dim_, world_dim_>, and not "only"
        // of type Cell<1, space_dim_, world_dim_>
        for(int i(0) ; i < 4 ; ++i)
        {
          ASSERT(typeid(*_edges[i]) == typeid(Edge_), "The " + stringify(i) + "-th edge (index "
                 + stringify(_edges[i]->index()) + ") must be of type Edge<space_dim_, world_dim_>.");
        }

        unsigned char num_subitems_per_subdim[2] = {4,4};
        this->_set_num_subitems_per_subdim(2, num_subitems_per_subdim);
        this->_init_neighbours();
// COMMENT_HILMAR: Eigentlich haette ich das lieber in die Konstruktoren-Liste gepackt, also sowas in der Art:
//    : CellData<2, space_dim_, world_dim_>({4,4})
// (was nicht kompiliert). Wie kann man denn on-the-fly ein Array anlegen und durchreichen?
        // determine the orientation of the four edges
        _determine_edge_orientation();
      }


      /* **********
      * functions *
      ************/
      /**
      * \brief returns number of vertices
      *
      * \return number of vertices
      */
      inline unsigned char num_vertices() const
      {
        CONTEXT("BaseMesh::Quad::num_vertices()");
        return 4;
      }


      /**
      * \brief returns pointer to the vertex at given index
      *
      * \param[in] index
      * index of the vertex to be returned
      *
      * \return pointer to the vertex at given index
      */
      inline Vertex_* vertex(unsigned char const index) const
      {
        CONTEXT("BaseMesh::Quad::vertex()");
        ASSERT(index < num_vertices(), "Index " + stringify(index) + " must not exceed number of vertices "
               + stringify(num_vertices()) + ".");
        return _vertices[index];
      }


      /**
      * \brief returns number of edges
      *
      * \return number of edges
      */
      inline unsigned char num_edges() const
      {
        CONTEXT("BaseMesh::Quad::num_edges()");
        return 4;
      }


      /**
      * \brief returns pointer to the edge at given index
      *
      * \param[in] index
      * index of the edge to be returned
      *
      * \return pointer to the edge at given index
      */
      inline Cell_1D_* edge(unsigned char const index) const
      {
        CONTEXT("BaseMesh::Quad::edge()");
        ASSERT(index < num_edges(), "Index " + stringify(index) + " must not exceed number of edges "
               + stringify(num_edges()) + ".");
        return _edges[index];
      }


      /**
      * \brief returns next vertex of vertex with given index w.r.t. to ccw ordering
      *
      * \param[in] index
      * index of the vertex whose next vertex is to be returned
      *
      * \return next vertex of given vertex
      */
      inline Vertex_* next_vertex_ccw(unsigned char const index) const
      {
        CONTEXT("BaseMesh::Quad::next_vertex_ccw()");
        ASSERT(index < num_vertices(), "Index " + stringify(index) + " must not exceed number of vertices "
               + stringify(num_vertices()) + ".");
        return _vertices[Numbering::quad_next_vertex_ccw[index]];
      }


      /**
      * \brief returns previous vertex of vertex with given index w.r.t. to ccw ordering
      *
      * \param[in] index
      * index of the vertex whose previous vertex is to be returned
      *
      * \return previous vertex of given vertex
      */
      inline Vertex_* previous_vertex_ccw(unsigned char const index) const
      {
        CONTEXT("BaseMesh::Quad::previous_vertex_ccw()");
        ASSERT(index < num_vertices(), "Index " + stringify(index) + " must not exceed number of vertices "
               + stringify(num_vertices()) + ".");
        return _vertices[Numbering::quad_previous_vertex_ccw[index]];
      }


      /**
      * \brief returns next edge of edge with given index w.r.t. to ccw ordering
      *
      * \param[in] index
      * index of the edge whose next edge is to be returned
      *
      * \return next edge of given edge
      */
      inline Cell_1D_* next_edge_ccw(unsigned char const index) const
      {
        CONTEXT("BaseMesh::Quad::next_edge_ccw()");
        ASSERT(index < num_edges(), "Index " + stringify(index) + " must not exceed number of edges "
               + stringify(num_edges()) + ".");
        return _edges[Numbering::quad_next_edge_ccw[index]];
      }


      /**
      * \brief pure virtual function for returning previous edge of edge with given index w.r.t. to ccw ordering
      *
      * \param[in] index
      * index of the edge whose previous edge is to be returned
      *
      * \return previous edge of given edge
      */
      inline Cell_1D_* previous_edge_ccw(unsigned char const index) const
      {
        CONTEXT("BaseMesh::Quad::previous_edge_ccw()");
        ASSERT(index < num_edges(), "Index " + stringify(index) + " must not exceed number of edges "
               + stringify(num_edges()) + ".");
        return _edges[Numbering::quad_previous_edge_ccw[index]];
      }


      /**
      * \brief subdivision routine splitting a quad and storing parent/child information
      *
      * \param[in,out] subdiv_data
      * pointer to the subdivision data object
      *
      * \todo New centre vertex of the quad has to be computed properly!
      */
      inline void subdivide(SubdivisionData<2, space_dim_, world_dim_>* subdiv_data)
      {
        CONTEXT("BaseMesh::Quad::subdivide()");
        try
        {
          // assure that this cell has not been divided yet
          if(!this->active())
          {
            throw InternalError("Quad " + this->print_index() + " is already subdivided!");
          }

          this->set_subdiv_data(subdiv_data);

          // clear all vectors of created entities in the SubdivisionData object
          this->subdiv_data()->clear_created();

          if(this->subdiv_data()->type == NONCONFORM_SAME_TYPE)
          {
            // Perform a nonconform subdivision without changing the cell type (1 quad --> 4 quads).

            // local numbering (old and new)
            //         k1                                       e2     e3
            //   w2---------w3          -----v1------         -------------
            //   |           |          |     |     |       e5|    e9     |e7
            //   |           |          |  q2 | q3  |         |     |     |
            // k2|           |k3  ---> v2-----v4----v3        --e10---e11--
            //   |           |          |  q0 | q1  |       e4|     |     |e6
            //   |           |          |     |     |         |     e8    |
            //   w0---------w1          -----v0------         -------------
            //         k0                                        e0    e1

            /// vertices that this action creates and/or reuses
            Vertex_* new_vertices[5];

            /// edges that this action creates and/or reuses
            Cell_1D_* new_edges[12];

            // loop over all edges and split them eventually, creating new vertices and edges on the way
            for(unsigned char iedge(0) ; iedge < 4 ; ++iedge)
            {
              // if edge has no children, create them
              if (edge(iedge)->active())
              {
                // subdivide edge
                SubdivisionData<1, space_dim_, world_dim_>* subdiv_data
                  = new SubdivisionData<1, space_dim_, world_dim_>(CONFORM_SAME_TYPE);
                edge(iedge)->subdivide(subdiv_data);

                // add the created vertices/edges to the vector of created vertices/edges
                this->subdiv_data()->created_vertices.push_back(edge(iedge)->subdiv_data()->created_vertex);
                this->subdiv_data()->created_edges.push_back(edge(iedge)->subdiv_data()->created_cells[0]);
                this->subdiv_data()->created_edges.push_back(edge(iedge)->subdiv_data()->created_cells[1]);
              }

              // add new vertex to array of new vertices (exploit that the vertex shared by the edge children is stored
              // as second vertex within the structure of both edge children)
              new_vertices[iedge] = edge(iedge)->child(0)->vertex(1);

              // add new edges to array of new edges, respect the orientation of the edge
              if(_edge_has_correct_orientation[iedge])
              {
                // if the edge has the orientation of the quad, then child 0 is the first edge
                new_edges[2*iedge]   = edge(iedge)->child(0);
                new_edges[2*iedge+1] = edge(iedge)->child(1);
              }
              else
              {
                // if the edge does not have the orientation of the quad, then child 1 is the first edge.
                new_edges[2*iedge]   = edge(iedge)->child(1);
                new_edges[2*iedge+1] = edge(iedge)->child(0);
              }
// COMMENT_HILMAR: Beachte, dass wir auch in dem Fall, dass eine Kante schon Kinder hatte, *nicht* annehmen koennen,
// dass sie vom Nachbarelement erstellt worden ist. Beispiel: 2 benachbarte Zellen C0 und C1. C0 zerteilt sich, hat
// somit die gemeinsame Kante e zerteilt, sodass die Reihenfolge der Kinder der lokalen Orientierung von C0 entspricht.
// Irgendwann spaeter zerteilt sich C1 und nutzt aus, dass die Kinder der Kante e ja von C0 angelegt worden sein
// muessen und dreht deren Reihenfolge also einfach um. Hier stimmt die Annahme also noch.
// Jetzt vergroebert sich C0 wieder, Kante e bleibt bestehen, da sie ja noch von C1 benutzt wird. Dann irgendwann
// verfeinert sich C0 wieder: Nun ist es aber falsch anzunehmen, dass der Nachbar (C1) die Kante angelegt haben muss!
// In Wirklichkeit war C0 es selbst, hat das aber inzwischen "vergessen".
            } // for(unsigned char iedge(0) ; iedge < 4 ; ++iedge)

            // create new centre vertex v4
            //   -----v1------
            //   |     |     |
            //   |  q2 | q3  |
            //  v2-----v4----v3
            //   |  q0 | q1  |
            //   |     |     |
            //   -----v0------

//COMMENT_HILMAR: Das hier funktioniert nur fuer world_dim_ = 2!
//            double const* x0 = new_vertices[0]->coords();
//            double const* x1 = new_vertices[1]->coords();
//            double const* x2 = new_vertices[2]->coords();
//            double const* x3 = new_vertices[3]->coords();
//
//            double p[2];
//
//            double denom = (x0[0]-x1[0])*(x2[1]-x3[1]) - (x0[1]-x1[1])*(x2[0]-x3[0]);
//            double fac0 = x0[0]*x1[1]-x0[1]*x1[0];
//            double fac1 = x2[0]*x3[1]-x2[1]*x3[0];
//
//            p[0] = ( fac0*(x2[0]-x3[0]) - (x0[0]-x1[0])*fac1 ) / denom;
//            p[1] = ( fac0*(x2[1]-x3[1]) - (x0[1]-x1[1])*fac1 ) / denom;

// COMMENT_HILMAR: For the time being simply compute the midpoint of the quad as average of the four vertices
// until we find out, what is the best way of computing this point correctly.

// Note that in 3D the two lines connecting the edge midpoints do not necessarily intersect!
// One possible strategy: Find the points on the two lines where they have the smallest distance, take the average
// of these two points.
// Another strategy: Use FE techniques, i.e. consider the bilinear mapping from the 2D reference element (quad
// [-1,1] x [-1,1]) to the actual quad, use this mapping to compute the center node. Problem: How to deal with the
// 3D case, i.e., where the quad is a 2D structure in the 3D space (face of a hexa)? Here, one actually maps from one
// face of the 3D reference element (cube [-1,1] x [-1,1] x [-1,1]) to the face of the actual hexa (trilinear mapping
// restricted to one face). So, it might be necessary to compute the coordinates of the new vertex already *outside*
// this routine...
            double p[world_dim_];
            for(unsigned char i(0) ; i < world_dim_ ; ++i)
            {
              p[i] = 0;
              for(int j(0) ; j < num_vertices() ; ++j)
              {
                p[i] += vertex(j)->coord(i);
              }
              p[i] /= num_vertices();
            }
            new_vertices[4] = new Vertex<world_dim_>(p);

            this->subdiv_data()->created_vertices.push_back(new_vertices[4]);

            // Create the four new edges in the interior of the quad (these are always new, have no children and cannot
            // be reused). Set the new centre vertex of the quad (v4) as the end vertex of these edges.
            for (unsigned char i(0) ; i < num_edges() ; ++i)
            {
              new_edges[i+8] = new Edge<space_dim_, world_dim_>(new_vertices[i], new_vertices[4], 0);
              this->subdiv_data()->created_edges.push_back(new_edges[i+8]);
            }

            // set number of children to 4
            this->_set_num_children(4);

            // finally, create new quads and add them as children
            //
            // local numbering of the four children:
            //
            // w2          e1           w3
            //    +---------+---------+
            //    |1   3   3|2   2   0|
            //    |         |         |              w2----v1-----w3        --e2-----e3--
            //    |0   2   1|1   3   0|               |     |     |         |     |     |
            //    |         |         |               |  q2 | q3  |        e5     e9    e7
            //    |0   2   2|3   3   1|               |     |     |         |     |     |
            // e2 +---------+---------+ e3           v2----v4-----v3        --e10---e11--
            //    |2   2   0|1   3   3|               |     |     |         |     |     |
            //    |         |         |               |  q0 | q1  |        e4     e8    e6
            //    |1   0   0|0   1   1|               |     |     |         |     |     |
            //    |         |         |              w0--x-v0-----w1        --e0----e1---
            //    |3   3   1|0   2   2|
            //    +---------+---------+
            // w0          e0           w1
            //
            // building rule: For each child, the inner edge with local index i builds a 'T' with edge i of the
            //                parent quad, i.e. it is connected to this edge of the parent quad in its centre.
            // ==> vertex i of child i is the centre vertex
            //
            // These facts are exploited, e.g., within the hexa subdivision routine, so don't change this!
            this->_set_child(0, new Quad(new_vertices[4], new_vertices[0], new_vertices[2], vertex(0),
                                         new_edges[8], new_edges[4], new_edges[10], new_edges[0],
                                         this->refinement_level()+1));
            this->_set_child(1, new Quad(new_vertices[0], new_vertices[4], vertex(1), new_vertices[3],
                                         new_edges[8], new_edges[6], new_edges[1], new_edges[11],
                                         this->refinement_level()+1));
            this->_set_child(2, new Quad(new_vertices[2], vertex(2), new_vertices[4], new_vertices[1],
                                         new_edges[5], new_edges[9], new_edges[10], new_edges[2],
                                         this->refinement_level()+1));
            this->_set_child(3, new Quad(vertex(3), new_vertices[3], new_vertices[1], new_vertices[4],
                                         new_edges[7], new_edges[9], new_edges[3], new_edges[11],
                                         this->refinement_level()+1));

            // Geometric interpretation of this building rule:
            // To obtain the numbering of the parent quad from the numbering of a child, simply "mirror" the child at
            // the diagonal displayed in the following figure
            //   +-----+-----+
            //   |    /|\    |
            //   |  /  |  \  |
            //   |/    |    \|
            //   +-----+-----+
            //   |\    |    /|
            //   |  \  |  /  |
            //   |    \|/    |
            //   +-----+-----+

            // add the quads to the vector of new created cells
            for (unsigned char i(0) ; i < this->num_children() ; ++i)
            {
              this->child(i)->set_parent(this);
              this->subdiv_data()->created_cells.push_back(this->child(i));
            }

            // set internal neighbourhood (external neighbourhood is set outside this function)
            // (in case space_dim_ > 2, an empty dummy function is called; see CellData)
            // edge neighbours
            this->child(0)->add_neighbour(SDIM_EDGE, 0, this->child(1));
            this->child(0)->add_neighbour(SDIM_EDGE, 2, this->child(2));
            this->child(1)->add_neighbour(SDIM_EDGE, 0, this->child(0));
            this->child(1)->add_neighbour(SDIM_EDGE, 3, this->child(3));
            this->child(2)->add_neighbour(SDIM_EDGE, 1, this->child(3));
            this->child(2)->add_neighbour(SDIM_EDGE, 2, this->child(0));
            this->child(3)->add_neighbour(SDIM_EDGE, 1, this->child(2));
            this->child(3)->add_neighbour(SDIM_EDGE, 3, this->child(1));
            // vertex neighbours
            this->child(0)->add_neighbour(SDIM_VERTEX, 0, this->child(3));
            this->child(1)->add_neighbour(SDIM_VERTEX, 1, this->child(2));
            this->child(2)->add_neighbour(SDIM_VERTEX, 2, this->child(1));
            this->child(3)->add_neighbour(SDIM_VERTEX, 3, this->child(0));
          }
          else
          {
            throw InternalError("Wrong type of subdivision in quad " + this->print_index()
                                + ". Currently, only subdivision NONCONFORM_SAME_TYPE is supported.");
          }
        }
        catch(Exception& e)
        {
          ErrorHandler::exception_occured(e);
        }
      } // subdivide()


      /**
      * \brief validates this cell
      *
      * \param[in,out] stream
      * stream validation info is written into
      */
      inline void validate(std::ostream& stream) const
      {
        CONTEXT("BaseMesh::Quad::validate()");
        try
        {
          if(space_dim_ == 2)
          {
            stream << "Validating quad " << this->print_index() << "..." << std::endl;
          }

          String s = "Quad " + this->print_index() + ": ";

          // validate that all vertices and edges are set
          for(unsigned char ivert(0) ; ivert < num_vertices() ; ++ivert)
          {
            if (vertex(ivert) == nullptr)
            {
              s += "Vertex " + stringify((int)ivert) + " is null.\n";
              throw new InternalError(s);
            }
          }
          for(unsigned char iedge(0) ; iedge < num_edges() ; ++iedge)
          {
            if (edge(iedge) == nullptr)
            {
              s += "Edge " + stringify((int)iedge) + " is null.\n";
              throw new InternalError(s);
            }
          }

          // validate subitems (here: egdes)
          for(unsigned char iedge(0) ; iedge < num_edges() ; ++iedge)
          {
            edge(iedge)->validate(stream);
          }

          // validate children numbering
          if(!this->active())
          {
            if(this->subdiv_data()->type == NONCONFORM_SAME_TYPE)
            {
              // check centre vertex in all children (see numbering scheme in subdivide routine)
              for(unsigned char ichild(0) ; ichild < num_edges()-1 ; ++ichild)
              {
                if(this->child(ichild)->vertex(ichild) != this->child(ichild+1)->vertex(ichild+1))
                {
                  s += "Centre vertex has wrong number in child " + stringify((int)ichild) + " or child "
                       + stringify(ichild+1) + ".\n";
                  throw new InternalError(s);
                }
              }
              // check corner vertices of the parent quad (see numbering scheme in subdivide routine)
              if(this->child(0)->vertex(3) != vertex(0) || this->child(1)->vertex(2) != vertex(1) ||
                 this->child(2)->vertex(1) != vertex(2) || this->child(3)->vertex(0) != vertex(3))
              {
                s += "One corner vertex has a wrong number in the corresponding child.\n";
                throw new InternalError(s);
              }

              // check the interior edges and and their vertices (see numbering scheme in subdivide routine)
              for(unsigned char i(0) ; i < num_edges()-1 ; ++i)
              {
                Cell_1D_* edge0 = this->child(Numbering::quad_edge_vertices[i][0])->edge(i);
                Cell_1D_* edge1 = this->child(Numbering::quad_edge_vertices[i][1])->edge(i);
                if(edge0 != edge1)
                {
                  s += "Interior edge connected to the " + stringify((int)i)
                       + "-th edge of the parent quad has a wrong number in one of the incident children.\n";
                  throw new InternalError(s);
                }
                Vertex_* vert_quad_centre = this->child(i)->vertex(i);
                Vertex_* vert_edge_centre = edge(i)->child(0)->vertex(1);
                if(!(edge0->vertex(0) == vert_edge_centre &&  edge0->vertex(1) == vert_quad_centre))
                {
                  s += "There is something wrong with the end vertices of the interior edge connected to the "
                       + stringify((int)i) + "-th edge of the parent quad.\n";
                  throw new InternalError(s);
                }
              }
            }
          }

          // validate parent-child relations
          this->validate_history(stream);

          // validate neighbours
          if (this->active())
          {
            CellDataValidation<2, space_dim_, world_dim_>::validate_neighbourhood(this, stream);
          }
        }
        catch(Exception& e)
        {
          ErrorHandler::exception_occured(e);
        }
      }


      /**
      * \brief prints information about this quad to the given stream
      *
      * \param[in,out] stream
      * stream to write into
      */
      inline void print(std::ostream& stream) const
      {
        CONTEXT("BaseMesh::Quad::print()");
        stream << "Quad";
        this->print_index(stream);

        stream << ": [V ";
        _vertices[0]->print_index(stream);
        for(int i(1) ; i < num_vertices() ; ++i)
        {
          stream << ", ";
          _vertices[i]->print_index(stream);
        }
        stream << "] [";

        stream << "E ";
        _edges[0]->print_index(stream);
        if(_edge_has_correct_orientation[0])
        {
          stream << "(+)";
        }
        else
        {
          stream << "(-)";
        }
        for(int i(1) ; i < num_edges() ; ++i)
        {
          stream << ", ";
          _edges[i]->print_index(stream);
          if(_edge_has_correct_orientation[i])
          {
            stream << "(+)";
          }
          else
          {
            stream << "(-)";
          }
        }
        stream << "] ";
        Cell<2, space_dim_, world_dim_>::print_history(stream);
        // print neighbourhood information (if there is any)
        CellData<2, space_dim_, world_dim_>::print(stream);
      }
    };
  } // namespace BaseMesh
} // namespace FEAST

#endif // KERNEL_BASE_MESH_CELL_2D_QUAD_HPP
