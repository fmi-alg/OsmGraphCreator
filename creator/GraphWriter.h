#ifndef OSM_GRAPH_TOOLS_GRAPH_WRITER_H
#define OSM_GRAPH_TOOLS_GRAPH_WRITER_H
#include "types.h"
#include "RamGraph.h"
#include <sserialize/stats/ProgressInfo.h>
#include <sserialize/Static/DynamicFixedLengthVector.h>
#include <ostream>
#include <algorithm>

namespace osm {
namespace graphtools {
namespace creator {

struct GraphWriter {
	virtual ~GraphWriter() {}
	
	virtual void beginGraph() {}
	virtual void beginHeader() {}
	virtual void endHeader() {}
	virtual void beginNodes() {}
	virtual void endNodes() {}
	virtual void beginEdges() {}
	virtual void endEdges() {}
	virtual void endGraph() {}

	virtual void writeHeader(uint64_t nodeCount, uint64_t edgeCount) = 0;
	virtual void writeNode(const Node & node, const Coordinates & coordinates) = 0;
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

struct DropGraphWriter: public GraphWriter {
	~DropGraphWriter() override {}
	void writeHeader(uint64_t nodeCount, uint64_t edgeCount) override {}
	void writeNode(const Node & node, const Coordinates & coordinates) override {}
	void writeEdge(const Edge & edge) override {}
};

class TopologyTextGraphWriter: public GraphWriter {
private:
	std::shared_ptr<std::ostream> m_out;
protected:
	inline std::ostream & out() { return *m_out; }
public:
	TopologyTextGraphWriter(std::shared_ptr<std::ostream> out);
	virtual ~TopologyTextGraphWriter();
	virtual void writeHeader(uint64_t nodeCount, uint64_t edgeCount);
	virtual void writeNode(const Node & node, const Coordinates & coordinates);
	virtual void writeEdge(const Edge & edge);
};

class FmiTextGraphWriter: public GraphWriter {
private:
	std::shared_ptr<std::ostream> m_out;
protected:
	inline std::ostream & out() { return *m_out; }
public:
	FmiTextGraphWriter(std::shared_ptr<std::ostream> out);
	virtual ~FmiTextGraphWriter();
	virtual void writeHeader(uint64_t nodeCount, uint64_t edgeCount);
	virtual void writeNode(const Node & node, const Coordinates & coordinates);
	virtual void writeEdge(const Edge & edge);
};

class FmiMaxSpeedTextGraphWriter: public FmiTextGraphWriter {
public:
	FmiMaxSpeedTextGraphWriter(std::shared_ptr<std::ostream> out);
	virtual ~FmiMaxSpeedTextGraphWriter();
	virtual void writeHeader(uint64_t nodeCount, uint64_t edgeCount);
	virtual void writeEdge(const Edge & edge);
};

class FmiBinaryGraphWriter: public GraphWriter {
private:
	std::shared_ptr<std::ostream> m_out;
protected:
	inline std::ostream & out() { return *m_out; }
public:
	FmiBinaryGraphWriter(std::shared_ptr<std::ostream> out);
	virtual ~FmiBinaryGraphWriter();
	void putInt(int32_t v);
	void putLong(int64_t v);
	void putDouble(double v);
	virtual void writeHeader(uint64_t nodeCount, uint64_t edgeCount);
	virtual void writeNode(const Node & node, const Coordinates & coordinates);
	virtual void writeEdge(const Edge & edge);
};

class FmiMaxSpeedBinaryGraphWriter: public FmiBinaryGraphWriter {
public:
	FmiMaxSpeedBinaryGraphWriter(std::shared_ptr<std::ostream> out);
	virtual ~FmiMaxSpeedBinaryGraphWriter();
	virtual void writeHeader(uint64_t nodeCount, uint64_t edgeCount);
	virtual void writeEdge(const Edge & edge);
};

class SortedEdgeWriter: public GraphWriter {
private:
	std::shared_ptr<GraphWriter> m_baseGraphWriter;
	std::vector<Edge> m_edges;
public:
	SortedEdgeWriter(std::shared_ptr<GraphWriter> & baseGraphWriter) : m_baseGraphWriter(baseGraphWriter) {}
	virtual ~SortedEdgeWriter() {}
	virtual void beginGraph() {m_baseGraphWriter->beginGraph();}
	virtual void beginHeader() {m_baseGraphWriter->beginHeader();}
	virtual void endHeader() {m_baseGraphWriter->endHeader();}
	virtual void beginNodes() {m_baseGraphWriter->beginNodes();}
	virtual void endNodes() {m_baseGraphWriter->endNodes();}
	virtual void beginEdges() {}
	virtual void endEdges() {
		std::sort(m_edges.begin(), m_edges.end(), [](const Edge & e1, const Edge & e2) {
			return (e1.source == e2.source ? e1.target < e2.target : e1.source < e2.source);
		});
		m_baseGraphWriter->beginEdges();
		for(const Edge & e : m_edges) {
			m_baseGraphWriter->writeEdge(e);
		}
		m_baseGraphWriter->endEdges();
		m_edges = std::vector<Edge>();
	}
	virtual void endGraph() {m_baseGraphWriter->endGraph();}

