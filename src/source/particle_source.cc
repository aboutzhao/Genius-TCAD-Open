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

#include <fstream>

#include "mesh.h"
#include "particle_source.h"
#include "simulation_system.h"
#include "simulation_region.h"
#include "semiconductor_region.h"
#include "interpolation_2d_csa.h"
#include "interpolation_3d_qshep.h"
#include "mathfunc.h"
#include "log.h"

using PhysicalUnit::s;
using PhysicalUnit::um;
using PhysicalUnit::eV;
using PhysicalUnit::cm;
using PhysicalUnit::g;

//-------------------------------------------------------------------------------------------------------------------------

Particle_Source_DataFile::Particle_Source_DataFile(SimulationSystem &system, const Parser::Card &c):Particle_Source(system)
{
  MESSAGE<<"Setting Radiation Source from data file..."; RECORD();

  genius_assert(c.key() == "PARTICLE");

  // set Particle profile here.
  if(c.is_enum_value("profile","fromfile2d"))
    set_particle_profile_fromfile2d(c);

  if(c.is_enum_value("profile","fromfile3d"))
    set_particle_profile_fromfile3d(c);

  _t0     = c.get_real("t0", 0.0)*s;
  _t_max  = c.get_real("tmax", 0.0)*s;
  _t_char = c.get_real("t.char", 2e-12)*s;
  _quan_eff = c.get_real("quan.eff", 3.6)*eV;

  MESSAGE<<"ok\n"<<std::endl; RECORD();

}



void Particle_Source_DataFile::set_particle_profile_fromfile2d(const Parser::Card &c)
{
  interpolator = AutoPtr<InterpolationBase>(new Interpolation2D_CSA);
  interpolator->set_interpolation_type(0, InterpolationBase::Asinh);

  if(Genius::processor_id()==0)
  {
    VectorValue<double> translate(c.get_real("translate.x", 0.0) * um,
                                  c.get_real("translate.y", 0.0) * um,
                                  c.get_real("translate.z", 0.0) * um);
    TensorValue<double> transform(c.get_real("transform.xx", 1.0), c.get_real("transform.xy", 0.0), c.get_real("transform.xz", 0.0),
                                  c.get_real("transform.yx", 0.0), c.get_real("transform.yy", 1.0), c.get_real("transform.yz", 0.0),
                                  c.get_real("transform.zx", 0.0), c.get_real("transform.zy", 0.0), c.get_real("transform.zz", 1.0));

    std::string file = c.get_string("profile.file", "");
    std::ifstream in(file.c_str());
    if(!in.good())
    {
      MESSAGE<<"ERROR at " << c.get_fileline() <<" PARTICLE: file "<<file<<" can't be opened."<<std::endl; RECORD();
      genius_error();
    }

    Point p;
    double energy;

    while(!in.eof())
    {
      in >> p[0];// read x location
      in >> p[1];// read y location
      p *= um;     // scale to um
      p = transform*p + translate; //do transform & translate
      in >> energy;
      energy *= eV/(pow(um,3));
      if(!in.fail())
        interpolator->add_scatter_data(p, 0, energy);
    }
    if(!in.eof())
    {
      MESSAGE<<"ERROR at " << c.get_fileline() <<": error reading data file " << file << std::endl; RECORD();
      genius_error();
    }
    in.close();
  }

  interpolator->broadcast(0);
  interpolator->setup(0);
}


void Particle_Source_DataFile::set_particle_profile_fromfile3d(const Parser::Card &c)
{
  interpolator = AutoPtr<InterpolationBase>(new Interpolation3D_qshep);
  interpolator->set_interpolation_type(0, InterpolationBase::Asinh);


  VectorValue<double> translate(c.get_real("translate.x", 0.0) * um,
                                c.get_real("translate.y", 0.0) * um,
                                c.get_real("translate.z", 0.0) * um);
  TensorValue<double> transform(c.get_real("transform.xx", 1.0), c.get_real("transform.xy", 0.0), c.get_real("transform.xz", 0.0),
                                c.get_real("transform.yx", 0.0), c.get_real("transform.yy", 1.0), c.get_real("transform.yz", 0.0),
                                c.get_real("transform.zx", 0.0), c.get_real("transform.zy", 0.0), c.get_real("transform.zz", 1.0));

  if(Genius::processor_id()==0)
  {
    std::string file = c.get_string("profile.file", "");
    std::ifstream in(file.c_str());
    if(!in.good())
    {
      MESSAGE<<"ERROR at " << c.get_fileline() <<" PARTICLE: file "<<file<<" can't be opened."<<std::endl; RECORD();
      genius_error();
    }

    Point p;
    double energy;

    while(!in.eof())
    {
      in >> p[0];// read x location
      in >> p[1];// read y location
      in >> p[2];// read z location
      p *= um;     // scale to um
      p = transform*p + translate; //do transform & translate
      in >> energy;
      energy *= eV/(pow(um,3));
      if(!in.fail())
        interpolator->add_scatter_data(p, 0, energy);
    }
    if(!in.eof())
    {
      MESSAGE<<"ERROR at " << c.get_fileline() <<": error reading data file " << file << std::endl; RECORD();
      genius_error();
    }
    in.close();
  }
  interpolator->broadcast(0);
  interpolator->setup(0);
}


