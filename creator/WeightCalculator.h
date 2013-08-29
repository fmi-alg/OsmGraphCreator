#ifndef OSM_GRAPH_TOOLS_WEIGHT_CALCULATOR_H
#define OSM_GRAPH_TOOLS_WEIGHT_CALCULATOR_H
#include <unordered_map>
#include "types.h"

namespace osm {
namespace graphtools {
namespace creator {

struct WeightCalculator {
	virtual int calc(const Coordinates & src, const Coordinates & dest, int type) = 0;
};

struct NoWeightCalculator: public WeightCalculator {
	virtual int calc(const Coordinates & src, const Coordinates & dest, int type);
};

struct GeodesicDistanceWeightCalculator: public WeightCalculator {
	virtual int calc(const Coordinates & src, const Coordinates & dest, int type);
};

struct WeightedGeodesicDistanceWeightCalculator: public WeightCalculator {
	std::shared_ptr< std::unordered_map<int, double> > typeToWeight;
	virtual int calc(const Coordinates & src, const Coordinates & dest, int type);
};

}}}//end namespace
#endif