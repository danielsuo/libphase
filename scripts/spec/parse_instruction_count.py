import sys

path = sys.argv[1]
print(path)

with open(path, "r") as f:
    for line in f:
        if line.find("==>") > -1:
            benchmark = line.split()[1].split("/")[-1].replace(".count", "")
        if line.split(",")[0].isnumeric():
            num_ins = int(line.split(",")[0])
            print("{:>20}: {:>20,}".format(benchmark, num_ins))
