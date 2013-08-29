#include "WeightCalculator.h"
#include <GeographicLib/Geodesic.hpp>

namespace osm {
namespace graphtools {
namespace creator {

int NoWeightCalculator::calc(const Coordinates & src, const Coordinates & dest, int type) {
	return 1;
}

int GeodesicDistanceWeightCalculator::calc(const Coordinates & src, const Coordinates & dest, int type) {
	double length;
	GeographicLib::Geodesic::WGS84.Inverse(src.lat, src.lon, dest.lat, dest.lon, length);
	return length;
}

int WeightedGeodesicDistanceWeightCalculator::calc(const Coordinates & src, const Coordinates & dest, int type) {
	double length;
	GeographicLib::Geodesic::WGS84.Inverse(src.lat, src.lon, dest.lat, dest.lon, length);
	return length*typeToWeight->at(type);
};

}}}//end namespace