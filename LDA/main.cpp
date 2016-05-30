#include "LDAFrame.h"
#include "OriginalSampler.h"
#include "Evaluator.h"
#include "JudgeByIteration.h"
#include <iostream>

using namespace std;

#define NUMBER_OF_TOPIC 4
#define MIN_ACCEPT_LENGTH 2
#define MAX_ITERATION 1000
#define INFO_INTERVAL 100
#define DATA_DIR "../stemmed_text/"

int main()
{
    Corpus* corpus = new Corpus(NUMBER_OF_TOPIC);
    corpus->defaultRead(DATA_DIR, MIN_ACCEPT_LENGTH);
    
    GibbsSampler* sampler = new OriginalSampler(corpus);
    Evaluator* evaluator = new Evaluator(corpus);
    ConvergenceJudge* judge = new JudgeByIteration(corpus, evaluator, MAX_ITERATION);

    LDAFrame* lda = new LDAFrame(corpus, sampler, evaluator, judge);
    lda->solve(PERPLEXITY, INFO_INTERVAL);

    evaluator->printInfo(TOPIC_PREDICTION, MAX_ITERATION);

    return 0;
}