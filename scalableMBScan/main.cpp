# include <iostream>
# include <string>
# include "data.h"
# include "iforest.h"
# include <string>
# include <boost/archive/binary_oarchive.hpp>
# include <boost/archive/binary_iarchive.hpp>
# include <boost/archive/text_oarchive.hpp>
# include <boost/archive/text_iarchive.hpp>
# include <boost/serialization/vector.hpp>
# include <boost/serialization/map.hpp>

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
	const int &numOfTrees = atoi(argv[1]);
	const double &samplingFactor = atof(argv[2]);
	const int &minSampleSize = atoi(argv[3]);
	const string &dataFile = argv[4];
	const string &deletionFile = argv[5];
	const string &batchFile = argv[6];
	const string &computationHistoryFile = argv[7];
	const string &massFile = argv[8];
    string operationType = "static";
    string & refOperationType = operationType;
    struct timespec static_start,static_end;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &static_start);
    data *dataObject = new data();
    const data &refDataObject = *dataObject;
    dataObject->createDataVector(dataFile);
    //cout<<"data vector created"<<endl;
    dataObject->removeDeletedPoints(deletionFile);
    //cout<<"older deleted points remoevd"<<endl;
    dataObject->readCurrentBatch(batchFile, refOperationType);
    //cout<<"current batch read"<<endl;
    iforest *iForestObject = new iforest(numOfTrees);
    int sampleSize;
    //cout<<"before static"<<endl;
    if(refOperationType == "static"){
        /*struct timespec static_start,static_end;
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &static_start);*/
        //cout<<"before static"<<endl;
        int countOfCurrentPoints = (dataObject->getnumInstances())-(dataObject->getDeletedPointIndeciesSize());
        sampleSize = countOfCurrentPoints * samplingFactor < minSampleSize ? minSampleSize :countOfCurrentPoints * samplingFactor;
        sampleSize = countOfCurrentPoints < sampleSize ? countOfCurrentPoints : sampleSize;

        struct timespec start_forest,end_forest;
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_forest);
        iForestObject->createStaticForest(refDataObject, sampleSize);
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_forest);
        double forestTime =  (((end_forest.tv_sec - start_forest.tv_sec) * 1e9)+(end_forest.tv_nsec - start_forest.tv_nsec))*1e-9;
        cout << "Time taken to create forest: " << fixed << forestTime<<"sec"<<endl;


        struct timespec start_nodeMass,end_nodeMass;
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_nodeMass);
        iForestObject->computeNodeMass(refDataObject);
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_nodeMass);
        double nodeMassTime =  (((end_nodeMass.tv_sec - start_nodeMass.tv_sec) * 1e9)+(end_nodeMass.tv_nsec - start_nodeMass.tv_nsec))*1e-9;
        cout << "Time taken to compute nodeMass: " << fixed << nodeMassTime<<"sec"<<endl;

        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &static_end);
        ramUsed = max(getValue(1),ramUsed);
        vMemUsed = max(getValue(2),vMemUsed);
        cout<< "ram used: "<<getValue(1)<<endl;

        //cout<<"nodemasss computed"<<endl;
        //iForestObject->massMatrixComputation(massFile);
        iForestObject->writeComputationHistory(computationHistoryFile);
        double staticTime =  (((static_end.tv_sec - static_start.tv_sec) * 1e9)+(static_end.tv_nsec - static_start.tv_nsec))*1e-9;
        cout << "Time taken by static algorithm " << fixed << staticTime<<"sec"<<endl;
        //cout<<"static done"<<endl;
        //iForestObject.delete();

    }
    else{
        struct timespec start_readHistroy,end_readHistory;
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_readHistroy);
        iForestObject->readComputationHistory(computationHistoryFile);
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_readHistory);
        double readHistoryTime =  (((end_readHistory.tv_sec - start_readHistroy.tv_sec) * 1e9)+(end_readHistory.tv_nsec - start_readHistroy.tv_nsec))*1e-9;
        cout << "Time taken to read computation history: " << fixed << readHistoryTime<<"sec"<<endl;

        struct timespec start_createIncrementalForest,end_createIncrementalForest;
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_createIncrementalForest);
        int countOfUpdatedPoints = dataObject->getCurrentBatchIndeciesSize();
        sampleSize = countOfUpdatedPoints * samplingFactor < minSampleSize ? minSampleSize :countOfUpdatedPoints * samplingFactor;
        sampleSize = countOfUpdatedPoints < sampleSize ? countOfUpdatedPoints : sampleSize;
        iForestObject->createIncrementalForest(refDataObject, sampleSize, refOperationType);
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_createIncrementalForest);
        double createIncrementalForestTime =  (((end_createIncrementalForest.tv_sec - start_createIncrementalForest.tv_sec) * 1e9)+(end_createIncrementalForest.tv_nsec - start_createIncrementalForest.tv_nsec))*1e-9;
        cout << "Time taken to create incremental forest: " << fixed << createIncrementalForestTime<<"sec"<<endl;

        struct timespec start_computeIncrementalNodeMass,end_computeIncrementalNodeMass;
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_computeIncrementalNodeMass);
        iForestObject->computeIncrementalNodeMass(refDataObject, refOperationType);
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_computeIncrementalNodeMass);
        double computeIncrementalNodeMassTime =  (((end_computeIncrementalNodeMass.tv_sec - start_computeIncrementalNodeMass.tv_sec) * 1e9)+(end_computeIncrementalNodeMass.tv_nsec - start_computeIncrementalNodeMass.tv_nsec))*1e-9;
        cout << "Time taken to compute incremental node mass: " << fixed << computeIncrementalNodeMassTime<<"sec"<<endl;

        struct timespec start_writeHistory,end_writeHistory;
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_writeHistory);
        iForestObject->writeComputationHistory(computationHistoryFile);
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_writeHistory);
        double writeHistoryTime =  (((end_writeHistory.tv_sec - start_writeHistory.tv_sec) * 1e9)+(end_writeHistory.tv_nsec - start_writeHistory.tv_nsec))*1e-9;
        cout << "Time taken to write computation history: " << fixed << writeHistoryTime<<"sec"<<endl;

        struct timespec start_updateDataFiles,end_updateDataFiles;
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_updateDataFiles);
        dataObject->appendCurrentBatchToExistingData(refOperationType, dataFile, deletionFile);
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_updateDataFiles);
        double updateDataFilesTime =  (((end_updateDataFiles.tv_sec - start_updateDataFiles.tv_sec) * 1e9)+(end_updateDataFiles.tv_nsec - start_updateDataFiles.tv_nsec))*1e-9;
        cout << "Time taken to update previous datafiles: " << fixed << updateDataFilesTime<<"sec"<<endl;
    }


    return 0;
}
