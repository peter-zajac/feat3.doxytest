#include <kernel/adjacency/graph.hpp>
#include <kernel/adjacency/colouring.hpp>
#include <kernel/adjacency/permutation.hpp>

namespace FEAST
{
  namespace Adjacency
  {
    Graph::Graph() :
      _num_nodes_domain(0),
      _num_nodes_image(0),
      _num_indices_image(0),
      _domain_ptr(nullptr),
      _domain_end(nullptr),
      _image_idx(nullptr),
      _shared(false)
    {
      CONTEXT("Graph::Graph() [default]");
    }

    // allocation constructor
    Graph::Graph(
      Index num_nodes_domain,
      Index num_nodes_image,
      Index num_indices_image,
      bool alloc_domain_end)
        :
      _num_nodes_domain(num_nodes_domain),
      _num_nodes_image(num_nodes_image),
      _num_indices_image(num_indices_image),
      _domain_ptr(nullptr),
      _domain_end(nullptr),
      _image_idx(nullptr),
      _shared(false)
    {
      CONTEXT("Graph::Graph() [alloc]");
      _domain_ptr = new Index[_num_nodes_domain+1];
      if(alloc_domain_end)
      {
        _domain_end = new Index[_num_nodes_domain];
      }
      _image_idx = new Index[_num_indices_image];
    }

    // "Using-Arrays" Constructor
    Graph::Graph(
      Index num_nodes_domain,
      Index num_nodes_image,
      Index num_indices_image,
      Index* domain_ptr,
      Index* domain_end,
      Index* image_idx,
      bool shared)
        :
      _num_nodes_domain(num_nodes_domain),
      _num_nodes_image(num_nodes_image),
      _num_indices_image(num_indices_image),
      _domain_ptr(domain_ptr),
      _domain_end(domain_end),
      _image_idx(image_idx),
      _shared(shared)
    {
      CONTEXT("Graph::Graph() [using-arrays]");
    }

    // "Copy-Arrays" Constructor
    Graph::Graph(
      Index num_nodes_domain,
      Index num_nodes_image,
      Index num_indices_image,
      const Index* domain_ptr,
      const Index* domain_end,
      const Index* image_idx)
        :
      _num_nodes_domain(num_nodes_domain),
      _num_nodes_image(num_nodes_image),
      _num_indices_image(num_indices_image),
      _domain_ptr(nullptr),
      _domain_end(nullptr),
      _image_idx(nullptr),
      _shared(false)
    {
      CONTEXT("Graph::Graph() [copy-arrays]");

      _domain_ptr = new Index[num_nodes_domain+1];
      for(Index i(0); i <= num_nodes_domain; ++i)
      {
        _domain_ptr[i] = domain_ptr[i];
      }
      if(domain_end != nullptr)
      {
        _domain_end = new Index[num_nodes_domain];
        for(Index i(0); i < num_nodes_domain; ++i)
        {
          _domain_end[i] = domain_end[i];
        }
      }
      _image_idx = new Index[num_indices_image];
      for(Index i(0); i < num_indices_image; ++i)
      {
        _image_idx[i] = image_idx[i];
      }
    }

    // "Colouring" Constructor
    Graph::Graph(const Colouring& col)
    {
      CONTEXT("Graph::Graph() [colouring]");

      // get 'size' of the graph
      _num_nodes_domain = col.get_max_colour() + 1;
      _num_nodes_image = col.get_num_nodes();
      _num_indices_image = col.get_num_nodes();
      _shared = false;

      // get colouring
      const Index* colouring = col.get_colouring();

      // we ignore the domain_end-array of the original graph
      _domain_end = nullptr;

      // create domain array
      _domain_ptr = new Index[_num_nodes_domain+1];
      _domain_ptr[0] = 0;

      // create image array
      _image_idx = new Index[_num_indices_image];

      // index counter
      Index idx_counter = 0;

      // loop over all colours
      for(Index i(0); i < _num_nodes_domain; ++i)
      {
        // loop over all nodes
        for(Index j(0); j < _num_nodes_image; ++j)
        {
          // if node j has the colour i
          if(i == colouring [j])
          {
            _image_idx[idx_counter] = j;
            ++idx_counter;
          }
        }
        _domain_ptr[i+1] = idx_counter;
      }
    }

    /// copy CTOR
    Graph::Graph(const Graph& other) :
      _num_nodes_domain(other.get_num_nodes_domain()),
      _num_nodes_image(other.get_num_nodes_image()),
      _num_indices_image(other.get_num_indices()),
      _domain_ptr(nullptr),
      _domain_end(nullptr),
      _image_idx(nullptr),
      _shared(false)
    {
      CONTEXT("Graph::Graph() [copy]");

      _domain_ptr = new Index[_num_nodes_domain+1];
      for(Index i(0); i <= _num_nodes_domain; ++i)
      {
        _domain_ptr[i] = other._domain_ptr[i];
      }
      if(other._domain_end != nullptr)
      {
        _domain_end = new Index[_num_nodes_domain];
        for(Index i(0); i < _num_nodes_domain; ++i)
        {
          _domain_end[i] = other._domain_end[i];
        }
      }
      _image_idx = new Index[_num_indices_image];
      for(Index i(0); i < _num_indices_image; ++i)
      {
        _image_idx[i] = other._image_idx[i];
      }
    }

