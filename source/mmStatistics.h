// ************* COPYRIGHT AND CONFIDENTIALITY INFORMATION *********
// Copyright 2021 - InterDigital
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
// http ://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissionsand
// limitations under the License.
//
// Author: jean-eudes.marvie@interdigital.com
// *****************************************************************

#ifndef _MM_STATISTICS_H_
#define _MM_STATISTICS_H_

#include <algorithm>	// for std::min and std::max
#include <cmath>		// for pow and sqrt,
#include <limits>		// for nan

namespace Statistics {

	struct Results {
		double min;
		double max;
		double mean;
		double variance;
		double stdDev;
		long double sum; // use long double to reduce risks of overflow
		double minkowsky;
	};

	// sampler is a lambda func that takes size_t parameter and return associated 
	// sample value as a double (use closure to store the iterated array of values).
	template <typename F>
	inline void compute(size_t nbSamples, F&& sampler, Results& output) {

		output.min = std::numeric_limits<double>::quiet_NaN();
		output.max = std::numeric_limits<double>::quiet_NaN();
		output.mean = std::numeric_limits<double>::quiet_NaN();
		output.variance = std::numeric_limits<double>::quiet_NaN();
		output.stdDev = std::numeric_limits<double>::quiet_NaN();
		output.sum = std::numeric_limits<double>::quiet_NaN();
		output.minkowsky = std::numeric_limits<double>::quiet_NaN();

		if (nbSamples == 0) {
			return;
		}

		double fract = 1.0 / nbSamples;
		output.minkowsky = 0.0;
		
		// init mean, min and max
		const double sample = sampler(0);
		output.mean = fract * sample; // we do not use sum for the computation since it might be overflow
		output.max = sample;
		output.min = sample;
		output.sum = sample;
		// compute mean, min and max
		for (size_t i = 1; i < nbSamples; ++i) {
			const double sample = sampler(i);
			output.mean += fract * sample; // we do not use sum for the computation since it might be overflow
			output.max = std::max(output.max, sample);
			output.min = std::min(output.min, sample);
			output.sum += sample;
		}
		// compute variance
		output.variance = 0.0;
		for (size_t i = 0; i < nbSamples; ++i) {
			output.variance += fract * pow(sampler(i) - output.mean, 2.0);
		}
		output.stdDev = sqrt(output.variance);

		// compute Minkowsky with parameter ms=3
		const double ms = 3;
		// finalize the code here
		for (size_t i = 0; i < nbSamples; i++) {
			output.minkowsky += fract * pow(abs(sampler(i)), ms);
		}
		output.minkowsky = pow(output.minkowsky, 1.0 / ms);
	}

	// 
	inline void printToLog(Results& stats, std::string prefix, std::ostream& out) {
		out << prefix << "Min=" << stats.min << std::endl;
		out << prefix << "Max=" << stats.max << std::endl;
		out << prefix << "Sum=" << stats.sum << std::endl;
		out << prefix << "Mean=" << stats.mean << std::endl;
		out << prefix << "Variance=" << stats.variance << std::endl;
		out << prefix << "StdDev=" << stats.stdDev << std::endl;
		out << prefix << "Minkowsky=" << stats.minkowsky << std::endl;
	}
	
};

#endif