void Particle_Source_DataFile::update_system()
{
  // interpolate to mesh node
  for(unsigned int n=0; n<_system.n_regions(); n++)
  {
    SimulationRegion * region = _system.region(n);

    if( region->type() != SemiconductorRegion ) continue;

    SimulationRegion::node_iterator node_it = region->nodes_begin();
    for(; node_it != region->nodes_end(); ++node_it)
    {
      FVM_Node * fvm_node = node_it->second;

      if(!fvm_node->on_local()) continue;

      FVM_NodeData * node_data = fvm_node->node_data();
      double E = interpolator->get_interpolated_value(*(fvm_node->root_node()), 0);
#ifdef CYGWIN
      node_data->PatG() += 2*E/_quan_eff/_t_char/sqrt(3.1415926536)/(1+Erf((_t_max-_t0)/_t_char));
#else
      node_data->PatG() += 2*E/_quan_eff/_t_char/sqrt(3.1415926536)/(1+erf((_t_max-_t0)/_t_char));
#endif

    }
  }
}


double Particle_Source_DataFile::carrier_generation(double t)
{
  if( t>= _t0 && (t-_t_max)*(t-_t_max)/(_t_char*_t_char)<30)
    return exp(-(t-_t_max)*(t-_t_max)/(_t_char*_t_char));
  return 0;
}


//-------------------------------------------------------------------------------------------------------------------------

Particle_Source_Analytic::Particle_Source_Analytic(SimulationSystem &system, const Parser::Card &c):Particle_Source(system)
{
  MESSAGE<<"Setting Radiation Source from analytic expression..."; RECORD();

  genius_assert(c.key() == "PARTICLE");

  _start.x() = c.get_real("x", 0.0)*um;
  _start.y() = c.get_real("y", 0.0)*um;
  _start.z() = c.get_real("z", 0.0)*um;

  // ray direction
  double phi   = c.get_real("k.phi",   0.0)/180.0*3.14159265358979323846;
  double theta = c.get_real("k.theta", 0.0)/180.0*3.14159265358979323846;
  _dir = Point(sin(phi)*cos(theta), cos(phi), sin(phi)*sin(theta));

  _t0     = c.get_real("t0", 0.0)*s;
  _t_max  = c.get_real("tmax", 0.0)*s;
  _t_char = c.get_real("t.char", 2e-12)*s;
  _quan_eff = c.get_real("quan.eff", 3.6)*eV;

  _lateral_char = c.get_real("lateral.char", 0.1)*um;
  _length = c.get_real("length", 50.0)*um;

  _LET = c.get_real("let", 0.0)*1e6*eV*cm*cm/(g/1e3);

  MESSAGE<<"ok\n"<<std::endl; RECORD();
}


double Particle_Source_Analytic::carrier_generation(double t)
{
  if( t>= _t0 && (t-_t_max)*(t-_t_max)/(_t_char*_t_char)<30)
    return exp(-(t-_t_max)*(t-_t_max)/(_t_char*_t_char));
  return 0;
}



void Particle_Source_Analytic::update_system()
{
  const double pi = 3.1415926536;
  bool is_2d = _system.mesh().mesh_dimension() == 2;

  for(unsigned int n=0; n<_system.n_regions(); n++)
  {
    SimulationRegion * region = _system.region(n);

    if( region->type() != SemiconductorRegion ) continue;

    SemiconductorSimulationRegion * semi_region = dynamic_cast<SemiconductorSimulationRegion *>(region);
    double E = _LET*semi_region->material()->basic->Density(semi_region->T_external());

    double lateral_norm;
    if(is_2d)
      lateral_norm = sqrt(pi)*_lateral_char*1.0*um;
    else
      lateral_norm = pi*_lateral_char*_lateral_char;

#ifdef CYGWIN
    double G0 = E/_quan_eff/(lateral_norm)/(_t_char/2.0*sqrt(pi)*(1+Erf((_t_max-_t0)/_t_char)));
#else
    double G0 = E/_quan_eff/(lateral_norm)/(_t_char/2.0*sqrt(pi)*(1+erf((_t_max-_t0)/_t_char)));
#endif

    SimulationRegion::node_iterator node_it = semi_region->nodes_begin();
    for(; node_it != semi_region->nodes_end(); ++node_it)
    {
      FVM_Node * fvm_node = node_it->second;

      if(!fvm_node->on_local()) continue;

      Point  p = (*fvm_node->root_node());
      double d = (p-_start).dot(_dir);
      double r = (p-_start - (p-_start).dot(_dir)*_dir).size();

      FVM_NodeData * node_data = fvm_node->node_data();
      if(d < _length)
        node_data->PatG() += G0*exp(-r*r/(_lateral_char*_lateral_char));
      else
        node_data->PatG() += 0;
    }
  }

}



//-------------------------------------------------------------------------------------------------------------------------

