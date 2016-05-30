# coding:utf-8
# 把标准化之后的单词进一步进行词干化

from nltk.stem.lancaster import LancasterStemmer

st = LancasterStemmer()

for i in range(400):
    lemmatized = file('../lemmatized_text/' + str(i) + '.txt')
    stemmed = file('../stemmed_text/' + str(i) + '.txt', 'w')

    for line in lemmatized:
        stemmed.write(st.stem(line[:-1]) + '\n')


