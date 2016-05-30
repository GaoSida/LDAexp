#include "Corpus.h"
#include <fstream>
#include <sstream>
#include <iostream>
using namespace std;

Corpus::Corpus(int K)
{
    this->K = K;
    this->topic_token_sum = new int[K];
    this->alpha = new double[K];
    // 超参alpha初始化为默认值
    for (int k = 0; k < K; k++)
        this->alpha[k] = 50.0 / K;
}

Corpus::~Corpus()
{
    delete[] ground_truth;
    delete[] N;
    delete[] topic_token_sum;
    delete[] alpha;
    delete[] beta;

    for (int m = 0; m < M; m++)
    {
        delete[] doc_token_list[m];
        delete[] doc_by_topic_cnt[m];
        delete[] doc_by_topic_distribution[m];
    }
    delete[] doc_token_list;
    delete[] doc_by_topic_cnt;
    delete[] doc_by_topic_distribution;

    for (int k = 0; k < K; k++)
    {
        delete[] topic_by_word_cnt;
        delete[] topic_by_word_distribution;
    }
}

void Corpus::defaultRead(string dir, int accept)
{
    // 先统计文件总数
    int current_doc_cnt = 0;
    while (true)
    {
        stringstream ss;
        ss << dir << current_doc_cnt << ".txt";
        ifstream fin(ss.str().c_str());
        if (fin)
        {
            current_doc_cnt++;
            fin.close();
        }
        else
            break;
    }

    M = current_doc_cnt;
    ground_truth = new int[M];
    N = new int[M];
    doc_token_list = new Token*[M];

    // 开始读文件
    N_sum = 0;
    for (int m = 0; m < M; m++)
    {
        stringstream ss;
        ss << dir << m << ".txt";
        ifstream fin(ss.str().c_str());

        // 第一行是ground_truth
        fin >> ground_truth[m];
        // 之后每行一个token
        string token;
        vector<int> document;
        while (fin >> token)
        {
            if (token.length() < accept)
                continue;
            if (dictionary.find(token) == dictionary.end())
            {
                // 加入新的词汇
                dictionary[token] = V;
                word_list.push_back(token);
                V++;
            }
            document.push_back(dictionary[token]);
        }
        fin.close();

        N[m] = document.size();
        N_sum += N[m];
        doc_token_list[m] = new Token[N[m]];
        for (int n = 0; n < N[m]; n++)
            doc_token_list[m][n].word_no = document[n];
    }

    // 不需要读入的变量也进行初始化
    doc_by_topic_cnt = new int*[M];
    doc_by_topic_distribution = new double*[M];
    for (int m = 0; m < M; m++)
    {
        doc_by_topic_cnt[m] = new int[K];
        doc_by_topic_distribution[m] = new double[K];
    }

    topic_by_word_cnt = new int*[K];
    topic_by_word_distribution = new double*[K];
    for (int k = 0; k < K; k++)
    {
        topic_by_word_cnt[k] = new int[V];
        topic_by_word_distribution[k] = new double[V];
    }

    topic_token_sum = new int[K];

    // 超参数beta设置为默认值
    beta = new double[V];
    for (int v = 0; v < V; v++)
        beta[v] = 0.01;
    beta_sum = 0.01 * V;
}

void Corpus::calcDistributions()
{
    for (int m = 0; m < M; m++)
        for (int k = 0; k < K; k++)
        {
            doc_by_topic_distribution[m][k] =
                1.0 * doc_by_topic_cnt[m][k] / N[m];
        }

    for (int k = 0; k < K; k++)
        for (int v = 0; v < V; v++)
        {
            topic_by_word_distribution[k][v] = 
                1.0 * topic_by_word_cnt[k][v] / topic_token_sum[k];
        }
}






