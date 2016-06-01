#include "SortedValueRankPair.h"
#include <algorithm>
using namespace std;

SortedValueRankList::SortedValueRankList(int* original, int length)
{
    this->length = length;
    sorted_list = new value_rank[length];
    sorted_rank = new int[length];

    for (int i = 0; i < length; i++)
    {
        sorted_list[i].value = original[i];
        sorted_list[i].rank = i;
    }
    sort(sorted_list, sorted_list + length);
    for (int i = 0; i < length; i++)
    {
        sorted_rank[sorted_list[i].rank] = i;
    }
}

SortedValueRankList::~SortedValueRankList()
{
    delete[] sorted_list;
    delete[] sorted_rank;
}

value_rank& SortedValueRankList::get(int current_pos)
{
    return sorted_list[current_pos];
}

value_rank& SortedValueRankList::getOriginal(int original_rank)
{
    return sorted_list[sorted_rank[original_rank]];
}

void SortedValueRankList::swapAdjust(int change)
{
    int j = change;
    while (j < length - 1 && sorted_list[j + 1] < sorted_list[j])
    {
        value_rank temp = sorted_list[j + 1];
        sorted_list[j + 1] = sorted_list[j];
        sorted_list[j] = temp;

        sorted_rank[sorted_list[j + 1].rank] = j + 1;
        sorted_rank[sorted_list[j].rank] = j;
        j++;
    }

    while (j > 0 && sorted_list[j] < sorted_list[j - 1])
    {
        value_rank temp = sorted_list[j - 1];
        sorted_list[j - 1] = sorted_list[j];
        sorted_list[j] = temp;

        sorted_rank[sorted_list[j - 1].rank] = j - 1;
        sorted_rank[sorted_list[j].rank] = j;
        j--;
    }
}

void SortedValueRankList::increaseOriginal(int original_rank)
{
    sorted_list[sorted_rank[original_rank]].value++;
    swapAdjust(sorted_rank[original_rank]);
}

void SortedValueRankList::decreaseOriginal(int original_rank)
{
    sorted_list[sorted_rank[original_rank]].value--;
    swapAdjust(sorted_rank[original_rank]);
}


/*
// 冒泡排序，同时维护每个元素的位置。复杂度过高，改为定点交换。
// 由于每次数组的变动极小，所以这里冒泡的均摊复杂度（交换次数）小于log(n)
// 但这浪费的时间很长
void bubbleSort(value_rank* sorted_list, int* rank_list, int N)
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
*/


