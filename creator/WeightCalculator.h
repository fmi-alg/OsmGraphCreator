#ifndef OSM_GRAPH_TOOLS_WEIGHT_CALCULATOR_H
#define OSM_GRAPH_TOOLS_WEIGHT_CALCULATOR_H
#include <unordered_map>
#include <sserialize/spatial/DistanceCalculator.h>
#include "types.h"

namespace osm {
namespace graphtools {
namespace creator {

struct WeightCalculator {
	virtual ~WeightCalculator() {}
	virtual int calc(const Edge & edge) = 0;
};

struct NoWeightCalculator: public WeightCalculator {
	~NoWeightCalculator() override {}
	int calc(const Edge & edges) override;
};

struct GeodesicDistanceWeightCalculator: public WeightCalculator {
	GeodesicDistanceWeightCalculator(StatePtr state) : state(state)  {}
	~GeodesicDistanceWeightCalculator() override {}
	StatePtr state;
	sserialize::spatial::detail::GeodesicDistanceCalculator distCalc;
	
	int calc(const Edge & edge) override;
};

struct WeightedGeodesicDistanceWeightCalculator: public WeightCalculator {
	WeightedGeodesicDistanceWeightCalculator(StatePtr state) : state(state)  {}
	~WeightedGeodesicDistanceWeightCalculator() override {}
	StatePtr state;
	sserialize::spatial::detail::GeodesicDistanceCalculator distCalc;
	std::shared_ptr< std::unordered_map<int, double> > typeToWeight;
	
	int calc(const Edge & edge) override;
};

struct MaxSpeedGeodesicDistanceWeightCalculator: public WeightCalculator {
	MaxSpeedGeodesicDistanceWeightCalculator(StatePtr state) : state(state) {}
	~MaxSpeedGeodesicDistanceWeightCalculator() override {}
	StatePtr state;
	sserialize::spatial::detail::GeodesicDistanceCalculator distCalc;
	///Calulate travel time in seconds
	int calc(const Edge & edge) override;
};

}}}//end namespace
#endif
