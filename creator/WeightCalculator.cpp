#include "WeightCalculator.h"
#include <GeographicLib/Geodesic.hpp>

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
	Coordinates src( state->nodes.at(edge.source).coordinates );
	Coordinates dest( state->nodes.at(edge.target).coordinates );
	GeographicLib::Geodesic::WGS84.Inverse(src.lat, src.lon, dest.lat, dest.lon, length);
	return length;
}

int WeightedGeodesicDistanceWeightCalculator::calc(const osm::graphtools::creator::Edge & edge) {
	double length;
	Coordinates src( state->nodes.at(edge.source).coordinates );
	Coordinates dest( state->nodes.at(edge.target).coordinates );
	GeographicLib::Geodesic::WGS84.Inverse(src.lat, src.lon, dest.lat, dest.lon, length);
	return length*typeToWeight->at(edge.type);
};

int MaxSpeedGeodesicDistanceWeightCalculator::calc(const Edge & edge) {
	double length;
	Coordinates src( state->nodes.at(edge.source).coordinates );
	Coordinates dest( state->nodes.at(edge.target).coordinates );
	GeographicLib::Geodesic::WGS84.Inverse(src.lat, src.lon, dest.lat, dest.lon, length);
	return length/(edge.maxspeed*3.6) ;
}


}}}//end namespace