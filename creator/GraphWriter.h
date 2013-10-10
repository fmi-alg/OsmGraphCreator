#ifndef OSM_GRAPH_TOOLS_GRAPH_WRITER_H
#define OSM_GRAPH_TOOLS_GRAPH_WRITER_H
#include "types.h"
#include "RamGraph.h"
#include <sserialize/utility/ProgressInfo.h>
#include <sserialize/Static/DynamicFixedLengthVector.h>
#include <ostream>

namespace osm {
namespace graphtools {
namespace creator {

struct GraphWriter {
	virtual void beginGraph() {}
	virtual void beginHeader() {}
	virtual void endHeader() {}
	virtual void beginNodes() {}
	virtual void endNodes() {}
	virtual void beginEdges() {}
	virtual void endEdges() {}
	virtual void endGraph() {}

	virtual void writeHeader(uint64_t nodeCount, uint64_t edgeCount) = 0;
	virtual void writeNode(const Node & node) = 0;
	virtual void writeEdge(const Edge & edge) = 0;
	template<typename TIterator>
	void writeNodes(TIterator begin, TIterator end) {
		sserialize::ProgressInfo progress;
		progress.begin(end-begin, "Writing out nodes");
		for(TIterator it(begin); it != end; ++it) {
			writeNode(*it);
			progress(it-begin);
		}
		progress.end();
	}
	template<typename TIterator>
	void writeEdges(TIterator begin, TIterator end) {
		sserialize::ProgressInfo progress;
		progress.begin(end-begin, "Writing out edges");
		for(TIterator it(begin); it != end; ++it) {
			writeEdge(*it);
			progress(it-begin);
		}
		progress.end();
	}
};

class FmiTextGraphWriter: public GraphWriter {
private:
	std::ostream & m_out;
protected:
	inline std::ostream & out() { return m_out; }
public:
	FmiTextGraphWriter(std::ostream & out);
	virtual ~FmiTextGraphWriter();
	virtual void writeHeader(uint64_t nodeCount, uint64_t edgeCount);
	virtual void writeNode(const Node & node);
	virtual void writeEdge(const Edge & edge);
};

class FmiMaxSpeedTextGraphWriter: public FmiTextGraphWriter {
public:
	FmiMaxSpeedTextGraphWriter(std::ostream & out);
	virtual ~FmiMaxSpeedTextGraphWriter();
	virtual void writeHeader(uint64_t nodeCount, uint64_t edgeCount);
	virtual void writeEdge(const Edge & edge);
};

class FmiBinaryGraphWriter: public GraphWriter {
private:
	std::ostream & m_out;
protected:
	inline std::ostream & out() { return m_out; }
public:
	FmiBinaryGraphWriter(std::ostream & out);
	virtual ~FmiBinaryGraphWriter();
	void putInt(int32_t v);
	void putLong(int64_t v);
	void putDouble(double v);
	virtual void writeHeader(uint64_t nodeCount, uint64_t edgeCount);
	virtual void writeNode(const Node & node);
	virtual void writeEdge(const Edge & edge);
};

class FmiMaxSpeedBinaryGraphWriter: public FmiBinaryGraphWriter {
public:
	FmiMaxSpeedBinaryGraphWriter(std::ostream & out);
	virtual ~FmiMaxSpeedBinaryGraphWriter();
	virtual void writeHeader(uint64_t nodeCount, uint64_t edgeCount);	
	virtual void writeEdge(const Edge & edge);
};

class RamGraphWriter: public graphtools::creator::GraphWriter {
	sserialize::UByteArrayAdapter m_data;
	osm::graphs::ram::RamGraph m_graph;
	osm::graphs::ram::EdgeContainerSizeType m_edgeBegin;
public:
	RamGraphWriter(const sserialize::UByteArrayAdapter & data);
	virtual ~RamGraphWriter();
	virtual void endGraph();
	osm::graphs::ram::RamGraph & graph();
	virtual void writeHeader(uint64_t nodeCount, uint64_t edgeCount);
	virtual void writeNode(const graphtools::creator::Node & node);
	virtual void writeEdge(const graphtools::creator::Edge & edge);
};

class StaticGraphWriter: public graphtools::creator::GraphWriter {
private:
	sserialize::UByteArrayAdapter m_data;
	sserialize::Static::DynamicFixedLengthVector<osm::graphs::ram::Node> m_nodes;
	sserialize::Static::DynamicFixedLengthVector<osm::graphs::ram::Edge> m_edges;
	uint32_t m_edgeBegin;
	std::vector<uint32_t> m_edgeOffsets; 
public:
	StaticGraphWriter(const sserialize::UByteArrayAdapter & data);
	virtual ~StaticGraphWriter();
	virtual void writeHeader(uint64_t nodeCount, uint64_t edgeCount);
	virtual void writeNode(const graphtools::creator::Node & node);
	virtual void writeEdge(const graphtools::creator::Edge & edge);
};

class PlotGraph: public graphtools::creator::GraphWriter {
private:
	std::ostream & m_out;
	std::vector<Coordinates> m_nodes;
protected:
	inline std::ostream & out() { return m_out; }
public:
	PlotGraph(std::ostream & out);
	virtual ~PlotGraph();
	virtual void writeHeader(uint64_t nodeCount, uint64_t edgeCount);
	virtual void writeNode(const graphtools::creator::Node & node);
	virtual void writeEdge(const graphtools::creator::Edge & edge);
};

}}}//end namespace


#endif