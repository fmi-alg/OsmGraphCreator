#include "RamGraph.h"


namespace osm {
namespace graphs {
namespace ram {

RamGraphWriter::RamGraphWriter() {}

RamGraphWriter::~RamGraphWriter();

RamGraph & RamGraphWriter::graph();

void RamGraphWriter::writeHeader(uint64_t nodeCount, uint64_t edgeCount) {
	m_graph.nodes().resize(nodeCount);
	m_graph.edges().resize(edgeCount);
}

void RamGraphWriter::writeNode(const graphtools::creator::Node & node) {
	m_graph.nodes().at(node.id) = osm::graphs::ram::Node();
}

void RamGraphWriter::writeEdge(const graphtools::creator::Edge & edge);

}}}//end namespace