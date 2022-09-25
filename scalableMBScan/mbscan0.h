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
        void addToNeighborList(const int pid) { this->_neighborList.insert(pid); }
        void addToNeighborList(const std::vector<int> &pids)
        {
            for (int pid : pids)
                this->_neighborList.insert(pid);
        }
        bool isInNeighborList(const int pid) const
        { return this->_neighborList.end() != this->_neighborList.find(pid); }

        


    public:
        std::vector<std::vector<int>> clusters;
        std::vector<int> corePoints;
        std::vector<int> borderPoints;
        std::vector<int> noisePoints;
        std::vector<int> clusterId;

    private:
        std::vector<bool> _visited;
        std::vector<bool> _assigned;
        std::set<int> _neighborList;
        int _datalen;
        int _datadim;
        int _minpts;
        float _mu;
        iforest *_iforest;
        #if !LARGEDATASET
        	vector<vector<double>> _massMatrix;
        #else
        	vector<set<int>> _potentialNeighbors;
        #endif //!LARGEDATASET
};


int mbscan::Run(const data &dataOb, const float mu, const int min)
{
	//cout<<"inside run of mbscan" <<endl;
    if(dataOb.getnumInstances() < 1)
        return ERROR_TYPE::FAILED;
    if (dataOb.getnumAttributes() < 1)
        return ERROR_TYPE::FAILED;
    //cout<<"inside run of mbscan" <<endl;
    if (min < 1)
        return ERROR_TYPE::FAILED;

    this->_datalen = dataOb.getnumInstances();
    this->_visited = std::vector<bool>(this->_datalen, false);
    this->_assigned = std::vector<bool>(this->_datalen, false);
    this->clusters.clear();
    this->noisePoints.clear();
    this->borderPoints.clear();
    this->corePoints.clear();
    this->clusterId.clear();
    this->clusterId.resize(this->_datalen, -1);
    //this->_minpts = min;
    //this->_mu = mu;
    this->_datadim = dataOb.getnumAttributes();
    this->_iforest = new iforest(100);
    //cout<<"iForest object creation done"<<endl;
    this->_iforest->Run(dataOb, 256);
    this->_minpts = this->_iforest->getminPts();
    this->_mu = this->_iforest->getMu();
    //cout<<"iForest creation done"<<endl;
    #if !LARGEDATASET
    	this->_massMatrix.clear();
    	this->_iforest->massMatrixComputation(this->_massMatrix);
    #else
    	this->_potentialNeighbors.clear();
    	this->_potentialNeighbors.resize(dataOb.getnumInstances());
    	this->_iforest->preprocessingForClustering(this->_mu*dataOb.getnumInstances(),this->_minpts);
    	this->_iforest->findPotentialNeighbors(dataOb, this->_potentialNeighbors);
    #endif //!LARGEDATASET
    
    //cout<<"datalen and datadim are "<< this->_datalen<<" "<<this->_datadim<<endl;
    for(int pid = 0; pid < this->_datalen; ++pid)
    {
        this->_neighborList.clear();
		if (!this->_visited[pid])
        {
            this->_visited[pid] = true;

            // Outliner it maybe noise or on the border of one cluster.
			const std::vector<int> neighbors = this->regionQuery(dataOb, pid);
            //cout<<"point"<<pid<<" has "<<neighbors.size()<<"neighbors"<<endl;
            //cout<<"point"<<pid<<" has "<<this->_minpts<<"neighbors"<<endl;
    
    
            if (neighbors.size() < this->_minpts)
            {
                continue;
            }
            else
            {
            
            	this->corePoints.push_back(pid);
                int cid = (int)this->clusters.size();
                this->clusters.push_back(std::vector<int>());
                // first blood
                this->addToNeighborList(pid);
                this->addToCluster(pid, cid);
                this->expandCluster(dataOb, cid, neighbors);
            }
        }
    }

   /* for (int pid = 0; pid < this->_datalen; ++pid)
    {
        if (!this->_assigned[pid])
        {
            this->noisePoints.push_back(pid);
        }
    }*/

/*#if BRUTEFORCE
#else
    //this->destroyKdtree();
#endif // !BRUTEFORCE*/

    return ERROR_TYPE::SUCCESS;
}


std::vector<int> mbscan::regionQuery(const data &dataOb, const int pid) const
{
    std::vector<int> neighbors;
	neighbors.clear();
	#if !LARGEDATASET
	    for (int i = 0; i < this->_datalen; ++i){
    	    double distance;
    	    if(i < pid){distance = this->_massMatrix[i][pid];}
    	    else{distance = this->_massMatrix[pid][i];}
    	    //cout<<"distance between "<< pid<<" and " <<i<<" "<<distance<<" mu = "<<this->_mu<<endl;
    	    if (i != pid && distance < this->_mu)
    	    {
    	    	neighbors.push_back(i);
    	    }
    	    //cout<<"point"<<pid<<" has "<<neighbors.size()<<"neighbors"<<endl;
		}
	#else
		//cout<<"point"<<pid<<" has "<<_potentialNeighbors[pid].size()<<"neighbors"<<endl;
		for (auto i : this->_potentialNeighbors[pid]){
    	    double distance;
    	    distance = this->_iforest->dissScoreComputation(i,pid,dataOb.getnumInstances());
    	    //cout<<"distance between "<< pid<<" and " <<i<<" "<<distance<<endl;
    	    //cout<<"distance between "<< pid<<" and " <<i<<" "<<distance<<" mu = "<<this->_mu<<endl;
    	    //cout<<pid<<" and " <<i<<" "<<distance<<"---";
    	    
    	    if (i != pid && distance < this->_mu){neighbors.push_back(i);}
		}
		//cout<<"point"<<pid<<" has "<<neighbors.size()<<"neighbors"<<endl;
	#endif
	//cout<<"point"<<pid<<" has "<<neighbors.size()<<"neighbors"<<endl;
    //cout<<neighbors.size()<<" "endl;
    
    return neighbors;

}


void mbscan::expandCluster(const data &dataOb, const int cid, const std::vector<int> &neighbors)
{
    std::queue<int> neighborQueue;
    for (int pid : neighbors)
	{
		//this->_inCurrentQueue[pid] = true;
		neighborQueue.push(pid);	
	}
    //this->addToNeighborList(neighbors);

    while (neighborQueue.size() > 0)
    {
        const int pid = neighborQueue.front();
        neighborQueue.pop();

        if (!this->_visited[pid])
        {
            // not been visited, great! , hurry to mark it visited
            this->_visited[pid] = true;
            const std::vector<int> pidneighbors = this->regionQuery(dataOb, pid);
			//cout<<"point"<<pid<<" has "<<pidneighbors.size()<<"neighbors"<<endl;
           // cout<<this->_minpts<<endl;
            // Core point, the neighbors will be expanded
            if (pidneighbors.size() >= this->_minpts)
            {
            	this->corePoints.push_back(pid);
                this->addToCluster(pid, cid);
                for (int pidnid : pidneighbors)
                {
                    if (!this->isInNeighborList(pidnid))
					//if (!this->_inCurrentQueue[pidnid])                    
					{
                        neighborQueue.push(pidnid);
						this->addToNeighborList(pidnid);
                    }
                }
            }
            else
            {
            	this->borderPoints.push_back(pid);
            	this->addToCluster(pid, cid);
            	
            }
        }
    }
}



void mbscan::addToCluster(const int pid, const int cid)
{
    this->clusters[cid].push_back(pid);
    this->_assigned[pid]= true;
    this->clusterId[pid] = cid;
}







#endif // mbscan_H
