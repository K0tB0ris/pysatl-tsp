from collections.abc import Iterator
from typing import Any
from pysatl_tsp.core import Handler
from pysatl_tsp.core.data_providers import SimpleDataProvider
import cffi
from pysatl_tsp._c.lib import *


ffi = cffi.FFI()


class CAddFiveHandler(Handler[float, float]):

    def __init__(self, source: Handler[Any, float] | None = None):
        super().__init__(source)
        if source is not None:
            self.handler = tsp_init_handler(ffi.NULL, source.handler, tsp_op_addFive, ffi.NULL)
        else:
            self.handler = tsp_init_handler(ffi.NULL, ffi.NULL, tsp_op_addFive, ffi.NULL)

    def __iter__(self) -> Iterator[float | None]:
        if self.source is None:
            raise ValueError("Source is not set")
        self.src_itr = iter(self.source)
        self.handler.py_iter = ffi.cast("void *", id(self.src_itr))
        return self

    def __next__(self):
        res = tsp_next_buffer(self.handler, 5)
        if res != ffi.NULL:
            return res[0]
        else:
            raise StopIteration

    def __del__(self):
        tsp_free_handler(self.handler)


class CMultFiveHandler(Handler[float, float]):

    def __init__(self, source: Handler[Any, float] | None = None):
        super().__init__(source)
        if source is not None:
            self.handler = tsp_init_handler(ffi.NULL, source.handler, tsp_op_multFive, ffi.NULL)
        else:
            self.handler = tsp_init_handler(ffi.NULL, ffi.NULL, tsp_op_multFive, ffi.NULL)

    def __iter__(self) -> Iterator[float | None]:
        if self.source is None:
            raise ValueError("Source is not set")
        self.src_itr = iter(self.source)
        self.handler.py_iter = ffi.cast("void*", id(self.src_itr))
        return self

    def __next__(self):
        res = tsp_next_buffer(self.handler, 5)
        if res != ffi.NULL:
            return res[0]
        else:
            raise StopIteration

    def __del__(self):

        tsp_free_handler(self.handler)



data = [1.2]
for i in range(1, 20):
    data.append(data[0] + i)
print(data)
provider = SimpleDataProvider(data)
for elem in provider:
    print(elem, end=" ")
print("\n-----------------------")

pipeline = (provider | CAddFiveHandler() | CAddFiveHandler() | CAddFiveHandler())
for elem in pipeline:
    print(elem)
print("--------------------------------------------------------\n")

pipeline1 = (provider | CAddFiveHandler() | CAddFiveHandler() | CMultFiveHandler())
for elem in pipeline1:
    print(elem, end=" ")
print("\n\n--------------------------------------------------------\n")

