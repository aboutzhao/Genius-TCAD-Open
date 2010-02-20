/********************************************************************************/
/*     888888    888888888   88     888  88888   888      888    88888888       */
/*   8       8   8           8 8     8     8      8        8    8               */
/*  8            8           8  8    8     8      8        8    8               */
/*  8            888888888   8   8   8     8      8        8     8888888        */
/*  8      8888  8           8    8  8     8      8        8            8       */
/*   8       8   8           8     8 8     8      8        8            8       */
/*     888888    888888888  888     88   88888     88888888     88888888        */
/*                                                                              */
/*       A Three-Dimensional General Purpose Semiconductor Simulator.           */
/*                                                                              */
/*                                                                              */
/*  Copyright (C) 2007-2008                                                     */
/*  Cogenda Pte Ltd                                                             */
/*                                                                              */
/*  Please contact Cogenda Pte Ltd for license information                      */
/*                                                                              */
/*  Author: Gong Ding   gdiso@ustc.edu                                          */
/*                                                                              */
/********************************************************************************/

//  $Id: fvm_node_info.h,v 1.38 2008/07/09 05:58:16 gdiso Exp $

#ifndef __fvm_node_h__
#define __fvm_node_h__


#include <vector>
#include <set>

#include "node.h"
#include "fvm_node_data.h"

class Elem;

/**
 * for FVM usage, we need to construct control volume (CV)
 * A CV has a centre node (root node) which has more information:
 *    its neighbor node
 *    and which element this belongs to.
 * FVM_Node is equal to CV in most of the case.
 * However, when the root node lies on the interface of several subdomains,
 * the control volume is splited by these subdomains.
 * For this situation, several FVM Nodes will be generated
 * to make sure each FVM Node always belongs to one subdomain.
 * Every FVM nodes have the same Node * pointer but has the part
 * of the whole control volume.
 \verbatim
   for 2D case, a node lies on interface will be separated into 2 FVM_Node
   each one takes the half of the origin control volume:

          |/
     .----|/---.
    /     |/    \
   /      |/     \
  /      o|o      \
  \       |/      /
   \      |/     /
    \_____|/____/
          |/
      |/

   \endverbatim
 */

class FVM_Node
{

public:

  /**
   * constructor
   */
  FVM_Node(const Node *n=NULL);

  /*
   * copy constructor, default is ok
   */

  /**
   * destructor, we should be carefully here.
   * do not delete any pointer except _node_data!
   */
  ~FVM_Node()
  {
    _node = 0;
    delete _node_data;
    delete _elem_has_this_node;
    delete _node_neighbor;
    delete _cv_surface_area;
    delete _ghost_nodes;
  }


  /**
   * @return the centre node pointer
   */
  const Node * root_node() const
    { return _node; }

  /**
   * @return the pointer to nodal data
   */
  FVM_NodeData * node_data()
  { return _node_data;}

  /**
   * @return the const pointer to nodal data
   */
  const FVM_NodeData * node_data() const
    { return _node_data;}


  /**
   * let this fvm_node hold FVM_NodeData
   */
  void hold_node_data(FVM_NodeData * data)
  { _node_data = data; }

  /**
   * @return true if this FVM_Node belongs to local processor
   */
  bool on_processor() const
  { return _node->processor_id() == Genius::processor_id(); }

  /**
   * @return true if this FVM_Node belongs to local processor or is a ghost FVM_Node
   */
  bool on_local() const
  { return _node->on_local(); }

  /**
   * @return true if _global_offset and _local_offset are valid
   */
  bool is_valid() const
  { return (_global_offset != invalid_uint && _local_offset != invalid_uint);}

  /**
   * set element-node map
   */
  void set_elem_it_belongs(const Elem * el, unsigned int n)
  {
    if(_elem_has_this_node==NULL)
      _elem_has_this_node = new std::vector< std::pair<const Elem *, unsigned int> >;

    _elem_has_this_node->push_back( std::pair<const Elem *,unsigned int>(el,n) );
  }

  /**
   * set the node which connects to me as my neighbor.
   */
  void set_node_neighbor(const Node * n, FVM_Node *fn=NULL)
  {
    if( _node_neighbor == NULL)
      _node_neighbor = new std::map< const Node *, FVM_Node * >;

    (*_node_neighbor)[n] = fn;
  }

