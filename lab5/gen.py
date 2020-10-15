import random
n = 300
for i in range(0, n):
    for i in range(0, n + 1):
        print(random.randint(0,9), end = ' ')
    print("\n",end = '')
