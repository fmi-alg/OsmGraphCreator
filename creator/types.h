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
enum WeightCalculatorType {WC_NONE, WC_DISTANCE, WC_TIME};
enum GraphType {GT_FMI_TEXT, GT_FMI_BINARY};

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
struct Edge {
	Edge() {}
	Edge(uint32_t source, uint32_t target, int32_t weight, int32_t type) : source(source), target(target), weight(weight), type(type) {}
	Edge & reverse() {
		std::swap(source, target);
		return *this;
	}
	uint32_t source;
	uint32_t target;
	int32_t weight;
	int32_t type;
	std::string carryover;
};

struct State {
	State() : hwTagIds(new std::unordered_map<std::string, int>()), typeToWeight(new std::unordered_map<int, double>()), implicitOneWay(new std::unordered_set<int>()),
				osmIdToMyNodeId(new std::unordered_map<int64_t, uint32_t>()), invalidWays(new std::unordered_set<int64_t>()), nodes(new std::vector<Node>())
	{} 
	std::shared_ptr< std::unordered_map<std::string, int> > hwTagIds;
	std::shared_ptr< std::unordered_map<int, double> > typeToWeight; //weight is in 1/100 sec to travel 1 m
	std::shared_ptr< std::unordered_set<int> > implicitOneWay;
	std::shared_ptr< std::unordered_map<int64_t, uint32_t> > osmIdToMyNodeId;
	std::shared_ptr< std::unordered_set<int64_t> > invalidWays;
	std::shared_ptr< std::vector<Node> > nodes;
};

}}}//end namespace


#endif