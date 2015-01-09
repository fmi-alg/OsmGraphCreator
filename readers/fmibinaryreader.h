#include <stdint.h>

namespace OsmGraphWriter {


class FmiBinaryReader {
protected:
	int32_t getInt32(uint8_t* &offset);
	int64_t getInt64(uint8_t* &offset);
	double getDouble(uint8_t* &offset);
	///@return true if everything ok
	virtual bool readGraph(uint8_t * data, uint8_t * end) = 0;
public:
	FmiBinaryReader();
	virtual ~FmiBinaryReader();
	bool read(char* path);
};

}//end namespace