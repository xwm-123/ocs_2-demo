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

#include <gtest/gtest.h>
#include <cstdlib>
#include <ctime>
#include <iostream>

#include <ocs2_core/initialization/DefaultInitializer.h>
#include <ocs2_core/control/FeedforwardController.h>
#include <ocs2_oc/rollout/TimeTriggeredRollout.h>
#include <ocs2_oc/test/EXP0.h>

#include <ocs2_ddp/ILQR.h>
#include <ocs2_ddp/SLQ.h>

class Exp0 : public testing::Test {
 protected:
  static constexpr size_t STATE_DIM = 2;
  static constexpr size_t INPUT_DIM = 1;
  static constexpr ocs2::scalar_t expectedCost = 9.766;
  static constexpr ocs2::scalar_t expectedStateInputEqConstraintISE = 0.0;
  static constexpr ocs2::scalar_t expectedStateEqConstraintISE = 0.0;

  Exp0() {
    // event times
    const ocs2::scalar_array_t eventTimes{0.1897};
    const std::vector<size_t> modeSequence{0, 1};
    referenceManagerPtr = ocs2::getExp0ReferenceManager(eventTimes, modeSequence);

    // partitioning times
    partitioningTimes = ocs2::scalar_array_t{startTime, eventTimes[0], finalTime};

    // rollout settings
    const auto rolloutSettings = []() {
      ocs2::rollout::Settings rolloutSettings;
      rolloutSettings.absTolODE = 1e-10;
      rolloutSettings.relTolODE = 1e-7;
      rolloutSettings.maxNumStepsPerSecond = 10000;
      return rolloutSettings;
    }();

    // dynamics and rollout
    ocs2::EXP0_System system(referenceManagerPtr);
    rolloutPtr.reset(new ocs2::TimeTriggeredRollout(system, rolloutSettings));

    // optimal control problem
    problemPtr.reset(new ocs2::OptimalControlProblem);
    problemPtr->dynamicsPtr.reset(system.clone());

    // cost function
    problemPtr->costPtr->add("cost", std::unique_ptr<ocs2::StateInputCost>(new ocs2::EXP0_Cost()));
    problemPtr->finalCostPtr->add("finalCost", std::unique_ptr<ocs2::StateCost>(new ocs2::EXP0_FinalCost()));

    // operatingTrajectories
    initializerPtr.reset(new ocs2::DefaultInitializer(INPUT_DIM));
  }

  ocs2::ddp::Settings getSettings(ocs2::ddp::Algorithm algorithmType, size_t numThreads, ocs2::search_strategy::Type strategy,
                                  bool display = false) const {
    ocs2::ddp::Settings ddpSettings;
    ddpSettings.algorithm_ = algorithmType;
    ddpSettings.nThreads_ = numThreads;
    ddpSettings.preComputeRiccatiTerms_ = true;
    ddpSettings.displayInfo_ = false;
    ddpSettings.displayShortSummary_ = display;
    ddpSettings.absTolODE_ = 1e-10;
    ddpSettings.relTolODE_ = 1e-7;
    ddpSettings.maxNumStepsPerSecond_ = 10000;
    ddpSettings.maxNumIterations_ = 30;
    ddpSettings.minRelCost_ = 1e-3;
    ddpSettings.checkNumericalStability_ = true;
    ddpSettings.useNominalTimeForBackwardPass_ = false;
    ddpSettings.useFeedbackPolicy_ = true;
    ddpSettings.debugPrintRollout_ = false;
    ddpSettings.strategy_ = strategy;
    ddpSettings.lineSearch_.minStepLength_ = 0.0001;
    return ddpSettings;
  }

  std::string getTestName(const ocs2::ddp::Settings& ddpSettings) const {
    std::string testName;
    testName += "EXP0 Test { ";
    testName += "Algorithm: " + ocs2::ddp::toAlgorithmName(ddpSettings.algorithm_) + ",  ";
    testName += "Strategy: " + ocs2::search_strategy::toString(ddpSettings.strategy_) + ",  ";
    testName += "#threads: " + std::to_string(ddpSettings.nThreads_) + " }";
    return testName;
  }

