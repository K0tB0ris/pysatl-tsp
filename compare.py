import time

import cffi

from pysatl_tsp.core.data_providers import SimpleDataProvider
from pysatl_tsp.implementations.processor.sma_handler import CSMAHandler, SMAHandler

ffi = cffi.FFI()

# Create a data source
data = [float(i) for i in range(100000)]
for i in range(5):
    data = data + data
provider = SimpleDataProvider(data)

pipe1 = provider | SMAHandler()
pipe2 = provider | CSMAHandler()

start_time = time.monotonic()
for i in pipe1:
    pass
first_mark = time.monotonic()
for i in pipe2:
    pass
end_time = time.monotonic()

print(f"Work time with pure python objects: {round(first_mark - start_time, 5)}s")
print(f"Work time with c/python objects: {round(end_time - first_mark, 5)}s")
