/*    Copyright (c) 2010-2012, Delft University of Technology
 *    All rights reserved.
 *
 *    Redistribution and use in source and binary forms, with or without modification, are
 *    permitted provided that the following conditions are met:
 *      - Redistributions of source code must retain the above copyright notice, this list of
 *        conditions and the following disclaimer.
 *      - Redistributions in binary form must reproduce the above copyright notice, this list of
 *        conditions and the following disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *      - Neither the name of the Delft University of Technology nor the names of its contributors
 *        may be used to endorse or promote products derived from this software without specific
 *        prior written permission.
 *
 *    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS
 *    OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 *    MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *    COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *    EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 *    GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 *    AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *    NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 *    OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *    Changelog
 *      YYMMDD    Author            Comment
 *      110617    D. Dirkx          Creation of code.
 *      120324    K. Kumar          Minor Doxygen comment corrections, added astrodynamics
 *                                  namespace layer; added missing Eigen include-statement.
 *
 *    References
 *
 */

#ifndef TUDAT_AERODYNAMIC_ACCELERATION_H
#define TUDAT_AERODYNAMIC_ACCELERATION_H

#include <Eigen/Core>

#include "Tudat/Astrodynamics/Aerodynamics/aerodynamicCoefficientInterface.h"
#include "Tudat/Astrodynamics/Aerodynamics/aerodynamicForce.h"

namespace tudat
{
namespace astrodynamics
{
namespace acceleration_models
{

//! Compute the aerodynamic acceleration in same reference frame as input coefficients.
/*!
 * This function computes the aerodynamic acceleration. It takes primitive types as arguments to
 * perform the calculations. Therefore, these quantities (dynamicPressure, reference area and
 * aerodynamic coefficients) have to computed before passing them to this function.
 * \param dynamicPressure Dynamic pressure at which the body undergoing the acceleration flies.
 * \param referenceArea Reference area of the aerodynamic coefficients.
 * \param aerodynamicCoefficients. Aerodynamic coefficients in right-handed reference frame.
 * \param vehicleMass Mass of vehicle undergoing acceleration.
 * \return Resultant aerodynamic acceleration, given in reference frame in which the
 *          aerodynamic coefficients were given.
 */
Eigen::VectorXd computeAerodynamicAcceleration( const double dynamicPressure,
                                                const double referenceArea,
                                                const Eigen::Vector3d& aerodynamicCoefficients,
                                                const double vehicleMass )
{
    return force_models::computeAerodynamicForce( dynamicPressure,referenceArea,
                                                  aerodynamicCoefficients ) / vehicleMass;
}

//! Compute the aerodynamic acceleration in same reference frame as input coefficients.
/*!
 * This function computes the aerodynamic acceleration. It takes the dynamic pressure and an
 * aerodynamic coefficient interface as input. The coefficient interface has to have been
 * updated with current vehicle conditions before being passed to this function. Aerodynamic
 * coefficients and reference area are then retrieved from it.
 * \param dynamicPressure Dynamic pressure at which the body undergoing the acceleration flies.
 * \param coefficientInterface AerodynamicCoefficientInterface class from which reference area
 *          and coefficients are retrieved.
 * \param vehicleMass Mass of vehicle undergoing acceleration.
 * \return Resultant aerodynamic acceleration, given in reference frame in which the
 *          aerodynamic coefficients were given.
 */
Eigen::MatrixXd computeAerodynamicAcceleration(
        const double dynamicPressure, AerodynamicCoefficientInterface& coefficientInterface,
        const double vehicleMass )
{
    return force_models::computeAerodynamicForce( dynamicPressure, coefficientInterface )
            / vehicleMass;
}

} // namespace acceleration_models
} // namespace astrodynamics
} // namespace tudat

#endif // TUDAT_AERODYNAMIC_ACCELERATION_H