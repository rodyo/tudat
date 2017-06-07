/*    Copyright (c) 2010-2017, Delft University of Technology
 *    All rigths reserved
 *
 *    This file is part of the Tudat. Redistribution and use in source and
 *    binary forms, with or without modification, are permitted exclusively
 *    under the terms of the Modified BSD license. You should have received
 *    a copy of the license with this file. If not, please or visit:
 *    http://tudat.tudelft.nl/LICENSE.
 */

#ifndef TUDAT_SIMULATEOBSERVATIONS_H
#define TUDAT_SIMULATEOBSERVATIONS_H

#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>

#include "Tudat/Astrodynamics/ObservationModels/observationSimulator.h"

namespace tudat
{

namespace observation_models
{

//! Base struct for defining times at which observations are to be simulated.
/*!
 *  Base struct for defining times at which observations are to be simulated. Here, only the link end from which the
 *  observation is to be calculated is defined. Derived classes are used for defining the times themselves
 *  (either directly or through some algorithm).
 */
template< typename TimeType >
struct ObservationSimulationTimeSettings
{
    //! Constructor, defines link end type.
    /*!
     *  Constructor, defines link end type from which observations are to be simulated.
     *  \param linkEndType Link end type from which observations are to be simulated.
     */
    ObservationSimulationTimeSettings( const LinkEndType linkEndType ):linkEndType_( linkEndType ){ }

    //! Destructor.
    virtual ~ObservationSimulationTimeSettings( ){ }

    //! Link end type from which observations are to be simulated.
    LinkEndType linkEndType_;
};

template< typename TimeType >
struct TabulatedObservationSimulationTimeSettings: public ObservationSimulationTimeSettings< TimeType >
{
    TabulatedObservationSimulationTimeSettings(
            const LinkEndType linkEndType, const std::vector< TimeType >& simulationTimes ): ObservationSimulationTimeSettings< TimeType >( linkEndType ),
        simulationTimes_( simulationTimes ){ }

    ~TabulatedObservationSimulationTimeSettings( ){ }

