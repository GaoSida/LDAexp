# coding:utf-8
# 从原始语料中提取出正文
# 同时重新对文章进行编号，并单独用一个文件记录原始的类别

import os

file_classes = ['atheism', 'crypt', 'guns', 'mac']
original_diretories = ['../original_text/c1_atheism/', '../original_text/c2_sci.crypt/', \
                   '../original_text/c3_talk.politics.guns/', '../original_text/c4_comp.sys.mac.hardware/']

ground_truth = []
total_count = 0

for i in range(len(file_classes)):
    for j in range(10000, 60000):
        original_path = original_diretories[i] + str(j)
        if os.path.exists(original_path):
            original_text = file(original_path)

            pure_text = file('../pure_text/' + str(total_count) + '.txt', 'w')
            ground_truth.append(i)
            total_count += 1

            flag = False      # 第一个空行之后是正文的开始
            for line in original_text:
                if flag == False:
                    if line == '\n':
                        flag = True
                else:
                    pure_text.write(line)

class_label = file('../pure_text/class_label.csv', 'w')
class_label.write('file_no,label,class\n')
for i in range(len(ground_truth)):
    class_label.write(str(i) + ',' + str(ground_truth[i]) + ',' + file_classes[ground_truth[i]] + '\n')















