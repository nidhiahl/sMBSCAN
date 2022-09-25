#include "iforest.h"
/*#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>*/
#include <math.h>
using namespace std;


iforest::iforest()
{
    //ctor
}

iforest::iforest(const int & numiTrees) 
{
    this->numiTrees = numiTrees;
}

iforest::~iforest()
{
    //dtor
}

void iforest::Run(data *dataObject, int sampleSize){
	this->_data = dataObject;
	const data &refdataObject = *dataObject;
	//cout<<"inside run of iforest"<<endl;
	createStaticForest(refdataObject, sampleSize);
	//cout<<"static iForest created"<<endl;
	computeNodeMass(refdataObject);
	//cout<<"nodemass compute"<<endl;
}

void iforest::createStaticForest(const data & dataObject, int sampleSize){
    iTrees.resize(numiTrees);
    //cout<<"----Forest creation----"<<endl;
    for(int treeId = 0; treeId < numiTrees; treeId++){
        //cout<<treeId<<" Tree"<<endl;
        iTrees[treeId] = new itree();
        //cout<<"tree object with id "<<treeId<<" is created"<<endl;
        iTrees[treeId]->createStaticTree(dataObject, sampleSize);
        //cout<<"tree with id "<<treeId<<" is created"<<endl;
    }
}

void iforest::computeNodeMass(const data & dataObject){
    //cout<<"----NodeMass compute----"<<endl;
    for(int treeId = 0; treeId < this->numiTrees; treeId++){
        //cout<<treeId<<" Tree"<<endl;
        iTrees[treeId]->computeNodeMassforTree(dataObject);
        this->mu += iTrees[treeId]->largestLeafSize;
        this->minPts += iTrees[treeId]->smallestLeafSize;
    }
    //cout<<"minpts"<<this->minPts<<endl;
    this->minPts += (this->mu - this->minPts)*.15;
    this->mu /= dataObject.getnumInstances();
    this->mu /= this->numiTrees;
    this->minPts /= this->numiTrees;
    //cout<<"minpts"<<this->minPts<<endl;
    //this->minPts = 9;
    this->mu += .05;
    //cout<<"Mu"<<this->mu<<endl;
}



//Preprocessing or mark the nodes for clustering large DS
void iforest::markNode(treenode *node, vector<vector<char>> &mNodes , int muPoints, int minPts, int treeId)
{
	if(node->nodeMass >= minPts && node->nodeMass <= muPoints )
	{
		mNodes[treeId][node->nodeId] = '1';
	}
	else if(!node->isLeaf)
	{
		markNode(node->lChildAdd, mNodes,muPoints, minPts, treeId);
		markNode(node->rChildAdd, mNodes,muPoints, minPts, treeId);
	}
	else
	{
		mNodes[treeId][node->nodeId] = '2';
	}
}


void iforest::computeNeighborhood(vector<vector<int>> &_neighborsList, double mu, int minPts, float treeThold)
{
	int totalInstances = _data->getnumInstances();
	_neighborsList.clear();
    _neighborsList.resize(totalInstances);
    int numDistanceComputation = 0;
    int reqDistanceComputation = 0;
    int maxNodesinTree = iTrees[0]->getMaxNumOfNodes();
	vector<vector<char>> markedNodes;
	int muPoints = mu * totalInstances;
	int neighborinAtleast = treeThold*numiTrees;
	//computeLCAlookup(maxNodesinTree);
	for(int treeId = 0; treeId < numiTrees; treeId++)
	{	
		markedNodes.push_back(vector<char>(maxNodesinTree,'0'));
		markNode(iTrees[treeId]->treeNode[0],markedNodes, muPoints, minPts ,treeId);
	}

	vector<int> neighborsofPoint;
	for(int point = 0; point < totalInstances; point++)
	{	
		neighborsofPoint=vector<int>(totalInstances, 0);
		for(int tree = 0; tree < numiTrees; tree++)
		{	
			int markednodeIdforPoint = iTrees[tree]->pointToNode[point];     //initialize with the leaf node of the point and find marked node
			//cout<<"before"<<markedNodes[tree][markednodeIdforPoint]<<endl;
						
			if(markedNodes[tree][markednodeIdforPoint] == '2'){continue;}
			//cout<<"after"<<markedNodes[tree][markednodeIdforPoint]<<endl;
			while(markedNodes[tree][markednodeIdforPoint] != '1')
			{	
				markednodeIdforPoint = (int)(markednodeIdforPoint-1)/2;
			
			}
			
			for(auto neighborId : iTrees[tree]->treeNode[markednodeIdforPoint]->dataPointIndex)
			{	//cout<<"nidddd"<<neighborId<<endl;
				neighborsofPoint[neighborId]++;		
				if(neighborsofPoint[neighborId] == neighborinAtleast)
				{	//cout<<"first appearance"<<neighborId<<endl;
					if(point < neighborId )
					{	
						numDistanceComputation++;
						if(dissScoreComputation(point,neighborId,totalInstances,mu) < mu)
						{	reqDistanceComputation++;
							_neighborsList[point].push_back(neighborId);
							_neighborsList[neighborId].push_back(point);
						}	
					}
				}
			}
		}
	}
	cout<<"#Potential pairs"<<numDistanceComputation<<endl;
	cout<<"#Qualifying pairs"<<reqDistanceComputation<<endl;
}













