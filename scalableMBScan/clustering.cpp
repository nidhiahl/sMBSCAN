#include <iostream>
#include <string>
# include "data.h"
//# include "iforest.h"
#include "dbscan.h"
#include <string>
/*#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
*/
using namespace std;

int main(int argc, char* argv[])
{
	const string &dataFile = argv[1];
	
	
	dbscan * dbscanObject = new dbscan();
	data * dataObject = new data();
	dataObject->Run(dataFile);
	const data &refDataObject = *dataObject;
	dbscanObject->Run(refDataObject, 1, 31);
	
	auto clusters = dbscanObject->clusters;
	cout<< "execution complete"<<endl;
	std::string name;
    //std::cout << "filename:";
    std::cin >> name;
    std::ofstream fileprint;
    fileprint.open(name);
    int ctr = 0;
    //cout<<"size of clusters"<<clusters.size()<<endl;
    fileprint<<"x y class\n";
    for (auto y : clusters)
    {
        //std::cout << "here";
		//cout<<"Size of y"<< y.size()<<endl;
        // ofstream myfile;
        // myfile.open("example.txt");
        // myfile << "Writing this to a file.\n";
        // myfile.close();
        
        for(auto x : y)
        {
        	for (int a = 0; a < refDataObject.getnumAttributes(); a++)
    		{
    			fileprint << refDataObject.dataVector[x]->attributes[a]<<" ";
    		
    		}
    		fileprint << ctr<<"\n";
        }
        ctr++;
        //std::cout << "\n";
    }
	
	delete dbscanObject;
	delete dataObject;

return 0;

}

