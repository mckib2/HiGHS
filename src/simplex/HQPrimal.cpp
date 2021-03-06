/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                       */
/*    This file is part of the HiGHS linear optimization suite           */
/*                                                                       */
/*    Written and engineered 2008-2020 at the University of Edinburgh    */
/*                                                                       */
/*    Available as open-source under the MIT License                     */
/*                                                                       */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/**@file simplex/HQPrimal.cpp
 * @brief
 * @author Julian Hall, Ivet Galabova, Qi Huangfu and Michael Feldmeier
 */
#include "simplex/HQPrimal.h"

#include <cassert>
#include <cstdio>
#include <iostream>

#include "io/HighsIO.h"
#include "lp_data/HConst.h"
#include "simplex/HSimplex.h"
#include "simplex/HSimplexDebug.h"
#include "simplex/SimplexTimer.h"
#include "util/HighsRandom.h"
#include "util/HighsUtils.h"

using std::runtime_error;

HighsStatus HQPrimal::solve() {
  HighsOptions& options = workHMO.options_;
  HighsSimplexInfo& simplex_info = workHMO.simplex_info_;
  HighsSimplexLpStatus& simplex_lp_status = workHMO.simplex_lp_status_;
  workHMO.scaled_model_status_ = HighsModelStatus::NOTSET;
  // Assumes that the LP has a positive number of rows, since
  // unconstrained LPs should be solved in solveLpSimplex
  bool positive_num_row = workHMO.simplex_lp_.numRow_ > 0;
  assert(positive_num_row);
  if (!positive_num_row) {
    HighsLogMessage(options.logfile, HighsMessageType::ERROR,
                    "HPrimal::solve called for LP with non-positive (%d) "
                    "number of constraints",
                    workHMO.simplex_lp_.numRow_);
    return HighsStatus::Error;
  }
  invertHint = INVERT_HINT_NO;

  // Setup aspects of the model data which are needed for solve() but better
  // left until now for efficiency reasons.
  // ToDo primal simplex version
  // setup_for_solve(workHMO);

  // Set SolveBailout to be true if control is to be returned immediately to
  // calling function
  // ToDo Move to Simplex
  //  SolveBailout = false;

  // Initialise working environment
  // Does LOTS, including initialisation of edge weights. Should only
  // be called if model dimension changes
  // ToDo primal simplex version
  // init();
  // initParallel();

  // ToDo primal simplex version
  // initialiseCost(workHMO, 1); //  model->initCost(1);
  assert(simplex_lp_status.has_fresh_invert);
  if (!simplex_lp_status.has_fresh_invert) {
    printf(
        "ERROR: Should enter with fresh INVERT - unless no_invert_on_optimal "
        "is set\n");
  }
  // Consider initialising edge weights - create Primal variants
  //
#ifdef HiGHSDEV
  //  printf("simplex_lp_status.has_dual_steepest_edge_weights 2 = %d;
  //  dual_edge_weight_mode = %d; DualEdgeWeightMode::STEEPEST_EDGE = %d\n",
  //	 simplex_lp_status.has_dual_steepest_edge_weights,
  // dual_edge_weight_mode, DualEdgeWeightMode::STEEPEST_EDGE);cout<<flush;
  //  printf("Edge weights known? %d\n",
  //  !simplex_lp_status.has_dual_steepest_edge_weights);cout<<flush;
#endif
  /*
  if (!simplex_lp_status.has_dual_steepest_edge_weights) {
    // Edge weights are not known
    // Set up edge weights according to dual_edge_weight_mode and
  initialise_dual_steepest_edge_weights
    // Using dual Devex edge weights
    // Zero the number of Devex frameworks used and set up the first one
    devex_index.assign(solver_num_tot, 0);
    initialiseDevexFramework();
    // Indicate that edge weights are known
    simplex_lp_status.has_dual_steepest_edge_weights = true;
  }
  */

  // ToDo Determine primal simplex phase from initial primal values
  //
  /*
  computePrimal(workHMO);
  compute_primal_infeasible_in_??(workHMO, &dualInfeasCount);
  solvePhase = ??InfeasCount > 0 ? 1 : 2;
  */
  solvePhase = 0;  // Frig to skip while (solvePhase) {*}
  assert(simplex_lp_status.has_primal_objective_value);
  simplex_info.updated_primal_objective_value =
      simplex_info.primal_objective_value;
  solve_bailout = false;
  // Possibly bail out immediately if iteration limit is current value
  if (bailout()) return HighsStatus::Warning;
    // Check that the model is OK to solve:
    //
    // Level 0 just checks the flags
    //
    // Level 1 also checks that the basis is OK and that the necessary
    // data in work* is populated.
    //
    // Level 2 (will) checks things like the nonbasic duals and basic
    // primal values
    //
    // Level 3 (will) checks expensive things like the INVERT and
    // steepeest edge weights
    //
    // ToDo Write primal simplex equivalent
    /*
  // ToDo Adapt debugOkForSolve to be used by primal
    if (debugOkForSolve(workHMO, solvePhase) == HighsDebugStatus::LOGICAL_ERROR)
    return HighsStatus::Error;
    */
#ifdef HiGHSDEV
    //  reportSimplexLpStatus(simplex_lp_status, "Before HQPrimal major solving
    //  loop");
#endif
  // The major solving loop

  // Initialise the iteration analysis. Necessary for strategy, but
  // much is for development and only switched on with HiGHSDEV
  // ToDo Move to simplex and adapt so it's OK for primal and dual
  //  iterationAnalysisInitialise();

  while (solvePhase) {
    /*
    int it0 = workHMO.iteration_counts_.simplex;
    switch (solvePhase) {
      case 1:
        analysis->simplexTimerStart(SimplexPrimalPhase1Clock);
        solvePhase1();
        analysis->simplexTimerStop(SimplexPrimalPhase1Clock);
        simplex_info.primal_phase1_iteration_count +=
    (workHMO.iteration_counts_.simplex - it0); break; case 2:
        analysis->simplexTimerStart(SimplexPrimalPhase2Clock);
        solvePhase2();
        analysis->simplexTimerStop(SimplexPrimalPhase2Clock);
        simplex_info.primal_phase2_iteration_count +=
    (workHMO.iteration_counts_.simplex - it0); break; case 4:
    break; default: solvePhase = 0; break;
    }
    // Jump for primal
    if (solvePhase == 4) break;
    // Possibly bail out
    if (SolveBailout) break;
    */
  }
  solvePhase = 2;
  assert(solve_bailout == false);
  analysis = &workHMO.simplex_analysis_;
  if (solvePhase == 2) {
    int it0 = workHMO.iteration_counts_.simplex;

    analysis->simplexTimerStart(SimplexPrimalPhase2Clock);
    solvePhase2();
    analysis->simplexTimerStop(SimplexPrimalPhase2Clock);

    simplex_info.primal_phase2_iteration_count +=
        (workHMO.iteration_counts_.simplex - it0);
    if (bailout()) return HighsStatus::Warning;
  }
  /*
  // ToDo Adapt debugOkForSolve to be used by primal
  if (debugOkForSolve(workHMO, solvePhase) == HighsDebugStatus::LOGICAL_ERROR)
    return HighsStatus::Error;
  */
  return HighsStatus::OK;
}

