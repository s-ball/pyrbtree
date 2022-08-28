#  Copyright (c) 2022 SBA - MIT License

import unittest
import pyrbtree
import random
import sys


class TestRBTree(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        random.seed(0)

    def test_extension_present(self):
        self.assertTrue(hasattr(pyrbtree, '_rbtree'))

    def test_ordered_iteration(self):
        tree = pyrbtree._rbtree.RBTree()
        data = [2 * chr(ord('a') + i) for i in range(16)]
        random.shuffle(data)
        for elt in data:
            tree.insert(elt)
        self.assertEqual(sorted(data), list(tree))

    def test_multi_values(self):
        tree = pyrbtree._rbtree.RBTree()
        self.assertIsNone(tree.insert('aa'))
        for i in range(8):
            self.assertEqual('aa', tree.insert('aa'))
        self.assertEqual(1, tree.count)
        self.assertEqual(('aa',), tuple(tree))

    def test_insert_error(self):
        tree = pyrbtree._rbtree.RBTree()
        data = [2 * chr(ord('a') + i) for i in range(8)]
        for elt in data:
            tree.insert(elt)
        with self.assertRaises(pyrbtree.InsertError) as ctx:
            tree.insert(1)
        self.assertIsInstance(ctx.exception.__cause__, TypeError)

    def test_search_error(self):
        tree = pyrbtree._rbtree.RBTree()
        data = [2 * chr(ord('a') + i) for i in range(8)]
        for elt in data:
            tree.insert(elt)
        with self.assertRaises(pyrbtree.SearchError) as ctx:
            tree.search(1)
        self.assertIsInstance(ctx.exception.__cause__, TypeError)

    def test_remove(self):
        tree = pyrbtree._rbtree.RBTree()
        data = [2 * chr(ord('a') + i) for i in range(8)]
        for elt in data:
            tree.insert(elt)
        count = len(data)
        random.shuffle(data)
        for elt in data:
            self.assertEqual(elt, tree.remove(elt))
            count -= 1
            self.assertEqual(count, tree.count)
            self.assertIsNone(tree.remove(elt))
            self.assertEqual(count, tree.count)
        self.assertEqual(0, tree.black_depth)

    def test_find(self):
        tree = pyrbtree._rbtree.RBTree()
        data = [2 * chr(ord('a') + i) for i in range(8)]
        for elt in data:
            tree.insert(elt)
        self.assertIsNone(tree.find('A'))
        self.assertIsNone(tree.find(1))
        random.shuffle(data)
        for elt in data:
            self.assertEqual(elt, tree.find(elt))

    def test_refcount(self):
        tree = pyrbtree._rbtree.RBTree()
        a, b = 'aa', 'bb'
        ra, rb = sys.getrefcount(a), sys.getrefcount(b)
        tree.insert(a)
        self.assertEqual(ra+1, sys.getrefcount(a))
        self.assertEqual(a, tree.remove(a))
        self.assertEqual(ra, sys.getrefcount(a))
        tree.insert(b)
        del tree
        self.assertEqual(rb, sys.getrefcount(b))

    def test_clone(self):
        tree = pyrbtree._rbtree.RBTree()
        a, b = 'aa', 'bb'
        ra, rb = sys.getrefcount(a), sys.getrefcount(b)
        tree.insert(a)
        tree.insert(b)
        t2 = tree.clone()
        self.assertEqual(ra+2, sys.getrefcount(a))
        self.assertEqual(rb+2, sys.getrefcount(b))
        tree.remove('aa')
        self.assertEqual(ra+1, sys.getrefcount(a))
        t2.remove('aa')
        self.assertEqual(ra, sys.getrefcount(a))


if __name__ == '__main__':
    unittest.main()
