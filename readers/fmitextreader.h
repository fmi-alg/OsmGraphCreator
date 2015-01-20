#ifndef OSM_GRAPH_CREATOR_FMI_TEXT_READER_H
#define OSM_GRAPH_CREATOR_FMI_TEXT_READER_H
#include <stdint.h>
#include <string>

namespace OsmGraphWriter {


class FmiTextReader {
public:
	typedef enum {GT_STANDARD, GT_MAXSPEED, GT_UNDEFINED} GraphType;
	void readGraph(std::istream & input);
public:
	FmiTextReader();
	virtual ~FmiTextReader();
	///throws great error messages and eats your kitten afterwards
	void read(char* path);
	virtual void header(GraphType type, int32_t nodeCount, int32_t edgeCount) = 0;
	virtual void node(int32_t nodeId, int64_t osmId, double lat, double lon, int32_t elev, int32_t stringCarryOverSize, const char * stringCarryOver) = 0;
	///@param maxSpeed 0 if type == GT_STANDARD
	virtual void edge(int32_t source, int32_t target, int32_t weight, int32_t type, int32_t maxSpeed, int32_t stringCarryOverSize, const char * stringCarryOver) = 0;
};


}//end namespace
#endif