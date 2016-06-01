#ifndef __LDAEXP_ORIGINAL_SAMPLER__
#define __LDAEXP_ORIGINAL_SAMPLER__

#include "GibbsSampler.h"
#include <iostream>
#include <ctime>
using namespace std;

// 原始的抽样方法O(K)

class OriginalSampler : public GibbsSampler
{
    double* weight;
public:
    OriginalSampler(Corpus* corpus) : GibbsSampler(corpus)
    {
         weight = new double[corpus->K];
    }

    ~OriginalSampler()
    {
        delete weight;
    }
    
    int sample(int doc, int token, int iteration)
    {
        //long long start = clock();
        // 计算各个话题被抽出的权重
        double weight_sum = 0;

        int t = corpus->doc_token_list[doc][token].word_no;         // 这个token对应的单词序号

        for (int k = 0; k < corpus->K; k++)
        {
            weight[k] = 1.0 * (corpus->doc_by_topic_cnt[doc][k] + corpus->alpha[k]) *
                        (corpus->topic_by_word_cnt[k][t] + corpus->beta[t]) /
                        (corpus->topic_token_sum[k] + corpus->beta_sum);
            weight_sum += weight[k];
        }

        // 抽出一个话题
        double u = 1.0 * rand() / RAND_MAX;
        double p_sum = 0;     // 累积概率
        for (int k = 0; k < corpus->K; k++)
        {
            p_sum += weight[k] / weight_sum;
            if (u < p_sum)
            {
                //cout << clock() - start << endl;
                return k;
            }
        }
        // 不可能执行下面的语句
        return -1;
    }
};

#endif