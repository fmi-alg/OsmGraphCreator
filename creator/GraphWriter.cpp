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
	out() << n.id << " " << n.osmId<< " " << n.coordinates.lat << " " << n.coordinates.lon << " " << n.elev << " " << n.carryover.size() << " " << n.carryover << std::endl;
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
	putInt(n.carryover.size());
	out().write(n.carryover.c_str(), n.carryover.size());
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

}}}//end namespace