#ifndef OSM_GRAPH_TOOLS_WEIGHT_CALCULATOR_H
#define OSM_GRAPH_TOOLS_WEIGHT_CALCULATOR_H
#include <unordered_map>
#include <sserialize/spatial/DistanceCalculator.h>
#include "types.h"

namespace osm {
namespace graphtools {
namespace creator {

struct WeightCalculator {
	virtual int calc(const Edge & edge) = 0;
};

struct NoWeightCalculator: public WeightCalculator {
	virtual int calc(const Edge & edges);
};

struct GeodesicDistanceWeightCalculator: public WeightCalculator {
	GeodesicDistanceWeightCalculator(StatePtr state) : state(state)  {}
	StatePtr state;
	sserialize::spatial::detail::GeodesicDistanceCalculator distCalc;
	
	virtual int calc(const Edge & edge);
};

struct WeightedGeodesicDistanceWeightCalculator: public WeightCalculator {
	WeightedGeodesicDistanceWeightCalculator(StatePtr state) : state(state)  {}
	StatePtr state;
	sserialize::spatial::detail::GeodesicDistanceCalculator distCalc;
	std::shared_ptr< std::unordered_map<int, double> > typeToWeight;
	
	virtual int calc(const Edge & edge);
};

struct MaxSpeedGeodesicDistanceWeightCalculator: public WeightCalculator {
	MaxSpeedGeodesicDistanceWeightCalculator(StatePtr state) : state(state) {}
	StatePtr state;
	sserialize::spatial::detail::GeodesicDistanceCalculator distCalc;
	std::shared_ptr< std::unordered_map<int, double> > typeToWeight;
	
	virtual int calc(const Edge & edge);
};

}}}//end namespace
#endif