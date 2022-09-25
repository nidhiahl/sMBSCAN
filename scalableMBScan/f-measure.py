# main.py
import sys
import pandas as pd
import numpy as np

def main():
	args = sys.argv[1:]
	if len(args) == 1:
		path_file_1 = args[0]
		#path_file_2 = args[1]
		#print(path_file_1)
	
	adata = pd.read_csv(path_file_1,sep=' ')
	
	'''pdata = pd.read_csv(path_file_2)
	
	adata = pd.read_csv("/home/nidhi/Dropbox/Work1.2/results/data actualSpiral.csv")
	pdata = pd.read_csv("/home/nidhi/Dropbox/Work1.2/results/data/predictedSpiral.csv")
'''
	actualClasses = adata['actual'].unique()
	#numClasses

	predictedClasses = adata['predicted'].unique()
	#numClusters
	
	#adata["predicted"] = pdata["class"]
	
	#adata
	
	#data.rename(columns = {'class': 'Actual class'}, inplace = True)
	
	#data["predicted class"] =1
	
	#data
	confusionMatrix = np.zeros(shape=(len(predictedClasses),len(actualClasses)), dtype=int)
	
	for index,rows in adata.iterrows():
	    #print(rows)
	    confusionMatrix[rows["predicted"]-1][rows["actual"]-1] = confusionMatrix[rows["predicted"]-1][rows["actual"]-1] +1 
	
	#confusionMatrix
	
	precision = [0.0]*len(predictedClasses)
	recall = [0.0]*len(predictedClasses)
	
	
	#arr = np.array(confusionMatrix)
	
	#arr.shape
	
	for i,rows in enumerate(confusionMatrix):
	    #print (rows)
	    precision[i]=(max(rows))/sum(rows)
	    colindex=np.argmax(rows)
	    colsum = sum(confusionMatrix[:,colindex])
	    recall[i] = max(rows)/colsum
	
	#precision, recall

	f_measure = [0.0]*len(predictedClasses)
	
	for i in range(len(predictedClasses)):
	    f_measure[i] = (2*precision[i]*recall[i])/(precision[i]+recall[i])
	
	print(sum(f_measure)/len(f_measure))

	
	
	
	
	
if __name__ == "__main__":
    main()
