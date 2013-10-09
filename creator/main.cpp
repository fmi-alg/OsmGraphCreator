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
					for(int i = 0, s = way.refsSize(); i < s; ++i) {
						tempLargestId = std::max<int64_t>(tempLargestId, way.ref(i));
						tempSmallestId = std::min<int64_t>(tempSmallestId, way.ref(i));
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


int main(int argc, char ** argv) {
	if (argc < 4) {
		std::cout << "prog -g (fmitext|fmibinary|fmimaxspeedtext|fmimaxspeedbinary|sserializeoffsetarray) -t (none|distance|time|maxspeed) -c <config> -o <outfile> <infile>" << std::endl;
		return -1;
	}

	std::string configFileName;
	std::string inputFileName;
	std::string outFileName;
	StatePtr state(new State());
	WeightCalculatorType wcType = WC_DISTANCE;
	GraphType graphType = GT_FMI_TEXT;
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
	if (graphType != GT_SSERIALIZE_OFFSET_ARRAY) {
		outFile.open(outFileName);
		if (!outFile.is_open()) {
			std::cout << "Failed to open out file " << outFileName << std::endl;
			return -1;
		}
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
	case GT_SSERIALIZE_OFFSET_ARRAY:
		graphWriter.reset(new RamGraphWriter());
		break;
	case GT_PLOT:
		graphWriter.reset( new PlotGraph(outFile) );
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
			if (hugheHashMapPopulate >= 0) {
				std::cout << "Find node id bounds" << std::endl;
				uint64_t smallestId, largestId = 0;
				inFile.dataSeek(0);
				findNodeIdBounds(inFile, smallestId, largestId);
				if (hugheHashMapPopulate != 0)
					largestId = std::min<uint64_t>(smallestId+hugheHashMapPopulate, largestId);
				state->osmIdToMyNodeId = sserialize::DirectHugheHash<uint32_t>(smallestId, largestId, true);
			}
		
			inFile.dataSeek(0);
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
	
	if (graphType == GT_SSERIALIZE_OFFSET_ARRAY) {
		NodeDegreeProcessor nodeDegreeProcessor(state);
		inFile.dataSeek(0);
		WayParser wayParser("Processing ways", inFile, state->cfg.hwTagIds);
		wayParser.parse(nodeDegreeProcessor);
	}
	
	{//write the nodes out
		graphWriter->writeHeader(state->nodes.size(), edgeCount);
		graphWriter->writeNodes(state->nodes.begin(), state->nodes.end());
	}

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
		inFile.dataSeek(0);
		WayParser wayParser("Processing ways", inFile, state->cfg.hwTagIds);
		wayParser.parse(finalWayProcessor);
	}
	
	if (graphType == osm::graphtools::creator::GT_SSERIALIZE_OFFSET_ARRAY) {
		sserialize::UByteArrayAdapter outFile( sserialize::UByteArrayAdapter::createFile(100*state->nodes.size()+ sserialize::SerializationInfo<osm::graphs::ram::Edge>::length*edgeCount, outFileName) );
		static_cast<RamGraphWriter*>( graphWriter.get() )->graph().serialize(outFile);
		if (outFile.tellPutPtr() < outFile.size()) {
			outFile.shrinkStorage( outFile.size() - outFile.tellPutPtr() );
		}
	}

	inFile.close();

	return 0;
}