// $Id: mesh_refinement_smoothing.cc,v 1.2 2008/03/20 13:33:40 gdiso Exp $

// The libMesh Finite Element Library.
// Copyright (C) 2002-2007  Benjamin S. Kirk, John W. Peterson
  
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
  
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
  
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


// Local Includes -----------------------------------
#include "genius_common.h"


// only compile these functions if the user requests AMR support
#ifdef ENABLE_AMR

#include "mesh_refinement.h"
#include "mesh_base.h"
#include "elem.h"



//-----------------------------------------------------------------
// Mesh refinement methods
bool MeshRefinement::limit_level_mismatch_at_node (const unsigned int max_mismatch)
{
  bool flags_changed = false;


  // Vector holding the maximum element level that touches a node.
  std::vector<unsigned char> max_level_at_node (_mesh.n_nodes(), 0);
  std::vector<unsigned char> max_p_level_at_node (_mesh.n_nodes(), 0);


  // Loop over all the active elements & fill the vector
  {
    MeshBase::element_iterator       elem_it  = _mesh.active_elements_begin();
    const MeshBase::element_iterator elem_end = _mesh.active_elements_end(); 
    
    for (; elem_it != elem_end; ++elem_it)
      {
	const Elem* elem = *elem_it;	
	const unsigned char elem_level =
	  elem->level() + ((elem->refinement_flag() == Elem::REFINE) ? 1 : 0);
	const unsigned char elem_p_level =
	  elem->p_level() + ((elem->p_refinement_flag() == Elem::REFINE) ? 1 : 0);

	// Set the max_level at each node
	for (unsigned int n=0; n<elem->n_nodes(); n++)
	  {
	    const unsigned int node_number = elem->node(n);

	    assert (node_number < max_level_at_node.size());
	    
	    max_level_at_node[node_number] =
	      std::max (max_level_at_node[node_number], elem_level);
	    max_p_level_at_node[node_number] =
	      std::max (max_p_level_at_node[node_number], elem_p_level);
	  }
      }     
  }


  // Now loop over the active elements and flag the elements
  // who violate the requested level mismatch
  {
    MeshBase::element_iterator       elem_it  = _mesh.active_elements_begin();
    const MeshBase::element_iterator elem_end = _mesh.active_elements_end(); 
    
    for (; elem_it != elem_end; ++elem_it)
      {
	Elem* elem = *elem_it;	
	const unsigned int elem_level = elem->level();
	const unsigned int elem_p_level = elem->p_level();
	
	// Skip the element if it is already fully flagged
	if (elem->refinement_flag() == Elem::REFINE &&
            elem->p_refinement_flag() == Elem::REFINE)
	  continue;

	// Loop over the nodes, check for possible mismatch
	for (unsigned int n=0; n<elem->n_nodes(); n++)
	  {
	    const unsigned int node_number = elem->node(n);

	    // Flag the element for refinement if it violates
	    // the requested level mismatch
	    if ( (elem_level + max_mismatch) < max_level_at_node[node_number]
                 && elem->refinement_flag() != Elem::REFINE)
	      {
		elem->set_refinement_flag (Elem::REFINE);
		flags_changed = true;
	      }
	    if ( (elem_p_level + max_mismatch) < max_p_level_at_node[node_number]
                 && elem->p_refinement_flag() != Elem::REFINE)
	      {
		elem->set_p_refinement_flag (Elem::REFINE);
		flags_changed = true;
	      }
	  }
      }     
  }
  
  return flags_changed;
}



