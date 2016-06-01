#include "LDAFrame.h"
#include "OriginalSampler.h"
#include "FastSampler.h"
#include "Evaluator.h"
#include "JudgeByIteration.h"
#include <iostream>

using namespace std;

#define NUMBER_OF_TOPIC 1000
#define MIN_ACCEPT_LENGTH 2
#define MAX_ITERATION 1000
#define INFO_INTERVAL 10
#define DATA_DIR "../stemmed_text/"

/*
    1000 Topic 在 400 篇小文档 上面 10次迭代的时间
    原始：60s左右（一直如此）
    Fast：30s左右（进过多轮迭代后下降下来） 有一定随机性
*/

int main()
{
    Corpus* corpus = new Corpus(NUMBER_OF_TOPIC);
    corpus->defaultRead(DATA_DIR, MIN_ACCEPT_LENGTH);
    
    GibbsSampler* sampler = new OriginalSampler(corpus);
    //GibbsSampler* sampler = new FastSampler(corpus);
    Evaluator* evaluator = new Evaluator(corpus);
    ConvergenceJudge* judge = new JudgeByIteration(corpus, evaluator, MAX_ITERATION);

    LDAFrame* lda = new LDAFrame(corpus, sampler, evaluator, judge);
    lda->solve(TIME_INTERVAL, INFO_INTERVAL);

    evaluator->printInfo(TOPIC_SUMMARY, MAX_ITERATION);

    return 0;
}