#include <iostream>
#include "fmibinaryreader.h"

class MyGraphReader: public OsmGraphWriter::FmiBinaryReader {
private:
	std::ostream & out() { return std::cout; }
	GraphType m_gt;
public:
	MyGraphReader() { out().precision(15); out() << std::fixed;}
	~MyGraphReader() {}
	virtual void header(GraphType type, int32_t nodeCount, int32_t edgeCount) {
		out() << "# Id : 0" << std::endl;
		out() << "# Timestamp : " << time(0) << std::endl;
		out() << "# Type : " << (type == GT_STANDARD ? "standard" : "maxspeed") << std::endl;
		out() << "# Revision: 1 " << std::endl;
		out() << nodeCount << std::endl;
		out() << edgeCount << std::endl;
		m_gt = type;
	}
	virtual void edge(int32_t source, int32_t target, int32_t weight, int32_t type, int32_t maxSpeed, int32_t stringCarryOverSize, const uint8_t* stringCarryOver) {
		out() << source << " " << target << " " << weight << " " << type;
		if (m_gt == GT_MAXSPEED) {
			out() << " " << maxSpeed;
		}
		out() << " " << stringCarryOverSize << " ";
		if (stringCarryOverSize); {
			out().write((const char*)stringCarryOver, stringCarryOverSize);
		}
		out() << "\n";
	}
	virtual void node(int32_t nodeId, int64_t osmId, double lat, double lon, int32_t elev, int32_t stringCarryOverSize, const uint8_t* stringCarryOver) {
		out() << nodeId << " " << osmId << " " << lat << " " << lon << " " << elev << " " << stringCarryOverSize << " ";
		if (stringCarryOverSize) {
			out().write((const char*)stringCarryOver, stringCarryOverSize);
		}
		out() << "\n";
	}
    
};

int main(int argc, char ** argv) {
	if (argc < 2) {
		std::cout << "Not enough arguments. Need filename\n";
	}
	

}