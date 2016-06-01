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

    sum_p[0] = 0;
    double current_norm_a = norm_a[doc];
    double current_norm_b = norm_b[t];


    double u = 1.0 * rand() / RAND_MAX;
    int new_topic = -1;
    //long long start = clock();
    for (int i = 1; i <= corpus->K; i++)
    {
        topic[i] = sorted_N_kd[doc][corpus->K - i].rank;    // 在本文档中的N_kd，越大的越先考虑
        // k: 第i个进行精确计算的话题
        sum_p[i] = sum_p[i - 1] + 1.0 * (corpus->doc_by_topic_cnt[doc][topic[i]] + corpus->alpha[topic[i]]) *
                   (corpus->topic_by_word_cnt[topic[i]][t] + corpus->beta[t]) /
                   (corpus->topic_token_sum[topic[i]] + corpus->beta_sum);

        current_norm_a = current_norm_a - vector_a[doc][topic[i]] * vector_a[doc][topic[i]];
        current_norm_b = current_norm_b - vector_b[t][topic[i]] * vector_b[t][topic[i]];
        if (i != corpus->K)
            Z[i] = sum_p[i] + sqrt(current_norm_a) * sqrt(current_norm_b)
                / (sorted_N_k[0].value + corpus->beta_sum);
        else
            Z[i] = sum_p[i];

        if (u * Z[i] <= sum_p[i])
        {
            if (i == 1 || u * Z[i] > sum_p[i - 1])
                new_topic = topic[i];
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
    //cout << clock() - start << endl;
    update(doc, t, old_topic, new_topic);     // 更新需要维护的数据结构
    return new_topic;
}

void FastSampler::update(int doc, int word, int old_topic, int new_topic)
{
    if (old_topic == new_topic)
        return;

    norm_a[doc] = norm_a[doc] - 2 * vector_a[doc][old_topic] + 1
                              + 2 * vector_a[doc][new_topic] + 1;   // 平方差公式
    norm_b[word] = norm_b[word] - 2 * vector_b[word][old_topic] + 1
                                + 2 * vector_b[word][new_topic] + 1;
    vector_a[doc][old_topic]--;
    vector_a[doc][new_topic]++;
    vector_b[word][old_topic]--;
    vector_b[word][new_topic]++;

    sorted_N_k[sorted_N_k_rank[old_topic]].value--;
    sorted_N_k[sorted_N_k_rank[new_topic]].value++;
    //value_rank::bubbleSort(sorted_N_k, sorted_N_k_rank, corpus->K);
    value_rank::swapAdjust(sorted_N_k, sorted_N_k_rank, corpus->K, sorted_N_k_rank[old_topic]);
    value_rank::swapAdjust(sorted_N_k, sorted_N_k_rank, corpus->K, sorted_N_k_rank[new_topic]);

    sorted_N_kd[doc][sorted_N_kd_rank[doc][old_topic]].value--;
    sorted_N_kd[doc][sorted_N_kd_rank[doc][new_topic]].value++;
    //value_rank::bubbleSort(sorted_N_kd[doc], sorted_N_kd_rank[doc], corpus->K);
    value_rank::swapAdjust(sorted_N_kd[doc], sorted_N_kd_rank[doc], corpus->K, sorted_N_kd_rank[doc][old_topic]);
    value_rank::swapAdjust(sorted_N_kd[doc], sorted_N_kd_rank[doc], corpus->K, sorted_N_kd_rank[doc][new_topic]);
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

    sorted_N_k = new value_rank[corpus->K];
    sorted_N_k_rank = new int[corpus->K];
    for (int k = 0; k < corpus->K; k++)
    {
        sorted_N_k[k].rank = k;
        sorted_N_k[k].value = corpus->topic_token_sum[k] + int(k == old_k);
    }
    sort(sorted_N_k, sorted_N_k + corpus->K);
    for (int k = 0; k < corpus->K; k++)
    {
        sorted_N_k_rank[sorted_N_k[k].rank] = k;
    }

    sorted_N_kd = new value_rank*[corpus->M];
    sorted_N_kd_rank = new int*[corpus->M];
    for (int m = 0; m < corpus->M; m++)
    {
        sorted_N_kd[m] = new value_rank[corpus->K];
        sorted_N_kd_rank[m] = new int[corpus->K];

        for (int k = 0; k < corpus->K; k++)
        {
            sorted_N_kd[m][k].rank = k;
            sorted_N_kd[m][k].value = corpus->doc_by_topic_cnt[m][k] +
                int(m == doc && k == old_k);
        }
        sort(sorted_N_kd[m], sorted_N_kd[m] + corpus->K);
        for (int k = 0; k < corpus->K; k++)
        {
            sorted_N_kd_rank[m][sorted_N_kd[m][k].rank] = k;
        }
    }

    this->initialized = true;
}

FastSampler::~FastSampler()
{
    if (this->initialized)
    {
        for (int m = 0; m < corpus->M; m++)
        {
            delete[] vector_a[m];
            delete[] sorted_N_kd[m];
            delete[] sorted_N_kd_rank[m];
        }
        delete[] vector_a;
        delete[] sorted_N_kd;
        delete[] sorted_N_kd_rank;

        for (int v = 0; v < corpus->V; v++)
        {
            delete[] vector_b[v];
        }
        delete[] vector_b;

        delete norm_a;
        delete norm_b;
        delete sorted_N_k;
        delete sorted_N_k_rank;


    }
    delete[] sum_p;
    delete[] Z;
    delete[] topic;
}