//-----------------------------------------------------------------
// Mesh refinement methods
bool MeshRefinement::limit_level_mismatch_at_edge (const unsigned int max_mismatch)
{
  bool flags_changed = false;


  // Maps holding the maximum element level that touches an edge
  std::map<std::pair<unsigned int, unsigned int>, unsigned char>
    max_level_at_edge;
  std::map<std::pair<unsigned int, unsigned int>, unsigned char>
    max_p_level_at_edge;

  // Loop over all the active elements & fill the maps
  {
    MeshBase::element_iterator       elem_it  = _mesh.active_elements_begin();
    const MeshBase::element_iterator elem_end = _mesh.active_elements_end(); 
    
    for (; elem_it != elem_end; ++elem_it)
      {
	const Elem* elem = *elem_it;	
	const unsigned char elem_level =
	  elem->level() + ((elem->refinement_flag() == Elem::REFINE) ? 1 : 0);
	const unsigned char elem_p_level =
	  elem->p_level() + ((elem->p_refinement_flag() == Elem::REFINE) ? 1 : 0);

	// Set the max_level at each edge
	for (unsigned int n=0; n<elem->n_edges(); n++)
	  {
            AutoPtr<Elem> edge = elem->build_edge(n);
            unsigned int childnode0 = edge->node(0);
            unsigned int childnode1 = edge->node(1);
            if (childnode1 < childnode0)
              std::swap(childnode0, childnode1);

	    for (const Elem *p = elem; p != NULL; p = elem->parent())
	      {
                AutoPtr<Elem> pedge = p->build_edge(n);
		unsigned int node0 = pedge->node(0);
		unsigned int node1 = pedge->node(1);

		if (node0 != childnode0 && node0 != childnode1
		    && node1 != childnode0 && node1 != childnode1)
		  break;

                if (node1 < node0)
                  std::swap(node0, node1);
		
                std::pair<unsigned int, unsigned int> edge_key =
                  std::make_pair(node0, node1);

                if (max_level_at_edge.find(edge_key) ==
                    max_level_at_edge.end())
                  {
                    max_level_at_edge[edge_key] = elem_level;
	            max_p_level_at_edge[edge_key] = elem_p_level;
                  }
                else
                  {
	            max_level_at_edge[edge_key] =
	              std::max (max_level_at_edge[edge_key], elem_level);
	            max_p_level_at_edge[edge_key] =
	              std::max (max_p_level_at_edge[edge_key], elem_p_level);
                  }
              }
	  }
      }     
  }


  // Now loop over the active elements and flag the elements
  // who violate the requested level mismatch
  {
    MeshBase::element_iterator       elem_it  = _mesh.active_elements_begin();
    const MeshBase::element_iterator elem_end = _mesh.active_elements_end(); 
    
    for (; elem_it != elem_end; ++elem_it)
      {
	Elem* elem = *elem_it;	
	const unsigned int elem_level = elem->level();
	const unsigned int elem_p_level = elem->p_level();
	
	// Skip the element if it is already fully flagged
	if (elem->refinement_flag() == Elem::REFINE &&
            elem->p_refinement_flag() == Elem::REFINE)
	  continue;

	// Loop over the nodes, check for possible mismatch
	for (unsigned int n=0; n<elem->n_edges(); n++)
	  {
            AutoPtr<Elem> edge = elem->build_edge(n);
            unsigned int node0 = edge->node(0);
            unsigned int node1 = edge->node(1);
            if (node1 < node0)
              std::swap(node0, node1);

            std::pair<unsigned int, unsigned int> edge_key =
              std::make_pair(node0, node1);

	    // Flag the element for refinement if it violates
	    // the requested level mismatch
	    if ( (elem_level + max_mismatch) < max_level_at_edge[edge_key]
                 && elem->refinement_flag() != Elem::REFINE)
	      {
		elem->set_refinement_flag (Elem::REFINE);
		flags_changed = true;
	      }
	    if ( (elem_p_level + max_mismatch) < max_p_level_at_edge[edge_key]
                 && elem->p_refinement_flag() != Elem::REFINE)
	      {
		elem->set_p_refinement_flag (Elem::REFINE);
		flags_changed = true;
	      }
	  }
      }     
  }
  
  return flags_changed;
}




