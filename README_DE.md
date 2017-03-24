
#Installation
##Clone

git clone --recursive https://github.com/fmi-alg/OsmGraphCreator.git

##Benötigte Bibliotheken:

- Cmake
- Protocol buffer Bibliothek für C++ (meistens protobuf-* im Paketmanager)
- CGAL (meistens libcgal im Paketmanager)
- zlib-dev

##Kompilieren

1. cd OsmGraphCreator
2. mkdir build
3. cd build
4. cmake CMAKE_BUILD_TYPE=lto ../
5. make
6. cd creator



#Daten

Original OSM-Daten im PBF-Format bekommt man z.B. von
http://download.geofabrik.de/

#Nutzung

Der OsmGraphCreator erstellt (Straßen-) Graphen auf Basis von OpenStreetMap daten.
Diese werden von verschiedenen Projekten am FMI genutzt.
Je nach Anwendung werden unterschiedliche Graph- und Datenformate benötigt.

##Erstellung eines Graphen mit maxspeed Informationen

./creator -s -g fmimaxspeedtext -t time -c ../../data/configs/car.cfg -o out.ftxt data.osm.pbf

Dies erzeugt nach einiger Zeit ein Datei mit folgendem Format:
1. Anzahl Knoten
2. Anzahl Kanten
3. Knoten
4. Kanten

Ein Knoten ist wie folgt kodiert:
<Knoten ID> <OSM ID> <Längegrad> <Breitengrad> <Höhe über NN>

0 1212985 53.82233040000000557 10.72339740000000141 0
Ist Ein Knoten mit
- der Id 0
- Osm Id 1212985
- Längengrad 53.82233040000000557
- Breitengrad 10.72339740000000141
- einer Höhe über NN von 0

Eine Kante hingegen:
<Startknoten ID> <Endknoten ID> <Gewicht> <Typ> <Maximal erlaubte Geschwindigkeit>

626656 626655 67 15 30

Ist eine Kanten
- von Knoten 626656 zu Knoten 626655
- mit dem Gewicht (hier Länge) von 67
- dem Typ 15 = highway:service
- einer maximalen Geschwindigkeit von 30 kmh