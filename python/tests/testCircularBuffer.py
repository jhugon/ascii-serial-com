import unittest
import unittest.mock
from asciiserialcom.circularBuffer import Circular_Buffer
from asciiserialcom.circularBuffer import Circular_Buffer_Bytes


class TestCircularBufferList(unittest.TestCase):
    def setUp(self):
        self.buf = Circular_Buffer(10, lambda n: [0] * n)

    def test_push_back_until_full(self):
        buf = self.buf
        # print("only init:",buf)
        self.assertFalse(buf.isFull())
        self.assertTrue(buf.isEmpty())
        buf.push_back([0])
        # print("push_back([0])",buf)
        self.assertFalse(buf.isFull())
        self.assertFalse(buf.isEmpty())
        self.assertEqual(len(buf), 1)
        buf.push_back(range(1, 6))
        # print("push_back(range(1,6))",buf)
        self.assertFalse(buf.isFull())
        self.assertFalse(buf.isEmpty())
        self.assertEqual(len(buf), 6)
        buf.push_back(range(6, 10))
        # print("push_back(range(6,10))",buf)
        self.assertTrue(buf.isFull())
        self.assertFalse(buf.isEmpty())
        self.assertEqual(len(buf), 10)
        popped = buf.pop_back(len(buf))
        # print("pop_back(10)",buf)
        self.assertFalse(buf.isFull())
        self.assertTrue(buf.isEmpty())
        self.assertEqual(popped, list(range(10)))

    def test_push_back_past_full(self):
        buf = self.buf
        # print("only init:",buf)
        self.assertFalse(buf.isFull())
        self.assertTrue(buf.isEmpty())
        buf.push_back(range(11))
        # print("push_back([0])",buf)
        self.assertTrue(buf.isFull())
        self.assertFalse(buf.isEmpty())
        self.assertEqual(len(buf), 10)
        popped = buf.pop_back(len(buf))
        # print("pop_back(10)",buf)
        self.assertFalse(buf.isFull())
        self.assertTrue(buf.isEmpty())
        self.assertEqual(popped, list(range(1, 11)))

        buf.push_back(range(100))
        self.assertTrue(buf.isFull())
        self.assertFalse(buf.isEmpty())
        self.assertEqual(len(buf), 10)
        popped = buf.pop_back(len(buf))
        # print("pop_back(10)",buf)
        self.assertFalse(buf.isFull())
        self.assertTrue(buf.isEmpty())
        self.assertEqual(popped, list(range(90, 100)))

    def test_push_pop_back(self):
        buf = self.buf
        self.assertFalse(buf.isFull())
        self.assertTrue(buf.isEmpty())

        buf.push_back(range(5))
        self.assertFalse(buf.isFull())
        self.assertFalse(buf.isEmpty())
        self.assertEqual(len(buf), 5)

        popped = buf.pop_back(2)
        self.assertFalse(buf.isFull())
        self.assertFalse(buf.isEmpty())
        self.assertEqual(len(buf), 3)
        self.assertEqual(popped, list(range(3, 5)))

        buf.push_back([1111, 2222, 3333])
        self.assertFalse(buf.isFull())
        self.assertFalse(buf.isEmpty())
        self.assertEqual(len(buf), 6)

        popped = buf.pop_back(5)
        self.assertFalse(buf.isFull())
        self.assertFalse(buf.isEmpty())
        self.assertEqual(len(buf), 1)
        self.assertEqual(popped, [1, 2, 1111, 2222, 3333])

        buf.push_back(range(20))
        self.assertTrue(buf.isFull())
        self.assertFalse(buf.isEmpty())
        self.assertEqual(len(buf), 10)

        with self.assertRaises(ValueError):
            popped = buf.pop_back(11)

        popped = buf.pop_back(10)
        self.assertFalse(buf.isFull())
        self.assertTrue(buf.isEmpty())
        self.assertEqual(len(buf), 0)
        self.assertEqual(popped, list(range(10, 20)))

    def test_push_front_until_full(self):
        buf = self.buf
        self.assertFalse(buf.isFull())
        self.assertTrue(buf.isEmpty())
        buf.push_front([0])
        self.assertFalse(buf.isFull())
        self.assertFalse(buf.isEmpty())
        self.assertEqual(len(buf), 1)
        buf.push_front(range(1, 6))
        self.assertFalse(buf.isFull())
        self.assertFalse(buf.isEmpty())
        self.assertEqual(len(buf), 6)
        buf.push_front(range(6, 10))
        self.assertTrue(buf.isFull())
        self.assertFalse(buf.isEmpty())
        self.assertEqual(len(buf), 10)
        popped = buf.pop_back(len(buf))
        self.assertFalse(buf.isFull())
        self.assertTrue(buf.isEmpty())
        self.assertEqual(popped, [6, 7, 8, 9, 1, 2, 3, 4, 5, 0])

    def test_push_front_past_full(self):
        buf = self.buf
        self.assertFalse(buf.isFull())
        self.assertTrue(buf.isEmpty())
        buf.push_front(range(20))
        self.assertTrue(buf.isFull())
        self.assertFalse(buf.isEmpty())
        self.assertEqual(len(buf), 10)
        popped = buf.pop_back(len(buf))
        self.assertFalse(buf.isFull())
        self.assertTrue(buf.isEmpty())
        self.assertEqual(popped, list(range(10)))

    def test_push_pop_front(self):
        buf = self.buf
        self.assertFalse(buf.isFull())
        self.assertTrue(buf.isEmpty())

        buf.push_front(range(5))
        self.assertFalse(buf.isFull())
        self.assertFalse(buf.isEmpty())
        self.assertEqual(len(buf), 5)

        popped = buf.pop_front(2)
        self.assertFalse(buf.isFull())
        self.assertFalse(buf.isEmpty())
        self.assertEqual(len(buf), 3)
        self.assertEqual(popped, list(range(2)))

        buf.push_front([1111, 2222, 3333])
        self.assertFalse(buf.isFull())
        self.assertFalse(buf.isEmpty())
        self.assertEqual(len(buf), 6)

        popped = buf.pop_front(5)
        self.assertFalse(buf.isFull())
        self.assertFalse(buf.isEmpty())
        self.assertEqual(len(buf), 1)
        self.assertEqual(popped, [1111, 2222, 3333, 2, 3])

        buf.push_front(range(20))
        self.assertTrue(buf.isFull())
        self.assertFalse(buf.isEmpty())
        self.assertEqual(len(buf), 10)

        with self.assertRaises(ValueError):
            popped = buf.pop_front(11)

        popped = buf.pop_front(10)
        self.assertFalse(buf.isFull())
        self.assertTrue(buf.isEmpty())
        self.assertEqual(len(buf), 0)
        self.assertEqual(popped, list(range(10)))

    def test_push_pop_front_back(self):
        buf = self.buf
        self.assertFalse(buf.isFull())
        self.assertTrue(buf.isEmpty())

        buf.push_front(range(5))
        self.assertFalse(buf.isFull())
        self.assertFalse(buf.isEmpty())
        self.assertEqual(len(buf), 5)

        popped = buf.pop_front(2)
        self.assertFalse(buf.isFull())
        self.assertFalse(buf.isEmpty())
        self.assertEqual(len(buf), 3)
        self.assertEqual(popped, list(range(2)))

        buf.push_back([1111, 2222, 3333])
        self.assertFalse(buf.isFull())
        self.assertFalse(buf.isEmpty())
        self.assertEqual(len(buf), 6)

        popped = buf.pop_back(5)
        self.assertFalse(buf.isFull())
        self.assertFalse(buf.isEmpty())
        self.assertEqual(len(buf), 1)
        self.assertEqual(popped, [3, 4, 1111, 2222, 3333])


class TestCircularBufferBytes(unittest.TestCase):
    def setUp(self):
        self.buf = Circular_Buffer_Bytes(10)

    def test_push_pop(self):
        buf = self.buf
        self.assertFalse(buf.isFull())
        self.assertTrue(buf.isEmpty())

        buf.push_front(bytes(range(5)))
        self.assertFalse(buf.isFull())
        self.assertFalse(buf.isEmpty())
        self.assertEqual(len(buf), 5)

        popped = buf.pop_front(2)
        self.assertFalse(buf.isFull())
        self.assertFalse(buf.isEmpty())
        self.assertEqual(len(buf), 3)
        self.assertEqual(popped, bytes(range(2)))

        buf.push_back(bytes([111, 222, 255]))
        self.assertFalse(buf.isFull())
        self.assertFalse(buf.isEmpty())
        self.assertEqual(len(buf), 6)

        popped = buf.pop_back(5)
        self.assertFalse(buf.isFull())
        self.assertFalse(buf.isEmpty())
        self.assertEqual(len(buf), 1)
        self.assertEqual(popped, bytes([3, 4, 111, 222, 255]))


if __name__ == "__main__":
    unittest.main()
