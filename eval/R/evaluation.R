setwd("D:/shared/Benchmark/ALL/")
# setwd("C:/Users/heyma/Documents/Benchmark/csv")
setwd("D:/shared/Benchmark/backup/csv_202105050305")
setwd("C:/Users/user/Documents/GitHub/Skyrmion/eval/evaluation_202105070044")
setwd("D:/shared/Benchmark/backup/csv_202105070812")
setwd("C:/Users/user/Documents/GitHub/Skyrmion/eval/evaluation_202105050305")
setwd("D:/shared/Benchmark/backup/csv_202105070044")

# install.packages("stringr")
# library("stringr")

# result <- c()

for(setting in 1:16){
  combineData <- c()
  # print(setting)
  print(paste("*_setting", setting, "_nodeInfo.csv", sep = ""))
  for(data in list.files(pattern = paste("*_setting", setting, "_nodeInfo.csv", sep = "")) ){
    print(data)
    nodeInfo <- read.csv(data, header = T)
    s <- unlist(strsplit(data, "_"))
    combineData <- rbind(combineData, c(filename=data, workload = s[1], setting = setting, colSums(nodeInfo[,-1])) )
  }
  
  print(paste("setting", setting, "_evaluation.csv", sep = ""))
  write.csv(combineData, paste("setting", setting, "_evaluation.csv", sep = ""), row.names = F, quote = F)
}

evaluation <- c()
for(data in list.files(pattern = "*_evaluation.csv") ){
  evalInfo <- read.csv(data, header = T)
  dataSize <- rep(c(10, 100, 5, 50))
  
  evalInfo <- cbind(evalInfo, dataSize = dataSize)
  evalInfo <- cbind(evalInfo, Energy = evalInfo$shiftCounter * 6e11 + evalInfo$insertCounter * 1.1e13 + evalInfo$removeCounter * 2e12 + evalInfo$migrateCounter * 2e12)
  evalInfo <- cbind(evalInfo, Latency = evalInfo$shiftCounter * 0.88 + evalInfo$insertCounter * 0.19 + evalInfo$removeCounter * 0.15 + evalInfo$migrateCounter * 0.15)
  
  evaluation <- rbind(evaluation, evalInfo)
}
write.csv(evaluation, "evaluation.csv", row.names = F, quote = F)

evaluation[grep( "tpch100_setting\\d+_", evaluation$workload),]

# D <- evaluation[grep( "tpch100_setting\\d+_", evaluation$workload),]
# plot(1:nrow(D), D$shiftCounter, type = "h")
# barplot(D$shiftCounter, names.arg = str_extract(D$workload, "ing\\d+") )
