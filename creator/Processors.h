#ifndef OSM_GRAPH_TOOLS_PROCESSORS_H
#define OSM_GRAPH_TOOLS_PROCESSORS_H
#include <vendor/osmpbf/osmfile.h>
#include <vendor/osmpbf/iway.h>
#include <vendor/osmpbf/inode.h>
#include <vendor/osmpbf/filter.h>
#include <vendor/osmpbf/primitiveblockinputadaptor.h>
#include "types.h"
#include "conversion.h"
#include "GraphWriter.h"
#include "WeightCalculator.h"
#include <unordered_set>


namespace osm {
namespace graphtools {
namespace creator {

inline bool isUndirectedEdge(std::shared_ptr< std::unordered_set<int> > implicitOneWay, int ows, int hwType) {
	if (ows == OW_YES) {
		return false;
	}
	if (implicitOneWay->count(hwType)) {
		return (ows == OW_NO);
	}
	return true;
}

void gatherNodes(osmpbf::OSMFileIn & inFile, std::shared_ptr< std::unordered_set<int64_t> > nodeRefs, std::shared_ptr< std::vector<Node> > nodes, std::shared_ptr< std::unordered_map<int64_t, uint32_t> > osmIdToMyNodeId) {
	osmpbf::PrimitiveBlockInputAdaptor pbi;
	uint32_t nodeId = 0;
	inFile.dataSeek(0);
	sserialize::ProgressInfo progress;
	progress.begin(inFile.dataSize(), "Processing nodes");
	while (inFile.parseNextBlock(pbi)) {
		if (pbi.isNull())
			continue;
		progress(inFile.dataPosition());

		if (pbi.nodesSize()) {
			for (osmpbf::INodeStream node = pbi.getNodeStream(); !node.isNull(); node.next()) {
				int64_t osmId = node.id();
				if (nodeRefs->count(osmId) ) {
					nodeRefs->erase(osmId);
					Node n(nodeId, osmId, Coordinates(node.latd(), node.lond()), 0);
					nodes->push_back(n);
					++nodeId;
					(*osmIdToMyNodeId)[n.osmId] = n.id;
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
			}

			progress(inFile.dataPosition());
			if (pbi.waysSize()) {
				for (osmpbf::IWayStream way = pbi.getWayStream(); !way.isNull(); way.next()) {
					OneWayStatus ows = OW_IMPLICIT;
					int hwType = 0;
					uint8_t done = 2;
					bool process = false;
					for(int i = 0, s = way.tagsSize(); i < s; ++i) {
						uint32_t keyId = way.keyId(i);
						if (keyId == onewayTagId) {
							ows = (toBool(way.value(i) ) > 0 ? OW_YES : OW_NO);
							--done;
							if (done)
								continue;
							else
								break;
						}
						if (keyId == highwayTagId) {
							uint32_t valueId = way.valueId(i);
							if ( strIdToHwId.count( valueId ) && way.refsSize() > 1) {
								hwType = strIdToHwId[valueId];
								process = true;
							}
							else {
								break;
							}
							--done;
							if (done)
								continue;
							else
								break;
						}
					}
					if (process) {
						processor(ows, hwType, way);
					}
				}
			}
		}
		progress.end();
	}
};

struct RefGatherProcessor {
	RefGatherProcessor(std::shared_ptr< std::unordered_set<int64_t> > nodeRefs, std::shared_ptr< std::unordered_set<int> > implicitOneWay, uint64_t * edgeCount) :
	nodeRefs(nodeRefs), implicitOneWay(implicitOneWay), edgeCount(edgeCount) {}
	std::shared_ptr< std::unordered_set<int64_t> > nodeRefs;
	std::shared_ptr< std::unordered_set<int> > implicitOneWay;
	uint64_t * edgeCount;
	inline void operator()(int ows, int hwType, const osmpbf::IWay & way) {
		for(osmpbf::IWayStream::RefIterator refIt(way.refBegin()), refEnd(way.refEnd()); refIt != refEnd; ++refIt) {
			nodeRefs->insert(*refIt);
		}
		uint32_t myEdgeCount = way.refsSize()-1;
		if (isUndirectedEdge(implicitOneWay, ows, hwType)) {
			myEdgeCount *= 2;
		}
		*edgeCount += myEdgeCount;
	}
};

struct InvalidWayMarkingProcessor {
	InvalidWayMarkingProcessor(std::shared_ptr< std::unordered_set<int64_t> > nodeRefs,
									std::shared_ptr< std::unordered_set<int64_t> > invalidWays,
									std::shared_ptr< std::unordered_set<int> > implicitOneWay, uint64_t * edgeCount) :
	nodeRefs(nodeRefs), invalidWays(invalidWays), implicitOneWay(implicitOneWay), edgeCount(edgeCount) {}
	
	std::shared_ptr< std::unordered_set<int64_t> > nodeRefs;
	std::shared_ptr< std::unordered_set<int64_t> > invalidWays;
	std::shared_ptr< std::unordered_set<int> > implicitOneWay;
	uint64_t * edgeCount;

	inline void operator()(int ows, int hwType, const osmpbf::IWay & way) {
		bool invalid = false;
		for(osmpbf::IWayStream::RefIterator refIt(way.refBegin()), refEnd(way.refEnd()); refIt != refEnd; ++refIt) {
			if (nodeRefs->count(*refIt)) {
				invalid = true;
				break;
			}
		}
		if (invalid) {
			uint32_t myEdgeCount = way.refsSize()-1;
			if (isUndirectedEdge(implicitOneWay, ows, hwType)) {
				myEdgeCount *= 2;
			}
			*edgeCount -= myEdgeCount;
		}
	}
};

struct FinalWayProcessor {
	FinalWayProcessor(std::shared_ptr< std::unordered_map<int64_t, uint32_t> > osmIdToMyNodeId,
						std::shared_ptr< std::unordered_set<int64_t> > invalidWays,
						std::shared_ptr< std::unordered_set<int> > implicitOneWay,
						std::shared_ptr<GraphWriter> graphWriter,
						std::shared_ptr< std::vector<Node> > nodes, std::shared_ptr<WeightCalculator> weightCalculator) :
	osmIdToMyNodeId(osmIdToMyNodeId), invalidWays(invalidWays), implicitOneWay(implicitOneWay), graphWriter(graphWriter), nodes(nodes), weightCalculator(weightCalculator) {}
	
	std::shared_ptr< std::unordered_map<int64_t, uint32_t> > osmIdToMyNodeId;
	std::shared_ptr< std::unordered_set<int64_t> > invalidWays;
	std::shared_ptr< std::unordered_set<int> > implicitOneWay;
	std::shared_ptr< GraphWriter > graphWriter;
	std::shared_ptr< std::vector<Node> > nodes;
	std::shared_ptr< WeightCalculator > weightCalculator;
	
	inline void operator()(int ows, int hwType, const osmpbf::IWay & way) {
		if (invalidWays->count(way.id()) == 0) {
			bool undirectEdge = isUndirectedEdge(implicitOneWay, ows, hwType);
			osmpbf::IWayStream::RefIterator refSrc(way.refBegin());
			osmpbf::IWayStream::RefIterator refTg(way.refBegin()); ++refTg;
			osmpbf::IWayStream::RefIterator refEnd(way.refEnd());
			for(; refTg != refEnd; ++refTg, ++refSrc) {
				Edge e(osmIdToMyNodeId->at(*refSrc), osmIdToMyNodeId->at(*refTg), 1, hwType);
				e.weight = weightCalculator->calc(nodes->at(e.source).coordinates, nodes->at(e.target).coordinates, e.type);
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