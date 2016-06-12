#include "LightSampler.h"
#include <iostream>
using namespace std;

LightSampler::LightSampler(Corpus* corpus) : GibbsSampler(corpus)
{
    word_drawers = new AliasDrawer*[corpus->V];
    word_drawer_timers = new int[corpus->V];
    word_proposal = new double*[corpus->V];
    word_proposal_sum = new double[corpus->V];
    stale_word_proposal = new double*[corpus->V];
    stale_word_proposal_sum = new double[corpus->V];
    for (int v = 0; v < corpus->V; v++)
    {
        word_drawers[v] = new AliasDrawer(corpus->K);
        word_proposal[v] = new double[corpus->K];
        stale_word_proposal[v] = new double[corpus->K];
        word_drawer_timers[v] = 0;
    }

    doc_drawers = new AliasDrawer*[corpus->M];
    doc_drawer_timers = new int[corpus->M];
    doc_proposal = new double*[corpus->M];
    doc_proposal_sum = new double[corpus->M];
    stale_doc_proposal = new double*[corpus->M];
    stale_doc_proposal_sum = new double[corpus->M];
    for (int m = 0; m < corpus->M; m++)
    {
        doc_drawers[m] = new AliasDrawer(corpus->K);
        doc_proposal[m] = new double[corpus->K];
        stale_doc_proposal[m] = new double[corpus->K];
        doc_drawer_timers[m] = 0;
    }

    proba = new double[corpus->K];
    initialized = false;
}

LightSampler::~LightSampler()
{
    for (int v = 0; v < corpus->V; v++)
    {
        delete word_drawers[v];
        delete[] word_proposal[v];
        delete[] stale_word_proposal[v];
    }
    delete[] word_drawers;
    delete[] word_drawer_timers;
    delete[] word_proposal;
    delete[] word_proposal_sum;
    delete[] stale_word_proposal;
    delete[] stale_word_proposal_sum;

    for (int m = 0; m < corpus->M; m++)
    {
        delete doc_drawers[m];
        delete[] doc_proposal[m];
        delete[] stale_doc_proposal[m];
    }
    delete[] doc_drawers;
    delete[] doc_drawer_timers;
    delete[] doc_proposal;
    delete[] doc_proposal_sum;
    delete[] stale_doc_proposal;
    delete[] stale_doc_proposal_sum;

    delete[] proba;
}

void LightSampler::lightInitialize(int doc, int word, int old_topic)
{
    for (int v = 0; v < corpus->V; v++)
    {
        word_proposal_sum[v] = 0;
        for (int k = 0; k < corpus->K; k++)
        {
            word_proposal[v][k] = (corpus->topic_by_word_cnt[k][v] + int(v == word && k == old_topic) + corpus->beta[v])
                / (corpus->topic_token_sum[k] + int(k == old_topic) + corpus->beta_sum);
            word_proposal_sum[v] += word_proposal[v][k];
        }
    }
    /*
    for (int i = 0; i < corpus->K; i++)
    {
        cout << word_proposal[1][i] << " ";
    }
    cout << endl;
    */
    for (int m = 0; m < corpus->M; m++)
    {
        doc_proposal_sum[m] = 0;
        for (int k = 0; k < corpus->K; k++)
        {
            doc_proposal[m][k] = (corpus->doc_by_topic_cnt[m][k] + int(m == doc && k == old_topic) + corpus->alpha[k]);
            doc_proposal_sum[m] += doc_proposal[m][k];
        }
    }
}

int LightSampler::sample(int doc, int token, int iteration)
{
    int new_topic = -1;
    int old_topic = corpus->doc_token_list[doc][token].topic;
    int word = corpus->doc_token_list[doc][token].word_no;

    if (!initialized)
    {
        lightInitialize(doc, word, old_topic);
        initialized = true;
    }

    // 更新各个实时计数的值
    word_proposal_sum[word] -= word_proposal[word][old_topic];
    word_proposal[word][old_topic] = (corpus->topic_by_word_cnt[old_topic][word] + corpus->beta[word])
                / (corpus->topic_token_sum[old_topic] + corpus->beta_sum);
    word_proposal_sum[word] += word_proposal[word][old_topic];

    doc_proposal_sum[doc] -= doc_proposal[doc][old_topic];
    doc_proposal[doc][old_topic] = (corpus->doc_by_topic_cnt[doc][old_topic] + corpus->alpha[old_topic]);
    doc_proposal_sum[doc] += doc_proposal[doc][old_topic];

    if (iteration % 2)
    {
        // 抽word分布
        //cout << "in word" << endl;
        if (word_drawer_timers[word] == 0)
        {
            if (word_proposal_sum[word] < 0)
            {
                cout << word << " " << word_proposal_sum[0] << endl;
                exit(0);
            }
            stale_word_proposal_sum[word] = word_proposal_sum[word];
            for (int k = 0; k < corpus->K; k++)
            {
                stale_word_proposal[word][k] = word_proposal[word][k];
                proba[k] = stale_word_proposal[word][k] / stale_word_proposal_sum[word];   
            }
            //cout << "start create" << endl;
            word_drawers[word]->createTable(proba);
            //cout << "end create" << endl;
            word_drawer_timers[word] = corpus->K;
        }

        word_drawer_timers[word]--;
        new_topic = word_drawers[word]->draw();

        double Pie_accept = doc_proposal[doc][new_topic] / doc_proposal[doc][old_topic] *
            word_proposal[word][new_topic] / word_proposal[word][old_topic] *
            stale_word_proposal[word][old_topic] / stale_word_proposal[word][new_topic];
        double roll = 1.0 * rand() / RAND_MAX;
        if (roll > Pie_accept)      // 拒绝
            new_topic = old_topic;
        //cout << "out word" << endl;
    }
    else
    {
        // 抽doc分布
        //cout << "in doc" << endl;
        if (doc_drawer_timers[doc] == 0)
        {
            stale_doc_proposal_sum[doc] = doc_proposal_sum[doc];
            for (int k = 0; k < corpus->K; k++)
            {
                stale_doc_proposal[doc][k] = doc_proposal[doc][k];
                proba[k] = stale_doc_proposal[doc][k] / stale_doc_proposal_sum[doc];   
            }
            doc_drawers[doc]->createTable(proba);
            doc_drawer_timers[doc] = corpus->K;
        }

        doc_drawer_timers[doc]--;
        new_topic = doc_drawers[doc]->draw();

        double Pie_accept = doc_proposal[doc][new_topic] / doc_proposal[doc][old_topic] *
            word_proposal[word][new_topic] / word_proposal[word][old_topic] *
            stale_doc_proposal[doc][old_topic] / stale_doc_proposal[doc][new_topic];
        double roll = 1.0 * rand() / RAND_MAX;
        if (roll > Pie_accept)      // 拒绝
            new_topic = old_topic;
        //cout << "out doc" << endl;
    }

    // 更新各个实时计数的值
    word_proposal_sum[word] -= word_proposal[word][new_topic];
    word_proposal[word][new_topic] = (corpus->topic_by_word_cnt[new_topic][word] + corpus->beta[word])
                / (corpus->topic_token_sum[new_topic] + corpus->beta_sum);
    word_proposal_sum[word] += word_proposal[word][new_topic];

    doc_proposal_sum[doc] -= doc_proposal[doc][new_topic];
    doc_proposal[doc][new_topic] = (corpus->doc_by_topic_cnt[doc][new_topic] + corpus->alpha[new_topic]);
    doc_proposal_sum[doc] += doc_proposal[doc][new_topic];
    
    return new_topic;
}








