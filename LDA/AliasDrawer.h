#ifndef __LDAEXP_ALIAS_DRAWER__
#define __LDAEXP_ALIAS_DRAWER__

#define ESP 1e-6
#include <cstdlib>

class AliasDrawer
{
    int N;                              // 总个数
    double* proba;                      // N个概率
    int* alias;                         // 替身表
public:
    AliasDrawer(int n);
    ~AliasDrawer();
    void createTable(double* p);        // 传入N维概率向量建表
    int draw();                         // 随机抽取一个 0 ~ N-1 的整数
};

#endif