  void performanceIndexTest(const ocs2::ddp::Settings& ddpSettings, const ocs2::PerformanceIndex& performanceIndex) const {
    const auto testName = getTestName(ddpSettings);
    EXPECT_LT(fabs(performanceIndex.totalCost - expectedCost), 10 * ddpSettings.minRelCost_)
        << "MESSAGE: " << testName << ": failed in the total cost test!";
    EXPECT_LT(fabs(performanceIndex.stateInputEqConstraintISE - expectedStateInputEqConstraintISE), 10 * ddpSettings.constraintTolerance_)
        << "MESSAGE: " << testName << ": failed in state-input equality constraint ISE test!";
    EXPECT_LT(fabs(performanceIndex.stateEqConstraintISE - expectedStateEqConstraintISE), 10 * ddpSettings.constraintTolerance_)
        << "MESSAGE: " << testName << ": failed in state-only equality constraint ISE test!";
  }

  const ocs2::scalar_t startTime = 0.0;
  const ocs2::scalar_t finalTime = 2.0;
  const ocs2::vector_t initState = (ocs2::vector_t(STATE_DIM) << 0.0, 2.0).finished();
  ocs2::scalar_array_t partitioningTimes;
  std::shared_ptr<ocs2::ReferenceManager> referenceManagerPtr;

  std::unique_ptr<ocs2::TimeTriggeredRollout> rolloutPtr;
  std::unique_ptr<ocs2::OptimalControlProblem> problemPtr;
  std::unique_ptr<ocs2::Initializer> initializerPtr;
};

constexpr size_t Exp0::STATE_DIM;
constexpr size_t Exp0::INPUT_DIM;
constexpr ocs2::scalar_t Exp0::expectedCost;
constexpr ocs2::scalar_t Exp0::expectedStateInputEqConstraintISE;
constexpr ocs2::scalar_t Exp0::expectedStateEqConstraintISE;

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
TEST_F(Exp0, ddp_feedback_policy) {
  // ddp settings
  auto ddpSettings = getSettings(ocs2::ddp::Algorithm::SLQ, 2, ocs2::search_strategy::Type::LINE_SEARCH);
  ddpSettings.useFeedbackPolicy_ = true;

  // instantiate
  ocs2::SLQ ddp(ddpSettings, *rolloutPtr, *problemPtr, *initializerPtr);
  ddp.setReferenceManager(referenceManagerPtr);

  // run ddp
  ddp.run(startTime, initState, finalTime, partitioningTimes);
  // get solution
  const auto solution = ddp.primalSolution(finalTime);
  const auto* ctrlPtr = dynamic_cast<ocs2::LinearController*>(solution.controllerPtr_.get());

  EXPECT_TRUE(ctrlPtr != nullptr) << "MESSAGE: SLQ solution does not contain a linear feedback policy!";
  EXPECT_DOUBLE_EQ(ctrlPtr->timeStamp_.back(), finalTime) << "MESSAGE: SLQ failed in policy final time of controller!";
  EXPECT_DOUBLE_EQ(solution.timeTrajectory_.back(), finalTime) << "MESSAGE: SLQ failed in policy final time of trajectory!";
}

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
TEST_F(Exp0, ddp_feedforward_policy) {
  // ddp settings
  auto ddpSettings = getSettings(ocs2::ddp::Algorithm::SLQ, 2, ocs2::search_strategy::Type::LINE_SEARCH);
  ddpSettings.useFeedbackPolicy_ = false;

  // instantiate
  ocs2::SLQ ddp(ddpSettings, *rolloutPtr, *problemPtr, *initializerPtr);
  ddp.setReferenceManager(referenceManagerPtr);

  // run ddp
  ddp.run(startTime, initState, finalTime, partitioningTimes);
  // get solution
  const auto solution = ddp.primalSolution(finalTime);
  const auto* ctrlPtr = dynamic_cast<ocs2::FeedforwardController*>(solution.controllerPtr_.get());

  EXPECT_TRUE(ctrlPtr != nullptr) << "MESSAGE: SLQ solution does not contain a feedforward policy!";
  EXPECT_DOUBLE_EQ(ctrlPtr->timeStamp_.back(), finalTime) << "MESSAGE: SLQ failed in policy final time of controller!";
  EXPECT_DOUBLE_EQ(solution.timeTrajectory_.back(), finalTime) << "MESSAGE: SLQ failed in policy final time of trajectory!";
}

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
TEST_F(Exp0, ddp_caching) {
  // ddp settings
  auto ddpSettings = getSettings(ocs2::ddp::Algorithm::SLQ, 2, ocs2::search_strategy::Type::LINE_SEARCH);
  ddpSettings.displayInfo_ = false;

  // event times
  const ocs2::scalar_array_t eventTimes{1.0};
  const std::vector<size_t> modeSequence{0, 1};
  referenceManagerPtr = ocs2::getExp0ReferenceManager(eventTimes, modeSequence);

  // instantiate
  ocs2::SLQ ddp(ddpSettings, *rolloutPtr, *problemPtr, *initializerPtr);
  ddp.setReferenceManager(referenceManagerPtr);

  // run single core SLQ (no active event)
  ocs2::scalar_t startTime = 0.2;
  ocs2::scalar_t finalTime = 0.7;
  EXPECT_NO_THROW(ddp.run(startTime, initState, finalTime, partitioningTimes));

  // run similar to the MPC setup (a new partition)
  startTime = 0.4;
  finalTime = 0.9;
  EXPECT_NO_THROW(ddp.run(startTime, initState, finalTime, partitioningTimes, std::vector<ocs2::ControllerBase*>()));

  // run similar to the MPC setup (one active event)
  startTime = 0.6;
  finalTime = 1.2;
  EXPECT_NO_THROW(ddp.run(startTime, initState, finalTime, partitioningTimes, std::vector<ocs2::ControllerBase*>()));

  // run similar to the MPC setup (no active event + a new partition)
  startTime = 1.1;
  finalTime = 1.5;
  EXPECT_NO_THROW(ddp.run(startTime, initState, finalTime, partitioningTimes, std::vector<ocs2::ControllerBase*>()));

  // run similar to the MPC setup (no overlap)
  startTime = 1.6;
  finalTime = 2.0;
  EXPECT_NO_THROW(ddp.run(startTime, initState, finalTime, partitioningTimes, std::vector<ocs2::ControllerBase*>()));
}

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
/* Add parameterized test suite */
class Exp0Param : public Exp0, public testing::WithParamInterface<std::tuple<ocs2::search_strategy::Type, size_t>> {
 protected:
  ocs2::search_strategy::Type getSearchStrategy() { return std::get<0>(GetParam()); }

