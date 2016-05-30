# coding:utf-8
# 把纯文本中的非单词字符去掉，并进行词法标准化

import os
from nltk.corpus import stopwords
from nltk.stem.wordnet import WordNetLemmatizer

english_stopwords = stopwords.words('english')
lmtzr = WordNetLemmatizer()

for i in range(400):
    pure_text = file('../pure_text/' + str(i) + '.txt').read()
    # 转换成ascii字符
    stripped = (c for c in pure_text if 0 <= ord(c) <= 127)
    pure_text = ''.join(stripped)
    # 全换为小写
    pure_text = pure_text.lower()
    # 把非字母字符一律换成空格
    for ch in range(128):
        ch = chr(ch)
        if not (ch >= 'a' and ch <= 'z'):
            pure_text = pure_text.replace(ch, ' ')

    processed_text = file('../lemmatized_text/' + str(i) + '.txt', 'w')
    # 第一行写类号
    processed_text.write(str(i / 100) + '\n')
    # 之后每行一个词干化的单词
    words = pure_text.split()
    for word in words:
        if not (word in english_stopwords):
            processed_text.write(lmtzr.lemmatize(word) + '\n')
    




