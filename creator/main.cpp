#include <iostream>
#include <fstream>
#include "Processors.h"

using namespace osm::graphtools::creator;

//Config file format:
//highway-value
//type-id (integer)
//weight (double) in km/h
bool readConfig(const std::string & fileName, State::Configuration & cfg) {
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
		cfg.hwTagIds[value] = id;
		cfg.typeToWeight[id] = 360.0 / atof(weight.c_str()); // 100 / ( (w*1000)/3600 )
	}
	return true;
}


int main(int argc, char ** argv) {
	if (argc < 4) {
		std::cout << "prog -g (fmitext|fmibinary|fmimaxspeedtext|fmimaxspeedbinary) -t (none|distance|time|maxspeed) -c <config> -o <outfile> <infile>" << std::endl;
		return -1;
	}

	std::string configFileName;
	std::string inputFileName;
	std::string outFileName;
	StatePtr state(new State());
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
			else if (wcS == "maxspeed") {
				wcType = WC_MAXSPEED;
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
			else if (gtS == "fmimaxspeedtext") {
				graphType = GT_FMI_MAXSPEED_TEXT;
			}
			else if (gtS == "fmimaxspeedbinary") {
				graphType = GT_FMI_MAXSPEED_BINARY;
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
	
	if (!readConfig(configFileName, state->cfg)) {
		std::cout << "Failed to read config" << std::endl;
	}
	else {
		std::cout << "Found " << state->cfg.hwTagIds.size() << " config entries" << std::endl;
	}
	
	if (state->cfg.hwTagIds.count("motorway")) {
		state->cfg.implicitOneWay.insert(state->cfg.hwTagIds.at("motorway"));
	}
	if (state->cfg.hwTagIds.count("motorway_link")) {
		state->cfg.implicitOneWay.insert(state->cfg.hwTagIds.at("motorway_link"));
	}
	
	uint64_t edgeCount = 0;
	std::shared_ptr< GraphWriter > graphWriter;
	switch (graphType) {
	case GT_FMI_BINARY:
		graphWriter.reset(new FmiBinaryGraphWriter(outFile));
		break;
	case GT_FMI_MAXSPEED_BINARY:
		graphWriter.reset(new FmiMaxSpeedBinaryGraphWriter(outFile));
		break;
	case GT_FMI_MAXSPEED_TEXT:
		graphWriter.reset(new FmiMaxSpeedTextGraphWriter(outFile));
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
			RefGatherProcessor refGatherProcessor(state, nodeRefs, &edgeCount);
			WayParser wayParser("Gathering nodeRefs", inFile, state->cfg.hwTagIds);
			wayParser.parse(refGatherProcessor);
		}
		state->nodes.reserve(nodeRefs->size());
		gatherNodes(inFile, *nodeRefs, state);
		

		if (nodeRefs->size()) { //check if we have to mark some ways invalid
			inFile.dataSeek(0);
			InvalidWayMarkingProcessor iwmP(state, nodeRefs, &edgeCount);
			WayParser wayParser("Marking invalid ways", inFile, state->cfg.hwTagIds);
			wayParser.parse(iwmP);
		}
	}
	{//write the nodes out
		graphWriter->writeHeader(state->nodes.size(), edgeCount);
		graphWriter->writeNodes(state->nodes.begin(), state->nodes.end());
	}

	inFile.dataSeek(0);
	{
		std::shared_ptr< WeightCalculator > weightCalculator;
		switch (wcType) {
		case WC_NONE:
			weightCalculator.reset(new NoWeightCalculator());
			break;
		case WC_TIME:
			weightCalculator.reset(new GeodesicDistanceWeightCalculator(state));
			break;
		case WC_MAXSPEED:
			weightCalculator.reset(new MaxSpeedGeodesicDistanceWeightCalculator(state));
			break;
		case WC_DISTANCE:
		default:
			weightCalculator.reset(new GeodesicDistanceWeightCalculator(state));
			break;
		};
		FinalWayProcessor finalWayProcessor(state, graphWriter, weightCalculator);
		WayParser wayParser("Processing ways", inFile, state->cfg.hwTagIds);
		wayParser.parse(finalWayProcessor);
	}

	inFile.close();

	return 0;
}