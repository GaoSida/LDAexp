#ifndef __LDAEXP_LIGHT_SAMPLER__
#define __LDAEXP_LIGHT_SAMPLER__

#include "GibbsSampler.h"
#include "AliasDrawer.h"

#define MH_STEP 4
#define STALE 2

class LightSampler : public GibbsSampler
{
    AliasDrawer** word_drawers;         // V 个抽样器
    int* word_drawer_timers;            // 记录每个抽样器剩余的有效使用次数
    double** word_proposal;             // V * K 随时更新
    double* word_proposal_sum;          // 长度V，随时更新
    double** stale_word_proposal;       // V * K 构造抽样器时的较为陈旧的值
    double* stale_word_proposal_sum;    // 长度V，构造抽样器时的较为陈旧的值

    AliasDrawer** doc_drawers;          // M 个抽样器
    int* doc_drawer_timers;             // 记录每个抽样器剩余的有效使用次数
    double** doc_proposal;              // M * K 随时更新
    double* doc_proposal_sum;           // 长度M，随时更新
    double** stale_doc_proposal;        // M * K 构造抽样器时的较为陈旧的值
    double* stale_doc_proposal_sum;     // 长度M，构造抽样器时的较为陈旧的值

    bool initialized;
    double* proba;                      // 长度为K
    void lightInitialize(int doc, int word, int old_topic);
public:
    LightSampler(Corpus* corpus);
    ~LightSampler();
    int sample(int doc, int token, int iteration);
};

#endif