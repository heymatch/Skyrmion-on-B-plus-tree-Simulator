# setwd("D:/shared/Benchmark/ALL/csv")
# setwd("C:/Users/heyma/Documents/Benchmark/csv")
setwd("D:/shared/Benchmark/backup/csv_202105050305")


# result <- c()

for(setting in 1:16){
  combineData <- c()
  # print(setting)
  print(paste("*_setting", setting, "_nodeInfo.csv", sep = ""))
  for(data in list.files(pattern = paste("*_setting", setting, "_nodeInfo.csv", sep = "")) ){
    print(data)
    nodeInfo <- read.csv(data, header = T)
    combineData <- rbind(combineData, c(workload=data, colSums(nodeInfo[,-1])) )
  }
  
  print(paste("setting", setting, "_evaluation.csv", sep = ""))
  write.csv(combineData, paste("setting", setting, "_evaluation.csv", sep = ""), row.names = F, quote = F)
}




evaluation <- c()
for(data in list.files(pattern = "*_evaluation.csv") ){
  evalInfo <- read.csv(data, header = T)
  evaluation <- rbind(evaluation, evalInfo)
}

D <- evaluation[grep( "_setting1_", evaluation$workload),]
# plot(1:nrow(D), D$shiftCounter, type = "h")
barplot(D$shiftCounter, D$workload)
