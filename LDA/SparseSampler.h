#ifndef __LDAEXP_SPARSE_SAMPLER__
#define __LDAEXP_SPARSE_SAMPLER__

#include "GibbsSampler.h"
#include "SortedValueRankPair.h"

// 一种非近似的算法
// 进行项的拆分之后，优化的一个重要基础是beta的每个分量的值都一样
// 这个值，在伪代码中写作beta，在我们这里是 corpus->beta[0]

class SparseSampler : public GibbsSampler
{
    // 算法中的符号，参照博客 http://www.flickering.cn/nlp/2014/10/lda工程实践之算法篇-2-sparselda算法/
    double G;               // smoothing only bucket
    double* g;              // 长度为k，G中的每一项
    double* c;              // 长度为k，word topic bucket的系数。以文档为单位进行初始化和重置。
    double F;               // document topic bucket
    double* f;              // 长度为k，F中的每一项。以文档为单位进行初始化。
    double E;               // word topic bucket。每个单词都要现算。
    double* e;              // 长度为K，E中的每一项

    // 有序化，模拟稀疏存储。非零的会集中在前面。
    SortedValueRankList** sorted_document_topic_cnt;
    SortedValueRankList** sorted_word_topic_cnt;

    bool initialized;
    void sparseInitialize(int doc, int old_topic);
    void startDocument(int doc, int word, int old_topic);     // 进入一篇文档时的准备
    void endDocument(int doc, int new_topic);                 // 离开一篇文档时的善后
    void excludeToken(bool start_of_doc, int doc, int word, int old_topic);    // 在采样之前做
    void includeToken(int doc, int word, int new_topic);      // 采样之后收尾
public:
    SparseSampler(Corpus* corpus);
    ~SparseSampler();
    int sample(int doc, int token, int iteration);
};


#endif