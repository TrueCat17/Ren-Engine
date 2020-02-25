#include "path_finder.h"

#include <algorithm>
#include <vector>
#include <map>
#include <mutex>
#include <cmath>


#include "media/image_manipulator.h"
#include "media/py_utils.h"

#include "utils/utils.h"



static uint32_t divCeil(uint32_t a, uint32_t b) {
	return (a + b - 1) / b;
}
static uint16_t divRound(uint16_t a, uint16_t b) {
	return (a + b / 2) / b;
}



struct Map {
	std::vector<bool> content;
	uint32_t originalWidth, originalHeight;
	uint32_t w, h;
	uint8_t scale;
	uint8_t _padding[sizeof(void*) - 1];
};

struct SimplePoint {
	uint16_t x, y;

	bool operator<(const SimplePoint &o) const {
		return std::tie(x, y) < std::tie(o.x, o.y);
	}
};
typedef std::vector<SimplePoint> Path;

struct MipMapParams {
	std::string free;
	std::vector<std::tuple<std::string, int, int>> objects;
	std::vector<std::tuple<std::string, int, int>> places;
	std::vector<std::tuple<std::string, std::string, int, int>> exits;
	uint8_t objectBorder;
	uint8_t minScale;
	uint8_t countScales;
	uint8_t _padding[sizeof(void*) - 3];

	bool operator==(const MipMapParams &o) {
		std::tuple a(  free,   objects,   places,   exits,   objectBorder,   minScale,   countScales);
		std::tuple b(o.free, o.objects, o.places, o.exits, o.objectBorder, o.minScale, o.countScales);
		return a == b;
	}
};

struct MipMap {
	static const uint8_t MAX_SCALE = 64;
	uint32_t originalWidth, originalHeight;

	std::vector<Map*> scales;
	std::map<std::pair<SimplePoint, SimplePoint>, Path> exitPaths;

	MipMapParams params;

	~MipMap() {
		for (Map *map : scales) {
			delete map;
		}
	}
};

static std::map<std::string, MipMap*> mipMaps;

struct Point {
	uint16_t x, y;
	Point *parent;
	float costFromStart, costGuess;
};
struct PointNears {
	Point* array[8];
};



static std::mutex pathFinderMutex;


static std::vector<bool> bitmapObject;
static size_t widthBitmapObject, heightBitmapObject;

template<bool freeTransparent>
static void bitmapMake(const SurfacePtr &surface) {
	widthBitmapObject = size_t(surface->w);
	heightBitmapObject = size_t(surface->h);
	size_t size = widthBitmapObject * heightBitmapObject;
	bitmapObject.reserve(size);

	if (surface->format->BytesPerPixel == 3) {
		bitmapObject.assign(size, true);
	}else {
		bitmapObject.assign(size, false);
		const Uint8 *pixels = (const Uint8*)surface->pixels;

		if (surface->format->BytesPerPixel == 4) {
			for (size_t y = 0; y < heightBitmapObject; ++y) {
				const Uint8 *line = pixels + y * widthBitmapObject * 4;
				const size_t lineBitmap = y * widthBitmapObject;

				for (size_t x = 0; x < widthBitmapObject; ++x) {
					if (bool(line[x * 4 + Ashift / 8]) == freeTransparent) {
						bitmapObject[lineBitmap + x] = true;
					}
				}
			}
		}else {
			SDL_Color *colors = surface->format->palette->colors;

			Uint32 key;
			bool hasColorKey = SDL_GetColorKey(surface.get(), &key) == 0;
			SDL_ClearError();//if has not color key
			if (hasColorKey) {
				SDL_Color &color = colors[key];
				color.r = color.g = color.b = color.a = 0;
			}

			size_t i = 0;
			size_t last8 = size - (size % 8);
			for (; i < last8; i += 8) {
				if (bool(colors[pixels[i + 0]].a) == freeTransparent) bitmapObject[i + 0] = true;
				if (bool(colors[pixels[i + 1]].a) == freeTransparent) bitmapObject[i + 1] = true;
				if (bool(colors[pixels[i + 2]].a) == freeTransparent) bitmapObject[i + 2] = true;
				if (bool(colors[pixels[i + 3]].a) == freeTransparent) bitmapObject[i + 3] = true;
				if (bool(colors[pixels[i + 4]].a) == freeTransparent) bitmapObject[i + 4] = true;
				if (bool(colors[pixels[i + 5]].a) == freeTransparent) bitmapObject[i + 5] = true;
				if (bool(colors[pixels[i + 6]].a) == freeTransparent) bitmapObject[i + 6] = true;
				if (bool(colors[pixels[i + 7]].a) == freeTransparent) bitmapObject[i + 7] = true;
			}
			for (; i < size; ++i) {
				if (bool(colors[pixels[i]].a) == freeTransparent) bitmapObject[i] = true;
			}
		}
	}
}

