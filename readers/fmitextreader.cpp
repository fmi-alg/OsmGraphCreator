#include "fmitextreader.h"
#include <stdexcept>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string>
#include <sys/stat.h>
#include <iostream>
#include <fstream>

namespace OsmGraphWriter {

FmiTextReader::FmiTextReader() {}
FmiTextReader::~FmiTextReader() {}

void FmiTextReader::read(char * path) {
	std::ifstream inFile;
	inFile.open(path);
	if (!inFile.is_open()) {
		throw std::runtime_error("Could not open file");
	}
	readGraph(inFile);
}

void FmiTextReader::readGraph(std::istream & input) {
	GraphType gt = GT_UNDEFINED;
	//skip text header
	for(uint8_t nlc = 0; !input.eof() && nlc < 5; ++nlc) {
		std::string str;
		std::getline(input, str);
		if (str.find("standard") != std::string::npos) {
			gt = GT_STANDARD;
		}
		else if (str.find("maxspeed") != std::string::npos) {
			gt = GT_MAXSPEED;
		}
	}
	if (gt == GT_UNDEFINED) {
		throw std::runtime_error("Could not detect graph type");
	}
	
	//begin now points to the first "real" data
	
	//header
	int32_t nodeCount = 0;
	int32_t edgeCount = 0;
	input >> nodeCount >> edgeCount;
	header(gt, nodeCount, edgeCount);
	
	{//nodes
		int32_t nodeId;
		int64_t osmId;
		double lat, lon;
		int32_t elev;
		int32_t i(0);
		for(; i < nodeCount && !input.eof(); ++i) {
			input >> nodeId >> osmId >> lat >> lon >> elev;
			node(nodeId, osmId, lat, lon, elev, 0, 0);
		}
		if (i < nodeCount) {
			throw std::runtime_error("Not enough nodes");
			return;
		}
	}
	{//edges
		int32_t source;
		int32_t target;
		int32_t weight;
		int32_t type;
		int32_t maxSpeed = 0;
		int32_t i(0);
		for(; i < edgeCount && !input.eof(); ++i) {
			input >> source >> target >> weight >> type;
			if (gt == GT_MAXSPEED) {
				input >> maxSpeed;
			}
			edge(source, target, weight, type, maxSpeed, 0, 0);
		}
		if (i < edgeCount) {
			throw std::runtime_error("Not enough edges");
			return;
		}
	}
}

}//end namespace
