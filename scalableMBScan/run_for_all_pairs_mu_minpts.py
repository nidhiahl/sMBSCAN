#python3 run_for_all_pairs_mu_minpts.py /home/nidhi/Dropbox/Work1.2/inputDataFiles/[INPUTFILENAME] 
import sys, os

def main():
	dataset = str(sys.argv[1])
	#os.system("g++ -o minPtsMuRunOforiginalMBSCAN mbscanClustering.cpp data.cpp iforest.cpp itree.cpp treenode.cpp -lboost_serialization")

	mu = [0.1,0.15,0.2,0.25,0.3,0.35,0.4,0.45,0.5,0.55,0.6,0.65,0.7,0.75,0.8,0.85,0.9,0.95,1]
	minPts = [1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30]
	#print("minPts 0.1 0.15 0.2 0.25 0.3 0.35 0.4 0.45 0.5 0.55 0.6 0.65 0.7 0.75 0.8 0.85 0.9 0.95 1\n")
	for minP in minPts:
		#print(str(minP)+" ")
		for mmu in mu:
			os.system("./minPtsMuRunOforiginalMBSCAN "+dataset+" "+str(mmu)+" "+str(minP)+" Clusteringresult.csv")
			#print("clustering done")
			os.system("python3 f-measure.py Clusteringresult.csv")
			#print(" ")
		#print('\n')

	
if __name__ == "__main__":
    main()



















