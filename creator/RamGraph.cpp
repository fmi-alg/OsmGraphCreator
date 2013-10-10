#include "RamGraph.h"
#include <sserialize/containers/MultiVarBitArray.h>
#include <sserialize/utility/utilmath.h>
#include <sserialize/utility/ProgressInfo.h>
#include <sserialize/Static/Deque.h>

namespace osm {
namespace graphs {
namespace ram {

Node::Node(sserialize::UByteArrayAdapter data) {data >> edgesBegin >> edgeCount >> coords; }

void RamGraph::serialize(sserialize::UByteArrayAdapter & dest) const {
	dest.reserveFromPutPtr(100*nodes().size()+20*edges().size());
	if (! nodes().size())
		return;
	std::vector<uint32_t> edgeOffsets;
	sserialize::ProgressInfo info;
	info.begin(nodes().size(), "Processing graph nodes");
	for(std::size_t i = 0, s = nodes().size(); i < s; ++i) {
		const Node & node = nodes()[i];
		edgeOffsets.push_back(node.edgesBegin);
		info(i);
	}
	info.end();
	std::cout << "Creating Edgeoffset vector..." << std::flush;
	sserialize::Static::SortedOffsetIndexPrivate::create(edgeOffsets, dest);
	std::cout << "done" << std::endl;
	
	std::cout << "Creating edge vector..." << std::flush;
	dest << m_edges;
	std::cout << "done" << std::endl;
	
	{
		sserialize::Static::DequeCreator<Node::Coordinates> payloadCreator(dest);
		info.begin(nodes().size(), "Creating node payload vector");
		for(std::size_t i = 0, s = nodes().size(); i < s; ++i) {
			const Node & node = nodes()[i];
			payloadCreator.put(node.coords);
			info(i);
		}
		info.end();
		payloadCreator.flush();
	}
	std::cout << "Serialized Graph with " << nodes().size() << " nodes and " << edges().size() << " edges" << std::endl;
}



}}}//end namespace