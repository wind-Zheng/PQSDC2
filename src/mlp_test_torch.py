import torch
import torch.nn as nn
import numpy as np
import time
import sys

class Model(nn.Module):
    def __init__(self):
        super(Model, self).__init__()
        self.fc1 = nn.Linear(4, 16)
        self.fc2 = nn.Linear(16, 2)
        self.relu = nn.ReLU()

    def forward(self, x):
        out = self.fc1(x)
        out = self.relu(out)
        out = self.fc2(out)
        return out


model = Model()
model.load_state_dict(torch.load('model.ckpt'))

args = sys.argv
# 打印命令行参数
#print("命令行参数列表：", args)
Files = [args[1]]
#print("Files：", args[1])
# Files = ['SRR8386204.csv', 'SRR8386224.csv', 'SRR8386225.csv', 'ERR7091256_1.csv', 'ERR7091268_1.csv', 'SRR013951.csv',
#          'SRR027520.csv', 'SRR554369.csv', 'SRR17794741.csv', 'SRR17794724.csv', 'SRR12175235.csv']

for file in Files:
    with open(file, 'r') as f:
        lines = f.readlines()

    start_time = time.time()
    test_data = []
    test_targets = []

    for line in lines:
        line = line.strip().split(',')
        test_data.append([float(x) for x in line[:4]])
        test_targets.append(int(line[-1]))

    test_data = torch.from_numpy(np.array(test_data, dtype=np.float32))
    test_targets = torch.from_numpy(np.array(test_targets, dtype=np.int64))

    output = model(test_data)
    pred = torch.argmax(output, dim=1)

    acc = (pred == test_targets).float().mean()

    # 记录结束时间
    end_time = time.time()
    # 计算运行时间
    run_time = end_time - start_time
    # 打印运行时间
    print("程序运行时间为：", run_time, "秒")

    print(file,'的mlp预测正确率:', acc.item())
    print(pred)
    outFile=args[1]+".pre"
    with open(outFile, 'w') as file:
        line = ''.join(str(value.item()) for value in pred)
        file.write(line)

