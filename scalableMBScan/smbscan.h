#ifndef smbscan_H
#define smbscan_H

#include <vector>
#include <queue>
#include <set>
#include <memory>
#include <functional>
#include <math.h>
#include "data.h"
#include "iforest.h"

class smbscan final
{
    enum ERROR_TYPE
    {
        SUCCESS = 0,
        FAILED,
        COUNT
    };

    public:
        smbscan() {}
        ~smbscan() {}
        int Run(data * dataOb, iforest * iForestOb, const float mu, const int min, const float treeTh);
        void smbscanInitialization(data * dataOb, iforest * iForestOb, const float mu, const int min, const float treeTh);
        void computeNeighborhood();
        void computeNeighborhoodUsingLCAlookup();
        void computeNeighborhoodUsingLCAlookup_runningAvg();
        
        void markNode(treenode *node, vector<vector<char>> &mNodes , int muPoints, int treeId);
        void doClustering();
        //void computeLCAlookup(int maxNodes);
        
        

    protected:

    private:         
        std::vector<int> regionQuery(const int pid) const;
        void addToCluster(const int pid, const int cid);				//add point "pid" to a cluster having cluster id "cid"
        void expandCluster(const int cid, const std::vector<int> &neighbors);
        
    public:
        std::vector<std::vector<int>> clusters;
        //std::vector<int> corePoints;
        //std::vector<int> borderPoints;
        //std::vector<int> noisePoints;
        std::vector<int> clusterId;
   
        std::vector<bool> _visited;
        std::vector<bool> _inCurrentQueue;        
		int _datalen;
        int _datadim;
        int _minpts;
        float _mu;
        float _treeThold;
        iforest * _iforest;
        data *_data;
		vector<vector<int>> _neighborsList;

};


int smbscan::Run(data * dataOb, iforest * iForestOb, const float mu, const int min, const float treeTh)
{
    if(dataOb->getnumInstances() < 1)
        return ERROR_TYPE::FAILED;
    if (dataOb->getnumAttributes() < 1)
        return ERROR_TYPE::FAILED;
    if (min < 1)
        return ERROR_TYPE::FAILED;
        
	smbscanInitialization(dataOb,iForestOb, mu, min, treeTh);
	computeNeighborhood();
	doClustering();
	
    return ERROR_TYPE::SUCCESS;
}




