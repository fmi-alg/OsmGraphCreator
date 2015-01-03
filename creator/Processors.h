#ifndef OSM_GRAPH_TOOLS_PROCESSORS_H
#define OSM_GRAPH_TOOLS_PROCESSORS_H
#include <osmpbf/osmfile.h>
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


namespace osm {
namespace graphtools {
namespace creator {

inline bool isUndirectedEdge(const std::unordered_set<int> & implicitOneWay, int ows, int hwType) {
	if (ows == OW_YES) {
		return false;
	}
	if (implicitOneWay.count(hwType)) {
		return (ows == OW_NO);
	}
	return true;
}

inline void gatherNodes(osmpbf::OSMFileIn & inFile, std::unordered_set<int64_t> & nodeRefs, StatePtr state) {
	osmpbf::PrimitiveBlockInputAdaptor pbi;
	uint32_t nodeId = 0;
	inFile.dataSeek(0);
	sserialize::ProgressInfo progress;
	progress.begin(nodeRefs.size(), "Gathering nodes");
	while (inFile.parseNextBlock(pbi) && nodeRefs.size()) {
		if (pbi.isNull())
			continue;
		progress(nodeId);

		if (pbi.nodesSize()) {
			for (osmpbf::INodeStream node = pbi.getNodeStream(); !node.isNull(); node.next()) {
				int64_t osmId = node.id();
				if (nodeRefs.count(osmId) ) {
					nodeRefs.erase(osmId);
					Node n(nodeId, osmId, 0);
					state->nodes.push_back(n);
					state->nodeCoordinates.push_back(Coordinates(node.latd(), node.lond()));
					++nodeId;
					state->osmIdToMyNodeId[n.osmId] = n.id;
				}
			}
		}
	}
	progress.end();
}

struct WayParser {
	WayParser(const std::string & message, osmpbf::OSMFileIn & inFile, const std::unordered_map<std::string, int> & hwTagIds) :
	message(message), inFile(inFile), hwTagIds(hwTagIds) {}
	std::string message;
	osmpbf::OSMFileIn & inFile;
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

struct RefGatherProcessor {
	RefGatherProcessor(StatePtr state, std::shared_ptr< std::unordered_set<int64_t> > nodeRefs, uint64_t * edgeCount) :
	state(state), nodeRefs(nodeRefs), edgeCount(edgeCount) {}
	StatePtr state;
	std::shared_ptr< std::unordered_set<int64_t> > nodeRefs;
	uint64_t * edgeCount;
	
	std::unordered_set<std::string> kS;
	inline const std::unordered_set<std::string> & keysToStore() const { return kS; }
	
	inline void operator()(int ows, int hwType, const std::unordered_map<std::string, std::string> & storedKv, const osmpbf::IWay & way) {
		for(osmpbf::IWayStream::RefIterator refIt(way.refBegin()), refEnd(way.refEnd()); refIt != refEnd; ++refIt) {
			nodeRefs->insert(*refIt);
		}
		uint32_t myEdgeCount = way.refsSize()-1;
		if (isUndirectedEdge(state->cfg.implicitOneWay, ows, hwType)) {
			myEdgeCount *= 2;
		}
		*edgeCount += myEdgeCount;
	}
};

struct InvalidWayMarkingProcessor {
	InvalidWayMarkingProcessor(StatePtr state, std::shared_ptr< std::unordered_set<int64_t> > unfetchedNodeRefs, uint64_t * edgeCount) :
	state(state), unfetchedNodeRefs(unfetchedNodeRefs), edgeCount(edgeCount)
	{
	}
	
	StatePtr state;
	std::shared_ptr< std::unordered_set<int64_t> > unfetchedNodeRefs;
	uint64_t * edgeCount;

	std::unordered_set<std::string> kS;
	inline const std::unordered_set<std::string> & keysToStore() const { return kS; }
	
	inline void operator()(int ows, int hwType, const std::unordered_map<std::string, std::string> & storedKv, const osmpbf::IWay & way) {
		bool invalid = false;
		for(osmpbf::IWayStream::RefIterator refIt(way.refBegin()), refEnd(way.refEnd()); refIt != refEnd; ++refIt) {
			if (unfetchedNodeRefs->count(*refIt)) {
				invalid = true;
				break;
			}
		}
		if (invalid) {
			uint32_t myEdgeCount = way.refsSize()-1;
			if (isUndirectedEdge(state->cfg.implicitOneWay, ows, hwType)) {
				myEdgeCount *= 2;
			}
			*edgeCount -= myEdgeCount;
			state->invalidWays.insert(way.id());
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
	
	inline void operator()(int ows, int hwType, const std::unordered_map<std::string, std::string> & storedKv, const osmpbf::IWay & way) {
		if (state->invalidWays.count(way.id()) == 0) {
			bool undirectEdge = isUndirectedEdge(state->cfg.implicitOneWay, ows, hwType);
			osmpbf::IWayStream::RefIterator refSrc(way.refBegin());
			osmpbf::IWayStream::RefIterator refTg(way.refBegin()); ++refTg;
			osmpbf::IWayStream::RefIterator refEnd(way.refEnd());
			for(; refTg != refEnd; ++refTg, ++refSrc) {
				Edge e(state->osmIdToMyNodeId.at(*refSrc), state->osmIdToMyNodeId.at(*refTg), 1, hwType, 0);
				state->nodes.at(e.source).outdegree += 1;
				state->nodes.at(e.target).indegree += 1;
				if (undirectEdge) {
					e.reverse();
					state->nodes.at(e.source).outdegree += 1;
					state->nodes.at(e.target).indegree += 1;
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
			bool undirectEdge = isUndirectedEdge(state->cfg.implicitOneWay, ows, hwType);
			int maxSpeed = 0;
			if (!storedKv.count("maxspeed") || !parseMaxSpeed(storedKv.at("maxspeed"), maxSpeed)) {
				maxSpeed = state->cfg.maxSpeedFromType(hwType);
			}
			osmpbf::IWayStream::RefIterator refSrc(way.refBegin());
			osmpbf::IWayStream::RefIterator refTg(way.refBegin()); ++refTg;
			osmpbf::IWayStream::RefIterator refEnd(way.refEnd());
			for(; refTg != refEnd; ++refTg, ++refSrc) {
				Edge e(state->osmIdToMyNodeId.at(*refSrc), state->osmIdToMyNodeId.at(*refTg), 1, hwType, maxSpeed);
				e.weight = weightCalculator->calc(e);
				graphWriter->writeEdge(e);
				if (undirectEdge) {
					graphWriter->writeEdge(e.reverse());
				}
			}
		}
	};
};

}}}//end namespace


#endif