#ifndef OSM_GRAPH_TOOLS_RAM_GRAPH_H
#define OSM_GRAPH_TOOLS_RAM_GRAPH_H
#include <stdint.h>
#include <vector>
#include <sserialize/utility/UByteArrayAdapter.h>
#include <sserialize/utility/SerializationInfo.h>

namespace osm {
namespace graphs {
namespace ram {

typedef uint32_t NodeContainerSizeType;
typedef uint32_t EdgeContainerSizeType;

struct Edge {
	typedef NodeContainerSizeType DestType;
	typedef double WeightType;
	typedef uint8_t TypeType;
	DestType dest;
	WeightType weight;
	TypeType type;
	Edge() {}
	Edge(sserialize::UByteArrayAdapter data) {
		data >> dest >> weight >> type;
	}
};



struct Node {
	Node() : edgesBegin(0), edgeCount(0) {}
	Node(EdgeContainerSizeType edgesBegin, uint16_t edgeCount, double lat, double lon) : edgesBegin(edgesBegin), edgeCount(edgeCount), coords(lat, lon) {}
	~Node() {}
	EdgeContainerSizeType edgesBegin;
	uint16_t edgeCount;
	struct Coordinates {
		Coordinates() {}
		Coordinates(double lat, double lon) : lat(lat), lon(lon) {}
		Coordinates(sserialize::UByteArrayAdapter data) { data >> lat >> lon; }
		double lat;
		double lon;
	} coords;
	Node(sserialize::UByteArrayAdapter data);
};

class RamGraph {
private:
	std::vector<Node> m_nodes;
	std::vector<Edge> m_edges;
public:
	RamGraph() {}
	virtual ~RamGraph() {}
	inline void swap(RamGraph & other) {
		std::swap(other.nodes(), nodes());
		std::swap(other.edges(), edges());
	}
	inline const std::vector<Node> & nodes() const { return m_nodes; }
	inline std::vector<Node> & nodes() { return m_nodes; }
	inline const std::vector<Edge> & edges() const { return m_edges; }
	inline std::vector<Edge> & edges() { return m_edges; }
	
	void serialize(sserialize::UByteArrayAdapter & dest) const;
};

inline sserialize::UByteArrayAdapter & operator<<(sserialize::UByteArrayAdapter & dest, const osm::graphs::ram::Node::Coordinates & coords) {
	return dest << coords.lat << coords.lon;
}

inline sserialize::UByteArrayAdapter & operator>>(sserialize::UByteArrayAdapter & dest, osm::graphs::ram::Node::Coordinates & coords) {
	return dest >> coords.lat >> coords.lon;
}

inline sserialize::UByteArrayAdapter & operator<<(sserialize::UByteArrayAdapter & dest, const osm::graphs::ram::Node & node) {
	return dest << node.edgesBegin << node.edgeCount << node.coords;
}

inline sserialize::UByteArrayAdapter & operator<<(sserialize::UByteArrayAdapter & dest, const osm::graphs::ram::Edge & edge) {
	return dest << edge.dest << edge.weight << edge.type;
}

}}}//end namespace

namespace sserialize {

template<>
struct SerializationInfo<osm::graphs::ram::Node::Coordinates> {
	static const bool is_fixed_length = SerializationInfo<double>::is_fixed_length;
	static const OffsetType length = 2*SerializationInfo<double>::length;
	static const OffsetType max_length = 2*SerializationInfo<double>::max_length;
	static const OffsetType min_length = 2*SerializationInfo<double>::min_length;
	static inline OffsetType sizeInBytes(const osm::graphs::ram::Node::Coordinates & value) { return length; }
};

template<>
struct SerializationInfo<osm::graphs::ram::Node> {
	static const bool is_fixed_length = SerializationInfo<osm::graphs::ram::EdgeContainerSizeType>::is_fixed_length && SerializationInfo<uint16_t>::is_fixed_length && SerializationInfo<osm::graphs::ram::Node::Coordinates>::is_fixed_length;
	static const OffsetType length = SerializationInfo<osm::graphs::ram::EdgeContainerSizeType>::length + SerializationInfo<uint16_t>::length + SerializationInfo<osm::graphs::ram::Node::Coordinates>::length;
	static const OffsetType max_length = SerializationInfo<osm::graphs::ram::EdgeContainerSizeType>::max_length + SerializationInfo<uint16_t>::max_length + SerializationInfo<osm::graphs::ram::Node::Coordinates>::max_length;
	static const OffsetType min_length = SerializationInfo<osm::graphs::ram::EdgeContainerSizeType>::min_length + SerializationInfo<uint16_t>::min_length + SerializationInfo<osm::graphs::ram::Node::Coordinates>::min_length;
	static inline OffsetType sizeInBytes(const osm::graphs::ram::Node & value) { return length; }
};

template<>
struct SerializationInfo<osm::graphs::ram::Edge> {
	static const bool is_fixed_length = SerializationInfo<osm::graphs::ram::Edge::DestType>::is_fixed_length && SerializationInfo<osm::graphs::ram::Edge::WeightType>::is_fixed_length && SerializationInfo<osm::graphs::ram::Edge::TypeType>::is_fixed_length;
	static const OffsetType length = SerializationInfo<osm::graphs::ram::Edge::DestType>::length + SerializationInfo<osm::graphs::ram::Edge::WeightType>::length + SerializationInfo<osm::graphs::ram::Edge::TypeType>::length;
	static const OffsetType max_length = SerializationInfo<osm::graphs::ram::Edge::DestType>::max_length + SerializationInfo<osm::graphs::ram::Edge::WeightType>::max_length + SerializationInfo<osm::graphs::ram::Edge::TypeType>::max_length;
	static const OffsetType min_length = SerializationInfo<osm::graphs::ram::Edge::DestType>::min_length + SerializationInfo<osm::graphs::ram::Edge::WeightType>::min_length + SerializationInfo<osm::graphs::ram::Edge::TypeType>::min_length;
	static inline OffsetType sizeInBytes(const osm::graphs::ram::Edge & value) { return length; }
};

}

#endif