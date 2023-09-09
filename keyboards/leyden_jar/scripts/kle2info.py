import sys
import json

if len(sys.argv) != 3:
    print("Usage: kle2info <in-kle.json> <out-info.json>")
    sys.exit(1)

f = None
try:
    f = open(sys.argv[1], "r")
except FileNotFoundError:
    print("Cannot open",sys.argv[1], "for reading")
    sys.exit(1)

kleJson = json.load(f)
f.close()

info = []
currentY = 0

for row in kleJson:
    currentX = 0
    currentW = 1
    if type(row) is not dict:
        for elem in row:
            if type(elem) is str or elem.get("a") is None:
                if type(elem) is dict:
                    currentX += elem.get("x", 0)
                    currentY += elem.get("y", 0)
                    currentW = elem.get("w", 1)
                else:
                    rowColString = elem.split(",")
                    x = 255
                    y = 255
                    if len(rowColString) == 2:
                        x = int(rowColString[0])    
                        y = int(rowColString[1])
                    infoElem = dict()
                    infoElem["matrix"] = [x, y]
                    if currentW != 1:
                        infoElem["w"] = currentW
                    infoElem["x"] = currentX
                    infoElem["y"] = currentY
                    info.append(infoElem)
                    currentX += currentW
                    currentW = 1
        currentY += 1

try:
    f = open(sys.argv[2], "w")
except FileNotFoundError:
    print("Cannot open",sys.argv[2], "for writing")
    sys.exit(1)

f.write("{\n")
f.write("\t\"layouts\": {\n")
f.write("\t\t\"LAYOUT_all\": {\n")
f.write("\t\t\t\"layout\": [\n")

for elem in info:
    s = "\t\t\t\t{ \"matrix\": [" + str(elem["matrix"][0]) + ", " + str(elem["matrix"][1]) + "]"
    if elem.get("w") is not None:
        s = s + ", \"w\": " + str(elem["w"])
    s = s + ", \"x\": " + str(elem["x"])
    s = s + ", \"y\": " + str(elem["y"])
    if elem != info[-1]:
        s = s + " },\n"
    else:
        s = s + " }\n"
    f.write(s)

f.write("\t\t\t]\n")
f.write("\t\t}\n")
f.write("\t}\n")
f.write("}\n")

f.close()        
        
