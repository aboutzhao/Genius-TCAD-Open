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

#ifndef __boundary_condition_ii_h__
#define __boundary_condition_ii_h__


#include "boundary_condition.h"

/**
 * The insulator-to-insulator interface, i.e. nitride SiO2-Si3N4-SiO2 gate
 */
class InsulatorInsulatorInterfaceBC : public BoundaryCondition
{
public:

  /**
   * constructor
   */
  InsulatorInsulatorInterfaceBC(SimulationSystem  & system, const std::string & label="");

  /**
   * destructor
   */
  virtual ~InsulatorInsulatorInterfaceBC(){}


  /**
   * @return boundary condition type
   */
  virtual BCType bc_type() const
    { return IF_Insulator_Insulator; }

  /**
   * @return boundary condition type in string
   */
  virtual std::string bc_type_name() const
  { return "IF_Insulator_Insulator"; }


  /**
   * @return boundary type
   */
  virtual BoundaryType boundary_type() const
    { return INTERFACE; }


  /**
   * @return true iff this boundary has a current flow
   */
  virtual bool has_current_flow() const
  { return false; }

  /**
   * @return the string which indicates the boundary condition
   */
  virtual std::string boundary_condition_in_string() const;

public:

#ifdef TCAD_SOLVERS

  //////////////////////////////////////////////////////////////////////////////////////////////
  //----------------Function and Jacobian evaluate for Poisson's Equation---------------------//
  //////////////////////////////////////////////////////////////////////////////////////////////

  /**
   * fill solution data and scaling constant into petsc vector
   */
  virtual void Poissin_Fill_Value(Vec x, Vec L);

  /**
   * preprocess Jacobian function for poisson solver
   */
  virtual void Poissin_Function_Preprocess(PetscScalar *, Vec, std::vector<PetscInt>&, std::vector<PetscInt>&, std::vector<PetscInt>&);

  /**
   * build function and its jacobian for poisson solver, nothing to do
   */
  virtual void Poissin_Function(PetscScalar * , Vec , InsertMode &);

  /**
   * preprocess Jacobian Matrix for poisson solver
   */
  virtual void Poissin_Jacobian_Preprocess(PetscScalar *, SparseMatrix<PetscScalar> *, std::vector<PetscInt>&, std::vector<PetscInt>&, std::vector<PetscInt>&);

  /**
   * build function and its jacobian for poisson solver, nothing to do
   */
  virtual void Poissin_Jacobian(PetscScalar * , SparseMatrix<PetscScalar> *, InsertMode &);



  //////////////////////////////////////////////////////////////////////////////////
  //----------------Function and Jacobian evaluate for L1 DDM---------------------//
  //////////////////////////////////////////////////////////////////////////////////

  /**
   * preprocess function for level 1 DDM solver
   */
  virtual void DDM1_Function_Preprocess(PetscScalar *, Vec, std::vector<PetscInt>&, std::vector<PetscInt>&, std::vector<PetscInt>&);

  /**
   * build function and its jacobian for DDML1 solver
   */
  virtual void DDM1_Function(PetscScalar * , Vec , InsertMode &);

  /**
   * preprocess Jacobian Matrix of level 1 DDM equation.
   */
  virtual void DDM1_Jacobian_Preprocess(PetscScalar *, SparseMatrix<PetscScalar> *, std::vector<PetscInt>&, std::vector<PetscInt>&, std::vector<PetscInt>&);

  /**
   * build function and its jacobian for DDML1 solver
   */
  virtual void DDM1_Jacobian(PetscScalar * , SparseMatrix<PetscScalar> *, InsertMode &);


  //////////////////////////////////////////////////////////////////////////////////
  //----------------Function and Jacobian evaluate for L2 DDM---------------------//
  //////////////////////////////////////////////////////////////////////////////////

  /**
   * preprocess function for level 2 DDM solver
   */
  virtual void DDM2_Function_Preprocess(PetscScalar * ,Vec, std::vector<PetscInt>&, std::vector<PetscInt>&, std::vector<PetscInt>&);

  /**
   * build function and its jacobian for DDML2 solver
   */
  virtual void DDM2_Function(PetscScalar * , Vec , InsertMode &);

