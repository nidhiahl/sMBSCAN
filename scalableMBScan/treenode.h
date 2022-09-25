#ifndef TREENODE_H
#define TREENODE_H
# include "data.h"
#include <vector>
//#include <boost/serialization/list.hpp>

class treenode
{
    public:

        int nodeId;
        int parentId;
        int lChildId;
        int rChildId;
        treenode *parentAdd;
        treenode *lChildAdd;
        treenode *rChildAdd;
        vector<int> dataPointIndex;
        //vector<int> deletedPointIndex;
        //vector<int> addedPointIndex;
        int splitAttribute;
        double splitValue;
        double minimumVal;
        double maximumVal;
        int nodeMass;
        int nodeHeight;
        bool isLeaf;
        bool isActive;
        //bool isChanged;
        //bool deletionHappened;
        //bool deletionProcessed;
        //bool nodeVanishes;


        treenode();
        treenode(int);
        virtual ~treenode();
        double splitInfoSelection(const data &);
        //double splitInfoSelectionForUpdates(const data &);

    protected:

    private:
        
};

#endif // TREENODE_H
