# pyrbtree

## Description

This package provides an implementation of a tree
based set and mapping. For performance reasons, it
uses an underlying binary extension wrapping a C
library: CRBTree.

## Usage

The package declares 2 classes: `TreeSet`, which behaves
as a normal set, but maintains a natural order thanks to
the underlying Red-Black tree, and `TreeMap` which is the
same as a mapping.

At the time of this writing, most methods from `set` and
`dict` should work as expected, except for `copy` which 
is currently not implemented.

Example:

```
from pyrbtree import TreeSet

ts = TreeSet('acbfd')
ts.add('e')
print(ts)
```

should output:

    {'a', 'b', 'c', 'd', 'e', 'f'}

## Installation

### From PyPI

    pip install pyrbtree

### From GitHub

This is the recommended way if you want to contribute or simply tweak
`pyrbtree` to your own requirements. You can get a local copy by
downloading a zipfile but if you want to make changes, you should
 rather clone the repository to have access to all `git` goodies:

    git clone https://github.com/s-ball/pyrbtree.git --recurse-submodules

You can then install it in your main Python installation or in a venv with:

    pip install .

or on Windows with the launcher:

    py -m pip install .

#### Development

The GitHub repository contains an `unittest` test package. From the
main folder, tests can be launched with:

You must install from the repository in development mode:

    git clone https://github.com/s-ball/pyrbtree.git --recurse-submodules
    pip install -e .

Then you can run the tests with:

    python -m unittest discover

### Wheel format

The extension module only uses the so-called limited API. The binary wheel is expected to
be compatible with any Python version above 3.7 for the same architecture. Currently, it
is only provided for a Windows 64 bits platform.
## Contributions

Contributions are welcome, including issues on GitHub.
Problems are expected to be documented so that they can be reproduced. But
I only develop this on my free time, so I cannot guarantee quick answers...

## Disclaimer: alpha quality

The main classes are already well tested, but the `copy` method is
still to be implemented.

## License

This work is licenced under a MIT Licence. See [LICENSE.txt](https://github.com/s-ball/pyrbtree/blob/master/LICENSE.txt)
