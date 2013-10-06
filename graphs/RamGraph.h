#ifndef OSM_GRAPH_TOOLS_RAM_GRAPH_H
#define OSM_GRAPH_TOOLS_RAM_GRAPH_H
#include <stdint.h>
#include <vector>

namespace osm {
namespace graphs {
namespace ram {

typedef uint64_t NodeContainerSizeType;

struct Edge {
	NodeContainerSizeType dest;
	double weight;
};

struct Node {
	Edge<EdgePayload> * edgesBegin;
	uint16_t edgeCount;
};

class RamGraph {
private:
	std::vector<>
public:
};


}}}//end namespace

#endif