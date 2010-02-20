
// $Id: mesh_refinement_flagging.cc,v 1.2 2008/03/20 13:33:40 gdiso Exp $

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



// only compile these functions if the user requests AMR support
// Local Includes -----------------------------------
#include "genius_common.h"

#ifdef ENABLE_AMR

// C++ includes
#include <algorithm> // for std::sort

// Local includes
#include "error_vector.h"
#include "mesh_refinement.h"
#include "mesh_base.h"
#include "elem.h"



//-----------------------------------------------------------------
// Mesh refinement methods
void MeshRefinement::flag_elements_by_error_fraction (const ErrorVector& error_per_cell,
						      const Real refine_frac,
						      const Real coarsen_frac,
						      const unsigned int max_l)
{
  // The function arguments are currently just there for
  // backwards_compatibility
  if (!_use_member_parameters)
  {
    _refine_fraction = refine_frac;
    _coarsen_fraction = coarsen_frac;
    _max_h_level = max_l;
  }

  // Check for valid fractions..
  // The fraction values must be in [0,1]
  assert (_refine_fraction  >= 0. && _refine_fraction  <= 1.);
  assert (_coarsen_fraction >= 0. && _coarsen_fraction <= 1.);

  // Clean up the refinement flags.  These could be left
  // over from previous refinement steps.
  this->clean_refinement_flags();

  // We're getting the minimum and maximum error values
  // for the ACTIVE elements
  Real error_min = 1.e30;
  Real error_max = 0.;

  // And, if necessary, for their parents
  Real parent_error_min = 1.e30;
  Real parent_error_max = 0.;

  // Prepare another error vector if we need to sum parent errors
  ErrorVector error_per_parent;
  if (_coarsen_by_parents)
  {
    create_parent_error_vector(error_per_cell,
			       error_per_parent,
			       parent_error_min,
			       parent_error_max);
  }

  // We need to loop over all active elements to find the minimum
  MeshBase::element_iterator       elem_it  = _mesh.active_elements_begin();
  const MeshBase::element_iterator elem_end = _mesh.active_elements_end();

  for (; elem_it != elem_end; ++elem_it)
  {
    const unsigned int id  = (*elem_it)->id();
    assert (id < error_per_cell.size());

    error_max = std::max (error_max, error_per_cell[id]);
    error_min = std::min (error_min, error_per_cell[id]);
  }

  // Compute the cutoff values for coarsening and refinement
  const Real error_delta = (error_max - error_min);
  const Real parent_error_delta = parent_error_max - parent_error_min;

  const Real refine_cutoff  = (1.0 - _refine_fraction)*error_max;
  const Real coarsen_cutoff = _coarsen_fraction*error_delta + error_min;
  const Real parent_cutoff  = _coarsen_fraction*parent_error_delta + error_min;

//   // Print information about the error
//   std::cout << " Error Information:"                     << std::endl
// 	    << " ------------------"                     << std::endl
// 	    << "   min:              " << error_min      << std::endl
// 	    << "   max:              " << error_max      << std::endl
// 	    << "   delta:            " << error_delta    << std::endl
// 	    << "     refine_cutoff:  " << refine_cutoff  << std::endl
// 	    << "     coarsen_cutoff: " << coarsen_cutoff << std::endl;



  // Loop over the elements and flag them for coarsening or
  // refinement based on the element error

  elem_it  = _mesh.active_elements_begin();

  for (; elem_it != elem_end; ++elem_it)
  {
    Elem* elem             = *elem_it;
    const unsigned int id  = elem->id();

    assert (id < error_per_cell.size());

    const float elem_error = error_per_cell[id];

    if (_coarsen_by_parents)
    {
      Elem* parent           = elem->parent();
      if (parent)
      {
	const unsigned int parentid  = parent->id();
	if (error_per_parent[parentid] >= 0. &&
	    error_per_parent[parentid] <= parent_cutoff)
	  elem->set_refinement_flag(Elem::COARSEN);
      }
    }
    // Flag the element for coarsening if its error
    // is <= coarsen_fraction*delta + error_min
    else if (elem_error <= coarsen_cutoff)
    {
      elem->set_refinement_flag(Elem::COARSEN);
    }

    // Flag the element for refinement if its error
    // is >= refinement_cutoff.
    if (elem_error >= refine_cutoff)
      if (elem->level() < _max_h_level)
	elem->set_refinement_flag(Elem::REFINE);
  }
}



