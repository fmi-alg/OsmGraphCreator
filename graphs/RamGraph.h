#ifndef OSM_GRAPH_TOOLS_RAM_GRAPH_H
#define OSM_GRAPH_TOOLS_RAM_GRAPH_H
#include <stdint.h>
#include <vector>
#include "../creator/GraphWriter.h"

namespace osm {
namespace graphs {
namespace ram {

typedef uint64_t NodeContainerSizeType;

struct Edge {
	NodeContainerSizeType dest;
	double weight;
	int type;
};

struct Node {
	Node() : edgesBegin(0), edgeCount(0) {}
	~Node() {}
	void pushEdge();
	Edge * edgesBegin;
	uint16_t edgeCount;
};

class RamGraph {
private:
	std::vector<Node> m_nodes;
	std::vector<Edge> m_edges;
public:
	RamGraph();
	virtual ~RamGraph();
	void swap(RamGraph & other);
	const std::vector<Node> & nodes() const;
	std::vector<Node> & nodes();
	const std::vector<Edge> & edges() const;
	std::vector<Edge> & edges();
};

class RamGraphWriter: public graphtools::creator::GraphWriter {
	RamGraph m_graph;
public:
	RamGraphWriter();
	virtual ~RamGraphWriter();
	RamGraph & graph();
	virtual void writeHeader(uint64_t nodeCount, uint64_t edgeCount);
	virtual void writeNode(const graphtools::creator::Node & node);
	virtual void writeEdge(const graphtools::creator::Edge & edge);
};

}}}//end namespace

#endif