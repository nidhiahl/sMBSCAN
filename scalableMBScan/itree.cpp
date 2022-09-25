#include "itree.h"
#include <math.h>

itree::itree()
{
    //ctor
}

itree::~itree()
{
    //dtor
}

void itree::createStaticTree(const data & dataObject, int sampleSize){
    maxTreeHeight = (int)log2(sampleSize);
    maxNumOfNodes = (int)pow(2.0,(double)(maxTreeHeight+1))-1;
    treeNode.resize(maxNumOfNodes);
    for(int nodeId = 0; nodeId < maxNumOfNodes; nodeId++){
    	//cout<<"inside static tree create"<<endl;
        treeNode[nodeId] = nullptr;
    }
    vector<int> &sample = dataObject.getSample(sampleSize);
    //cout<<"inside static tree create"<<endl;
    for(int nodeId = 0; nodeId < maxNumOfNodes; nodeId++){
        //cout<<"for nodeid"<<nodeId<<endl;
        if(nodeId == 0){
            treeNode[nodeId] = new treenode(nodeId);
            treeNode[nodeId]->dataPointIndex.resize(sampleSize);
            for(int in = 0; in < sampleSize; in++){
                //cout<<"for each point in node0"<<endl;
                treeNode[nodeId]->dataPointIndex[in] = sample.at(in);
            }
            if(treeNode[nodeId]->dataPointIndex.size() == 0){
            	treeNode[nodeId] = nullptr;
            	treeNode[nodeId]->parentAdd = nullptr;
            }
            else{
            	treeNode[nodeId]->parentAdd = treeNode[nodeId];
            	treeNode[nodeId]->parentId = nodeId;
            }
            //cout<<"node 0 initializaiotn done"<<endl;
        }
        if(treeNode[nodeId] == nullptr){continue;}

        treeNode[nodeId]->nodeHeight = (int)(log2((double)(nodeId+1)));
		//cout<<"nodeheight nodeUD= "<<nodeId<<endl;
        if(treeNode[nodeId]->dataPointIndex.size() <= 1 || treeNode[nodeId]->nodeHeight == maxTreeHeight){
        	//cout<<"leaf node"<<nodeId<<"---";
        	treeNode[nodeId]->isLeaf = bool(1);
        	treeNode[nodeId]->isActive = bool(1);
            continue;
        }//cout<<"if node is not leaf"<<endl;
        treeNode[nodeId]->splitInfoSelection((dataObject));
        //cout<<"split info selection"<<endl;
        treeNode[nodeId]->splitValue = treeNode[nodeId]->splitInfoSelection((dataObject));
        //cout<<nodeId<<"node: splitValue: "<<treeNode[nodeId]->splitValue<<" splitAttribute: "<<treeNode[nodeId]->splitAttribute<<endl;
        treenode *left = new treenode(2*nodeId+1);
        treenode *right = new treenode(2*nodeId+2);
	
        for(int i=0; i<treeNode[nodeId]->dataPointIndex.size(); i++){
            if(dataObject.dataVector[treeNode[nodeId]->dataPointIndex[i]]->attributes[treeNode[nodeId]->splitAttribute]<treeNode[nodeId]->splitValue){
                left->dataPointIndex.push_back(treeNode[nodeId]->dataPointIndex[i]);
            }
            else{
                right->dataPointIndex.push_back(treeNode[nodeId]->dataPointIndex[i]);
            }

        }
     	left->parentAdd = treeNode[nodeId];
     	left->parentId = nodeId;
      	treeNode[nodeId]->lChildAdd = left;
      	treeNode[nodeId]->lChildId = 2*nodeId+1;
      	treeNode[2*nodeId+1] = left;

      	right->parentAdd = treeNode[nodeId];
      	right->parentId = nodeId;
      	treeNode[nodeId]->rChildAdd = right;
      	treeNode[nodeId]->rChildId = 2*nodeId+2;
      	treeNode[2*nodeId+2] = right;

        treeNode[nodeId]->isActive = bool(1);
        //cout<<"ActiveNode"<<nodeId<<"--";
    }
}



