#ifndef __LDAEXP_ALIAS_SAMPLER__
#define __LDAEXP_ALIAS_SAMPLER__

#include "GibbsSampler.h"

class AliasSampler : public GibbsSampler
{
public:
    AliasSampler(Corpus* corpus);
    ~AliasSampler();
};


#endif