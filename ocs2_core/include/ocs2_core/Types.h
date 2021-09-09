/******************************************************************************
Copyright (c) 2020, Farbod Farshidian. All rights reserved.

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

#include <Eigen/Dense>
#include <Eigen/StdVector>
#include <ostream>
#include <vector>

namespace ocs2 {

/** size_t trajectory type. */
using size_array_t = std::vector<size_t>;
/** Array of size_t trajectory type. */
using size_array2_t = std::vector<size_array_t>;

/** Scalar type. */
using scalar_t = double;
/** Scalar trajectory type. */
using scalar_array_t = std::vector<scalar_t>;
/** Array of scalar trajectory type. */
using scalar_array2_t = std::vector<scalar_array_t>;
/** Array of arrays of scalar trajectory type. */
using scalar_array3_t = std::vector<scalar_array2_t>;

/** Dynamic-size vector type. */
using vector_t = Eigen::Matrix<scalar_t, Eigen::Dynamic, 1>;
/** Dynamic vector's trajectory type. */
using vector_array_t = std::vector<vector_t>;
/** Array of dynamic vector's trajectory type. */
using vector_array2_t = std::vector<vector_array_t>;
/** Array of arrays of dynamic vector trajectory type. */
using vector_array3_t = std::vector<vector_array2_t>;

/** Dynamic-size row vector type. */
using row_vector_t = Eigen::Matrix<scalar_t, 1, Eigen::Dynamic>;

/** Dynamic-size matrix type. */
using matrix_t = Eigen::Matrix<scalar_t, Eigen::Dynamic, Eigen::Dynamic>;
/** Dynamic matrix's trajectory type. */
using matrix_array_t = std::vector<matrix_t>;
/** Array of dynamic matrix's trajectory type. */
using matrix_array2_t = std::vector<matrix_array_t>;
/** Array of arrays of dynamic matrix trajectory type. */
using matrix_array3_t = std::vector<matrix_array2_t>;

/** Eigen scalar type. */
using eigen_scalar_t = Eigen::Matrix<scalar_t, 1, 1>;
/** Eigen scalar trajectory type. */
using eigen_scalar_array_t = std::vector<eigen_scalar_t>;
/** Array of eigen scalar trajectory type. */
using eigen_scalar_array2_t = std::vector<eigen_scalar_array_t>;
/** Array of arrays of eigen scalar trajectory type. */
using eigen_scalar_array3_t = std::vector<eigen_scalar_array2_t>;

/**
 * Defines the quadratic approximation
 * f(x,u) = 1/2 dx' dfdxx dx + du' dfdux dx + 1/2 du' dfduu du + dfdx' dx + dfdu' du + f
 */
struct ScalarFunctionQuadraticApproximation {
  /** Second derivative w.r.t state */
  matrix_t dfdxx;
  /** Second derivative w.r.t input (lhs) and state (rhs) */
  matrix_t dfdux;
  /** Second derivative w.r.t input */
  matrix_t dfduu;
  /** First derivative w.r.t state */
  vector_t dfdx;
  /** First derivative w.r.t input */
  vector_t dfdu;
  /** Constant term */
  scalar_t f = 0.;

  /** Default constructor */
  ScalarFunctionQuadraticApproximation() = default;

  /** Construct and resize the members to given size. */
  ScalarFunctionQuadraticApproximation(size_t nx, size_t nu);

  /** Compound additinon assignment opeartor */
  ScalarFunctionQuadraticApproximation& operator+=(const ScalarFunctionQuadraticApproximation& rhs);

  /**
   * Resize the members to the given size
   * @param[in] nx State dimension
   * @param[in] nu Input dimension
   */
  ScalarFunctionQuadraticApproximation& resize(size_t nx, size_t nu);

  /**
   * Resizes the members to the given size, and sets all coefficients to zero.
   * @param[in] nx State dimension
   * @param[in] nu Input dimension
   */
  ScalarFunctionQuadraticApproximation& setZero(size_t nx, size_t nu);

