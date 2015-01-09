#include "fmibinary_maxspeedreader.h"

namespace OsmGraphWriter {

FmiMaxSpeedBinaryReader::FmiMaxSpeedBinaryReader() {}
FmiMaxSpeedBinaryReader::~FmiMaxSpeedBinaryReader() {}
bool FmiMaxSpeedBinaryReader::readGraph(uint8_t* begin, uint8_t * end) {
	//skip the text header
	for(uint8_t nlc = 0; begin < end && nlc < 4;) {
		if (*begin == '\n') {
			++nlc;
		}
		++begin;
	}
	//begin now points to the first "real" data
	
	//header
	int32_t nodeCount = getInt32(begin);
	int32_t edgeCount = getInt32(begin);
	header(nodeCount, edgeCount);
	
	{//nodes
		int32_t nodeId;
		int64_t osmId;
		double lat, lon;
		int32_t elev;
		int32_t stringCarryOverSize;
		int32_t i(0);
		for(; i < nodeCount && begin < end; ++i) {
			nodeId = getInt32(begin);
			osmId = getInt64(begin);
			lat = getDouble(begin);
			lon = getDouble(begin);
			elev = getInt32(begin);
			stringCarryOverSize = getInt32(begin);
			if (begin+stringCarryOverSize <= end) {
				node(nodeId, osmId, lat, lon, elev, stringCarryOverSize, begin);
			}
			begin += stringCarryOverSize;
		}
		if (i < nodeCount) {
			return false;
		}
	}
	{//edges
		int32_t source;
		int32_t target;
		int32_t weight;
		int32_t type;
		int32_t maxSpeed;
		int32_t stringCarryOverSize;
		int32_t i(0);
		for(; i < edgeCount && begin < end; ++i) {
			source = getInt32(begin);
			target = getInt32(begin);
			weight = getInt32(begin);
			type = getInt32(begin);
			maxSpeed = getInt32(begin);
			stringCarryOverSize = getInt32(begin);
			if (begin+stringCarryOverSize <= end) {
				node(source, target, weight, type, maxSpeed, stringCarryOverSize, begin);
			}
			begin += stringCarryOverSize;
		}
		if (i < edgeCount) {
			return false;
		}
	}
	return true;
}




}