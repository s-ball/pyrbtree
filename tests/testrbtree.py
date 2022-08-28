#  Copyright (c) 2022 SBA - MIT License
import sys
import unittest
from pyrbtree import TreeSet, TreeMap


class TestTreeSet(unittest.TestCase):
    def setUp(self) -> None:
        self.iterable = 'abcdbcdacdba'
        self.ts = TreeSet(self.iterable)

    def test_compare_to_set(self):
        s = set(self.iterable)
        self.assertEqual(sorted(s), list(self.ts))

    def test_union(self):
        ts2 = TreeSet('abef')
        self.assertEqual(TreeSet('efabcd'), self.ts | ts2)

    def test_intersection(self):
        ts2 = TreeSet('abef')
        self.assertEqual(TreeSet('ab'), ts2 & self.ts)

    def test_repr(self):
        self.assertEqual("{'a', 'b', 'c', 'd'}", repr(self.ts))

    def test_copy(self):
        a = 'aa'
        b = 'bb'
        ra = sys.getrefcount(a)
        rb = sys.getrefcount(b)
        ts = TreeSet((a, b))
        self.assertEqual(ra + 1, sys.getrefcount(a))
        self.assertEqual(rb + 1, sys.getrefcount(b))
        t2 = ts.copy()
        self.assertEqual(ra + 2, sys.getrefcount(a))
        self.assertEqual(rb + 2, sys.getrefcount(b))
        ts.discard('aa')
        self.assertEqual(ra + 1, sys.getrefcount(a))
        t2.discard('aa')
        self.assertEqual(ra, sys.getrefcount(a))
        del ts
        self.assertEqual(rb + 1, sys.getrefcount(b))
        del t2
        self.assertEqual(rb, sys.getrefcount(b))


class TestTreeMap(unittest.TestCase):
    def setUp(self) -> None:
        self.iterable = (('a', 1), ('b', 2), ('c', 3), ('d', 4),
                         ('b', 5), ('c', 6), ('d', 7), ('a', 8),
                         ('c', 9), ('d', 10), ('b', 11), ('a', 12))
        self.tm = TreeMap(self.iterable)

    def test_compare_to_dict(self):
        d = dict(self.iterable)
        self.assertEqual(d, self.tm)
        self.assertEqual(sorted(d.items()), list(self.tm.items()))

    def test_repr(self):
        del self.tm['c']
        del self.tm['d']
        self.assertEqual("{'a': 12, 'b': 11}", repr(self.tm))


if __name__ == '__main__':
    unittest.main()
