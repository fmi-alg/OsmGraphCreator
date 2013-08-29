#ifndef OSM_GRAPH_TOOLS_CONVERSION_H
#define OSM_GRAPH_TOOLS_CONVERSION_H
#include "types.h"

namespace osm {
namespace graphtools {
namespace creator {
	inline bool toBool(const std::string & str) {
		return str == "true" || str == "yes" || str == "1";
	}

}}}//end namespace

#endif