static std::vector<bool> bitmapBorderObject;
static void bitmapAddBorders(size_t radius) {
	size_t w = widthBitmapObject + radius * 2;
	size_t h = heightBitmapObject + radius * 2;
	bitmapBorderObject.reserve(w * h);
	bitmapBorderObject.assign(w * h, false);

	for (size_t y = 0; y < h; ++y) {
		size_t line = y * w;

		size_t yOrig = y - radius;
		size_t lineOrig = yOrig * widthBitmapObject;

		for (size_t x = 0; x < w; ++x) {
			size_t xOrig = x - radius;

			if (xOrig < widthBitmapObject && yOrig < heightBitmapObject) {
				bool free = bitmapObject[lineOrig + xOrig];
				if (!free) continue;
			}

			bool free = true;
			if (yOrig < heightBitmapObject) {
				size_t xMin = xOrig > radius ? xOrig - radius : 0;
				size_t xMax = std::min(xOrig + radius + 1, widthBitmapObject);

				for (size_t i = xMin; i < xMax; ++i) {
					if (!bitmapObject[lineOrig + i]) {
						free = false;
						break;
					}
				}
			}
			if (free) {
				bitmapBorderObject[line + x] = true;
			}
		}
	}
}

static void bitmapDraw(std::vector<bool> &map, int mapW, int mapH, const std::vector<bool> &object, int objX, int objY, int objW, int objH) {
	int startX = std::max(objX, 0);
	int endX = std::min(objX + objW, mapW);
	int startY = std::max(objY, 0);
	int endY = std::min(objY + objH, mapH);

	for (int y = startY; y < endY; ++y) {
		int origY = y - startY;
		for (int x = startX; x < endX; ++x) {
			int origX = x - startX;
			if (!object[size_t(origY * objW + origX)]) {
				map[size_t(y * mapW + x)] = false;
			}
		}
	}
}



