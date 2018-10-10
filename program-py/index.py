import random
import os

workdir = os.getcwd()

def getandwrite(filename):
    with open(os.path.join(workdir, filename), "w") as file:
        result = [chr(x) for x in [random.randint(97, 120) for i in range(10)]]
        file.write(''.join(result))
        file.write('\n')
        return result

firststring = getandwrite("fileone.txt")
secondstring = getandwrite("filetwo.txt")
thirdstring = getandwrite("filethree.txt")

print(''.join(firststring))
print(''.join(secondstring))
print(''.join(thirdstring))

firstrand = random.randint(1, 42)
secondrand = random.randint(1, 42)

print(firstrand)
print(secondrand)
print(firstrand * secondrand)
