#include "rbtree.h"

#define Py_LIMITED_API
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <structmember.h>

// The module object
static PyObject* mod = NULL;

// Exception names
static const char* subexc[] = { "InsertError", "SearchError", };
static const char* err_label = "Incompatible types";

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
        if (NULL == PyErr_Occurred()) {
            PyErr_SetString(PyExc_StopIteration, "");
        }
    }
    else {
        Py_INCREF(data);
    }
    return data;
}

static PyObject* RBIterObject_iter(PyObject* self) {
    return self;
}

static PyMethodDef iter_methods[] = {
    {"next", (PyCFunction)RBIterObjectNext, METH_NOARGS,
    PyDoc_STR("Get next element")},
    {NULL},
};

static PyType_Slot RBIterSlot[] = {
    {Py_tp_doc, PyDoc_STR("Internal RBIter")},
    {Py_tp_methods, iter_methods},
    {Py_tp_iter, RBIterObject_iter},
    {Py_tp_iternext, RBIterObjectNext},
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

static int key_comp(const void* first, const void* second, int* err) {
    if ((!PyTuple_Check((PyObject*)first)) || (!PyTuple_Check((PyObject*)second))) {
        goto except;
    }
    PyObject* ofirst = PyTuple_GetItem((PyObject*)first, 0);
    PyObject* osecond = PyTuple_GetItem((PyObject*)second, 0);
    if ((NULL == ofirst) || (NULL == osecond)) {
        goto except;
    }
    return obj_comp(ofirst, osecond, err);
except:
    PyErr_SetString(PyExc_TypeError, "Tuple expected");
    *err = 1;
    return 0;
}

static int
RBTree_init(RBTreeObject* self, PyObject* args, PyObject* kwds) {
    int mapping = 0;
    if (! PyArg_ParseTuple(args, "|p", &mapping)) return -1;
    RBinit2(&self->tree, mapping ? key_comp : obj_comp);
    return 0;
}

static void node_deleter(const void* data) {
    Py_DECREF((PyObject*)data);
}
static void RBTree_dealloc(RBTreeObject* self) {
    RBdestroy(&self->tree, node_deleter);
}

int wrap_exception(const char *exc_name, const char* label) {
    PyObject* exc, * val, * val2, * tb;
    PyObject* exc_type = NULL;

    if (NULL == PyErr_Occurred()) {
        exc_type = PyObject_GetAttrString(mod, exc_name);
        if (NULL == exc_type) {
            exc_type = PyExc_RuntimeError;
            Py_INCREF(exc_type);
        }
        PyErr_SetString(exc_type, label);
    }
    else {
        PyErr_Fetch(&exc, &val, &tb);
        PyErr_NormalizeException(&exc, &val, &tb);
        if (tb != NULL) {
            PyException_SetTraceback(val, tb);
            Py_DECREF(tb);
        }
        Py_DECREF(exc);
        exc_type = PyObject_GetAttrString(mod, exc_name);
        if (NULL == exc_type) {
            exc_type = PyExc_RuntimeError;
            Py_INCREF(exc_type);
        }
        PyErr_SetString(exc_type, label);
        PyErr_Fetch(&exc, &val2, &tb);
        PyErr_NormalizeException(&exc, &val2, &tb);
        Py_INCREF(val);
        PyException_SetCause(val2, val);
        PyException_SetContext(val2, val);
        PyErr_Restore(exc, val2, tb);
    }
    Py_DECREF(exc_type);
    return 1;
}

static PyObject* RBTreeInsert(RBTreeObject* self, PyObject* args) {
    PyObject* data = NULL;
    int cr = PyArg_UnpackTuple(args, "value", 1, 2, &data);
    if (!cr) return NULL;
    int err = 0;
    PyObject* previous = RBinsert(&self->tree, data, &err);
    if (err) {
        wrap_exception(subexc[0], err_label);
        return NULL;
    }
    Py_INCREF(data);
    if (NULL == previous) {
        Py_RETURN_NONE;
    }
    return previous;
}

static PyObject* RBTreeRemove(RBTreeObject* self, PyObject* args) {
    PyObject* data = NULL;
    int cr = PyArg_UnpackTuple(args, "key", 1, 1, &data);
    if (!cr) return NULL;
    PyObject* old = RBremove(&self->tree, data);
    if (!old) {
        if (NULL != PyErr_Occurred()) {
            PyErr_Clear();
        }
        Py_RETURN_NONE;
    }
    return old;
}

static PyObject* find(RBTreeObject* self, PyObject* data) {
    PyObject* old = RBfind(&self->tree, data);
    if (!old) {
        if (NULL != PyErr_Occurred()) {
            PyErr_Clear();
        }
        Py_RETURN_NONE;
    }
    Py_INCREF(old);
    return old;
}

static PyObject* RBTreeGetItem(RBTreeObject* self, PyObject* key) {
    PyObject* obj = PyTuple_Pack(2, key, Py_None);
    if (NULL == obj) return NULL;
    PyObject* old = find(self, obj);
    Py_DECREF(obj);
    if (old == Py_None) {
        Py_DECREF(old);
        PyErr_SetString(PyExc_KeyError, "Not found");
        return NULL;
    }
    obj = old;
    old = PyTuple_GetItem(obj, 1);
    Py_INCREF(old);
    Py_DECREF(obj);
    return old;
}

static PyObject* build_args(PyObject* key, PyObject* val) {
    PyObject* obj = PyTuple_Pack(2, key, val);
    if (NULL == obj) return NULL;
    PyObject* args = PyTuple_Pack(1, obj);
    Py_DECREF(obj);
    return args;
}
static int RBTreeSetItem(RBTreeObject* self, PyObject* key, PyObject* val) {
    PyObject* args, *old = NULL;
    if (NULL == val) {
        args = build_args(key, Py_None);
        if (NULL == args) return -1;
        old = RBTreeRemove(self, args);
        Py_DECREF(args);
        if (Py_None == old) {
            Py_DECREF(old);
            PyErr_SetString(PyExc_KeyError, "Not found");
            return -1;
        }
    }
    else {
        args = build_args(key, val);
        old = RBTreeInsert(self, args);
        Py_DECREF(args);
        if (NULL == old) return -1;
    }
    Py_DECREF(old);
    return 0;
}

static PyObject* RBTreeFind(RBTreeObject* self, PyObject* args) {
    PyObject* data = NULL;
    int cr = PyArg_UnpackTuple(args, "key", 1, 1, &data);
    if (!cr) return NULL;
    return find(self, data);
}

static PyObject* build_iter(RBIter *iter) {
    RBIterObject* iterobj = NULL;
    if (NULL == iter) {
        wrap_exception(subexc[1], err_label);
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
    RBIter* iter = RBfirst(&self->tree);
    return build_iter(iter);
}

static PyObject* RBTree_iter(PyObject* self) {
    return RBTreeFirst((RBTreeObject*)self, NULL);
}

static PyMethodDef methods[] = {
    {"insert", (PyCFunction)RBTreeInsert, METH_VARARGS, PyDoc_STR("Inserts an element")},
    {"remove", (PyCFunction)RBTreeRemove, METH_VARARGS, PyDoc_STR("Removes an element")},
    {"find", (PyCFunction)RBTreeFind, METH_VARARGS, PyDoc_STR("Finds an element")},
    {"search", (PyCFunction)RBTreeSearch, METH_VARARGS, PyDoc_STR("Searches for an element")},
    {"first", (PyCFunction)RBTreeFirst, METH_NOARGS, PyDoc_STR("Gets an iterator on first element") },
    {NULL},
};

static PyMemberDef members[] = {
    {"black_depth", T_UINT, offsetof(RBTree, black_depth) + offsetof(RBTreeObject, tree),
     READONLY, PyDoc_STR("Black depth of the (red-black) tree")},
    {"count", T_UINT, offsetof(RBTree, count) + offsetof(RBTreeObject, tree),
     READONLY, PyDoc_STR("Number of elements in the tree")},
    {NULL}
};

Py_ssize_t RBTree_length(PyObject* self) {
    return ((RBTreeObject*)self)->tree.count;
}

static PyType_Slot RBtreeSlot[] = {
    {Py_tp_doc, PyDoc_STR("Internal RBTree")},
    {Py_tp_init, (initproc) RBTree_init},
    {Py_tp_methods, methods},
    {Py_tp_members, members},
    {Py_tp_iter, RBTree_iter},
    {Py_mp_length, RBTree_length},
    {Py_mp_subscript, RBTreeGetItem},
    {Py_mp_ass_subscript, RBTreeSetItem},
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
    base_exc = PyErr_NewException("pyrbtree.RBError", NULL, NULL);
    if (!base_exc) goto except;
    cr = PyModule_AddObjectRef(mod, "pyrbtree.RBError", typ);
    if (cr) goto except;
    for (int i = 0; i < sizeof(subexc) / sizeof(*subexc); i++) {
        char name[32];
        strcpy(name, "pyrbtree.");
        strcat(name, subexc[i]);
        typ = PyErr_NewException(name, base_exc, NULL);
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
