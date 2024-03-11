#ifndef CONVERT_TO_PY_H
#define CONVERT_TO_PY_H

#include <string>

#include <Python.h>


template<typename T>
PyObject* convertToPy(T obj);



/* fixes for next problem:
 * simple way:
 *   convertToPy(str) = convertToPy<std::string>(str) - undefined
 * correct way:
 *   convertToPy(str) = convertToPy<const std::string&>(str) - ok
 * problem places are [const/not] and [ref/obj]
 */

template<>
PyObject* convertToPy<>(const std::string &obj);


//not template funcs: overloading, not specialization of template
inline
PyObject* convertToPy(std::string &obj) {
	return convertToPy<const std::string&>(obj);
}
inline
PyObject* convertToPy(const std::string &obj) {
	return convertToPy<const std::string&>(obj);
}


#endif // CONVERT_TO_PY_H