void HQPrimal::solvePhase2() {
  HighsSimplexInfo& simplex_info = workHMO.simplex_info_;
  HighsSimplexLpStatus& simplex_lp_status = workHMO.simplex_lp_status_;
  // When starting a new phase the (updated) primal objective function
  // value isn't known. Indicate this so that when the value
  // computed from scratch in build() isn't checked against the the
  // updated value
  simplex_lp_status.has_primal_objective_value = false;
  simplex_lp_status.has_dual_objective_value = false;
  // Set invertHint so that it's assigned when first tested
  invertHint = INVERT_HINT_NO;
  // Set solvePhase=2 so it's set if solvePhase2() is called directly
  solvePhase = 2;
  solve_bailout = false;
  // Possibly bail out immediately if iteration limit is current value
  if (bailout()) return;
  // Set up local copies of model dimensions
  solver_num_col = workHMO.simplex_lp_.numCol_;
  solver_num_row = workHMO.simplex_lp_.numRow_;
  solver_num_tot = solver_num_col + solver_num_row;

  analysis = &workHMO.simplex_analysis_;

  // Setup update limits
  simplex_info.update_limit =
      min(100 + solver_num_row / 100,
          1000);  // TODO: Consider allowing the dual limit to be used
  simplex_info.update_count = 0;

  // Setup local vectors
  col_aq.setup(solver_num_row);
  row_ep.setup(solver_num_row);
  row_ap.setup(solver_num_col);

  ph1SorterR.reserve(solver_num_row);
  ph1SorterT.reserve(solver_num_row);

#ifdef HiGHSDEV
  printf(
      "HQPrimal::solvePhase2 - WARNING: Not setting analysis->col_aq_density = "
      "0\n");
  printf(
      "HQPrimal::solvePhase2 - WARNING: Not setting analysis->row_ep_density = "
      "0\n");
#endif
  //  analysis->col_aq_density = 0;
  //  analysis->row_ep_density = 0;

  devexReset();

  no_free_columns = true;
  for (int iCol = 0; iCol < solver_num_tot; iCol++) {
    if (highs_isInfinity(-workHMO.simplex_info_.workLower_[iCol])) {
      if (highs_isInfinity(workHMO.simplex_info_.workUpper_[iCol])) {
        // Free column
        no_free_columns = false;
        break;
      }
    }
  }
#ifdef HiGHSDEV
  if (no_free_columns) {
    printf("Model has no free columns\n");
  } else {
    printf("Model has free columns\n");
  }
#endif

  // Setup other buffers

  HighsPrintMessage(workHMO.options_.output, workHMO.options_.message_level,
                    ML_DETAILED, "primal-phase2-start\n");
  // Main solving structure
  for (;;) {
    analysis->simplexTimerStart(IteratePrimalRebuildClock);
    primalRebuild();
    analysis->simplexTimerStop(IteratePrimalRebuildClock);

    if (isPrimalPhase1) {
      for (;;) {
        /* Primal phase 1 choose column */
        phase1ChooseColumn();
        if (columnIn == -1) {
          invertHint = INVERT_HINT_CHOOSE_COLUMN_FAIL;
          break;
        }

        /* Primal phase 1 choose row */
        phase1ChooseRow();
        if (rowOut == -1) {
          HighsLogMessage(workHMO.options_.logfile, HighsMessageType::ERROR,
                          "Primal phase 1 choose row failed");
          exit(0);
        }

        /* Primal phase 1 update */
        phase1Update();
        if (invertHint) {
          break;
        }
        if (bailout()) return;
      }
      /* Go to the next rebuild */
      if (invertHint) {
        /* Stop when the invert is new */
        if (simplex_lp_status.has_fresh_rebuild) {
          break;
        }
        continue;
      }
    }
    for (;;) {
      primalChooseColumn();
      if (columnIn == -1) {
        invertHint = INVERT_HINT_POSSIBLY_OPTIMAL;
        break;
      }
      primalChooseRow();
      if (rowOut == -1) {
        invertHint = INVERT_HINT_POSSIBLY_PRIMAL_UNBOUNDED;
        break;
      }
      primalUpdate();
      if (bailout()) return;
      if (invertHint) {
        break;
      }
    }
    // If the data are fresh from rebuild() and no flips have occurred, break
    // out of the outer loop to see what's ocurred
    if (simplex_lp_status.has_fresh_rebuild && num_flip_since_rebuild == 0)
      break;
  }
  // If bailing out, should have returned already
  assert(!solve_bailout);

  if (columnIn == -1) {
    HighsPrintMessage(workHMO.options_.output, workHMO.options_.message_level,
                      ML_DETAILED, "primal-optimal\n");
    HighsPrintMessage(workHMO.options_.output, workHMO.options_.message_level,
                      ML_DETAILED, "problem-optimal\n");
    workHMO.scaled_model_status_ = HighsModelStatus::OPTIMAL;
  } else {
    HighsPrintMessage(workHMO.options_.output, workHMO.options_.message_level,
                      ML_MINIMAL, "primal-unbounded\n");
    workHMO.scaled_model_status_ = HighsModelStatus::PRIMAL_UNBOUNDED;
  }
  computeDualObjectiveValue(workHMO);
}