    /// copy assignment
    Graph& Graph::operator=(const Graph& other)
    {
      CONTEXT("Graph::operator=()");
      if(!_shared)
      {
        if(_image_idx != nullptr)
          delete [] _image_idx;
        if(_domain_ptr != nullptr)
          delete [] _domain_ptr;
        if(_domain_end != nullptr)
          delete [] _domain_end;
      }

      _num_nodes_domain = other.get_num_nodes_domain();
      _num_nodes_image = other.get_num_nodes_image();
      _num_indices_image = other.get_num_indices();
      _shared = false;

      _domain_ptr = new Index[_num_nodes_domain+1];
      for(Index i(0); i <= _num_nodes_domain; ++i)
      {
        _domain_ptr[i] = other._domain_ptr[i];
      }
      if(other._domain_end != nullptr)
      {
        _domain_end = new Index[_num_nodes_domain];
        for(Index i(0); i < _num_nodes_domain; ++i)
        {
          _domain_end[i] = other._domain_end[i];
        }
      }
      else
      {
        _domain_end = nullptr;
      }
      _image_idx = new Index[_num_indices_image];
      for(Index i(0); i < _num_indices_image; ++i)
      {
        _image_idx[i] = other._image_idx[i];
      }

      return *this;
    }

    // "Permutation" copy CTOR
    Graph::Graph(const Graph& other, Permutation& domain_perm, Permutation& image_perm) :
      _num_nodes_domain(other.get_num_nodes_domain()),
      _num_nodes_image(other.get_num_nodes_image()),
      _num_indices_image(other.get_num_indices()),
      _domain_ptr(new Index[_num_nodes_domain+1]),
      _domain_end(nullptr),
      _image_idx(new Index[_num_indices_image]),
      _shared(false)
    {
      // get domain permutation
      Index* domain_perm_pos = domain_perm.get_perm_pos();

      // calculate new domain array
      _domain_ptr[0] = other._domain_ptr[0];
      for(Index i(0); i < _num_nodes_domain; ++i)
      {
        _domain_ptr[i+1] = other._domain_ptr[domain_perm_pos[i]+1]
                        - other._domain_ptr[domain_perm_pos[i]]
                        + _domain_ptr[i];
      }

      // get image permutation
      Index* image_perm_pos = image_perm.get_perm_pos();

      // calculate new image array
      Index count = 0;
      for(Index i(0); i < _num_nodes_domain; ++i)
      {
        Index _offset  = other._domain_ptr[domain_perm_pos[i]];
        Index _row_end = other._domain_ptr[domain_perm_pos[i]+1];
        for(Index j(_offset); j < _row_end; ++j)
        {
          _image_idx[count] = image_perm_pos[other._image_idx[j]];
          ++count;
        }
      }
    }

    // destructor
    Graph::~Graph()
    {
      CONTEXT("Graph::~Graph()");
      if(!_shared)
      {
        if(_image_idx != nullptr)
          delete [] _image_idx;
        if(_domain_ptr != nullptr)
          delete [] _domain_ptr;
        if(_domain_end != nullptr)
          delete [] _domain_end;
      }
    }

    Index Graph::degree() const
    {
      CONTEXT("Graph::degree()");
      Index deg = 0;
      if(_domain_end != nullptr)
      {
        for(Index i(0); i < _num_nodes_domain; ++i)
        {
          deg = std::max(deg, _domain_end[i] - _domain_ptr[i]);
        }
      }
      else
      {
        for(Index i(0); i < _num_nodes_domain; ++i)
        {
          deg = std::max(deg, _domain_ptr[i+1] - _domain_ptr[i]);
        }
      }
      return deg;
    }

    void Graph::sort_indices()
    {
      CONTEXT("Graph::sort_indices()");
      ASSERT_(_domain_ptr != nullptr);
      ASSERT_(_image_idx != nullptr);

      // fetch row-end pointer
      Index* domain_end = _domain_end != nullptr ? _domain_end : &_domain_ptr[1];

      // loop over all domain nodes
      for(Index i(0); i < _num_nodes_domain; ++i)
      {
        // apply linear insertion sort onto the adjacency indices
        for(Index j(_domain_ptr[i] + 1); j < domain_end[i]; ++j)
        {
          Index x = _image_idx[j];
          Index k(j);
          for(; (k > _domain_ptr[i]) && (x < _image_idx[k-1]); --k)
          {
            _image_idx[k] = _image_idx[k-1];
          }
          _image_idx[k] = x;
        }
      }
    }
  } // namespace Adjacency
} // namespace FEAST