void smbscan::smbscanInitialization(data * dataOb, iforest * iForestOb, const float mu, const int min,const float treeTh)
{
	this->_data = dataOb;
    this->_iforest = iForestOb;
    this->_datalen = dataOb->getnumInstances();
    this->_datadim = dataOb->getnumAttributes();
    this->_visited = std::vector<bool>(this->_datalen, false);
    this->clusters.clear();
    //this->noisePoints.clear();
    //this->borderPoints.clear();
    //this->corePoints.clear();
    this->clusterId.clear();
    this->clusterId.resize(this->_datalen, -1);
    this->_minpts = min;
    this->_mu = mu;
    this->_treeThold = treeTh;
    this->_minpts = this->_iforest->getminPts();
    this->_mu = this->_iforest->getMu();

}
void smbscan::computeNeighborhood()
{
	this->_neighborsList.clear();
    this->_neighborsList.resize(this->_datalen);
    int numDistanceComputation = 0;
    int reqDistanceComputation = 0;
    int maxNodesinTree = this->_iforest->iTrees[0]->getMaxNumOfNodes();
	vector<vector<char>> markedNodes;
	int muPoints = this->_mu * this->_datalen;
	int numiTree = this->_iforest->numiTrees;
	int neighborinAtleast = this->_treeThold*numiTree;
	//computeLCAlookup(maxNodesinTree);
	for(int treeId = 0; treeId < numiTree; treeId++)
	{	
		markedNodes.push_back(vector<char>(maxNodesinTree,'0'));
		markNode(this->_iforest->iTrees[treeId]->treeNode[0],markedNodes, muPoints ,treeId);
	}

	vector<int> neighborsofPoint(this->_datalen, 0);
	for(int point = 0; point < this->_datalen; point++)
	{	
		neighborsofPoint=vector<int>(this->_datalen, 0);
		for(int tree = 0; tree < numiTree; tree++)
		{	
			int markednodeIdforPoint = this->_iforest->iTrees[tree]->pointToNode[point];     //initialize with the leaf node of the point and find marked node
			//cout<<"before"<<markedNodes[tree][markednodeIdforPoint]<<endl;
						
			if(markedNodes[tree][markednodeIdforPoint] == '2'){continue;}
			//cout<<"after"<<markedNodes[tree][markednodeIdforPoint]<<endl;
			while(markedNodes[tree][markednodeIdforPoint] != '1')
			{	
				markednodeIdforPoint = (int)(markednodeIdforPoint-1)/2;
			
			}
			
			for(auto neighborId : this->_iforest->iTrees[tree]->treeNode[markednodeIdforPoint]->dataPointIndex)
			{	//cout<<"nidddd"<<neighborId<<endl;
				neighborsofPoint[neighborId]++;		
				if(neighborsofPoint[neighborId] == neighborinAtleast)
				{	//cout<<"first appearance"<<neighborId<<endl;
					if(point < neighborId )
					{	
						numDistanceComputation++;
						if(this->_iforest->dissScoreComputation(point,neighborId,this->_datalen,this->_mu) < this->_mu)
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



void smbscan::computeNeighborhoodUsingLCAlookup()
{
	this->_neighborsList.clear();
    this->_neighborsList.resize(this->_datalen);
    int numDistanceComputation = 0;
    int reqDistanceComputation = 0;
    int maxNodesinTree = this->_iforest->iTrees[0]->getMaxNumOfNodes();
	vector<vector<char>> markedNodes;
	int muPoints = this->_mu * this->_datalen;
	int numiTree = this->_iforest->numiTrees;
	int neighborinAtleast = this->_treeThold*numiTree;
	this->_iforest->computeLCAlookup(maxNodesinTree);
	for(int treeId = 0; treeId < numiTree; treeId++)
	{	
		markedNodes.push_back(vector<char>(maxNodesinTree,'0'));
		markNode(this->_iforest->iTrees[treeId]->treeNode[0],markedNodes, muPoints ,treeId);
	}

	vector<int> neighborsofPoint(this->_datalen, 0);
	for(int point = 0; point < this->_datalen; point++)
	{	
		neighborsofPoint=vector<int>(this->_datalen, 0);
		for(int tree = 0; tree < numiTree; tree++)
		{	
			int markednodeIdforPoint = this->_iforest->iTrees[tree]->pointToNode[point];     //initialize with the leaf node of the point and find marked node
			//cout<<"before"<<markedNodes[tree][markednodeIdforPoint]<<endl;
						
			if(markedNodes[tree][markednodeIdforPoint] == '2'){continue;}
			//cout<<"after"<<markedNodes[tree][markednodeIdforPoint]<<endl;
			while(markedNodes[tree][markednodeIdforPoint] != '1')
			{	
				markednodeIdforPoint = (int)(markednodeIdforPoint-1)/2;
			
			}
			//cout<<"point"<<point<<endl;
			for(auto neighborId : this->_iforest->iTrees[tree]->treeNode[markednodeIdforPoint]->dataPointIndex)
			{	//cout<<"nidddd"<<neighborId<<endl;
				neighborsofPoint[neighborId]++;		
				if(neighborsofPoint[neighborId] == neighborinAtleast)
				{	//cout<<"first appearance"<<neighborId<<endl;
				//cout<<this->_iforest->dissScoreComputationUsingLCAlookup(point,neighborId,this->_datalen,this->_mu);
					if(point < neighborId)
					{	
						numDistanceComputation++;
						if(this->_iforest->dissScoreComputationUsingLCAlookup(point,neighborId,this->_datalen,this->_mu) < this->_mu)
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



void smbscan::computeNeighborhoodUsingLCAlookup_runningAvg()
{
	this->_neighborsList.clear();
    this->_neighborsList.resize(this->_datalen);
    int numDistanceComputation = 0;
    int reqDistanceComputation = 0;
    int maxNodesinTree = this->_iforest->iTrees[0]->getMaxNumOfNodes();
	vector<vector<char>> markedNodes;
	int muPoints = this->_mu * this->_datalen;
	int numiTree = this->_iforest->numiTrees;
	int neighborinAtleast = this->_treeThold*numiTree;
	this->_iforest->computeLCAlookup(maxNodesinTree);
	for(int treeId = 0; treeId < numiTree; treeId++)
	{	
		markedNodes.push_back(vector<char>(maxNodesinTree,'0'));
		markNode(this->_iforest->iTrees[treeId]->treeNode[0],markedNodes, muPoints ,treeId);
	}

	vector<int> neighborsofPoint(this->_datalen, 0);
	for(int point = 0; point < this->_datalen; point++)
	{	
		neighborsofPoint=vector<int>(this->_datalen, 0);
		for(int tree = 0; tree < numiTree; tree++)
		{	
			int markednodeIdforPoint = this->_iforest->iTrees[tree]->pointToNode[point];     //initialize with the leaf node of the point and find marked node
			//cout<<"before"<<markedNodes[tree][markednodeIdforPoint]<<endl;
						
			if(markedNodes[tree][markednodeIdforPoint] == '2'){continue;}
			//cout<<"after"<<markedNodes[tree][markednodeIdforPoint]<<endl;
			while(markedNodes[tree][markednodeIdforPoint] != '1')
			{	
				markednodeIdforPoint = (int)(markednodeIdforPoint-1)/2;
			
			}
			//cout<<"point"<<point<<endl;
			for(auto neighborId : this->_iforest->iTrees[tree]->treeNode[markednodeIdforPoint]->dataPointIndex)
			{	//cout<<"nidddd"<<neighborId<<endl;
				neighborsofPoint[neighborId]++;		
				if(neighborsofPoint[neighborId] == neighborinAtleast)
				{	
					if(point < neighborId)
					{
						numDistanceComputation++;
						if(this->_iforest->dissScoreComputationUsingLCAlookup_runningAvg(point,neighborId,this->_datalen,this->_mu) < this->_mu)
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



void smbscan::markNode(treenode *node, vector<vector<char>> &mNodes , int muPoints, int treeId)
{
	if(node->nodeMass >= this->_minpts && node->nodeMass <= muPoints )
	{
		mNodes[treeId][node->nodeId] = '1';
	}
	else if(!node->isLeaf)
	{
		markNode(node->lChildAdd, mNodes,muPoints, treeId);
		markNode(node->rChildAdd, mNodes,muPoints, treeId);
	}
	else
	{
		mNodes[treeId][node->nodeId] = '2';
	}
}

void smbscan::doClustering()
{
	for(int pid = 0; pid < this->_datalen; ++pid)
    {
		this->_inCurrentQueue.clear();
		this->_inCurrentQueue.resize(0);
		this->_inCurrentQueue = vector<bool>(this->_datalen,false);       
		if (!this->_visited[pid])
        {
            this->_visited[pid] = true;

            // Outliner it maybe noise or on the border of one cluster.
          	this->_inCurrentQueue[pid] = true;
            const std::vector<int> neighbors = this->_neighborsList[pid];//this->regionQuery(pid);
            if (neighbors.size() < this->_minpts)
            {
                continue;
            }
            else
            {
            	//this->corePoints.push_back(pid);
                int cid = (int)this->clusters.size();
                this->clusters.push_back(std::vector<int>());
                // first blood
                this->addToCluster(pid, cid);
                this->expandCluster(cid, neighbors);
            }
        }
    }
}

std::vector<int> smbscan::regionQuery(const int pid) const
{
	return this->_neighborsList[pid];
	//std::vector<int> neighbors(this->_neighborsList[pid].begin(), this->_neighborsList[pid].end());	
	//return neighbors;
}


void smbscan::expandCluster(const int cid, const std::vector<int> &neighbors)
{
    std::queue<int> neighborQueue;
    for (int pid : neighbors)
	{
		this->_inCurrentQueue[pid] = true;
		neighborQueue.push(pid);	
	}
    while (neighborQueue.size() > 0)
    {
        const int pid = neighborQueue.front();
        neighborQueue.pop();

        if (!this->_visited[pid])
        {
            this->_visited[pid] = true;
            const std::vector<int> pidneighbors = this->_neighborsList[pid]; //this->regionQuery(pid);
			// Core point, the neighbors will be expanded
            this->addToCluster(pid, cid);
			if (pidneighbors.size() >= this->_minpts)
            {
            	//this->corePoints.push_back(pid); 
                for (int pidnid : pidneighbors)
                {
                    if (!this->_inCurrentQueue[pidnid])                    
					{
                        neighborQueue.push(pidnid);
						this->_inCurrentQueue[pidnid] = true;
                    }
                }
            }
            else
            {
            	//this->borderPoints.push_back(pid);
            }
        }
    }
}

void smbscan::addToCluster(const int pid, const int cid)
{
    this->clusters[cid].push_back(pid);
    this->clusterId[pid] = cid;
}

#endif // smbscan_H
