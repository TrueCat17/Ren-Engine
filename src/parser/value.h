#ifndef VALUE_H
#define VALUE_H

#include "utils/string.h"
#include "utils/utils.h"

enum {
	DOUBLE_VALUE,
	STRING_VALUE
};
namespace qwe {
class Value {
public:
	char type;

	double dValue = 0;
	String sValue = "";

	Value(double d);
	Value(String s);

	operator double() const { if (type != DOUBLE_VALUE) {Utils::outMsg("Value is not double type");} return dValue; }
	operator String() const { if (type != STRING_VALUE) {Utils::outMsg("Value is not String type");} return sValue; }
};
}
#endif // VALUE_H
