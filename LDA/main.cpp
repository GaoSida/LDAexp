#include "LDAFrame.h"
#include "OriginalSampler.h"
#include "FastSampler.h"
#include "SparseSampler.h"
#include "Evaluator.h"
#include "JudgeByIteration.h"
#include <iostream>

#include "AliasDrawer.h"

using namespace std;

#define NUMBER_OF_TOPIC 1000
#define MIN_ACCEPT_LENGTH 2
#define MAX_ITERATION 1000
#define INFO_INTERVAL 10
#define DATA_DIR "../stemmed_text/"

/*
    1000 Topic 在 400 篇小文档 上面 10次迭代的时间
        > 原始：60~62s左右（一直如此）
        > FastLDA：13~15s左右（进过几十轮迭代后下降下来）
        > SparseLDA：3~4s左右
*/

int main()
{
    srand((unsigned)time(0));
    /*
        即便不改变随机种子，三种精确解法的中间结果也不会完全一样。
        因为后两种方法考虑概率的顺序不同。（即，对于 0~1 区间上的划分不一样）
    */

    Corpus* corpus = new Corpus(NUMBER_OF_TOPIC);
    corpus->defaultRead(DATA_DIR, MIN_ACCEPT_LENGTH);
    
    //GibbsSampler* sampler = new OriginalSampler(corpus);
    GibbsSampler* sampler = new FastSampler(corpus);
    //GibbsSampler* sampler = new SparseSampler(corpus);
    Evaluator* evaluator = new Evaluator(corpus);
    ConvergenceJudge* judge = new JudgeByIteration(corpus, evaluator, MAX_ITERATION);

    LDAFrame* lda = new LDAFrame(corpus, sampler, evaluator, judge);
    lda->solve(TIME_INTERVAL, INFO_INTERVAL);

    evaluator->printInfo(TOPIC_SUMMARY, MAX_ITERATION);

    return 0;
}