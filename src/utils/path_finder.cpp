#include "path_finder.h"

#include <algorithm>
#include <vector>
#include <map>
#include <mutex>
#include <cmath>


#include "media/image_manipulator.h"
#include "media/py_utils/convert_to_py.h"
#include "media/py_utils.h"

#include "utils/file_system.h"
#include "utils/scope_exit.h"
#include "utils/utils.h"


//only for debug
[[maybe_unused]]
static void saveMap(std::vector<bool> map, int w, int h, const std::string &name, int scale) {
	Uint32 resFormat = SDL_PIXELFORMAT_INDEX8;
	int resPitch = w * int(SDL_BITSPERPIXEL(resFormat)) / 8;
	resPitch = (resPitch + 3) & ~3;//align to 4
	Uint8 *resPixels = (Uint8*)SDL_malloc(size_t(h * resPitch));

	SurfacePtr res = SDL_CreateRGBSurfaceWithFormatFrom(
	            resPixels, w, h, SDL_BITSPERPIXEL(resFormat), resPitch, resFormat);
	SDL_SetSurfaceBlendMode(res.get(), SDL_BLENDMODE_NONE);
	res->flags &= Uint32(~SDL_PREALLOC);

	SDL_Palette *palette = res->format->palette;
	palette->ncolors = 2;
	palette->colors[0] = {0, 0, 0, 0};
	palette->colors[1] = {0, 0, 0, 255};

	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < w; ++x) {
			bool free = map[size_t(y * w + x)];
			resPixels[y * resPitch + x] = free;
		}
	}

	std::string dir = "../var/maps/";
	if (!FileSystem::exists(dir)) {
		FileSystem::createDirectory(dir);
	}
	ImageManipulator::saveSurface(res, dir + name + "-" + std::to_string(scale) + ".png", "None", "None", true);
}



static const float EXIT_COST = 20;

static uint32_t divCeil(uint32_t a, uint32_t b) {
	return (a + b - 1) / b;
}


struct Map {
	std::vector<bool> content;
	uint32_t originalWidth, originalHeight;
	uint32_t w, h;
	uint8_t scale;
};

struct SimplePoint {
	PointInt x, y;

	bool operator<(const SimplePoint &o) const {
		return std::tie(x, y) < std::tie(o.x, o.y);
	}
};
typedef std::vector<SimplePoint> Path;

struct LocationPlace {
	std::string name;
	int x, y;
	std::string toLocationName;
	std::string toPlaceName;
	bool enable;
	bool isBannedExit;

	bool operator==(const LocationPlace &o) const {
		std::tuple a(  x,   y,   toLocationName,   toPlaceName);
		std::tuple b(o.x, o.y, o.toLocationName, o.toPlaceName);
		return a == b;
	}
};

struct MipMapParams {
	std::string free;
	std::vector<std::tuple<std::string, int, int>> objects;
	std::vector<LocationPlace> places;
	uint8_t objectBorder;
	uint8_t minScale;
	uint8_t countScales;

	bool operator==(const MipMapParams &o) const {
		std::tuple a(  free,   objects,   places,   objectBorder,   minScale,   countScales);
		std::tuple b(o.free, o.objects, o.places, o.objectBorder, o.minScale, o.countScales);
		return a == b;
	}
};

struct MipMap {
	static const uint8_t MAX_SCALE = 64;

	uint32_t originalWidth, originalHeight;
	std::string name;
	MipMapParams params;

	std::vector<Map*> scales;
	std::map<std::pair<SimplePoint, SimplePoint>, std::pair<Path, float>> exitPaths;

	~MipMap() {
		for (Map *map : scales) {
			delete map;
		}
	}
};

static std::map<std::string, MipMap*> mipMaps;

