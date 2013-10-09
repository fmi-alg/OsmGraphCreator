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
	out() << "# Id : 0" << std::endl;
	out() << "# Timestamp : " << time(0) << std::endl;
	out() << "# Type : standard" << std::endl;
	out() << "# Revision: 1 " << std::endl;
	out() << nodeCount << std::endl;
	out() << edgeCount << std::endl;
}

void FmiTextGraphWriter::writeNode(const Node & n) {
	out() << n.id << " " << n.osmId<< " " << n.coordinates.lat << " " << n.coordinates.lon << " " << n.elev << " " << n.stringCarryOverSize << " ";
	if (n.stringCarryOverSize)
		out().write(n.stringCarryOverData, n.stringCarryOverSize);
	out() << std::endl;
}

void FmiTextGraphWriter::writeEdge(const Edge & e) {
	out() << e.source << " " << e.target << " " << e.weight << " " << e.type << " " << e.carryover.size() << " " << e.carryover << std::endl;
}

FmiMaxSpeedTextGraphWriter::FmiMaxSpeedTextGraphWriter(std::ostream & out) : FmiTextGraphWriter(out) {}
FmiMaxSpeedTextGraphWriter::~FmiMaxSpeedTextGraphWriter() {}

void FmiMaxSpeedTextGraphWriter::writeHeader(uint64_t nodeCount, uint64_t edgeCount) {
	out() << "# Id : 0" << std::endl;
	out() << "# Timestamp : " << time(0) << std::endl;
	out() << "# Type : maxspeed" << std::endl;
	out() << "# Revision: 1 " << std::endl;
	out() << nodeCount << std::endl;
	out() << edgeCount << std::endl;
}

void FmiMaxSpeedTextGraphWriter::writeEdge(const Edge & e) {
	out() << e.source << " " << e.target << " " << e.weight << " " << e.type << " " << e.maxspeed << " " << e.carryover.size() << " " << e.carryover << std::endl;
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
	union {
		double d;
		int64_t i;
	} tmp;
	tmp.d = v;
	putLong(tmp.i);
}

void FmiBinaryGraphWriter::writeHeader(uint64_t nodeCount, uint64_t edgeCount) {
	out() << "# Id : 0" << std::endl;
	out() << "# Timestamp : " << time(0) << std::endl;
	out() << "# Type : standard" << std::endl;
	out() << "# Revision: 1 " << std::endl;
	out() << std::endl;
	putInt(nodeCount);
	putInt(edgeCount);
}

void FmiBinaryGraphWriter::writeNode(const Node & n) {
	putInt(n.id);
	putLong(n.osmId);
	putDouble(n.coordinates.lat);
	putDouble(n.coordinates.lon);
	putInt(n.elev);
	putInt(n.stringCarryOverSize);
	out().write(n.stringCarryOverData, n.stringCarryOverSize);
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
	out() << "# Id : 0" << std::endl;
	out() << "# Timestamp : " << time(0) << std::endl;
	out() << "# Type : maxspeed" << std::endl;
	out() << "# Revision: 1 " << std::endl;
	out() << std::endl;
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

RamGraphWriter::RamGraphWriter() : m_edgeBegin(0) {}

RamGraphWriter::~RamGraphWriter() {}

osm::graphs::ram::RamGraph & RamGraphWriter::graph() { return m_graph; }

void RamGraphWriter::writeHeader(uint64_t nodeCount, uint64_t edgeCount) {
	m_graph.nodes().reserve(nodeCount);
	m_graph.edges().resize(edgeCount);
}

void RamGraphWriter::writeNode(const graphtools::creator::Node & node) {
	m_graph.nodes().push_back( osm::graphs::ram::Node(m_edgeBegin, node.outdegree) );
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

PlotGraph::PlotGraph(std::ostream & out) : m_out(out) {}
PlotGraph::~PlotGraph() {}
void PlotGraph::writeHeader(uint64_t nodeCount, uint64_t edgeCount) {
	m_nodes.reserve(nodeCount);
}
void PlotGraph::writeNode(const graphtools::creator::Node & node) {
	m_nodes.push_back(node.coordinates);
}
void PlotGraph::writeEdge(const graphtools::creator::Edge & edge) {
	out() <<  m_nodes[edge.source].lon << " " << m_nodes[edge.source].lat << " " << m_nodes[edge.target].lon << " " << m_nodes[edge.target].lat << std::endl;
}

}}}//end namespace