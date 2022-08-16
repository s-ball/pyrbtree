#include "rbtree.h"

#define Py_LIMITED_API
#define PY_SSIZE_T_CLEAN
#include <Python.h>

// The module object
static PyObject* mod = NULL;

// Exception names
static const char* subexc[] = { "INSERT_ERROR", "BLACK_VIOLATION", "RED_VIOLATION",
    "RED_ROOT", "DEPTH_ERROR", "ORDER_ERROR", "COUNT_ERROR", };

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
    {"next", (PyCFunction)RBIterObjectNext, METH_NOARGS,
    PyDoc_STR("Get next element")},
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
        PyObject* exc = PyObject_GetAttrString(mod, subexc[0]);
        if (exc) {
            PyErr_SetString(exc, "Could not insert element");
            Py_DECREF(exc);
        }
        else {
            PyErr_SetString(PyExc_RuntimeError, "Could not insert element");
        }
        return NULL;
    }
    Py_INCREF(data);
    return (NULL == previous) ? Py_None : previous;
}

static PyObject* RBTreeRemove(RBTreeObject* self, PyObject* args) {
    PyObject* data = NULL;
    int cr = PyArg_UnpackTuple(args, "key", 1, 1, &data);
    if (!cr) return NULL;
    PyObject* old = RBremove(&self->tree, data);
    if (!old) Py_RETURN_NONE;
    return old;
}

static PyObject* RBTreeFind(RBTreeObject* self, PyObject* args) {
    PyObject* data = NULL;
    int cr = PyArg_UnpackTuple(args, "key", 1, 1, &data);
    if (!cr) return NULL;
    PyObject* old = RBfind(&self->tree, data);
    if (!old) Py_RETURN_NONE;
    return old;
}

static PyObject* build_iter(RBIter *iter) {
    RBIterObject* iterobj = NULL;
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
    return (PyObject*)iterobj;
}

static PyObject* RBTreeSearch(RBTreeObject* self, PyObject* args) {
    PyObject* data = NULL;
    int cr = PyArg_UnpackTuple(args, "key", 1, 1, &data);
    if (!cr) return NULL;
    RBIter* iter = RBsearch(&self->tree, data);
    return build_iter(iter);
}

static PyObject* RBTreeFirst(RBTreeObject* self, PyObject *args) {
    RBIterObject* iterobj = NULL;
    RBIter* iter = RBfirst(&self->tree);
    return build_iter(iter);
}

static PyMethodDef methods[] = {
    {"insert", (PyCFunction)RBTreeInsert, METH_VARARGS, PyDoc_STR("Inserts an element")},
    {"remove", (PyCFunction)RBTreeRemove, METH_VARARGS, PyDoc_STR("Removes an element")},
    {"find", (PyCFunction)RBTreeFind, METH_VARARGS, PyDoc_STR("Finds an element")},
    {"search", (PyCFunction)RBTreeSearch, METH_VARARGS, PyDoc_STR("Searches for an element")},
    {"first", (PyCFunction)RBTreeFirst, METH_NOARGS, PyDoc_STR("Gets an iterator on first element") },
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
    PyObject* base_exc = NULL;
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
    // Add Exception classes
    base_exc = PyErr_NewException("RBError", NULL, NULL);
    if (!base_exc) goto except;
    cr = PyModule_AddObjectRef(mod, "RBError", typ);
    if (cr) goto except;
    for (int i = 0; i < sizeof(subexc) / sizeof(*subexc); i++) {
        typ = PyErr_NewException(subexc[i], base_exc, NULL);
        if (!typ) goto except;
        cr = PyModule_AddObjectRef(mod, subexc[i], typ);
        Py_DECREF(typ);
        if (cr) goto except;
    }
    if (0 != PyModule_AddStringConstant(mod, "__author__", "SBA")) goto except;
    goto finally;

except:
     if (mod) Py_DECREF(mod);

finally:
    if (base_exc) Py_DECREF(base_exc);
    return mod;
}