  /**
   * preprocess Jacobian Matrix of level 2 DDM equation.
   */
  virtual void DDM2_Jacobian_Preprocess(PetscScalar *,SparseMatrix<PetscScalar> *, std::vector<PetscInt>&, std::vector<PetscInt>&, std::vector<PetscInt>&);

  /**
   * build function and its jacobian for DDML2 solver
   */
  virtual void DDM2_Jacobian(PetscScalar * , SparseMatrix<PetscScalar> *, InsertMode &);


  //////////////////////////////////////////////////////////////////////////////////
  //----------------Function and Jacobian evaluate for EBM   ---------------------//
  //////////////////////////////////////////////////////////////////////////////////

  /**
   * preprocess function for EBM3 solver
   */
  virtual void EBM3_Function_Preprocess(PetscScalar *,Vec, std::vector<PetscInt>&, std::vector<PetscInt>&, std::vector<PetscInt>&);

  /**
   * build function and its jacobian for EBM3 solver
   */
  virtual void EBM3_Function(PetscScalar * , Vec , InsertMode &);

  /**
   * preprocess Jacobian Matrix of EBM3 solver
   */
  virtual void EBM3_Jacobian_Preprocess(PetscScalar * ,SparseMatrix<PetscScalar> *, std::vector<PetscInt>&, std::vector<PetscInt>&, std::vector<PetscInt>&);

  /**
   * build function and its jacobian for EBM3 solver
   */
  virtual void EBM3_Jacobian(PetscScalar * , SparseMatrix<PetscScalar> *, InsertMode &);



  //////////////////////////////////////////////////////////////////////////////////
  //--------------Matrix and RHS Vector evaluate for DDM AC Solver----------------//
  //////////////////////////////////////////////////////////////////////////////////

  /**
   *  evaluating matrix and rhs vector for ddm ac solver
   */
  virtual void DDMAC_Fill_Matrix_Vector( Mat A, Vec b, const Mat J, const PetscScalar omega, InsertMode & add_value_flag );


#ifdef COGENDA_COMMERCIAL_PRODUCT
  //////////////////////////////////////////////////////////////////////////////////
  //----------------- functions for Gummel DDML1 solver --------------------------//
  //////////////////////////////////////////////////////////////////////////////////

  /**
   * function for reserve none zero pattern in petsc matrix.
   */
  virtual void DDM1_Half_Implicit_Current_Reserve(Mat A, InsertMode &add_value_flag);

  /**
   * function for preprocess build RHS and matrix for half implicit current continuity equation.
   */
  virtual void DDM1_Half_Implicit_Current_Preprocess(Vec f, Mat A, std::vector<PetscInt> &src,  std::vector<PetscInt> &dst, std::vector<PetscInt> &clear);

  /**
   * function for build RHS and matrix for half implicit current continuity equation.
   */
  virtual void DDM1_Half_Implicit_Current(PetscScalar * x, Mat A, Vec r, InsertMode &add_value_flag);


  /**
   * function for preprocess build RHS and matrix for half implicit poisson correction equation.
   */
  virtual void DDM1_Half_Implicit_Poisson_Correction_Preprocess(Vec f, std::vector<PetscInt> &src,  std::vector<PetscInt> &dst, std::vector<PetscInt> &clear);

  /**
   * function for build RHS and matrix for half implicit poisson correction equation.
   */
  virtual void DDM1_Half_Implicit_Poisson_Correction(PetscScalar * x, Mat A, Vec r, InsertMode &add_value_flag);

#endif

#endif


#ifdef IDC_SOLVERS
#ifdef COGENDA_COMMERCIAL_PRODUCT
  //////////////////////////////////////////////////////////////////////////////////
  //--------------Function and Jacobian evaluate for RIC Solver-------------------//
  //////////////////////////////////////////////////////////////////////////////////


  /**
   * fill solution data into petsc vector of RIC equation.
   */
  virtual void RIC_Fill_Value(Vec x, Vec L);

  /**
   * preprocess function for RIC solver
   */
  virtual void RIC_Function_Preprocess(PetscScalar *, Vec, std::vector<PetscInt>&, std::vector<PetscInt>&, std::vector<PetscInt>&);

  /**
   * build function and its jacobian for RIC solver
   */
  virtual void RIC_Function(PetscScalar * , Vec , InsertMode &);

