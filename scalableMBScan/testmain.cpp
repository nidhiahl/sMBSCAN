# include <iostream>
# include <string>
# include "data.h"
# include "iforest.h"
# include <string>
//# include <Python.h>
# include <cstdlib>
# include <boost/archive/binary_oarchive.hpp>
# include <boost/archive/binary_iarchive.hpp>
# include <boost/archive/text_oarchive.hpp>
# include <boost/archive/text_iarchive.hpp>
# include <boost/serialization/vector.hpp>
# include <boost/serialization/map.hpp>

using namespace std;
int main(int argc, char* argv[])
{	
	/*char filename[] = "f-measure.py";
	FILE* fp;

	Py_Initialize();

	fp = _Py_fopen(filename, "r");
	PyRun_SimpleFile(fp, filename);

	Py_Finalize();*/
	
	
int result = system("/usr/bin/python3 f-measure.py Clusteringresult.csv");
cout << result; 
  
    return 0;
}