void iforest::findNeighbors( data * dataObject, vector<vector<int>> &potentialNeighbors, double mu, int minPts)
{
	int maxNodesinTree = iTrees[0]->getMaxNumOfNodes();
	vector<vector<char>> markedNodes;
	int totalInstances = dataObject->getnumInstances();
	int muPoints = mu * totalInstances;
	int neighborinAtleast = 0.01*numiTrees;
	//computeLCAlookup(maxNodesinTree);
	for(int treeId = 0; treeId < numiTrees; treeId++)
	{	
		markedNodes.push_back(vector<char>(maxNodesinTree,false));
		markNode(iTrees[treeId]->treeNode[0],markedNodes, muPoints, minPts ,treeId);
	}
	//cout<<"markedNodes matrix creation done"<<endl;
	//vector<int> firstAppearanceUpdate(totalInstances, -1);
	//vector<int> countOfTotalAppearances(totalInstances, 0);
	vector<int> neighborsofPoint(totalInstances, 0);
	for(int point = 0; point < totalInstances; point++)
	{	
		neighborsofPoint=vector<int>(totalInstances, 0);
		for(int tree = 0; tree < numiTrees; tree++)
		{	
			int markednodeIdforPoint = this->iTrees[tree]->pointToNode[point];     //initialize with the leaf node of the point and find marked node			
			
			while(markedNodes[tree][markednodeIdforPoint] != true)
			{	
				markednodeIdforPoint = (int)(markednodeIdforPoint-1)/2;
			
			}
			
			for(auto neighborId : this->iTrees[tree]->treeNode[markednodeIdforPoint]->dataPointIndex)
			{	
				
				neighborsofPoint[neighborId]++;		
				if(neighborsofPoint[neighborId] == neighborinAtleast)
				{	
					if(point < neighborId && dissScoreComputation(point,neighborId,totalInstances,mu) < mu)
					{	
						potentialNeighbors[point].push_back(neighborId);
						potentialNeighbors[neighborId].push_back(point);
					}
				}
			}
		}
	}	
}






/*
//Version1.0:Very basic findNeighbors
void iforest::findNeighbors(data * dataObject, vector<vector<int>> &potentialNeighbors, double mu)
{
	int maxNodesinTree = iTrees[0]->getMaxNumOfNodes();
	vector<vector<bool>> markedNodes;
	int totalInstances = dataObject->getnumInstances();
	int muPoints = mu * totalInstances;
	computeLCAlookup(maxNodesinTree);
	cout<<"LCA matrix creation done"<<endl;
	for(int treeId = 0; treeId < numiTrees; treeId++)
	{	cout<<"111tree"<<treeId<<endl;
		markedNodes.push_back(vector<bool>(maxNodesinTree,false));
		cout<<"22222tree"<<treeId<<endl;
		markNode(iTrees[treeId]->treeNode[0],markedNodes, muPoints,treeId);
		cout<<"33333tree"<<treeId<<endl;
	}
	cout<<"markedNodes matrix creation done"<<endl;
	vector<int> neighborsofPoint(totalInstances,0);
	cout<<"\n4444tree"<<endl;
	for(int point = 0; point < totalInstances; point++)
	{	neighborsofPoint.clear();
		neighborsofPoint = vector<int>(totalInstances,0);
		for(int tree = 0; tree < this->numiTrees; tree++)
		{	
			cout<<"\ntree"<<tree;
			//for(int id = 0; id<maxNodesinTree;id++){
				//if(markedNodes[tree][id] ==true)
				//{cout<<id<<" ";}}
			//cout<<this->iTrees[tree]->pointToNode[point];
			int markednodeIdforPoint = this->iTrees[tree]->pointToNode[point];     //initialize with the leaf node of the point and find marked node			
			//cout<<"leaf for "<<point<<" is "<<markednodeIdforPoint<<endl;
			//cout<<"\ntree"<<tree<<endl;;
			while(markedNodes[tree][markednodeIdforPoint] != true)
			{
				markednodeIdforPoint = (int)(markednodeIdforPoint-1)/2;
			
			}
			//cout<<"\ntree"<<tree<<endl;
			
			for(auto neighborId : this->iTrees[tree]->treeNode[markednodeIdforPoint]->dataPointIndex)
			{
				neighborsofPoint[neighborId]++;
				if(neighborsofPoint[neighborId] == 1)
				{
					if(point < neighborId)
					{
						if(dissScoreComputation(point,neighborId,totalInstances,mu) < mu)
						{
							potentialNeighbors[point].push_back(neighborId);
							potentialNeighbors[neighborId].push_back(point);
						}
					}
				}
			}
			cout<<"\ntree"<<tree<<endl;
		}
	}	
}*/




