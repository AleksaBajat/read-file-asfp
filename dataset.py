import random

random.seed(123)
locations = [f"loc{l}" for l in range(10000)]

cur = 1

def generate_lines():
    for i in range(10000000):
        temp = random.randint(-999, 999) / 10
        yield f"{random.choice(locations)};{temp}\n"


with open("gas_stations.csv", "w", buffering=1024*1024) as f:
        f.writelines(generate_lines())
