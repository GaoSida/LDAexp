#include "AliasDrawer.h"
#include <queue>
#include <iostream>
using namespace std;

AliasDrawer::AliasDrawer(int n)
{
    N = n;
    proba = new double[N];
    alias = new int[N];
}
AliasDrawer::~AliasDrawer()
{
    delete[] proba;
    delete[] alias;
}

void AliasDrawer::createTable(double* p)
{
    queue<int> underfull;
    queue<int> overfull;

    // 拷贝概率，得到两个队列的初始值
    for (int i = 0; i < N; i++)
    {
        proba[i] = p[i] * N;
        if (proba[i] < 1 - ESP)
            underfull.push(i);
        else if (proba[i] > 1 + ESP)
            overfull.push(i);
        else
            alias[i] = i;
    }
    // 构造 alias 表
    while (!underfull.empty())
    {
        int under = underfull.front();
        underfull.pop();
        int over = overfull.front();
        overfull.pop();
        
        alias[under] = over;
        proba[over] -= (1 - proba[under]);

        if (proba[over] < 1 - ESP && proba[over] > 0)
            underfull.push(over);
        else if (proba[over] > 1 + ESP)
            overfull.push(over);
        else
            alias[over] = over;
    }
}
    
int AliasDrawer::draw()
{
    int sample = (1.0 * N * rand() / RAND_MAX);

    if (1.0 * rand() / RAND_MAX < proba[sample])
        return sample;
    else
        return alias[sample];
}

// 验证正确性
/*
    double p[7] = {0.05, 0.15, 0.12, 0.28, 0.3, 0.03, 0.07};
    int cnt[7] = {0, 0, 0, 0, 0, 0, 0};
    AliasDrawer* ad = new AliasDrawer(7);
    ad->createTable(p);
    for (int i = 0; i < 100000; i++)
    {
        cnt[ad->draw()]++;
    }

    for (int i = 0; i < 7; i++)
        cout << cnt[i] << endl;
*/
