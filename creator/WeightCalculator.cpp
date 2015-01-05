#include "WeightCalculator.h"

namespace osm {
namespace graphtools {
namespace creator {

inline double kmh_to_ms(double kmh) {
	return kmh*3.6;
}

int NoWeightCalculator::calc(const osm::graphtools::creator::Edge & edge) {
	return 1;
}

int GeodesicDistanceWeightCalculator::calc(const osm::graphtools::creator::Edge & edge) {
	double length;
	const Coordinates & src = state->nodeCoordinates[edge.source];
	const Coordinates & dest = state->nodeCoordinates[edge.target];
	length = distCalc.calc(src.lat, src.lon, dest.lat, dest.lon);
	return length;
}

int WeightedGeodesicDistanceWeightCalculator::calc(const osm::graphtools::creator::Edge & edge) {
	double length;
	const Coordinates & src = state->nodeCoordinates[edge.source];
	const Coordinates & dest = state->nodeCoordinates[edge.target];
	length = distCalc.calc(src.lat, src.lon, dest.lat, dest.lon);
	return length*state->cfg.typeToWeight.at(edge.type);
};

int MaxSpeedGeodesicDistanceWeightCalculator::calc(const Edge & edge) {
	double length;
	const Coordinates & src = state->nodeCoordinates[edge.source];
	const Coordinates & dest = state->nodeCoordinates[edge.target];
	length = distCalc.calc(src.lat, src.lon, dest.lat, dest.lon);
	return length*3600/edge.maxspeed;
}


}}}//end namespace