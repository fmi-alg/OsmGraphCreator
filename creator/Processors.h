#ifndef OSM_GRAPH_TOOLS_PROCESSORS_H
#define OSM_GRAPH_TOOLS_PROCESSORS_H
#include <osmpbf/pbistream.h>
#include <osmpbf/iway.h>
#include <osmpbf/inode.h>
#include <osmpbf/filter.h>
#include <osmpbf/primitiveblockinputadaptor.h>
#include "types.h"
#include "conversion.h"
#include "GraphWriter.h"
#include "WeightCalculator.h"
#include "MaxSpeedParser.h"
#include <unordered_set>
#include <sstream>


namespace osm {
namespace graphtools {
namespace creator {

inline std::string escape_for_json(std::string const & str) {
	std::string result;
	for(char const & c : str) {
		switch(c) {
			case '"':
				result += "\\\"";
				break;
			case '\\':
				result += "\\\\";
				break;
			case '\n':
				result += "\\n";
				break;
			case '\t':
				result += "\\t";
				break;
			case '\r':
				result += "\\r";
				break;
			default:
				result += c;
				break;
		}
	}
	return result;
}

inline std::string tags2json(osmpbf::IPrimitive const & primitive) {
	if (!primitive.tagsSize()) {
		return "{}";
	}
	std::stringstream ss;
	char c = '{';
	for(std::size_t i(0), s(primitive.tagsSize()); i < s; ++i) {
		ss << c;
		c = ',';
		ss << '"' << escape_for_json(primitive.key(i)) << "\":\"" << escape_for_json(primitive.value(i)) << '"';
	}
	ss << '}';
	return ss.str();
}

inline bool isUndirectedEdge(const std::unordered_set<int> & implicitOneWay, int ows, int hwType) {
	if (ows == OW_YES) {
		return false;
	}
	if (implicitOneWay.count(hwType)) {
		return (ows == OW_NO);
	}
	return true;
}

inline void gatherNodes(osmpbf::PbiStream & inFile, StatePtr state) {
	osmpbf::PrimitiveBlockInputAdaptor pbi;
	uint32_t nodeId = 0;
	inFile.dataSeek(0);
	sserialize::ProgressInfo progress;
	progress.begin(state->osmIdToMyNodeId.size(), "Collecting nodes");
	while (inFile.parseNextBlock(pbi) && nodeId < progress.targetCount) {
		if (pbi.isNull()) {
			continue;
		}
		progress(nodeId);

		if (pbi.nodesSize()) {
			for (osmpbf::INodeStream node = pbi.getNodeStream(); !node.isNull(); node.next()) {
				int64_t osmId = node.id();
				if (state->osmIdToMyNodeId.count(osmId)) {
					Node n(nodeId, osmId, 0);
					#ifdef CONFIG_CREATOR_COPY_TAGS
					n.tags = tags2json(node);
					#endif
					state->nodes.push_back(n);
					state->nodeCoordinates.push_back(Coordinates(node.latd(), node.lond()));
					++nodeId;
					if (nodeId == 0) { //check for overflow
						throw std::runtime_error("Too many nodes");
					}
					state->osmIdToMyNodeId[n.osmId] = n.id;
				}
			}
		}
	}
	progress.end();
}

///deletes all available nodes from state->osmIdToMyNodeId
inline void deleteAvailableNodes(osmpbf::PbiStream & inFile, StatePtr state) {
	osmpbf::PrimitiveBlockInputAdaptor pbi;
	uint32_t nodeId = 0;
	inFile.dataSeek(0);
	sserialize::ProgressInfo progress;
	progress.begin(inFile.dataSize(), "Finding unavailable nodes");
	while (inFile.parseNextBlock(pbi) && nodeId < progress.targetCount) {
		if (pbi.isNull()) {
			continue;
		}
		progress(inFile.dataPosition());

		if (pbi.nodesSize()) {
			for (osmpbf::INodeStream node = pbi.getNodeStream(); !node.isNull(); node.next()) {
				int64_t osmId = node.id();
				if ( (! state->cmd.withBounds || state->cmd.bounds.contains(node.latd(), node.lond())) && state->osmIdToMyNodeId.count(osmId)) {
					state->osmIdToMyNodeId.unmark(osmId);
				}
			}
		}
	}
	progress.end();
}

struct WayParser {
	WayParser(const std::string & message, osmpbf::PbiStream & inFile, const std::unordered_map<std::string, int> & hwTagIds) :
	message(message), inFile(inFile), hwTagIds(hwTagIds) {}
	std::string message;
	osmpbf::PbiStream & inFile;
	std::unordered_map<std::string, int> hwTagIds;
	