static std::vector<bool> freeMap;
void PathFinder::updateLocation(const std::string &name, const std::string &freePath, uint8_t objectBorder,
                                PyObject *objects, PyObject *places, PyObject *exits,
                                uint8_t minScale, uint8_t countScales)
{
	std::lock_guard g(pathFinderMutex);

	if (!PyList_CheckExact(objects) || !PyList_CheckExact(places) || !PyList_CheckExact(exits)) {
		Utils::outMsg("PathFinder::updateLocation", "Expects type of objects, places and exits is list");
		return;
	}

	long countObjectElements = Py_SIZE(objects);
	long countPlaceElements = Py_SIZE(places);
	long countExitElements = Py_SIZE(exits);
	if ((countObjectElements % 3) || (countPlaceElements % 3) || (countExitElements % 4)) {
		Utils::outMsg("PathFinder::updateLocation", "Expects objects == [freeImage, x, y] * N, places == [name, x, y] * N, exits == [loc_name, place_name, x, y] * N");
		return;
	}

	if (!minScale || minScale > MipMap::MAX_SCALE || (minScale & (minScale - 1))) {
		Utils::outMsg("PathFinder::updateLocation", "minScale(" + std::to_string(minScale) + ") == 0, more than " + std::to_string(MipMap::MAX_SCALE) + " or is not power of 2");
		return;
	}
	if (!countScales) {
		Utils::outMsg("PathFinder::updateLocation", "countScales == 0");
		return;
	}

	MipMap *mipMap;
	auto it = mipMaps.find(name);
	if (it == mipMaps.end()) {
		mipMap = new MipMap();
	}else {
		mipMap = it->second;
	}

	MipMapParams params;
	params.free = freePath;

	params.objects.reserve(size_t(countObjectElements) / 3);
	for (long i = 0; i < countObjectElements; i += 3) {
		PyObject *pyObjectFree = PyList_GET_ITEM(objects, i + 0);
		PyObject *pyX = PyList_GET_ITEM(objects, i + 1);
		PyObject *pyY = PyList_GET_ITEM(objects, i + 2);

		if (!PyString_CheckExact(pyObjectFree) || !PyInt_CheckExact(pyX) || !PyInt_CheckExact(pyY)) {
			Utils::outMsg("PathFinder::updateLocation", "Expects objects == [str, int, int] * N");
			continue;
		}

		std::string objectFreePath = PyString_AS_STRING(pyObjectFree);
		int startX = int(PyInt_AS_LONG(pyX));
		int startY = int(PyInt_AS_LONG(pyY));

		params.objects.push_back({objectFreePath, startX, startY});
	}

	params.places.reserve(size_t(countPlaceElements) / 3);
	for (long i = 0; i < countPlaceElements; i += 3) {
		PyObject *pyName = PyList_GET_ITEM(places, i + 0);
		PyObject *pyX = PyList_GET_ITEM(places, i + 1);
		PyObject *pyY = PyList_GET_ITEM(places, i + 2);

		if (!PyString_CheckExact(pyName) || !PyInt_CheckExact(pyX) || !PyInt_CheckExact(pyY)) {
			Utils::outMsg("PathFinder::updateLocation", "Expects places == [str, int, int] * N");
			continue;
		}

		std::string name = PyString_AS_STRING(pyName);
		int startX = int(PyInt_AS_LONG(pyX));
		int startY = int(PyInt_AS_LONG(pyY));

		params.places.push_back({name, startX, startY});
	}

	params.exits.reserve(size_t(countExitElements) / 4);
	for (long i = 0; i < countExitElements; i += 4) {
		PyObject *pyLocation = PyList_GET_ITEM(exits, i + 0);
		PyObject *pyPlace = PyList_GET_ITEM(exits, i + 1);
		PyObject *pyX = PyList_GET_ITEM(exits, i + 2);
		PyObject *pyY = PyList_GET_ITEM(exits, i + 3);

		if (!PyString_CheckExact(pyLocation) || !PyString_CheckExact(pyPlace) || !PyInt_CheckExact(pyX) || !PyInt_CheckExact(pyY)) {
			Utils::outMsg("PathFinder::updateLocation", "Expects exits == [str, str, int, int] * N");
			continue;
		}

		std::string location = PyString_AS_STRING(pyLocation);
		std::string place = PyString_AS_STRING(pyLocation);
		int startX = int(PyInt_AS_LONG(pyX));
		int startY = int(PyInt_AS_LONG(pyY));

		params.exits.push_back({location, place, startX, startY});
	}

	params.objectBorder = objectBorder;
	params.minScale = minScale;
	params.countScales = countScales;

	if (mipMap->params == params) return;
	mipMap->params = params;

	SurfacePtr free = ImageManipulator::getImage(freePath, false);
	if (!free) return;

	mipMap->originalWidth = uint32_t(free->w);
	mipMap->originalHeight = uint32_t(free->h);
	mipMap->scales.reserve(countScales);
	mipMaps[name] = mipMap;

	bitmapMake<true>(free);
	freeMap.swap(bitmapObject);

	for (auto obj : params.objects) {
		const std::string &objectFreePath = std::get<0>(obj);
		int startX = std::get<1>(obj);
		int startY = std::get<2>(obj);

		SurfacePtr objectFree = ImageManipulator::getImage(objectFreePath, false);
		if (!objectFree) continue;

		bitmapMake<false>(objectFree);
		if (objectBorder) {
			bitmapAddBorders(objectBorder);
		}
		bitmapDraw(
		    freeMap, free->w, free->h,
		    objectBorder ? bitmapBorderObject : bitmapObject,
		    startX - objectBorder, startY - objectBorder, objectFree->w + 2 * objectBorder, objectFree->h + 2 * objectBorder
		);
	}


	std::vector<bool> *prevContent = &freeMap;
	uint8_t prevScale = 1;
	uint32_t prevWidth = uint32_t(free->w);
	uint32_t prevHeight = uint32_t(free->h);

	uint8_t scale = minScale;
	for (uint8_t i = 0; i < countScales; ++i) {
		Map *map = new Map();
		mipMap->scales.insert(mipMap->scales.begin(), map);

		map->originalWidth = mipMap->originalWidth;
		map->originalHeight = mipMap->originalHeight;
		map->w = divCeil(uint32_t(free->w), scale);
		map->h = divCeil(uint32_t(free->h), scale);
		map->scale = scale;
		map->content.resize(map->w * map->h, true);

		uint8_t dScale = scale / prevScale;
		prevScale = scale;

		for (uint32_t blockY = 0; blockY < map->h; ++blockY) {
			uint32_t startY = blockY * dScale;
			uint32_t endY = std::min((blockY + 1) * dScale, prevHeight);
			uint32_t blockLine = blockY * map->w;

			for (uint32_t blockX = 0; blockX < map->w; ++blockX) {
				uint32_t startX = blockX * dScale;
				uint32_t endX = std::min((blockX + 1) * dScale, prevWidth);

				for (uint32_t y = startY; y < endY; ++y) {
					uint32_t line = y * prevWidth;

					for (uint32_t x = startX; x < endX; ++x) {
						if (!(*prevContent)[line + x]) {
							map->content[blockLine + blockX] = false;
							y = endY;
							break;
						}
					}
				}
			}
		}

		prevWidth = map->w;
		prevHeight = map->h;
		prevContent = &map->content;

		scale *= 2;
		if (scale > MipMap::MAX_SCALE) break;
	}
}




