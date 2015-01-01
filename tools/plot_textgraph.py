import math
import sys
import codecs
import argparse
import matplotlib.pyplot as plt
import numpy as np
from mpl_toolkits.basemap import Basemap

sys.stdout = codecs.getwriter('utf-8')(sys.stdout)

cmdLineParser = argparse.ArgumentParser(description="Plot a graph")
cmdLineParser.add_argument('filename',  metavar='filename',  help="Name of the file to process",  type=str,  nargs='+')
parsedCmdLine = cmdLineParser.parse_args()

#Set up orthographic map projection with
# perspective of satellite looking down at 50N, 100W.
# use low resolution coastlines.

minLat = 1000.0
maxLat = -1000.0
minLon = 1000.0
maxLon = -1000.0

segments = []
f = open(parsedCmdLine.filename[0])
for i in range(0, 1000):
	line = f.readline()
#for line in f:
	sline = line.split(" ")
	if (len(sline)):
		coords = map(lambda x : float(x), sline)
		minLat = min(minLat, min(coords[0], coords[2]))
		maxLat = max(maxLat, max(coords[0], coords[2]))
		minLon = min(minLon, min(coords[1], coords[3]))
		maxLon = max(maxLon, max(coords[1], coords[3]))
		segments.append([[coords[0], coords[2]], [coords[1], coords[3]]]);

m = Basemap(llcrnrlon=minLon, llcrnrlat=minLat, urcrnrlon=maxLon, urcrnrlat=maxLat)

for segment in segments:
	m.plot(segment[1], segment[0], color='black', linewidth=1)

# draw coastlines, country boundaries, fill continents.
#m.drawcoastlines(linewidth=0.25)
#m.drawcountries(linewidth=0.25)
#m.fillcontinents(color='coral',lake_color='aqua')
# draw the edge of the map projection region (the projection limb)
#m.drawmapboundary(fill_color='aqua')
# draw lat/lon grid lines every 30 degrees.
#m.drawmeridians(np.arange(0,360,30))
#m.drawparallels(np.arange(-90,90,30))
#make up some data on a regular lat/lon grid.
plt.title('Graph')
plt.show()
