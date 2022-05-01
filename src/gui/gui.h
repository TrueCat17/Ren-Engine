#ifndef GUI_H
#define GUI_H

typedef struct _object PyObject;

class GUI {
public:
	static PyObject* getScreenTimes();
	static void update();
};

#endif // GUI_H
