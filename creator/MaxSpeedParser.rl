#include "MaxSpeedParser.h"

#include <sserialize/containers/SetOpTreePrivateSimple.h>
#include <iostream>

//^([0-9][\.0-9]+?)(?:[ ]?(?:kmh|km/h|mph|kph))?$

%%{

	machine maxSpeedParser;
	
	action storenum {
		maxSpeed = atof(pb);
	}
	
	action mph {
		maxSpeed *=  1.609344;
	}
	
	action kph {
		maxSpeed *= 1.852;
	}

	action errorHandler {
		return false;
	}
	
	myspace = (space | "\n");
	floatnum = (digit+ ('.' digit+ )? );
	kmh = ("kmh" | "km/h");
	mph = ("mph" | "mp/h");
	kph = ("kph" | "kp/h");
	format = (kmh | (mph %mph) | (kph %kph));
	main :=  (myspace* . floatnum  %storenum . myspace* . format) @err(errorHandler);
	
	write data;
}%%

bool parseMaxSpeed(const std::string & str, double & maxSpeed) {
	const char * pb = str.c_str();
	const char * p = pb;
	const char * pe = pb + str.size();
	const char * eof = pe;
// 	const char * ts = p;
	int cs;
	
	%% write init;
	
	%% write exec;
	
	return true;
}