void HQPrimal::primalRebuild() {
  HighsSimplexInfo& simplex_info = workHMO.simplex_info_;
  HighsSimplexLpStatus& simplex_lp_status = workHMO.simplex_lp_status_;

  // Record whether the update objective value should be tested. If
  // the objective value is known, then the updated objective value
  // should be correct - once the correction due to recomputing the
  // dual values has been applied.
  //
  // Note that computePrimalObjectiveValue sets
  // has_primal_objective_value
  //
  // Have to do this before INVERT, as this permutes the indices of
  // basic variables, and baseValue only corresponds to the new
  // ordering once computePrimal has been called
  const bool check_updated_objective_value =
      simplex_lp_status.has_primal_objective_value;
  double previous_primal_objective_value;
  if (check_updated_objective_value) {
    debugUpdatedObjectiveValue(workHMO, algorithm, solvePhase, "Before INVERT");
    previous_primal_objective_value =
        simplex_info.updated_primal_objective_value;
  } else {
    // Reset the knowledge of previous objective values
    debugUpdatedObjectiveValue(workHMO, algorithm, -1, "");
  }

  // Rebuild workHMO.factor_ - only if we got updates
  int sv_invertHint = invertHint;
  invertHint = INVERT_HINT_NO;
  // Possibly Rebuild workHMO.factor_
  bool reInvert = simplex_info.update_count > 0;
  if (!invert_if_row_out_negative) {
    // Don't reinvert if columnIn is negative [equivalently, if sv_invertHint ==
    // INVERT_HINT_POSSIBLY_OPTIMAL]
    if (sv_invertHint == INVERT_HINT_POSSIBLY_OPTIMAL) {
      assert(columnIn == -1);
      reInvert = false;
    }
  }
  if (reInvert) {
    analysis->simplexTimerStart(InvertClock);
    int rank_deficiency = computeFactor(workHMO);
    analysis->simplexTimerStop(InvertClock);
    if (rank_deficiency) {
      throw runtime_error("Primal reInvert: singular-basis-matrix");
    }
    simplex_info.update_count = 0;
  }
  analysis->simplexTimerStart(ComputeDualClock);
  computeDual(workHMO);
  analysis->simplexTimerStop(ComputeDualClock);

  analysis->simplexTimerStart(ComputePrimalClock);
  computePrimal(workHMO);
  analysis->simplexTimerStop(ComputePrimalClock);

  // Primal objective section
  analysis->simplexTimerStart(ComputePrObjClock);
  computePrimalObjectiveValue(workHMO);
  analysis->simplexTimerStop(ComputePrObjClock);

  if (check_updated_objective_value) {
    // Apply the objective value correction due to computing primal
    // values from scratch.
    const double primal_objective_value_correction =
        simplex_info.primal_objective_value - previous_primal_objective_value;
    simplex_info.updated_primal_objective_value +=
        primal_objective_value_correction;
    debugUpdatedObjectiveValue(workHMO, algorithm);
  }
  // Now that there's a new dual_objective_value, reset the updated
  // value
  simplex_info.updated_primal_objective_value =
      simplex_info.primal_objective_value;

  computeSimplexInfeasible(workHMO);
  // Determine whether simplex_info.num_primal_infeasibilities and
  // simplex_info.num_dual_infeasibilities can be used
  copySimplexInfeasible(workHMO);

  /* Whether to switch to primal phase 1 */
  isPrimalPhase1 = 0;
  HighsSolutionParams& scaled_solution_params = workHMO.scaled_solution_params_;
  if (scaled_solution_params.num_primal_infeasibilities > 0) {
    isPrimalPhase1 = 1;
    phase1ComputeDual();
  }

  reportRebuild(sv_invertHint);
#ifdef HiGHSDEV
  if (simplex_info.analyse_rebuild_time) {
    int total_rebuilds =
        analysis->simplexTimerNumCall(IteratePrimalRebuildClock);
    double total_rebuild_time =
        analysis->simplexTimerRead(IteratePrimalRebuildClock);
    printf(
        "Primal     rebuild %d (%1d) on iteration %9d: Total rebuild time %g\n",
        total_rebuilds, sv_invertHint, workHMO.iteration_counts_.simplex,
        total_rebuild_time);
  }
#endif
  num_flip_since_rebuild = 0;
  // Data are fresh from rebuild
  simplex_lp_status.has_fresh_rebuild = true;
}

void HQPrimal::primalChooseColumn() {
  HighsRandom& random = workHMO.random_;
  const int* jFlag = &workHMO.simplex_basis_.nonbasicFlag_[0];
  const int* jMove = &workHMO.simplex_basis_.nonbasicMove_[0];
  double* workDual = &workHMO.simplex_info_.workDual_[0];
  const double* workLower = &workHMO.simplex_info_.workLower_[0];
  const double* workUpper = &workHMO.simplex_info_.workUpper_[0];
  const double dualTolerance =
      workHMO.scaled_solution_params_.dual_feasibility_tolerance;

  analysis->simplexTimerStart(ChuzcPrimalClock);
  columnIn = -1;
  double bestInfeas = 0;
  if (no_free_columns) {
    const int numSection = 1;
    int startSection = random.integer() % numSection;
    int deltaCol = (solver_num_tot + numSection - 1) / numSection;
    int fromCol = startSection * deltaCol;
    int toCol = min(fromCol + deltaCol, solver_num_tot);
    int numPass = 1;
    //    printf("\nstartSection = %1d; deltaCol = %d\n", startSection,
    //    deltaCol);
    for (;;) {
      //      printf("CHUZC: %1d [%6d, %6d] %6d\n", numPass, fromCol, toCol,
      //      solver_num_tot);
      for (int iCol = fromCol; iCol < toCol; iCol++) {
        // Then look at dual infeasible
        if (jMove[iCol] * workDual[iCol] < -dualTolerance) {
          if (bestInfeas * devex_weight[iCol] < fabs(workDual[iCol])) {
            bestInfeas = fabs(workDual[iCol]) / devex_weight[iCol];
            columnIn = iCol;
          }
        }
      }
      if (columnIn >= 0 || numPass == numSection) {
        //	printf("Break from CHUZC after %d passes\n", numPass);
        break;
      }
      if (toCol == solver_num_tot) {
        fromCol = 0;
        toCol = deltaCol;
      } else {
        fromCol = toCol;
        toCol = min(fromCol + deltaCol, solver_num_tot);
      }
      numPass++;
    }
  } else {
    for (int iCol = 0; iCol < solver_num_tot; iCol++) {
      if (jFlag[iCol] && fabs(workDual[iCol]) > dualTolerance) {
        // Always take free
        // TODO: if we found free,
        // Then deal with it in dual phase 1
        if (workLower[iCol] <= -HIGHS_CONST_INF &&
            workUpper[iCol] >= HIGHS_CONST_INF) {
          columnIn = iCol;
          break;
        }
        // Then look at dual infeasible
        if (jMove[iCol] * workDual[iCol] < -dualTolerance) {
          if (bestInfeas * devex_weight[iCol] < fabs(workDual[iCol])) {
            bestInfeas = fabs(workDual[iCol]) / devex_weight[iCol];
            columnIn = iCol;
          }
        }
      }
    }
  }
  analysis->simplexTimerStop(ChuzcPrimalClock);
}

