#include <iostream>
#include <string>
#include <cstring>
#include "data.h"
#include "iforest.h"
#include "mbscan.h"
#include "smbscan.h"

using namespace std;

void writeToFile(const data &refDataObject, auto clusterId, string resultFileName)
    {   
    	std::ofstream fileWrite(resultFileName.c_str(),ios::out);
		if(!fileWrite){
		cout<<"Can not open output data file: "<<resultFileName<<endl;
		exit(0);
		}
    	fileWrite<<"id actual predicted\n";
    	for (auto y = 0; y <refDataObject.getnumInstances();y++)
    	{
        	fileWrite<<y<<" "<<refDataObject.dataVector[y]->label<<" "<<clusterId[y]+1<<"\n";
        
    	}
    	fileWrite.close();
    } 


int main(int argc, char* argv[])
{
	struct timespec dataPrep_start,dataPrep_end;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &dataPrep_start);	
	
	const string &inputDataFileName = argv[1];
	const string &datasetName = argv[4];
	
	data * dataObject = new data();
	const data &refDataObject = * dataObject;
	dataObject->Run(inputDataFileName);
	const int totalInstances = refDataObject.getnumInstances();
	
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &dataPrep_end);
	double dataPrepTime =  (((dataPrep_end.tv_sec - dataPrep_start.tv_sec) * 1e9)+(dataPrep_end.tv_nsec - dataPrep_start.tv_nsec))*1e-9;
	//cout << "Time taken for dataPrepration " << fixed << dataPrepTime<<"sec"<<endl;
     
	
	
	int numiTrees = 100;
	int sampleSize = min(256, totalInstances);
	
	struct timespec iForest_start,iForest_end;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &iForest_start);	
	
	iforest * iForestObject = new iforest(numiTrees);
	const iforest &refiForestObject = * iForestObject;
	iForestObject->Run(dataObject,sampleSize);

	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &iForest_end);
	double iForestTime =  (((iForest_end.tv_sec - iForest_start.tv_sec) * 1e9)+(iForest_end.tv_nsec - iForest_start.tv_nsec))*1e-9;
	//cout << "Time taken for iForest " << fixed << iForestTime<<"sec"<<endl;
    
	const float mu = atof(argv[2]);
	const int minPts = atoi(argv[3]);
	
	
//----------------------------------------------------mbsacn----------------------------------------------------//	
	/*cout<<"MBSCAN"<<endl;
	struct timespec mbscan_start,mbscan_end;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &mbscan_start);	
	
	mbscan *mbscanObject = new mbscan();
	mbscanObject->Run(dataObject,iForestObject,mu,minPts);
	
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &mbscan_end);
	double mbscanTime =  (((mbscan_end.tv_sec - mbscan_start.tv_sec) * 1e9)+(mbscan_end.tv_nsec - mbscan_start.tv_nsec))*1e-9;
	cout<< fixed << dataPrepTime+iForestTime+mbscanTime<<"sec"<<endl;
    
    writeToFile(refDataObject, mbscanObject->clusterId,datasetName+"_mbscanResult.csv");
	
	vector<vector<int>> neighborList1;
	neighborList1.resize(totalInstances);
	for(int point1 = 0; point1 < totalInstances; point1++)
	{
		for(int point2 = 0; point2 < point1; point2++)
		{
			if(mbscanObject->_massMatrix[point1][point2] <= mu)
			{
				neighborList1[point1].push_back(point2);
				neighborList1[point2].push_back(point1);
			}
		}
	}
		
*/
//----------------------------------------------------smbscan----------------------------------------------------//	
	cout<<"SMBSCAN"<<endl;
	struct timespec smbscan_start,smbscan_end;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &smbscan_start);	
	
	smbscan *smbscanObject = new smbscan();
	smbscanObject->smbscanInitialization(dataObject,iForestObject, mu, minPts, 0.01);
	smbscanObject->computeNeighborhood();
	smbscanObject->doClustering();
	
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &smbscan_end);
	double smbscanTime =  (((smbscan_end.tv_sec - smbscan_start.tv_sec) * 1e9)+(smbscan_end.tv_nsec - smbscan_start.tv_nsec))*1e-9;
	cout<< fixed << dataPrepTime+iForestTime+smbscanTime<<"sec"<<endl;
   
   	//writeToFile(refDataObject, smbscanObject->clusterId,datasetName+"_smbscanResult.csv");

