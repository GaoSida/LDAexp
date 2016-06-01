#ifndef __LDAEXP_SORTED_VALUE_RANK__
#define __LDAEXP_SORTED_VALUE_RANK__

// 值-秩结构体，其值可变，但这里的rank相当于变量名，不能变。
// 具体到本问题中，rank就是话题角标k。
struct value_rank
{
    int rank;
    double value;

    bool operator<(const value_rank &vr) const
    {
        return this->value < vr.value;
    }

    // 冒泡排序，同时维护每个元素的位置。
    // 由于每次数组的变动极小，所以这里冒泡的均摊复杂度（交换次数）小于log(n)
    // 但这浪费的时间很长
    static void bubbleSort(value_rank* sorted_list, int* rank_list, int N)
    {
        for (int i = 0; i < N; i++)
        {
            bool exchange_flag = false;
            for (int j = i; j < N - 1; j++)
            {
                if (sorted_list[j + 1] < sorted_list[j])
                {
                    value_rank temp = sorted_list[j + 1];
                    sorted_list[j + 1] = sorted_list[j];
                    sorted_list[j] = temp;

                    rank_list[sorted_list[j + 1].rank] = j + 1;
                    rank_list[sorted_list[j].rank] = j;

                    exchange_flag = true;
                }
            }
            if (!exchange_flag)
            {
                return;
            }
        }
    }

    static void swapAdjust(value_rank* sorted_list, int* rank_list, int N, int change)
    {
        int j = change;
        while (j < N - 1 && sorted_list[j + 1] < sorted_list[j])
        {
            value_rank temp = sorted_list[j + 1];
            sorted_list[j + 1] = sorted_list[j];
            sorted_list[j] = temp;

            rank_list[sorted_list[j + 1].rank] = j + 1;
            rank_list[sorted_list[j].rank] = j;
            j++;
        }

        while (j > 0 && sorted_list[j] < sorted_list[j - 1])
        {
            value_rank temp = sorted_list[j - 1];
            sorted_list[j - 1] = sorted_list[j];
            sorted_list[j] = temp;

            rank_list[sorted_list[j - 1].rank] = j - 1;
            rank_list[sorted_list[j].rank] = j;
            j--;
        }
    }
};

#endif