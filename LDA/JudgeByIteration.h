#ifndef __LDAEXP_JUDGE_BY_ITERATION__
#define __LDAEXP_JUDGE_BY_ITERATION__

#include "ConvergenceJudge.h"

class JudgeByIteration : public ConvergenceJudge
{
    int target_iteration;
public:
    JudgeByIteration(Corpus* corpus, Evaluator* evaluator, int target)
         : ConvergenceJudge(corpus, evaluator)
    {
        target_iteration = target;
    }
    bool isConvergence(int iteration)
    {
        return iteration >= target_iteration;
    }
};

#endif