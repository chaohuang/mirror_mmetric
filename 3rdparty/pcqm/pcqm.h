// Copyright (c) 2020 University of Lyon and CNRS (France).
// All rights reserved.
//
//The code is the implementation of our paper
//"PCQM: A Full-Reference Quality Metric for Colored 3D Point Clouds"
//Gabriel Meynet, Yana Nehme, Julie Digne, Guillaume Lavoué
//Presented at QoMEX 2020.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.

// When including this header in static linking, 
// one must define tinyply in one and only one CPP file (e.g main file).
// just before including pcqm.h
// Example:
// #define TINYPLY_IMPLEMENTATION
// #include "pcqm.h"

#ifndef  PCQM_H_
#define  PCQM_H_

#include <string>
#include "PointSet.h"

/**
* \brief Computes PCQM using point sets given as parameter
* \return EXIT_SUCCESS if the code executed successfuly.
*/
float compute_pcqm( 
    PointSet& refptset,
	PointSet& regptset,
    // associated file names (for logging)
    const std::string reffile,
	const std::string regfile,
	// PCQM params
	const double RadiusCurvature = 0.004,
	const int threshold_knnsearch = 20,
	const double radius_factor = 2.0);

/**
* \brief Computes PCQM using filenames given as parameter
* \return EXIT_SUCCESS if the code executed successfuly.
*/
float compute_pcqm( 
    const std::string reffile,
	const std::string regfile,
	// PCQM params
	const double RadiusCurvature = 0.004,
	const int threshold_knnsearch = 20,
	const double radius_factor = 2.0);

#endif
