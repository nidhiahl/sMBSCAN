#include "treenode.h"

treenode::treenode()
{
    splitAttribute = -1;
    isLeaf = bool(0);
    isActive = bool(0);
    //deletionHappened = bool(0);
    //deletionProcessed = bool(0);
    //nodeVanishes = bool(0);
    nodeId = -1;
    parentId = -1;
    lChildId = -1;
    rChildId = -1;
    nodeMass = 0;
    dataPointIndex.resize(0);
    //deletedPointIndex.resize(0);
    //addedPointIndex.resize(0);
    parentAdd = nullptr;
    lChildAdd = nullptr;
    rChildAdd = nullptr;
}

treenode::treenode(int nodeId)
{
    splitAttribute = -1;
    isLeaf = bool(0);
    isActive = bool(0);
    //deletionHappened = bool(0);
    //deletionProcessed = bool(0);
    //nodeVanishes = bool(0);
    //nodeId = -1;
    parentId = -1;
    lChildId = -1;
    rChildId = -1;
    nodeMass = 0;
    dataPointIndex.resize(0);
    //deletedPointIndex.resize(0);
    //addedPointIndex.resize(0);
    parentAdd = nullptr;
    lChildAdd = nullptr;
    rChildAdd = nullptr;
    this->nodeId = nodeId;
}

treenode::~treenode()
{
    //dtor
}


double treenode::splitInfoSelection(const data &dataObject){
	int maxVal = -999999;
	int minVal = 999999;
	int attempts = 0;
	//cout<<"inside split info selection"<<endl;
	while(attempts < 10){
		splitAttribute = rand()%dataObject.getnumAttributes();
		//cout<<"splitAttr"<<dataPointIndex.size()<<endl;
		for( int i = 0; i < dataPointIndex.size(); i++){
        		if(maxVal < dataObject.dataVector[(dataPointIndex[i])]->attributes[splitAttribute]){
				maxVal = dataObject.dataVector[(dataPointIndex[i])]->attributes[splitAttribute];
			}
        		if(minVal > dataObject.dataVector[(dataPointIndex[i])]->attributes[splitAttribute]){
        			minVal = dataObject.dataVector[(dataPointIndex[i])]->attributes[splitAttribute];
        		}
        }
        attempts = attempts + 1;
        double dataDiff = maxVal - minVal;
		if(dataDiff >= 0.0000000000000001){
			attempts = 10;
		}
	}
	//cout<<"diiffffeeerreeennncccee = "<<maxVal-minVal<<endl;
	maximumVal = maxVal;
	minimumVal = minVal;
	return (minVal + ((double)rand()/RAND_MAX)*(maxVal-minVal));
}


