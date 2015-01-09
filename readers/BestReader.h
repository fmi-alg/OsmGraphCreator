
/** Binary format:
  *
  * --------------------------------------------------
  * MAGICOOKIE             |VERSION|
  * --------------------------------------------------
  * STINKEFINGER IN UNICODE|u32    |
  *
  *
  */


class FmiBestBinaryReader {
public:
	typedef enum {GT_INVALID=0x0, GT_STANDARD=0x1, GT_MAXSPEED=0x2} GraphType;
	struct Header {
		GraphType gt;
		int64_t nodeCount;
		int64_t edgeCount;
	};
	struct Node {
		int64_t id;
		int64_t osmId;
		double lon; //wgs84
		double lat; //wgs84
		int32_t height; //in meter
	};
	struct Edge {
		int64_t source;
		int64_t target;
		int32_t type;
		double weight;
	};
protected:
	int32_t getInt32(char*& offset);
	int64_t getInt64(char*& offset);
	double getDouble(char*& offset);
	void readGraph(char* inBegin, char* end);
public:
	FmiBestBinaryReader();
	virtual ~FmiBestBinaryReader();
	///throws great error messages and eats your kitten afterwards
	void read(char* path);
	virtual void header(const Header & header) = 0;
	virtual void node(const Node & node) = 0;
	virtual void edge(const Edge & edge) = 0;
};