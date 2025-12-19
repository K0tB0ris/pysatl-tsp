from __future__ import annotations

from collections.abc import Iterator
from typing import Any, cast

import cffi

from pysatl_tsp._c.lib import (
    tsp_free_handler,
    tsp_free_midpoint_data,
    tsp_init_handler,
    tsp_midpoint_data_init,
    tsp_next_chain,
    tsp_op_MIDPOINT,
)
from pysatl_tsp.core import Handler
from pysatl_tsp.core.processor.inductive.moving_window_handler import MovingWindowHandler

ffi = cffi.FFI()


class MidpointHandler(MovingWindowHandler[float | None, float | None]):
    """Midpoint price handler.

    Calculates the average of highest and lowest values over the period.
    This handler is useful for identifying the central price level within a range,
    providing a simple measure of the balance between high and low extremes.

    Inherits parameters from MovingWindowHandler:
    :param length: The period for the calculation, defaults to 10
    :param source: Input data source, defaults to None

    Example:
        ```python
        # Create a data source with numeric values
        data_source = SimpleDataProvider([1.0, 2.0, 3.0, 5.0, 4.0, 3.0, 2.0, 1.0])

        # Create a midpoint handler with length of 4
        midpoint_handler = MidpointHandler(length=4)
        midpoint_handler.set_source(data_source)

        # Process the data
        for value in midpoint_handler:
            print(value)

        # Output:
        # None
        # None
        # None
        # 3.0  (midpoint of [1.0, 2.0, 3.0, 5.0] = (1.0 + 5.0) / 2 = 3.0)
        # 3.5  (midpoint of [2.0, 3.0, 5.0, 4.0] = (2.0 + 5.0) / 2 = 3.5)
        # 4.0  (midpoint of [3.0, 5.0, 4.0, 3.0] = (3.0 + 5.0) / 2 = 4.0)
        # 3.5  (midpoint of [5.0, 4.0, 3.0, 2.0] = (2.0 + 5.0) / 2 = 3.5)
        # 3.0  (midpoint of [4.0, 3.0, 2.0, 1.0] = (1.0 + 4.0) / 2 = 2.5)
        ```
    """

    def _compute_result(self, state: dict[str, Any]) -> float | None:
        """Calculate midpoint as (highest + lowest) / 2.

        Computes the average of the highest and lowest values in the current
        window. Returns None if there aren't enough values to fill the window
        or if any value in the window is None.

        :param state: Current state dictionary containing window values
        :return: Midpoint value or None if conditions aren't met
        """
        values = state["values"]

        if len(values) < self.length:
            return None

        highest = float("-inf")
        lowest = float("inf")
        for v in values:
            if v is None:
                return None
            highest = max(highest, v)
            lowest = min(lowest, v)

        return (highest + lowest) / 2


class CMidpointHandler(Handler[float | None, float | None]):
    def __init__(
        self,
        length: int = 10,
        source: Handler[Any, float | None] | None = None,
    ):
        super().__init__(source)
        self.length = length if length and length > 0 else 10
        if source is not None:
            if hasattr(source, "handler"):
                self.handler = tsp_init_handler(
                    ffi.cast("void *", tsp_midpoint_data_init(self.length)), source.handler, tsp_op_MIDPOINT, ffi.NULL
                )
            else:
                self.handler = tsp_init_handler(
                    ffi.cast("void *", tsp_midpoint_data_init(self.length)), ffi.NULL, tsp_op_MIDPOINT, ffi.NULL
                )
        else:
            self.handler = tsp_init_handler(
                ffi.cast("void *", tsp_midpoint_data_init(self.length)), ffi.NULL, tsp_op_MIDPOINT, ffi.NULL
            )

    def __iter__(self) -> Iterator[float | None]:
        if self.source is None:
            raise ValueError("Source is not set")
        self.src_itr = iter(self.source)
        self.handler.py_iter = ffi.cast("void*", id(self.src_itr))
        return self

    def __next__(self) -> float | None:
        res = tsp_next_chain(self.handler, 64)
        if res != ffi.NULL:
            if cast(float, res[0]) == cast(float, res[0]):
                return cast(float, res[0])
            else:
                return None
        else:
            raise StopIteration

    def __del__(self) -> None:
        tsp_free_midpoint_data(self.handler.data)
        tsp_free_handler(self.handler)
