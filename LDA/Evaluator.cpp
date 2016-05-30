#include "Evaluator.h"
#include <iostream>
#include <cmath>
using namespace std;

Evaluator::Evaluator(Corpus* corpus)
{
    this->corpus = corpus;
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
    if ((info & TOPIC_PREDICTION) == TOPIC_PREDICTION)
    {
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
            cout << "doc " << m << ": topic " << max_topic << "; proba = " << p_max_topic << endl;
        }
    }
    cout << endl;
}
