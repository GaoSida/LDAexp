#ifndef __LDAEXP_FAST_SAMPLER__
#define __LDAEXP_FAST_SAMPLER__

#include "GibbsSampler.h"

// FastLDA 文中提出的算法。
// 本算法纯粹用数学trick加速抽样过程，不存在任何近似。
// 即，得到的结果应该和原来的LDA完全一致。

class FastSampler : public GibbsSampler
{
public:
    FastSampler(Corpus* corpus) : GibbsSampler(corpus) {}

    int sample(int doc, int token, int iteration)
    {
        
    }
};

#endif