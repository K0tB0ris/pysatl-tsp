# This program describes the C library builder.
import pathlib

from cffi import FFI

ffibuilder = FFI()

# Preparing to get paths to C files
project_name = "pysatl_tsp"
c_sources = "c"
current_dir = pathlib.Path(__file__).parent
project_dir = current_dir / project_name
c_def = """"""
headers = ""
src = []

# Read and get C headers
for item in project_dir.rglob("*.h"):
    with open(item) as f:
        c_def += f.read()
        headers += f'#include "{item.relative_to(project_dir)}"\n'

print("CDEF \n")
print(c_def)

print("\n HEADERS \n")
print(headers)

# Get list of path to all *.c files
for item in (project_dir / c_sources).rglob("*.c"):
    src.append(str(item.relative_to(current_dir)))

# Set options to compile C library
ffibuilder.cdef(c_def)
ffibuilder.set_source(
    f"{project_name}._c",
    headers,
    sources=src,
    extra_compile_args=["-fno-omit-frame-pointer", "-Wall", "-Wextra", "-O2"],
)

# Compile C library
if __name__ == "__main__":
    ffibuilder.compile(verbose=True)