	template<typename TOPERATOR>
	void parse(TOPERATOR & processor) {
		std::unordered_map<int, int> strIdToHwId;
		std::unordered_set<int> keysToStore;
		std::unordered_map<std::string, std::string> storedKv;
		osmpbf::PrimitiveBlockInputAdaptor pbi;

		sserialize::ProgressInfo progress;
		progress.begin(inFile.dataSize(), message);
		while (inFile.parseNextBlock(pbi)) {
			if (pbi.isNull())
				continue;
			uint32_t highwayTagId = pbi.findString("highway");
			
			if (highwayTagId == 0)
				continue;
			uint32_t onewayTagId = pbi.findString("oneway");
				
			strIdToHwId.clear();
			for(int i = 0, s = pbi.stringTableSize(); i < s; ++i) {
				const std::string & str = pbi.queryStringTable(i);
				if (hwTagIds.count(str) > 0) {
					strIdToHwId[i] = hwTagIds.at(str);
				}
				if (processor.keysToStore().count(str)) {
					keysToStore.insert(i);
				}
			}

			progress(inFile.dataPosition());
			if (pbi.waysSize()) {
				for (osmpbf::IWayStream way = pbi.getWayStream(); !way.isNull(); way.next()) {
					OneWayStatus ows = OW_IMPLICIT;
					int hwType = 0;
					bool process = false;
					for(int i = 0, s = way.tagsSize(); i < s; ++i) {
						uint32_t keyId = way.keyId(i);
						if (keyId == onewayTagId) {
							ows = (toBool(way.value(i) ) > 0 ? OW_YES : OW_NO);
						}
						else if (keyId == highwayTagId) {
							uint32_t valueId = way.valueId(i);
							if ( strIdToHwId.count( valueId ) && way.refsSize() > 1) {
								hwType = strIdToHwId[valueId];
								process = true;
							}
							else {
								break;
							}
						}
						if(keysToStore.count(keyId)) {
							storedKv[way.key(i)] = way.value(i);
						}
					}
					if (process) {
						processor(ows, hwType, storedKv, way);
					}
					storedKv.clear();
				}
			}
		}
		progress.end();
	}
};

///Get the min/max node id
struct MinMaxNodeIdProcessor {
	MinMaxNodeIdProcessor() {}
	MinMaxNodeIdProcessor(MinMaxNodeIdProcessor const &) = delete;
	MinMaxNodeIdProcessor(MinMaxNodeIdProcessor &&) = delete;
	sserialize::AtomicMax<int64_t> largestId;
	sserialize::AtomicMin<int64_t> smallestId;
	std::atomic<uint64_t> refNodeCount{0};
	
	std::unordered_set<std::string> kS;
	inline const std::unordered_set<std::string> & keysToStore() const { return kS; }
	
	inline void operator()(int /*ows*/, int /*hwType*/, const std::unordered_map<std::string, std::string> & /*storedKv*/, const osmpbf::IWay & way) {
		for(osmpbf::IWayStream::RefIterator refIt(way.refBegin()), refEnd(way.refEnd()); refIt != refEnd; ++refIt) {
			int64_t refId = *refIt;
			largestId.update(refId);
			smallestId.update(refId);
		}
		refNodeCount.fetch_add(way.refsSize(), std::memory_order_relaxed);
	}
};

///Marks all nodes needed by valid ways into state->osmIdToMyNodeId
struct AllNodesGatherProcessor {
	AllNodesGatherProcessor(StatePtr state) :
	state(state) {}
	StatePtr state;
	
	std::unordered_set<std::string> kS;
	inline const std::unordered_set<std::string> & keysToStore() const { return kS; }
	
	inline void operator()(int /*ows*/, int /*hwType*/, const std::unordered_map<std::string, std::string> & /*storedKv*/, const osmpbf::IWay & way) {
		for(osmpbf::IWayStream::RefIterator refIt(way.refBegin()), refEnd(way.refEnd()); refIt != refEnd; ++refIt) {
			state->osmIdToMyNodeId.mark(*refIt);
		}
	}
};

///Marks all nodes needed by valid ways into state->osmIdToMyNodeId
///BUT taking invalid ways into account and updating edgeCount
struct NodeRefGatherProcessor {
	NodeRefGatherProcessor(StatePtr state) :
	state(state) {}
	StatePtr state;
	
	std::unordered_set<std::string> kS;
	inline const std::unordered_set<std::string> & keysToStore() const { return kS; }
	
	inline void operator()(int ows, int hwType, const std::unordered_map<std::string, std::string> & /*storedKv*/, const osmpbf::IWay & way) {
		if (state->invalidWays.count(way.id())) { //check if way is valid
			return;
		}
		for(osmpbf::IWayStream::RefIterator refIt(way.refBegin()), refEnd(way.refEnd()); refIt != refEnd; ++refIt) {
			state->osmIdToMyNodeId.mark(*refIt);
		}
		assert(way.refsSize() > 0);
		uint64_t myEdgeCount = way.refsSize()-1;
		if (state->cmd.addReverseEdges && isUndirectedEdge(state->cfg.implicitOneWay, ows, hwType)) {
			myEdgeCount *= 2;
		}
		state->edgeCount += myEdgeCount;
	}
};

///This marks ways invalid if a node is in state->osmIdToMyNodeId and in the way
struct InvalidWayMarkingProcessor {
	InvalidWayMarkingProcessor(StatePtr state) :
	state(state)
	{}
	