/*//findNeighbors optimized implementation 1. using 2 int arrays for each point & 2. reducing dissScorecomputation to half
void iforest::findNeighbors(const data & dataObject, vector<vector<int>> &potentialNeighbors, double mu)
{
	int maxNodesinTree = iTrees[0]->getMaxNumOfNodes();
	vector<vector<bool>> markedNodes;
	int totalInstances = dataObject.getnumInstances();
	int muPoints = mu * totalInstances;
	//computeLCAlookup(maxNodesinTree);
	for(int treeId = 0; treeId < numiTrees; treeId++)
	{	
		markedNodes.push_back(vector<bool>(maxNodesinTree,false));
		markNode(iTrees[treeId]->treeNode[0],markedNodes, muPoints,treeId);
	}
	//cout<<"markedNodes matrix creation done"<<endl;
	vector<int> firstAppearanceUpdate(totalInstances, -1);
	vector<int> countOfTotalAppearances(totalInstances, 0);
	for(int point = 0; point < totalInstances; point++)
	{	
		//neighborsofPoint[totalInstances] = {0};
		for(int tree = 0; tree < this->numiTrees; tree++)
		{	
			//cout<<"\ntree"<<tree;
			//for(int id = 0; id<maxNodesinTree;id++){
				//if(markedNodes[tree][id] ==true)
				//{cout<<id<<" ";}}
			int markednodeIdforPoint = this->iTrees[tree]->pointToNode[point];     //initialize with the leaf node of the point and find marked node			
			//cout<<"leaf for "<<point<<" is "<<markednodeIdforPoint<<endl;
			
			while(markedNodes[tree][markednodeIdforPoint] != true)
			{
				markednodeIdforPoint = (int)(markednodeIdforPoint-1)/2;
			
			}
			for(auto neighborId : this->iTrees[tree]->treeNode[markednodeIdforPoint]->dataPointIndex)
			{	
				if(firstAppearanceUpdate[neighborId] < point)
				{
					firstAppearanceUpdate[neighborId] = point;
					countOfTotalAppearances[neighborId] = 1;
				}
				else if(firstAppearanceUpdate[neighborId] == point)
				{
					countOfTotalAppearances[neighborId]++;
				}
				
				if(countOfTotalAppearances[neighborId] == 1)
				{	
					if(point <= neighborId && dissScoreComputation(point,neighborId,totalInstances,mu) < mu)
					{
						potentialNeighbors[point].push_back(neighborId);
						potentialNeighbors[neighborId].push_back(point);
					}
				}
			}
		}
	}	
}*/


