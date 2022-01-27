#ifndef PATH_FINDER_H
#define PATH_FINDER_H

#include <Python.h>
#include <string>


typedef uint16_t PointInt;

class PathFinder {
public:
	static void updateLocation(const std::string &name, const std::string &freePath,
	                           uint8_t objectBorder, PyObject *objects, PyObject *places,
	                           uint8_t minScale, uint8_t countScales);

	//using algo A-star (A*)
	static PyObject* findPath(const std::string &location, PointInt xStart, PointInt yStart,
	                          PointInt xEnd, PointInt yEnd, PyObject *objects);

	//using Dijkstra's algorithm
	static PyObject* findPathBetweenLocations(const std::string &startLocation, PointInt xStart, PointInt yStart,
	                                          const std::string &endLocation, PointInt xEnd, PointInt yEnd,
	                                          PyObject *bannedExits, bool bruteForce);
};

#endif // PATH_FINDER_H
