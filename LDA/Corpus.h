#ifndef __LDAEXP_CORPUS__
#define __LDAEXP_CORPUS__

#include <string>
#include <vector>
#include <map>

struct Token
{
    int word_no;
    int topic;
};

struct Corpus
{
    int V;                                       // 出现的单词(word)数
    std::map<std::string, int> dictionary;       // word映射到编号，共V个元素，标号从0开始
    std::vector<std::string> word_list;          // 单词表，长度为 V
    int* ground_truth;                           // 原始语料库中的分类，从0开始的整数。长度为M。
    int M;                                       // 文档数
    int* N;                                      // 长度为M，每篇文档的token数
    int N_sum;                                   // 所有文档的总token数目
    int K;                                       // 话题总数
    Token** doc_token_list;                      // 为了方便实现，下两个数据结构的合并。
//  int** doc_token_list;                        // 每个文档中一个token序列，token是word编号。M行，每行长度为N[m]
//  int** doc_token_topic;                       // 每个token当前的话题标记
    int** doc_by_topic_cnt;                      // M * K 的矩阵，记录当前每个话题的token数目
    double** doc_by_topic_distribution;          // M * K 的矩阵，每个文档的的话题分布
    int** topic_by_word_cnt;                     // K * V 的矩阵，记录每个话题下每个单词的token数目
    double** topic_by_word_distribution;         // K * V 的矩阵，每个话题下各个单词的分布
    int* topic_token_sum;                        // 长度为K，每个话题下的token总数
    double* alpha;                               // 超参数alpha，doc-topic分布的Dirichlet分布的参数。长度为K。
    double* beta;                                // 超参数beta，topic-word分布的Dirichlet分布的参数。长度为V。
    double beta_sum;                             // beta各分量的和

    Corpus(int K);
    ~Corpus();
    void calcDistributions();                    // 使用话题计数信息计算分布结果
    void defaultRead(std::string dir, int accept);     // 读入文件目录下的文件。单词最短长度为accept。格式约定见实现部分。
                                                 // 读入后，每个数组都应得到了定义。
};

#endif