//----------------------------------------------------smbscan----------------------------------------------------//	
	cout<<"SMBSCAN computeneighborhood in iforest"<<endl;
	struct timespec smbscan1_start,smbscan1_end;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &smbscan1_start);	
	
	smbscan *smbscanObject1 = new smbscan();
	smbscanObject1->smbscanInitialization(dataObject,iForestObject, mu, minPts, 0.01);
	smbscanObject1->_iforest->computeNeighborhood(smbscanObject1->_neighborsList, mu,minPts,0.01);
	smbscanObject1->doClustering();
	
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &smbscan1_end);
	double smbscan1Time =  (((smbscan1_end.tv_sec - smbscan1_start.tv_sec) * 1e9)+(smbscan1_end.tv_nsec - smbscan1_start.tv_nsec))*1e-9;
	cout<< fixed << dataPrepTime+iForestTime+smbscan1Time<<"sec"<<endl;
   
   	//writeToFile(refDataObject, smbscanObject->clusterId,datasetName+"_smbscanResult.csv");



//----------------------------------------------------smbsacn_LCA---------------------------------------------------//	
	/*cout<<"SMBSCAN_LCA"<<endl;
	struct timespec smbscanLCA_start,smbscanLCA_end;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &smbscanLCA_start);	
	
	smbscan *smbscanLCAObject = new smbscan();
	smbscanLCAObject->smbscanInitialization(dataObject,iForestObject, mu, minPts,0.01);
	smbscanLCAObject->computeNeighborhoodUsingLCAlookup();
	smbscanLCAObject->doClustering();
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &smbscanLCA_end);
	double smbscanLCATime =  (((smbscanLCA_end.tv_sec - smbscanLCA_start.tv_sec) * 1e9)+(smbscanLCA_end.tv_nsec - smbscanLCA_start.tv_nsec))*1e-9;
	cout<< fixed << dataPrepTime+iForestTime+smbscanLCATime<<"sec"<<endl;
     
    writeToFile(refDataObject, smbscanLCAObject->clusterId,datasetName+"_smbscanLCAResult.csv");
	

//----------------------------------------------------smbsacn_LCA_runAvg---------------------------------------------------//        
	cout<<"SMBSCAN_LCA_runAvg"<<endl;
	struct timespec smbscanLCA_runAvg_start,smbscanLCA_runAvg_end;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &smbscanLCA_runAvg_start);	
	
	smbscan *smbscanLCA_runAvg_Object = new smbscan();
	smbscanLCA_runAvg_Object->smbscanInitialization(dataObject,iForestObject, mu, minPts,0.01);
	smbscanLCA_runAvg_Object->computeNeighborhoodUsingLCAlookup_runningAvg();
	smbscanLCA_runAvg_Object->doClustering();
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &smbscanLCA_runAvg_end);
	double smbscanLCA_runAvgTime =  (((smbscanLCA_runAvg_end.tv_sec - smbscanLCA_runAvg_start.tv_sec) * 1e9)+(smbscanLCA_runAvg_end.tv_nsec - smbscanLCA_runAvg_start.tv_nsec))*1e-9;
	cout << fixed << dataPrepTime+iForestTime+smbscanLCA_runAvgTime<<"sec"<<endl;
    
    writeToFile(refDataObject, smbscanLCA_runAvg_Object->clusterId,datasetName+"_smbscanLCA_runAvgResult.csv");
	
//----------------------------------------------------smbsacn_LCA_25%---------------------------------------------------// 
	cout<<"SMBSCAN_LCA_25"<<endl;
	struct timespec smbscanLCAObject25_start,smbscanLCAObject25_end;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &smbscanLCAObject25_start);	
	
	smbscan *smbscanLCAObject25 = new smbscan();
	smbscanLCAObject25->smbscanInitialization(dataObject,iForestObject, mu, minPts,0.25);
	smbscanLCAObject25->computeNeighborhoodUsingLCAlookup();
	smbscanLCAObject25->doClustering();
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &smbscanLCAObject25_end);
	double smbscanLCAObject25Time =  (((smbscanLCAObject25_end.tv_sec - smbscanLCAObject25_start.tv_sec) * 1e9)+(smbscanLCAObject25_end.tv_nsec - smbscanLCAObject25_start.tv_nsec))*1e-9;
	cout << fixed << dataPrepTime+iForestTime+smbscanLCAObject25Time<<"sec"<<endl;
    
    writeToFile(refDataObject, smbscanLCAObject25->clusterId,datasetName+"_smbscanLCA25Result.csv");
	
//----------------------------------------------------smbsacn_LCA_runAvg_25----------------------------------------------------//
	cout<<"SMBSCAN_LCA_runAvg_25"<<endl;
	struct timespec smbscanLCA_runAvg_25_start,smbscanLCA_runAvg_25_end;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &smbscanLCA_runAvg_25_start);	
	
	smbscan *smbscanLCA_runAvg_Object25 = new smbscan();
	smbscanLCA_runAvg_Object25->smbscanInitialization(dataObject,iForestObject, mu, minPts,0.25);
	smbscanLCA_runAvg_Object25->computeNeighborhoodUsingLCAlookup_runningAvg();
	smbscanLCA_runAvg_Object25->doClustering();
	
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &smbscanLCA_runAvg_25_end);
	double smbscanLCA_runAvg_25Time =  (((smbscanLCA_runAvg_25_end.tv_sec - smbscanLCA_runAvg_25_start.tv_sec) * 1e9)+(smbscanLCA_runAvg_25_end.tv_nsec - smbscanLCA_runAvg_25_start.tv_nsec))*1e-9;
	cout << fixed << dataPrepTime+iForestTime+smbscanLCA_runAvg_25Time<<"sec"<<endl;
    
    
    writeToFile(refDataObject, smbscanLCA_runAvg_Object->clusterId,datasetName+"_smbscanLCA_runAvg_25Result.csv");
	
	
//----------------------------------------------------smbsacn_LCA_50----------------------------------------------------//
	cout<<"SMBSCAN_LCA_50"<<endl;
	struct timespec smbscanLCAObject50_start,smbscanLCAObject50_end;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &smbscanLCAObject50_start);	
	
	smbscan *smbscanLCAObject50 = new smbscan();
	smbscanLCAObject50->smbscanInitialization(dataObject,iForestObject, mu, minPts,0.5);
	smbscanLCAObject50->computeNeighborhoodUsingLCAlookup();
	smbscanLCAObject50->doClustering();
	
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &smbscanLCAObject50_end);
	double smbscanLCAObject50Time =  (((smbscanLCAObject50_end.tv_sec - smbscanLCAObject50_start.tv_sec) * 1e9)+(smbscanLCAObject50_end.tv_nsec - smbscanLCAObject50_start.tv_nsec))*1e-9;
	cout <<fixed << dataPrepTime+iForestTime+smbscanLCAObject50Time<<"sec"<<endl;
    
    writeToFile(refDataObject, smbscanLCAObject50->clusterId,datasetName+"_smbscanLCA50Result.csv");
	
	
 //----------------------------------------------------smbsacn_LCA_runAvg_50----------------------------------------------------//   
    cout<<"SMBSCAN_LCA_runAvg_50"<<endl;
    struct timespec smbscanLCA_runAvg_50_start,smbscanLCA_runAvg_50_end;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &smbscanLCA_runAvg_50_start);	
	
	smbscan *smbscanLCA_runAvg_Object50 = new smbscan();
	smbscanLCA_runAvg_Object50->smbscanInitialization(dataObject,iForestObject, mu, minPts,0.5);
	smbscanLCA_runAvg_Object50->computeNeighborhoodUsingLCAlookup_runningAvg();
	smbscanLCA_runAvg_Object50->doClustering();
	
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &smbscanLCA_runAvg_50_end);
	double smbscanLCA_runAvg_50Time =  (((smbscanLCA_runAvg_50_end.tv_sec - smbscanLCA_runAvg_50_start.tv_sec) * 1e9)+(smbscanLCA_runAvg_50_end.tv_nsec - smbscanLCA_runAvg_50_start.tv_nsec))*1e-9;
	cout << fixed << dataPrepTime+iForestTime+smbscanLCA_runAvg_50Time<<"sec"<<endl;
    
    writeToFile(refDataObject, smbscanLCA_runAvg_Object->clusterId,datasetName+"_smbscanLCA_runAvg_50Result.csv");
	
	
//----------------------------------------------------smbsacn_LCA_65----------------------------------------------------//    
	cout<<"SMBSCAN_LCA_65"<<endl;
	struct timespec smbscanLCAObject65_start,smbscanLCAObject65_end;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &smbscanLCAObject65_start);	
	
	smbscan *smbscanLCAObject65 = new smbscan();
	smbscanLCAObject65->smbscanInitialization(dataObject,iForestObject, mu, minPts,0.65);
	smbscanLCAObject65->computeNeighborhoodUsingLCAlookup();
	smbscanLCAObject65->doClustering();
	
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &smbscanLCAObject65_end);
	double smbscanLCAObject65Time =  (((smbscanLCAObject65_end.tv_sec - smbscanLCAObject65_start.tv_sec) * 1e9)+(smbscanLCAObject65_end.tv_nsec - smbscanLCAObject65_start.tv_nsec))*1e-9;
	cout << fixed << dataPrepTime+iForestTime+smbscanLCAObject65Time<<"sec"<<endl;
    
    writeToFile(refDataObject, smbscanLCAObject65->clusterId,datasetName+"_smbscanLCA65Result.csv");
	
	
//----------------------------------------------------smbsacn_LCA_runAvg_65----------------------------------------------------//
	cout<<"SMBSCAN_LCA_runAvg_65"<<endl;
	struct timespec smbscanLCA_runAvg_65_start,smbscanLCA_runAvg_65_end;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &smbscanLCA_runAvg_65_start);	
	
	smbscan *smbscanLCA_runAvg_Object65 = new smbscan();
	smbscanLCA_runAvg_Object65->smbscanInitialization(dataObject,iForestObject, mu, minPts,0.65);
	smbscanLCA_runAvg_Object65->computeNeighborhoodUsingLCAlookup_runningAvg();
	smbscanLCA_runAvg_Object65->doClustering();
	
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &smbscanLCA_runAvg_65_end);
	double smbscanLCA_runAvg_65Time =  (((smbscanLCA_runAvg_65_end.tv_sec - smbscanLCA_runAvg_65_start.tv_sec) * 1e9)+(smbscanLCA_runAvg_65_end.tv_nsec - smbscanLCA_runAvg_65_start.tv_nsec))*1e-9;
	cout << fixed << dataPrepTime+iForestTime+smbscanLCA_runAvg_65Time<<"sec"<<endl;
    
    writeToFile(refDataObject, smbscanLCA_runAvg_Object->clusterId,datasetName+"_smbscanLCA_runAvg_65Result.csv");
	
	
//----------------------------------------------------smbsacn_LCA_75----------------------------------------------------//
	cout<<"SMBSCAN_LCA_75"<<endl;
	struct timespec smbscanLCAObject75_start,smbscanLCAObject75_end;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &smbscanLCAObject75_start);	
	
	smbscan *smbscanLCAObject75 = new smbscan();
	smbscanLCAObject75->smbscanInitialization(dataObject,iForestObject, mu, minPts,0.75);
	smbscanLCAObject75->computeNeighborhoodUsingLCAlookup();
	smbscanLCAObject75->doClustering();
	
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &smbscanLCAObject75_end);
	double smbscanLCAObject75Time =  (((smbscanLCAObject75_end.tv_sec - smbscanLCAObject75_start.tv_sec) * 1e9)+(smbscanLCAObject75_end.tv_nsec - smbscanLCAObject75_start.tv_nsec))*1e-9;
	cout << fixed << dataPrepTime+iForestTime+smbscanLCAObject75Time<<"sec"<<endl;
    
    writeToFile(refDataObject, smbscanLCAObject75->clusterId,datasetName+"_smbscanLCA75Result.csv");
	
	
//----------------------------------------------------smbsacn_LCA_runAvg_75----------------------------------------------------//	
	cout<<"SMBSCAN_LCA_runAvg_75"<<endl;
	struct timespec smbscanLCA_runAvg_75_start,smbscanLCA_runAvg_75_end;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &smbscanLCA_runAvg_75_start);	
	
	smbscan *smbscanLCA_runAvg_Object75 = new smbscan();
	smbscanLCA_runAvg_Object75->smbscanInitialization(dataObject,iForestObject, mu, minPts,0.75);
	smbscanLCA_runAvg_Object75->computeNeighborhoodUsingLCAlookup_runningAvg();
	smbscanLCA_runAvg_Object75->doClustering();
	
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &smbscanLCA_runAvg_75_end);
	double smbscanLCA_runAvg_75Time =  (((smbscanLCA_runAvg_75_end.tv_sec - smbscanLCA_runAvg_75_start.tv_sec) * 1e9)+(smbscanLCA_runAvg_75_end.tv_nsec - smbscanLCA_runAvg_75_start.tv_nsec))*1e-9;
	cout <<fixed << dataPrepTime+iForestTime+smbscanLCA_runAvg_75Time<<"sec"<<endl;
    
    
    writeToFile(refDataObject, smbscanLCA_runAvg_Object->clusterId,datasetName+"_smbscanLCA_runAvg_75Result.csv");
	
	
	
	
	/*for(int dataPoint = 0; dataPoint < totalInstances; dataPoint++)
	{	cout<<"\nCount of Neighbors for dataPoint "<<dataPoint<<"in originalMBSCN is "<<neighborList1[dataPoint].size()<<endl;
		for(auto x:neighborList1[dataPoint]){cout<<x<<" ";}
		cout<<"\nCount of Neighbors for dataPoint "<<dataPoint<<" in smbscan is "<<smbscanObject->_neighborsList[dataPoint].size()<<endl;
		for(auto x:smbscanObject->_neighborsList[dataPoint]){cout<<x<<" ";}

		cout<<"\nCount of Neighbors for dataPoint "<<dataPoint<<" in smbscanLCA is "<<smbscanLCAObject->_neighborsList[dataPoint].size()<<endl;
		for(auto x:smbscanLCAObject->_neighborsList[dataPoint]){cout<<x<<" ";}
		
		cout<<"\nCount of Neighbors for dataPoint "<<dataPoint<<" in smbscanLCA is "<<smbscanLCA_runAvg_Object->_neighborsList[dataPoint].size()<<endl;
		for(auto x:smbscanLCA_runAvg_Object->_neighborsList[dataPoint]){cout<<x<<" ";}
	
		cout<<"\nCount of Neighbors for dataPoint "<<dataPoint<<" in smbscanLCA is "<<smbscanLCAObject25->_neighborsList[dataPoint].size()<<endl;
		for(auto x:smbscanLCAObject25->_neighborsList[dataPoint]){cout<<x<<" ";}
		
		cout<<"\nCount of Neighbors for dataPoint "<<dataPoint<<" in smbscanLCA is "<<smbscanLCAObject50->_neighborsList[dataPoint].size()<<endl;
		for(auto x:smbscanLCAObject50->_neighborsList[dataPoint]){cout<<x<<" ";}
		
		cout<<"\nCount of Neighbors for dataPoint "<<dataPoint<<" in smbscanLCA is "<<smbscanLCAObject65->_neighborsList[dataPoint].size()<<endl;
		for(auto x:smbscanLCAObject65->_neighborsList[dataPoint]){cout<<x<<" ";}
		
		cout<<"\nCount of Neighbors for dataPoint "<<dataPoint<<" in smbscanLCA is "<<smbscanLCAObject75->_neighborsList[dataPoint].size()<<endl;
		for(auto x:smbscanLCAObject75->_neighborsList[dataPoint]){cout<<x<<" ";}
		
	}*/
	
	/*mbscan * mbscanObject = new mbscan();
	cout<<"original mbscan  object creation done"<<endl;
	vector<vector<double>> massMatrix;
	iForestObject->massMatrixComputation(massMatrix);
	cout<<"mass matrix computation done"<<endl;
	vector<vector<int>> neighborList1;
	neighborList1.resize(totalInstances);
	for(int point1 = 0; point1 < totalInstances; point1++)
	{
		for(int point2 = 0; point2 < point1; point2++)
		{
			if(massMatrix[point1][point2] <= mu)
			{
				neighborList1[point1].push_back(point2);
				neighborList1[point2].push_back(point1);
			}
		}
	}
	cout<<"NL for origiinal mbscan done"<<endl;
	
	
	//mbscanObject->Run(dataObject, iForestObject, mu, minPts);
	
	smbscan * smbscanObject = new smbscan();
	vector<vector<int>> neighborList2;
	neighborList2.resize(totalInstances);
	
	iForestObject->findNeighbors(dataObject,neighborList2,mu,minPts);
	
	
	for(int dataPoint = 0; dataPoint < totalInstances; dataPoint++)
	{	cout<<"\nCount of Neighbors for dataPoint "<<dataPoint<<"in originalMBSCN is "<<neighborList1[dataPoint].size()<<endl;
		for(auto x:neighborList1[dataPoint]){cout<<x<<" ";}
		cout<<"\nCount of Neighbors for dataPoint "<<dataPoint<<" in smbscan is "<<neighborList2[dataPoint].size()<<endl;
		for(auto x:neighborList2[dataPoint]){cout<<x<<" ";}
	}
	//smbscanObject->Run(dataObject, iForestObject, mu, minPts);
	*/
	return 0;
}
