//
// Created by kevin on 8/4/16.
//

#ifndef STATISTICS_H
#define STATISTICS_H

#include <vector>
#include <cmath>

namespace Mathematics {
    namespace Statistics {

        template <class T>
        T Mean(const std::vector<T>& samples) {
            T val = 0;
            for (const T& sample : samples) {
                val += sample;
            }
            return val / samples.size();
        }

        template <class T>
        T SampleVariance(const std::vector<T>& samples) {
            if (samples.size() <= 1) {
                return 0;
            }
            T meanOfSamples = Mean(samples);
            T sum = 0;
            for (const T& sample : samples) {
                sum += pow(sample - meanOfSamples, static_cast<T>(2));
            }
            return sum / (samples.size() - 1);
        }

        template <class T>
        T Variance(const std::vector<T>& samples) {
            if (samples.size() == 0) {
                return 0;
            }
            T meanOfSamples = Mean(samples);
            T sum = 0;
            for (const T& sample : samples) {
                sum += pow(sample - meanOfSamples, static_cast<T>(2));
            }
            return sum / samples.size();
        }

        template <class T>
        T StdDeviation(const std::vector<T>& samples) {
            return sqrt(Variance(samples));
        }

        template <class T>
        T SampleStdDeviation(const std::vector<T>& samples) {
            return sqrt(SampleVariance(samples));
        }

        template <class T>
        T StdDeviationOfTheMean(T stdDeviation, std::size_t sampleSize) {
            return stdDeviation / sqrt(sampleSize);
        }

    };
}


#endif //STATISTICS_H