  /**
   * Factory function with zero initialization
   * @param[in] nx State dimension
   * @param[in] nu Input dimension
   * @return Zero initialized object of given size.
   */
  static ScalarFunctionQuadraticApproximation Zero(size_t nx, size_t nu);
};

std::ostream& operator<<(std::ostream& out, const ScalarFunctionQuadraticApproximation& f);

/**
 * Defines the linear model of a vector-valued function
 * f(x,u) = dfdx dx + dfdu du + f
 */
struct VectorFunctionLinearApproximation {
  /** Derivative w.r.t state */
  matrix_t dfdx;
  /** Derivative w.r.t input */
  matrix_t dfdu;
  /** Constant term */
  vector_t f;

  /** Default constructor */
  VectorFunctionLinearApproximation() = default;

  /** Construct and resize the members to given size. */
  VectorFunctionLinearApproximation(size_t nv, size_t nx, size_t nu);

  /**
   * Resize the members to the given size
   * @param[in] nv Vector dimension
   * @param[in] nx State dimension
   * @param[in] nu Input dimension
   */
  VectorFunctionLinearApproximation& resize(size_t nv, size_t nx, size_t nu);

  /**
   * Resizes the members to the given size, and sets all coefficients to zero.
   * @param[in] nv Vector dimension
   * @param[in] nx State dimension
   * @param[in] nu Input dimension
   */
  VectorFunctionLinearApproximation& setZero(size_t nv, size_t nx, size_t nu);

  /**
   * Factory function with zero initialization
   * @param[in] nv Vector dimension
   * @param[in] nx State dimension
   * @param[in] nu Input dimension
   * @return Zero initialized object of given size.
   */
  static VectorFunctionLinearApproximation Zero(size_t nv, size_t nx, size_t nu);
};

std::ostream& operator<<(std::ostream& out, const VectorFunctionLinearApproximation& f);

/**
 * Defines quadratic approximation of a vector-valued function
 * f[i](x,u) = 1/2 dx' dfdxx[i] dx + du' dfdux[i] dx + 1/2 du' dfduu[i] du + dfdx[i,:] dx + dfdu[i,:] du + f[i]
 */
struct VectorFunctionQuadraticApproximation {
  /** Second derivative w.r.t state */
  matrix_array_t dfdxx;
  /** Second derivative w.r.t input (lhs) and state (rhs) */
  matrix_array_t dfdux;
  /** Second derivative w.r.t input */
  matrix_array_t dfduu;
  /** First derivative w.r.t state */
  matrix_t dfdx;
  /** First derivative w.r.t input */
  matrix_t dfdu;
  /** Constant term */
  vector_t f;

  /** Default constructor */
  VectorFunctionQuadraticApproximation() = default;

  /** Construct and resize the members to given size. */
  VectorFunctionQuadraticApproximation(size_t nv, size_t nx, size_t nu);

  /**
   * Resize the members to the given size
   * @param[in] nv Vector dimension
   * @param[in] nx State dimension
   * @param[in] nu Input dimension
   */
  VectorFunctionQuadraticApproximation& resize(size_t nv, size_t nx, size_t nu);

  /**
   * Resizes the members to the given size, and sets all coefficients to zero.
   * @param[in] nv Vector dimension
   * @param[in] nx State dimension
   * @param[in] nu Input dimension
   */
  VectorFunctionQuadraticApproximation& setZero(size_t nv, size_t nx, size_t nu);

  /**
   * Factory function with zero initialization
   * @param[in] nv Vector dimension
   * @param[in] nx State dimension
   * @param[in] nu Input dimension
   * @return Zero initialized object of given size.
   */
  static VectorFunctionQuadraticApproximation Zero(size_t nv, size_t nx, size_t nu);
};

std::ostream& operator<<(std::ostream& out, const VectorFunctionQuadraticApproximation& f);

}  // namespace ocs2
