#include "GraphWriter.h"
/* uint*_t */
#include <stdint.h>
/* memcpy */
#include <string.h>

#include <type_traits>
#include <functional>
#include <limits>
#include <sserialize/utility/utilmath.h>

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
#include <sserialize/utility/ProgressInfo.h>


namespace osm {
namespace graphtools {
namespace creator {

// # Id : [hexstring]
// # Timestamp : [int]
// # Type: standard
// # Revision: 1


FmiTextGraphWriter::FmiTextGraphWriter(std::ostream & out) :  m_out(out) {}
FmiTextGraphWriter::~FmiTextGraphWriter(){}

void FmiTextGraphWriter::writeHeader(uint64_t nodeCount, uint64_t edgeCount) {
	out() << "# Id : 0\n";
	out() << "# Timestamp : " << time(0) << "\n";
	out() << "# Type : standard" << "\n";
	out() << "# Revision: 1 " << "\n\n";
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

FmiMaxSpeedTextGraphWriter::FmiMaxSpeedTextGraphWriter(std::ostream & out) : FmiTextGraphWriter(out) {}
FmiMaxSpeedTextGraphWriter::~FmiMaxSpeedTextGraphWriter() {}

void FmiMaxSpeedTextGraphWriter::writeHeader(uint64_t nodeCount, uint64_t edgeCount) {
	out() << "# Id : 0\n";
	out() << "# Timestamp : " << time(0) << "\n";
	out() << "# Type : maxspeed" << "\n";
	out() << "# Revision: 1 " << "\n\n";
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

FmiBinaryGraphWriter::FmiBinaryGraphWriter(std::ostream & out) :  m_out(out) {}


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


FmiMaxSpeedBinaryGraphWriter::FmiMaxSpeedBinaryGraphWriter(std::ostream & out) : FmiBinaryGraphWriter(out) {}
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

PlotGraph::PlotGraph(std::ostream & out) : m_out(out) {}
PlotGraph::~PlotGraph() {}
void PlotGraph::writeHeader(uint64_t nodeCount, uint64_t edgeCount) {
	m_nodes.reserve(nodeCount);
}
void PlotGraph::writeNode(const osm::graphtools::creator::Node & node, const osm::graphtools::creator::Coordinates & coordinates) {
	m_nodes.push_back(coordinates);
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