#ifndef OSM_GRAPH_TOOLS_RAM_GRAPH_H
#define OSM_GRAPH_TOOLS_RAM_GRAPH_H
#include <stdint.h>
#include <vector>
#include <sserialize/utility/UByteArrayAdapter.h>
#include <sserialize/utility/SerializationInfo.h>

namespace osm {
namespace graphs {
namespace ram {

typedef uint64_t NodeContainerSizeType;
typedef uint64_t EdgeContainerSizeType;

struct Edge {
	typedef NodeContainerSizeType DestType;
	typedef double WeightType;
	typedef int8_t TypeType;
	DestType dest;
	WeightType weight;
	TypeType type;
};



struct Node {
	Node() : edgesBegin(0), edgeCount(0) {}
	Node(EdgeContainerSizeType edgesBegin, uint16_t edgeCount) : edgesBegin(edgesBegin), edgeCount(edgeCount) {}
	~Node() {}
	EdgeContainerSizeType edgesBegin;
	uint16_t edgeCount;
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

}}}//end namespace

namespace sserialize {
template<>
struct SerializationInfo<osm::graphs::ram::Edge> {
	static const bool is_fixed_length = SerializationInfo<osm::graphs::ram::Edge::DestType>::is_fixed_length && SerializationInfo<osm::graphs::ram::Edge::WeightType>::is_fixed_length && SerializationInfo<osm::graphs::ram::Edge::TypeType>::is_fixed_length;
	static const OffsetType length = SerializationInfo<osm::graphs::ram::Edge::DestType>::length + SerializationInfo<osm::graphs::ram::Edge::WeightType>::length + SerializationInfo<osm::graphs::ram::Edge::TypeType>::length;
	static const OffsetType max_length = SerializationInfo<osm::graphs::ram::Edge::DestType>::max_length + SerializationInfo<osm::graphs::ram::Edge::WeightType>::max_length + SerializationInfo<osm::graphs::ram::Edge::TypeType>::max_length;
	static const OffsetType min_length = SerializationInfo<osm::graphs::ram::Edge::DestType>::min_length + SerializationInfo<osm::graphs::ram::Edge::WeightType>::min_length + SerializationInfo<osm::graphs::ram::Edge::TypeType>::min_length;
	static inline OffsetType sizeInBytes(const osm::graphs::ram::Edge & value) { return length; }
};

}

inline sserialize::UByteArrayAdapter operator<<(sserialize::UByteArrayAdapter & dest, const osm::graphs::ram::Edge & edge) {
	return dest << edge.dest << edge.weight << edge.type;
}

#endif