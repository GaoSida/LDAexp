#ifndef __LDAEXP_LDAFRAME__
#define __LDAEXP_LDAFRAME__

#include "Corpus.h"
#include "GibbsSampler.h"
#include "Evaluator.h"
#include "ConvergenceJudge.h"


class LDAFrame
{
    Corpus* corpus;
    GibbsSampler* sampler;
    Evaluator* evaluator;
    ConvergenceJudge* judge;
    void initialize();        // 初始化：随机分配话题，并随之更新计数器
public:
    LDAFrame(Corpus* corpus, GibbsSampler* sampler, Evaluator* evaluator, ConvergenceJudge* judge);
    void solve(int info, int interval);     // 参数指定执行中输出什么信息，多少次循环输出一次
};

#endif