void MeshRefinement::flag_elements_by_error_tolerance (const ErrorVector& error_per_cell_in)
{
  // Check for valid fractions..
  // The fraction values must be in [0,1]
  assert (_coarsen_threshold  >= 0. && _refine_fraction  <= 1.);
  assert (_refine_fraction  >= 0. && _refine_fraction  <= 1.);
  assert (_coarsen_fraction >= 0. && _coarsen_fraction <= 1.);

  // How much error per cell will we tolerate?
  const Real local_refinement_tolerance =
    _absolute_global_tolerance / std::sqrt(static_cast<Real>(_mesh.n_active_elem()));
  const Real local_coarsening_tolerance =
    local_refinement_tolerance * _coarsen_threshold;

  // Prepare another error vector if we need to sum parent errors
  ErrorVector error_per_parent;
  if (_coarsen_by_parents)
  {
    Real parent_error_min, parent_error_max;

    create_parent_error_vector(error_per_cell_in,
			       error_per_parent,
			       parent_error_min,
			       parent_error_max);
  }

  MeshBase::element_iterator       elem_it  = _mesh.active_elements_begin();
  const MeshBase::element_iterator elem_end = _mesh.active_elements_end();

  for (; elem_it != elem_end; ++elem_it)
  {
    Elem* elem = *elem_it;
    Elem* parent = elem->parent();
    const unsigned int elem_number = elem->id();
    const float        elem_error  = error_per_cell_in[elem_number];

    if (elem_error > local_refinement_tolerance &&
	elem->level() < _max_h_level)
      elem->set_refinement_flag(Elem::REFINE);

    if (!_coarsen_by_parents && elem_error <
	local_coarsening_tolerance)
      elem->set_refinement_flag(Elem::COARSEN);

    if (_coarsen_by_parents && parent)
    {
      float parent_error = error_per_parent[parent->id()];
      if (parent_error >= 0.)
      {
	const Real parent_coarsening_tolerance =
	  std::sqrt(parent->n_children() *
		    local_coarsening_tolerance *
		    local_coarsening_tolerance);
	if (parent_error < parent_coarsening_tolerance)
	  elem->set_refinement_flag(Elem::COARSEN);
      }
    }
  }
}



void MeshRefinement::flag_elements_by_error_threshold (const ErrorVector& error_per_cell_in,
                                                       const Real refine_threshold,
					               const Real coarsen_threshold,
					               const unsigned int max_level)
{
  if (!_use_member_parameters)
  {
     _max_h_level = max_level;
  }

  // Prepare another error vector if we need to sum parent errors
  ErrorVector error_per_parent;
  if (_coarsen_by_parents)
  {
    Real parent_error_min, parent_error_max;

    create_parent_error_vector(error_per_cell_in,
			       error_per_parent,
			       parent_error_min,
			       parent_error_max);
  }

  MeshBase::element_iterator       elem_it  = _mesh.active_elements_begin();
  const MeshBase::element_iterator elem_end = _mesh.active_elements_end();

  for (; elem_it != elem_end; ++elem_it)
  {
    Elem* elem = *elem_it;
    Elem* parent = elem->parent();
    const unsigned int elem_number = elem->id();
    const float        elem_error  = error_per_cell_in[elem_number];

    if (elem_error > refine_threshold && elem->level() < _max_h_level)
      elem->set_refinement_flag(Elem::REFINE);

    if (!_coarsen_by_parents && elem_error < coarsen_threshold)
      elem->set_refinement_flag(Elem::COARSEN);

    if (_coarsen_by_parents && parent)
    {
      float parent_error = error_per_parent[parent->id()];
      if (parent_error >= 0.)
      {
	const Real parent_coarsening_tolerance = std::sqrt(parent->n_children() * coarsen_threshold * coarsen_threshold);
	if (parent_error < parent_coarsening_tolerance)
	  elem->set_refinement_flag(Elem::COARSEN);
      }
    }
  }
}



