#ifndef __LDAEXP_ALIAS_SAMPLER__
#define __LDAEXP_ALIAS_SAMPLER__

#include "GibbsSampler.h"
#include "AliasDrawer.h"
#include "SortedValueRankPair.h"

class AliasSampler : public GibbsSampler
{
    // 由于这是一个通用的抽样器，所以在空间上有所浪费：我们额外存储了归一化之后的概率
    AliasDrawer** alias_drawers;        // V 个alias抽样器，O(k)建立，O(1)采样
    int* alias_drawers_timer;           // V 个alias抽样器的定时器，减到0之后，需要重新建表
    double** stale_q_wt;                // 记录当前alias抽样器中，构造时的老q_wt
    double* stale_Q_w;                  // 记录当前alias抽样器中，构造时的老Q_w
    double** dense_wt;                  // V * K，随时更新抽样的最新概率。均摊O(1)。对应论文中的q_wt项（最新的）。
    double* dense_sum_w;                // V，随时更新的抽样的归一化项。对应论文中的Q_w项。

    // 临时向量，避免反复的new
    double* proba;                      // 长度为K的向量，计算归一化概率的容器
    double* sparse_value_list;          // 长度为K的向量，存放了稀疏化的项
    int* sparse_topic_list;             // 长度为K的向量，存放了稀疏化项对应的话题
    double P_sparse_sum;                // 论文中的P项，即稀疏项的和

    SortedValueRankList** sorted_document_topic_cnt;         // 对N_kd进行模拟稀疏化存储

    bool initialized;
    void aliasInitialize(int doc, int word, int old_topic);  // 在第一次调用时进行数据成员的初始化
public:
    AliasSampler(Corpus* corpus);
    ~AliasSampler();
    int sample(int doc, int token, int iteration);
};


#endif