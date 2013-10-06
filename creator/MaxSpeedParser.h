#ifndef OSM_GRAPH_TOOLS_MAX_SPEED_PARSER_H
#define OSM_GRAPH_TOOLS_MAX_SPEED_PARSER_H
#include <string>

///@return maxspeed in km/h, -1 on error
bool parseMaxSpeed(const std::string & str, double & maxspeed);

inline bool parseMaxSpeed(const std::string & str, int & maxspeed) {
	double tmp;
	bool ok = parseMaxSpeed(str, tmp);
	maxspeed = tmp;
	return ok;
}


#endif