x = 1.0
i = 0
for i in range(0, 99999999):
    x = (i + i + 2 * i + 1 - 0.379) / (x)

print(x)