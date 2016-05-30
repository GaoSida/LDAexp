#include "LDAFrame.h"
#include <cstdlib>
#include <iostream>
using namespace std;

LDAFrame::LDAFrame(Corpus* corpus, GibbsSampler* sampler, 
    Evaluator* evaluator, ConvergenceJudge* judge)
{
    this->corpus = corpus;
    this->sampler = sampler;
    this->evaluator = evaluator;
    this->judge = judge;
}

void LDAFrame::initialize()
{
    // 计数器清零
    for (int m = 0; m < corpus->M; m++)
        for (int k = 0; k < corpus->K; k++)
            corpus->doc_by_topic_cnt[m][k] = 0;

    for (int k = 0; k < corpus->K; k++)
        for (int t = 0; t < corpus->V; t++)
            corpus->topic_by_word_cnt[k][t] = 0;

    for (int k = 0; k < corpus->K; k++)
        corpus->topic_token_sum[k] = 0;

    // 随机分配初始话题
    srand((unsigned)time(0));
    for (int m = 0; m < corpus->M; m++)
        for (int n = 0; n < corpus->N[m]; n++)
        {
            int t = corpus->doc_token_list[m][n].word_no;     // token对应的单词序号
            int k = rand() % corpus->K;               // 本次抽出的话题
            corpus->doc_token_list[m][n].topic = k;
            corpus->doc_by_topic_cnt[m][k]++;
            corpus->topic_by_word_cnt[k][t]++;
            corpus->topic_token_sum[k]++;
        }
}

void LDAFrame::solve(int info, int interval)
{
    this->initialize();
    int iteration = 0;
    while (!judge->isConvergence(iteration))
    {
        iteration++;
        for (int m = 0; m < corpus->M; m++)
            for (int n = 0; n < corpus->N[m]; n++)
            {
                int t = corpus->doc_token_list[m][n].word_no;     // token对应的单词序号
                int k = corpus->doc_token_list[m][n].topic;    // 当前token标记的话题

                // 重新采样并更新计数
                corpus->doc_by_topic_cnt[m][k]--;
                corpus->topic_by_word_cnt[k][t]--;
                corpus->topic_token_sum[k]--;

                // 进入采样时，当前token已经不算在任何计数中了，所以里面不用对此做减1处理
                k = sampler->sample(m, n, iteration);
                corpus->doc_token_list[m][n].topic = k;

                corpus->doc_by_topic_cnt[m][k]++;
                corpus->topic_by_word_cnt[k][t]++;
                corpus->topic_token_sum[k]++;
            }
        if (iteration % interval == 0)
            evaluator->printInfo(info, iteration);
    }
    corpus->calcDistributions();
}



