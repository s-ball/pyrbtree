#  Copyright (c) 2022 SBA - MIT License

from . import _rbtree
from collections import abc
from typing import Iterable, Mapping, Union


class TreeSetIter:
    def __init__(self, it: _rbtree.RBIter):
        self._iter = it

    def __iter__(self):
        return self

    def __next__(self):
        return next(self._iter)


class TreeSet(abc.MutableSet):
    def __init__(self, it: Iterable = None):
        self._tree = _rbtree.RBTree()
        if it is not None:
            for elt in it:
                self._tree.insert(elt)

    def __iter__(self) -> TreeSetIter:
        return TreeSetIter(self._tree.first())

    def add(self, value) -> None:
        self._tree.insert(value)

    def discard(self, value) -> None:
        self._tree.remove(value)

    def __len__(self) -> int:
        return self._tree.count

    def __contains__(self, item) -> bool:
        return self._tree.find(item) is not None

    def __repr__(self):
        return '{' + ', '.join(repr(elt) for elt in self._tree) + '}'


class TreeMapIter:
    def __init__(self, it: _rbtree.RBIter):
        self._it = it

    def __iter__(self):
        return self

    def __next__(self):
        return next(self._it)[0]


class TreeMap(abc.MutableMapping):
    def __init__(self, src: Union[Mapping, Iterable[Iterable]] = None, **kwargs):
        self._tree = _rbtree.RBTree(True)
        if src is not None:
            try:
                src = src.items()
            except AttributeError:
                pass
            for elt in src:
                try:
                    if len(elt) != 2:
                        raise ValueError('An iterable of 2-iterable'
                                         ' is required')
                except TypeError as exc:
                    raise ValueError('An iterable of 2-iterable'
                                     ' is required') from exc
                self._tree.insert(elt)
        for elt in kwargs.items():
            self._tree.insert(elt)

    def __getitem__(self, key):
        return self._tree[key]

    def __setitem__(self, key, value):
        self._tree[key] = value

    def __delitem__(self, key):
        del self._tree[key]

    def __iter__(self):
        return TreeMapIter(self._tree.first())

    def __len__(self):
        return self._tree.count

    def __repr__(self):
        return '{' + ', '.join(': '.join(str(i) for i in elt)
                               for elt in self._tree) + '}'
