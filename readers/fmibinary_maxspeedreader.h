#include "fmibinaryreader.h"

namespace OsmGraphWriter {

class FmiMaxSpeedBinaryReader: public FmiBinaryReader {
protected:
	virtual bool readGraph(uint8_t* data, uint8_t * end);
public:
	FmiMaxSpeedBinaryReader();
	virtual ~FmiMaxSpeedBinaryReader();
	virtual void header(int32_t nodeCount, int32_t edgeCount);
	virtual void node(int32_t nodeId, int64_t osmId, double lat, double lon, int32_t elev, int32_t stringCarryOverSize, const uint8_t * stringCarryOver);
	virtual void edge(int32_t source, int32_t target, int32_t weight, int32_t type, int32_t maxSpeed, int32_t stringCarryOverSize, const uint8_t * stringCarryOver);
};

}//end namespace