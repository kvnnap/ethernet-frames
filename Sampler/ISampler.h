//
// Created by kvnna on 22/06/2016.
//

#ifndef ISAMPLER_H
#define ISAMPLER_H

#include <stddef.h>
#include <vector>

namespace Sampler
{
    class ISampler {
    public:
        virtual ~ISampler();

        virtual float nextSample() = 0;
        virtual float nextSample(float min, float max) = 0;
        virtual std::vector<float> nextSamples(size_t p_numSamples) = 0;

        virtual size_t nextSample(size_t a, size_t b) = 0;
    };
}

#endif //ISAMPLER_H
