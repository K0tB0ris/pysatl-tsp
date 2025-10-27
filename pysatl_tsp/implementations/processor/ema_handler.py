from collections.abc import Iterator
from typing import Any, cast

import cffi

from pysatl_tsp._c.lib import (
    tsp_ema_data_init,
    tsp_free_ema_data,
    tsp_free_handler,
    tsp_init_handler,
    tsp_next_chain,
    tsp_op_EMA,
)
from pysatl_tsp.core import Handler
from pysatl_tsp.core.processor import InductiveHandler
from pysatl_tsp.core.scrubber import ScrubberWindow

ffi = cffi.FFI()


class EMAHandler(InductiveHandler[float | None, float | None]):
    """Exponential Moving Average (EMA) handler.

    Calculates EMA values for a sequence of input values, matching the functionality
    of pandas_ta.EMA implementation.

    :param length: The period for EMA calculation, defaults to 10
    :param adjust: Whether to use adjusted weights in calculation, defaults to False
    :param sma: Whether to use SMA for initial value, defaults to True
    :param alpha: Custom smoothing factor, defaults to 2/(length+1) if None
    :param source: Input data source, defaults to None

    Example:
        ```python
        # Create a data source with numeric values
        data_source = SimpleDataProvider([1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0])

        # Create an EMA handler with length of 5
        ema_handler = EMAHandler(length=5)
        ema_handler.set_source(data_source)

        # Process the data
        for value in ema_handler:
            print(value)

        # The first 4 values will be None since we're using SMA initialization
        # The 5th value will be the SMA of the first 5 values
        # Subsequent values will be EMA values based on the formula
        ```
    """

    def __init__(
        self,
        length: int = 10,
        adjust: bool = False,
        sma: bool = True,
        alpha: float | None = None,
        source: Handler[Any, float | None] | None = None,
    ):
        """Initialize EMA handler with specified parameters.

        :param length: The period for EMA calculation, defaults to 10
        :param adjust: Whether to use adjusted weights in calculation, defaults to False
        :param sma: Whether to use SMA for initial value, defaults to True
        :param alpha: Custom smoothing factor, defaults to 2/(length+1) if None
        :param source: Input data source, defaults to None
        """
        super().__init__(source)
        self.length = length
        self.adjust = adjust
        self.sma = sma
        if alpha is None:
            self.alpha = 2 / (self.length + 1)
        else:
            self.alpha = alpha

    def _initialize_state(self) -> dict[str, Any]:
        """Initialize state for EMA calculation.

        Creates the initial state dictionary with a window to collect values,
        variables to track the EMA calculation, and a position counter.

        :return: Dictionary containing initial state variables
        """
        return {
            "window": ScrubberWindow(),
            "ema_numerator": None if self.sma else 0,
            "ema_denominator": None if self.sma else 0,
            "position": 0,
        }

    def _update_state(self, state: dict[str, Any], value: float | None) -> dict[str, Any]:
        """Update state with a new value.

        Implements the same logic as the original pandas_ta.EMA function:
        - If sma=True, initialize EMA with the SMA of first 'length' values
        - If sma=False, initialize EMA with the first value
        - Then apply the standard EMA formula for subsequent values

        When adjust=True, uses an adjusted weighting method that gives
        more weight to recent observations.

        :param state: Current state dictionary
        :param value: New value to incorporate into the EMA calculation
        :return: Updated state dictionary
        """
        state["position"] += 1
        if value is not None:
            state["window"].append(value)

        if self.sma and state["position"] != self.length:
            return state

        if self.sma:
            if len(state["window"]):
                sma_value = sum(state["window"].values) / len(state["window"])
                state["window"].clear()
                state["ema_numerator"] = sma_value
                state["ema_denominator"] = 1
            self.sma = False
        elif self.adjust and value is not None:
            state["ema_numerator"] = (1 - self.alpha) * state["ema_numerator"] + value
            state["ema_denominator"] = (1 - self.alpha) * state["ema_denominator"] + 1
        elif not self.adjust and value is not None:
            if state["ema_denominator"]:
                state["ema_numerator"] = (1 - self.alpha) * state["ema_numerator"] + self.alpha * value
            else:
                state["ema_numerator"] = value
            state["ema_denominator"] = 1

        return state

    def _compute_result(self, state: dict[str, Any]) -> float | None:
        """Return the current EMA value or None if not yet initialized.

        Calculates the final EMA value by dividing the numerator by the denominator
        if the denominator exists (indicating that EMA is initialized).

        :param state: Current state of the handler
        :return: Current EMA value or None if not yet calculated
        """
        if state["ema_denominator"]:
            res: float = state["ema_numerator"] / state["ema_denominator"]
            return res
        return None


class CEMAHandler(Handler[float | None, float | None]):
    def __init__(
        self,
        length: int = 10,
        adjust: bool = False,
        sma: bool = True,
        alpha: float | None = None,
        source: Handler[Any, float | None] | None = None,
    ):
        super().__init__(source)
        self.length = length if length and length > 0 else 10
        if adjust:
            self.adjust = 1
        else:
            self.adjust = 0
        if sma:
            self.sma = 1
        else:
            self.sma = 0
        self.alpha = alpha
        if self.alpha is None:
            self.c_alpha = ffi.NULL
        else:
            self.c_alpha = ffi.new("double *", float(self.alpha))  # type: ignore
        if source is not None:
            if hasattr(source, "handler"):
                self.handler = tsp_init_handler(
                    ffi.cast("void *", tsp_ema_data_init(self.length, self.sma, self.c_alpha, self.adjust)),
                    source.handler,
                    tsp_op_EMA,
                    ffi.NULL,
                )
            else:
                self.handler = tsp_init_handler(
                    ffi.cast("void *", tsp_ema_data_init(self.length, self.sma, self.c_alpha, self.adjust)),
                    ffi.NULL,
                    tsp_op_EMA,
                    ffi.NULL,
                )
        else:
            self.handler = tsp_init_handler(
                ffi.cast("void *", tsp_ema_data_init(self.length, self.sma, self.c_alpha, self.adjust)),
                ffi.NULL,
                tsp_op_EMA,
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
        tsp_free_ema_data(self.handler.data)
        tsp_free_handler(self.handler)
