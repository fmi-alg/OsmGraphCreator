# OsmGraphCreator

The OsmGraphCreator is a command line utility to extract roads from an OpenStreetMap data set.

## Setup

### Clone

```bash
git clone --recursive <url>
```

### Dependencies

* cmake
* google protobuf
* zlib
* libicu-dev
* libcppunit-dev
* crypto++
* ragel
* cgal

### Building

```bash
    mkdir build
    cd build
    cmake ../
    make
```

### Docker

A Dockerfile can be found in the folder docker.

## Usage

The project currently builds a single executable located in the `creator` folder of your build folder.

```bash
./creator -g fmitext -t distance -c ../../data/configs/car.cfg -o mygraph.txt mygraph.osm.pbf
```

### Configuration

The `creator` needs a configuration file to map edge types to maximum driving speed which is used if an edge does not have a maxspeed tag.
Sample configuration file can be found in the folder `data/configs`.
Note that only configured edge types are part of the graph.
The configuration file format is quite simple:

```text
<osm highway keys' value>
<our own edge id>
<maximum driving speed in km/h>
```

See the file [all.cfg](data/configs/all.cfg) for an example with almost all edge types.

### Edge weights

The edge weight is selected by the `-t` option with the following options:

* *distance* calculates the distance in $[\frac{m}{distance\ multiplier}]$
* *time* calculates travel time based on edge type in $[\frac{s}{time\ multiplier}]$
* *time* calculates travel time based on maxspeed tag and edge type in $[\frac{s}{time\ multiplier}]$

The *time multiplier* and *distance multiplier* allow weights to be scaled according to the needs of the user.
By default distance is measured in meters and time in centiseconds.

### Advanced options

The `creator` has some advanced options to process large graphs.

**Connected components**:
Splitting the graph into its connected components is possible using the following options:

* `-cc` split graph into connected components
* `-ccs NUM` drops all connected components that are smaller than NUM

**Selecting a subset of the data**:
A subset of the input can be selected using the `-b' option.

## File Formats

The `creator` supports multiple output formats which are described in the following.
See the `readers` folder for examples.

### Text Formats

The format of text graphs is always

```text
METADATA
NODES
EDGES
```

#### Metadata

This information is common to all text formats.
It is the header of the files.

```text
Id : <unsigned integer>
Timestamp : <UNIX timestamp>
Type: <type of the graph>
Revision: <unsigned integer>
<number of nodes as unsigned integer>
<number of edges as unsigned integer>
```

### fmitext

The `fmitext` graph is of type `standard`.

**Node format**:

```text
NODE_ID OSM_ID LAT LON ELEVATION
<uint32_t> <int64_t> <double> <double> <uint16_t>
```

**Edge format**:

```text
SOURCE_NODE_ID TARGET_NODE_ID WEIGHT TYPE
<uint32_t> <uint32_t> <int32_t> <int32_t>
```

The interpretation of the `WEIGHT` depends on the selected options.
See the options `-dm`, `-tm` and `-t`.

### fmimaxspeedtext

The `fmimaxspeedtext` graph is of type `maxspeed`.

**Node format**:

```text
NODE_ID OSM_ID LAT LON ELEVATION
<uint32_t> <int64_t> <double> <double> <uint16_t>
```

**Edge format**:

```text
SOURCE_NODE_ID TARGET_NODE_ID WEIGHT TYPE MAXSPEED
<uint32_t> <uint32_t> <int32_t> <int32_t> <int32_t>
```

The interpretation of the `WEIGHT` depends on the selected options.
The MAXSPEED field is in km/h.
See the options `-dm`, `-tm` and `-t`.

### Examples

```bash
# travel time as edge weight
# edge speed from maxspeed tag
# only roads available to cars
# split into connected components, but drop all with less than 1024 nodes
# use files bremen, hamburg and saarland from Geofabrik (you have to download these yourself)
# write resulting files to compound directory
/creator -g fmimaxspeedtext -t time -ccs 1024 -c ../../../../data/configs/car.cfg -o compound/ bremen-latest.osm.pbf hamburg-latest.osm.pbf saarland-latest.osm.pbf
```
