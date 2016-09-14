//
// Created by kvnna on 22/06/2016.
//

#ifndef MSC_CANDELA_UNIFORMSAMPLER_H
#define MSC_CANDELA_UNIFORMSAMPLER_H


#include <random>
#include "ISampler.h"

namespace Sampler
{
    class UniformSampler
        : public ISampler
    {
    public:
        UniformSampler();
        // Virtuals
        float nextSample() override;
        float nextSample(float min, float max) override;
        std::vector<float> nextSamples(size_t p_numSamples) override;

        // Integers
        size_t nextSample(size_t a, size_t b) override;

    private:
        std::mt19937 mt;
        std::uniform_real_distribution<float> dist;
    };
}




#endif //MSC_CANDELA_UNIFORMSAMPLER_H
