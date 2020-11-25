#include "GraphWriter.h"
/* uint*_t */
#include <stdint.h>
/* memcpy */
#include <string.h>

#include <type_traits>
#include <functional>
#include <limits>

/* make sure be32toh and be64toh are present */
#if defined(__linux__)
#  if defined(__ANDROID__)
#    include <sys/endian.h>
#    define be16toh(x) betoh16(x)
#    define be32toh(x) betoh32(x)
#    define be64toh(x) betoh64(x)
#  else
#    include <endian.h>
#  endif
#elif defined(__FreeBSD__) || defined(__NetBSD__)
#  include <sys/endian.h>
#elif defined(__OpenBSD__)
#  include <sys/types.h>
#  define be16toh(x) betoh16(x)
#  define be32toh(x) betoh32(x)
#  define be64toh(x) betoh64(x)
#else
/* htons/htonl */
#include <arpa/inet.h>
#  define htobe16(x) htons(x)
#  define be16toh(x) htons(x)
#  define htobe32(x) htonl(x)
#  define be32toh(x) htonl(x)
#  define htobe64(x) my_htobe64(x)
#  define be64toh(x) htobe64(x)
static uint64_t my_htobe64(uint64_t x) {
	if (htobe32(1) == 1) return x;
	return ((uint64_t)htobe32(x)) << 32 | htobe32(x >> 32);
}

#endif
#include <sserialize/stats/ProgressInfo.h>
#include <sserialize/containers/UnionFind.h>