	virtual void writeHeader(uint64_t nodeCount, uint64_t edgeCount) {
		m_edges.reserve(edgeCount);
		m_baseGraphWriter->writeHeader(nodeCount, edgeCount);
	}
	virtual void writeNode(const Node & node, const Coordinates & coordinates) { m_baseGraphWriter->writeNode(node, coordinates); };
	virtual void writeEdge(const Edge & edge) { m_edges.push_back(edge); }
};

class RamGraphWriter: public GraphWriter {
	sserialize::UByteArrayAdapter m_data;
	osm::graphs::ram::RamGraph m_graph;
	osm::graphs::ram::EdgeContainerSizeType m_edgeBegin;
public:
	RamGraphWriter(const sserialize::UByteArrayAdapter & data);
	virtual ~RamGraphWriter();
	virtual void endGraph();
	virtual void writeHeader(uint64_t nodeCount, uint64_t edgeCount);
	virtual void writeNode(const graphtools::creator::Node & node, const Coordinates & coordinates);
	virtual void writeEdge(const graphtools::creator::Edge & edge);
	osm::graphs::ram::RamGraph & graph();
};

///Writes each connected component into an extra file
class CCGraphWriter: public GraphWriter {
public:
	using CCId = uint32_t;
	///A factory that creates a new graph writer for the given connected component id
	using GraphWriterFactory = std::function<std::shared_ptr<GraphWriter>(CCId)>;
public:
	CCGraphWriter(GraphWriterFactory factory);
	~CCGraphWriter() override;
	void endGraph() override;
	void writeHeader(uint64_t nodeCount, uint64_t edgeCount) override;
	void writeNode(const graphtools::creator::Node & node, const Coordinates & coordinates) override;
	void writeEdge(const graphtools::creator::Edge & edge) override;
private:
	std::vector< std::pair<Node, Coordinates> > m_nodes;
	std::vector<Edge> m_edges;
	GraphWriterFactory m_f;
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
	virtual void writeNode(const graphtools::creator::Node & node, const Coordinates & coordinates);
	virtual void writeEdge(const graphtools::creator::Edge & edge);
};

class PlotGraph: public graphtools::creator::GraphWriter {
private:
	std::shared_ptr<std::ostream> m_out;
	std::vector<Coordinates> m_nodes;
protected:
	inline std::ostream & out() { return *m_out; }
public:
	PlotGraph(std::shared_ptr<std::ostream> out);
	virtual ~PlotGraph();
	virtual void writeHeader(uint64_t nodeCount, uint64_t edgeCount);
	virtual void writeNode(const graphtools::creator::Node & node, const Coordinates & coordinates);
	virtual void writeEdge(const graphtools::creator::Edge & edge);
};

}}}//end namespace


#endif