bool MeshRefinement::flag_elements_by_nelem_target (const ErrorVector& error_per_cell)
{
  // Check for valid fractions..
  // The fraction values must be in [0,1]
  assert (_refine_fraction  >= 0. && _refine_fraction  <= 1.);
  assert (_coarsen_fraction >= 0. && _coarsen_fraction <= 1.);

  // This function is currently only coded to work when coarsening by
  // parents - it's too hard to guess how many coarsenings will be
  // performed otherwise.
  assert (_coarsen_by_parents);

  // The number of active elements in the mesh - hopefully less than
  // 2 billion on 32 bit machines
  const unsigned int n_active_elem  = _mesh.n_active_elem();

  // The maximum number of active elements to flag for coarsening
  const unsigned int max_elem_coarsen =
    static_cast<unsigned int>(_coarsen_fraction * n_active_elem) + 1;

  // The maximum number of elements to flag for refinement
  const unsigned int max_elem_refine  =
    static_cast<unsigned int>(_refine_fraction  * n_active_elem) + 1;

  // Clean up the refinement flags.  These could be left
  // over from previous refinement steps.
  this->clean_refinement_flags();

  // The target number of elements to add or remove
  const int n_elem_new = _nelem_target - n_active_elem;

  // Create an vector with active element errors and ids,
  // sorted by highest errors first
  std::vector<std::pair<float, unsigned int> > sorted_error;

  sorted_error.reserve (n_active_elem);

  MeshBase::element_iterator       elem_it  = _mesh.active_elements_begin();
  const MeshBase::element_iterator elem_end = _mesh.active_elements_end();

  for (; elem_it != elem_end; ++elem_it)
    {
      unsigned int eid = (*elem_it)->id();
      genius_assert(eid < error_per_cell.size());
      sorted_error.push_back
        (std::make_pair(error_per_cell[eid], eid));
    }

  // Default sort works since pairs are sorted lexicographically
  std::sort (sorted_error.begin(), sorted_error.end());
  std::reverse (sorted_error.begin(), sorted_error.end());

  // Create a sorted error vector with coarsenable parent elements
  // only, sorted by lowest errors first
  ErrorVector error_per_parent;
  std::vector<std::pair<float, unsigned int> > sorted_parent_error;
  Real parent_error_min, parent_error_max;

  create_parent_error_vector(error_per_cell,
                             error_per_parent,
                             parent_error_min,
                             parent_error_max);

  // create_parent_error_vector sets values for non-parents and
  // non-coarsenable parents to -1.  Get rid of them.
  for (unsigned int i=0; i != error_per_parent.size(); ++i)
    if (error_per_parent[i] != -1)
      sorted_parent_error.push_back(std::make_pair(error_per_parent[i], i));

  std::sort (sorted_parent_error.begin(), sorted_parent_error.end());

  // Keep track of how many elements we plan to coarsen & refine
  unsigned int coarsen_count = 0;
  unsigned int refine_count = 0;

  const unsigned int dim = _mesh.mesh_dimension();
  unsigned int twotodim = 1;
  for (unsigned int i=0; i!=dim; ++i)
    twotodim *= 2;

  // First, let's try to get our element count to target_nelem
  if (n_elem_new >= 0)
  {
    // Every element refinement creates at least
    // 2^dim-1 new elements
    refine_count =
      std::min(static_cast<unsigned int>(n_elem_new / (twotodim-1)),
	       max_elem_refine);
  }
  else
  {
    // Every successful element coarsening is likely to destroy
    // 2^dim-1 net elements.
    coarsen_count =
      std::min(static_cast<unsigned int>(-n_elem_new / (twotodim-1)),
	       max_elem_coarsen);
  }

  // Next, let's see if we can trade any refinement for coarsening
  while (coarsen_count < max_elem_coarsen &&
         refine_count < max_elem_refine &&
         coarsen_count < sorted_parent_error.size() &&
         refine_count < sorted_error.size() &&
         sorted_error[refine_count].first >
	 sorted_parent_error[coarsen_count].first * _coarsen_threshold)
  {
    coarsen_count++;
    refine_count++;
  }

  if (refine_count > max_elem_refine)
    refine_count = max_elem_refine;
  unsigned int successful_refine_count = 0;
  for (unsigned int i=0; i != sorted_error.size(); ++i)
    {
      if (successful_refine_count >= refine_count)
        break;

      unsigned int eid = sorted_error[i].second;
      Elem *elem = _mesh.elem(eid);
      if (elem->level() < _max_h_level)
        {
	  elem->set_refinement_flag(Elem::REFINE);
	  successful_refine_count++;
        }
    }

  // If we couldn't refine enough elements, don't coarsen too many
  // either
  if (coarsen_count < (refine_count - successful_refine_count))
    coarsen_count = 0;
  else
    coarsen_count -= (refine_count - successful_refine_count);

  if (coarsen_count > max_elem_coarsen)
    coarsen_count = max_elem_coarsen;
  unsigned int successful_coarsen_count = 0;
  for (unsigned int i=0; i != sorted_parent_error.size(); ++i)
    {
      if (successful_coarsen_count >= coarsen_count * twotodim)
        break;

      unsigned int parent_id = sorted_parent_error[i].second;
      Elem *parent = _mesh.elem(parent_id);
      for (unsigned int c=0; c != parent->n_children(); ++c)
        {
          Elem *elem = parent->child(c);
          if (elem->active())
            {
              elem->set_refinement_flag(Elem::COARSEN);
              successful_coarsen_count++;
            }
        }
    }

  // Return true if we've done all the AMR/C we can
  if (!successful_coarsen_count &&
      !successful_refine_count)
    return true;
  // And false if there may still be more to do.
  return false;
}



