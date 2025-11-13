from collections.abc import Iterator
from typing import Any, cast

import cffi

from pysatl_tsp._c.lib import (
    tsp_free_fwma_data,
    tsp_free_handler,
    tsp_fwma_data_init,
    tsp_init_handler,
    tsp_next_chain,
    tsp_op_FWMA,
)
from pysatl_tsp.core import Handler
from pysatl_tsp.core.processor.inductive.weighted_moving_average_handler import WeightedMovingAverageHandler

ffi = cffi.FFI()


class FWMAHandler(WeightedMovingAverageHandler):
    """Fibonacci Weighted Moving Average (FWMA) handler.

    Calculates a moving average using Fibonacci sequence numbers as weights.
    The Fibonacci sequence (1, 1, 2, 3, 5, 8, 13, ...) provides a natural
    weighting scheme where each number is the sum of the two preceding ones.

    By default, higher weights are assigned to more recent values (when asc=False),
    making this moving average more responsive to recent changes in the data.

    Inherits general behavior from WeightedMovingAverageHandler, only changing
    the weight calculation method.

    Example:
        ```python
        # Create a data source with numeric values
        data_source = SimpleDataProvider([1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0])

        # Create a FWMA handler with length of 5
        fwma_handler = FWMAHandler(length=5)
        fwma_handler.set_source(data_source)

        # Process the data
        for value in fwma_handler:
            print(value)

        # First 4 values will be None (not enough data points)
        # Subsequent values will be weighted averages using Fibonacci weights
        # For length=5, weights would be [0.01, 0.01, 0.02, 0.03, 0.05] (normalized)
        # or [0.05, 0.03, 0.02, 0.01, 0.01] when asc=False (default)
        ```
    """

    def _calculate_weights(self, length: int, asc: bool) -> list[float]:
        """Calculate Fibonacci weights for FWMA.

        Generates weights based on the Fibonacci sequence and normalizes them
        to sum to 1.0. The sequence order can be reversed based on the asc parameter.

        :param length: The number of weights to generate
        :param asc: Whether weights should be in ascending order
        :return: A list of normalized weights summing to 1.0
        """
        sequence = self._fibonacci_sequence(length)

        if not asc:
            sequence = sequence[::-1]

        # Normalize the weights to sum to 1.0
        total = sum(sequence)
        return [x / total for x in sequence]

    def _fibonacci_sequence(self, n: int) -> list[float]:
        """Generate the Fibonacci sequence of specified length.

        Creates a list containing the first n numbers in the Fibonacci sequence.
        The sequence starts with 1, 1 and each subsequent number is the sum of
        the two preceding ones.

        :param n: The length of the sequence to generate
        :return: A list containing the Fibonacci sequence
        """
        if n <= 0:
            return []

        if n == 1:
            return [1.0]

        sequence = [1.0, 1.0]
        for i in range(2, n):
            sequence.append(sequence[i - 1] + sequence[i - 2])

        return sequence


class CFWMAHandler(Handler[float | None, float | None]):
    def __init__(
        self,
        length: int = 10,
        asc: bool = False,
        source: Handler[Any, float | None] | None = None,
    ):
        super().__init__(source)
        self.length = length if length and length > 0 else 10
        if asc:
            self.asc = 1
        else:
            self.asc = 0

        if source is not None:
            if hasattr(source, "handler"):
                self.handler = tsp_init_handler(
                    ffi.cast("void *", tsp_fwma_data_init(self.length, self.asc)),
                    source.handler,
                    tsp_op_FWMA,
                    ffi.NULL,
                )
            else:
                self.handler = tsp_init_handler(
                    ffi.cast("void *", tsp_fwma_data_init(self.length, self.asc)),
                    ffi.NULL,
                    tsp_op_FWMA,
                    ffi.NULL,
                )
        else:
            self.handler = tsp_init_handler(
                ffi.cast("void *", tsp_fwma_data_init(self.length, self.asc)),
                ffi.NULL,
                tsp_op_FWMA,
                ffi.NULL,
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
            if cast(float, res[0]) != float("inf"):
                return cast(float, res[0])
            else:
                return None
        else:
            raise StopIteration

    def __del__(self) -> None:
        tsp_free_fwma_data(self.handler.data)
        tsp_free_handler(self.handler)
