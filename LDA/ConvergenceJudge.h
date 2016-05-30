#ifndef __LDAEXP_CONVERGENCE_JUDGE__
#define __LDAEXP_CONVERGENCE_JUDGE__

#include "Evaluator.h"
#include "Corpus.h"

class ConvergenceJudge
{
protected:
    Corpus* corpus;
    Evaluator* evaluator;
public:
    ConvergenceJudge(Corpus* corpus, Evaluator* evaluator)
    {
        this->corpus = corpus;
        this->evaluator = evaluator;
    }
    virtual bool isConvergence(int iteration) = 0;     // 判定收敛的时候也参考次数
};

#endif