void HQPrimal::primalChooseRow() {
  const double* baseLower = &workHMO.simplex_info_.baseLower_[0];
  const double* baseUpper = &workHMO.simplex_info_.baseUpper_[0];
  double* baseValue = &workHMO.simplex_info_.baseValue_[0];
  const double primalTolerance =
      workHMO.scaled_solution_params_.primal_feasibility_tolerance;

  // Compute pivot column
  analysis->simplexTimerStart(FtranClock);
  col_aq.clear();
  col_aq.packFlag = true;
  workHMO.matrix_.collect_aj(col_aq, columnIn, 1);
#ifdef HiGHSDEV
  HighsSimplexInfo& simplex_info = workHMO.simplex_info_;
  if (simplex_info.analyse_iterations)
    analysis->operationRecordBefore(ANALYSIS_OPERATION_TYPE_FTRAN, col_aq,
                                    analysis->col_aq_density);
#endif
  workHMO.factor_.ftran(col_aq, analysis->col_aq_density,
                        analysis->pointer_serial_factor_clocks);
  analysis->simplexTimerStop(FtranClock);
#ifdef HiGHSDEV
  if (simplex_info.analyse_iterations)
    analysis->operationRecordAfter(ANALYSIS_OPERATION_TYPE_FTRAN, col_aq);
#endif

  const double local_col_aq_density = (double)col_aq.count / solver_num_row;
  analysis->updateOperationResultDensity(local_col_aq_density,
                                         analysis->col_aq_density);

  const bool check_dual = false;
  if (check_dual) {
    const double* workCost = &workHMO.simplex_info_.workCost_[0];
    const double* workDual = &workHMO.simplex_info_.workDual_[0];
    const int* basicIndex = &workHMO.simplex_basis_.basicIndex_[0];
    double check_dual_value = workCost[columnIn];
    for (int i = 0; i < col_aq.count; i++) {
      int row = col_aq.index[i];
      int col = basicIndex[row];
      double value = col_aq.array[row];
      double cost = workCost[col];
      check_dual_value -= value * cost;
      //    printf("Entry %2d: [%2d, %12g] Cost = %12g; check_dual_value =
      //    %12g\n", i, row, value, cost, check_dual_value);
    }
    thetaDual = workDual[columnIn];
    double dual_error =
        fabs(check_dual_value - thetaDual) / max(1.0, fabs(thetaDual));
    if (dual_error > 1e-8)
      printf("Checking dual: updated = %12g; direct = %12g; error = %12g\n",
             thetaDual, check_dual_value, dual_error);
  }

  analysis->simplexTimerStart(Chuzr1Clock);
  // Initialize
  rowOut = -1;

  // Choose row pass 1
  double alphaTol = workHMO.simplex_info_.update_count < 10
                        ? 1e-9
                        : workHMO.simplex_info_.update_count < 20 ? 1e-8 : 1e-7;
  const int* jMove = &workHMO.simplex_basis_.nonbasicMove_[0];
  int moveIn = jMove[columnIn];
  if (moveIn == 0) {
    // If there's still free in the N
    // We would report not-solved
    // Need to handle free
  }
  double relaxTheta = 1e100;
  double relaxSpace;
  for (int i = 0; i < col_aq.count; i++) {
    int index = col_aq.index[i];
    alpha = col_aq.array[index] * moveIn;
    if (alpha > alphaTol) {
      relaxSpace = baseValue[index] - baseLower[index] + primalTolerance;
      if (relaxSpace < relaxTheta * alpha) relaxTheta = relaxSpace / alpha;
    } else if (alpha < -alphaTol) {
      relaxSpace = baseValue[index] - baseUpper[index] - primalTolerance;
      if (relaxSpace > relaxTheta * alpha) relaxTheta = relaxSpace / alpha;
    }
  }
  analysis->simplexTimerStop(Chuzr1Clock);

  analysis->simplexTimerStart(Chuzr2Clock);
  double bestAlpha = 0;
  for (int i = 0; i < col_aq.count; i++) {
    int index = col_aq.index[i];
    alpha = col_aq.array[index] * moveIn;
    if (alpha > alphaTol) {
      // Positive pivotal column entry
      double tightSpace = baseValue[index] - baseLower[index];
      if (tightSpace < relaxTheta * alpha) {
        if (bestAlpha < alpha) {
          bestAlpha = alpha;
          rowOut = index;
        }
      }
    } else if (alpha < -alphaTol) {
      // Negative pivotal column entry
      double tightSpace = baseValue[index] - baseUpper[index];
      if (tightSpace > relaxTheta * alpha) {
        if (bestAlpha < -alpha) {
          bestAlpha = -alpha;
          rowOut = index;
        }
      }
    }
  }
  analysis->simplexTimerStop(Chuzr2Clock);
}

void HQPrimal::primalUpdate() {
  int* jMove = &workHMO.simplex_basis_.nonbasicMove_[0];
  double* workDual = &workHMO.simplex_info_.workDual_[0];
  const double* workLower = &workHMO.simplex_info_.workLower_[0];
  const double* workUpper = &workHMO.simplex_info_.workUpper_[0];
  const double* baseLower = &workHMO.simplex_info_.baseLower_[0];
  const double* baseUpper = &workHMO.simplex_info_.baseUpper_[0];
  double* workValue = &workHMO.simplex_info_.workValue_[0];
  double* baseValue = &workHMO.simplex_info_.baseValue_[0];
  const double primalTolerance =
      workHMO.scaled_solution_params_.primal_feasibility_tolerance;
  HighsSimplexInfo& simplex_info = workHMO.simplex_info_;

  // Compute thetaPrimal
  int moveIn = jMove[columnIn];
  //  int
  columnOut = workHMO.simplex_basis_.basicIndex_[rowOut];
  //  double
  alpha = col_aq.array[rowOut];
  //  double
  thetaPrimal = 0;
  if (alpha * moveIn > 0) {
    // Lower bound
    thetaPrimal = (baseValue[rowOut] - baseLower[rowOut]) / alpha;
  } else {
    // Upper bound
    thetaPrimal = (baseValue[rowOut] - baseUpper[rowOut]) / alpha;
  }

  // 1. Make sure it is inside bounds or just flip bound
  double lowerIn = workLower[columnIn];
  double upperIn = workUpper[columnIn];
  double valueIn = workValue[columnIn] + thetaPrimal;
  bool flipped = false;
  if (jMove[columnIn] == 1) {
    if (valueIn > upperIn + primalTolerance) {
      // Flip to upper
      workValue[columnIn] = upperIn;
      thetaPrimal = upperIn - lowerIn;
      flipped = true;
      jMove[columnIn] = -1;
    }
  } else if (jMove[columnIn] == -1) {
    if (valueIn < lowerIn - primalTolerance) {
      // Flip to lower
      workValue[columnIn] = lowerIn;
      thetaPrimal = lowerIn - upperIn;
      flipped = true;
      jMove[columnIn] = 1;
    }
  }

  analysis->simplexTimerStart(UpdatePrimalClock);
  for (int i = 0; i < col_aq.count; i++) {
    int index = col_aq.index[i];
    baseValue[index] -= thetaPrimal * col_aq.array[index];
  }
  analysis->simplexTimerStop(UpdatePrimalClock);

  simplex_info.updated_primal_objective_value +=
      workDual[columnIn] * thetaPrimal;

  // Why is the detailed primal infeasibility information needed?
  computeSimplexPrimalInfeasible(workHMO);
  copySimplexPrimalInfeasible(workHMO);

  // If flipped, then no need touch the pivots
  if (flipped) {
    rowOut = -1;
    numericalTrouble = 0;
    thetaDual = workDual[columnIn];
    iterationAnalysis();
    num_flip_since_rebuild++;
    return;
  }

  // Pivot in
  int sourceOut = alpha * moveIn > 0 ? -1 : 1;
  update_pivots(workHMO, columnIn, rowOut, sourceOut);

  baseValue[rowOut] = valueIn;

  analysis->simplexTimerStart(CollectPrIfsClock);
  // Check for any possible infeasible
  for (int iRow = 0; iRow < solver_num_row; iRow++) {
    if (baseValue[iRow] < baseLower[iRow] - primalTolerance) {
      invertHint = INVERT_HINT_PRIMAL_INFEASIBLE_IN_PRIMAL_SIMPLEX;
    } else if (baseValue[iRow] > baseUpper[iRow] + primalTolerance) {
      invertHint = INVERT_HINT_PRIMAL_INFEASIBLE_IN_PRIMAL_SIMPLEX;
    }
  }
  analysis->simplexTimerStop(CollectPrIfsClock);

  // 2. Now we can update the dual

  analysis->simplexTimerStart(BtranClock);
  row_ep.clear();
  row_ap.clear();
  row_ep.count = 1;
  row_ep.index[0] = rowOut;
  row_ep.array[rowOut] = 1;
  row_ep.packFlag = true;
#ifdef HiGHSDEV
  if (simplex_info.analyse_iterations)
    analysis->operationRecordBefore(ANALYSIS_OPERATION_TYPE_BTRAN_EP, row_ep,
                                    analysis->row_ep_density);
#endif
  workHMO.factor_.btran(row_ep, analysis->row_ep_density,
                        analysis->pointer_serial_factor_clocks);
#ifdef HiGHSDEV
  if (simplex_info.analyse_iterations)
    analysis->operationRecordAfter(ANALYSIS_OPERATION_TYPE_BTRAN_EP, row_ep);
#endif
  analysis->simplexTimerStop(BtranClock);
  //
  // PRICE
  //
  computeTableauRowFromPiP(workHMO, row_ep, row_ap);
  /*
#ifdef HiGHSDEV
  if (simplex_info.analyse_iterations) {
  analysis->operationRecordBefore(ANALYSIS_OPERATION_TYPE_PRICE_AP, row_ep,
analysis->row_ap_density); analysis->num_row_price++;
  }
#endif
  analysis->simplexTimerStart(PriceClock);
  workHMO.matrix_.priceByRowSparseResult(row_ap, row_ep);
  analysis->simplexTimerStop(PriceClock);
#ifdef HiGHSDEV
  if (simplex_info.analyse_iterations)
  analysis->operationRecordAfter(ANALYSIS_OPERATION_TYPE_PRICE_AP, row_ep);
#endif

  const double local_row_ep_density = (double)row_ep.count / solver_num_row;
  analysis->updateOperationResultDensity(local_row_ep_density,
analysis->row_ep_density);
  */
  analysis->simplexTimerStart(UpdateDualClock);
  //  double
  thetaDual = workDual[columnIn] / alpha;
  for (int i = 0; i < row_ap.count; i++) {
    int iCol = row_ap.index[i];
    workDual[iCol] -= thetaDual * row_ap.array[iCol];
  }
  for (int i = 0; i < row_ep.count; i++) {
    int iGet = row_ep.index[i];
    int iCol = iGet + solver_num_col;
    workDual[iCol] -= thetaDual * row_ep.array[iGet];
  }
  analysis->simplexTimerStop(UpdateDualClock);

  /* Update the devex weight */
  devexUpdate();

  // After dual update in primal simplex the dual objective value is not known
  workHMO.simplex_lp_status_.has_dual_objective_value = false;

  // updateVerify for primal
  numericalTrouble = 0;
  /*
  double aCol = fabs(alpha);
  double alphaRow;
  if (columnIn < workHMO.simplex_lp_.numCol_) {
    alphaRow = row_ap.array[columnIn];
  } else {
    alphaRow = row_ep.array[rowOut];
  }
  double aRow = fabs(alphaRow);
  double aDiff = fabs(aCol - aRow);
  numericalTrouble = aDiff / min(aCol, aRow);
  if (numericalTrouble > 1e-7)
    printf("Numerical check: alphaCol = %12g, alphaRow = a%12g, aDiff = a%12g:
  measure = %12g\n", alpha, alphaRow, aDiff, numericalTrouble);
  // Reinvert if the relative difference is large enough, and updates have been
  performed
  //  if (numericalTrouble > 1e-7 && workHMO.simplex_info_.update_count > 0)
  invertHint = INVERT_HINT_POSSIBLY_SINGULAR_BASIS;
  */
  // Dual for the pivot
  workDual[columnIn] = 0;
  workDual[columnOut] = -thetaDual;

  // Update workHMO.factor_ basis
  update_factor(workHMO, &col_aq, &row_ep, &rowOut, &invertHint);
  update_matrix(workHMO, columnIn, columnOut);
  if (simplex_info.update_count >= simplex_info.update_limit) {
    invertHint = INVERT_HINT_UPDATE_LIMIT_REACHED;
  }
  // Move this to Simplex class once it's created
  // simplex_method.record_pivots(columnIn, columnOut, alpha);
  workHMO.iteration_counts_.simplex++;

  /* Reset the devex when there are too many errors */
  if (num_bad_devex_weight > 3) {
    devexReset();
  }

  // Report on the iteration
  iterationAnalysis();
}