  /**
   * preprocess Jacobian Matrix of RIC equation.
   */
  virtual void RIC_Jacobian_Preprocess(PetscScalar *, SparseMatrix<PetscScalar> *,std::vector<PetscInt>&, std::vector<PetscInt>&, std::vector<PetscInt>&);

  /**
   * build function and its jacobian for RIC solver
   */
  virtual void RIC_Jacobian(PetscScalar * , SparseMatrix<PetscScalar> *, InsertMode &);

  //////////////////////////////////////////////////////////////////////////////////
  //--------------Function and Jacobian evaluate for DICTAT Solver----------------//
  //////////////////////////////////////////////////////////////////////////////////


  /**
   * fill solution data into petsc vector of DICTAT equation.
   */
  virtual void DICTAT_Fill_Value(Vec x, Vec L);

  /**
   * preprocess function for DICTAT solver
   */
  virtual void DICTAT_Function_Preprocess(PetscScalar *, Vec, std::vector<PetscInt>&, std::vector<PetscInt>&, std::vector<PetscInt>&);

  /**
   * build function and its jacobian for DICTAT solver
   */
  virtual void DICTAT_Function(PetscScalar * , Vec , InsertMode &);

  /**
   * preprocess Jacobian Matrix of DICTAT equation.
   */
  virtual void DICTAT_Jacobian_Preprocess(PetscScalar *, SparseMatrix<PetscScalar> *, std::vector<PetscInt>&, std::vector<PetscInt>&, std::vector<PetscInt>&);

  /**
   * build function and its jacobian for DICTAT solver
   */
  virtual void DICTAT_Jacobian(PetscScalar * , SparseMatrix<PetscScalar> *, InsertMode &);

#endif
#endif
  
#ifdef COGENDA_COMMERCIAL_PRODUCT

  //////////////////////////////////////////////////////////////////////////////////
  //-------------- Function and Jacobian evaluate for TID Solver -----------------//
  //////////////////////////////////////////////////////////////////////////////////


  /**
   * fill vector of TID drift equation.
   */
  virtual void TID_Drift_Fill_Value(Vec , Vec );

  /**
   * preprocess for TID drift equation
   */
  virtual void TID_Drift_Function_Preprocess(PetscScalar *, Vec, std::vector<PetscInt>&, std::vector<PetscInt>&, std::vector<PetscInt>&);

  /**
   * evaluating TID drift equation
   */
  virtual void TID_Drift_Function(PetscScalar * x, Vec f, InsertMode &add_value_flag);
  
  /**
   * preprocess Jacobian Matrix of TID drift equation.
   */
  virtual void TID_Drift_Jacobian_Preprocess(PetscScalar *, SparseMatrix<PetscScalar> *, std::vector<PetscInt>&, std::vector<PetscInt>&, std::vector<PetscInt>&);
 
  /**
   * evaluating Jacobian of TID drift equation.
   */
  virtual void TID_Drift_Jacobian(PetscScalar * x, SparseMatrix<PetscScalar> *, InsertMode &add_value_flag);
  
  
  
  //////////////////////////////////////////////////////////////////////////////////
  //----------Function and Jacobian evaluate for TID drift-reaction Solver--------//
  //////////////////////////////////////////////////////////////////////////////////


  /**
   * fill vector of TID drift-reaction equation.
   */
  virtual void TID_DriftReaction_Fill_Value(Vec , Vec );

  /**
   * preprocess for TID drift-reaction equation
   */
  virtual void TID_DriftReaction_Function_Preprocess(PetscScalar *, Vec, std::vector<PetscInt>&, std::vector<PetscInt>&, std::vector<PetscInt>&);

  /**
   * evaluating TID drift-reaction equation
   */
  virtual void TID_DriftReaction_Function(PetscScalar * x, Vec f, InsertMode &add_value_flag);
  
  /**
   * preprocess Jacobian Matrix of TID drift-reaction equation.
   */
  virtual void TID_DriftReaction_Jacobian_Preprocess(PetscScalar *, SparseMatrix<PetscScalar> *, std::vector<PetscInt>&, std::vector<PetscInt>&, std::vector<PetscInt>&);
 
  /**
   * evaluating Jacobian of TID drift-reaction equation.
   */
  virtual void TID_DriftReaction_Jacobian(PetscScalar * x, SparseMatrix<PetscScalar> *, InsertMode &add_value_flag);

#endif

};







#endif

