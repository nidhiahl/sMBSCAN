#include "data.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <fstream>


data::data()
{
    //ctor
}

data::~data()
{
    //dtor
}



/**************Getter methods*****************/


void data::setnumInstances(int numInstances){
    this->numInstances = numInstances;
}

void data::setnumAttributes(int numAttributes){
    this->numAttributes = numAttributes;
}

/**************Setter methods*****************/


int data::getnumInstances()const{
    return this->numInstances;
}

int data::getnumAttributes() const {
    return this->numAttributes;
}

int data::getDeletedPointIndeciesSize()const {
    return deletedPointIndecies.size();
}

int data::getCurrentBatchIndeciesSize() const {
    return currentBatchIndecies.size();
}

int data::getCurrentBatchIndecies(int indx) const {
    return currentBatchIndecies[indx];
}

vector<point*> data::getDataVector() const {
    //const vector<point*> & refDataVector = this->dataVector;
    return dataVector;
}


/*******takes complete dataset File as input************/

void data::Run(const string & dataFile)
{
	createDataVector(dataFile);
}


//REQUIED FOR DYNAMIC DATA PREPARATION
/*void data::Run(const string & dataFile,const string & deletionFile, const string & batchFile, string & refopreationType)
{
	createDataVector(dataFile);
	
	removeDeletedPoints(deletionFile);                
	readCurrentBatch(batchFile, refOperationType);
}*/


void data::createDataVector(const string & dataFile){
	ifstream inDataFile(dataFile.c_str(),ios::in|ios::binary) ;
	if(!inDataFile){
		cout<<"Can not open input data file: "<<dataFile<<endl;
		exit(0);
	}
	inDataFile>>numInstances>>numAttributes;
	//cout<<numInstances<<" "<<numAttributes<<endl;
	dataVector.resize(numInstances);
	for(int instance = 0; instance < numInstances; instance++){
		point *newPoint = new point();
		newPoint->attributes = new double[numAttributes];
		int id;
		inDataFile>>id;
		for(int attribute = 0; attribute < numAttributes; attribute++){
			inDataFile>>newPoint->attributes[attribute];
		}
		inDataFile>>newPoint->label;
		newPoint->isPresent = bool(1);
		dataVector[id] = newPoint;
	}
	inDataFile.close();
}


void data::removeDeletedPoints(const string & deletionFile){
    ifstream inDeletionFile(deletionFile.c_str(), ios::in|ios::binary);
    if(!inDeletionFile){
        cout<<"Can not open input deletion file "<<deletionFile<<endl;
        exit(0);
    }
    int id;
    while(inDeletionFile>>id){
        dataVector[id]->isPresent = bool(0);
        deletedPointIndecies.push_back(id);
    }
    inDeletionFile.close();
}

void data::readCurrentBatch(const string & batchFile, string & operationType){
    ifstream inBatchFile(batchFile.c_str(),ios::in|ios::binary) ;
    if(!inBatchFile){
        cout<<"Can not open input data file: "<<batchFile<<endl;
    exit(0);
    }
    int numUpdates;
    int id;
    inBatchFile>>operationType;
    if(operationType == "static"){
        //cout<<"run static version of the algorithm"<<endl;
        inBatchFile.close();
    }else if(operationType == "addition"){
        //append points to dataVector
        inBatchFile>>numUpdates;
        this->numInstances +=numUpdates;
        dataVector.resize(numInstances);
        //currentBatchIndecies.resize(numUpdates);
        while(inBatchFile>>id){
            point *newPoint = new point();
            newPoint->attributes = new double[numAttributes];
            for(int attribute = 0; attribute < numAttributes; attribute++){
                inBatchFile>>newPoint->attributes[attribute];
            }
            inBatchFile>>newPoint->label;
            newPoint->isPresent = bool(1);
            dataVector[id] = newPoint;
            currentBatchIndecies.push_back(id);
        }
        inBatchFile.close();
    }else if(operationType == "deletion"){
        //remove current batch of deleted points from dataVector
        inBatchFile>>numUpdates;
        //currentBatchIndecies.resize(numUpdates);
        while(inBatchFile>>id){
            dataVector[id]->isPresent = bool(0);
            currentBatchIndecies.push_back(id);
        }
        inBatchFile.close();
    }else{
        cout<<operationType<<" is not a valid operation defined"<<endl;
        exit(0);
    }

}

void data::appendCurrentBatchToExistingData(const string & refOperationType, const string & dataFile, const string & deletionFile){
    if(refOperationType == "addition"){
        ofstream outDataFile(dataFile.c_str(), ios::app|ios::binary);
        if(!outDataFile){
            cout<<"Can not open output data file: "<<dataFile<<endl;
            exit(0);
        }
        for(int i = 0; i < getCurrentBatchIndeciesSize(); i++){
            int id = getCurrentBatchIndecies(i);
            outDataFile<<id<<" ";
            for(int attr = 0; attr < getnumAttributes(); attr++){
                outDataFile<<dataVector[id]->attributes[attr]<<" ";
            }
            outDataFile<<endl;
        }
        outDataFile.close();
        outDataFile.open(dataFile.c_str(), ios::out|ios::binary);
        //outDataFile.seekp(0, ios::beg);
        outDataFile<<this->numInstances<<" "<<this->numAttributes<<endl;
        outDataFile.close();
    }
    else if(refOperationType == "deletion"){
        ofstream outDeletionFile(deletionFile.c_str(), ios::app|ios::binary);
        if(!outDeletionFile){
            cout<<"Can not open output file "<<deletionFile<<endl;
            exit(0);
        }
        for(int i = 0; i<getCurrentBatchIndeciesSize(); i++){
            outDeletionFile<<getCurrentBatchIndecies(i)<<endl;
        }
        outDeletionFile.close();
    }
}

vector<int> & data::getUpdateSample(int sampleSize) const {
	vector<int>* sample = new vector<int>;
	vector<int> &refSample = *sample;
	int cnt = 0;
	for(int i = 0; i < currentBatchIndecies.size(); i++){
		if(dataVector[i]->isPresent){
			(*sample).push_back(i);
			cnt++;
		}
	}
	std::random_shuffle((*sample).begin(),(*sample).end());
	(*sample).erase((*sample).begin()+sampleSize,(*sample).begin()+cnt);
	return refSample;
}


vector<int> & data::getSample(int sampleSize) const {
	//cout<<"inside data.h getSample"<<endl;
	vector<int>* sample = new vector<int>;
	//cout<<"new int vector sample is created"<<endl;
	vector<int> &refSample = *sample;
	//cout<<"new int vector ref sample is created"<<endl;
	
	int cnt = 0;
	//cout<<"numInstances " <<numInstances<<endl;
	for(int i = 0; i < numInstances; i++){
		//cout<<"i = "<<i<<endl;
		if(dataVector[i]->isPresent){
			//cout<<"i = "<<i<<endl;
			(*sample).push_back(i);
			cnt++;
		}
	}
	//cout<<"out of for loop"<<endl;
	std::random_shuffle((*sample).begin(),(*sample).end());
	(*sample).erase((*sample).begin()+sampleSize,(*sample).begin()+cnt);
	return refSample;
}
