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

//  $Id: solver_specify.h,v 1.4 2008/07/09 12:25:19 gdiso Exp $

#ifndef __solver_specify_h__
#define __solver_specify_h__


#include <string>
#include <vector>
#include <deque>
#include <string>


#include "enum_solver_specify.h"
#include "enum_petsc_type.h"
#include "key.h"


/**
 * this namespace stores information for controlling the solver.
 * genius fill these information by user's input deck and pass it to each solver.
 */
namespace SolverSpecify
{
  /**
   * enum value for specify which solver to be used, can be POISSON, DDML1 ..
   */
  extern SolverType     Solver;

  /**
   * enum value for specify which solution this solver will do. i.e. transient , steadystate...
   */
  extern SolutionType     Type;

  /**
   * label to identify this solve step
   */
  extern std::string      label;

  /**
   * the prefix string for output file
   */
  extern std::string      out_prefix;

  /**
   * hooks to be installed
   */
  extern std::deque<std::string> Hooks;

  /**
   * parameters for the hooks
   */
  extern std::map<std::string, std::vector<Parser::Parameter> > Hook_Parameters;

  /**
   * nonlinear solver scheme: basic, line search, trust region...
   */
  extern NonLinearSolverType     NS;

  /**
   * linear solver scheme: LU, BCGS, GMRES ...
   */
  extern LinearSolverType        LS;

  /**
   * preconditioner scheme: ASM ILU ...
   */
  extern PreconditionerType      PC;

  /**
   * Newton damping
   */
  extern DampingScheme     Damping;


  //--------------------------------------------
  // linear solver convergence criteria
  //--------------------------------------------

  /**
   * relative error tolerance
   */
  extern double   ksp_rtol;

  /**
   * abs error tolerance
   */
  extern double   ksp_atol;

  /**
   * abs error tolerance is set as max(ksp_atol_fnorm*fnorm, ksp_atol)
   * where fnorm is the nonlinear function norm
   */
  extern double   ksp_atol_fnorm;


  //--------------------------------------------
  // nonlinear solver convergence criteria
  //--------------------------------------------

   /**
   * Max nonlinear iteration number
   */
  extern unsigned int      MaxIteration;

  /**
   * When relative error of solution variable less
   * than this value, solution is considered converged.
   */
  extern double      relative_toler;

  /**
   * When relative error is used as converged criteria,
   * the equation norm should satisfy the absolute
   * converged criteria with a relaxation of
   * this value.
   */
  extern double      toler_relax;

  /**
   * The absolute converged criteria for the Poisson equation.
   */
  extern double      poisson_abs_toler;

  /**
   * The absolute converged criteria for the electron continuity equation.
   */
  extern double      elec_continuity_abs_toler;

  /**
   * The absolute converged criteria for the hole continuity equation.
   */
  extern double      hole_continuity_abs_toler;

  /**
   * The absolute converged criteria for the lattice heat equation equation.
   */
  extern double      heat_equation_abs_toler;

  /**
   * The absolute converged criteria for the electron energy balance equation.
   */
  extern double      elec_energy_abs_toler;

  /**
   * The absolute converged criteria for the hole energy balance equation.
   */
  extern double      hole_energy_abs_toler;

  /**
   * The absolute converged criteria for the electron quantum potential equation.
   */
  extern double      elec_quantum_abs_toler;

  /**
   * The absolute converged criteria for the hole quantum potential equation.
   */
  extern double      hole_quantum_abs_toler;

  /**
   * The absolute converged criteria for the electrode bias equation.
   */
  extern double      electrode_abs_toler;

  //--------------------------------------------
  // TS (transient solver)
  //--------------------------------------------

  /**
   * TS indicator
   */
  extern bool      TimeDependent;

  /**
   * transient scheme
   */
  extern TSType    TS_type;

  /**
   * start time of transient simulation
   */
  extern double    TStart;

  /**
   * user defined time step of transient simulation
   * it is just a reference value
   */
  extern double    TStep;

  /**
   * the maximum time step. TStep will not exceed this value
   */
  extern double    TStepMax;

  /**
   * stop time of transient simulation
   */
  extern double    TStop;

  /**
   * indicate if auto step control should be used
   */
  extern bool      AutoStep;

  /**
   * indicate if predict of next solution value should be used
   */
  extern bool      Predict;

  /**
   * relative tol of TS truncate error, used in AutoStep
   */
  extern double    TS_rtol;

  /**
   * absolute tol of TS truncate error, used in AutoStep
   */
  extern double    TS_atol;

  /**
   * indicate BDF2 can be started.
   */
  extern bool      BDF2_restart;

  /**
   * use initial condition, only for mixA solver
   */
  extern bool      UIC;

  /**
   * current time
   */
  extern double    clock;

  /**
   * current time step
   */
  extern double    dt;

  /**
   * last time step
   */
  extern double    dt_last;

  /**
   * previous time step
   */
  extern double    dt_last_last;

  /**
   *  the simulation cycles
   */
  extern int       T_Cycles;


  //------------------------------------------------------
  // parameters for DC and TRACE simulation
  //------------------------------------------------------

  /**
   * electrode(s) the voltage DC sweep will be performanced
   */
  extern std::vector<std::string>    Electrode_VScan;

  /**
   * start voltage of DC sweep
   */
  extern double    VStart;

  /**
   * voltage step
   */
  extern double    VStep;

  /**
   * max voltage step
   */
  extern double    VStepMax;

  /**
   * stop voltage of DC sweep
   */
  extern double    VStop;

  /**
   * electrode the current DC sweep will be performanced
   */
  extern std::vector<std::string>    Electrode_IScan;

  /**
   * start current of DC sweep
   */
  extern double    IStart;

  /**
   * current step
   */
  extern double    IStep;

  /**
   * max current step
   */
  extern double    IStepMax;

  /**
   * stop current
   */
  extern double    IStop;

  /**
   *  the simulation cycles
   */
  extern int       DC_Cycles;

  /**
   * ramp up the voltage/current sources in circuit, only for mixA solver
   */
  extern int       RampUpSteps;

  /**
   * the voltage step for ramp up
   */
  extern double    RampUpVStep;

  /**
   * the current step for ramp up
   */
  extern double    RampUpIStep;

  //------------------------------------------------------
  // parameters for MIX simulation
  //------------------------------------------------------

  /**
   * TCP port number
   */
  extern unsigned short int ServerPort;

  //------------------------------------------------------
  // parameters for AC simulation
  //------------------------------------------------------
  /**
   * electrode for AC small signal sweep
   */
  extern std::vector<std::string>     Electrode_ACScan;

  /**
   * the amplitude of small signal for AC sweep
   */
  extern double    VAC;

  /**
   * start frequency
   */
  extern double    FStart;

  /**
   * frequency multiple factor
   */
  extern double    FMultiple;

  /**
   * stop frequency
   */
  extern double    FStop;

  /**
   * current frequency
   */
  extern double    Freq;

  /**
   * when OptG is true,
   * the optical carrier generation is considered in the simulation
   */
  extern bool      OptG;

  /**
   * when PatG is true,
   * the particle carrier generation is considered in the simulation
   */
  extern bool      PatG;

  /**
   * constructor, set default values
   */
  extern void set_default_parameter();

  /**
   * convert string to enum
   */
  extern SolutionType type_string_to_enum(const std::string s);
}



#endif //#define __solver_specify_h__
