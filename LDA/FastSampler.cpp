#include "FastSampler.h"
#include <algorithm>
#include <cmath>
#include <iostream>
using namespace std;

FastSampler::FastSampler(Corpus* corpus) : GibbsSampler(corpus) 
{
    this->initialized = false;
    sum_p = new double[corpus->K + 1];
    Z = new double[corpus->K + 1];
    topic = new int[corpus->K + 1];        // 精确计算的话题的顺序
}

// 抽样中使用的变量名基本参照论文的伪代码
// 其中，只是论文中的k替换成了i，因为此时的i不再是一个topic，而是检查的次序
// 真正的topic是topic[i]
int FastSampler::sample(int doc, int token, int iteration)
{
    if (!initialized)
        this->fastInitialize(doc, token);

    int t = corpus->doc_token_list[doc][token].word_no;         // 这个token对应的单词序号
    int old_topic = corpus->doc_token_list[doc][token].topic;   // 这个token原本的topic序号

    excludeToken(doc, t, old_topic);

    sum_p[0] = 0;
    double current_norm_a = norm_a[doc];
    double current_norm_b = norm_b[t];


    double u = 1.0 * rand() / RAND_MAX;
    int new_topic = -1;
    
    for (int i = 1; i <= corpus->K; i++)
    {
        int k = sorted_N_kd[doc]->get(corpus->K - i).rank;    // 在本文档中的N_kd，越大的越先考虑
        topic[i] = k;
        // k: 第i个进行精确计算的话题
        sum_p[i] = sum_p[i - 1] + 1.0 * vector_a[doc][k] * vector_b[t][k] * vector_c[k];

        current_norm_a = current_norm_a - vector_a[doc][k] * vector_a[doc][k];
        current_norm_b = current_norm_b - vector_b[t][k] * vector_b[t][k];

        if (i != corpus->K)
            Z[i] = sum_p[i] + sqrt(current_norm_a) * sqrt(current_norm_b)
                / (sorted_N_k->get(0).value + corpus->beta_sum);
        else
            Z[i] = sum_p[i];

        if (u * Z[i] <= sum_p[i])
        {
            if (i == 1 || u * Z[i] > sum_p[i - 1])
                new_topic = k;
            else
            {
                u = (u * Z[i - 1] - sum_p[i - 1]) * Z[i] / (Z[i - 1] - Z[i]);
                for (int t = 1; t <= i; t++)
                {
                    if (sum_p[t] >= u)
                    {
                        new_topic = topic[t];
                        break;
                    }
                }
            }
            break;
        }
    }

    includeToken(doc, t, new_topic);     // 更新需要维护的数据结构
    return new_topic;
}

void FastSampler::excludeToken(int doc, int word, int old_topic)
{
    norm_a[doc] = norm_a[doc] - 2 * vector_a[doc][old_topic] + 1;
    norm_b[word] = norm_b[word] - 2 * vector_b[word][old_topic] + 1;

    vector_a[doc][old_topic]--;
    vector_b[word][old_topic]--;
    vector_c[old_topic] = 1.0 / (corpus->topic_token_sum[old_topic] + corpus->beta_sum); 

    sorted_N_k->decreaseOriginal(old_topic);
    sorted_N_kd[doc]->decreaseOriginal(old_topic);
}

void FastSampler::includeToken(int doc, int word, int new_topic)
{
    norm_a[doc] = norm_a[doc] + 2 * vector_a[doc][new_topic] + 1;
    norm_b[word] = norm_b[word] + 2 * vector_b[word][new_topic] + 1;

    vector_a[doc][new_topic]++;
    vector_b[word][new_topic]++;
    vector_c[new_topic] = 1.0 / (corpus->topic_token_sum[new_topic] + 1 + corpus->beta_sum); 

    sorted_N_k->increaseOriginal(new_topic);
    sorted_N_kd[doc]->increaseOriginal(new_topic);
}

void FastSampler::fastInitialize(int doc, int token)
{
    int old_k = corpus->doc_token_list[doc][token].topic;    // 原来的话题标记，用于恢复总计数
    int word = corpus->doc_token_list[doc][token].word_no;

    vector_a = new double*[corpus->M];
    norm_a = new double[corpus->M];
    for (int m = 0; m < corpus->M; m++)
    {
        vector_a[m] = new double[corpus->K];
        norm_a[m] = 0;
        for (int k = 0; k < corpus->K; k++)
        {
            // 恢复计数为总数
            vector_a[m][k] = corpus->doc_by_topic_cnt[m][k] + 
                int(m == doc && k == old_k) + corpus->alpha[k];
            norm_a[m] += vector_a[m][k] * vector_a[m][k];
        }
    }

    vector_b = new double*[corpus->V];
    norm_b = new double[corpus->V];
    for (int v = 0; v < corpus->V; v++)
    {
        vector_b[v] = new double[corpus->K];
        norm_b[v] = 0;
        for (int k = 0; k < corpus->K; k++)
        {
            vector_b[v][k] = corpus->topic_by_word_cnt[k][v] + 
                int(v == word && k == old_k) + corpus->beta[v];
            norm_b[v] += vector_b[v][k] * vector_b[v][k];
        }
    }

    vector_c = new double[corpus->K];
    for (int k = 0; k < corpus->K; k++)
    {
        vector_c[k] = 1.0 / (corpus->topic_token_sum[k] + int(k == old_k) + corpus->beta_sum);
    }

    sorted_N_k = new SortedValueRankList(corpus->topic_token_sum, corpus->K);
    sorted_N_k->increaseOriginal(old_k);    // 恢复计数

    sorted_N_kd = new SortedValueRankList*[corpus->M];
    for (int m = 0; m < corpus->M; m++)
    {
        sorted_N_kd[m] = new SortedValueRankList(corpus->doc_by_topic_cnt[m], corpus->K);

    }
    sorted_N_kd[doc]->increaseOriginal(old_k);

    this->initialized = true;
}

FastSampler::~FastSampler()
{
    if (this->initialized)
    {
        for (int m = 0; m < corpus->M; m++)
        {
            delete[] vector_a[m];
            delete sorted_N_kd[m];
        }
        delete[] vector_a;
        delete[] sorted_N_kd;

        for (int v = 0; v < corpus->V; v++)
        {
            delete[] vector_b[v];
        }
        delete[] vector_b;

        delete[] vector_c;
        delete[] norm_a;
        delete[] norm_b;
        delete sorted_N_k;
    }
    delete[] sum_p;
    delete[] Z;
    delete[] topic;
}