  /**
   * set ghost node, which has the same root_node but in different region
   */
  void set_ghost_node(FVM_Node * fn, unsigned int sub_id, Real area)
  {
    if( _ghost_nodes == NULL)
      _ghost_nodes = new std::map< FVM_Node *, std::pair<unsigned int, Real> >;

    std::pair<unsigned int, Real> gf(sub_id,area);
    _ghost_nodes->insert( std::pair< FVM_Node *, std::pair<unsigned int, Real> >(fn, gf) );
  }

  /**
   * set interface area of the ghost node
   */
  void set_ghost_node_area(unsigned int sub_id, Real area)
  {
    // the sub_id of boundary face equal to this FVM_Node and _ghost_nodes are empty
    // this is a boundary face, not interface face
    if( sub_id == _subdomain_id && _ghost_nodes==NULL )
    {
      _ghost_nodes = new std::map< FVM_Node *, std::pair<unsigned int, Real> >;
      std::pair<unsigned int, Real> gf(invalid_uint, area);
      _ghost_nodes->insert( std::pair< FVM_Node *, std::pair<unsigned int, Real> >((FVM_Node *)NULL, gf) );
      return;
    }

    // else we find in ghost nodes which matches sub_id
    genius_assert(_ghost_nodes);
    std::map< FVM_Node *, std::pair<unsigned int, Real> >::iterator it = _ghost_nodes->begin();
    for(; it!=_ghost_nodes->end(); ++it)
      if( (*it).second.first ==  sub_id )
      {
        (*it).second.second = area;
      }
  }

  /**
   * assign neighbor related cv surface area
   */
  Real & cv_surface_area(const Node * neighbor)
  {
    genius_assert(this->is_neighbor(neighbor));

    if( _cv_surface_area == NULL )
      _cv_surface_area = new std::map< const Node *, Real >;

    return (*_cv_surface_area)[neighbor];
  }

  /**
   * get neighbor related cv surface area
   */
  Real cv_surface_area(const Node * neighbor) const
  {
    genius_assert(this->is_neighbor(neighbor));
    genius_assert(_cv_surface_area);
    return (*_cv_surface_area->find(neighbor)).second;
  }

  typedef std::map< FVM_Node *, std::pair<unsigned int, Real> >::const_iterator fvm_ghost_node_iterator;

  /**
   * @return the number of ghost node, which in different region.
   * the NULL node (indicate outside boundary) is also considered here.
   */
  unsigned int n_ghost_node() const
  {
     genius_assert(_ghost_nodes);
     return _ghost_nodes->size();
  }

  /**
   * @return the number of ghost node, which in different region. No NULL nodes!
   */
  unsigned int n_pure_ghost_node() const
  {
    genius_assert(_ghost_nodes);
    // sun NULL ghost node
    if( _ghost_nodes->find(NULL)!=_ghost_nodes->end() )
      return _ghost_nodes->size() -1 ;
    return _ghost_nodes->size();
  }

  /**
   * @return the begin position of _ghost_nodes
   */
  fvm_ghost_node_iterator  ghost_node_begin() const
    { genius_assert(_ghost_nodes); return _ghost_nodes->begin(); }


  /**
   * @return the end position of _ghost_nodes
   */
  fvm_ghost_node_iterator  ghost_node_end() const
    { genius_assert(_ghost_nodes); return _ghost_nodes->end(); }

  /**
   * @return ith ghost FVM_Node
   */
  FVM_Node * ghost_fvm_node(unsigned int i) const
  {
    genius_assert(_ghost_nodes);
    genius_assert(i<n_ghost_node());
    fvm_ghost_node_iterator  it = ghost_node_begin();
    for(unsigned int l=0; l<i; ++i, ++it);
    return (*it).first;
  }

  /**
   * @return the area of CV (control volume) surface to outside boundary
   \verbatim
          |/
     .----|/
    /     |/
   /      |/
  /      o|/ <-- the area of this surface
  \       |/
   \      |/
    \_____|/
          |/
          |/
   \endverbatim
   */
  Real outside_boundary_surface_area() const
  {
    genius_assert(_ghost_nodes);
    Real area = 0.0;
    fvm_ghost_node_iterator  it = ghost_node_begin();
    for(; it!=ghost_node_end(); ++it )
       area += (*it).second.second;
    return area;
  }

  /**
   * set the bc index for this node
   */
  void set_subdomain_id (unsigned int sbd_id)
  { _subdomain_id = sbd_id; }


  /**
   * set the boundary index for this node
   * Please note it is boundary id here, not boundary condition index.
   */
  void set_boundary_id (short int bn_id)
  { _boundary_id = bn_id; }


  /**
   * set the volume of CV
   */
  void set_control_volume( Real v)
  { _volume = v; }

