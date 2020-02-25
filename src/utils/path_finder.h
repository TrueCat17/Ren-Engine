#ifndef PATH_FINDER_H
#define PATH_FINDER_H

#include <Python.h>
#include <string>


class PathFinder {
public:
	static void updateLocation(const std::string &name, const std::string &freePath, uint8_t objectBorder, PyObject *objects, PyObject *places, PyObject *exits, uint8_t minScale, uint8_t countScales);

	//using algo A-star (A*)
	static PyObject* findPath(const std::string &location, uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd, PyObject *objects);

	//using Dijkstra's algorithm
	static PyObject* findPathBetweenLocations(const std::string &startLocation, uint16_t xStart, uint16_t yStart, const std::string &endLocation, uint16_t xEnd, uint16_t yEnd, bool bruteForce);
};

#endif // PATH_FINDER_H
