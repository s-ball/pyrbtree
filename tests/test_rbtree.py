#  Copyright (c) 2022 SBA - MIT License

import unittest
import pyrbtree
import random


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


if __name__ == '__main__':
    unittest.main()
