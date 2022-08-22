#  Copyright (c) 2022 SBA - MIT License

import unittest
import pyrbtree
import random
import sys


class TestRBTreeMap(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        random.seed(0)

    def test_ordered_iteration(self):
        tree = pyrbtree._rbtree.RBTree(True)
        data = [(2 * chr(ord('a') + i), 16 - i) for i in range(16)]
        random.shuffle(data)
        for elt in data:
            tree.insert(elt)
        self.assertEqual(sorted(data), list(tree))

    def test_multi_values(self):
        tree = pyrbtree._rbtree.RBTree(True)
        self.assertIsNone(tree.insert(('aa', 1)))
        for i in range(8):
            self.assertEqual('aa', tree.insert(('aa', i))[0])
        self.assertEqual(1, tree.count)
        self.assertEqual((('aa', 7),), tuple(tree))

    def test_insert_error(self):
        tree = pyrbtree._rbtree.RBTree(True)
        data = [(2 * chr(ord('a') + i), 8 - i) for i in range(8)]
        for elt in data:
            tree.insert(elt)
        with self.assertRaises(pyrbtree.InsertError) as ctx:
            tree.insert((1, 8))
        self.assertIsInstance(ctx.exception.__cause__, TypeError)

    def test_search_error(self):
        tree = pyrbtree._rbtree.RBTree(True)
        data = [(2 * chr(ord('a') + i), 8 - i) for i in range(8)]
        for elt in data:
            tree.insert(elt)
        with self.assertRaises(pyrbtree.SearchError) as ctx:
            tree.search((1, None))
        self.assertIsInstance(ctx.exception.__cause__, TypeError)

    def test_remove(self):
        tree = pyrbtree._rbtree.RBTree(True)
        data = [(2 * chr(ord('a') + i), 8 - i) for i in range(8)]
        for elt in data:
            tree.insert(elt)
        count = len(data)
        random.shuffle(data)
        for elt in data:
            self.assertEqual(elt, tree.remove((elt[0], None)))
            count -= 1
            self.assertEqual(count, tree.count)
            self.assertIsNone(tree.remove(elt))
            self.assertEqual(count, tree.count)
        self.assertEqual(0, tree.black_depth)

    def test_find(self):
        tree = pyrbtree._rbtree.RBTree(True)
        data = [(2 * chr(ord('a') + i), 8 - i) for i in range(8)]
        for elt in data:
            tree.insert(elt)
        self.assertIsNone(tree.find(('A', 1)))
        self.assertIsNone(tree.find((1, 1)))
        random.shuffle(data)
        for elt in data:
            self.assertEqual(elt, tree.find((elt[0], 0)))

    def test_refcount(self):
        tree = pyrbtree._rbtree.RBTree(True)
        a, b = ('aa', 1), ('bb', 2)
        ra, rb = sys.getrefcount(a), sys.getrefcount(b)
        tree.insert(a)
        self.assertEqual(ra+1, sys.getrefcount(a))
        self.assertEqual(a, tree.remove(('aa', None)))
        self.assertEqual(ra, sys.getrefcount(a))
        tree.insert(b)
        del tree
        self.assertEqual(rb, sys.getrefcount(b))

    def test_refcount_multiple(self):
        tree = pyrbtree._rbtree.RBTree(True)
        a, b, c, d = (2 * chr(i + ord('a')) for i in range(4))
        ra, rb = sys.getrefcount(a), sys.getrefcount(b)
        rc, rd = sys.getrefcount(c), sys.getrefcount(c)
        tree.insert((a, b))
        self.assertEqual(ra+1, sys.getrefcount(a))
        self.assertEqual(rb+1, sys.getrefcount(b))
        self.assertEqual((a, b), tree.insert((a, c)))
        self.assertEqual(ra+1, sys.getrefcount(a))
        self.assertEqual(rb, sys.getrefcount(b))
        self.assertEqual(rc+1, sys.getrefcount(c))
        self.assertEqual((a, c), tree.remove((a, d)))
        self.assertEqual(ra, sys.getrefcount(a))
        self.assertEqual(rb, sys.getrefcount(b))
        self.assertEqual(rc, sys.getrefcount(c))
        self.assertEqual(rd, sys.getrefcount(d))

    def test_subscript(self):
        tree = pyrbtree._rbtree.RBTree(True)
        k, v = 'aa', (1, )
        ra, rb = sys.getrefcount(k), sys.getrefcount(v)
        tree[k] = v
        self.assertEqual(ra + 1, sys.getrefcount(k))
        self.assertEqual(rb + 1, sys.getrefcount(v))
        self.assertIs(tree[k], v)
        tree[k] = None
        self.assertEqual(ra + 1, sys.getrefcount(k))
        self.assertEqual(rb, sys.getrefcount(v))
        del tree[k]
        self.assertEqual(ra, sys.getrefcount(k))
        self.assertEqual(rb, sys.getrefcount(v))


if __name__ == '__main__':
    unittest.main()
