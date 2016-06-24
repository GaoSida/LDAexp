使用C++实现，LDA目录下是主体代码，其余文件夹是实验使用的小数据集和预处理结果（去除停用词、词干化）。代码的大致结构为，Corpus类是对数据集涉及的参数（比如当前的topic标记和各种计数）的封装；LDAFramework是LDA的框架实现，其中可变的部分为sample的方法，判断迭代结束的条件和中间输出的信息等等。OriginalSampler，FastSampler，SparseSampler，AliasSampler和LightSampler是原本的Gibbs Sampling方法，和四种加速方法。切换策略需要在main.cpp中修改一行代码，构造不同类的sampler。

实验使用的数据集是一个400篇文档的数据集，其中有4类（标记好的ground_truth），每类100篇。预处理之后，词典的大小为11,850；单词的总数为194,810。
算法的验证包括两部分，其一是正确性，即在Topic=4的参数下对文档进行聚类，观察Perplexity随着迭代次数的变化，和最终聚类结果对ground truth的符合程度。其二是速度，即在Topic=1000下，对10次迭代的时间进行计时。计时的结果为：

        > 原始【O(K)】：60~62s左右
        > FastLDA【O(K)】：13~15s左右（进过几十轮迭代后下降下来）
        > SparseLDA【O(Kd + Kw)】：3~4s左右
        > AliasLDA【O(Kd)】：3~4s左右 (2000 Topic 下，Sparse在5~6秒，Alias在4~5秒)
        > LightLDA【O(1)】：0~1s

可见，加速的效果是比较符合预期的。其中AliasLDA和SparseLDA没有拉开差距，是因为两者的渐进复杂度本来就差不多，实现上再引入一些常数的变化，导致目前的结果。

在这四种算法中，FastLDA和SparseLDA是精确算法（通过利用稀疏性和Caching一些需要重复计算的值加速），所以验证时我也直接和原本算法计算的概率做了比对，因此这两种算法的正确性是可以保证的。AliasLDA的Perplexity变化和聚类结果看起来也是正确的。现在的LightLDA还有一些问题，它的收敛非常缓慢。经过一些尝试，也看了一些其他来源的资料，每次sample多做了几个Metropolis Hasting Step，虽然收敛速度明显快了很多，但离精确的LDA还有一定距离。