# setwd("D:/shared/Benchmark/ALL/csv")
setwd("C:/Users/heyma/Documents/Benchmark/csv")

combineData <- c()

for(data in list.files(pattern = "*nodeInfo.csv")){
  print(data)
  nodeInfo <- read.csv(data, header = T)
  combineData <- rbind(combineData, c(workload=data, colSums(nodeInfo[,-1])) )
}