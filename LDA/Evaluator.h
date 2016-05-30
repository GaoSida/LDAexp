#ifndef __LDAEXP_EVALUATOR__
#define __LDAEXP_EVALUATOR__

#include "Corpus.h"

#define NO_INFO 0x0
#define TOPIC_PREDICTION 0x1
#define PERPLEXITY 0x2

class Evaluator
{
    Corpus* corpus;
public:
    Evaluator(Corpus* corpus);
    //double loglikelihood();        // 根据文本类别的ground truth评价
    double perplexity();             // perplexity评价标准
    void printInfo(int info, int iteration);      // 根据info的设置输出信息
};

#endif