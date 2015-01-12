#include <iostream>
#include <fstream>
#include <limits>
#include "Processors.h"
#include "RamGraph.h"

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
		if (value.size() == 0) {
			std::cout << "Empty value in confi" << std::endl;
			return false;
		}
		if (typeId.size() == 0) {
			std::cout << "Empty typeId in confi" << std::endl;
			return false;
		}
		if (weight.size() == 0) {
			std::cout << "Empty weight in confi" << std::endl;
			return false;
		}
		int id = atoi(typeId.c_str());
		cfg.hwTagIds[value] = id;
		cfg.typeToWeight[id] = 360.0 / atof(weight.c_str()); // 100 / ( (w*1000)/3600 )
	}
	return true;
}

bool findNodeIdBounds(osmpbf::OSMFileIn & inFile, uint64_t & smallestId, uint64_t & largestId) {
	osmpbf::PrimitiveBlockInputAdaptor pbi;

	int64_t tempLargestId = std::numeric_limits<int64_t>::min();
	int64_t tempSmallestId = std::numeric_limits<int64_t>::max();
	
	{
		while (inFile.parseNextBlock(pbi)) {
			if (pbi.isNull())
				continue;
			if (pbi.waysSize()) {
				for (osmpbf::IWayStream way = pbi.getWayStream(); !way.isNull(); way.next()) {
					for(osmpbf::IWayStream::RefIterator it(way.refBegin()), end(way.refEnd()); it != end; ++it) {
						int64_t refId = *it;
						tempLargestId = std::max<int64_t>(tempLargestId, refId);
						tempSmallestId = std::min<int64_t>(tempSmallestId, refId);
					}
				}
			}
		}
	}
	
	if (tempLargestId >= 0 && tempSmallestId >= 0) {
		largestId = tempLargestId;
		smallestId = tempSmallestId;
		return true;
	}
	return false;
}

void help() {
	std::cout << "USAGE: -g (fmitext|fmibinary|fmimaxspeedtext|fmimaxspeedbinary|sserializeoffsetarray|sserializelargeoffsetarray|plot) -t (none|distance|time|maxspeed) -c <config> -o <outfile> <infile>" << std::endl;
	std::cout << "<where \n \
	-g selects the output type. fmi(text|binary) is specified by https://theogit.fmi.uni-stuttgart.de/hartmafk/fmigraph/wikis/types \n \
	-t select the cost function of edges, maxspeed according to tag if specified, otherwise as defined in the config \n \
	-c path to to config (see sample configs)" << std::endl;
}