//dissScore computation with each time LCA computation  with and without maintaining runningAverage of mu
double iforest::dissScoreComputation(int point1,int point2, int totalPoints, double mu){
	double tempMass = 0;
	for(int treeId = 0; treeId < iTrees.size(); treeId++){
                int leafNodeforPoint1 = iTrees[treeId]->pointToNode[point1];
                //cout<<"leaf for point1 "<<leafNodeforPoint1<<" ";
                int leafNodeforPoint2 = iTrees[treeId]->pointToNode[point2];
                //cout<<"leaf for point2 "<<leafNodeforPoint2<<" ";
                int LCAnodeforPoint1_Pint2 = computeLCA(leafNodeforPoint1,leafNodeforPoint2);
                //cout<<LCAnodeforPoint1_Pint2<<" "<<endl;

                //cout<<"NodeMAss"<<iTrees[treeId]->treeNode[LCAnodeforPoint1_Pint2]->nodeMass<<endl;
                tempMass += iTrees[treeId]->treeNode[LCAnodeforPoint1_Pint2]->nodeMass;
                
                //Uncomment below 2 lines for maintaining runningAvg of mu and terminate early if happened to be
                //double runningAvgofMu = tempMass/totalPoints; 
                //if(runningAvgofMu > muXtrees){return (double)(runningAvgofMu/numiTrees);}
                                
                
                //cout<< tempMass<<" ";

	}
	//cout<<tempMass<<endl;
	tempMass = tempMass/numiTrees;
	//cout<<tempMass<<endl;
	tempMass = tempMass/totalPoints;
	//cout<<tempMass<<endl;
	//cout<<tempMass;
	return tempMass;
}


//dissScoreComputation using LCA lookup
double iforest::dissScoreComputationUsingLCAlookup(int point1,int point2, int totalPoints, double mu){
	double tempMass = 0;
	double muXtrees = mu*numiTrees;
	for(int treeId = 0; treeId < iTrees.size(); treeId++){
				//cout<<"leaf for point1 "<<endl;
                
                int leafNodeforPoint1 = iTrees[treeId]->pointToNode[point1];
                //cout<<"leaf for point1 "<<leafNodeforPoint1<<endl;
                int leafNodeforPoint2 = iTrees[treeId]->pointToNode[point2];
                //cout<<"leaf for point2 "<<leafNodeforPoint2<<endl;
                int LCAnodeforPoint1_Pint2;
                if(leafNodeforPoint1 <= leafNodeforPoint2)
                {
                	LCAnodeforPoint1_Pint2 = LCAlookup[leafNodeforPoint2][leafNodeforPoint1];
                }
                else
                {
                	LCAnodeforPoint1_Pint2 = LCAlookup[leafNodeforPoint1][leafNodeforPoint2];
                }
                //cout<<LCAnodeforPoint1_Pint2<<" "<<endl;

                //cout<<"NodeMAss"<<iTrees[treeId]->treeNode[LCAnodeforPoint1_Pint2]->nodeMass<<endl;
                tempMass += iTrees[treeId]->treeNode[LCAnodeforPoint1_Pint2]->nodeMass;
                
                //Uncomment below 2 lines for maintaining runningAvg of mu and terminate early if happened to be
                //double runningAvgofMu = tempMass/totalPoints; 
                //if(runningAvgofMu > muXtrees){return (double)(runningAvgofMu/numiTrees);}
                
                
                //cout<< tempMass<<" ";

	}
	//cout<<tempMass<<endl;
	tempMass = tempMass/numiTrees;
	//cout<<tempMass<<endl;
	tempMass = tempMass/totalPoints;
	//cout<<tempMass<<endl;
	//cout<<tempMass;
	return tempMass;
}

double iforest::dissScoreComputationUsingLCAlookup_runningAvg(int point1,int point2, int totalPoints, double mu){
	double tempMass = 0;
	double muXtrees = mu*numiTrees;
	for(int treeId = 0; treeId < iTrees.size(); treeId++){
				//cout<<"leaf for point1 "<<endl;
                
                int leafNodeforPoint1 = iTrees[treeId]->pointToNode[point1];
                //cout<<"leaf for point1 "<<leafNodeforPoint1<<endl;
                int leafNodeforPoint2 = iTrees[treeId]->pointToNode[point2];
                //cout<<"leaf for point2 "<<leafNodeforPoint2<<endl;
                int LCAnodeforPoint1_Pint2;
                if(leafNodeforPoint1 <= leafNodeforPoint2)
                {
                	LCAnodeforPoint1_Pint2 = LCAlookup[leafNodeforPoint2][leafNodeforPoint1];
                }
                else
                {
                	LCAnodeforPoint1_Pint2 = LCAlookup[leafNodeforPoint1][leafNodeforPoint2];
                }
                //cout<<LCAnodeforPoint1_Pint2<<" "<<endl;

                //cout<<"NodeMAss"<<iTrees[treeId]->treeNode[LCAnodeforPoint1_Pint2]->nodeMass<<endl;
                tempMass += iTrees[treeId]->treeNode[LCAnodeforPoint1_Pint2]->nodeMass;
                
                //Uncomment below 2 lines for maintaining runningAvg of mu and terminate early if happened to be
                double runningAvgofMu = tempMass/totalPoints; 
                if(runningAvgofMu > muXtrees){return (double)(runningAvgofMu/numiTrees);}
                
                
                //cout<< tempMass<<" ";

	}
	//cout<<tempMass<<endl;
	tempMass = tempMass/numiTrees;
	//cout<<tempMass<<endl;
	tempMass = tempMass/totalPoints;
	//cout<<tempMass<<endl;
	//cout<<tempMass;
	return tempMass;
}



