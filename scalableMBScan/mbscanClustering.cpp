#include <iostream>
#include <string>
# include "data.h"
//# include "iforest.h"
#include "mbscan1.h"
#include <string>
#include <cstring>
/*#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
*/
using namespace std;

int parseLine(char *line){
    int i = strlen(line);
    const char *p = line;
    while(*p <'0' || *p >'9')
	p++;
    line[i-3] = '\0';
    i = atoi(p);
    return i;
}

int getValue(int par){
    FILE *file = fopen("/proc/self/status","r");
    int result = -1;
    char line[128];

    while(fgets(line,128,file) != NULL){
	if(par == 1){               //Ram used
   	    if(strncmp(line, "VmRSS:", 6) == 0){
		result = parseLine(line);
		break;
	    }
	}
	if(par == 2){               //virtual memory used
   	    if(strncmp(line, "VmSize:", 7) == 0){
		result = parseLine(line);
		break;
	    }
	}
    }
    fclose(file);
    return result;
}


int main(int argc, char* argv[])
{
	srand(time(0));
    int ramUsed = 0;
    int vMemUsed = 0;	
	
	struct timespec dataPrep_start,dataPrep_end;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &dataPrep_start);	
	
	const string &dataFile = argv[1];
	//cout<<"read file"<<endl;
	data * dataObject = new data();
	//cout<<"dataobject created"<<endl;
	dataObject->Run(dataFile);
	//cout<<"dataPrepared"<<endl;
	const data &refDataObject = *dataObject;
	//cout<<"dataobjectreference"<<endl;
	
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &dataPrep_end);
	double dataPrepTime =  (((dataPrep_end.tv_sec - dataPrep_start.tv_sec) * 1e9)+(dataPrep_end.tv_nsec - dataPrep_start.tv_nsec))*1e-9;
	cout << "Time taken for dataPrepration " << fixed << dataPrepTime<<"sec"<<endl;
        
	ramUsed = max(getValue(1),ramUsed);
	vMemUsed = max(getValue(2),vMemUsed);
	//cout<< "ram used: "<<getValue(1)<<endl;	
	
	
	/******************DataPreparation done*************************/
	
	struct timespec clustering_start,clustering_end;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &clustering_start);		
	const double muVal = atof(argv[2]);
	const int minPtsVal = atoi(argv[3]);
	mbscan * mbscanObject = new mbscan();
	//cout<<"mbscanobject created"<<endl;
	mbscanObject->Run(refDataObject, muVal, minPtsVal);
	//cout<<"mbscan executed"<<endl;

	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &clustering_end);
	double clusteringTime =  (((clustering_end.tv_sec - clustering_start.tv_sec) * 1e9)+(clustering_end.tv_nsec - clustering_start.tv_nsec))*1e-9;
	cout << "Time taken for clustreing " << fixed << clusteringTime<<"sec"<<endl;
        
	ramUsed = max(getValue(1),ramUsed);
	vMemUsed = max(getValue(2),vMemUsed);
	//cout<< "ram used: "<<getValue(1)<<endl;
	
	/******************Clustering done*************************/	
	
	const string &resultFile = argv[4];
	auto clusterId = mbscanObject->clusterId;
    std::ofstream fileprint;
    fileprint.open(resultFile);
    //cout<<"size of clusters"<<clusterId.size()<<endl;
    fileprint<<"id actual predicted\n";
    for (auto y = 0; y <refDataObject.getnumInstances();y++)
    {
        fileprint<<y<<" "<<refDataObject.dataVector[y]->label<<" "<<clusterId[y]+1<<"\n";
        
    }
	
	/******************Writing result in file done*************************/
	
	delete mbscanObject;
	delete dataObject;

return 0;

}

