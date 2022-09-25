#python3 run_for_all_pairs_mu_minpts.py /home/nidhi/Dropbox/Work1.2/inputDataFiles/[INPUTFILENAME] 
import sys, os

datasets = ['iris.csv', 'wine.csv', 'seeds.csv', 'spiral.csv', 'movementLibra.csv', 'wdbc.csv', 'segment.csv', 'D31.csv', 'thyorid.csv']

os.system("g++ -o minPtsMuRunOforiginalMBSCAN mbscanClustering.cpp data.cpp iforest.cpp itree.cpp treenode.cpp -lboost_serialization")


for dataset in datasets:
	os.system("python3 run_for_all_pairs_mu_minpts.py"+" "+"inputDataFiles/"+dataset+" "+">allPairF-measure_"+dataset)
	print(dataset+" done")
	#os.system("python3 run_for_all_pairs_mu_minpts.py"+" "+"inputDataFiles/"+dataset)
	