/* Compute the reduced cost for primal phase 1 with artificial cost. */
void HQPrimal::phase1ComputeDual() {
  /* Alias to problem size, tolerance and work arrays */
  const int nRow = workHMO.lp_.numRow_;
  const int nCol = workHMO.lp_.numCol_;
  const double dFeasTol =
      workHMO.scaled_solution_params_.primal_feasibility_tolerance;
  const double* baseLower = &workHMO.simplex_info_.baseLower_[0];
  const double* baseUpper = &workHMO.simplex_info_.baseUpper_[0];
  const double* baseValue = &workHMO.simplex_info_.baseValue_[0];

  analysis->simplexTimerStart(BtranClock);
  /* Setup artificial cost and compute pi with BTran */
  HVector buffer;
  buffer.setup(nRow);
  buffer.clear();
  for (int iRow = 0; iRow < nRow; iRow++) {
    buffer.index[iRow] = iRow;
    if (baseValue[iRow] < baseLower[iRow] - dFeasTol) {
      buffer.array[iRow] = -1.0;
    } else if (baseValue[iRow] > baseUpper[iRow] + dFeasTol) {
      buffer.array[iRow] = 1.0;
    } else {
      buffer.array[iRow] = 0.0;
    }
  }
#ifdef HiGHSDEV
  HighsSimplexInfo& simplex_info = workHMO.simplex_info_;
  if (simplex_info.analyse_iterations)
    analysis->operationRecordBefore(ANALYSIS_OPERATION_TYPE_BTRAN_EP, buffer,
                                    analysis->row_ep_density);
#endif
  workHMO.factor_.btran(buffer, 1, analysis->pointer_serial_factor_clocks);
#ifdef HiGHSDEV
  if (simplex_info.analyse_iterations)
    analysis->operationRecordAfter(ANALYSIS_OPERATION_TYPE_BTRAN_EP, buffer);
#endif
  analysis->simplexTimerStop(BtranClock);

  analysis->simplexTimerStart(PriceClock);
  /* The compute the phase 1 reduced cost for all variables by SpMV */
  HVector bufferLong;
  bufferLong.setup(nCol);
  bufferLong.clear();
#ifdef HiGHSDEV
  if (simplex_info.analyse_iterations) {
    analysis->operationRecordBefore(ANALYSIS_OPERATION_TYPE_PRICE_AP, buffer,
                                    0.0);
    analysis->num_col_price++;
  }
#endif
  workHMO.matrix_.priceByColumn(bufferLong, buffer);
#ifdef HiGHSDEV
  if (simplex_info.analyse_iterations)
    analysis->operationRecordAfter(ANALYSIS_OPERATION_TYPE_PRICE_AP, row_ap);
#endif
  analysis->simplexTimerStop(PriceClock);

  const int* nbFlag = &workHMO.simplex_basis_.nonbasicFlag_[0];
  double* workDual = &workHMO.simplex_info_.workDual_[0];
  for (int iSeq = 0; iSeq < nCol + nRow; iSeq++) {
    workDual[iSeq] = 0.0;
  }
  for (int iSeq = 0; iSeq < nCol; iSeq++) {
    if (nbFlag[iSeq]) workDual[iSeq] = -bufferLong.array[iSeq];
  }
  for (int iRow = 0, iSeq = nCol; iRow < nRow; iRow++, iSeq++) {
    if (nbFlag[iSeq]) workDual[iSeq] = -buffer.array[iRow];
  }

  /* Recompute number of dual infeasible variables with the phase 1 cost */
  computeSimplexDualInfeasible(workHMO);
  // Determine whether simplex_info.num_dual_infeasibilities can be used
  copySimplexDualInfeasible(workHMO);
}

