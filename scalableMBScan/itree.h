#ifndef ITREE_H
#define ITREE_H
#include "data.h"
#include "treenode.h"
//#include <boost/serialization/list.hpp>
#include <vector>

/*typedef struct pToN{
    //double* attributes;
    bool isPresent;
    int coresspondingNode;
}pToN*/

class itree
{
    public:
        itree();
        virtual ~itree();
        void createStaticTree(const data &,int);
        //void insertNewPointsIniTree(const data &, int);
        //void deletePointsFromiTree(const data &);
        //void freezeSubtree(treenode *);
        //void deleteSubTree(treenode *, const data &);
        void computeNodeMassforTree(const data &);
        //void computeNodeMassAfterInsertionInTree(const data &);
        int getMaxNumOfNodes();
        //long double computePathLength(int pointX, const data & origDataset);
        //long double pathLengthEstimationForUnbuiltTree(int nodeSize);
        //double insertInExistingTree(const data &);
        //void isNodePotentialtoChange(int *, treenode *, const data &);
        //void clearaddedPointIndexForEachNode(int *, treenode *);
        vector<treenode*> treeNode;
        vector<int> pointToNode;
        
        /*Mark Nodes for clustering large DS*/
        //vector<int> markedNodes;
	int smallestLeafSize;
	int largestLeafSize;

    protected:

    private:
    int maxTreeHeight;
    int maxNumOfNodes;

};
#endif // ITREE_H
