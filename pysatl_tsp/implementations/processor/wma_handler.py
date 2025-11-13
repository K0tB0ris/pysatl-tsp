from collections.abc import Iterator
from typing import Any, cast

import cffi
import numpy as np

from pysatl_tsp._c.lib import (
    tsp_free_handler,
    tsp_free_wma_data,
    tsp_init_handler,
    tsp_next_chain,
    tsp_op_WMA,
    tsp_wma_data_init,
)
from pysatl_tsp.core import Handler
from pysatl_tsp.core.processor.inductive.weighted_moving_average_handler import WeightedMovingAverageHandler

ffi = cffi.FFI()


class WMAHandler(WeightedMovingAverageHandler):
    """Weighted Moving Average (WMA) handler.

    Calculates a moving average where each data point is multiplied by a weight
    before being included in the average. The weights increase or decrease linearly,
    giving more importance to recent data points by default.

    This implementation matches the functionality of pandas_ta.wma, using a linear
    weighting scheme where the weight of each value is proportional to its position
    in the window.

    Inherits parameters from WeightedMovingAverageHandler:
    :param length: The period for the calculation, defaults to 10
    :param asc: Whether weights should be applied in ascending order, defaults to False
                When False, most recent values get higher weights
    :param source: Input data source, defaults to None

    Example:
        ```python
        # Create a data source with numeric values
        data_source = SimpleDataProvider([1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0])

        # Create a WMA handler with length of 4
        wma_handler = WMAHandler(length=4)
        wma_handler.set_source(data_source)

        # Process the data
        for value in wma_handler:
            print(value)

        # First 3 values will be None (not enough data points)
        # For length=4, weights would be [0.1, 0.2, 0.3, 0.4] (with asc=False)
        # So the 4th value would be: (1.0*0.1 + 2.0*0.2 + 3.0*0.3 + 4.0*0.4) = 3.0
        # Similarly, the 5th value: (2.0*0.1 + 3.0*0.2 + 4.0*0.3 + 5.0*0.4) = 4.0
        ```
    """

    def _calculate_weights(self, length: int, asc: bool) -> list[float]:
        """Calculate linear weights for WMA.

        Generates a sequence of linearly increasing weights that sum to 1.0.
        The weights increase from 1 to length, and are then normalized by dividing
        by their sum. The order can be reversed based on the asc parameter.

        :param length: The number of weights to generate
        :param asc: Whether weights should be in ascending order
                   When False (default), higher weights are assigned to more recent values
        :return: A list of normalized weights summing to 1.0
        """
        weights: list[float] = list(np.arange(1, length + 1))
        if not asc:
            weights = weights[::-1]

        # Calculate total weight (sum of weights)
        total_weight = np.sum(weights)

        # Return normalized weights
        return [float(w) / total_weight for w in weights]


class CWMAHandler(Handler[float | None, float | None]):
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
                    ffi.cast("void *", tsp_wma_data_init(self.length, self.asc)),
                    source.handler,
                    tsp_op_WMA,
                    ffi.NULL,
                )
            else:
                self.handler = tsp_init_handler(
                    ffi.cast("void *", tsp_wma_data_init(self.length, self.asc)),
                    ffi.NULL,
                    tsp_op_WMA,
                    ffi.NULL,
                )
        else:
            self.handler = tsp_init_handler(
                ffi.cast("void *", tsp_wma_data_init(self.length, self.asc)),
                ffi.NULL,
                tsp_op_WMA,
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
        tsp_free_wma_data(self.handler.data)
        tsp_free_handler(self.handler)
