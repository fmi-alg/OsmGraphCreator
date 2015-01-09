#include <stdint.h>
#include <string>

namespace OsmGraphWriter {


class FmiBinaryReader {
public:
	typedef enum {GT_STANDARD, GT_MAXSPEED} GraphType;
protected:
	int32_t getInt32(char*& offset);
	int64_t getInt64(char*& offset);
	double getDouble(char*& offset);
	void readGraph(char* inBegin, char* end);
public:
	FmiBinaryReader();
	virtual ~FmiBinaryReader();
	///throws great error messages and eats your kitten afterwards
	void read(char* path);
	virtual void header(GraphType type, int32_t nodeCount, int32_t edgeCount) = 0;
	virtual void node(int32_t nodeId, int64_t osmId, double lat, double lon, int32_t elev, int32_t stringCarryOverSize, const char * stringCarryOver) = 0;
	///@param maxSpeed 0 if type == GT_STANDARD
	virtual void edge(int32_t source, int32_t target, int32_t weight, int32_t type, int32_t maxSpeed, int32_t stringCarryOverSize, const char * stringCarryOver) = 0;
};


}//end namespace