bool MeshRefinement::eliminate_unrefined_patches ()
{
  bool flags_changed = false;

  MeshBase::element_iterator       elem_it  = _mesh.active_elements_begin();
  const MeshBase::element_iterator elem_end = _mesh.active_elements_end(); 

  for (; elem_it != elem_end; ++elem_it)
    {
      Elem* elem = *elem_it;
      // First assume that we'll have to flag this element for both h
      // and p refinement, then change our minds if we see any
      // neighbors that are as coarse or coarser than us.
      bool h_flag_me = true,
           p_flag_me = true;


      // Skip the element if it is already fully flagged for refinement
      if (elem->p_refinement_flag() == Elem::REFINE)
	p_flag_me = false;
      if (elem->refinement_flag() == Elem::REFINE)
        {
          h_flag_me = false;
          if (!p_flag_me)
            continue;
        }
      // Test the parent if that is already flagged for coarsening
      else if (elem->refinement_flag() == Elem::COARSEN)
        {
          genius_assert(elem->parent());
	  elem = elem->parent();
          // FIXME - this doesn't seem right - RHS
          if (elem->refinement_flag() != Elem::COARSEN_INACTIVE)
            continue;
          p_flag_me = false;
        }

      const unsigned int my_level = elem->level();
      int my_p_adjustment = 0;
      if (elem->p_refinement_flag() == Elem::REFINE)
        my_p_adjustment = 1;
      else if (elem->p_refinement_flag() == Elem::COARSEN)
        {
          genius_assert(elem->p_level() > 0);
          my_p_adjustment = -1;
        }
      const unsigned int my_new_p_level = elem->p_level() +
                                          my_p_adjustment;

      // Check all the element neighbors
      for (unsigned int n=0; n<elem->n_neighbors(); n++)
        {
          const Elem *neighbor = elem->neighbor(n);
	  // Quit if the element is on the boundary
	  if (neighbor == NULL)
            {
              h_flag_me = false;
              p_flag_me = false;
              break;
            }
	  // if the neighbor will be equally or less refined
	  // than we are, then we do not need to h refine ourselves.
	  if (h_flag_me && 
              (neighbor->level() < my_level) ||
	      ((neighbor->active()) &&
	       (neighbor->refinement_flag() != Elem::REFINE))
              || (neighbor->refinement_flag() ==
                  Elem::COARSEN_INACTIVE))
            {
	      h_flag_me = false;
              if (!p_flag_me)
                break;
            }
	  if (p_flag_me)
            {
	      // if active neighbors will have a p level
	      // equal to or lower than ours, then we do not need to p
              // refine ourselves.
              if (neighbor->active())
                {
                  int p_adjustment = 0;
                  if (neighbor->p_refinement_flag() == Elem::REFINE)
                    p_adjustment = 1;
                  else if (neighbor->p_refinement_flag() == Elem::COARSEN)
                    {
                      genius_assert(neighbor->p_level() > 0);
                      p_adjustment = -1;
                    }
                  if (my_new_p_level >= neighbor->p_level() + p_adjustment)
                    {
                      p_flag_me = false;
                      if (!h_flag_me)
                        break;
                    }
                }
              // If we have inactive neighbors, we need to
              // test all their active descendants which neighbor us
              else
                {
                  if (neighbor->min_new_p_level_by_neighbor(elem,
                      my_new_p_level + 2) <= my_new_p_level)
                    {
                      p_flag_me = false;
                      if (!h_flag_me)
                        break;
                    }
                }
            }
        }

      if (h_flag_me)
	{
	  // Parents that would create islands should no longer
          // coarsen
          if (elem->refinement_flag() == Elem::COARSEN_INACTIVE)
            {
              for (unsigned int c=0; c<elem->n_children(); c++)
                {
                  genius_assert(elem->child(c)->refinement_flag() ==
                         Elem::COARSEN);
                  elem->child(c)->set_refinement_flag(Elem::DO_NOTHING);
                }
              elem->set_refinement_flag(Elem::INACTIVE);
            }
          else
	    elem->set_refinement_flag(Elem::REFINE);
	  flags_changed = true;
	}      
      if (p_flag_me)
	{
          if (elem->p_refinement_flag() == Elem::COARSEN)
	    elem->set_p_refinement_flag(Elem::DO_NOTHING);
          else 
	    elem->set_p_refinement_flag(Elem::REFINE);
	  flags_changed = true;
	}      
    }

  return flags_changed;
}


#endif