static Path path;
static Map *currentMap;

static std::vector<Point> mapPoints;

static std::vector<Point*> openPoints;
static std::vector<Point*> closePoints;


static bool containsPoint(const std::vector<Point*> *vec, Point *point) {
	return std::binary_search(vec->begin(), vec->end(), point);
}
static void insertPoint(std::vector<Point*> &vec, Point *point) {
	auto it = std::lower_bound(vec.begin(), vec.end(), point);
	if (it == vec.end() || *it != point) {
		vec.insert(it, point);
	}
}



static void preparePoints() {
	openPoints.reserve(1024);
	closePoints.reserve(1024);
	path.reserve(64);

	openPoints.clear();
	closePoints.clear();
	path.clear();

	uint32_t count = currentMap->w * currentMap->h;
	if (mapPoints.capacity() < count) {
		mapPoints.resize(count);
	}
}


static float calcDist(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
	uint32_t dX = x1 > x2 ? x1 - x2 : x2 - x1;
	uint32_t dY = y1 > y2 ? y1 - y2 : y2 - y1;
	return std::sqrt(float(dX * dX + dY * dY));
}
static Point* getNewPoint(uint16_t x, uint16_t y, float costFromStart = 0, Point *parent = nullptr, Point *end = nullptr) {
	Point *res = &mapPoints[currentMap->w * y + x];

	for (auto *vec : {&openPoints, &closePoints}) {
		if (containsPoint(vec, res)) {
			if (costFromStart < res->costFromStart) {
				res->costFromStart = costFromStart;
				res->costGuess = costFromStart + calcDist(x, y, end->x, end->y);
				res->parent = parent;
			}
			return nullptr;
		}
	}

	res->x = x;
	res->y = y;
	res->parent = parent;
	res->costFromStart = costFromStart;
	res->costGuess = costFromStart + (end ? calcDist(x, y, end->x, end->y) : 0);

	return res;
}


