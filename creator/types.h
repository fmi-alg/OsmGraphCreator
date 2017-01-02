#ifndef OSM_GRAPH_TOOLS_TYPES_H
#define OSM_GRAPH_TOOLS_TYPES_H
#include <memory>
#include <string>
#include <stdint.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <sserialize/templated/DirectHugeHash.h>

namespace osm {
namespace graphtools {
namespace creator {

enum OneWayStatus {OW_YES, OW_NO, OW_IMPLICIT};
enum WeightCalculatorType {WC_NONE, WC_DISTANCE, WC_TIME, WC_MAXSPEED};
enum GraphType {GT_NONE, GT_TOPO_TEXT, GT_FMI_TEXT, GT_FMI_BINARY, GT_FMI_MAXSPEED_BINARY, GT_FMI_MAXSPEED_TEXT, GT_SSERIALIZE_OFFSET_ARRAY, GT_PLOT, GT_SSERIALIZE_LARGE_OFFSET_ARRAY};

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
	id(id), osmId(osmId), elev(elev), indegree(0), outdegree(0), stringCarryOverSize(0), stringCarryOverData(0) {}
	uint32_t id;
	int64_t osmId;
	uint16_t elev;
	uint16_t indegree;
	uint16_t outdegree;
	uint16_t stringCarryOverSize;
	char * stringCarryOverData; //not zero-terminated
};

//[source][target][weight][type][sizecarryover][carryover] //kante
///@member maxspeed this is always in km/h
///@meber weight this is usualy either the length of the arc or the travel time
struct Edge {
	Edge() {}
	Edge(uint32_t source, uint32_t target, int32_t weight, int32_t type, int32_t maxspeed) : source(source), target(target), weight(weight), type(type), maxspeed(maxspeed) {}
	Edge & reverse() {
		std::swap(source, target);
		return *this;
	}
	uint32_t source;
	uint32_t target;
	int32_t weight;
	int32_t type;
	int32_t maxspeed; //in km/h
	std::string carryover;
};

struct State {
	struct Configuration {
		std::unordered_map<std::string, int> hwTagIds;
		std::unordered_map<int, double> typeToWeight; //weight is in 1/100 sec to travel 1 m
		std::unordered_set<int> implicitOneWay;
		inline double maxSpeedFromType(int type) { return 360.0/typeToWeight.at(type); }
	} cfg;
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