void itree::computeNodeMassforTree(const data & dataObject){
    smallestLeafSize = dataObject.getnumInstances(), largestLeafSize=0;
    int numOfPointsPresent = 0;
    pointToNode.resize(dataObject.getnumInstances());
    for(int i = 0; i < dataObject.getnumInstances(); i++){
        if(dataObject.dataVector[i]->isPresent){
            //pointToNode[i].isPresent = bool(1);
            numOfPointsPresent++;
        }
        pointToNode[i] = -1;
    }

    for(int nodeId = 0; nodeId < maxNumOfNodes; nodeId++){
    	if(treeNode[nodeId] == nullptr){continue;}
        treeNode[nodeId]->dataPointIndex.resize(0);
    }

    for(int nodeId = 0; nodeId < maxNumOfNodes; nodeId++){
        if(treeNode[nodeId] == nullptr){continue;}
        if(nodeId == 0){
            //treeNode[nodeId]->dataPointIndex.resize(numOfPointsPresent);
            for(int in = 0; in < dataObject.getnumInstances(); in++){
                if(dataObject.dataVector[in]->isPresent){
                    treeNode[nodeId]->dataPointIndex.push_back(in);
                }
            }
        }
        treeNode[nodeId]->nodeMass = treeNode[nodeId]->dataPointIndex.size();
        //cout<<"nodeMass of node"<<nodeId<<" is"<<treeNode[nodeId]->nodeMass<<endl;
        if(treeNode[nodeId]->isLeaf){
        //cout<<"\nleafnode"<<nodeId<<"----";
            for(int in = 0; in < treeNode[nodeId]->dataPointIndex.size(); in++){
                //cout<<"leafnode"<<nodeId<<"----";
                //cout<<treeNode[nodeId]->dataPointIndex[in]<<" ";
                pointToNode[treeNode[nodeId]->dataPointIndex[in]] = nodeId;
            }
            //if(treeNode[nodeId]->nodeMass == 0){cout<<"zero leafnode mass"<<endl;}
            if(smallestLeafSize >= treeNode[nodeId]->nodeMass && treeNode[nodeId]->nodeMass >= 1){smallestLeafSize = treeNode[nodeId]->nodeMass;}
            if(largestLeafSize <= treeNode[nodeId]->nodeMass){largestLeafSize = treeNode[nodeId]->nodeMass;}
            continue;
        }

	//cout<<"\nActive Nonleaf nodeId"<<nodeId<<"---";
        for(int i=0; i<treeNode[nodeId]->dataPointIndex.size(); i++){
		//cout<<treeNode[nodeId]->dataPointIndex[i]<<" ";
            if(dataObject.dataVector[treeNode[nodeId]->dataPointIndex[i]]->attributes[treeNode[nodeId]->splitAttribute] < treeNode[nodeId]->splitValue){
	
            	 if(treeNode[2*nodeId + 1] == nullptr){
            	 	treeNode[2*nodeId + 1] = new treenode(2*nodeId + 1);
            	 	treeNode[2*nodeId + 1]->isLeaf = bool(1);
            	 }
                treeNode[2*nodeId + 1]->dataPointIndex.push_back(treeNode[nodeId]->dataPointIndex[i]);
            }
            else{
                if(treeNode[2*nodeId + 2] == nullptr){
                	treeNode[2*nodeId + 2] = new treenode(2*nodeId + 1);
                	treeNode[2*nodeId + 2]->isLeaf = bool(1);
                }
                treeNode[2*nodeId + 2]->dataPointIndex.push_back(treeNode[nodeId]->dataPointIndex[i]);
            }
        }
    }
    
}


int itree::getMaxNumOfNodes(){
    return maxNumOfNodes;
}




