#include "py_code_disk_cache.h"

#include <algorithm>
#include <fstream>
#include <vector>

#include "media/py_utils.h"
#include "utils/message.h"
#include "utils/utils.h"


static const char *path = "../var/py_code_cache";
const uint32_t MAX_SIZE = 5 << 20; //5 MB
const uint32_t VERSION = PY_VERSION_HEX + 1;

static bool changed = false;
static uint32_t lastId = 0;


struct PyCode {
	std::string code;
	std::string fileName;
	uint32_t numLine;
	uint32_t id;
	PyObject *pyCodeObject;
	std::vector<char> serialized;

	PyCode(const std::vector<char> &code,
	       const std::vector<char> &fileName,
	       uint32_t numLine,
	       uint32_t id,
	       const std::vector<char> &serialized):
	    code(code.data(), code.size()),
	    fileName(fileName.data(), fileName.size()),
	    numLine(numLine),
	    id(id),
	    pyCodeObject(nullptr),
	    serialized(serialized)
	{}

	PyCode(const std::string &code,
	       const std::string &fileName,
	       uint32_t numLine,
	       uint32_t id,
	       PyObject *pyCodeObject):
	    code(code),
	    fileName(fileName),
	    numLine(numLine),
	    id(id),
	    pyCodeObject(pyCodeObject)
	{}

	void makeDump() {
		PyObject *pyBytes = PyObject_CallFunction(PyUtils::marshalDumps, "O", pyCodeObject);
		pyCodeObject = nullptr;

		if (!pyBytes || !PyBytes_CheckExact(pyBytes)) {
			Message::outMsg("PyCodeDiskCache, makeDump", "Failed to call marshal.dumps(code_object)");
			PyErr_Clear();
			return;
		}

		const char *serializedPtr = PyBytes_AS_STRING(pyBytes);
		serialized.insert(serialized.begin(), serializedPtr, serializedPtr + Py_SIZE(pyBytes));

		Py_DECREF(pyBytes);
	}
};
static std::vector<PyCode> serializedCodes;



static uint32_t read4(std::ifstream &is) {
	char v[4];
	is.read(v, 4);

	uint32_t res;
	memcpy(&res, v, 4);
	return res;
}

void PyCodeDiskCache::init() {
	std::ifstream is(path, std::ios::binary);
	if (!is.is_open()) return;

	uint32_t version = read4(is);
	if (version != VERSION) return;

	uint32_t count = read4(is);
	serializedCodes.reserve(count);

	std::vector<char> code(64 << 10);
	std::vector<char> fileName(256);
	std::vector<char> serialized(64 << 10);

	for (size_t i = 0; i < count; ++i) {
		uint32_t codeSize = read4(is);
		code.resize(codeSize);
		is.read(code.data(), codeSize);

		uint32_t fileNameSize = read4(is);
		fileName.resize(fileNameSize);
		is.read(fileName.data(), fileNameSize);

		uint32_t numLine = read4(is);

		uint32_t id = read4(is);
		lastId = std::max(lastId, id);

		uint32_t serializedSize = read4(is);
		serialized.resize(serializedSize);
		is.read(serialized.data(), serializedSize);

		serializedCodes.push_back(PyCode(code, fileName, numLine, id, serialized));
	}
}



static bool isSmallCode(const std::string code) {
	return code.find('\n') == size_t(-1);
}

PyObject* PyCodeDiskCache::get(const std::string &code, const std::string &fileName, uint32_t numLine) {
	if (isSmallCode(code)) return nullptr;

	PyCode *pyCode = nullptr;
	for (PyCode &t : serializedCodes) {
		if (t.code == code && t.fileName == fileName && t.numLine == numLine) {
			t.id = ++lastId;
			pyCode = &t;
			break;
		}
	}
	if (!pyCode) return nullptr;

	const std::vector<char> &bytes = pyCode->serialized;
	PyObject *pySerialized = PyBytes_FromStringAndSize(bytes.data(), Py_ssize_t(bytes.size()));

	PyObject *pyCodeObject = PyObject_CallFunction(PyUtils::marshalLoads, "O", pySerialized);
	if (!pyCodeObject) {
		Message::outMsg("PyCodeDiskCache::get", "Failed to call marshal.loads(serialized)");
		PyErr_Clear();
	}

	return pyCodeObject;
}


void PyCodeDiskCache::set(const std::string &code, const std::string&fileName, uint32_t numLine,
                          PyObject *pyCodeObject)
{
	if (isSmallCode(code)) return;

	changed = true;
	serializedCodes.push_back(PyCode(code, fileName, numLine, ++lastId, pyCodeObject));
}



static void trim() {
	std::sort(serializedCodes.begin(), serializedCodes.end(),
	          [](const PyCode &a, const PyCode &b) -> bool { return a.id > b.id; });

	size_t size = 0;
	size_t i;
	for (i = 0; i < serializedCodes.size(); ++i) {
		const PyCode &pyCode = serializedCodes[i];
		size += pyCode.code.size();
		size += pyCode.fileName.size();
		size += pyCode.serialized.size();

		if (size >= MAX_SIZE) break;
	}
	if (size < MAX_SIZE) return;

	serializedCodes.erase(serializedCodes.begin() + long(i), serializedCodes.end());
}

static void write4(std::ofstream &os, uint32_t v32) {
	char v[4];
	memcpy(v, &v32, 4);

	os.write(v, 4);
}

void PyCodeDiskCache::checkSaving() {
	if (!changed) return;

	double st = Utils::getTimer();
	for (PyCode &pyCode : serializedCodes) {
		if (pyCode.pyCodeObject) {
			pyCode.makeDump();
		}

		if (Utils::getTimer() - st > 0.005) return;
	}

	trim();
	changed = false;

	std::ofstream os(path, std::ios::binary);
	write4(os, VERSION);

	write4(os, uint32_t(serializedCodes.size()));
	for (const PyCode &pyCode : serializedCodes) {
		write4(os, uint32_t(pyCode.code.size()));
		os << pyCode.code;

		write4(os, uint32_t(pyCode.fileName.size()));
		os << pyCode.fileName;

		write4(os, pyCode.numLine);

		write4(os, pyCode.id);

		write4(os, uint32_t(pyCode.serialized.size()));
		os.write(pyCode.serialized.data(), std::streamsize(pyCode.serialized.size()));
	}
}