  size_t getNumThreads() { return std::get<1>(GetParam()); }
};

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
TEST_P(Exp0Param, SLQ) {
  // ddp settings
  const auto ddpSettings = getSettings(ocs2::ddp::Algorithm::SLQ, getNumThreads(), getSearchStrategy());

  // instantiate
  ocs2::SLQ ddp(ddpSettings, *rolloutPtr, *problemPtr, *initializerPtr);
  ddp.setReferenceManager(referenceManagerPtr);

  if (ddpSettings.displayInfo_ || ddpSettings.displayShortSummary_) {
    std::cerr << "\n" << getTestName(ddpSettings) << "\n";
  }

  // run ddp
  ddp.run(startTime, initState, finalTime, partitioningTimes);
  // get performance index
  const auto performanceIndex = ddp.getPerformanceIndeces();

  // performanceIndeces test
  performanceIndexTest(ddpSettings, performanceIndex);
}

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
TEST_P(Exp0Param, ILQR) {
  // ddp settings
  const auto ddpSettings = getSettings(ocs2::ddp::Algorithm::ILQR, getNumThreads(), getSearchStrategy());

  // instantiate
  ocs2::ILQR ddp(ddpSettings, *rolloutPtr, *problemPtr, *initializerPtr);
  ddp.setReferenceManager(referenceManagerPtr);

  if (ddpSettings.displayInfo_ || ddpSettings.displayShortSummary_) {
    std::cerr << "\n" << getTestName(ddpSettings) << "\n";
  }

  // run ddp
  ddp.run(startTime, initState, finalTime, partitioningTimes);
  // get performance index
  const auto performanceIndex = ddp.getPerformanceIndeces();

  // performanceIndeces test
  performanceIndexTest(ddpSettings, performanceIndex);
}

INSTANTIATE_TEST_CASE_P(Exp0ParamCase, Exp0Param,
                        testing::Combine(testing::ValuesIn({ocs2::search_strategy::Type::LINE_SEARCH,
                                                            ocs2::search_strategy::Type::LEVENBERG_MARQUARDT}),
                                         testing::ValuesIn({size_t(1), size_t(3)})), /* num threads */
                        [](const testing::TestParamInfo<Exp0Param::ParamType>& info) {
                          /* returns test name for gtest summary */
                          std::string name;
                          name += ocs2::search_strategy::toString(std::get<0>(info.param)) + "__";
                          name += std::get<1>(info.param) == 1 ? "SINGLE_THREAD" : "MULTI_THREAD";
                          return name;
                        });
