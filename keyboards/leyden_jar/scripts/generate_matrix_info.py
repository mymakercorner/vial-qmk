import sys
import json

if len(sys.argv) != 4:
    print("Usage: generate_matrix_info <in-info.json> <qmk-layout.h> <out-info.json>")
    sys.exit(1)

f = None
try:
    f = open(sys.argv[1], "r")
except FileNotFoundError:
    print("Cannot open",sys.argv[1], "for reading")
    sys.exit(1)

infoJson = json.load(f)
f.close()

infoLayout = None

if infoJson.get("layouts") == None or infoJson["layouts"].get("LAYOUT_all") == None or infoJson["layouts"]["LAYOUT_all"].get("layout") == None:
    print("Not a valid info.json file...")
else:
    infoLayout = infoJson["layouts"]["LAYOUT_all"]["layout"]

try:
    f = open(sys.argv[2], "r")
except FileNotFoundError:
    print("Cannot open",sys.argv[2], "for reading")
    sys.exit(1)
    
infoQmk = f.read()
f.close()

layoutAllStartStr = "LAYOUT_all( \\\n"
posLayoutStart = infoQmk.find(layoutAllStartStr)
posLayoutStart += len(layoutAllStartStr)
layoutAllEndStr = "\n) \\"
posLayoutEnd = infoQmk.find(layoutAllEndStr)

layoutQmkStr = infoQmk[posLayoutStart:posLayoutEnd]
rowLayoutQmkStr = layoutQmkStr.split("\\")

layoutQmk = []

for rowStr in rowLayoutQmkStr:
    colStr = rowStr.split(",")
    rowQmk = []
    for keyStr in colStr:
        cleankeyStr = keyStr.strip()
        if len(cleankeyStr) > 0:
            rowQmk.append(cleankeyStr)
    if len(rowQmk) > 0:
        layoutQmk.append(rowQmk)

matrixStartStr = "{ \\\n"
posMatrixStart = infoQmk.find(matrixStartStr)
posMatrixStart += len(matrixStartStr)
matrixEndStr = "\\\n}"
posMatrixEnd = infoQmk.find(matrixEndStr)

matrixQmkStr = infoQmk[posMatrixStart:posMatrixEnd]    

rowMatrixQmkStr = matrixQmkStr.split("\\")

matrixQmk = []

for rowStr in rowMatrixQmkStr:
    s = rowStr.replace("{", "")
    s = s.replace("}", "")
    colStr = s.split(",")
    rowQmk = []
    for keyStr in colStr:
        cleankeyStr = keyStr.strip()
        if len(cleankeyStr) > 0:
            rowQmk.append(cleankeyStr)
    if len(rowQmk) > 0:
        matrixQmk.append(rowQmk)

# Sanity checks

nbLayoutQmkKeys = 0
for row in layoutQmk:
    nbLayoutQmkKeys += len(row)

nbInfoKeys = len(infoLayout)

if nbLayoutQmkKeys != nbInfoKeys:
    print("ERROR: info file has", nbInfoKeys, "keys", "while QMK layout has", nbLayoutQmkKeys)
    sys.exit(1)

infoOffset = 0    
for row in layoutQmk:
    for key in row:
        for matrixX in range(len(matrixQmk)):
            for matrixY in range(len(matrixQmk[matrixX])):
                if key == matrixQmk[matrixX][matrixY]:
                    infoLayout[infoOffset]["matrix"][0] = matrixX
                    infoLayout[infoOffset]["matrix"][1] = matrixY
                    infoOffset += 1

# Sanity checks
for key in infoLayout:
    if key["matrix"][0] == 255 or key["matrix"][1] == 255:
        print("ERROR: info file has unpopulated matrix information")
        sys.exit(1)

try:
    f = open(sys.argv[3], "w")
except FileNotFoundError:
    print("Cannot open",sys.argv[3], "for writing")
    sys.exit(1)

f.write("{\n")
f.write("\t\"layouts\": {\n")
f.write("\t\t\"LAYOUT_all\": {\n")
f.write("\t\t\t\"layout\": [\n")

for elem in infoLayout:
    s = "\t\t\t\t{ \"matrix\": [" + str(elem["matrix"][0]) + ", " + str(elem["matrix"][1]) + "]"
    if elem.get("w") is not None:
        s = s + ", \"w\": " + str(elem["w"])
    s = s + ", \"x\": " + str(elem["x"])
    s = s + ", \"y\": " + str(elem["y"])
    if elem != infoLayout[-1]:
        s = s + " },\n"
    else:
        s = s + " }\n"
    f.write(s)

f.write("\t\t\t]\n")
f.write("\t\t}\n")
f.write("\t}\n")
f.write("}\n")

f.close()        

print("end")
