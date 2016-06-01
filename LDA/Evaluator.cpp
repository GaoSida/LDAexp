#include "Evaluator.h"
#include <iostream>
#include <cmath>
#include <ctime>
using namespace std;

Evaluator::Evaluator(Corpus* corpus)
{
    this->corpus = corpus;
    last_time = time(0);
}

double Evaluator::perplexity()
{
    corpus->calcDistributions();

    double numerator = 0;
    for (int m = 0; m < corpus->M; m++)
        for (int n = 0; n < corpus->N[m]; n++)
        {
            int t = corpus->doc_token_list[m][n].word_no;    // 单词序号

            double p_sum = 0;
            for (int k = 0; k < corpus->K; k++)
                p_sum += corpus->doc_by_topic_distribution[m][k] *
                    corpus->topic_by_word_distribution[k][t];

            numerator += log(p_sum);
        }

    return exp(- numerator / corpus->N_sum);
}

void Evaluator::printInfo(int info, int iteration)
{
    if (info == NO_INFO)
        return;

    cout << "iteration " << iteration << ":" << endl;
    if ((info & PERPLEXITY) == PERPLEXITY)
    {
        cout << "perplexity = " << perplexity() << "; ";
    }
    if ((info & TIME_INTERVAL) == TIME_INTERVAL)
    {
        int current_time = time(0);
        cout << "time interval = " << current_time - last_time << "; ";
        last_time = current_time; 
    }
    if ((info & TOPIC_PREDICTION) == TOPIC_PREDICTION || (info & TOPIC_SUMMARY) == TOPIC_SUMMARY)
    {
        // 统计每个ground_truth类下，聚出的每个类的大小。大小为 K * K。
        int** cluster_result = new int*[corpus->K];
        for (int k = 0; k < corpus->K; k++)
        {
            cluster_result[k] = new int[corpus->K];
            for (int kk = 0; kk < corpus->K; kk++)
                cluster_result[k][kk] = 0;
        }
        for (int m = 0; m < corpus->M; m++)
        {
            double p_max_topic = 0;
            int max_topic = 0;
            for (int k = 0; k < corpus->K; k++)
            {
                if (corpus->doc_by_topic_distribution[m][k] > p_max_topic)
                {
                    max_topic = k;
                    p_max_topic = corpus->doc_by_topic_distribution[m][k];
                }
            }
            if ((info & TOPIC_PREDICTION) == TOPIC_PREDICTION)
                cout << "doc " << m << ": topic " << max_topic << "; proba = " << p_max_topic << endl;

            cluster_result[corpus->ground_truth[m]][max_topic]++;
        }
        if ((info & TOPIC_SUMMARY) == TOPIC_SUMMARY)
        {
            for (int t = 0; t < corpus->K; t++)
            {
                int max_cluster = 0;
                int max_cluster_cnt = cluster_result[t][0];
                int cluster_sum = 0;

                for (int k = 0; k < corpus->K; k++)
                {
                    cluster_sum += cluster_result[t][k];
                    if (cluster_result[t][k] > max_cluster_cnt)
                    {
                        max_cluster_cnt = cluster_result[t][k];
                        max_cluster = k;
                    }
                }
                cout << "ground_truth=" << t << ": max_cluster=" 
                    << max_cluster << " @ " << 1.0 * max_cluster_cnt / cluster_sum << endl;
            }
        }
    }

    cout << endl;
}
