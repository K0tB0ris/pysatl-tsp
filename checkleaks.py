import time
from collections.abc import Iterator
from typing import Any

import cffi

from pysatl_tsp._c.lib import tsp_free_handler, tsp_init_handler, tsp_next_chain, tsp_op_addFive
from pysatl_tsp.core import Handler
from pysatl_tsp.core.data_providers import SimpleDataProvider
from pysatl_tsp.implementations.processor.sma_handler import CMAHandler, MAHandler

ffi = cffi.FFI()


class CSumHandler(Handler[float, float]):
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
        res = tsp_next_chain(self.handler, 64)
        if res != ffi.NULL:
            return res[0]
        else:
            raise StopIteration

    def __del__(self):
        tsp_free_handler(self.handler)

# Create a data source
data = [float(i) for i in range(100)]
for i in range(5):
    data = data + data
provider = SimpleDataProvider(data)

pipe2 = provider | CSumHandler() | CMAHandler()

start_time = time.monotonic()
for i in pipe2:
    pass
end_time = time.monotonic()

print(f"Work time with c/python objects: {round(end_time - start_time, 5)}s")
