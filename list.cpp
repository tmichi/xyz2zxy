#include <vector>
#include <cstdlib>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

int main(int argc, char** argv){
	std::stringstream ss;
	ss<<"ls -1 "<<std::string(argv[1])<<"* > file.txt";
	std::system(ss.str().c_str());
	std::vector<std::string> files;
	std::ifstream fin("file.txt");	
	while(!fin.eof()){
		std::string f;
		fin>>f;
		files.push_back(f);
		std::cerr<<f<<std::endl;

	}
	return 0;

}