void MeshRefinement::flag_elements_by_elem_fraction (const ErrorVector& error_per_cell,
						     const Real refine_frac,
						     const Real coarsen_frac,
						     const unsigned int max_l)
{
  // The function arguments are currently just there for
  // backwards_compatibility
  if (!_use_member_parameters)
  {
    _refine_fraction = refine_frac;
    _coarsen_fraction = coarsen_frac;
    _max_h_level = max_l;
  }

  // Check for valid fractions..
  // The fraction values must be in [0,1]
  assert (_refine_fraction  >= 0. && _refine_fraction  <= 1.);
  assert (_coarsen_fraction >= 0. && _coarsen_fraction <= 1.);

  // The number of active elements in the mesh
  const unsigned int n_active_elem  = _mesh.n_elem();

  // The number of elements to flag for coarsening
  const unsigned int n_elem_coarsen =
    static_cast<unsigned int>(_coarsen_fraction * n_active_elem);

  // The number of elements to flag for refinement
  const unsigned int n_elem_refine =
    static_cast<unsigned int>(_refine_fraction  * n_active_elem);



  // Clean up the refinement flags.  These could be left
  // over from previous refinement steps.
  this->clean_refinement_flags();


  // This vector stores the error and element number for all the
  // active elements.  It will be sorted and the top & bottom
  // elements will then be flagged for coarsening & refinement
  std::vector<float> sorted_error;

  sorted_error.reserve (n_active_elem);

  // Loop over the active elements and create the entry
  // in the sorted_error vector
  MeshBase::element_iterator       elem_it  = _mesh.active_elements_begin();
  const MeshBase::element_iterator elem_end = _mesh.active_elements_end();

  for (; elem_it != elem_end; ++elem_it)
    sorted_error.push_back (error_per_cell[(*elem_it)->id()]);

  // Now sort the sorted_error vector
  std::sort (sorted_error.begin(), sorted_error.end());

  // If we're coarsening by parents:
  // Create a sorted error vector with coarsenable parent elements
  // only, sorted by lowest errors first
  ErrorVector error_per_parent, sorted_parent_error;
  if (_coarsen_by_parents)
  {
    Real parent_error_min, parent_error_max;

    create_parent_error_vector(error_per_cell,
			       error_per_parent,
			       parent_error_min,
			       parent_error_max);

    sorted_parent_error = error_per_parent;
    std::sort (sorted_parent_error.begin(), sorted_parent_error.end());

    // All the other error values will be 0., so get rid of them.
    sorted_parent_error.erase (std::remove(sorted_parent_error.begin(),
					   sorted_parent_error.end(), 0.),
			       sorted_parent_error.end());
  }


  float top_error= 0., bottom_error = 0.;

  // Get the maximum error value corresponding to the
  // bottom n_elem_coarsen elements
  if (_coarsen_by_parents && n_elem_coarsen)
    {
      const unsigned int dim = _mesh.mesh_dimension();
      unsigned int twotodim = 1;
      for (unsigned int i=0; i!=dim; ++i)
        twotodim *= 2;

      unsigned int n_parent_coarsen = n_elem_coarsen / (twotodim - 1);

      if (n_parent_coarsen)
	bottom_error = sorted_parent_error[n_parent_coarsen - 1];
    }
  else if (n_elem_coarsen)
    {
      bottom_error = sorted_error[n_elem_coarsen - 1];
    }

  if (n_elem_refine)
    top_error = sorted_error[sorted_error.size() - n_elem_refine + 1];


  // Finally, let's do the element flagging
  elem_it  = _mesh.active_elements_begin();
  for (; elem_it != elem_end; ++elem_it)
    {
      Elem* elem = *elem_it;
      Elem* parent = elem->parent();

      if (_coarsen_by_parents && parent && n_elem_coarsen &&
          error_per_parent[parent->id()] <= bottom_error)
        elem->set_refinement_flag(Elem::COARSEN);

      if (!_coarsen_by_parents && n_elem_coarsen &&
          error_per_cell[elem->id()] <= bottom_error)
        elem->set_refinement_flag(Elem::COARSEN);

      if (n_elem_refine &&
          elem->level() < _max_h_level &&
          error_per_cell[elem->id()] >= top_error)
        elem->set_refinement_flag(Elem::REFINE);

    }
}



