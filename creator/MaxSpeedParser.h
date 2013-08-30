#ifndef OSM_GRAPH_TOOLS_MAX_SPEED_PARSER_H
#define OSM_GRAPH_TOOLS_MAX_SPEED_PARSER_H
#include <string>

///@return maxspeed in km/h, -1 on error
double parseMaxSpeed(const std::string & str);

#endif