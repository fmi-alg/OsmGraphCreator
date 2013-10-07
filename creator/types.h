#ifndef OSM_GRAPH_TOOLS_TYPES_H
#define OSM_GRAPH_TOOLS_TYPES_H
#include <memory>
#include <string>
#include <stdint.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace osm {
namespace graphtools {
namespace creator {

enum OneWayStatus {OW_YES, OW_NO, OW_IMPLICIT};
enum WeightCalculatorType {WC_NONE, WC_DISTANCE, WC_TIME, WC_MAXSPEED};
enum GraphType {GT_FMI_TEXT, GT_FMI_BINARY, GT_FMI_MAXSPEED_BINARY, GT_FMI_MAXSPEED_TEXT};

struct Coordinates {
	Coordinates() {}
	Coordinates(double lat, double lon) : lat(lat), lon(lon) {}
	double lat;
	double lon;
};

//[Id] [osmId] [lat] [lon] [elevation] [carryover] //Knoten
struct Node {
	Node() {}
	Node(uint32_t id, int64_t osmId, const Coordinates & coordinates, int elev) :
	id(id), osmId(osmId), coordinates(coordinates), elev(elev) {}
	uint32_t id;
	int64_t osmId;
	Coordinates coordinates;
	int elev;
	std::string carryover;
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
	std::unordered_map<int64_t, uint32_t> osmIdToMyNodeId;
	std::unordered_set<int64_t> invalidWays;
	std::vector<Node> nodes;
};

typedef std::shared_ptr<State> StatePtr;

}}}//end namespace


#endif