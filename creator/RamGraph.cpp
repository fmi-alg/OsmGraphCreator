#include "RamGraph.h"
#include <sserialize/containers/MultiVarBitArray.h>
#include <sserialize/utility/utilmath.h>
#include <sserialize/Static/Deque.h>

namespace osm {
namespace graphs {
namespace ram {

void RamGraph::serialize(sserialize::UByteArrayAdapter & dest) const {
	if (! nodes().size())
		return;
	std::vector<uint8_t> config;
	uint32_t maxEdgeCount = 0;
	for(const Node & node : nodes()) {
		maxEdgeCount = std::max<uint32_t>(node.edgeCount, maxEdgeCount);
	}
	config.push_back( std::max<uint8_t>(1, sserialize::msb(nodes().back().edgesBegin)+1) );
	config.push_back( std::max<uint8_t>(1, sserialize::msb(maxEdgeCount)+1) );

	sserialize::MultiVarBitArrayCreator nodeListCreator(config, dest);
	for(std::size_t i = 0, s = nodes().size(); i < s; ++i) {
		nodeListCreator.set(i, 0, nodes()[i].edgesBegin);
		nodeListCreator.set(i, 1, nodes()[i].edgeCount);
	}
	nodeListCreator.flush();
	
	dest << m_edges;
	
	std::cout << "Serialized Graph with " << nodes().size() << " nodes and " << edges().size() << " edges" << std::endl;
	std::cout << "Node-list config: edgeBegin=" << (int)config[0] << ", edgeCount=" << (int)config[1] << " bits" << std::endl;
	
}



}}}//end namespace