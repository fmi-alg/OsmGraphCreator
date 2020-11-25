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
		if (inFile.eof()) {
			break;
		}
		std::getline(inFile, typeId);
		if (inFile.eof()) {
			break;
		}
		std::getline(inFile, weight);
		if (value.size() == 0) {
			std::cout << "Empty value in config" << std::endl;
			return false;
		}
		if (typeId.size() == 0) {
			std::cout << "Empty typeId in config" << std::endl;
			return false;
		}
		if (weight.size() == 0) {
			std::cout << "Empty weight in config" << std::endl;
			return false;
		}
		int id = atoi(typeId.c_str());
		cfg.hwTagIds[value] = id;
		cfg.typeToWeight[id] = 360.0 / atof(weight.c_str()); // 100 / ( (w*1000)/3600 )
	}
	return true;
}

void help() {
	std::cout << "USAGE: -g (topotext|fmitext|fmibinary|fmimaxspeedtext|fmimaxspeedbinary|sserializeoffsetarray|sserializelargeoffsetarray|plot) -t (none|distance|time|maxspeed) -dm <number> -tm <number> -c <config> -o <outfile> <infiles>" << std::endl;
	std::cout << "where \n"
	"-g selects the output type\n"
	"\tfmi(maxspeed)(text|binary) is specified by https://theogit.fmi.uni-stuttgart.de/hartmafk/fmigraph/wikis/types \n"
	"\ttopotext only has the topology. Format is obvious.\n"
	"\tplot can be used to plot the graph with gnuplot\n"
	"-t selects the cost function of edges\n"
	"\tdistance calculates the distance in [m/<-dm>] \n"
	"\ttime calculates travel time based on edge type in [s/<-tm>]\n"
	"\tmaxspeed calculates travel time based on maxspeed tag and edge type in [s/<-tm>]\n"
	"-c path to to config (see sample configs) \n"
	"-s sort edges according to source and target \n"
	"-cc split graph into connected components \n"
	"-hs NUM use a direct hashing scheme with NUM entries for the osmid->nodeid hash. Set to auto for auto-size.\n"
	"-b \"minlat maxlat minlon maxlon\" \n"
	"-dm specifies the distance multiplier. For 1000 the distance is in mm. Default 1\n"
	"-tm specifies the time multiplier. For 1000 the time is in ms. Default 100\n"
	"--no-reverse-edge" << std::endl;
}


