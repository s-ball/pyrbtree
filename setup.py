#  Copyright (c) 2022 SBA - MIT License

from setuptools import setup, Extension
import os.path


kwargs = dict(
    # more metadata
    ext_modules=[
        Extension('pyrbtree._rbtree', [os.path.join('pyrbtree', '_rbtree', '_rbtree.c'),
                                         os.path.join('pyrbtree', 'CRBTree', 'rbtree', 'rbtree.c'),
                                         os.path.join('pyrbtree', 'CRBTree', 'rbtree', 'rbversion.c')],
                  include_dirs=[os.path.join('pyrbtree', 'CRBTree', 'rbtree')],
                  define_macros=[('EXPORT', '')]
                  )])

setup(**kwargs)