  /**
   * @return the fvm_node's subdomain id
   */
  unsigned int subdomain_id () const
    { return _subdomain_id; }

  /**
   * @return the node's bc index
   */
  short int boundary_id () const
    { return _boundary_id; }


  /**
   * @return the offset of nodal solution data in global petsc vector
   */
  unsigned int global_offset () const
    { return _global_offset; }

  /**
   * function for set global offset
   */
  void set_global_offset (unsigned int pos )
  { _global_offset = pos; }


  /**
   * @return the offset of nodal solution data in local vector
   */
  unsigned int local_offset () const
    { return _local_offset; }

  /**
   * function for set local offset
   */
  void set_local_offset (unsigned int pos )
  { _local_offset = pos; }

  /**
   * @return the volume of this FVM cell
   */
  Real volume() const
    { return _volume; }


  typedef std::vector< std::pair<const Elem *,unsigned int> >::const_iterator fvm_element_iterator;

  /**
   * @return the begin position of _elem_has_this_node
   */
  fvm_element_iterator  elem_begin() const
    { genius_assert(_elem_has_this_node); return  _elem_has_this_node->begin(); }

  /**
   * @return the end position of _elem_has_this_node
   */
  fvm_element_iterator  elem_end() const
    { genius_assert(_elem_has_this_node); return  _elem_has_this_node->end(); }



  typedef std::map<const Node *, FVM_Node *>::const_iterator fvm_neighbor_node_iterator;


  /**
   * @return the number of node neighbors, only this region
   */
  unsigned int n_node_neighbors() const
    { genius_assert(_node_neighbor); return _node_neighbor->size(); }


  /**
   * get PDE involved node structure. including itself and neighbor nodes, and also the ghost node and neighbors of the ghost node
   * return as <region, node_num>. when elem_based is true, all the nodes belong to neighbor elements will be returned.
   * else only consider nodes which link this node by an edge.
   */
  void PDE_node_pattern(std::vector<std::pair<unsigned int, unsigned int> > &, bool elem_based=false) const;


  /**
   * get PDE involved node structure. only consider (neighbor) nodes NOT on this processor.
   */
  void PDE_off_processor_node_pattern(std::vector<std::pair<unsigned int, unsigned int> > &, bool elem_based=false) const;


  /**
   * @return true iff node is a neighbor of this FVM_Node
   */
  bool is_neighbor(const Node * node) const
  { genius_assert(_node_neighbor); return _node_neighbor->find(node)!=_node_neighbor->end(); }


  /**
   * @return the begin position of _node_neighbor
   */
  fvm_neighbor_node_iterator  neighbor_node_begin() const
    { genius_assert(_node_neighbor); return _node_neighbor->begin(); }


  /**
   * @return the end position of _node_neighbor
   */
  fvm_neighbor_node_iterator  neighbor_node_end() const
    { genius_assert(_node_neighbor); return _node_neighbor->end(); }


  /**
   * combine several FVM_Node with same root node
   */
  void operator += (const FVM_Node &other_node);


private:

  /**
   * the pointer to corresponding Node, we don't want to modify it
   */
  const Node * _node;

  /**
   * the pointer to corresponding Nodal data
   */
  FVM_NodeData * _node_data;

  /**
   * The vector of:
   * the element this node belongs to, and the index of this node in element
   * @note only contain elements in the same subdomain!
   */
  std::vector< std::pair<const Elem *, unsigned int> > * _elem_has_this_node;


  /**
   * the node neighbor (link this node by a side edge) as well as their FVM_Node map.
   * only neighbors belong to same region (have the same subdomain id) are recorded.
   */
  std::map< const Node *, FVM_Node * > * _node_neighbor;


  /**
   * control volume surface area. indicated by neighbor Node
   */
  std::map< const Node *, Real > * _cv_surface_area;

  /**
   * the FVM Node with same root node, but in different region
   * record the region index of ghost node as well as the area of interface
   * the NULL ghost node means this node on the boundary
   */
  std::map< FVM_Node *, std::pair<unsigned int, Real> > * _ghost_nodes ;

  /**
   * the volume of this CV (control volume)
   */
  Real _volume;

  /**
   * the boundary index of this node
   */
  short int _boundary_id;

  /**
   * the subdomain id of this node
   */
  unsigned int _subdomain_id;

  /**
   * the offset of nodal solution data in global petsc vector
   * this value should be set by every solver
   */
  unsigned int _global_offset;

  /**
   * the offset of nodal solution data in local petsc vector
   * this value should be set by every solver
   */
  unsigned int _local_offset;

};


#endif
