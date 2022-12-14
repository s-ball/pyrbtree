#  Copyright (c) 2022 SBA - MIT License

from .version import __version__
from . import _rbtree
from .rbtree import TreeSet, TreeMap


InsertError = _rbtree.InsertError
SearchError = _rbtree.SearchError
