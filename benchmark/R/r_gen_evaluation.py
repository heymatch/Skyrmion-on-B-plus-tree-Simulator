# coding: utf-8
L = ['exponental', 'hotspot', 'sequential', 'uniform', 'zipfian', 'tpch', 'latest']
L2= ['5','10','50','100']
s1="df <- data.frame(as.list(colsums(nodeInfo[,-1])))"
for i in range(7):
    for j in range(4):
        for k in range(1,17):
            s='nodeInfo <- read.csv("'+L[i]+L2[j]+'_setting' + str(k) +'_nodeInfo.csv", header = T)'
            print(s)
            print(s1)
            s2='write.csv(df, "'+L[i]+L2[j]+'_setting' + str(k) +'_evaluation.csv", row.names = F, quote = F)'
            print(s2)
            print()
