#include "SparseSampler.h"
#include <algorithm>
#include <iostream>
using namespace std;

SparseSampler::SparseSampler(Corpus* corpus) : GibbsSampler(corpus)
{
    initialized = false;
}

void SparseSampler::sparseInitialize(int doc, int old_topic)
{
    initialized = true;
    // 本算法的数据结构
    G = 0;
    g = new double[corpus->K];
    c = new double[corpus->K];
    e = new double[corpus->K];
    f = new double[corpus->K];
    for (int k = 0; k < corpus->K; k++)
    {
        c[k] = corpus->alpha[old_topic] /
            (corpus->beta_sum + corpus->topic_token_sum[k] + int(old_topic == k));
        g[k] = c[k] * corpus->beta[0];
        G += g[k];
        f[k] = 0;
        e[k] = 0;
    }

    // 两个计数进行有序化
    sorted_document_topic_cnt = new SortedValueRankList*[corpus->M];
    for (int m = 0; m < corpus->M; m++)
    {
        sorted_document_topic_cnt[m] = new SortedValueRankList(corpus->doc_by_topic_cnt[m], corpus->K);
    }
    sorted_document_topic_cnt[doc]->increaseOriginal(old_topic);

    sorted_word_topic_cnt = new SortedValueRankList*[corpus->V];
    int* temp = new int[corpus->K];
    for (int v = 0; v < corpus->V; v++)
    {
        for (int k = 0; k < corpus->K; k++)
            temp[k] = corpus->topic_by_word_cnt[k][v];
        sorted_word_topic_cnt[v] = new SortedValueRankList(temp, corpus->K);
    }
    sorted_word_topic_cnt[doc]->increaseOriginal(old_topic);
    delete[] temp;
}

SparseSampler::~SparseSampler()
{
    delete[] g;
    delete[] c;
    delete[] f;
    delete[] e;

    for (int m = 0; m < corpus->M; m++)
        delete sorted_document_topic_cnt[m];
    delete[] sorted_document_topic_cnt;

    for (int v = 0; v < corpus->V; v++)
        delete sorted_word_topic_cnt[v];
    delete[] sorted_word_topic_cnt;
}

inline void SparseSampler::startDocument(int doc, int word, int old_topic)
{
    F = 0;

    for (int i = 1; i <= corpus->K; i++)
    {
        int n_kd = sorted_document_topic_cnt[doc]->get(corpus->K - i).value;
        if (n_kd == 0)
            break;
        int k = sorted_document_topic_cnt[doc]->get(corpus->K - i).rank;    // 现在计算的这个非0的话题

        // 计算c和f的时候，已经排除了第一个词
        c[k] = (n_kd + corpus->alpha[k]) / (corpus->topic_token_sum[k] + corpus->beta_sum);
        f[k] = (n_kd * corpus->beta[0]) / (corpus->topic_token_sum[k] + corpus->beta_sum);
        F += f[k];
    }
}

inline void SparseSampler::excludeToken(bool start_of_doc, int doc, int word, int old_topic)
{
    // 如果是文档的第一个词，c和f都不必再算了，计数也不必更新
    if (!start_of_doc)
    {
        int n_kd = sorted_document_topic_cnt[doc]->getOriginal(old_topic).value;

        c[old_topic] = (n_kd + corpus->alpha[old_topic]) 
            / (corpus->topic_token_sum[old_topic] + corpus->beta_sum);

        F -= f[old_topic];
        f[old_topic] = (n_kd * corpus->beta[0])
            / (corpus->topic_token_sum[old_topic] + corpus->beta_sum);
        F += f[old_topic];
    }

    G -= g[old_topic];
    g[old_topic] = corpus->alpha[old_topic] * corpus->beta[0]
        / (corpus->topic_token_sum[old_topic] + corpus->beta_sum);
    G += g[old_topic];
}

int SparseSampler::sample(int doc, int token, int iteration)
{
    int new_topic = -1;
    int old_topic = corpus->doc_token_list[doc][token].topic;
    int word = corpus->doc_token_list[doc][token].word_no;

    if (!initialized)
        sparseInitialize(doc, old_topic);

    sorted_document_topic_cnt[doc]->decreaseOriginal(old_topic);
    sorted_word_topic_cnt[word]->decreaseOriginal(old_topic);

    if (token == 0)
        startDocument(doc, word, old_topic);
    
    excludeToken(token == 0, doc, word, old_topic);

    // 开始抽样
    E = 0;
    for (int i = 1; i <= corpus->K; i++)
    {
        int n_kt = sorted_word_topic_cnt[word]->get(corpus->K - i).value;
        if (n_kt == 0)
            break;
        int k = sorted_word_topic_cnt[word]->get(corpus->K - i).rank;
        e[k] = n_kt * c[k];
        E += e[k];
    }


    double Q = E + F + G;
    double u = Q * 1.0 * rand() / RAND_MAX;

    if (u < E)
    {
        double sum_e = 0;
        for (int i = 1; i <= corpus->K; i++)
        {
            int k = sorted_word_topic_cnt[word]->get(corpus->K - i).rank;
            sum_e += e[k];
            if (u < sum_e)
            {
                new_topic = k;
                break;
            }
        }
    }
    else if (u < E + F)
    {
        double sum_f = E;
        for (int i = 1; i <= corpus->K; i++)
        {
            int k = sorted_document_topic_cnt[doc]->get(corpus->K - i).rank;
            sum_f += f[k];
            if (u < sum_f)
            {
                new_topic = k;
                break;
            }
        }
    }
    else
    {
        double sum_g = E + F;
        for (int k = 0; k < corpus->K; k++)
        {
            sum_g += g[k];
            if (u < sum_g)
            {
                new_topic = k;
                break;
            }
        }
    }

    sorted_document_topic_cnt[doc]->increaseOriginal(new_topic);
    sorted_word_topic_cnt[word]->increaseOriginal(new_topic);
    
    includeToken(doc, word, new_topic);
    if (token == corpus->N[doc] - 1)
        endDocument(doc, new_topic);
    
    return new_topic;
}

inline void SparseSampler::includeToken(int doc, int word, int new_topic)
{
    int n_k = corpus->topic_token_sum[new_topic] + 1;
    int n_kd = sorted_document_topic_cnt[doc]->getOriginal(new_topic).value;

    c[new_topic] = (n_kd + corpus->alpha[new_topic]) / (n_k + corpus->beta_sum);

    F -= f[new_topic];
    f[new_topic] = (n_kd * corpus->beta[0]) / (n_k + corpus->beta_sum);
    F += f[new_topic];
    
    G -= g[new_topic];
    g[new_topic] = corpus->alpha[new_topic] * corpus->beta[0] / (n_k + corpus->beta_sum);
    G += g[new_topic];
}

inline void SparseSampler::endDocument(int doc, int new_topic)
{
    // 重置c的值
    for (int i = 0; i < corpus->K; i++)
    {
        int n_kd = sorted_document_topic_cnt[doc]->get(i).value;
        if (n_kd == 0)
            break;
        int k = sorted_document_topic_cnt[doc]->get(i).rank;    // 现在计算的这个非0的话题

        // 需要恢复计数 N_k
        c[k] = corpus->alpha[k] /
               (corpus->topic_token_sum[k] + int(k == new_topic) + corpus->beta_sum);
    }
}



