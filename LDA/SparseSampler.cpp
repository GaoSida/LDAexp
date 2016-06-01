#include "SparseSampler.h"

using namespace std;

SparseSampler::SparseSampler(Corpus* corpus) : GibbsSampler(corpus)
{
    initialized = false;
}

void SparseSampler::sparseInitialize(int old_topic, int word)
{
    G = 0;
    g = new double[corpus->K];
    c = new double[corpus->K];
    for (int k = 0; k < corpus->K; k++)
    {
        c[k] = corpus->alpha[old_topic] /
            (corpus->beta_sum + corpus->topic_token_sum[k] + int(old_topic == k));
        g[k] = c[k] * corpus->beta[word];
        G += g[k];
    }
}

void SparseSampler::startDocument()
{
    
}

int SparseSampler::sample(int doc, int token, int iteration)
{
    int old_topic = corpus->doc_token_list[doc][token].topic;
    int word = corpus->doc_token_list[doc][token].word_no;

    if (!initialized)
        sparseInitialize(old_topic, word);
    if (token == 0)
        startDocument();
    startToken();



    endToken();
    if (token == corpus->N[doc] - 1)
        endDocument();
}






