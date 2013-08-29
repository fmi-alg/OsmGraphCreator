#include <iostream>
#include <fstream>
#include "Processors.h"

using namespace osm::graphtools::creator;

//Config file format:
//highway-value
//type-id (integer)
//weight (double) in km/h
bool readConfig(const std::string & fileName, std::unordered_map<std::string, int> & hwTagIds, std::unordered_map<int, double> & typeToWeight) {
	std::ifstream inFile;
	inFile.open(fileName);
	if (!inFile.is_open()) {
		return false;
	}
	while(!inFile.eof()) {
		std::string value;
		std::string typeId;
		std::string weight;
		std::getline(inFile, value);
		std::getline(inFile, typeId);
		std::getline(inFile, weight);
		
		int id = atoi(typeId.c_str());
		hwTagIds[value] = id;
		typeToWeight[id] = 360.0 / atof(weight.c_str()); // 100 / ( (w*1000)/3600 )
	}
	return true;
}


int main(int argc, char ** argv) {
	if (argc < 4) {
		std::cout << "prog -g (fmitext| fmibinary) -t (none|distance|time) -c <config> -o <outfile> <infile>" << std::endl;
		return -1;
	}

	std::string configFileName;
	std::string inputFileName;
	std::string outFileName;
	State state;
	WeightCalculatorType wcType = WC_DISTANCE;
	GraphType graphType = GT_FMI_TEXT;
	
	
	for(int i = 1; i < argc;++i) {
		std::string token(argv[i]);
		if (token == "-t" && i+1 < argc) {
			std::string wcS(argv[i+1]);
			if  (wcS ==  "none") {
				wcType = WC_NONE;
			}
			else if (wcS == "distance") {
				wcType = WC_DISTANCE;
			}
			else if (wcS == "time") {
				wcType = WC_TIME;
			}
			else {
				std::cerr << "Invalid weight calculator" << std::endl;
				return -1;
			}
			++i;
		}
		else if (token == "-c" && i+1 < argc) {
			configFileName = std::string(argv[i+1]);
			++i;
		}
		else if (token == "-o" && i+1 < argc) {
			outFileName = std::string(argv[i+1]);
			++i;
		}
		else if (token == "-g" && i+1 < argc) {
			std::string gtS(argv[i+1]);
			if  (gtS ==  "fmitext") {
				graphType = GT_FMI_TEXT;
			}
			else if (gtS == "fmibinary") {
				graphType = GT_FMI_BINARY;
			}
			else {
				std::cerr << "Invalid graph type" << std::endl;
				return -1;
			}
			++i;
		}
		else {
			inputFileName = std::string(argv[i]);
		}
	}
	

	osmpbf::OSMFileIn inFile(inputFileName, false);

	if (!inFile.open()) {
		std::cout << "Failed to open " <<  inputFileName << std::endl;
		return -1;
	}
	
	std::ofstream outFile;
	outFile.open(outFileName);
	if (!outFile.is_open()) {
		std::cout << "Failed to open out file " << outFileName << std::endl;
		return -1;
	}
	
	if (!readConfig(configFileName, *state.hwTagIds, *state.typeToWeight)) {
		std::cout << "Failed to read config" << std::endl;
	}
	else {
		std::cout << "Found " << state.hwTagIds->size() << " config entries" << std::endl;
	}
	
	if (state.hwTagIds->count("motorway")) {
		state.implicitOneWay->insert(state.hwTagIds->at("motorway"));
	}
	if (state.hwTagIds->count("motorway_link")) {
		state.implicitOneWay->insert(state.hwTagIds->at("motorway_link"));
	}
	
	uint64_t edgeCount = 0;
	std::shared_ptr< GraphWriter > graphWriter;
	switch (graphType) {
	case GT_FMI_BINARY:
		graphWriter.reset(new FmiBinaryGraphWriter(outFile));
		break;
	case GT_FMI_TEXT:
	default:
		graphWriter.reset(new FmiTextGraphWriter(outFile));
		break;
	};

	{
		std::shared_ptr< std::unordered_set<int64_t> > nodeRefs(new std::unordered_set<int64_t>());

		//Now get all nodeRefs we need
		{
			RefGatherProcessor refGatherProcessor(nodeRefs, state.implicitOneWay, &edgeCount);
			WayParser wayParser("Gathering nodeRefs", inFile, *state.hwTagIds);
			wayParser.parse(refGatherProcessor);
		}
		state.nodes->reserve(nodeRefs->size());
		gatherNodes(inFile, nodeRefs, state.nodes, state.osmIdToMyNodeId);
		

		if (nodeRefs->size()) { //check if we have to mark some ways invalid
			inFile.dataSeek(0);
			InvalidWayMarkingProcessor iwmP(nodeRefs, state.invalidWays, state.implicitOneWay, &edgeCount);
			WayParser wayParser("Marking invalid ways", inFile, *state.hwTagIds);
			wayParser.parse(iwmP);
		}
	}
	{//write the nodes out
		graphWriter->writeHeader(state.nodes->size(), edgeCount);
		graphWriter->writeNodes(state.nodes->begin(), state.nodes->end());
	}

	inFile.dataSeek(0);
	{
		std::shared_ptr< WeightCalculator > weightCalculator;
		switch (wcType) {
		case WC_NONE:
			weightCalculator.reset(new NoWeightCalculator());
			break;
		case WC_TIME:
			weightCalculator.reset(new GeodesicDistanceWeightCalculator());
			break;
		case WC_DISTANCE:
		default:
			weightCalculator.reset(new GeodesicDistanceWeightCalculator());
		};
		FinalWayProcessor finalWayProcessor(state.osmIdToMyNodeId, state.invalidWays, state.implicitOneWay, graphWriter, state.nodes, weightCalculator);
		WayParser wayParser("Processing ways", inFile, *state.hwTagIds);
		wayParser.parse(finalWayProcessor);
	}

	inFile.close();

	return 0;
}