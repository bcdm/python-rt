/*
 *    $Id$
 *
 *    Copyright (C) 2011  Matthieu Bec
 *  
 *    This file is part of python-rt.
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http: *www.gnu.org/licenses/>.
 */

#include "Python.h"

#include <stdio.h>
#include <time.h>
#include <sched.h>
#include <sys/mman.h>
#include <string.h>

#ifndef PY_RT_VERSION
#define PY_RT_VERSION "devel"
#endif

static PyObject *py_rt_module;
static PyObject *py_rt_error;
PyDoc_STRVAR(py_rt_doc, "Real time flavored extensions for Python.");

/*
 * sched_setscheduler
 */
PyDoc_STRVAR(py_sched_setscheduler_doc,
	     "Usage: sched_setscheduler(prio=49,sched=SCHED_FIFO).");

static PyObject *py_sched_setscheduler(PyObject * self, PyObject * args,
				PyObject * kwds)
{

	static char *kwlist[] = { "prio", "sched", NULL };

	int prio = 49;		/* TODO #define'd ? */
	int sched = SCHED_FIFO;

	if (!PyArg_ParseTupleAndKeywords
	    (args, kwds, "|ii", kwlist, &prio, &sched)) {
		goto usage;
	}

	struct sched_param param;
	param.sched_priority = prio;

	if (sched_setscheduler(0, sched, &param)) {
		PyErr_SetFromErrno(py_rt_error);
		return NULL;
	}

	Py_INCREF(Py_None);
	return Py_None;

 usage:
	PyErr_Format(py_rt_error,py_sched_setscheduler_doc);
	return NULL;

}

/*
 * clock_nanosleep
 */
PyDoc_STRVAR(py_clock_nanosleep_doc, "Usage: clock_nanosleep(ns).");
static PyObject *py_clock_nanosleep(PyObject * self, PyObject * args, PyObject * kwds)
{

	long ns_to_sleep;

	if (!PyArg_ParseTuple(args, "l", &ns_to_sleep)) {
		goto usage;
	}

	struct timespec t;

  t.tv_sec=0;
	t.tv_nsec = ns_to_sleep;
	while (t.tv_nsec >= (1e9L)) {
		t.tv_nsec -= (1e9L);
		t.tv_sec++;
	};

	if (clock_nanosleep(CLOCK_MONOTONIC, 0, &t, NULL)) {
    PyErr_SetFromErrno(py_rt_error);
		return NULL;
  }

	Py_INCREF(Py_None);
	return Py_None;

 usage:
	PyErr_Format(py_rt_error, py_clock_nanosleep_doc);
	return NULL;

}

/*
 * nano_sched
 */
PyDoc_STRVAR(py_nanosched_doc, "Usage: ns_next = nanosched(ns_next, ns, func, [*args,**kwds]).");
static PyObject *py_nanosched(PyObject * self, PyObject * args, PyObject * kwds)
{

	struct timespec t;
  struct timespec tnow;
	long ns_to_sleep;
	long ns_next;
  long ns_avail;
  PyObject *func;
  PyObject *retval = NULL;
  
  clock_gettime(CLOCK_MONOTONIC, &tnow);
  
  if (!PyArg_ParseTuple(args, "llO", &ns_next, &ns_to_sleep, &func)) {
		goto usage;
  }

  if (ns_next>0) {
    if (ns_next < tnow.tv_nsec + tnow.tv_sec * (1e9L)) {
        PyErr_Format(py_rt_error, "clock overrun!");
        return NULL;
    }
    t.tv_sec=0;
    t.tv_nsec=ns_next;
    while (t.tv_nsec >= (1e9L)) {
			t.tv_nsec -= (1e9L);
			t.tv_sec++;
	  }
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t, NULL);
  } else {
     t=tnow;
     t.tv_sec++; /* first time, starts from now +1 second */
  }

  if (!PyCallable_Check (func)) {
    goto usage;
  }

  // callback
  //Py_XINCREF (kwds);
  Py_INCREF (func);
  //Py_INCREF (args);
  retval = PyObject_CallFunctionObjArgs(func, NULL); // var args + NULL sentinel
  //Py_XDECREF (kwds);
  //Py_DECREF (args);
  Py_DECREF (func);
  
  // compute and return remainder
	t.tv_nsec += ns_to_sleep;
	
	if (retval == NULL)
  {
      //printf ("xxxxxxxxxxxxxx\n");
      PyErr_Format(py_rt_error, "func callback failed!");
      // fixme: redundant?
      //if (PyErr_Occurred ())
	    //  {
	    //  PyErr_Print ();
	    //  PyErr_Clear ();
	    //  }
      return NULL;
  }

  clock_gettime(CLOCK_MONOTONIC, &tnow);
  /* tuple (tnext, tspent) */
  ns_next = t.tv_nsec + t.tv_sec * (1e9L);
  ns_avail = ns_next - (tnow.tv_nsec + tnow.tv_sec * (1e9L));
  retval = PyTuple_New (2);
  PyTuple_SetItem (retval, 0, PyInt_FromLong (ns_next));
  PyTuple_SetItem (retval, 1, PyInt_FromLong (ns_avail));
  return retval;

 usage:
	PyErr_Format(py_rt_error, py_nanosched_doc);
	return NULL;

}

/*
 * module functions
 */
static struct PyMethodDef py_rt_methods[] = {
	{"sched_setscheduler", (PyCFunction) py_sched_setscheduler,
	 METH_VARARGS | METH_KEYWORDS, py_sched_setscheduler_doc},
	{"clock_nanosleep", (PyCFunction) py_clock_nanosleep,
	 METH_VARARGS, py_clock_nanosleep_doc},
	{"nanosched", (PyCFunction) py_nanosched,
	 METH_VARARGS | METH_KEYWORDS, py_nanosched_doc},
  {NULL}		/* sentinel */
};

/*
 * module init
 */
PyMODINIT_FUNC initrt()
{

Py_Initialize();
	if ((py_rt_module =
	     Py_InitModule3("rt", py_rt_methods, py_rt_doc)) == NULL)
		return;

	py_rt_error = PyErr_NewException("rt.error", NULL, NULL);
	PyModule_AddObject(py_rt_module, "error", py_rt_error);

	PyModule_AddStringConstant(py_rt_module, "version", PY_RT_VERSION);
	PyModule_AddIntMacro(py_rt_module, SCHED_OTHER); /* the standard round-robin time-sharing policy */
	PyModule_AddIntMacro(py_rt_module, SCHED_FIFO); /* a first-in, first-out policy */
	PyModule_AddIntMacro(py_rt_module, SCHED_RR); /* a round-robin policy */
	PyModule_AddIntMacro(py_rt_module, SCHED_IDLE); /* for running very low priority background jobs */
	PyModule_AddIntMacro(py_rt_module, SCHED_BATCH); /* for "batch" style execution of processes */

}
