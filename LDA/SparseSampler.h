#ifndef __LDAEXP_SPARSE_SAMPLER__
#define __LDAEXP_SPARSE_SAMPLER__

#include "GibbsSampler.h"
#include "SortedValueRankPair.h"

class SparseSampler : public GibbsSampler
{
    // 算法中的符号，参照博客 http://www.flickering.cn/nlp/2014/10/lda工程实践之算法篇-2-sparselda算法/
    double G;               // smoothing only bucket
    double* g;              // 长度为k，G中的每一项
    double* c;              // 长度为k，word topic bucket的系数。以文档为单位进行初始化和重置。
    double F;               // document topic bucket
    double* f;              // 长度为k，F中的每一项。以文档为单位进行初始化。

    // 有序化，模拟稀疏存储。非零的会集中在前面。
    // 并没有进行很好的封装，初始化操作需要做在外面。
    value_rank** sorted_document_topic_cnt;
    int** sorted_document_topic_rank;
    value_rank** sorted_word_topic_cnt;
    int** sorted_word_topic_rank;

    bool initialized;
    void sparseInitialize(int old_topic, int word);
    void startDocument();
    void endDocument();
    void startToken();
    void endToken();
public:
    SparseSampler(Corpus* corpus);
    ~SparseSampler();
    int sample(int doc, int token, int iteration);
};


#endif