int iforest::computeLCA(int node1, int node2){
    while(node1!=node2){
        if(node1>node2){node1 = node1%2==0 ? (node1/2)-1 : (node1-1)/2;}
        else{node2 = node2%2==0 ? (node2/2)-1 : (node2-1)/2;}
    }
    return node1;
}


void iforest::computeLCAlookup(int maxNodes)
{	//LCAlookup.clear();
	LCAlookup.resize(maxNodes);
	for(int node1 = 0; node1 < maxNodes; node1++)
	{	//LCAlookup[node1].resize(node1+1);
		
		for(int node2 = 0; node2 <= node1; node2++)
		{
			int nodey = node2;int nodex = node1;
			//cout<<"node1 "<<node1<<"node2 "<<node2<<endl;
			
			//cout<<"nodex "<<nodex<<"nodey "<<nodey<<endl;
			while(nodex!=nodey)
			{
				if(nodex>nodey){nodex = nodex%2 == 0 ? (nodex/2)-1 : (nodex-1)/2;}
				else{nodey = nodey%2 == 0 ? (nodey/2)-1 : (nodey-1)/2;}
			}
			//cout<<"node1111 "<<node1<<"node222 "<<node2<<" LCA= "<<nodey<<endl;
			LCAlookup[node1].push_back(nodey);
			//LCAlookup[node1][node2] = nodey;
		}
	}
}






void iforest::massMatrixComputation(vector<vector<double>> &massMatrix){       //store mass values in matrix provided
    double tempMass = 0;

    int sizeOfPointsInTree ;
    vector<int> pointsInTree;
    vector<double> pairwiseMass;
    for(int p = 0; p < iTrees[0]->pointToNode.size(); p++){
    	if(iTrees[0]->pointToNode[p] >=0)
    	{
            pointsInTree.push_back(p);
        }
    }
    sizeOfPointsInTree = pointsInTree.size();
	for(int p1 = 0; p1 < sizeOfPointsInTree; p1++){
        int point1 = pointsInTree[p1];
        pairwiseMass.resize(0);
        for(int p2 = 0; p2 <= p1; p2++)
        {
	    	int point2 = pointsInTree[p2];
	    	tempMass=0;
        	for(int treeId = 0; treeId < iTrees.size(); treeId++)
        	{
        		int leafNodeforPoint1 = iTrees[treeId]->pointToNode[point1];
        		//cout<<"leaf for point1"<<leafNodeforPoint1<<" ";
        		int leafNodeforPoint2 = iTrees[treeId]->pointToNode[point2];
        		//cout<<"leaf for point2"<<leafNodeforPoint2<<" ";
            	int LCAnodeforPoint1_Pint2 = computeLCA(leafNodeforPoint1,leafNodeforPoint2);
            	//cout<<LCAnodeforPoint1_Pint2<<" "<<endl;
            	//cout<<"NodeMAss"<<iTrees[treeId]->treeNode[LCAnodeforPoint1_Pint2]->nodeMass<<endl;
            	tempMass += iTrees[treeId]->treeNode[LCAnodeforPoint1_Pint2]->nodeMass;
            	//cout<< tempMass<<" ";
			}
        	tempMass = tempMass/numiTrees;
        	tempMass = tempMass/sizeOfPointsInTree;
        	//cout<<tempMass<<endl;
        	pairwiseMass.push_back(tempMass);
        	//cout<<tempMass<<endl;
        	//cout<<point1<<" "<<point2<<": "<< tempMass<<endl;
        	//cout<<tempMass<<endl;
        	//cout<<point1<<" "<<point2<<": "<< pairwiseMass[p2]<<endl;
		}
    	massMatrix.push_back(pairwiseMass);
    	pairwiseMass.clear();
	}
}