namespace osm {
namespace graphtools {
namespace creator {

TopologyTextGraphWriter::TopologyTextGraphWriter(std::shared_ptr<std::ostream> out) :  m_out(out) {}
TopologyTextGraphWriter::~TopologyTextGraphWriter(){}

void TopologyTextGraphWriter::writeHeader(uint64_t nodeCount, uint64_t edgeCount) {
	out() << nodeCount << "\n";
	out() << edgeCount << "\n";
}

void TopologyTextGraphWriter::writeNode(const osm::graphtools::creator::Node & node, const osm::graphtools::creator::Coordinates & coordinates) {
	out() << coordinates.lat << " " << coordinates.lon << "\n";
}

void TopologyTextGraphWriter::writeEdge(const Edge & e) {
	out() << e.source << " " << e.target << "\n";
}

// # Id : [hexstring]
// # Timestamp : [int]
// # Type: standard
// # Revision: 1


FmiTextGraphWriter::FmiTextGraphWriter(std::shared_ptr<std::ostream> out) :  m_out(out) {}
FmiTextGraphWriter::~FmiTextGraphWriter(){}

void FmiTextGraphWriter::writeHeader(uint64_t nodeCount, uint64_t edgeCount) {
	out() << "# Id : 0\n";
	out() << "# Timestamp : " << time(0) << "\n";
	out() << "# Type : standard" << "\n";
	out() << "# Revision : 1" << "\n\n";
	out() << nodeCount << "\n";
	out() << edgeCount << "\n";
}

void FmiTextGraphWriter::writeNode(const osm::graphtools::creator::Node & node, const osm::graphtools::creator::Coordinates & coordinates) {
	out() << node.id << " " << node.osmId << " " << coordinates.lat << " " << coordinates.lon << " " << node.elev;
	if (node.stringCarryOverSize) {
		out() << " ";
		out().write(node.stringCarryOverData, node.stringCarryOverSize);
	}
	out() << "\n";
}

void FmiTextGraphWriter::writeEdge(const Edge & e) {
	out() << e.source << " " << e.target << " " << e.weight << " " << e.type;
	if (e.carryover.size()) {
		out() << " " << e.carryover;
	}
	out() << "\n";
}

FmiMaxSpeedTextGraphWriter::FmiMaxSpeedTextGraphWriter(std::shared_ptr<std::ostream> out) : FmiTextGraphWriter(out) {}
FmiMaxSpeedTextGraphWriter::~FmiMaxSpeedTextGraphWriter() {}

void FmiMaxSpeedTextGraphWriter::writeHeader(uint64_t nodeCount, uint64_t edgeCount) {
	out() << "# Id : 0\n";
	out() << "# Timestamp : " << time(0) << "\n";
	out() << "# Type : maxspeed" << "\n";
	out() << "# Revision : 1" << "\n\n";
	out() << nodeCount << "\n";
	out() << edgeCount << "\n";
}

void FmiMaxSpeedTextGraphWriter::writeEdge(const Edge & e) {
	out() << e.source << " " << e.target << " " << e.weight << " " << e.type << " " << e.maxspeed;
	if (e.carryover.size()) {
		out() << " " << e.carryover;
	}
	out() << "\n";
}

FmiBinaryGraphWriter::FmiBinaryGraphWriter(std::shared_ptr<std::ostream> out) :  m_out(out) {}


FmiBinaryGraphWriter::~FmiBinaryGraphWriter() {}

void FmiBinaryGraphWriter::putInt(int32_t v) {
	v = htobe32(v);
	char tmp[sizeof(v)];
	memcpy(tmp, &v, sizeof(v));
	out().write(tmp, sizeof(v));
}

void FmiBinaryGraphWriter::putLong(int64_t v) {
	v = htobe64(v);
	char tmp[sizeof(v)];
	memcpy(tmp, &v, sizeof(v));
	out().write(tmp, sizeof(v));
}

void FmiBinaryGraphWriter::putDouble(double v) {
	char tmp[sizeof(v)];
	memcpy(tmp, &v, sizeof(v));
	out().write(tmp, sizeof(v));
}

void FmiBinaryGraphWriter::writeHeader(uint64_t nodeCount, uint64_t edgeCount) {
	out() << "# Id : 0\n";
	out() << "# Timestamp : " << time(0) << "\n";
	out() << "# Type : standard" << "\n";
	out() << "# Revision: 1 " << "\n\n";
	putInt(nodeCount);
	putInt(edgeCount);
}

void FmiBinaryGraphWriter::writeNode(const osm::graphtools::creator::Node & node, const osm::graphtools::creator::Coordinates & coordinates) {
	putInt(node.id);
	putLong(node.osmId);
	putDouble(coordinates.lat);
	putDouble(coordinates.lon);
	putInt(node.elev);
	putInt(node.stringCarryOverSize);
	out().write(node.stringCarryOverData, node.stringCarryOverSize);
}

void FmiBinaryGraphWriter::writeEdge(const Edge & e) {
	putInt(e.source);
	putInt(e.target);
	putInt(e.weight);
	putInt(e.type);
	putInt(e.carryover.size());
	out().write(e.carryover.c_str(), e.carryover.size());
}


FmiMaxSpeedBinaryGraphWriter::FmiMaxSpeedBinaryGraphWriter(std::shared_ptr<std::ostream> out) : FmiBinaryGraphWriter(out) {}
FmiMaxSpeedBinaryGraphWriter::~FmiMaxSpeedBinaryGraphWriter() {}

void FmiMaxSpeedBinaryGraphWriter::writeHeader(uint64_t nodeCount, uint64_t edgeCount) {
	out() << "# Id : 0" << "\n";
	out() << "# Timestamp : " << time(0) << "\n";
	out() << "# Type : maxspeed" << "\n\n";
	out() << "# Revision: 1 " << "\n";
	putInt(nodeCount);
	putInt(edgeCount);
}

void FmiMaxSpeedBinaryGraphWriter::writeEdge(const Edge & e) {
	putInt(e.source);
	putInt(e.target);
	putInt(e.weight);
	putInt(e.type);
	putInt(e.maxspeed);
	putInt(e.carryover.size());
	out().write(e.carryover.c_str(), e.carryover.size());
}

RamGraphWriter::RamGraphWriter(const sserialize::UByteArrayAdapter & data) : m_data(data), m_edgeBegin(0) {}

RamGraphWriter::~RamGraphWriter() {}

osm::graphs::ram::RamGraph & RamGraphWriter::graph() { return m_graph; }

void RamGraphWriter::writeHeader(uint64_t nodeCount, uint64_t edgeCount) {
	m_graph.nodes().reserve(nodeCount);
	m_graph.edges().resize(edgeCount);
}

void RamGraphWriter::writeNode(const osm::graphtools::creator::Node & node, const osm::graphtools::creator::Coordinates & coordinates) {
	m_graph.nodes().push_back( osm::graphs::ram::Node(m_edgeBegin, node.outdegree, coordinates.lat, coordinates.lon) );
	m_edgeBegin += node.outdegree;
}

void RamGraphWriter::writeEdge(const graphtools::creator::Edge & edge) {
	osm::graphs::ram::Node & gnode = m_graph.nodes()[edge.source];
	osm::graphs::ram::Edge & gedge = m_graph.edges()[gnode.edgesBegin+gnode.edgeCount];
	++gnode.edgeCount;
	gedge.dest = edge.target;
	gedge.weight = edge.weight;
	gedge.type = edge.type;
}

void RamGraphWriter::endGraph() {
	m_data.resize(100*graph().nodes().size()+ sserialize::SerializationInfo<osm::graphs::ram::Edge>::length*graph().edges().size());
	graph().serialize(m_data);
	if (m_data.tellPutPtr() < m_data.size()) {
		m_data.shrinkStorage( m_data.size() - m_data.tellPutPtr() );
	}
}


CCGraphWriter::CCGraphWriter(GraphWriterFactory factory, uint32_t minCCSize) :
m_f(factory),
m_minCCSize(minCCSize)
{}

CCGraphWriter::~CCGraphWriter()
{}

void
CCGraphWriter::endGraph() {
	std::cout << "Finding connected components" << std::endl;
	using UnionFind = sserialize::UnionFind<uint32_t>;
	using UFHandle = UnionFind::handle_type;
	UnionFind uf;
	std::vector<UFHandle> ufh(m_nodes.size());
	//add all nodes as single sets
	std::cout << "Creating single sets for union find" << std::endl;
	for(std::size_t i(0), s(m_nodes.size()); i < s; ++i) {
		ufh.at(i) = uf.make_set(i);
	}
	std::cout << "Uniting all nodes using edges" << std::endl;
	//now unite all nodes that have an edge between each other
	for(auto const & e : m_edges) {
		uf.unite(ufh.at(e.source), ufh.at(e.target));
	}
	//Get all unique representatives
	std::cout << "Setting node representatives" << std::endl;
	std::unordered_map<UFHandle, std::pair<uint32_t, uint32_t>> cch; //first entry is node count second the edge count
	for(std::size_t i(0), s(m_nodes.size()); i < s; ++i) {
		cch[uf.find(ufh.at(i))] = std::pair<uint32_t, uint32_t>(0,0);
	}
	std::cout << "Found " << cch.size() << " connected components" << std::endl;
	//Now we have a Problem: For planet there will be thousands of connected components
	//However we cannot open thousands of files
	//We can either create a mapping vector which we sort according to our representative
	//Or we traverse all nodes/edges per connected component
	//Quick calc: About 10^9 Nodes and 2*10^9 edges sort takes -> log(10^9) ~ log(2^30) ~30 longer than a single pass
	//Thus sorting should be way faster
	//sort all nodes according to their representative
	auto sortByRep = [&](auto && a, auto && b) -> bool {
		auto repa = uf.find(ufh.at(a));
		auto repb = uf.find(ufh.at(b));
		return repa == repb ? a < b : repa < repb;
	};
	
	std::cout << "Sorting nodes according to their connected component" << std::endl;
	std::vector<uint32_t> nodesSortedByRep(m_nodes.size());
	for(std::size_t i(0); i < m_nodes.size(); ++i) {
		auto nodeRep = uf.find(ufh.at(i));
		cch.at(nodeRep).first += 1;
		nodesSortedByRep.at(i) = i;
	}
	std::sort(nodesSortedByRep.begin(), nodesSortedByRep.end(), sortByRep);
	
	//do the same for the edges
	//By definition the rep of source and target are the same
	std::cout << "Sorting edges according to their connected component" << std::endl;
	std::vector<uint32_t> edgesSortedByRep(m_edges.size());
	for(std::size_t i(0), s(m_edges.size()); i < s; ++i) {
		cch.at(uf.find(ufh.at(m_edges.at(i).source))).second += 1;
		edgesSortedByRep.at(i) = i;
	}
	std::sort(edgesSortedByRep.begin(), edgesSortedByRep.end(),
				[&](auto && a, auto && b) -> bool {
					return sortByRep(m_edges.at(a).source, m_edges.at(b).source);
				}
	);
	if (m_minCCSize > 0) {
		std::size_t count = 0;
		for(auto const & x : cch) {
			count += std::size_t(x.second.first >= m_minCCSize);
		}
		std::cout << "Found " << count << " connected components above your threshold" << std::endl;
	}

	//now write them out
	std::size_t nodePos{0};
	std::size_t edgePos{0};
	std::vector<uint32_t> nodeIdRemap(m_nodes.size()); //remaps nodeIds to cc local ids
	sserialize::ProgressInfo pinfo;
	std::size_t ccId = 0;
	pinfo.begin(cch.size(), "Writing connected components");
	for(std::size_t i(0), s(cch.size()); i < s; ++i) {
		UFHandle ccrep = uf.find( ufh.at(nodesSortedByRep.at(nodePos)) );
		auto nodeEdgeCount = cch.at(ccrep);
		
		if (nodeEdgeCount.first < m_minCCSize) {
			nodePos += nodeEdgeCount.first;
			edgePos += nodeEdgeCount.second;
			continue;
		}
		
		auto writer = m_f(ccId);
		writer->beginGraph();
		writer->beginHeader();
		writer->writeHeader(nodeEdgeCount.first, nodeEdgeCount.second);
		writer->endHeader();
		//now write all nodes and build mapping table on the fly
		writer->beginNodes();
		for(std::size_t cclNodeId(0); cclNodeId < nodeEdgeCount.first; ++cclNodeId, ++nodePos) {
			uint32_t globalNodeId = nodesSortedByRep.at(nodePos);
			assert(uf.find( ufh.at(globalNodeId) ) == ccrep);
			//we need to remap the id of the node first
			Node node = m_nodes.at(globalNodeId).first;
			node.id = cclNodeId;
			writer->writeNode(node, m_nodes.at(globalNodeId).second);
			nodeIdRemap.at(globalNodeId) = cclNodeId;
		}
		writer->endNodes();
		//And write all Edges but remap source/target to the new ids
		writer->beginEdges();
		for(std::size_t cclEdgeId(0); cclEdgeId < nodeEdgeCount.second; ++cclEdgeId, ++edgePos) {
			assert(uf.find( ufh.at( m_edges.at(edgesSortedByRep.at(edgePos)).source) ) == ccrep);
			Edge e = m_edges.at(edgesSortedByRep.at(edgePos));
			e.source = nodeIdRemap.at(e.source);
			e.target = nodeIdRemap.at(e.target);
			writer->writeEdge(e);
		}
		writer->endEdges();
		writer->endGraph();
		++ccId;
		pinfo(i, "ccid " + std::to_string(ccId) + ": #nodes=" + std::to_string(nodeEdgeCount.first) + " #edges=" + std::to_string(nodeEdgeCount.second));
	}
	pinfo.end();
}

void
CCGraphWriter::writeHeader(uint64_t nodeCount, uint64_t edgeCount) {
	m_nodes.reserve(nodeCount);
	m_edges.resize(edgeCount);
}

void
CCGraphWriter::writeNode(const graphtools::creator::Node & node, const Coordinates & coordinates) {
	m_nodes.emplace_back(node, coordinates);
}

void
CCGraphWriter::writeEdge(const graphtools::creator::Edge & edge) {
	m_edges.emplace_back(edge);
}

PlotGraph::PlotGraph(std::shared_ptr<std::ostream> out) : m_out(out) {}
PlotGraph::~PlotGraph() {}
void PlotGraph::writeHeader(uint64_t nodeCount, uint64_t edgeCount) {
	m_nodes.reserve(nodeCount);
}
void PlotGraph::writeNode(const osm::graphtools::creator::Node & node, const osm::graphtools::creator::Coordinates & coordinates) {
	m_nodes.emplace_back(coordinates);
}
void PlotGraph::writeEdge(const graphtools::creator::Edge & edge) {
	out() <<  m_nodes[edge.source].lon << " " << m_nodes[edge.source].lat << " " << m_nodes[edge.target].lon << " " << m_nodes[edge.target].lat << std::endl;
}

StaticGraphWriter::StaticGraphWriter(const sserialize::UByteArrayAdapter & data) : m_data(data) {}
StaticGraphWriter::~StaticGraphWriter() {}

void StaticGraphWriter::writeHeader(uint64_t nodeCount, uint64_t edgeCount) {
	sserialize::UByteArrayAdapter::OffsetType nodeSpaceUsage = sserialize::Static::DynamicFixedLengthVector<osm::graphs::ram::Node>::spaceUsage(nodeCount);
	sserialize::UByteArrayAdapter::OffsetType edgeSpaceUsage = sserialize::Static::DynamicFixedLengthVector<osm::graphs::ram::Edge>::spaceUsage(edgeCount);
	m_data.resize(nodeSpaceUsage + edgeSpaceUsage);
	m_nodes = sserialize::Static::DynamicFixedLengthVector<osm::graphs::ram::Node>(m_data);
	m_edges = sserialize::Static::DynamicFixedLengthVector<osm::graphs::ram::Edge>(m_data+nodeSpaceUsage);
	m_edges.resize(edgeCount);
	m_edgeBegin = 0;
	m_edgeOffsets.reserve(nodeCount);
}
void StaticGraphWriter::writeNode(const osm::graphtools::creator::Node & node, const osm::graphtools::creator::Coordinates & coordinates) {
	m_nodes.push_back(osm::graphs::ram::Node(m_edgeBegin, node.outdegree, coordinates.lat, coordinates.lon) );
	m_edgeOffsets.push_back(m_edgeBegin);
	m_edgeBegin += node.outdegree;
}

void StaticGraphWriter::writeEdge(const graphtools::creator::Edge & edge) {
	uint32_t & ef = m_edgeOffsets[edge.source];
	osm::graphs::ram::Edge oed;
	oed.dest = edge.target;
	oed.weight = edge.weight;
	oed.type = edge.type;
	m_edges.set(ef, oed);
	++ef;
}

}}}//end namespace