static MipMap* getLocation(const std::string &name) {
	auto it = mipMaps.find(name);
	if (it == mipMaps.end()) {
		Utils::outError("PathFinder::getLocation", "Location <%> not found", name);
		return nullptr;
	}
	return it->second;
}
static void makeOutsideMsg(const std::string &from, PointInt xStart, PointInt yStart, double xEnd, double yEnd,
                           const MipMap *startMap, const MipMap *endMap)
{
	std::string extra;
	if (endMap) {
		extra = Utils::format("start=%x%, end=%x%",
		                      startMap->originalWidth, startMap->originalHeight,
		                      endMap->originalWidth, endMap->originalHeight);
	}else {
		extra = Utils::format("%x%", startMap->originalWidth, startMap->originalHeight);
	}

	Utils::outError("PathFinder::" + from,
	                "Start (%, %) or/and End (%, %) point outside\n"
	                "location (%)",
	                xStart, yStart, xEnd, yEnd, extra);
}

struct Point {
	PointInt x, y;
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
				const Uint8 *line = pixels + y * size_t(surface->pitch);
				const size_t lineBitmap = y * widthBitmapObject;

				for (size_t x = 0; x < widthBitmapObject; ++x) {
					if (bool(line[x * 4 + Ashift / 8]) == freeTransparent) {
						bitmapObject[lineBitmap + x] = true;
					}
				}
			}
		}else {
			SDL_Color *colors = surface->format->palette->colors;

			if (SDL_HasColorKey(surface.get())) {
				Uint32 key;
				SDL_GetColorKey(surface.get(), &key);

				SDL_Color &color = colors[key];
				color.r = color.g = color.b = color.a = 0;
			}


			for (size_t y = 0; y < heightBitmapObject; ++y) {
				const Uint8 *line = pixels + y * size_t(surface->pitch);
				const size_t lineBitmap = y * widthBitmapObject;

				size_t last8 = widthBitmapObject - (widthBitmapObject % 8);
				size_t x = 0;
				for (; x < last8; x += 8) {
					if (bool(colors[line[x + 0]].a) == freeTransparent) bitmapObject[lineBitmap + x + 0] = true;
					if (bool(colors[line[x + 1]].a) == freeTransparent) bitmapObject[lineBitmap + x + 1] = true;
					if (bool(colors[line[x + 2]].a) == freeTransparent) bitmapObject[lineBitmap + x + 2] = true;
					if (bool(colors[line[x + 3]].a) == freeTransparent) bitmapObject[lineBitmap + x + 3] = true;
					if (bool(colors[line[x + 4]].a) == freeTransparent) bitmapObject[lineBitmap + x + 4] = true;
					if (bool(colors[line[x + 5]].a) == freeTransparent) bitmapObject[lineBitmap + x + 5] = true;
					if (bool(colors[line[x + 6]].a) == freeTransparent) bitmapObject[lineBitmap + x + 6] = true;
					if (bool(colors[line[x + 7]].a) == freeTransparent) bitmapObject[lineBitmap + x + 7] = true;
				}
				for (; x < widthBitmapObject; ++x) {
					if (bool(colors[line[x]].a) == freeTransparent) bitmapObject[lineBitmap + x] = true;
				}
			}
		}
	}
}

static std::vector<bool> bitmapBorderObject;
static void bitmapAddBorders(size_t radius) {
	int wOrig = int(widthBitmapObject);
	int r = int(radius);

	int w = int(widthBitmapObject + radius * 2);
	int h = int(heightBitmapObject);
	bitmapBorderObject.reserve(size_t(w * h));
	bitmapBorderObject.assign(size_t(w * h), false);

	for (int y = 0; y < h; ++y) {
		int line = y * w;
		int lineOrig = y * wOrig;

		for (int x = 0; x < w; ++x) {
			int xOrig = x - r;

			if (x >= r && xOrig < wOrig && 0) {
				bool free = bitmapObject[size_t(lineOrig + xOrig)];
				if (!free) continue;
			}

			int xMin = std::max(xOrig - r, 0);
			int xMax = std::min(xOrig + r + 1, wOrig);

			bool free = true;
			for (int i = xMin; i < xMax; ++i) {
				if (!bitmapObject[size_t(lineOrig + i)]) {
					free = false;
					break;
				}
			}
			if (free) {
				bitmapBorderObject[size_t(line + x)] = true;
			}
		}
	}
}

