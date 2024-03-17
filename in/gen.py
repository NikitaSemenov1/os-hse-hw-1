from string import ascii_letters, digits
import random


length = int(input())

chars = ascii_letters + digits

print(''.join([random.choice(chars) for _ in range(length)]))
