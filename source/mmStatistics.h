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

namespace Statistics {

	struct Results {
		double min;
		double max;
		double mean;
		double variance;
		double stdDev;
	};

	// sampler is a lambda func that takes size_t parameter and return associated 
	// sample value as a double (use closure to store the iterated array of values).
	template <typename F>
	inline void compute(size_t nbSamples, F&& sampler, Results& output) {

		double fract = 1.0 / nbSamples;
		output.mean = 0.0;
		output.min = std::numeric_limits<double>::max();
		output.max = std::numeric_limits<double>::min();
		
		// compute mean, min and max
		for (size_t i = 0; i < nbSamples; ++i) {
			const double sample = sampler(i);
			output.mean += fract * sample;
			output.max = std::max(output.max, sample);
			output.min = std::min(output.min, sample);
		}
		// compute variance
		output.variance = 0.0;
		for (size_t i = 0; i < nbSamples; ++i) {
			output.variance += fract * pow(sampler(i) - output.mean, 2.0);
		}
		output.stdDev = sqrt(output.variance);
	}

	// 
	inline void printToLog(Results& stats, std::string prefix, std::ostream& out) {
		out << prefix << "Min=" << stats.min << std::endl;
		out << prefix << "Max=" << stats.max << std::endl;
		out << prefix << "Mean=" << stats.mean << std::endl;
		out << prefix << "Variance=" << stats.variance << std::endl;
		out << prefix << "StdDev=" << stats.stdDev << std::endl;
	}
	
};

#endif