    std::vector< TimeType > simulationTimes_;
};

//! Function to compute observations at times defined by settings object using a given observation model
/*!
 *  Function to compute observations at times defined by settings object using a given observation model
 *  \param observationsToSimulate Object that computes/defines settings for observation times/reference link end
 *  \param observationModel Observation model that is to be used to compute observations
 *  \return Pair of observable values and observation time (with associated reference link end)
 */
template< typename ObservationScalarType = double, typename TimeType = double,
          int ObservationSize = 1 >
std::pair< Eigen::Matrix< ObservationScalarType, Eigen::Dynamic, 1 >,std::pair< std::vector< TimeType >, LinkEndType > >
simulateSingleObservationSet(
        const boost::shared_ptr< ObservationSimulationTimeSettings< TimeType > > observationsToSimulate,
        const boost::shared_ptr< ObservationModel< ObservationSize, ObservationScalarType, TimeType > > observationModel )
{
    // Delcare return type.
    std::pair< Eigen::Matrix< ObservationScalarType, Eigen::Dynamic, 1 >, std::pair< std::vector< TimeType >, LinkEndType > >
            simulatedObservations;

    // Simulate observations from tabulated times.
    if( boost::dynamic_pointer_cast< TabulatedObservationSimulationTimeSettings< TimeType > >( observationsToSimulate ) != NULL )
    {
        boost::shared_ptr< TabulatedObservationSimulationTimeSettings< TimeType > > tabulatedObservationSettings =
                boost::dynamic_pointer_cast< TabulatedObservationSimulationTimeSettings< TimeType > >( observationsToSimulate );

        // Simulate observations at requested pre-defined time.
        simulatedObservations = simulateObservationsWithCheckAndLinkEndIdOutput<
                ObservationSize, ObservationScalarType, TimeType >(
                    tabulatedObservationSettings->simulationTimes_, observationModel, observationsToSimulate->linkEndType_ );

    }

    return simulatedObservations;
}

//! Function to simulate observations for single observable and single set of link ends.
/*!
 *  Function to simulate observations for single observable and single set of link ends. From the observation time settings and
 *  the observation simulator, the required observations are simulated and returned.
 *  \param observationsToSimulate Object that computes/defines settings for observation times/reference link end
 *  \param observationSimulator Observation simulator for observable for which observations are to be calculated.
 *  \param linkEnds Link end set for which observations are to be calculated.
 *  \return Pair of first: vector of observations; second: vector of times at which observations are taken
 *  (reference to link end defined in observationsToSimulate).
 */
template< typename ObservationScalarType = double, typename TimeType = double, int ObservationSize = 1 >
std::pair< Eigen::Matrix< ObservationScalarType, Eigen::Dynamic, 1 >,std::pair< std::vector< TimeType >, LinkEndType > >
simulateSingleObservationSet(
        const boost::shared_ptr< ObservationSimulationTimeSettings< TimeType > > observationsToSimulate,
        const boost::shared_ptr< ObservationSimulator< ObservationSize, ObservationScalarType, TimeType > > observationSimulator,
        const LinkEnds& linkEnds )
{
    if( observationSimulator == NULL )
    {
        throw std::runtime_error( "Error when simulating single observation set, Observation simulator is NULL" );
    }

    return simulateSingleObservationSet< ObservationScalarType, TimeType, ObservationSize >(
                observationsToSimulate, observationSimulator->getObservationModel( linkEnds ) );
}

//! Function to generate ObservationSimulationTimeSettings objects from simple time list input.
/*!
 *  Function to generate ObservationSimulationTimeSettings objects, as required for observation simulation from
 *  simulateSingleObservationSet from simple time vectors input.
 *  \param originalMap List of observation times per link end set per observable type.
 *  \return TabulatedObservationSimulationTimeSettings objects from time list input (originalMap)
 */
template< typename TimeType = double >
std::map< ObservableType, std::map< LinkEnds, boost::shared_ptr< ObservationSimulationTimeSettings< TimeType > > > >
createObservationSimulationTimeSettingsMap(
        const std::map< ObservableType, std::map< LinkEnds, std::pair< std::vector< TimeType >, LinkEndType > > >&
        originalMap )
{
    // Declare return map.
    std::map< ObservableType, std::map< LinkEnds, boost::shared_ptr< ObservationSimulationTimeSettings< TimeType > > > >
            newMap;

    // Iterate over all observables.
    for( typename std::map< ObservableType, std::map< LinkEnds, std::pair< std::vector< TimeType >, LinkEndType > > >::
         const_iterator it = originalMap.begin( ); it != originalMap.end( ); it++ )
    {
        // Iterate over all link end sets
        for( typename std::map< LinkEnds, std::pair< std::vector< TimeType >, LinkEndType > >::const_iterator
             linkEndIterator = it->second.begin( ); linkEndIterator != it->second.end( ); linkEndIterator++ )
        {
            // Create ObservationSimulationTimeSettings from vector of times.
            newMap[ it->first ][ linkEndIterator->first ] = boost::make_shared<
                    TabulatedObservationSimulationTimeSettings< TimeType > >(
                        linkEndIterator->second.second, linkEndIterator->second.first );
        }
    }
    return newMap;
}

//! Function to simulate observations from set of observables and link and sets and simple vectors of requested times.
/*!
 *  Function to simulate observations from set of observables and link and sets and simple vectors of requested times.
 *  Function calls function for creation of ObservationSimulationTimeSettings and subsequently uses these for function call
 *  to observation simulation function.
 *  \param observationsToSimulate List of observation times per link end set per observable type.
 *  \param observationSimulators List of Observation simulators per link end set per observable type.
 *  \return Simulated observatoon values and associated times for requested observable types and link end sets.
 */
template< typename ObservationScalarType = double, typename TimeType = double >
std::map< ObservableType, std::map< LinkEnds, std::pair< Eigen::Matrix< ObservationScalarType, Eigen::Dynamic, 1 >,
std::pair< std::vector< TimeType >, LinkEndType > > > >
simulateObservations(
        const std::map< ObservableType, std::map< LinkEnds, std::pair< std::vector< TimeType >, LinkEndType > > >&
        observationsToSimulate,
        const std::map< ObservableType, boost::shared_ptr< ObservationSimulatorBase< ObservationScalarType, TimeType > > >&
        observationSimulators )
{
    return simulateObservations< ObservationScalarType, TimeType >(
                createObservationSimulationTimeSettingsMap( observationsToSimulate ), observationSimulators );
}

//! Function to simulate observations from set of observables and link and sets
/*!
 *  Function to simulate observations from set of observables, link ends and observation time settings
 *  Iterates over all observables and link ends and simulates observations.
 *  \param observationsToSimulate List of observation time settings per link end set per observable type.
 *  \param observationSimulators List of Observation simulators per link end set per observable type.
 *  \return Simulated observatoon values and associated times for requested observable types and link end sets.
 */
template< typename ObservationScalarType = double, typename TimeType = double >
std::map< ObservableType, std::map< LinkEnds, std::pair< Eigen::Matrix< ObservationScalarType, Eigen::Dynamic, 1 >,
std::pair< std::vector< TimeType >, LinkEndType > > > >
simulateObservations(
        const std::map< ObservableType, std::map< LinkEnds,
        boost::shared_ptr< ObservationSimulationTimeSettings< TimeType > > > >& observationsToSimulate,
        const std::map< ObservableType,
        boost::shared_ptr< ObservationSimulatorBase< ObservationScalarType, TimeType > > >& observationSimulators )
{
    // Declare return map.
    std::map< ObservableType, std::map< LinkEnds, std::pair< Eigen::Matrix< ObservationScalarType, Eigen::Dynamic, 1 >,
            std::pair< std::vector< TimeType >, LinkEndType > > > > observations;

    // Iterate over all observables.
    for( typename std::map< ObservableType, std::map< LinkEnds,
         boost::shared_ptr< ObservationSimulationTimeSettings< TimeType > >  > >::const_iterator observationIterator =
         observationsToSimulate.begin( ); observationIterator != observationsToSimulate.end( ); observationIterator++ )
    {
        // Iterate over all link ends for current observable.
        for( typename std::map< LinkEnds,
             boost::shared_ptr< ObservationSimulationTimeSettings< TimeType > > >::const_iterator linkEndIterator =
             observationIterator->second.begin( ); linkEndIterator != observationIterator->second.end( ); linkEndIterator++ )
        {
            int observationSize = observationSimulators.at( observationIterator->first )->getObservationSize(
                        linkEndIterator->first );

            switch( observationSize )
            {
            case 1:
            {
                boost::shared_ptr< ObservationSimulator< 1, ObservationScalarType, TimeType > > derivedObservationSimulator =
                        boost::dynamic_pointer_cast< ObservationSimulator< 1, ObservationScalarType, TimeType > >(
                            observationSimulators.at( observationIterator->first ) );

                if( derivedObservationSimulator == NULL )
                {
                    throw std::runtime_error( "Error when simulating observation: dynamic case to size 1 is NULL" );
                }

                // Simulate observations for current observable and link ends set.
                observations[ observationIterator->first ][ linkEndIterator->first ] =
                        simulateSingleObservationSet< ObservationScalarType, TimeType, 1 >(
                            linkEndIterator->second, derivedObservationSimulator,
                            linkEndIterator->first );
                break;
            }
            case 2:
            {
                boost::shared_ptr< ObservationSimulator< 2, ObservationScalarType, TimeType > > derivedObservationSimulator =
                        boost::dynamic_pointer_cast< ObservationSimulator< 2, ObservationScalarType, TimeType > >(
                            observationSimulators.at( observationIterator->first ) );

                if( derivedObservationSimulator == NULL )
                {
                    throw std::runtime_error( "Error when simulating observation: dynamic case to size 2 is NULL" );
                }

                // Simulate observations for current observable and link ends set.
                observations[ observationIterator->first ][ linkEndIterator->first ] =
                        simulateSingleObservationSet< ObservationScalarType, TimeType, 2 >(
                            linkEndIterator->second, derivedObservationSimulator,
                            linkEndIterator->first );
                break;
            }
            case 3:
            {
                boost::shared_ptr< ObservationSimulator< 3, ObservationScalarType, TimeType > > derivedObservationSimulator =
                        boost::dynamic_pointer_cast< ObservationSimulator< 3, ObservationScalarType, TimeType > >(
                            observationSimulators.at( observationIterator->first ) );

                if( derivedObservationSimulator == NULL )
                {
                    throw std::runtime_error( "Error when simulating observation: dynamic case to size 3 is NULL" );
                }

                // Simulate observations for current observable and link ends set.
                observations[ observationIterator->first ][ linkEndIterator->first ] =
                        simulateSingleObservationSet< ObservationScalarType, TimeType, 3 >(
                            linkEndIterator->second, derivedObservationSimulator, linkEndIterator->first );
                break;
            }
            default:
                throw std::runtime_error( "Error, simulation of observations not yet implemented for size " +
                                          boost::lexical_cast< std::string >( observationSize ) );

            }
        }
    }
    return observations;
}

//! Function to simulate observations with observation noise from set of observables and link and sets
/*!
 *  Function to simulate observations with observation noise from set of observables, link ends and observation time settings
 *  Iterates over all observables and link ends, simulates observations and adds noise according to given noise function.
 *  This function allows different noise functions to be defined for each observable/link ends combination
 *  \param observationsToSimulate List of observation time settings per link end set per observable type.
 *  \param observationSimulators List of Observation simulators per link end set per observable type.
 *  \param noiseFunctions Double map with functions that return the observation noise as a function of observation time.
 *  \return Simulated observatoon values and associated times for requested observable types and link end sets.
 */
template< typename ObservationScalarType = double, typename TimeType = double >
std::map< ObservableType, std::map< LinkEnds, std::pair< Eigen::Matrix< ObservationScalarType, Eigen::Dynamic, 1 >,
std::pair< std::vector< TimeType >, LinkEndType > > > >
simulateObservationsWithNoise(
        const std::map< ObservableType, std::map< LinkEnds,
        boost::shared_ptr< ObservationSimulationTimeSettings< TimeType > > > >& observationsToSimulate,
        const std::map< ObservableType,
        boost::shared_ptr< ObservationSimulatorBase< ObservationScalarType, TimeType > > >& observationSimulators,
        const std::map< ObservableType, std::map< LinkEnds, boost::function< Eigen::VectorXd( const double ) > > >& noiseFunctions )
{
    typedef std::map< LinkEnds, std::pair< Eigen::Matrix< ObservationScalarType, Eigen::Dynamic, 1 >,
            std::pair< std::vector< TimeType >, LinkEndType > > > SingelTypeObservationsMap;
    typedef std::map< ObservableType, std::map< LinkEnds, std::pair< Eigen::Matrix< ObservationScalarType, Eigen::Dynamic, 1 >,
            std::pair< std::vector< TimeType >, LinkEndType > > > > ObservationsMap;

    // Simulate noise-free observations
    ObservationsMap noiseFreeObservationsList = simulateObservations(
                observationsToSimulate, observationSimulators );

    // Declare return map with noisy observations.
    ObservationsMap noisyObservationsList;

    // Iterate over all observable types
    for( typename ObservationsMap::iterator observationIterator = noiseFreeObservationsList.begin( );
         observationIterator != noiseFreeObservationsList.end( ); observationIterator++ )
    {
        int currentObservableSize = getObservableSize( observationIterator->first );

        // Iterate over all link ends of current observable
        SingelTypeObservationsMap singleObservableObservationsWithNoise;
        for( typename SingelTypeObservationsMap::iterator linkIterator = observationIterator->second.begin( );
             linkIterator != observationIterator->second.end( ); linkIterator++ )
        {
            // Retrieve noise-free observations/times
            std::pair< Eigen::Matrix< ObservationScalarType, Eigen::Dynamic, 1 >,
                    std::pair< std::vector< TimeType >, LinkEndType > > nominalObservationsAndTimes =
                    linkIterator->second;
            Eigen::Matrix< ObservationScalarType, Eigen::Dynamic, 1 > noisyObservations = nominalObservationsAndTimes.first;
            std::vector< TimeType > nominalTimes = nominalObservationsAndTimes.second.first;

            // Check data consistency
            if( static_cast< int >( nominalTimes.size( ) ) != noisyObservations.rows( ) / currentObservableSize )
            {
                throw std::runtime_error( "Error when adding noise to observations, input data is inconsistent" );
            }

            // Retrieve noise function
            boost::function< Eigen::VectorXd( const double ) > currentNoiseFunction =
                    noiseFunctions.at( observationIterator->first ).at( linkIterator->first );

            // Simulate noise for all observations and add to calculated values
            for( unsigned int i = 0; i < nominalTimes.size( ); i++ )
            {
                // Check noise function consistency
                if( i == 0 )
                {
                    if( currentNoiseFunction( nominalTimes.at( i ) ).rows( ) != currentObservableSize )
                    {
                        throw std::runtime_error( "Error when adding noise to observations, noise size is inconsistent" );
                    }
                }

                // Add noise to observation
                noisyObservations.segment( i * currentObservableSize, currentObservableSize ) +=
                        currentNoiseFunction( nominalTimes.at( i ) );
            }

            singleObservableObservationsWithNoise[ linkIterator->first ] =
                    std::make_pair( noisyObservations, nominalObservationsAndTimes.second );
        }
        noisyObservationsList[ observationIterator->first ] = singleObservableObservationsWithNoise;
    }
    return noisyObservationsList;
}

Eigen::VectorXd getIdenticallyAndIndependentlyDistributedNoise(
        const boost::function< double( const double ) > noiseFunction,
        const int observationSize,
        const double evaluationTime )
{
    Eigen::VectorXd noiseValues = Eigen::VectorXd( observationSize );
    for( int i = 0; i < observationSize; i++ )
    {
        noiseValues( i ) = noiseFunction( evaluationTime );
    }
    return noiseValues;
}

//! Function to simulate observations with observation noise from set of observables and link and sets
/*!
 *  Function to simulate observations with observation noise from set of observables, link ends and observation time settings
 *  Iterates over all observables and link ends, simulates observations and adds noise according to given noise function.
 *  This function allows different noise functions to be defined for each observable/link ends combination. However, the noise
 *  functions required as input are defined as doubles. For multi-valued observables (e.g. angular position), the noise function
 *  will be equal for all entries.
 *  \param observationsToSimulate List of observation time settings per link end set per observable type.
 *  \param observationSimulators List of Observation simulators per link end set per observable type.
 *  \param noiseFunctions Double map with functions that return the observation noise as a function of observation time.
 *  \return Simulated observatoon values and associated times for requested observable types and link end sets.
 */
template< typename ObservationScalarType = double, typename TimeType = double >
std::map< ObservableType, std::map< LinkEnds, std::pair< Eigen::Matrix< ObservationScalarType, Eigen::Dynamic, 1 >,
std::pair< std::vector< TimeType >, LinkEndType > > > >
simulateObservationsWithNoise(
        const std::map< ObservableType, std::map< LinkEnds,
        boost::shared_ptr< ObservationSimulationTimeSettings< TimeType > > > >& observationsToSimulate,
        const std::map< ObservableType,
        boost::shared_ptr< ObservationSimulatorBase< ObservationScalarType, TimeType > > >& observationSimulators,
        const std::map< ObservableType, std::map< LinkEnds, boost::function< double( const double ) > > >& noiseFunctions )
{
    // Create noise map for input to simulation function
    std::map< ObservableType, std::map< LinkEnds, boost::function< Eigen::VectorXd( const double ) > > > noiseVectorFunctions;
    for( std::map< ObservableType, std::map< LinkEnds, boost::function< double( const double ) > > >::const_iterator noiseIterator =
         noiseFunctions.begin( ); noiseIterator != noiseFunctions.end( ); noiseIterator++ )
    {
        for( std::map< LinkEnds, boost::function< double( const double ) > >::const_iterator
             linkEndIterator = noiseIterator->second.begin( ); linkEndIterator != noiseIterator->second.end( ); linkEndIterator++ )
        {
            noiseVectorFunctions[ noiseIterator->first ][ linkEndIterator->first ] =
                    boost::bind(
                        &getIdenticallyAndIndependentlyDistributedNoise, linkEndIterator->second,
                        getObservableSize( noiseIterator->first ), _1 );
        }
    }

    // Simulate observations with noise
    return simulateObservationsWithNoise( observationsToSimulate, observationSimulators, noiseVectorFunctions );
}

//! Function to simulate observations with observation noise from set of observables and link and sets
/*!
 *  Function to simulate observations with observation noise from set of observables, link ends and observation time settings
 *  Iterates over all observables and link ends, simulates observations and adds noise according to given noise function.
 *  This function allows different noise functions to be defined for each observable, but independent of link end.
 *  \param observationsToSimulate List of observation time settings per link end set per observable type.
 *  \param observationSimulators List of Observation simulators per link end set per observable type.
 *  \param noiseFunctions Map with functions that return the observation noise as a function of observation time.
 *  \return Simulated observatoon values and associated times for requested observable types and link end sets.
 */
template< typename ObservationScalarType = double, typename TimeType = double >
std::map< ObservableType, std::map< LinkEnds, std::pair< Eigen::Matrix< ObservationScalarType, Eigen::Dynamic, 1 >,
std::pair< std::vector< TimeType >, LinkEndType > > > >
simulateObservationsWithNoise(
        const std::map< ObservableType, std::map< LinkEnds,
        boost::shared_ptr< ObservationSimulationTimeSettings< TimeType > > > >& observationsToSimulate,
        const std::map< ObservableType,
        boost::shared_ptr< ObservationSimulatorBase< ObservationScalarType, TimeType > > >& observationSimulators,
        const std::map< ObservableType, boost::function< Eigen::VectorXd( const double ) > >& noiseFunctions )
{
    std::map< ObservableType, std::map< LinkEnds, boost::function< Eigen::VectorXd( const double ) > > > fullNoiseFunctions;

    // Create noise map for input to simulation function
    for( typename std::map< ObservableType, std::map< LinkEnds,
         boost::shared_ptr< ObservationSimulationTimeSettings< TimeType > >  > >::const_iterator observationIterator =
         observationsToSimulate.begin( ); observationIterator != observationsToSimulate.end( ); observationIterator++ )
    {
        // Check input consistency
        if( noiseFunctions.at( observationIterator->first ) == 0 )
        {
            throw std::runtime_error( "Error when setting observation noise function, missing observable" );
        }

        for( typename std::map< LinkEnds,
             boost::shared_ptr< ObservationSimulationTimeSettings< TimeType > > >::const_iterator linkEndIterator =
             observationIterator->second.begin( ); linkEndIterator != observationIterator->second.end( ); linkEndIterator++ )
        {
            fullNoiseFunctions[ observationIterator->first ][ linkEndIterator->first ] =
                   noiseFunctions .at( observationIterator->first );
        }
    }

    // Simulate observations with noise
    return simulateObservationsWithNoise( observationsToSimulate, observationSimulators, fullNoiseFunctions );
}

//! Function to simulate observations with observation noise from set of observables and link and sets
/*!
 *  Function to simulate observations with observation noise from set of observables, link ends and observation time settings
 *  Iterates over all observables and link ends, simulates observations and adds noise according to given noise function.
 *  This function allows different noise functions to be defined for each observable, but independent of link end. The noise
 *  functions required as input are defined as doubles. For multi-valued observables (e.g. angular position), the noise function
 *  will be equal for all entries.
 *  \param observationsToSimulate List of observation time settings per link end set per observable type.
 *  \param observationSimulators List of Observation simulators per link end set per observable type.
 *  \param noiseFunctions Map with functions that return the observation noise as a function of observation time.
 *  \return Simulated observatoon values and associated times for requested observable types and link end sets.
 */
template< typename ObservationScalarType = double, typename TimeType = double >
std::map< ObservableType, std::map< LinkEnds, std::pair< Eigen::Matrix< ObservationScalarType, Eigen::Dynamic, 1 >,
std::pair< std::vector< TimeType >, LinkEndType > > > >
simulateObservationsWithNoise(
        const std::map< ObservableType, std::map< LinkEnds,
        boost::shared_ptr< ObservationSimulationTimeSettings< TimeType > > > >& observationsToSimulate,
        const std::map< ObservableType,
        boost::shared_ptr< ObservationSimulatorBase< ObservationScalarType, TimeType > > >& observationSimulators,
        const std::map< ObservableType, boost::function< double( const double ) > >& noiseFunctions )
{
    // Create noise map for input to simulation function
    std::map< ObservableType, boost::function< Eigen::VectorXd( const double ) > > noiseVectorFunctions;
    for( std::map< ObservableType, boost::function< double( const double ) > >::const_iterator noiseIterator =
         noiseFunctions.begin( ); noiseIterator != noiseFunctions.end( ); noiseIterator++ )
    {
        noiseVectorFunctions[ noiseIterator->first ] =
                boost::bind(
                    &getIdenticallyAndIndependentlyDistributedNoise, noiseIterator->second,
                    getObservableSize( noiseIterator->first ), _1 );
    }
    return simulateObservationsWithNoise( observationsToSimulate, observationSimulators, noiseVectorFunctions );
}

//! Function to simulate observations with observation noise from set of observables and link and sets
/*!
 *  Function to simulate observations with observation noise from set of observables, link ends and observation time settings
 *  Iterates over all observables and link ends, simulates observations and adds noise according to given noise function.
 *  This function allows a single noise function to be provided, which is used for each observable/link end combination
 *  \param observationsToSimulate List of observation time settings per link end set per observable type.
 *  \param observationSimulators List of Observation simulators per link end set per observable type.
 *  \param noiseFunction Function that returns the observation noise as a function of observation time.
 *  \return Simulated observatoon values and associated times for requested observable types and link end sets.
 */
template< typename ObservationScalarType = double, typename TimeType = double >
std::map< ObservableType, std::map< LinkEnds, std::pair< Eigen::Matrix< ObservationScalarType, Eigen::Dynamic, 1 >,
std::pair< std::vector< TimeType >, LinkEndType > > > >
simulateObservationsWithNoise(
        const std::map< ObservableType, std::map< LinkEnds,
        boost::shared_ptr< ObservationSimulationTimeSettings< TimeType > > > >& observationsToSimulate,
        const std::map< ObservableType,
        boost::shared_ptr< ObservationSimulatorBase< ObservationScalarType, TimeType > > >& observationSimulators,
        const boost::function< double( const double ) >& noiseFunction )
{
    // Create noise map for input to simulation function
    std::map< ObservableType, boost::function< double( const double ) > > noiseFunctionList;
    for( typename std::map< ObservableType, std::map< LinkEnds,
         boost::shared_ptr< ObservationSimulationTimeSettings< TimeType > >  > >::const_iterator observationIterator =
         observationsToSimulate.begin( ); observationIterator != observationsToSimulate.end( ); observationIterator++ )
    {
        noiseFunctionList[ observationIterator->first ] = noiseFunction;
    }

    // Simulate observations with noise
    return simulateObservationsWithNoise( observationsToSimulate, observationSimulators, noiseFunctionList );
}

}

}
#endif // TUDAT_SIMULATEOBSERVATIONS_H
