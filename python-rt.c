#include "Python.h"

#include "stdio.h"
#include <stdio.h>
#include <time.h>
#include <sched.h>
#include <sys/mman.h>
#include <string.h>

// use PyModule_AddIntMacro
// #define PY_RT_ADD_INT_CONSTANT(c)  { PyModule_AddIntConstant(py_rt_module, #c, c); }

#define NSEC_PER_SEC    (1e9L) /* The number of nsecs per sec. */

PyObject *py_rt_module;		// the object refering to this module
PyObject *py_rt_error;		// the module common error

PyDoc_VAR(py_rt_doc) = PyDoc_STR("Realtime for Python.\n");

// sched_setscheduler
PyDoc_STRVAR(py_sched_setscheduler_doc,
"usage: sched_setscheduler([prio=49,sched=SCHED_FIFO])\n\
Declare yourself as a real time task. Use 49 max. as\n\
PRREMPT_RT use 50 as the priority of kernel tasklets\n\
and interrupt handler by default.");

PyObject *py_sched_setscheduler(PyObject * self, PyObject * args,
				PyObject * kwds)
{

	static char *kwlist[] = { "prio", "sched", NULL };

	int prio = 49; // TODO #define from header?
	int sched = SCHED_FIFO;

	if (!PyArg_ParseTupleAndKeywords
	    (args, kwds, "|ii", kwlist, &prio, &sched)) {
		goto usage;
	}

	struct sched_param param;
	param.sched_priority = prio;

	if (sched_setscheduler(0, sched, &param) == -1) {
		PyErr_Format(py_rt_error, "sched_setscheduler failed!");
		return NULL;
	}

	Py_INCREF(Py_None);
	return Py_None;

 usage:
	PyErr_Format(py_rt_error,
		     "usage: sched_setscheduler(prio=[,sched=SCHED_FIFO])");
	return NULL;

}

// clock_nanosleep
PyDoc_STRVAR(py_clock_nanosleep_doc,
"usage: clock_nanosleep(ns)\n\
Sleep for nanoseconds.");

PyObject *py_clock_nanosleep(PyObject * self, PyObject * args,
				PyObject * kwds)
{

	long ns_to_sleep;

	if (!PyArg_ParseTuple
	    (args, "l", &ns_to_sleep)) {
		goto usage;
	}

	struct timespec t;
  
  clock_gettime(CLOCK_MONOTONIC ,&t);

  t.tv_nsec += ns_to_sleep;
  while (t.tv_nsec >= NSEC_PER_SEC) {
    t.tv_nsec -= NSEC_PER_SEC;
    t.tv_sec++;
  };

	clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t, NULL);

  Py_INCREF(Py_None);
	return Py_None;

 usage:
	PyErr_Format(py_rt_error,
		     "usage: clock_nanosleep(ns)");
	return NULL;

}
// the module's methods
static struct PyMethodDef py_rt_methods[] = {
	{"sched_setscheduler", (PyCFunction) py_sched_setscheduler,
	 METH_VARARGS | METH_KEYWORDS, py_sched_setscheduler_doc},
	{"clock_nanosleep", (PyCFunction) py_clock_nanosleep,
	 METH_VARARGS, py_clock_nanosleep_doc},
{NULL}			/* sentinel */
};

// 
PyMODINIT_FUNC initrt()
{
	Py_Initialize();
	if ((py_rt_module =
	     Py_InitModule3("rt", py_rt_methods, py_rt_doc)) == NULL)
		return;

	py_rt_error = PyErr_NewException("rt.error", NULL, NULL);
	PyModule_AddObject(py_rt_module, "error", py_rt_error);

	PyModule_AddStringConstant(py_rt_module, "version", PY_RT_VERSION);
  //PyModule_AddIntMacro(py_rt_module, SCHED_NORMAL);
  PyModule_AddIntMacro(py_rt_module, SCHED_FIFO);
  PyModule_AddIntMacro(py_rt_module, SCHED_RR);
  PyModule_AddIntMacro(py_rt_module, SCHED_BATCH);

}
