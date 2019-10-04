#include "config.h"

#include <fstream>
#include <iostream>

//glapp::Config glapp::Config::config;

glapp::Config::Config(void)
{
}


glapp::Config::~Config(void)
{
}


glapp::Config::Config(const std::string& filename)
{
	Reset(filename);
}


void glapp::Config::Reset(const std::string& filename)
{
	boost::property_tree::read_xml(filename, _tree);
	//if(){
	//	std::cout << "ERROR Config File Read :\t" << filename.c_str() << std::endl;
	//}
}
