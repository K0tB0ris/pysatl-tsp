class EvenNumbers:
    def __init__(self, max_numb):
        self.arr = [float(i) for i in range(0, max_numb + 1, 2)]
        self.current = 0

    def __iter__(self):
        """
        Returns the iterator object itself.
        This method is called when iter() is used on an instance of this class.
        """
        # self.current = 0
        return self

    def changecurrent(self, numb):
        self.current = numb

    def __next__(self):
        """
        Returns the next item in the sequence.
        This method is called by the next() function or implicitly by a for loop.
        """

        if self.current < len(self.arr):
            self.current += 1  # Increment by 2 to get the next even number
            return self.arr[self.current - 1]
        else:
            raise StopIteration  # Signal the end of iteration