/* Choose a pivot column for the phase 1 primal simplex method */
void HQPrimal::phase1ChooseColumn() {
  const int nSeq = workHMO.lp_.numCol_ + workHMO.lp_.numRow_;
  const int* nbMove = &workHMO.simplex_basis_.nonbasicMove_[0];
  const double* workDual = &workHMO.simplex_info_.workDual_[0];
  const double dDualTol =
      workHMO.scaled_solution_params_.dual_feasibility_tolerance;
  analysis->simplexTimerStart(ChuzcPrimalClock);
  double dBestScore = 0;
  columnIn = -1;
  for (int iSeq = 0; iSeq < nSeq; iSeq++) {
    double dMyDual = nbMove[iSeq] * workDual[iSeq];
    double dMyScore = dMyDual / devex_weight[iSeq];
    if (dMyDual < -dDualTol && dMyScore < dBestScore) {
      dBestScore = dMyScore;
      columnIn = iSeq;
    }
  }
  analysis->simplexTimerStop(ChuzcPrimalClock);
}

/* Choose a pivot row for the phase 1 primal simplex method */
void HQPrimal::phase1ChooseRow() {
  /* Alias to work arrays */
  const double dFeasTol =
      workHMO.scaled_solution_params_.primal_feasibility_tolerance;
  const double* baseLower = &workHMO.simplex_info_.baseLower_[0];
  const double* baseUpper = &workHMO.simplex_info_.baseUpper_[0];
  const double* baseValue = &workHMO.simplex_info_.baseValue_[0];

  /* Compute the transformed pivot column and update its density */
  analysis->simplexTimerStart(FtranClock);
  col_aq.clear();
  col_aq.packFlag = true;
  workHMO.matrix_.collect_aj(col_aq, columnIn, 1);
#ifdef HiGHSDEV
  HighsSimplexInfo& simplex_info = workHMO.simplex_info_;
  if (simplex_info.analyse_iterations)
    analysis->operationRecordBefore(ANALYSIS_OPERATION_TYPE_FTRAN, col_aq,
                                    analysis->col_aq_density);
#endif
  workHMO.factor_.ftran(col_aq, analysis->col_aq_density,
                        analysis->pointer_serial_factor_clocks);
  analysis->simplexTimerStop(FtranClock);
#ifdef HiGHSDEV
  if (simplex_info.analyse_iterations)
    analysis->operationRecordAfter(ANALYSIS_OPERATION_TYPE_FTRAN, col_aq);
#endif

  const double local_col_aq_density = (double)col_aq.count / solver_num_row;
  analysis->updateOperationResultDensity(local_col_aq_density,
                                         analysis->col_aq_density);

  /* Compute the reduced cost for the pivot column and compare it with the kept
   * value */
  double dCompDual = 0.0;
  for (int i = 0; i < col_aq.count; i++) {
    int iRow = col_aq.index[i];
    if (baseValue[iRow] < baseLower[iRow] - dFeasTol) {
      dCompDual -= col_aq.array[iRow] * -1.0;
    } else if (baseValue[iRow] > baseUpper[iRow] + dFeasTol) {
      dCompDual -= col_aq.array[iRow] * +1.0;
    }
  }
  if (fabs(workHMO.simplex_info_.workDual_[columnIn] - dCompDual) >
      (fabs(dCompDual) + 1.0) * 1e-9) {
    printf("==> Phase 1 reduced cost. Updated %g, Computed %g\n",
           workHMO.simplex_info_.workDual_[columnIn], dCompDual);
  }

  analysis->simplexTimerStart(Chuzr1Clock);
  /* Collect phase 1 theta lists */
  int nRow = workHMO.lp_.numRow_;
  const int iMoveIn = workHMO.simplex_basis_.nonbasicMove_[columnIn];
  const double dPivotTol =
      workHMO.simplex_info_.update_count < 10
          ? 1e-9
          : workHMO.simplex_info_.update_count < 20 ? 1e-8 : 1e-7;
  ph1SorterR.clear();
  ph1SorterT.clear();
  for (int i = 0; i < col_aq.count; i++) {
    int iRow = col_aq.index[i];
    double dAlpha = col_aq.array[iRow] * iMoveIn;

    /* When the basic variable x[i] decrease */
    if (dAlpha > +dPivotTol) {
      /* Whether it can become feasible by going below its upper bound */
      if (baseValue[iRow] > baseUpper[iRow] + dFeasTol) {
        double dFeasTheta =
            (baseValue[iRow] - baseUpper[iRow] - dFeasTol) / dAlpha;
        ph1SorterR.push_back(std::make_pair(dFeasTheta, iRow));
        ph1SorterT.push_back(std::make_pair(dFeasTheta, iRow));
      }
      /* Whether it can become infeasible (again) by going below its lower bound
       */
      if (baseValue[iRow] > baseLower[iRow] - dFeasTol &&
          baseLower[iRow] > -HIGHS_CONST_INF) {
        double dRelaxTheta =
            (baseValue[iRow] - baseLower[iRow] + dFeasTol) / dAlpha;
        double dTightTheta = (baseValue[iRow] - baseLower[iRow]) / dAlpha;
        ph1SorterR.push_back(std::make_pair(dRelaxTheta, iRow - nRow));
        ph1SorterT.push_back(std::make_pair(dTightTheta, iRow - nRow));
      }
    }

    /* When the basic variable x[i] increase */
    if (dAlpha < -dPivotTol) {
      /* Whether it can become feasible by going above its lower bound */
      if (baseValue[iRow] < baseLower[iRow] - dFeasTol) {
        double dFeasTheta =
            (baseValue[iRow] - baseLower[iRow] + dFeasTol) / dAlpha;
        ph1SorterR.push_back(std::make_pair(dFeasTheta, iRow - nRow));
        ph1SorterT.push_back(std::make_pair(dFeasTheta, iRow - nRow));
      }

      /* Whether it can become infeasible (again) by going above its upper bound
       */
      if (baseValue[iRow] < baseUpper[iRow] + dFeasTol &&
          baseUpper[iRow] < +HIGHS_CONST_INF) {
        double dRelaxTheta =
            (baseValue[iRow] - baseUpper[iRow] - dFeasTol) / dAlpha;
        double dTightTheta = (baseValue[iRow] - baseUpper[iRow]) / dAlpha;
        ph1SorterR.push_back(std::make_pair(dRelaxTheta, iRow));
        ph1SorterT.push_back(std::make_pair(dTightTheta, iRow));
      }
    }
  }

  analysis->simplexTimerStop(Chuzr1Clock);
  /* When there is no candidates at all, we can leave it here */
  if (ph1SorterR.empty()) {
    rowOut = -1;
    columnOut = -1;
    return;
  }

  /*
   * Now sort the relaxed theta to find the final break point.
   * TODO: Consider partial sort.
   *       Or heapify [O(n)] and then pop k points [kO(log(n))].
   */
  analysis->simplexTimerStart(Chuzr2Clock);
  std::sort(ph1SorterR.begin(), ph1SorterR.end());
  double dMaxTheta = ph1SorterR.at(0).first;
  double dGradient = fabs(workHMO.simplex_info_.workDual_[columnIn]);
  for (unsigned int i = 0; i < ph1SorterR.size(); i++) {
    double dMyTheta = ph1SorterR.at(i).first;
    int index = ph1SorterR.at(i).second;
    int iRow = index >= 0 ? index : index + nRow;
    dGradient -= fabs(col_aq.array[iRow]);
    /* Stop when the gradient start to decrease */
    if (dGradient <= 0) {
      break;
    }
    dMaxTheta = dMyTheta;
  }

  /* Find out the biggest possible alpha for pivot */
  std::sort(ph1SorterT.begin(), ph1SorterT.end());
  double dMaxAlpha = 0.0;
  unsigned int iLast = ph1SorterT.size();
  for (unsigned int i = 0; i < ph1SorterT.size(); i++) {
    double dMyTheta = ph1SorterT.at(i).first;
    int index = ph1SorterT.at(i).second;
    int iRow = index >= 0 ? index : index + nRow;
    double dAbsAlpha = fabs(col_aq.array[iRow]);
    /* Stop when the theta is too large */
    if (dMyTheta > dMaxTheta) {
      iLast = i;
      break;
    }
    /* Update the maximal possible alpha */
    if (dMaxAlpha < dAbsAlpha) {
      dMaxAlpha = dAbsAlpha;
    }
  }

  /* Finally choose a pivot with good enough alpha, backwardly */
  rowOut = -1;
  columnOut = -1;
  phase1OutBnd = 0;
  for (int i = iLast - 1; i >= 0; i--) {
    int index = ph1SorterT.at(i).second;
    int iRow = index >= 0 ? index : index + nRow;
    double dAbsAlpha = fabs(col_aq.array[iRow]);
    if (dAbsAlpha > dMaxAlpha * 0.1) {
      rowOut = iRow;
      phase1OutBnd = index > 0 ? 1 : -1;
      break;
    }
  }
  if (rowOut != -1) {
    columnOut = workHMO.simplex_basis_.basicIndex_[rowOut];
  }
  analysis->simplexTimerStop(Chuzr2Clock);
}

