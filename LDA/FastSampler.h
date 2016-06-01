#ifndef __LDAEXP_FAST_SAMPLER__
#define __LDAEXP_FAST_SAMPLER__

#include "GibbsSampler.h"
#include "SortedValueRankPair.h"

// FastLDA 文中提出的算法。
// 本算法纯粹用数学trick加速抽样过程，不存在任何近似。
// 即，得到的结果应该和原来的LDA完全一致。

class FastSampler : public GibbsSampler
{
    double** vector_a;          // M * K 的矩阵，存放 N_kd + alpha，即每篇文档中每个话题的计数。
    double** vector_b;          // V * K 的矩阵，存放 N_wk + beta，即每个单词在每个话题中的计数。

    double* norm_a;             // 长度为M的向量，每行向量a的二范数。没有开方。
    double* norm_b;             // 长度为V的向量，每行向量b的二范数。没有开方。

    value_rank* sorted_N_k;     // 长度为K的有序向量。为了方便地找到N_k的最小值。
    int* sorted_N_k_rank;       // 长度为K的向量。由rank，映射到在有序向量N_k中的位置。
    value_rank** sorted_N_kd;   // M * K 的矩阵。每行有序，按照每个文本中每个话题的个数从大到小排序。
    int** sorted_N_kd_rank;     // M * K 的矩阵。由rank，映射到在有序向量中的位置。

    bool initialized;
    void fastInitialize(int doc, int token);
    void update(int doc, int word, int old_topic, int new_topic);

    double* sum_p;
    double* Z;
    int* topic;                 // 精确计算的话题的顺序

public:
    FastSampler(Corpus* corpus);
    ~FastSampler();
    int sample(int doc, int token, int iteration);
};

#endif