	StatePtr state;

	std::unordered_set<std::string> kS;
	inline const std::unordered_set<std::string> & keysToStore() const { return kS; }
	
	inline void operator()(int /*ows*/, int /*hwType*/, const std::unordered_map<std::string, std::string> & /*storedKv*/, const osmpbf::IWay & way) {
		for(osmpbf::IWayStream::RefIterator refIt(way.refBegin()), refEnd(way.refEnd()); refIt != refEnd; ++refIt) {
			if (state->osmIdToMyNodeId.count(*refIt)) {
				state->invalidWays.insert(way.id());
				return;
			}
		}
	}
};

struct NodeDegreeProcessor {
	NodeDegreeProcessor(StatePtr state) :
	state(state)
	{}
	
	StatePtr state;
	std::unordered_set<std::string> kS;
	inline const std::unordered_set<std::string> & keysToStore() const { return kS; }
	
	inline void operator()(int ows, int hwType, const std::unordered_map<std::string, std::string> & /*storedKv*/, const osmpbf::IWay & way) {
		if (state->invalidWays.count(way.id()) == 0) {
			bool undirectEdge = isUndirectedEdge(state->cfg.implicitOneWay, ows, hwType);
			osmpbf::IWayStream::RefIterator refSrc(way.refBegin());
			osmpbf::IWayStream::RefIterator refTg(way.refBegin()); ++refTg;
			osmpbf::IWayStream::RefIterator refEnd(way.refEnd());
			for(; refTg != refEnd; ++refTg, ++refSrc) {
				Edge e(state->osmIdToMyNodeId.at(*refSrc), state->osmIdToMyNodeId.at(*refTg), 1, hwType, 0);
				#ifdef CONFIG_SUPPORT_SSERIALIZE_OFFSET_ARRAY_TARGET
				state->nodes.at(e.source).outdegree += 1;
				state->nodes.at(e.target).indegree += 1;
				#endif
				if (state->cmd.addReverseEdges && undirectEdge) {
					e.reverse();
					#ifdef CONFIG_SUPPORT_SSERIALIZE_OFFSET_ARRAY_TARGET
					state->nodes.at(e.source).outdegree += 1;
					state->nodes.at(e.target).indegree += 1;
					#endif
				}
			}
		}
	};
};

struct FinalWayProcessor {
	FinalWayProcessor(StatePtr state, std::shared_ptr<GraphWriter> graphWriter, std::shared_ptr<WeightCalculator> weightCalculator) :
	state(state), graphWriter(graphWriter), weightCalculator(weightCalculator)
	{
		kS.insert("maxspeed");
	}
	
	StatePtr state;
	std::shared_ptr< GraphWriter > graphWriter;
	std::shared_ptr< WeightCalculator > weightCalculator;
	
	std::unordered_set<std::string> kS;
	inline const std::unordered_set<std::string> & keysToStore() const { return kS; }
	
	inline void operator()(int ows, int hwType, const std::unordered_map<std::string, std::string> & storedKv, const osmpbf::IWay & way) {
		if (state->invalidWays.count(way.id()) == 0) {
			int maxSpeed = 0;
			if (!storedKv.count("maxspeed") || !parseMaxSpeed(storedKv.at("maxspeed"), maxSpeed)) {
				maxSpeed = state->cfg.maxSpeedFromType(hwType);
			}
			#ifdef CONFIG_CREATOR_COPY_TAGS
			std::string tags = tags2json(way);
			#endif
			osmpbf::IWayStream::RefIterator refSrc(way.refBegin());
			osmpbf::IWayStream::RefIterator refTg(way.refBegin()); ++refTg;
			osmpbf::IWayStream::RefIterator refEnd(way.refEnd());
			for(; refTg != refEnd; ++refTg, ++refSrc) {
				Edge e(state->osmIdToMyNodeId.at(*refSrc), state->osmIdToMyNodeId.at(*refTg), 1, hwType, maxSpeed);
				#ifdef CONFIG_CREATOR_COPY_TAGS
				e.tags = tags;
				#endif
				e.weight = weightCalculator->calc(e);
				graphWriter->writeEdge(e);
				if (state->cmd.addReverseEdges && isUndirectedEdge(state->cfg.implicitOneWay, ows, hwType)) {
					graphWriter->writeEdge(e.reverse());
				}
			}
		}
	};
};

}}}//end namespace


#endif