/* Update the primal and dual solutions for the primal phase 1 */
void HQPrimal::phase1Update() {
  /* Alias to bounds arrays */
  const double* workLower = &workHMO.simplex_info_.workLower_[0];
  const double* workUpper = &workHMO.simplex_info_.workUpper_[0];
  const double* baseLower = &workHMO.simplex_info_.baseLower_[0];
  const double* baseUpper = &workHMO.simplex_info_.baseUpper_[0];
  double* workValue = &workHMO.simplex_info_.workValue_[0];
  double* baseValue = &workHMO.simplex_info_.baseValue_[0];
  const int iMoveIn = workHMO.simplex_basis_.nonbasicMove_[columnIn];
  const double dFeasTol =
      workHMO.scaled_solution_params_.primal_feasibility_tolerance;

  /* Compute the primal theta and see if we should have do bound flip instead */
  alpha = col_aq.array[rowOut];
  thetaPrimal = 0.0;
  if (phase1OutBnd == 1) {
    thetaPrimal = (baseValue[rowOut] - baseUpper[rowOut]) / alpha;
  } else {
    thetaPrimal = (baseValue[rowOut] - baseLower[rowOut]) / alpha;
  }

  double lowerIn = workLower[columnIn];
  double upperIn = workUpper[columnIn];
  double valueIn = workValue[columnIn] + thetaPrimal;
  int ifFlip = 0;
  if (iMoveIn == +1 && valueIn > upperIn + dFeasTol) {
    workValue[columnIn] = upperIn;
    thetaPrimal = upperIn - lowerIn;
    ifFlip = 1;
    workHMO.simplex_basis_.nonbasicMove_[columnIn] = -1;
  }
  if (iMoveIn == -1 && valueIn < lowerIn - dFeasTol) {
    workValue[columnIn] = lowerIn;
    thetaPrimal = lowerIn - upperIn;
    ifFlip = 1;
    workHMO.simplex_basis_.nonbasicMove_[columnIn] = +1;
  }

  /* Update for the flip case */
  if (ifFlip) {
    /* Recompute things on flip */
    if (invertHint == 0) {
      analysis->simplexTimerStart(ComputePrimalClock);
      computePrimal(workHMO);
      analysis->simplexTimerStop(ComputePrimalClock);
      computeSimplexPrimalInfeasible(workHMO);
      // Assumes that only simplex_info_.*_primal_infeasibilities is
      // needed - probably only num_primal_infeasibilities
      //
      //      copySimplexPrimalInfeasible(workHMO);
      if (workHMO.simplex_info_.num_primal_infeasibilities > 0) {
        isPrimalPhase1 = 1;
        analysis->simplexTimerStart(ComputeDualClock);
        phase1ComputeDual();
        analysis->simplexTimerStop(ComputeDualClock);
      } else {
        invertHint = INVERT_HINT_UPDATE_LIMIT_REACHED;
      }
    }
    return;
  }

  /* Compute BTran for update LU */
  analysis->simplexTimerStart(BtranClock);
  row_ep.clear();
  row_ep.count = 1;
  row_ep.index[0] = rowOut;
  row_ep.array[rowOut] = 1;
  row_ep.packFlag = true;
#ifdef HiGHSDEV
  HighsSimplexInfo& simplex_info = workHMO.simplex_info_;
  if (simplex_info.analyse_iterations)
    analysis->operationRecordBefore(ANALYSIS_OPERATION_TYPE_BTRAN_EP, row_ep,
                                    analysis->row_ep_density);
#endif
  workHMO.factor_.btran(row_ep, analysis->row_ep_density,
                        analysis->pointer_serial_factor_clocks);
#ifdef HiGHSDEV
  if (simplex_info.analyse_iterations)
    analysis->operationRecordAfter(ANALYSIS_OPERATION_TYPE_BTRAN_EP, row_ep);
#endif
  analysis->simplexTimerStop(BtranClock);

  const double local_row_ep_density = (double)row_ep.count / solver_num_row;
  analysis->updateOperationResultDensity(local_row_ep_density,
                                         analysis->row_ep_density);

  /* Compute the whole pivot row for updating the devex weight */
#ifdef HiGHSDEV
  if (simplex_info.analyse_iterations) {
    analysis->operationRecordBefore(ANALYSIS_OPERATION_TYPE_PRICE_AP, row_ep,
                                    analysis->row_ap_density);
    analysis->num_row_price++;
  }
#endif
  analysis->simplexTimerStart(PriceClock);
  row_ap.clear();
  workHMO.matrix_.priceByRowSparseResult(row_ap, row_ep);
  analysis->simplexTimerStop(PriceClock);
#ifdef HiGHSDEV
  if (simplex_info.analyse_iterations)
    analysis->operationRecordAfter(ANALYSIS_OPERATION_TYPE_PRICE_AP, row_ep);
#endif

  /* Update the devex weight */
  devexUpdate();

  /* Update other things */
  update_pivots(workHMO, columnIn, rowOut, phase1OutBnd);
  update_factor(workHMO, &col_aq, &row_ep, &rowOut, &invertHint);
  update_matrix(workHMO, columnIn, columnOut);
  if (workHMO.simplex_info_.update_count >=
      workHMO.simplex_info_.update_limit) {
    invertHint = INVERT_HINT_UPDATE_LIMIT_REACHED;
  }

  /* Recompute dual and primal */
  if (invertHint == 0) {
    analysis->simplexTimerStart(ComputePrimalClock);
    computePrimal(workHMO);
    analysis->simplexTimerStop(ComputePrimalClock);
    computeSimplexPrimalInfeasible(workHMO);
    // Assumes that only simplex_info_.*_primal_infeasibilities is
    // needed - probably only num_primal_infeasibilities
    //
    //      copySimplexPrimalInfeasible(workHMO);
    if (workHMO.simplex_info_.num_primal_infeasibilities > 0) {
      isPrimalPhase1 = 1;
      analysis->simplexTimerStart(ComputeDualClock);
      phase1ComputeDual();
      analysis->simplexTimerStop(ComputeDualClock);
    } else {
      invertHint = INVERT_HINT_UPDATE_LIMIT_REACHED;
    }
  }

  /* Reset the devex framework when necessary */
  if (num_bad_devex_weight > 3) {
    devexReset();
  }

  // Move this to Simplex class once it's created
  // simplex_method.record_pivots(columnIn, columnOut, alpha);
  workHMO.iteration_counts_.simplex++;
}

