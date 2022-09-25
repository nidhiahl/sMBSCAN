#ifndef IFOREST_H
#define IFOREST_H
#include "data.h"
#include "itree.h"

class iforest
{
    public:
        iforest();
        iforest(const int &);
        virtual ~iforest();
        void Run(data *, int);
        void createStaticForest(const data &, int);
        //void createIncrementalForest(const data &, int, string &);
        void computeNodeMass(const data &);
        //void computeIncrementalNodeMass(const data &, string &);
        //void insertInExistingForest(const data &);
        //void writeComputationHistory(const string &);
        //void readComputationHistory(const string &);
        int computeLCA(int, int);
        //void massMatrixComputation(const string &);
        void massMatrixComputation(vector<vector<double>> &);
        //long double computeAvgPathLength(int pointX, const data & origDataset);
        //long double computeAnomalyScore(int pointX, const data & origDataset, int sampleSize);
        double dissScoreComputation(int point1,int point2, int totalPoints,double mu);
        double dissScoreComputationUsingLCAlookup(int point1,int point2, int totalPoints,double mu);
        double dissScoreComputationUsingLCAlookup_runningAvg(int point1,int point2, int totalPoints,double mu);
        
        data * _data;
        int numiTrees;
    	vector<itree*> iTrees;
    	int minPts=0;
    	double mu=0;
        
        
        /*For LargeDS mbscan clustering*/
        //vector<vector<int> markedNodes;
        //vector<vector<int>> potentialNeighborhood;
        //void preprocessingForClustering(int, int,const data & dataObject);
        void markNode(treenode *node, vector<vector<char>> &mNodes, int muPoints,int minPts, int treeId);
        //void markNode(treenode *,int,int,int);
        //void findPotentialNeighbors(const data & dataObject, vector<vector<int>> &potentialNeighbors);
        void findNeighbors(data * dataObject, vector<vector<int>> &potentialNeighbors, double mu,int minPts);
        void computeNeighborhood(vector<vector<int>> &_neighborsList, double mu, int minPts, float treeThold);
        int getminPts(){return this->minPts;}
        double getMu(){return this->mu;}
        //void computeParameters(double mu, int minPts);
        void computeLCAlookup(int maxNodes);
        vector<vector<int>> LCAlookup;
    	
};

#endif // IFOREST_H
