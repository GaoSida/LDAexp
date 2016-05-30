#ifndef __LDAEXP_GIBBS_SAMPLER__
#define __LDAEXP_GIBBS_SAMPLER__

#include "Corpus.h"
#include <cstdlib>

// Sampler操作的边界：
// 它不改动Corpus中的任何参数的值

class GibbsSampler
{
protected:
    Corpus* corpus;
public:
    GibbsSampler(Corpus* corpus)
    {
        this->corpus = corpus;
    }
    virtual int sample(int doc, int token, int iteration) = 0;   // 对doc号文档的token个词进行抽样。 
                    // 传入当前语料库状态和迭代次数（部分策略需要）。返回该token新的话题编号。
};

#endif