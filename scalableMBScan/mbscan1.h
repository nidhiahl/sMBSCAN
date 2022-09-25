#define LARGEDATASET true


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
        int Run(const data &dataOb, const float mu, const int min);

    protected:

    private:         
        std::vector<int> regionQuery(const data &dataOb, const int pid) const;
        void addToCluster(const int pid, const int cid);				//add point "pid" to a cluster having cluster id "cid"
        void expandCluster(const data &dataOB, const int cid, const std::vector<int> &neighbors);
        
    public:
        std::vector<std::vector<int>> clusters;
        //std::vector<int> corePoints;
        //std::vector<int> borderPoints;
        //std::vector<int> noisePoints;
        std::vector<int> clusterId;

    private:
        std::vector<bool> _visited;
        std::vector<bool> _inCurrentQueue;        
	int _datalen;
        int _datadim;
        int _minpts;
        float _mu;
        iforest *_iforest;
        #if !LARGEDATASET
        	vector<vector<double>> _massMatrix;
        #else
        	vector<vector<int>> _potentialNeighbors;
        #endif //!LARGEDATASET
};


int mbscan::Run(const data &dataOb, const float mu, const int min)
{
    if(dataOb.getnumInstances() < 1)
        return ERROR_TYPE::FAILED;
    if (dataOb.getnumAttributes() < 1)
        return ERROR_TYPE::FAILED;
    if (min < 1)
        return ERROR_TYPE::FAILED;
        
	//cout<<"dataOb.getnumInstances()"<<dataOb.getnumInstances()<<endl;
    this->_datalen = dataOb.getnumInstances();
    this->_datadim = dataOb.getnumAttributes();
    this->_visited = std::vector<bool>(this->_datalen, false);
    this->clusters.clear();
    //this->noisePoints.clear();
    //this->borderPoints.clear();
    //this->corePoints.clear();
    this->clusterId.clear();
    this->clusterId.resize(this->_datalen, -1);
    this->_minpts = min;
    this->_mu = mu;
    
    
    struct timespec iForest_start,iForest_end;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &iForest_start);

    this->_iforest = new iforest(100);
    //cout<<"this->datale"<<this->_datalen<<endl;
    this->_iforest->Run(dataOb, std::min(256,this->_datalen));
    
    this->_minpts = this->_iforest->getminPts();
    this->_mu = this->_iforest->getMu();
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &iForest_end);
    double iForestTime =  (((iForest_end.tv_sec - iForest_start.tv_sec) * 1e9)+(iForest_end.tv_nsec - iForest_start.tv_nsec))*1e-9;
    cout << "Time taken for iForest " << fixed << iForestTime<<"sec"<<endl;
    /*ramUsed = max(getValue(1),ramUsed);
    vMemUsed = max(getValue(2),vMemUsed);
    cout<< "ram used: "<<getValue(1)<<endl;*/
    
    
    struct timespec massMatrix_start,massMatrix_end;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &massMatrix_start);

    #if !LARGEDATASET
    	this->_massMatrix.clear();
    	this->_iforest->massMatrixComputation(this->_massMatrix);
    #else
    	this->_potentialNeighbors.clear();
    	this->_potentialNeighbors.resize(dataOb.getnumInstances());
    	this->_iforest->findNeighbors(dataOb,this->_potentialNeighbors, this->_mu);
    #endif //!LARGEDATASET
    
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &massMatrix_end);
	double massMatrixTime =  (((massMatrix_end.tv_sec - massMatrix_start.tv_sec) * 1e9)+(massMatrix_end.tv_nsec - massMatrix_start.tv_nsec))*1e-9;
	cout << "Time taken for massMatrix " << fixed << massMatrixTime<<"sec"<<endl;
        
	/*ramUsed = max(getValue(1),ramUsed);
	vMemUsed = max(getValue(2),vMemUsed);
	cout<< "ram used: "<<getValue(1)<<endl;*/


	struct timespec mbscan_start,mbscan_end;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &mbscan_start);


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
            const std::vector<int> neighbors = this->regionQuery(dataOb, pid);
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
                this->expandCluster(dataOb, cid, neighbors);
            }
        }
    }

	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &mbscan_end);
	double mbscanTime =  (((mbscan_end.tv_sec - mbscan_start.tv_sec) * 1e9)+(mbscan_end.tv_nsec - mbscan_start.tv_nsec))*1e-9;
	cout << "Time taken for mbscan " << fixed << mbscanTime<<"sec"<<endl;
        
	/*ramUsed = max(getValue(1),ramUsed);
	vMemUsed = max(getValue(2),vMemUsed);
	cout<< "ram used: "<<getValue(1)<<endl;	*/
	delete _iforest;
    return ERROR_TYPE::SUCCESS;
}


std::vector<int> mbscan::regionQuery(const data &dataOb, const int pid) const
{
    
	#if !LARGEDATASET
		std::vector<int> neighbors;
		neighbors.clear();
	    for (int i = 0; i < this->_datalen; ++i){
    	    double distance;
    	    if(pid < i){distance = this->_massMatrix[i][pid];}
    	    else{distance = this->_massMatrix[pid][i];}
    	    if (i != pid && distance < this->_mu)
    	    {
    	    	neighbors.push_back(i);
    	    }
		}
		
	#else
		
		std::vector<int> neighbors(this->_potentialNeighbors[pid].begin(), this->_potentialNeighbors[pid].end());
		
	#endif
	return neighbors;
}


void mbscan::expandCluster(const data &dataOb, const int cid, const std::vector<int> &neighbors)
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
            const std::vector<int> pidneighbors = this->regionQuery(dataOb, pid);
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
