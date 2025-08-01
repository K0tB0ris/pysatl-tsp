from collections.abc import Iterator
from typing import Any, cast

import cffi

from pysatl_tsp._c.lib import (
    tsp_free_handler,
    tsp_free_queue,
    tsp_init_handler,
    tsp_next_chain,
    tsp_op_MA,
    tsp_queue_init,
)
from pysatl_tsp.core import Handler
from pysatl_tsp.core.processor.inductive.moving_window_handler import MovingWindowHandler

ffi = cffi.FFI()


class SMAHandler(MovingWindowHandler[float | None, float | None]):
    """Simple Moving Average (SMA) Handler.

    The Simple Moving Average is the classic moving average that is the equally
    weighted average over n periods. It's one of the most common technical analysis
    tools that smooths price data by calculating the arithmetic mean of a given set
    of values over the specified period.

    This handler properly handles None values by ignoring them in the calculation,
    and allows specifying a minimum number of valid observations required before
    producing a result.

    :param length: The period for the SMA calculation, defaults to 10
    :param min_periods: Minimum number of observations required to have a value, defaults to length
    :param source: Input data source, defaults to None

    Example:
        ```python
        # Create a data source with numeric values and some None values
        data_source = SimpleDataProvider([1.0, 2.0, 3.0, None, 5.0, 6.0, 7.0, 8.0])

        # Create an SMA handler with length of 4 and minimum periods of 3
        sma_handler = SMAHandler(length=4, min_periods=3)
        sma_handler.set_source(data_source)

        # Process the data
        for value in sma_handler:
            print(value)

        # Output:
        # None
        # None
        # 2.0   # (1.0 + 2.0 + 3.0) / 3 (only 3 values, but min_periods=3)
        # 3.33  # (1.0 + 2.0 + 3.0 + 5.0) / 3 (ignoring None)
        # 3.33  # (2.0 + 3.0 + 5.0) / 3 (window moves, still ignoring None)
        # 4.67  # (3.0 + 5.0 + 6.0) / 3
        # 6.0   # (5.0 + 6.0 + 7.0) / 3
        # 6.5   # (5.0 + 6.0 + 7.0 + 8.0) / 4 (full window with all valid values)
        ```
    """

    def __init__(
        self, length: int = 10, min_periods: int | None = None, source: Handler[Any, float | None] | None = None
    ):
        """Initialize SMA handler with specified parameters.

        :param length: The period for the SMA calculation, defaults to 10
        :param min_periods: Minimum number of non-None observations required to calculate a result,
                           defaults to length if None
        :param source: Input data source, defaults to None
        """
        super().__init__(length=length, source=source)
        self.min_periods = min_periods if min_periods is not None else length

    def _compute_result(self, state: dict[str, Any]) -> float | None:
        """Calculate simple moving average based on current values in the window.

        This method:
        1. Filters out None values from the current window
        2. Checks if the number of valid values meets or exceeds min_periods
        3. Calculates the arithmetic mean of valid values

        The SMA is calculated as the sum of valid values divided by the count of valid values,
        which means None values are completely ignored rather than treated as zeros.

        :param state: Current state containing the values in the moving window
        :return: Simple moving average of valid values or None if there aren't enough valid values
        """
        values = state["values"]

        # Filter out None values
        valid_values: list[float] = [v for v in values if v is not None]

        # Check if we have enough valid values
        if len(valid_values) < self.min_periods:
            return None

        # Calculate simple average
        return sum(valid_values) / len(valid_values)


class MAHandler(MovingWindowHandler[float | None, float | None]):
    """Simple Moving Average (SMA) Handler.

    The Simple Moving Average is the classic moving average that is the equally
    weighted average over n periods. It's one of the most common technical analysis
    tools that smooths price data by calculating the arithmetic mean of a given set
    of values over the specified period.

    This handler properly handles None values by ignoring them in the calculation,
    and allows specifying a minimum number of valid observations required before
    producing a result.

    :param length: The period for the SMA calculation, defaults to 10
    :param min_periods: Minimum number of observations required to have a value, defaults to length
    :param source: Input data source, defaults to None

    Example:
        ```python
        # Create a data source with numeric values and some None values
        data_source = SimpleDataProvider([1.0, 2.0, 3.0, None, 5.0, 6.0, 7.0, 8.0])

        # Create an SMA handler with length of 4 and minimum periods of 3
        sma_handler = SMAHandler(length=4, min_periods=3)
        sma_handler.set_source(data_source)

        # Process the data
        for value in sma_handler:
            print(value)

        # Output:
        # None
        # None
        # 2.0   # (1.0 + 2.0 + 3.0) / 3 (only 3 values, but min_periods=3)
        # 3.33  # (1.0 + 2.0 + 3.0 + 5.0) / 3 (ignoring None)
        # 3.33  # (2.0 + 3.0 + 5.0) / 3 (window moves, still ignoring None)
        # 4.67  # (3.0 + 5.0 + 6.0) / 3
        # 6.0   # (5.0 + 6.0 + 7.0) / 3
        # 6.5   # (5.0 + 6.0 + 7.0 + 8.0) / 4 (full window with all valid values)
        ```
    """

    def __init__(self, length: int = 10, source: Handler[Any, float | None] | None = None):
        super().__init__(length=length, source=source)

    def _compute_result(self, state: dict[str, Any]) -> float | None:
        values = state["values"]

        # Calculate simple average
        return float(sum(values) / len(values))


class CMAHandler(Handler[float, float]):
    def __init__(self, length: int = 10, source: Handler[Any, float] | None = None):
        super().__init__(source=source)
        self.length = length if length and length > 0 else 10
        if source is not None:
            if hasattr(source, "handler"):
                self.handler = tsp_init_handler(
                    ffi.cast("void *", tsp_queue_init(self.length)), source.handler, tsp_op_MA, ffi.NULL
                )
            else:
                self.handler = tsp_init_handler(
                    ffi.cast("void *", tsp_queue_init(self.length)), ffi.NULL, tsp_op_MA, ffi.NULL
                )
        else:
            self.handler = tsp_init_handler(
                ffi.cast("void *", tsp_queue_init(self.length)), ffi.NULL, tsp_op_MA, ffi.NULL
            )

    def __iter__(self) -> Iterator[float]:
        if self.source is None:
            raise ValueError("Source is not set")
        self.src_itr = iter(self.source)
        self.handler.py_iter = ffi.cast("void*", id(self.src_itr))
        return self

    def __next__(self) -> float:
        res = tsp_next_chain(self.handler, 64)
        if res != ffi.NULL:
            return cast(float, res[0])
        else:
            raise StopIteration

    def __del__(self) -> None:
        tsp_free_queue(self.handler.data)
        tsp_free_handler(self.handler)