int main(int argc, char ** argv) {
	if (argc < 4) {
		help();
		return -1;
	}

	std::string configFileName;
	std::string inputFileName;
	std::string outFileName;
	StatePtr state(new State());
	WeightCalculatorType wcType = WC_DISTANCE;
	GraphType graphType = GT_NONE;
	int64_t hugheHashMapPopulate = -1;
	
	
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
			else if (gtS == "sserializeoffsetarray") {
				graphType = GT_SSERIALIZE_OFFSET_ARRAY;
			}
			else if (gtS == "sserializelargeoffsetarray") {
				graphType = GT_SSERIALIZE_LARGE_OFFSET_ARRAY;
			}
			else if (gtS == "plot") {
				graphType = GT_PLOT;
			}
			else {
				std::cerr << "Invalid graph type" << std::endl;
				return -1;
			}
			++i;
		}
		else if (token == "-hs" && i+1 < argc) {
			std::string v(argv[i+1]);
			if (v == "auto")
				hugheHashMapPopulate = 0;
			else
				hugheHashMapPopulate = atol(v.c_str());
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
	if (graphType != GT_SSERIALIZE_OFFSET_ARRAY && graphType != GT_SSERIALIZE_LARGE_OFFSET_ARRAY) {
		outFile.open(outFileName);
		if (!outFile.is_open()) {
			std::cout << "Failed to open out file " << outFileName << std::endl;
			return -1;
		}
		outFile << std::fixed << std::setprecision(std::numeric_limits<double>::digits10 + 2);
	}
	
	if (!readConfig(configFileName, state->cfg)) {
		std::cout << "Failed to read config" << std::endl;
		return -1;
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
	case GT_SSERIALIZE_OFFSET_ARRAY:
		graphWriter.reset( new RamGraphWriter( sserialize::UByteArrayAdapter::createFile(0, outFileName) ) );
		break;
	case GT_SSERIALIZE_LARGE_OFFSET_ARRAY:
		graphWriter.reset( new StaticGraphWriter( sserialize::UByteArrayAdapter::createFile(0, outFileName) ) );
		break;
	case GT_PLOT:
		graphWriter.reset( new PlotGraph(outFile) );
		break;
	case GT_FMI_TEXT:
		graphWriter.reset(new FmiTextGraphWriter(outFile));
		break;
	default:
		std::cout << "Unsuported graph format" << std::endl;
		return -1;
	};

	{
		//Now get all nodeRefs we need, store node usage count in state->osmIdToMyNodeId
		{
			if (hugheHashMapPopulate >= 0) {
				std::cout << "Find node id bounds" << std::endl;
				uint64_t smallestId, largestId = 0;
				inFile.dataSeek(0);
				findNodeIdBounds(inFile, smallestId, largestId);
				if (hugheHashMapPopulate != 0) {
					largestId = std::min<uint64_t>(smallestId+hugheHashMapPopulate, largestId);
				}
				state->osmIdToMyNodeId = State::OsmIdToMyNodeIdHashMap(smallestId, largestId, sserialize::MM_SHARED_MEMORY);
			}
		
			inFile.dataSeek(0);
			AllNodesGatherProcessor allNodesGatherProcessor(state);
			WayParser wayParser("Gathering all nodeRefs", inFile, state->cfg.hwTagIds);
			wayParser.parse(allNodesGatherProcessor);
		}
		
		deleteAvailableNodes(inFile, state);
		//state->osmIdToMyNodeId now only contains nodes that could not be retrieved
		if (state->osmIdToMyNodeId.size()) { //check if we have to mark some ways invalid
			inFile.dataSeek(0);
			InvalidWayMarkingProcessor iwmP(state);
			WayParser wayParser("Marking invalid ways", inFile, state->cfg.hwTagIds);
			wayParser.parse(iwmP);
		}
		state->osmIdToMyNodeId.clear();
		
		assert(state->edgeCount == 0);
		assert(state->osmIdToMyNodeId.size() == 0);
		
		//Rebuild nodeId hash, but this time invalid ways are taken into account
		inFile.dataSeek(0);
		NodeRefGatherProcessor refGatherProcessor(state);
		WayParser wayParser("Regathering nodeRefs", inFile, state->cfg.hwTagIds);
		wayParser.parse(refGatherProcessor);
		
		//Really fetch the nodes
		state->nodes.reserve(state->osmIdToMyNodeId.size());
		gatherNodes(inFile, state);
	}
	
	if (graphType == GT_SSERIALIZE_OFFSET_ARRAY || graphType == GT_SSERIALIZE_LARGE_OFFSET_ARRAY) {
		NodeDegreeProcessor nodeDegreeProcessor(state);
		inFile.dataSeek(0);
		WayParser wayParser("Adding node degree information", inFile, state->cfg.hwTagIds);
		wayParser.parse(nodeDegreeProcessor);
	}
	
	std::cout << "Graph has " << state->nodes.size() << " nodes and " << state->edgeCount << " edges." << std::endl;
	
	{//write the nodes out
		state->nodeCoordinates.reserve(state->nodes.size());
		graphWriter->beginGraph();
		graphWriter->beginHeader();
		graphWriter->writeHeader(state->nodes.size(), state->edgeCount);
		graphWriter->endHeader();
		graphWriter->beginNodes();
		sserialize::ProgressInfo info;
		info.begin(state->nodes.size(), "Writing out nodes");
		for(std::size_t i = 0, s = state->nodes.size(); i < s; ++i) {
			graphWriter->writeNode(state->nodes[i], state->nodeCoordinates[i]);
		}
		info.end();
		graphWriter->endNodes();
		state->nodes = std::vector<Node>();
	}

	{
		std::shared_ptr< WeightCalculator > weightCalculator;
		switch (wcType) {
		case WC_NONE:
			weightCalculator.reset(new NoWeightCalculator());
			break;
		case WC_TIME:
			weightCalculator.reset(new WeightedGeodesicDistanceWeightCalculator(state));
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
		inFile.dataSeek(0);
		WayParser wayParser("Processing ways", inFile, state->cfg.hwTagIds);
		graphWriter->beginEdges();
		wayParser.parse(finalWayProcessor);
		graphWriter->endEdges();
	}
	graphWriter->endGraph();

	inFile.close();

	return 0;
}