void MeshRefinement::flag_elements_by_mean_stddev (const ErrorVector& error_per_cell,
						   const Real refine_frac,
						   const Real coarsen_frac,
						   const unsigned int max_l)
{
  // The function arguments are currently just there for
  // backwards_compatibility
  if (!_use_member_parameters)
    {
      _refine_fraction = refine_frac;
      _coarsen_fraction = coarsen_frac;
      _max_h_level = max_l;
    }

  // Get the mean value from the error vector
  const Real mean = error_per_cell.mean();

  // Get the standard deviation.  This equals the
  // square-root of the variance
  const Real stddev = std::sqrt (error_per_cell.variance());

  // Check for valid fractions
  assert (_refine_fraction  >= 0. && _refine_fraction  <= 1.);
  assert (_coarsen_fraction >= 0. && _coarsen_fraction <= 1.);

  // The refine and coarsen cutoff
  const Real refine_cutoff  =  mean + _refine_fraction  * stddev;
  const Real coarsen_cutoff =  std::max(mean - _coarsen_fraction * stddev, 0.);

  // Loop over the elements and flag them for coarsening or
  // refinement based on the element error
  MeshBase::element_iterator       elem_it  = _mesh.active_elements_begin();
  const MeshBase::element_iterator elem_end = _mesh.active_elements_end();

  for (; elem_it != elem_end; ++elem_it)
    {
      Elem* elem             = *elem_it;
      const unsigned int id  = elem->id();

      assert (id < error_per_cell.size());

      const float elem_error = error_per_cell[id];

      // Possibly flag the element for coarsening ...
      if (elem_error <= coarsen_cutoff)
	elem->set_refinement_flag(Elem::COARSEN);

      // ... or refinement
      if ((elem_error >= refine_cutoff) && (elem->level() < _max_h_level))
	elem->set_refinement_flag(Elem::REFINE);
    }
}



void MeshRefinement::switch_h_to_p_refinement ()
{
  MeshBase::element_iterator       elem_it  = _mesh.elements_begin();
  const MeshBase::element_iterator elem_end = _mesh.elements_end();

  for ( ; elem_it != elem_end; ++elem_it)
    {
      if ((*elem_it)->active())
        {
          (*elem_it)->set_p_refinement_flag((*elem_it)->refinement_flag());
          (*elem_it)->set_refinement_flag(Elem::DO_NOTHING);
        }
      else
        {
          (*elem_it)->set_p_refinement_flag((*elem_it)->refinement_flag());
          (*elem_it)->set_refinement_flag(Elem::INACTIVE);
        }
    }
}



void MeshRefinement::add_p_to_h_refinement ()
{
  MeshBase::element_iterator       elem_it  = _mesh.elements_begin();
  const MeshBase::element_iterator elem_end = _mesh.elements_end();

  for ( ; elem_it != elem_end; ++elem_it)
    (*elem_it)->set_p_refinement_flag((*elem_it)->refinement_flag());
}



void MeshRefinement::clean_refinement_flags ()
{
  // Possibly clean up the refinement flags from
  // a previous step
//   elem_iterator       elem_it (_mesh.elements_begin());
//   const elem_iterator elem_end(_mesh.elements_end());

  MeshBase::element_iterator       elem_it  = _mesh.elements_begin();
  const MeshBase::element_iterator elem_end = _mesh.elements_end();

  for ( ; elem_it != elem_end; ++elem_it)
    {
      if ((*elem_it)->active())
        {
          (*elem_it)->set_refinement_flag(Elem::DO_NOTHING);
          (*elem_it)->set_p_refinement_flag(Elem::DO_NOTHING);
        }
      else
        {
          (*elem_it)->set_refinement_flag(Elem::INACTIVE);
          (*elem_it)->set_p_refinement_flag(Elem::INACTIVE);
        }
    }
}

#endif
