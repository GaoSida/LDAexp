#ifndef __LDAEXP_SORTED_VALUE_RANK__
#define __LDAEXP_SORTED_VALUE_RANK__

// 值-秩结构体，其值可变，但这里的rank相当于变量名，不能变。
// 具体到本问题中，rank就是话题角标k。
struct value_rank
{
    int rank;
    int value;

    bool operator<(const value_rank &vr) const
    {
        return this->value < vr.value;
    }
};

class SortedValueRankList
{
    int length;                     // 长度
    value_rank* sorted_list;        // 排序后的 值-原rank 对
    int* sorted_rank;               // 由原来的rank，查现在元素的实际位置
public:
    SortedValueRankList(int* original, int length);    // 传入一个原始数组复制过来
    ~SortedValueRankList();
    value_rank& get(int current_pos);                     // 寻排序后的位置取数
    value_rank& getOriginal(int original_rank);           // 按原来的rank取数
    void swapAdjust(int current_pos);                          // 因为改动了一个数而进行调整。传入实际的位置。
    void increaseOriginal(int original_rank);             // 对某个位置加1或减1，属于常用操作
    void decreaseOriginal(int original_rank);
};

#endif