/* Reset the devex weight */
void HQPrimal::devexReset() {
  const int nSeq = workHMO.lp_.numCol_ + workHMO.lp_.numRow_;
  devex_weight.assign(nSeq, 1.0);
  devex_index.assign(nSeq, 0);
  for (int iSeq = 0; iSeq < nSeq; iSeq++) {
    const int nonbasicFlag = workHMO.simplex_basis_.nonbasicFlag_[iSeq];
    devex_index[iSeq] = nonbasicFlag * nonbasicFlag;
  }
  num_devex_iterations = 0;
  num_bad_devex_weight = 0;
}

void HQPrimal::devexUpdate() {
  /* Compute the pivot weight from the reference set */
  analysis->simplexTimerStart(DevexUpdateWeightClock);
  double dPivotWeight = 0.0;
  for (int i = 0; i < col_aq.count; i++) {
    int iRow = col_aq.index[i];
    int iSeq = workHMO.simplex_basis_.basicIndex_[iRow];
    double dAlpha = devex_index[iSeq] * col_aq.array[iRow];
    dPivotWeight += dAlpha * dAlpha;
  }
  dPivotWeight += devex_index[columnIn] * 1.0;
  dPivotWeight = sqrt(dPivotWeight);

  /* Check if the saved weight is too large */
  if (devex_weight[columnIn] > 3.0 * dPivotWeight) {
    num_bad_devex_weight++;
  }

  /* Update the devex weight for all */
  double dPivot = col_aq.array[rowOut];
  dPivotWeight /= fabs(dPivot);

  for (int i = 0; i < row_ap.count; i++) {
    int iSeq = row_ap.index[i];
    double alpha = row_ap.array[iSeq];
    double devex = dPivotWeight * fabs(alpha);
    devex += devex_index[iSeq] * 1.0;
    if (devex_weight[iSeq] < devex) {
      devex_weight[iSeq] = devex;
    }
  }
  for (int i = 0; i < row_ep.count; i++) {
    int iPtr = row_ep.index[i];
    int iSeq = row_ep.index[i] + solver_num_col;
    double alpha = row_ep.array[iPtr];
    double devex = dPivotWeight * fabs(alpha);
    devex += devex_index[iSeq] * 1.0;
    if (devex_weight[iSeq] < devex) {
      devex_weight[iSeq] = devex;
    }
  }

  /* Update devex weight for the pivots */
  devex_weight[columnOut] = max(1.0, dPivotWeight);
  devex_weight[columnIn] = 1.0;
  num_devex_iterations++;
  analysis->simplexTimerStop(DevexUpdateWeightClock);
}

void HQPrimal::iterationAnalysisData() {
  //  HighsSolutionParams& scaled_solution_params =
  //  workHMO.scaled_solution_params_;
  HighsSimplexInfo& simplex_info = workHMO.simplex_info_;
  analysis->simplex_strategy = SIMPLEX_STRATEGY_PRIMAL;
  analysis->edge_weight_mode = DualEdgeWeightMode::DEVEX;
  analysis->solve_phase = solvePhase;
  analysis->simplex_iteration_count = workHMO.iteration_counts_.simplex;
  analysis->devex_iteration_count = num_devex_iterations;
  analysis->pivotal_row_index = rowOut;
  analysis->leaving_variable = columnOut;
  analysis->entering_variable = columnIn;
  analysis->invert_hint = invertHint;
  analysis->reduced_rhs_value = 0;
  analysis->reduced_cost_value = 0;
  analysis->edge_weight = 0;
  analysis->primal_delta = 0;
  analysis->primal_step = thetaPrimal;
  analysis->dual_step = thetaDual;
  analysis->pivot_value_from_column = alpha;
  analysis->pivot_value_from_row = alpha;  // Row;
  analysis->numerical_trouble = numericalTrouble;
  analysis->objective_value = simplex_info.updated_primal_objective_value;
  analysis->num_primal_infeasibilities =
      simplex_info.num_primal_infeasibilities;
  analysis->num_dual_infeasibilities = simplex_info.num_dual_infeasibilities;
  analysis->sum_primal_infeasibilities =
      simplex_info.sum_primal_infeasibilities;
  analysis->sum_dual_infeasibilities = simplex_info.sum_dual_infeasibilities;
#ifdef HiGHSDEV
  analysis->basis_condition = simplex_info.invert_condition;
#endif
  if ((analysis->edge_weight_mode == DualEdgeWeightMode::DEVEX) &&
      (num_devex_iterations == 0))
    analysis->num_devex_framework++;
}

void HQPrimal::iterationAnalysis() {
  // Possibly report on the iteration
  iterationAnalysisData();
  analysis->iterationReport();

#ifdef HiGHSDEV
  analysis->iterationRecord();
#endif
}

void HQPrimal::reportRebuild(const int rebuild_invert_hint) {
  analysis->simplexTimerStart(ReportRebuildClock);
  iterationAnalysisData();
  analysis->invert_hint = rebuild_invert_hint;
  analysis->invertReport();
  analysis->simplexTimerStop(ReportRebuildClock);
}

bool HQPrimal::bailout() {
  if (solve_bailout) {
    // Bailout has already been decided: check that it's for one of these
    // reasons
    assert(workHMO.scaled_model_status_ ==
               HighsModelStatus::REACHED_TIME_LIMIT ||
           workHMO.scaled_model_status_ ==
               HighsModelStatus::REACHED_ITERATION_LIMIT);
  } else if (workHMO.timer_.readRunHighsClock() > workHMO.options_.time_limit) {
    solve_bailout = true;
    workHMO.scaled_model_status_ = HighsModelStatus::REACHED_TIME_LIMIT;
  } else if (workHMO.iteration_counts_.simplex >=
             workHMO.options_.simplex_iteration_limit) {
    solve_bailout = true;
    workHMO.scaled_model_status_ = HighsModelStatus::REACHED_ITERATION_LIMIT;
  }
  return solve_bailout;
}
