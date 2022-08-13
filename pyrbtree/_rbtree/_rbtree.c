#include "CRBTree/rbtree/rbtree.h"

#define Py_LIMITED_API
#define PY_SSIZE_T_CLEAN
#include <Python.h>

static PyObject* mod = NULL;

/*
 * Documentation for _rbtree.
 */
PyDoc_STRVAR(_rbtree_doc, "CRBTree wrapper module");


static PyModuleDef _rbtree_def = {
    PyModuleDef_HEAD_INIT,
    "_rbtree",
    _rbtree_doc,
    -1,              /* m_size */
};

typedef struct {
    PyObject_HEAD
        /* Type-specific fields go here. */
        RBIter *iter;
} RBIterObject;

static void RBIter_dealloc(RBIterObject* self) {
    RBiter_release(self->iter);
}

static PyObject* RBIterObjectNext(RBIterObject* self) {
    PyObject* data = RBnext(self->iter);
    if (NULL == data) {
        Py_RETURN_NONE;
    }
    Py_INCREF(data);
    return data;
}

static PyMethodDef iter_methods[] = {
    {"next", (PyCFunction)RBIterObjectNext, METH_NOARGS, "Get next element"},
    {NULL},
};

static PyType_Slot RBIterSlot[] = {
    {Py_tp_doc, PyDoc_STR("Internal RBIter")},
    {Py_tp_methods, iter_methods},
    {Py_tp_dealloc, RBIter_dealloc},
    {0, NULL},
};

static PyType_Spec RBIterTypeSpec = {
    .name = "_rbtree.RBIter",
    .basicsize = sizeof(RBIterObject),
    .flags = Py_TPFLAGS_DEFAULT,
    .slots = RBIterSlot,
};


typedef struct {
    PyObject_HEAD
        /* Type-specific fields go here. */
        RBTree tree;
} RBTreeObject;

static int obj_comp(const void* first, const void* second, int *err) {
    PyObject* ofirst = (PyObject *)first;
    PyObject* osecond = (PyObject*)second;
    int cr = PyObject_RichCompareBool(ofirst, osecond, Py_LT);
    if (cr == 1) return -1;
    if (cr == -1) {
        *err = 1;
        return 0;
    }
    cr = PyObject_RichCompareBool(ofirst, osecond, Py_EQ);
    if (cr == -1) {
        *err = 1;
        return 0;
    }
    return (cr == 0);
}

static int
RBTree_init(RBTreeObject* self, PyObject* args, PyObject* kwds) {
    RBinit2(&self->tree, obj_comp);
    return 0;
}

static void node_deleter(const void* data) {
    Py_DECREF((PyObject*)data);
}
static void RBTree_dealloc(RBTreeObject* self) {
    RBdestroy(&self->tree, node_deleter);
}

static PyObject *RBTreeInsert(RBTreeObject *self, PyObject* args) {
    PyObject* data = NULL;
    int cr = PyArg_UnpackTuple(args, "value", 1, 2, &data);
    if (!cr) return NULL;
    int err = 0;
    PyObject* previous = RBinsert(&self->tree, data, &err);
    if (err) {
        PyErr_SetString(PyExc_RuntimeError, "Could not insert element");
        return NULL;
    }
    Py_INCREF(data);
    return (NULL == previous) ? Py_None : previous;
}

static PyObject* RBTreeFirst(RBTreeObject* self, PyObject *args) {
    RBIterObject* iterobj = NULL;
    RBIter* iter = RBfirst(&self->tree);
    if (NULL == iter) {
        PyErr_SetString(PyExc_MemoryError, "Could not allocate iterator");
        return NULL;
    }
    PyTypeObject* itertyp = (PyTypeObject*)PyObject_GetAttrString(mod, "RBIter");
    if (NULL == itertyp) goto except;
    iterobj = PyObject_New(RBIterObject, itertyp);
    if (NULL == iterobj) goto except;
    iterobj->iter = iter;
    goto finally;
except:
    if (iter) RBiter_release(iter);
finally:
    Py_XDECREF(itertyp);
    return (PyObject*) iterobj;
}

static PyMethodDef methods[] = {
    {"insert", (PyCFunction)RBTreeInsert, METH_VARARGS, "Inserts a element"},
    {"first", (PyCFunction)RBTreeFirst, METH_NOARGS, "Get an iterator on first element" },
    {NULL},
};

static PyType_Slot RBtreeSlot[] = {
    {Py_tp_doc, PyDoc_STR("Internal RBTree")},
    {Py_tp_init, (initproc) RBTree_init},
    {Py_tp_methods, methods},
    {Py_tp_dealloc, RBTree_dealloc},
    {0, NULL},
};

static PyType_Spec RBTreeTypeSpec = {
    .name = "_rbtree.RBTree",
    .basicsize = sizeof(RBTreeObject),
    .flags = Py_TPFLAGS_DEFAULT,
    .slots = RBtreeSlot,
};

PyMODINIT_FUNC PyInit__rbtree() {
    PyObject* typ = NULL;
    mod = PyModule_Create(&_rbtree_def);
    if (!mod) goto except;
    typ = PyType_FromSpec(&RBTreeTypeSpec);
    if (!typ) goto except;
    int cr = PyModule_AddObjectRef(mod, "RBTree", typ);
    Py_DECREF(typ);
    typ = PyType_FromSpec(&RBIterTypeSpec);
    if (!typ) goto except;
    cr = PyModule_AddObjectRef(mod, "RBIter", typ);
    Py_DECREF(typ);
    if (cr) goto except;
    if (0 != PyModule_AddStringConstant(mod, "__author__", "SBA")) goto except;
    char versionstr[12];
    unsigned const char *version = RBversion();
    sprintf(versionstr, "%d.%d.%d", version[0], version[1], version[2]);
    if (0 != PyModule_AddStringConstant(mod, "__version__", versionstr)) goto except;
    if (0 != PyModule_AddIntConstant(mod, "year", 2022)) goto except;
    goto finally;

except:
     if (mod) Py_DECREF(mod);
finally:
    return mod;
}
