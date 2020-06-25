"""
Circular buffer implementation in python
"""


class Circular_Buffer(object):
    """
    Implements a circular buffer using a collection
    """

    def __init__(self, N, collInitFunc):
        """
        N: int capacity of buffer
        collInitFunc: a function that returns
            the desired collection when called with a length argument
            Examples:
                lambda n: [0]*n
                lambda n: str('0')*n
                lambda n: bytearray(n)
        """
        self.capacity = N
        self.collInitFunc = collInitFunc
        self.data = self.collInitFunc(N)
        self.iStart = 0
        self.iStop = 0
        self.size = 0

    def push_back(self, b):
        """
        Add elements to the end of the circular buffer
        Does so by overwriting earlier contents if necessary

        b: collection of elements

        """
        for x in b:
            self.data[self.iStop] = x
            self.iStop = (self.iStop + 1) % self.capacity
            if self.size == self.capacity:
                self.iStart = (self.iStart + 1) % self.capacity
            else:
                self.size += 1

    def push_front(self, b):
        """
        Add elements to the start of the circular buffer

        Does so by overwriting later contents if necessary

        b: collection of elements
        """
        for x in reversed(b):
            self.iStart = (self.iStart - 1) % self.capacity
            self.data[self.iStart] = x
            if self.size == self.capacity:
                self.iStop = (self.iStop - 1) % self.capacity
            else:
                self.size += 1

    def pop_front(self, N):
        """
        Pop the first N elements off of start of the circular buffer and return them
        """
        if N > self.capacity:
            raise ValueError(
                "N is greater than capacity of buffer: ", N, " > ", self.capacity
            )
        N = min(N, self.size)
        result = self.collInitFunc(N)
        for i in range(min(N, self.size)):
            result[i] = self.data[self.iStart]
            self.iStart = (self.iStart + 1) % self.capacity
            self.size -= 1
        return result

    def pop_back(self, N):
        """
        Pop the last N elements off of the end of the circular buffer and return them
        """
        if N > self.capacity:
            raise ValueError(
                "N is greater than capacity of buffer: ", N, " > ", self.capacity
            )
        N = min(N, self.size)
        result = self.collInitFunc(N)
        for i in range(N):
            j = (self.iStop - N + i) % self.capacity
            result[i] = self.data[j]
        self.iStop = (self.iStop - N) % self.capacity
        self.size -= N
        return result

    def removeFrontTo(self, val, inclusive=False):
        """
        Remove front elements up to given val

        if inclusive, then remove the given val, otherwise all before the given val

        returns None
        """
        while True:
            if self.isEmpty():
                return
            elif self.data[self.iStart] == val:
                if inclusive:
                    self.iStart = (self.iStart + 1) % self.capacity
                    self.size -= 1
                return
            else:
                self.iStart = (self.iStart + 1) % self.capacity
                self.size -= 1

    def removeBackTo(self, val, inclusive=False):
        """
        Remove back elements to given val

        if inclusive, then remove the given val, otherwise all after the given val

        returns None
        """
        while True:
            if self.isEmpty():
                return
            elif self.data[self.iStop - 1] == val:
                if inclusive:
                    self.iStop = (self.iStop - 1) % self.capacity
                    self.size -= 1
                return
            else:
                self.iStop = (self.iStop - 1) % self.capacity
                self.size -= 1

    def count(self, x):
        """
        Returns number of elements equal to x in buffer
        """
        result = 0
        for i in range(self.size):
            j = (self.iStart + i) % self.capacity
            if self.data[j] == x:
                result += 1
        return result

    def findFirst(self, x):
        """
        Returns the index (from iStart) of the first occurance of x

        Returns None if no x found
        """
        for i in range(self.size):
            j = (self.iStart + i) % self.capacity
            if self.data[j] == x:
                return i
        return None

    def __len__(self):
        return self.size

    def isFull(self):
        return len(self) == self.capacity

    def isEmpty(self):
        return len(self) == 0

    def __str__(self):
        return "{}: capacity: {} size: {} iStart: {} iStop: {}\n    {}".format(
            self.__class__.__name__,
            self.capacity,
            self.size,
            self.iStart,
            self.iStop,
            self.data,
        )


class Circular_Buffer_Bytes(Circular_Buffer):
    """
    Implements a circular buffer as a bytearray object
    """

    def __init__(self, N):
        super().__init__(N, lambda n: bytearray(n))


if __name__ == "__main__":
    pass
