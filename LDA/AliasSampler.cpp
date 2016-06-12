#include "AliasSampler.h"
#include <iostream>
using namespace std;

AliasSampler::AliasSampler(Corpus* corpus) : GibbsSampler(corpus)
{
    alias_drawers = new AliasDrawer*[corpus->V];
    alias_drawers_timer = new int[corpus->V];
    stale_q_wt = new double*[corpus->V];
    stale_Q_w = new double[corpus->V];
    dense_wt = new double*[corpus->V];
    dense_sum_w = new double[corpus->V];
    for (int i = 0; i < corpus->V; i++)
    {
        alias_drawers[i] = new AliasDrawer(corpus->K);
        alias_drawers_timer[i] = 0;
        stale_q_wt[i] = new double[corpus->K];
        dense_wt[i] = new double[corpus->K];
    }
    // 因为还没有分配初始的topic，所以实际的值要到LDA初始化之后再给
    sorted_document_topic_cnt = new SortedValueRankList*[corpus->M];
    
    initialized = false;
    proba = new double[corpus->K];
    sparse_value_list = new double[corpus->K];
    sparse_topic_list = new int[corpus->K];
}

AliasSampler::~AliasSampler()
{
    for (int i = 0; i < corpus->V; i++)
    {
        delete alias_drawers[i];
        delete[] stale_q_wt[i];
        delete[] dense_wt[i];
    }
    delete[] alias_drawers;
    delete[] alias_drawers_timer;
    delete[] stale_q_wt;
    delete[] stale_Q_w;
    delete[] dense_wt;
    delete[] dense_sum_w;

    if (initialized)
    {
        for (int m = 0; m < corpus->M; m++)
        {
            delete sorted_document_topic_cnt[m];
        }
    }
    delete[] sorted_document_topic_cnt;
    delete[] proba;
    delete[] sparse_value_list;
    delete[] sparse_topic_list;   
}

void AliasSampler::aliasInitialize(int doc, int word, int old_topic)
{
    // 由于alias采样器的计数初始值为0，因此初始化时不作处理。
    // 等到采样时会通过判断计数进行初始化。
    // 只需要初始化dense项的缓存值即可。
    // dense项中存储的一直都是总共的值。没有排除任何一个单词。为了缓存，排除一个无所谓。
    for (int v = 0; v < corpus->V; v++)
    {
        dense_sum_w[v] = 0;
        for (int k = 0; k < corpus->K; k++)
        {
            dense_wt[v][k] = corpus->alpha[k] * 
                (corpus->topic_by_word_cnt[k][v] + int(v == word && k == old_topic) + corpus->beta[v])
                / (corpus->topic_token_sum[k] + int(k == old_topic) + corpus->beta_sum);
            dense_sum_w[v] += dense_wt[v][k];
        }
    }

    // 对有序计数初始化
    for (int m = 0; m < corpus->M; m++)
    {
        sorted_document_topic_cnt[m] = new SortedValueRankList(corpus->doc_by_topic_cnt[m], corpus->K);
    }
    sorted_document_topic_cnt[doc]->increaseOriginal(old_topic);
}

int AliasSampler::sample(int doc, int token, int iteration)
{
    int new_topic = -1;
    int old_topic = corpus->doc_token_list[doc][token].topic;
    int word = corpus->doc_token_list[doc][token].word_no;

    if (!initialized)
    {
        aliasInitialize(doc, word, old_topic);
        initialized = true;
    }
    //cout << "start" << endl;
    // 进入一个新单词的更新动作：维护有序化的稀疏项
    sorted_document_topic_cnt[doc]->decreaseOriginal(old_topic);
    // 更新dense项
    dense_sum_w[word] -= dense_wt[word][old_topic];
    dense_wt[word][old_topic] = corpus->alpha[old_topic] * 
        (corpus->topic_by_word_cnt[old_topic][word] + corpus->beta[word])
        / (corpus->topic_token_sum[old_topic] + corpus->beta_sum);
    dense_sum_w[word] += dense_wt[word][old_topic];
    
    // 首先对稀疏项求和
    P_sparse_sum = 0;
    for (int i = 1; i <= corpus->K; i++)
    {
        int n_td = sorted_document_topic_cnt[doc]->get(corpus->K - i).value;
        if (n_td == 0)
            break;
        int k = sorted_document_topic_cnt[doc]->get(corpus->K - i).rank;
        sparse_topic_list[i - 1] = k;
        sparse_value_list[i - 1] = n_td * dense_wt[word][k];
        P_sparse_sum += sparse_value_list[i - 1];
    }
    //cout << "sparse done" << endl;
    // 判断dense项的alias采样器使用次数，如果达到K次则重新计算
    if (alias_drawers_timer[word] == 0)
    {
        for (int k = 0; k < corpus->K; k++)
        {
            stale_q_wt[word][k] = dense_wt[word][k];
            stale_Q_w[word] = dense_sum_w[word];
            proba[k] = stale_q_wt[word][k] / stale_Q_w[word];
        }
        //cout << "start create" << endl;
        alias_drawers[word]->createTable(proba);
        //cout << "done create" << endl;
        alias_drawers_timer[word] = corpus->K;
    }
    alias_drawers_timer[word]--;
    //cout << "alias drwer" << endl;
    // 开始抽样
    double sum_PQ = P_sparse_sum + stale_Q_w[word];
    double u = sum_PQ * 1.0 * rand() / RAND_MAX;
    if (u > P_sparse_sum)
    {
        //cout << "draw" << endl;
        new_topic = alias_drawers[word]->draw();
    }
    else
    {
        // 在稀疏项中抽样
        double sum_q = 0;
        for (int k = 0; k < corpus->K; k++)
        {
            sum_q += sparse_value_list[k];
            if (u < sum_q)
            {
                new_topic = sparse_topic_list[k];
                break;
            }
        } 
    }
    //cout << "sample done" << endl; 
    // Metropolis Hasting 判断是否接受 s->t
    int n_td = sorted_document_topic_cnt[doc]->getOriginal(new_topic).value;
    int n_sd = sorted_document_topic_cnt[doc]->getOriginal(old_topic).value;
    double propOldSample = dense_wt[word][old_topic];
    double propNewSample = dense_wt[word][new_topic];
    double Pie_accept = (n_td + corpus->alpha[new_topic]) / (n_sd + corpus->alpha[old_topic]) * 
                        (propNewSample / propOldSample) *
                        (corpus->alpha[new_topic] * propOldSample + n_sd * propOldSample) /
                        (stale_q_wt[word][new_topic] + n_td * propNewSample);

    double roll = 1.0 * rand() / RAND_MAX;
    if (roll > Pie_accept)    // 拒绝
    {
        new_topic = old_topic;
    }
    
    sorted_document_topic_cnt[doc]->increaseOriginal(new_topic);
    dense_sum_w[word] -= dense_wt[word][new_topic];
    dense_wt[word][new_topic] = corpus->alpha[new_topic] * 
        (corpus->topic_by_word_cnt[new_topic][word] + corpus->beta[word])
        / (corpus->topic_token_sum[new_topic] + corpus->beta_sum);
    dense_sum_w[word] += dense_wt[word][new_topic];
    //cout << "all done" << endl;
    return new_topic;
}





