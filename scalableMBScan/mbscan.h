#ifndef mbscan_H
#define mbscan_H

#include <vector>
#include <queue>
#include <set>
#include <memory>
#include <functional>
#include <math.h>
#include "data.h"
#include "iforest.h"

class mbscan final
{
    enum ERROR_TYPE
    {
        SUCCESS = 0,
        FAILED,
        COUNT
    };

    public:
        mbscan() {}
        ~mbscan() {}
        int Run(data * dataOb, iforest * iForestOb, const float mu, const int min);
		void mbscanInitialization(data * dataOb, iforest * iForestOb, const float mu, const int min);		
		void computeDistanceMatrix();
		void doClustering();
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
        iforest * _iforest;
        data *_data;
        vector<vector<double>> _massMatrix;

};


int mbscan::Run(data * dataOb, iforest * iForestOb, const float mu, const int min)
{
    //cout<<"in run of mbscan"<<endl;	

	if(dataOb->getnumInstances() < 1)
        return ERROR_TYPE::FAILED;
    if (dataOb->getnumAttributes() < 1)
        return ERROR_TYPE::FAILED;
    if (min < 1)
        return ERROR_TYPE::FAILED;      
    //cout<<"in run of mbscan"<<endl;
    mbscanInitialization(dataOb, iForestOb, mu, min);
	computeDistanceMatrix();
    doClustering();

    return ERROR_TYPE::SUCCESS;
}

void mbscan::mbscanInitialization(data * dataOb, iforest * iForestOb, const float mu, const int min)
{
	  
	//cout<<"dataOb.getnumInstances()"<<dataOb.getnumInstances()<<endl;
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
    this->_minpts = this->_iforest->getminPts();
    this->_mu = this->_iforest->getMu();
    
}

void mbscan::computeDistanceMatrix()
{
	this->_massMatrix.clear();
    this->_iforest->massMatrixComputation(this->_massMatrix);
}

void mbscan::doClustering()
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
            const std::vector<int> neighbors = this->regionQuery(pid);
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

std::vector<int> mbscan::regionQuery(const int pid) const
{
	std::vector<int> neighbors;
	neighbors.clear();
    for (int i = 0; i < this->_datalen; ++i)
    {
   	    double distance;
   	    if(pid < i){distance = this->_massMatrix[i][pid];}
   	    else{distance = this->_massMatrix[pid][i];}
   	    if (i != pid && distance < this->_mu){neighbors.push_back(i);}
	}
	
	return neighbors;
}


void mbscan::expandCluster(const int cid, const std::vector<int> &neighbors)
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
            const std::vector<int> pidneighbors = this->regionQuery(pid);
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

void mbscan::addToCluster(const int pid, const int cid)
{
    this->clusters[cid].push_back(pid);
    this->clusterId[pid] = cid;
}

#endif // mbscan_H
