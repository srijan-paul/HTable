import random
import string

def word():
  return ''.join(random.choices(string.ascii_uppercase + string.ascii_lowercase + string.digits, k=random.randint(10, 50)))

file = open("dictionary.txt", "w+")

for i in range(100):
  file.write(word() + "\n")

file.close()
