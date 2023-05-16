import sys
import json

if len(sys.argv) != 3:
    print("Usage: info2kle <in-info.json> <out-kle.json>")
    sys.exit(1)

f = None
try:
    f = open(sys.argv[1], "r")
except FileNotFoundError:
    print("Cannot open",sys.argv[1], "for reading")
    sys.exit(1)

infoJson = json.load(f)
f.close()

kle = []

if infoJson.get("layouts") == None or infoJson["layouts"].get("LAYOUT_all") == None or infoJson["layouts"]["LAYOUT_all"].get("layout") == None:
    print("Not a valid info.json file...")
else:
    layout = infoJson["layouts"]["LAYOUT_all"]["layout"]
    currentY = 0
    currentX = 0
    currentRowList = None
    needNewRow = True
    
    for key in layout:
        if key.get("matrix") == None or len(key["matrix"]) != 2:
            print("No valid matrix information...")
        elif key.get("x") == None or key.get("y") == None:
            print("No key position information...")
        
        y = key["y"]
        x = key["x"]
        w = 1
        if key.get("w") is not None:
            w = key["w"]
        
        if (y - currentY) != 0:
            needNewRow = True
        
        if needNewRow is True:
            if currentRowList is not None and currentRowList is not False:
                kle.append(currentRowList)
            if currentRowList is not None:
                currentY += 1
            currentX = 0
            currentRowList = []
            needNewRow = False
        
        diffX = x - currentX
        diffY = y - currentY
            
        keyInfo = dict()
        if diffX != 0:
            keyInfo["x"] = diffX
            currentX += diffX
        if diffY != 0:
            keyInfo["y"] = diffY
            currentY += diffY
        if w != 1:
            keyInfo["w"] = w
        
        if bool(keyInfo) is not False:
            currentRowList.append(keyInfo)
        currentRowList.append(str(key["matrix"][0]) + ',' + str(key["matrix"][1]))
        currentX += w           
      
    if currentRowList is not None and currentRowList is not False:
        kle.append(currentRowList)
    
try:
    f = open(sys.argv[2], "w")
except FileNotFoundError:
    print("Cannot open",sys.argv[2], "for writing")
    sys.exit(1)

json.dump(kle, f, indent=4)
f.close()