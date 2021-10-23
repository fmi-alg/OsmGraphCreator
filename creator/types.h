#ifndef OSM_GRAPH_TOOLS_TYPES_H
#define OSM_GRAPH_TOOLS_TYPES_H
#include <memory>
#include <string>
#include <stdint.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <sserialize/containers/DirectHugeHash.h>
#include <sserialize/spatial/GeoRect.h>

namespace osm {
namespace graphtools {
namespace creator {

enum OneWayStatus {OW_YES, OW_NO, OW_IMPLICIT};
enum WeightCalculatorType {WC_NONE, WC_DISTANCE, WC_TIME, WC_MAXSPEED};
enum GraphType {GT_NONE, GT_TOPO_TEXT, GT_FMI_TEXT, GT_FMI_BINARY, GT_FMI_MAXSPEED_BINARY, GT_FMI_MAXSPEED_TEXT, GT_SSERIALIZE_OFFSET_ARRAY, GT_PLOT, GT_SSERIALIZE_LARGE_OFFSET_ARRAY};

enum class FilterMode {
	// Write topk components ordered by their size
	// Note that this may write more than k components if there are ties
	TopK,
	MinSize,
	All
};

struct Coordinates {
	Coordinates() {}
	Coordinates(double lat, double lon) : lat(lat), lon(lon) {}
	double lat;
	double lon;
};

//[Id] [osmId] [lat] [lon] [elevation] [carryover] //Knoten
struct Node {
	Node() {}
	Node(uint32_t id, int64_t osmId, uint16_t elev) :
	id(id), osmId(osmId), elev(elev)
	{}
	uint32_t id{std::numeric_limits<uint32_t>::max()};
	int64_t osmId{std::numeric_limits<int64_t>::min()};
	uint16_t elev{0};
#ifdef CONFIG_SUPPORT_SSERIALIZE_OFFSET_ARRAY_TARGET
	uint16_t indegree{0};
	uint16_t outdegree{0};
#endif
#ifdef CONFIG_SUPPORT_STRING_CARRY_OVER
	uint16_t stringCarryOverSize{0};
	char * stringCarryOverData{nullptr}; //not zero-terminated
#endif
};

//[source][target][weight][type][sizecarryover][carryover] //kante
///@member maxspeed this is always in km/h
///@meber weight this is usualy either the length of the arc or the travel time
struct Edge {
	Edge() {}
	Edge(uint32_t source, uint32_t target, int32_t weight, int32_t type, int32_t maxspeed) :
	source(source), target(target), weight(weight), type(type), maxspeed(maxspeed)
	{}
	Edge & reverse() {
		std::swap(source, target);
		return *this;
	}
	uint32_t source{std::numeric_limits<uint32_t>::max()};
	uint32_t target{std::numeric_limits<uint32_t>::max()};
	int32_t weight{0};
	int32_t type{std::numeric_limits<int32_t>::max()};
	int32_t maxspeed{0}; //in km/h
#ifdef CONFIG_SUPPORT_STRING_CARRY_OVER
	std::string carryover;
#endif
};

struct State {
	struct Configuration {
		std::unordered_map<std::string, int> hwTagIds;
		std::unordered_map<int, double> typeToWeight; //weight is in 1/100 sec to travel 1 m
		std::unordered_set<int> implicitOneWay;
		inline double maxSpeedFromType(int type) { return 360.0/typeToWeight.at(type); }
	} cfg;
	struct CommandLineOptions {
		bool withBounds = false;
		sserialize::spatial::GeoRect bounds;
		WeightCalculatorType wcType = WC_DISTANCE;
		GraphType graphType = GT_NONE;
		int64_t hugheHashMapPopulate = -1;
		bool sortedEdges = false;
		bool connectedComponents = false;
		FilterMode cc_filter_mode;
		std::size_t cc_filter_value{0};
		bool addReverseEdges = true;
		double distanceMult = 1; ///multiply with distance: 1000 -> distance is in mm
		double timeMult = 100; ///multiply with time: 1000 -> time is in ms 
	} cmd;
	typedef sserialize::DirectHugeHashMap<uint32_t> OsmIdToMyNodeIdHashMap;
	OsmIdToMyNodeIdHashMap osmIdToMyNodeId;
	std::unordered_set<int64_t> invalidWays;
	std::vector<Coordinates> nodeCoordinates;
	std::vector<Node> nodes; //this is only temporarily valid and gets deleted after writing out the nodes
	uint64_t edgeCount;
	State() : edgeCount(0) {}
};

typedef std::shared_ptr<State> StatePtr;

}}}//end namespace

#endif