static uint8_t getFromMap(uint16_t x, uint16_t y) {
	if (x < currentMap->w && y < currentMap->h) {
		return currentMap->content[y * currentMap->w + x];
	}
	return uint8_t(-1);
}

static const float sqrt2 = std::sqrt(2.0f);
static PointNears getNears(Point *point, Point *end) {
	PointNears res;

	uint16_t x = point->x;
	uint16_t y = point->y;

	float costFromStart = point->costFromStart;
	float costSide = costFromStart + 1;
	float costCorner = costFromStart + sqrt2;

	bool frees[8];
	frees[0] = getFromMap(x - 1, y) == 1;
	frees[1] = getFromMap(x, y - 1) == 1;
	frees[2] = getFromMap(x + 1, y) == 1;
	frees[3] = getFromMap(x, y + 1) == 1;
	frees[4] = frees[0] && frees[1] && getFromMap(x - 1, y - 1) == 1;
	frees[5] = frees[1] && frees[2] && getFromMap(x + 1, y - 1) == 1;
	frees[6] = frees[2] && frees[3] && getFromMap(x + 1, y + 1) == 1;
	frees[7] = frees[3] && frees[0] && getFromMap(x - 1, y + 1) == 1;

	//sides
	res.array[0] = frees[0] ? getNewPoint(x - 1, y, costSide, point, end) : nullptr;
	res.array[1] = frees[1] ? getNewPoint(x, y - 1, costSide, point, end) : nullptr;
	res.array[2] = frees[2] ? getNewPoint(x + 1, y, costSide, point, end) : nullptr;
	res.array[3] = frees[3] ? getNewPoint(x, y + 1, costSide, point, end) : nullptr;

	//corners
	res.array[4] = frees[4] ? getNewPoint(x - 1, y - 1, costCorner, point, end) : nullptr;
	res.array[5] = frees[5] ? getNewPoint(x + 1, y - 1, costCorner, point, end) : nullptr;
	res.array[6] = frees[6] ? getNewPoint(x + 1, y + 1, costCorner, point, end) : nullptr;
	res.array[7] = frees[7] ? getNewPoint(x - 1, y + 1, costCorner, point, end) : nullptr;

	return res;
}

static void getPath(Point *start, Point *end) {
	if (getFromMap(end->x, end->y) != 1) return;

	closePoints = {start};

	Point *last = start;
	while (last != end) {
		PointNears nears = getNears(last, end);
		for (int i = 0; i < 8; ++i) {
			if (nears.array[i]) {
				insertPoint(openPoints, nears.array[i]);
			}
		}

		if (openPoints.empty()) return;

		auto it = openPoints.begin();
		last = *it;
		for (auto tmpIt = openPoints.begin() + 1; tmpIt != openPoints.end(); ++tmpIt) {
			if ((*tmpIt)->costGuess < last->costGuess) {
				it = tmpIt;
				last = *tmpIt;
			}
		}

		openPoints.erase(it);
		insertPoint(closePoints, last);
	}

	while (last != start) {
		uint16_t x = std::min(uint16_t(last->x * currentMap->scale), uint16_t(currentMap->originalWidth - 1));
		uint16_t y = std::min(uint16_t(last->y * currentMap->scale), uint16_t(currentMap->originalHeight - 1));
		path.push_back({x, y});
		last = last->parent;
	}
}


