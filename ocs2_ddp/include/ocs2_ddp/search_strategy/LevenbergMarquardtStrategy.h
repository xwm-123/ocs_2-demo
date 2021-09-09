/******************************************************************************
Copyright (c) 2017, Farbod Farshidian. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of the copyright holder nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
******************************************************************************/

#pragma once

#include <functional>
#include <utility>
#include <vector>

#include <ocs2_core/Types.h>
#include <ocs2_core/control/LinearController.h>
#include <ocs2_core/dynamics/SystemDynamicsBase.h>
#include <ocs2_core/model_data/ModelData.h>
#include <ocs2_core/soft_constraint/SoftConstraintPenalty.h>
#include <ocs2_core/thread_support/ThreadPool.h>
#include <ocs2_oc/oc_problem/OptimalControlProblem.h>
#include <ocs2_oc/oc_solver/PerformanceIndex.h>
#include <ocs2_oc/rollout/RolloutBase.h>

#include "SearchStrategyBase.h"
#include "StrategySettings.h"

namespace ocs2 {

/**
 * Levenberg Marquardt strategy: The class computes the nominal controller and the nominal trajectories
 * as well the corresponding performance indices.
 * reference: Tassa et al., Synthesis and stabilization of complex behaviors through online trajectory optimization.
 */
class LevenbergMarquardtStrategy final : public SearchStrategyBase {
 public:
  /**
   * constructor.
   *
   * @param [in] baseSettings: The basic settings for the search strategy algorithms.
   * @param [in] settings: The Levenberg Marquardt settings.
   * @param [in] rolloutRef: A reference to the rollout.
   * @param [in] optimalControlProblemRef: A reference to the optimal control problem.
   * @param [in] ineqConstrPenaltyRef: A reference to the inequality constraints penalty.
   * @param [in] meritFunc: the merit function which gets the PerformanceIndex and returns the merit function value.
   */
  LevenbergMarquardtStrategy(search_strategy::Settings baseSettings, levenberg_marquardt::Settings settings, RolloutBase& rolloutRefStock,
                             OptimalControlProblem& optimalControlProblemRef, SoftConstraintPenalty& ineqConstrPenalty,
                             std::function<scalar_t(const PerformanceIndex&)> meritFunc);

  /**
   * Default destructor.
   */
  ~LevenbergMarquardtStrategy() override = default;

  LevenbergMarquardtStrategy(const LevenbergMarquardtStrategy&) = delete;
  LevenbergMarquardtStrategy& operator=(const LevenbergMarquardtStrategy&) = delete;

  void reset() override;

  bool run(scalar_t expectedCost, const ModeSchedule& modeSchedule, std::vector<LinearController>& controllersStock,
           PerformanceIndex& performanceIndex, scalar_array2_t& timeTrajectoriesStock, size_array2_t& postEventIndicesStock,
           vector_array2_t& stateTrajectoriesStock, vector_array2_t& inputTrajectoriesStock,
           std::vector<std::vector<ModelData>>& modelDataTrajectoriesStock, std::vector<std::vector<ModelData>>& modelDataEventTimesStock,
           scalar_t& avgTimeStepFP) override;

  std::pair<bool, std::string> checkConvergence(bool unreliableControllerIncrement, const PerformanceIndex& previousPerformanceIndex,
                                                const PerformanceIndex& currentPerformanceIndex) const override;

  void computeRiccatiModification(const ModelData& projectedModelData, matrix_t& deltaQm, vector_t& deltaGv,
                                  matrix_t& deltaGm) const override;

  matrix_t augmentHamiltonianHessian(const ModelData& modelData, const matrix_t& Hm) const override;

 private:
  // Levenberg-Marquardt
  struct LevenbergMarquardtModule {
    scalar_t pho = 1.0;                           // the ratio between actual reduction and predicted reduction
    scalar_t riccatiMultiple = 0.0;               // the Riccati multiple for Tikhonov regularization.
    scalar_t riccatiMultipleAdaptiveRatio = 1.0;  // the adaptive ratio of geometric progression for Riccati multiple.
    size_t numSuccessiveRejections = 0;           // the number of successive rejections of solution.
  };

  levenberg_marquardt::Settings settings_;
  LevenbergMarquardtModule levenbergMarquardtModule_;

  RolloutBase& rolloutRef_;
  OptimalControlProblem& optimalControlProblemRef_;
  SoftConstraintPenalty& ineqConstrPenaltyRef_;
  std::function<scalar_t(PerformanceIndex)> meritFunc_;

  scalar_t avgTimeStepFP_ = 0.0;
};

}  // namespace ocs2
