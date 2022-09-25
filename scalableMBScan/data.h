#ifndef DATA_H
#define DATA_H
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <set>
using namespace std;

typedef struct point{
    //double* attributes;
    double* attributes;
    bool isPresent;
    int label;
}point;

class data
{
    public:
        data();
        virtual ~data();
        void Run(const string &, const string &, const string &, string &);
        void Run(const string &);
        void createDataVector(const string &);
        void removeDeletedPoints(const string &);
        void readCurrentBatch(const string &, string &);
        vector<int> & getSample(int) const ;
        vector<int> & getUpdateSample(int) const ;
        vector<point*>  getDataVector() const ;
        int getnumInstances() const;
        void setnumInstances(int);
        int getnumAttributes() const;
        void setnumAttributes(int);
        int getDeletedPointIndeciesSize() const;
        int getCurrentBatchIndeciesSize() const;
        int getCurrentBatchIndecies(int) const;
        void appendCurrentBatchToExistingData(const string & , const string & , const string & );

        vector<point*> dataVector;
    protected:

    private:
    int numInstances;
    int numAttributes;
    //vector<point*> dataVector;
    vector<int> deletedPointIndecies;
    vector<int> currentBatchIndecies;
};

#endif // DATA_H