int main(int argc, char ** argv) {
	if (argc < 4) {
		help();
		return -1;
	}

	std::string configFileName;
	std::vector<std::string> inputFileNames;
	std::string outFileName;
	StatePtr state(new State());
	
	
	for(int i = 1; i < argc;++i) {
		std::string token(argv[i]);
		
		if (token == "-h" || token == "--help") {
			help();
			return 0;
		}
		else if (token == "-t" && i+1 < argc) {
			std::string wcS(argv[i+1]);
			if  (wcS ==  "none") {
				state->cmd.wcType = WC_NONE;
			}
			else if (wcS == "distance") {
				state->cmd.wcType = WC_DISTANCE;
			}
			else if (wcS == "time") {
				state->cmd.wcType = WC_TIME;
			}
			else if (wcS == "maxspeed") {
				state->cmd.wcType = WC_MAXSPEED;
			}
			else {
				std::cerr << "Invalid weight calculator" << std::endl;
				return -1;
			}
			++i;
		}
		else if (token == "-s") {
			state->cmd.sortedEdges = true;
		}
		else if (token == "-cc") {
			state->cmd.connectedComponents = true;
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
			if (gtS == "topotext") {
				state->cmd.graphType = GT_TOPO_TEXT;
			}
			else if  (gtS ==  "fmitext") {
				state->cmd.graphType = GT_FMI_TEXT;
			}
			else if (gtS == "fmibinary") {
				state->cmd.graphType = GT_FMI_BINARY;
			}
			else if (gtS == "fmimaxspeedtext") {
				state->cmd.graphType = GT_FMI_MAXSPEED_TEXT;
			}
			else if (gtS == "fmimaxspeedbinary") {
				state->cmd.graphType = GT_FMI_MAXSPEED_BINARY;
			}
			else if (gtS == "sserializeoffsetarray") {
				state->cmd.graphType = GT_SSERIALIZE_OFFSET_ARRAY;
			}
			else if (gtS == "sserializelargeoffsetarray") {
				state->cmd.graphType = GT_SSERIALIZE_LARGE_OFFSET_ARRAY;
			}
			else if (gtS == "plot") {
				state->cmd.graphType = GT_PLOT;
			}
			else {
				std::cerr << "Invalid graph type" << std::endl;
				return -1;
			}
			++i;
		}
		else if (token == "-hs" && i+1 < argc) {
			std::string v(argv[i+1]);
			if (v == "auto") {
				state->cmd.hugheHashMapPopulate = 0;
			}
			else {
				state->cmd.hugheHashMapPopulate = atol(v.c_str());
			}
			++i;
		}
		else if (token == "-dm" && i+1 < argc) {
			state->cmd.distanceMult = atof(argv[i+1]);
			++i;
		}
		else if (token == "-tm" && i+1 < argc) {
			state->cmd.timeMult = atof(argv[i+1]);
			++i;
		}
		else if (token == "-b" && i+1 < argc) {
			std::string v(argv[i+1]);
			state->cmd.bounds = sserialize::spatial::GeoRect(v);
			if (state->cmd.bounds.valid()) {
				state->cmd.withBounds = true;
			}
			else {
				std::cerr << "Could not read bounds specification" << std::endl;
				return -1;
			}
			++i;
		}
		else if (token == "--no-reverse-edge") {
			state->cmd.addReverseEdges = false;
		}
		else {
			inputFileNames.emplace_back(argv[i]);
		}
	}

	osmpbf::PbiStream inFile;
	try {
		inFile = osmpbf::PbiStream(inputFileNames);
	}
	catch(std::exception const & e) {
		std::cerr << e.what() << std::endl;
		return -1;
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
	
	auto graphWriterFactory = [&](std::string const & outFileName) {
		std::shared_ptr< GraphWriter > graphWriter;
		std::shared_ptr<std::ofstream> outFile;
		if (state->cmd.graphType != GT_SSERIALIZE_OFFSET_ARRAY && state->cmd.graphType != GT_SSERIALIZE_LARGE_OFFSET_ARRAY) {
			outFile = std::make_shared<std::ofstream>(outFileName);
			if (!outFile->is_open()) {
				throw std::runtime_error("Failed to open out file " + outFileName);
			}
			*outFile << std::fixed << std::setprecision(std::numeric_limits<double>::digits10 + 2);
		}
		switch (state->cmd.graphType) {
		case GT_TOPO_TEXT:
			graphWriter.reset(new TopologyTextGraphWriter(outFile));
			break;
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
		case GT_NONE:
			graphWriter.reset(new DropGraphWriter());
		};
		if (state->cmd.sortedEdges) {
			graphWriter.reset(new SortedEdgeWriter(graphWriter));
		}
		return graphWriter;
	};
	
	
	std::shared_ptr< GraphWriter > graphWriter;
	if (state->cmd.connectedComponents) {
		graphWriter.reset(
			new CCGraphWriter(
				[&](uint32_t ccId){
					return graphWriterFactory(outFileName + std::to_string(ccId) + ".cc");
				}
			)
		);
	}
	else {
		try {
			graphWriter = graphWriterFactory(outFileName);
		}
		catch (std::exception const & e) {
			std::cerr << "Error occured: " << e.what() << std::endl;
			return -1;
		}
	}

	{
		//Now get all nodeRefs we need, store node usage count in state->osmIdToMyNodeId
		{
			if (state->cmd.hugheHashMapPopulate >= 0) {
				inFile.dataSeek(0);
				MinMaxNodeIdProcessor minMaxNodeIdProcessor;
				WayParser wayParser("Calculating min/max node id for direct hash map", inFile, state->cfg.hwTagIds);
				wayParser.parse(minMaxNodeIdProcessor);
				minMaxNodeIdProcessor.largestId.update(0);
				minMaxNodeIdProcessor.smallestId.update(minMaxNodeIdProcessor.largestId.value());
				int64_t largestId = minMaxNodeIdProcessor.largestId.value();
				int64_t smallestId = minMaxNodeIdProcessor.smallestId.value();
				std::cout << "Min nodeId=" << smallestId << "\nMax nodeId=" << largestId << "\n";
				if (state->cmd.hugheHashMapPopulate > 0) {
					largestId= std::min<uint64_t>(smallestId+state->cmd.hugheHashMapPopulate, largestId);
				}
				std::cout << "DirectRange=" << smallestId << ":" << largestId << std::endl;
				state->osmIdToMyNodeId = State::OsmIdToMyNodeIdHashMap(minMaxNodeIdProcessor.smallestId.value(), largestId, sserialize::MM_SHARED_MEMORY);
			}
		
			inFile.dataSeek(0);
			AllNodesGatherProcessor allNodesGatherProcessor(state);
			WayParser wayParser("Collecting candidate node refs", inFile, state->cfg.hwTagIds);
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
		WayParser wayParser("Collecting needed node refs", inFile, state->cfg.hwTagIds);
		wayParser.parse(refGatherProcessor);
		
		//Really fetch the nodes
		state->nodes.reserve(state->osmIdToMyNodeId.size());
		gatherNodes(inFile, state);
	}
	
	if (state->cmd.graphType == GT_SSERIALIZE_OFFSET_ARRAY || state->cmd.graphType == GT_SSERIALIZE_LARGE_OFFSET_ARRAY) {
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
		switch (state->cmd.wcType) {
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

	return 0;
}
