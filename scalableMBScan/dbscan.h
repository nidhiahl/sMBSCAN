//#define BRUTEFORCE false
#ifndef DBSCAN_H
#define DBSCAN_H

#include <vector>
#include <queue>
#include <set>
#include <memory>
#include <functional>
#include <math.h>
#include "data.h"

/*#if !BRUTEFORCE
#include "iforest.h"
#endif
*/
class dbscan final
{
    enum ERROR_TYPE
    {
        SUCCESS = 0,
        FAILED,
        COUNT
    };

    public:
        dbscan() {}
        ~dbscan() {}
        int Run(const data &dataOb, const float eps, const int min);

    protected:

    private:
        std::vector<int> regionQuery(const data &dataOb, const int pid) const;
        void addToCluster(const int pid, const int cid);
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
        float _epsilon;
        

        /*#if !BRUTEFORCE
            //iForest;
        #endif*/
};


int dbscan::Run(const data &dataOb, const float eps, const int min)
{
	
    if(dataOb.getnumInstances() < 1)
        return ERROR_TYPE::FAILED;
       
    if (dataOb.getnumAttributes() < 1)
        return ERROR_TYPE::FAILED;
    //cout<<"inside run of dbscan" <<endl;
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
    this->_minpts = min;
    this->_epsilon = eps;
    this->_datadim = dataOb.getnumAttributes();

    /*#if BRUTEFORCE
    #else
        //this->buildKdtree(this->_data);
    #endif // !BRUTEFORCE*/
    
    //cout<<"datalen and datadim are "<< this->_datalen<<" "<<this->_datadim<<endl;
    for(int pid = 0; pid < this->_datalen; ++pid)
    {
        this->_neighborList.clear();
        if (!this->_visited[pid])
        {
            this->_visited[pid] = true;

            // Outliner it maybe noise or on the border of one cluster.
            const std::vector<int> neighbors = this->regionQuery(dataOb, pid);
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


std::vector<int> dbscan::regionQuery(const data &dataOb, const int pid) const
{
    std::vector<int> neighbors;

//#if BRUTEFORCE //brute force  O(n^2)
    for (int i = 0; i < this->_datalen; ++i){
        double distance;
        double sumOS = 0;
        for(int d = 0; d < dataOb.getnumAttributes(); d++){
            sumOS += pow(((dataOb.dataVector[pid]->attributes[d])-(dataOb.dataVector[i]->attributes[d])), 2);
        }
        distance = sqrt(sumOS);
        //cout<<"distance between "<< pid<<" and " <<i<<" "<<distance<<endl;
        if (i != pid && distance < this->_epsilon){neighbors.push_back(i);}
    }
//#else //kdtree
//    std::unique_ptr<double[]> v(new double[this->_datadim]);
//    for (uint c = 0; c < this->_datadim; ++c)
//    {
//        v[c] = (double)((*this->_data)[pid][c]);
//    }

//    kdres *presults = kd_nearest_range(this->_kdtree, v.get(), this->_epsilon);
//    while (!kd_res_end(presults))
//    {
        /* get the data and position of the current result item */
//        T *pch = (T *)kd_res_item(presults, v.get());
//        uint pnpid = (uint)(pch - &(*this->_data)[0]);
//        if (pid != pnpid)
//            neighbors.push_back(pnpid);
        /* go to the next entry */
//        kd_res_next(presults);
//    }
//    kd_res_free(presults);

//#endif // !BRUTEFORCE

    return neighbors;

}


void dbscan::expandCluster(const data &dataOb, const int cid, const std::vector<int> &neighbors)
{
    std::queue<int> neighborQueue;
    for (int pid : neighbors){neighborQueue.push(pid);}
    this->addToNeighborList(neighbors);

    while (neighborQueue.size() > 0)
    {
        const int pid = neighborQueue.front();
        neighborQueue.pop();

        if (!this->_visited[pid])
        {

            // not been visited, great! , hurry to mark it visited
            this->_visited[pid] = true;
            const std::vector<int> pidneighbors = this->regionQuery(dataOb, pid);

            // Core point, the neighbors will be expanded
            if (pidneighbors.size() >= this->_minpts)
            {
            	this->corePoints.push_back(pid);
                this->addToCluster(pid, cid);
                for (int pidnid : pidneighbors)
                {
                    if (!this->isInNeighborList(pidnid))
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



void dbscan::addToCluster(const int pid, const int cid)
{
    this->clusters[cid].push_back(pid);
    this->_assigned[pid]= true;
    this->clusterId[pid] = cid;
}







#endif // DBSCAN_H