static Map tmpMap;
PyObject* PathFinder::findPath(const std::string &location, uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd, PyObject* objects) {
	std::lock_guard g(pathFinderMutex);

	auto mipMapIt = mipMaps.find(location);
	if (mipMapIt == mipMaps.end()) {
		Utils::outMsg("PathFinder::findPath", "Location <" + location + "> not found");
		return PyTuple_New(0);
	}
	MipMap *mipMap = mipMapIt->second;

	if (xStart >= mipMap->originalWidth || xEnd >= mipMap->originalWidth || yStart >= mipMap->originalHeight || yEnd >= mipMap->originalHeight) {
		Utils::outMsg("PathFinder::findPath", "Start or/and End point outside location");
		return PyTuple_New(0);
	}
	if (objects != Py_None && !PyList_CheckExact(objects)) {
		Utils::outMsg("PathFinder::findPath", "Expects type(objects) is list");
		return PyTuple_New(0);
	}

	long countObjectElements = objects != Py_None ? Py_SIZE(objects) : 0;
	if (countObjectElements % 4) {
		Utils::outMsg("PathFinder::findPath", "Expects objects == [x, y, w, h] * N");
		return PyTuple_New(0);
	}


	Path *pathPtr = nullptr;

	if (!countObjectElements) {
		const auto pathIt = mipMap->exitPaths.find({{xStart, yStart}, {xEnd, yEnd}});
		if (pathIt != mipMap->exitPaths.end()) {
			pathPtr = &pathIt->second;
		}
	}
	if (!pathPtr) {
		for (size_t i = 0; i != mipMap->scales.size(); ++i) {
			currentMap = mipMap->scales[i];
			preparePoints();

			if (countObjectElements) {
				currentMap = &tmpMap;

				for (long j = 0; j < countObjectElements; j += 4) {
					PyObject *pyX = PyList_GET_ITEM(objects, i + 0);
					PyObject *pyY = PyList_GET_ITEM(objects, i + 1);
					PyObject *pyW = PyList_GET_ITEM(objects, i + 2);
					PyObject *pyH = PyList_GET_ITEM(objects, i + 3);
					if (!PyInt_CheckExact(pyX) || !PyInt_CheckExact(pyY) || !PyInt_CheckExact(pyW) || !PyInt_CheckExact(pyH)) {
						Utils::outMsg("PathFinder::findPath", "Expects type(objects[i]) is int");
						continue;
					}
					long x = PyInt_AS_LONG(pyX);
					long y = PyInt_AS_LONG(pyY);
					long w = PyInt_AS_LONG(pyW);
					long h = PyInt_AS_LONG(pyH);

					long startX = x / tmpMap.scale;
					long startY = y / tmpMap.scale;
					long endX = std::min((x + w) / tmpMap.scale, long(tmpMap.w - 1));
					long endY = std::min((y + h) / tmpMap.scale, long(tmpMap.h - 1));

					for (long y = startY; y <= endY; ++y) {
						for (long x = startX; x <= endX; ++x) {
							tmpMap.content[uint32_t(y * tmpMap.w + x)] = 1;
						}
					}
				}
			}

			uint16_t scale = currentMap->scale;
			Point *start = getNewPoint(divRound(xStart, scale), divRound(yStart, scale));
			Point *end = getNewPoint(divRound(xEnd, scale), divRound(yEnd, scale));
			getPath(start, end);

			if (!path.empty()) {
				path.front() = {xEnd, yEnd};
				break;
			}
		}
		pathPtr = &path;
	}

	long resSize = long(pathPtr->size()) * 2;
	PyObject *res = PyTuple_New(resSize);
	for (size_t i = pathPtr->size() - 1; i != size_t(-1); --i) {
		const SimplePoint &p = (*pathPtr)[i];
		PyTuple_SET_ITEM(res, resSize - long(i) * 2 - 2, PyInt_FromLong(p.x));
		PyTuple_SET_ITEM(res, resSize - long(i) * 2 - 1, PyInt_FromLong(p.y));
	}
	return res;
}

PyObject* PathFinder::findPathBetweenLocations(const std::string &startLocation, uint16_t xStart, uint16_t yStart, const std::string &endLocation, uint16_t xEnd, uint16_t yEnd, bool bruteForce) {
	if (!bruteForce && startLocation == endLocation) return PathFinder::findPath(startLocation, xStart, yStart, xEnd, yEnd, Py_None);

	std::lock_guard g(pathFinderMutex);



	PyObject *res = PyTuple_New(0);
	return res;
}