static void bitmapDraw(std::vector<bool> &map, int mapW, int mapH,
                       const std::vector<bool> &object, int objX, int objY, int objW, int objH)
{
	int startX = std::max(objX, 0);
	int endX = std::min(objX + objW, mapW);
	int startY = std::max(objY, 0);
	int endY = std::min(objY + objH, mapH);

	for (int y = startY; y < endY; ++y) {
		int line = y * mapW;

		int origY = y - startY;
		int origLine = origY * objW;

		for (int x = startX; x < endX; ++x) {
			int origX = x - startX;
			if (!object[size_t(origLine + origX)]) {
				map[size_t(line + x)] = false;
			}
		}
	}
}



static std::vector<bool> freeMap;
void PathFinder::updateLocation(const std::string &name, const std::string &freePath, uint8_t objectBorder,
                                PyObject *objects, PyObject *places,
                                uint8_t minScale, uint8_t countScales)
{
	std::lock_guard g(pathFinderMutex);

	if (!PyList_CheckExact(objects) || !PyList_CheckExact(places)) {
		Utils::outMsg("PathFinder::updateLocation", "Expects type of objects and places is list");
		return;
	}

	long countObjectElements = Py_SIZE(objects);
	long countPlaceElements = Py_SIZE(places);
	if ((countObjectElements % 3) || (countPlaceElements % 5)) {
		Utils::outMsg("PathFinder::updateLocation",
		              "Expects objects == [freeImage, x, y] * N, places == [name, x, y, loc_name, to_place_name] * N");
		return;
	}

	if (!minScale || minScale > MipMap::MAX_SCALE || (minScale & (minScale - 1))) {
		Utils::outError("PathFinder::updateLocation",
		                "minScale (%) == 0, more than % or is not a power of 2",
		                minScale, MipMap::MAX_SCALE);
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
		mipMap->exitPaths.clear();
	}

	MipMapParams params;
	params.free = freePath;

	params.objects.reserve(size_t(countObjectElements) / 3);
	for (long i = 0; i < countObjectElements; i += 3) {
		PyObject *pyObjectFree = PyList_GET_ITEM(objects, i + 0);
		PyObject *pyX = PyList_GET_ITEM(objects, i + 1);
		PyObject *pyY = PyList_GET_ITEM(objects, i + 2);

		if (!PyUnicode_CheckExact(pyObjectFree) || !PyLong_CheckExact(pyX) || !PyLong_CheckExact(pyY)) {
			Utils::outMsg("PathFinder::updateLocation", "Expects objects == [str, int, int] * N");
			continue;
		}

		std::string objectFreePath = PyUtils::objToStr(pyObjectFree);
		int overflowX, overflowY;
		int startX = int(PyLong_AsLongAndOverflow(pyX, &overflowX));
		int startY = int(PyLong_AsLongAndOverflow(pyY, &overflowY));
		if (overflowX || overflowY) {
			Utils::outMsg("PathFinder::updateLocation", "Object coordinates are overflowing");
			continue;
		}

		params.objects.push_back({objectFreePath, startX, startY});
	}

	params.places.reserve(size_t(countPlaceElements) / 5);
	for (long i = 0; i < countPlaceElements; i += 5) {
		PyObject *pyName = PyList_GET_ITEM(places, i + 0);
		PyObject *pyX = PyList_GET_ITEM(places, i + 1);
		PyObject *pyY = PyList_GET_ITEM(places, i + 2);
		PyObject *pyLocation = PyList_GET_ITEM(places, i + 3);
		PyObject *pyPlace = PyList_GET_ITEM(places, i + 4);

		if (!PyUnicode_CheckExact(pyName) || !PyLong_CheckExact(pyX) || !PyLong_CheckExact(pyY) ||
		    !PyUnicode_CheckExact(pyLocation) || !PyUnicode_CheckExact(pyPlace))
		{
			Utils::outMsg("PathFinder::updateLocation", "Expects places == [str, int, int, str, str] * N");
			continue;
		}

		std::string name = PyUtils::objToStr(pyName);
		int overflowX, overflowY;
		int startX = int(PyLong_AsLongAndOverflow(pyX, &overflowX));
		int startY = int(PyLong_AsLongAndOverflow(pyY, &overflowY));
		if (overflowX || overflowY) {
			Utils::outMsg("PathFinder::updateLocation", "Place coordinates are overflowing");
			continue;
		}
		std::string location = PyUtils::objToStr(pyLocation);
		std::string place = PyUtils::objToStr(pyPlace);

		params.places.push_back({name, startX, startY, location, place, false, false});
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
	mipMap->name = name;
	mipMap->scales.reserve(countScales);
	mipMaps[name] = mipMap;


	bitmapMake<true>(free);
	freeMap.swap(bitmapObject);

	for (const auto &obj : params.objects) {
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
		    startX - objectBorder, startY, objectFree->w + 2 * objectBorder, objectFree->h
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


static float calcDist(PointInt x1, PointInt y1, PointInt x2, PointInt y2) {
	PointInt dX = x1 > x2 ? x1 - x2 : x2 - x1;
	PointInt dY = y1 > y2 ? y1 - y2 : y2 - y1;
	return std::sqrt(float(dX * dX + dY * dY));
}
static Point* getNewPoint(PointInt x, PointInt y, float costFromStart = 0,
                          Point *parent = nullptr, Point *end = nullptr)
{
	Point *res = &mapPoints[currentMap->w * y + x];

	for (const auto *vec : {&openPoints, &closePoints}) {
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


static uint8_t getFromMap(PointInt x, PointInt y) {
	if (x < currentMap->w && y < currentMap->h) {
		return currentMap->content[y * currentMap->w + x];
	}
	return uint8_t(-1);
}
static void setToMap(PointInt x, PointInt y, bool value) {
	currentMap->content[y * currentMap->w + x] = value;
}

static const float sqrt2 = std::sqrt(2.0f);
static PointNears getNears(Point *point, Point *end) {
	PointNears res;

	auto x = point->x;
	auto y = point->y;

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
	uint8_t startValue = getFromMap(start->x, start->y);
	uint8_t endValue = getFromMap(end->x, end->y);
	if (startValue == uint8_t(-1) || endValue == uint8_t(-1)) return;

	//make start/end points free and return it back before exit
	ScopeExit se([start, end, startValue, endValue]() {
		setToMap(start->x, start->y, startValue);
		setToMap(end->x, end->y, endValue);
	});
	setToMap(start->x, start->y, true);
	setToMap(end->x, end->y, true);

	uint8_t scale = currentMap->scale;

	if (start == end) {
		PointInt x = std::min(PointInt(end->x * scale), PointInt(currentMap->originalWidth - 1));
		PointInt y = std::min(PointInt(end->y * scale), PointInt(currentMap->originalHeight - 1));
		path.push_back({x, y});
		return;
	}

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
		PointInt x = std::min(PointInt(last->x * scale + scale / 2), PointInt(currentMap->originalWidth - 1));
		PointInt y = std::min(PointInt(last->y * scale + scale / 2), PointInt(currentMap->originalHeight - 1));
		path.push_back({x, y});
		last = last->parent;
	}
}


static inline
bool checkMapPoint(const MipMap *mipMap, double x, double y) {
	if (x < 0 || x > std::numeric_limits<PointInt>::max()) return false;
	if (y < 0 || y > std::numeric_limits<PointInt>::max()) return false;
	if (x >= mipMap->originalWidth) return false;
	if (y >= mipMap->originalHeight) return false;
	return true;
}


static Map tmpMap;
PyObject* PathFinder::findPath(const std::string &location, PointInt xStart, PointInt yStart,
                               double xEndFloat, double yEndFloat, PyObject* objects)
{
	std::lock_guard g(pathFinderMutex);

	MipMap *mipMap = getLocation(location);
	if (!mipMap) return PyTuple_New(0);

	if (!checkMapPoint(mipMap, xStart, yStart) || !checkMapPoint(mipMap, xEndFloat, yEndFloat)) {
		makeOutsideMsg("findPath", xStart, yStart, xEndFloat, yEndFloat, mipMap, nullptr);
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


	PointInt xEnd = PointInt(xEndFloat);
	PointInt yEnd = PointInt(yEndFloat);

	Path *pathPtr = nullptr;

	if (!countObjectElements) {
		const auto pathIt = mipMap->exitPaths.find({{xStart, yStart}, {xEnd, yEnd}});
		if (pathIt != mipMap->exitPaths.end()) {
			pathPtr = &pathIt->second.first;
		}
	}
	if (!pathPtr) {
		for (size_t i = 0; i != mipMap->scales.size(); ++i) {
			currentMap = mipMap->scales[i];
			preparePoints();

			uint16_t scale = currentMap->scale;

			if (countObjectElements) {
				tmpMap = *currentMap;//copy
				currentMap = &tmpMap;

				for (long j = 0; j < countObjectElements; j += 4) {
					PyObject *pyX = PyList_GET_ITEM(objects, j + 0);
					PyObject *pyY = PyList_GET_ITEM(objects, j + 1);
					PyObject *pyW = PyList_GET_ITEM(objects, j + 2);
					PyObject *pyH = PyList_GET_ITEM(objects, j + 3);
					if (!PyLong_CheckExact(pyX) || !PyLong_CheckExact(pyY) ||
					    !PyLong_CheckExact(pyW) || !PyLong_CheckExact(pyH))
					{
						Utils::outMsg("PathFinder::findPath", "Expects type(objects[i]) is int");
						continue;
					}
					int overflowX, overflowY, overflowW, overflowH;
					long x = PyLong_AsLongAndOverflow(pyX, &overflowX);
					long y = PyLong_AsLongAndOverflow(pyY, &overflowY);
					long w = PyLong_AsLongAndOverflow(pyW, &overflowW);
					long h = PyLong_AsLongAndOverflow(pyH, &overflowH);
					if (overflowX || overflowY || overflowW || overflowH) {
						Utils::outMsg("PathFinder::updateLocation", "Object coordinates/sizes are overflowing");
						continue;
					}

					long startX = x / scale;
					long startY = y / scale;
					long endX = std::min((x + w) / scale, long(tmpMap.w - 1));
					long endY = std::min((y + h) / scale, long(tmpMap.h - 1));

					for (long y = startY; y <= endY; ++y) {
						long startIndex = y * tmpMap.w + startX;
						long endIndex = y * tmpMap.w + endX;
						std::fill(tmpMap.content.begin() + startIndex, tmpMap.content.begin() + endIndex, false);
					}
				}
			}

			Point *start = getNewPoint(xStart / scale, yStart / scale);
			Point *end = getNewPoint(xEnd / scale, yEnd / scale);
			getPath(start, end);

			if (!path.empty()) {
				path.back() = {xStart, yStart};
				break;
			}
		}
		pathPtr = &path;
	}

	long resSize = long(pathPtr->size()) * 2;
	PyObject *res = PyTuple_New(resSize);
	if (resSize) {
		for (size_t i = pathPtr->size() - 1; i != 0; --i) {
			const SimplePoint &p = (*pathPtr)[i];
			PyTuple_SET_ITEM(res, resSize - long(i) * 2 - 2, PyLong_FromLong(p.x));
			PyTuple_SET_ITEM(res, resSize - long(i) * 2 - 1, PyLong_FromLong(p.y));
		}
		PyTuple_SET_ITEM(res, resSize - 2, PyFloat_FromDouble(xEndFloat));
		PyTuple_SET_ITEM(res, resSize - 1, PyFloat_FromDouble(yEndFloat));
	}
	return res;
}



static std::map<std::tuple<MipMap*, SimplePoint, SimplePoint>, std::pair<Path, float>> tmpPaths;
static const std::pair<Path, float>& findAndSavePath(MipMap *mipMap, SimplePoint start, SimplePoint end, bool pathIsConst) {
	auto it1 = mipMap->exitPaths.find({start, end});
	if (it1 != mipMap->exitPaths.end()) {
		return it1->second;
	}
	auto it2 = tmpPaths.find({mipMap, start, end});
	if (it2 != tmpPaths.end()) {
		return it2->second;
	}

	for (size_t i = 0; i < mipMap->scales.size(); ++i) {
		currentMap = mipMap->scales[i];
		preparePoints();

		PointInt scale = currentMap->scale;
		Point *startPoint = getNewPoint(start.x / scale, start.y / scale);
		Point *endPoint = getNewPoint(end.x / scale, end.y / scale);
		getPath(startPoint, endPoint);

		if (!path.empty()) break;
	}

	float cost;
	if (!path.empty()) {
		if (path.size() == 1) {
			cost = calcDist(start.x, start.y, end.x, end.y);
			path[0] = end;
		}else {
			auto [startX, startY] = path.back();
			cost = calcDist(start.x, start.y, startX, startY);
			for (size_t i = 0; i < path.size() - 1; ++i) {
				cost += calcDist(path[i].x, path[i].y, path[i + 1].x, path[i + 1].y);
			}
		}
	}else {
		cost = -1;
	}

	Path reversePath(path.rbegin(), path.rend());
	if (pathIsConst) {
		mipMap->exitPaths[{end, start}] = {reversePath, cost};
		return mipMap->exitPaths[{start, end}] = {path, cost};
	}else {
		tmpPaths[{mipMap, end, start}] = {reversePath, cost};
		return tmpPaths[{mipMap, start, end}] = {path, cost};
	}
}

struct LocationNode {
	uint16_t id;

	uint16_t prevId;
	bool changed;
	bool isExit;
	bool isBannedExit;

	PointInt x, y;
	float cost;

	std::string placeName;
	std::string toLocationName, toPlaceName;

	MipMap *mipMap;
	std::vector<std::pair<uint16_t, float>> destinations;
};
static std::vector<LocationNode> locationNodes;

static Path resPath;
PyObject* PathFinder::findPathBetweenLocations(const std::string &startLocation, PointInt xStart, PointInt yStart,
                                               const std::string &endLocation, double xEndFloat, double yEndFloat,
                                               PyObject *bannedExits, bool bruteForce)
{
	if (!bruteForce && startLocation == endLocation) {
		return PathFinder::findPath(startLocation, xStart, yStart, xEndFloat, yEndFloat, Py_None);
	}

	std::lock_guard g(pathFinderMutex);

	MipMap *startMap = getLocation(startLocation);
	MipMap *endMap = getLocation(endLocation);
	if (!startMap || !endMap) return PyTuple_New(0);

	if (!checkMapPoint(startMap, xStart, yStart) || !checkMapPoint(endMap, xEndFloat, yEndFloat)) {
		makeOutsideMsg("findPathBetweenLocations", xStart, yStart, xEndFloat, yEndFloat, startMap, endMap);
		return PyTuple_New(0);
	}
	if (!PyList_CheckExact(bannedExits)) {
		Utils::outMsg("PathFinder::findPathBetweenLocations", "Expects type(bannedExits) is list");
		return PyTuple_New(0);
	}

	//enable only needed places
	for (auto [name, mipMap] : mipMaps) {
		for (LocationPlace &place : mipMap->params.places) {
			place.enable = false;
			place.isBannedExit = false;
		}
	}
	for (auto [name, mipMap] : mipMaps) {
		for (LocationPlace &place : mipMap->params.places) {
			if (place.toLocationName.empty()) continue;

			place.enable = true;

			MipMap *tmpLoc = getLocation(place.toLocationName);
			if (!tmpLoc) continue;
			for (LocationPlace &tmpPlace : tmpLoc->params.places) {
				if (tmpPlace.name == place.toPlaceName) {
					tmpPlace.enable = true;
					break;
				}
			}
		}
	}

	//disable banned places
	size_t s = size_t(Py_SIZE(bannedExits));
	for (size_t i = 0; i < s; ++i) {
		PyObject *elem = PyList_GET_ITEM(bannedExits, i);
		if (!PyTuple_CheckExact(elem) || Py_SIZE(elem) != 2) {
			Utils::outMsg("PathFinder::findPathBetweenLocations", "Expects bannedExits[i] is tuple with len 2");
			continue;
		}

		PyObject *pyLocationName = PyTuple_GET_ITEM(elem, 0);
		PyObject *pyPlaceName = PyTuple_GET_ITEM(elem, 1);
		if (!PyUnicode_CheckExact(pyLocationName) || !PyUnicode_CheckExact(pyPlaceName)) {
			Utils::outMsg("PathFinder::findPathBetweenLocations", "Expects bannedExits[i] == (str, str)");
			continue;
		}

		std::string locationName = PyUtils::objToStr(pyLocationName);
		MipMap *location = getLocation(locationName);
		if (!location) continue;

		std::string placeName = PyUtils::objToStr(pyPlaceName);

		for (LocationPlace &place : location->params.places) {
			if (place.name == placeName) {
				place.isBannedExit = true;
				break;
			}
		}
	}

	uint16_t errorNodeId = uint16_t(-3);//-1 - undefined, -2 - end, -3 - start

	locationNodes.clear();
	locationNodes.reserve(mipMaps.size() * 4);

	//add needed places and exits to nodes
	for (const auto &[name, mipMap] : mipMaps) {
		for (const LocationPlace &place : mipMap->params.places) {
			if (!place.enable) continue;

			PointInt x = PointInt(std::min(place.x, int(mipMap->originalWidth - 1)));
			PointInt y = PointInt(std::min(place.y, int(mipMap->originalHeight - 1)));

			locationNodes.push_back({});
			LocationNode &node = locationNodes.back();
			node.id = uint16_t(locationNodes.size() - 1);
			if (node.id == errorNodeId) {
				Utils::outMsg("PathFinder::findPathBetweenLocations", "Too many nodes");
				return PyTuple_New(0);
			}

			node.changed = false;
			node.isExit = !place.toLocationName.empty();
			node.isBannedExit = place.isBannedExit;
			node.x = x;
			node.y = y;
			node.cost = FLT_MAX;

			node.placeName = place.name;
			node.toLocationName = place.toLocationName;
			node.toPlaceName = place.toPlaceName;

			node.mipMap = mipMap;

			if (node.isExit) {
				locationNodes.push_back(node);
				LocationNode &place = locationNodes.back();
				place.id = uint16_t(locationNodes.size() - 1);
				if (place.id == errorNodeId) {
					Utils::outMsg("PathFinder::findPathBetweenLocations", "Too many nodes");
					return PyTuple_New(0);
				}
				place.isExit = false;
			}
		}
	}

	PointInt xEnd = PointInt(xEndFloat);
	PointInt yEnd = PointInt(yEndFloat);

	//add start point
	locationNodes.push_back({});
	LocationNode &startNode = locationNodes.back();
	startNode.id = uint16_t(locationNodes.size() - 1);
	startNode.isExit = false;
	startNode.x = xStart;
	startNode.y = yStart;
	startNode.cost = 0;
	startNode.mipMap = startMap;
	startNode.prevId = uint16_t(-1);

	//add end point
	locationNodes.push_back({});
	LocationNode &endNode = locationNodes.back();
	endNode.id = uint16_t(locationNodes.size() - 1);
	endNode.changed = false;
	endNode.isExit = false;
	endNode.x = xEnd;
	endNode.y = yEnd;
	endNode.cost = FLT_MAX;
	endNode.mipMap = endMap;
	endNode.prevId = uint16_t(-1);

	if (startMap == endMap) {
		auto &[path, cost] = findAndSavePath(endMap, {xStart, yStart}, {xEnd, yEnd}, false);
		startNode.destinations.push_back({endNode.id, cost});
	}

	//link nodes
	tmpPaths.clear();
	for (LocationNode &node : locationNodes) {
		if (node.isExit) {
			if (node.isBannedExit) continue;

			for (const LocationNode &tmpNode : locationNodes) {
				if (tmpNode.isExit) continue;

				if (node.toLocationName == tmpNode.mipMap->name && node.toPlaceName == tmpNode.placeName) {
					node.destinations.push_back({tmpNode.id, EXIT_COST});
					break;
				}
			}
		}else {
			SimplePoint from = {node.x, node.y};
			MipMap *mipMap = node.mipMap;

			node.destinations.reserve(mipMap->params.places.size());

			for (const LocationNode &exitNode : locationNodes) {
				if (exitNode.mipMap != mipMap) continue;
				if (!exitNode.isExit && exitNode.id != endNode.id) continue;
				if (exitNode.toLocationName == node.toLocationName && exitNode.toPlaceName == node.toPlaceName) continue;

				SimplePoint to = {exitNode.x, exitNode.y};
				//const - from [in] to [out], else - tmp
				bool pathIsConst = node.id != startNode.id && exitNode.id != endNode.id;
				auto &[path, cost] = findAndSavePath(mipMap, from, to, pathIsConst);

				if (!path.empty()) {
					node.destinations.push_back({exitNode.id, cost});
				}
			}
		}
	}

	//search
	startNode.changed = true;

	bool changed = true;
	while (changed) {
		changed = false;

		for (LocationNode &node : locationNodes) {
			if (!node.changed) continue;

			for (auto [id, cost] : node.destinations) {
				LocationNode &destination = locationNodes[id];
				float nextCost = node.cost + cost;

				if (destination.cost > nextCost) {
					destination.cost = nextCost;
					destination.prevId = node.id;
					destination.changed = true;
					changed = true;
				}
			}
			node.changed = false;
		}
	}

	//get result
	resPath.clear();
	uint16_t lastId = endNode.id;
	while (lastId != uint16_t(-1) && locationNodes[lastId].prevId != uint16_t(-1)) {
		LocationNode &localEndNode = locationNodes[lastId];
		uint16_t startId = localEndNode.prevId;
		LocationNode &localStartNode = locationNodes[startId];
		lastId = localStartNode.prevId;

		//const - from [in] to [out], else - tmp
		bool pathIsConst = endNode.id != localEndNode.id && startNode.id != localStartNode.id;
		auto &[subPath, cost] = findAndSavePath(localEndNode.mipMap,
		                                        {localStartNode.x, localStartNode.y},
		                                        {localEndNode.x, localEndNode.y},
		                                        pathIsConst);
		resPath.insert(resPath.end(), subPath.begin(), subPath.end());//path += subPath

		resPath.push_back({PointInt(-1), startId});
	}
	if (resPath.size() > 1) {
		resPath.back() = {xStart, yStart};
	}

	//set result
	long resSize = long(resPath.size()) * 2;
	PyObject *res = PyTuple_New(resSize);
	if (resSize) {
		for (size_t i = resPath.size() - 1; i != 0; --i) {
			auto [x, y] = resPath[i];

			if (x == PointInt(-1)) {
				LocationNode &node = locationNodes[y];
				PyTuple_SET_ITEM(res, resSize - long(i) * 2 - 2, convertToPy(node.mipMap->name));
				PyTuple_SET_ITEM(res, resSize - long(i) * 2 - 1, convertToPy(node.placeName));
			}else {
				PyTuple_SET_ITEM(res, resSize - long(i) * 2 - 2, PyLong_FromLong(x));
				PyTuple_SET_ITEM(res, resSize - long(i) * 2 - 1, PyLong_FromLong(y));
			}
		}
		PyTuple_SET_ITEM(res, resSize - 2, PyFloat_FromDouble(xEndFloat));
		PyTuple_SET_ITEM(res, resSize - 1, PyFloat_FromDouble(yEndFloat));
	}
	return res;
}
