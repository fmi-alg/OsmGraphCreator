#include <iostream>
#include <limits>
#include <iomanip>
#include "fmibinaryreader.h"

class MyGraphReader: public OsmGraphWriter::FmiBinaryReader {
private:
	std::ostream & out() { return std::cout; }
	GraphType m_gt;
public:
	MyGraphReader() { out() << std::fixed << std::setprecision(std::numeric_limits<double>::digits10 + 2);}
	~MyGraphReader() {}
	virtual void header(GraphType type, int32_t nodeCount, int32_t edgeCount) {
		out() << "# Id : 0" << std::endl;
		out() << "# Timestamp : " << time(0) << std::endl;
		out() << "# Type : " << (type == GT_STANDARD ? "standard" : "maxspeed") << std::endl;
		out() << "# Revision : 1\n\n";
		out() << nodeCount << std::endl;
		out() << edgeCount << std::endl;
		m_gt = type;
	}
	virtual void edge(int32_t source, int32_t target, int32_t weight, int32_t type, int32_t maxSpeed, int32_t stringCarryOverSize, const char* stringCarryOver) {
		out() << source << " " << target << " " << weight << " " << type;
		if (m_gt == GT_MAXSPEED) {
			out() << " " << maxSpeed;
		}
		if (stringCarryOverSize) {
			out() << " ";
			out().write(stringCarryOver, stringCarryOverSize);
		}
		out() << "\n";
	}
	virtual void node(int32_t nodeId, int64_t osmId, double lat, double lon, int32_t elev, int32_t stringCarryOverSize, const char* stringCarryOver) {
		out() << nodeId << " " << osmId << " " << lat << " " << lon << " " << elev;
		if (stringCarryOverSize) {
			out() << " ";
			out().write(stringCarryOver, stringCarryOverSize);
		}
		out() << "\n";
	}
};

int main(int argc, char ** argv) {
	if (argc < 2) {
		std::cerr << "Not enough arguments. Need filename\n";
	}
	MyGraphReader * gr = new MyGraphReader();
	try {
		gr->read(argv[1]);
	}
	catch (const std::exception & e) {
		std::cerr << "Failed to read the graph: " << e.what() << std::endl;
		return -1;
	}
	return 0;
}