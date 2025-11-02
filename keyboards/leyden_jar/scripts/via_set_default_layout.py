import sys
import json
import functools

# Checking that the number of arguments are correct

if len(sys.argv) != 4:
    print("Usage: via_set_default_layout <in-via.json> <in-layout.json> <out-via.json>")
    sys.exit(1)

# Opening the 2 input files

inVia = None
try:
    inVia = open(sys.argv[1], "r")
except FileNotFoundError:
    print("Cannot open",sys.argv[1], "for reading")
    sys.exit(1)

inLayout = None
try:
    inLayout = open(sys.argv[2], "r")
except FileNotFoundError:
    print("Cannot open",sys.argv[1], "for reading")
    sys.exit(1)

# Parsing JSON data of the 2 input files

inViaJson = None
try:
    inViaJson = json.load(inVia)
except json.JSONDecodeError:
    print("The file",sys.argv[1], "is not a valid JSON file")
    sys.exit(1)
inVia.close()

inLayoutJson = None
try:
    inLayoutJson = json.load(inLayout)
except json.JSONDecodeError:
    print("The file",sys.argv[2], "is not a valid JSON file")
    sys.exit(1)
inLayout.close()

# Preparing JSON KLE header for output file

outViaJson = dict()
try:
    outViaJson["name"] = inViaJson["name"]
    outViaJson["vendorId"] = inViaJson["vendorId"]
    outViaJson["productId"] = inViaJson["productId"]
    outViaJson["lighting"] = inViaJson["lighting"]
    outViaJson["matrix"] = inViaJson["matrix"]
    outViaJson["layouts"] = dict()
    outViaJson["layouts"]["labels"] = inViaJson["layouts"]["labels"].copy()
except:
    print("The file",sys.argv[1], "is missing requested VIA information")
    sys.exit(1)

# Reading layout options

layoutOptions = []
try:
    labels = inViaJson["layouts"]["labels"]
    for label in labels:
        option = dict()
        option["selected"] = 0
        if isinstance(label, str):
            option["checkbox"] = True
            option["name"] = label
            option["position"] = [{"x" : -1.0, "y" : -1.0}, {"x" : -1.0, "y" : -1.0}]
            option["index"] = [0, 1]
        else:
            option["checkbox"] = False
            option["name"] = label[0]
            option["choice"] = label[1:]
            option["position"] = []
            option["index"] = []
            for i in range(len(option["choice"])):
                option["position"].append({"x" : -1.0, "y" : -1.0})
                option["index"].append(i)    
        layoutOptions.append(option)
except:
    print("The file",sys.argv[1], "is missing VIA layout options information")
    sys.exit(1)

# Reading selected layouts

parseIndex = 0
for option, choice in inLayoutJson.items():
    if isinstance(choice, bool):
        index = 0
        if choice == True:
            index = 1
        layoutOptions[parseIndex]["selected"] = index
    else:
        if "choice" in layoutOptions[parseIndex]:
            layoutOptions[parseIndex]["selected"] = layoutOptions[parseIndex]["choice"].index(choice)
        else:
            layoutOptions[parseIndex]["selected"] = choice
    parseIndex += 1

# Reading JSON KLE data, transforming it in our own more user friendly format
# x and y positions are now absolutes
# w, w2, h, h2, c fields are all defined with default values if not in the original KLE data
# row_col array pair stores the row colomn info of each key
# option_choice array pair stores the VIA layout option, with [-1, -1] as default value if not in the original KLE data
# layout option/choice positions are also absolutes

currentY = 0.0
inViaKeyboardInfo = []
try:
    keymap = inViaJson["layouts"]["keymap"]
    for row in keymap:
        currentX = 0.0
        currentX2 = 0.0
        currentW = 1.0
        currentW2 = 0.0
        currentH = 1.0
        currentH2 = 0.0
        currentColor = "#cccccc"
        inViaRow = []

        for elem in row:
            if isinstance(elem, dict):
                currentX += float(elem.get("x", 0))
                currentX2 += float(elem.get("x2", 0))
                currentY += float(elem.get("y", 0))
                currentW = float(elem.get("w", 1))
                currentW2 = float(elem.get("w2", 0))
                currentH = float(elem.get("h", 1))
                currentH2 = float(elem.get("h2", 0))
                currentColor = elem.get("c", "#cccccc")
            else:
                splitElem = elem.split("\n\n\n")
                tmp = splitElem[0].split(",")
                rowColPair = [int(tmp[0]), int(tmp[1])]
                optionChoicePair = None
                if len(splitElem) == 2:
                    tmp = splitElem[1].split(",")
                    optionChoicePair = [int(tmp[0]), int(tmp[1])]
                else:
                    optionChoicePair = [-1, -1]
                keyInfo = dict()
                keyInfo["x"] = currentX
                keyInfo["x2"] = currentX2
                keyInfo["y"] = currentY
                keyInfo["w"] = currentW
                keyInfo["w2"] = currentW2
                keyInfo["h"] = currentH
                keyInfo["h2"] = currentH2
                keyInfo["c"] = currentColor
                keyInfo["row_col"] = rowColPair
                keyInfo["option_choice"] = optionChoicePair
                inViaRow.append(keyInfo)
                if (optionChoicePair[0] != -1 and optionChoicePair[1] != -1 and
                    layoutOptions[optionChoicePair[0]]["position"][optionChoicePair[1]]["x"] == -1.0 and
                    layoutOptions[optionChoicePair[0]]["position"][optionChoicePair[1]]["y"] == -1.0):
                    layoutOptions[optionChoicePair[0]]["position"][optionChoicePair[1]]["x"] = currentX + currentX2
                    layoutOptions[optionChoicePair[0]]["position"][optionChoicePair[1]]["y"] = currentY
                currentX += currentW
                currentX2 = 0.0
                currentW = 1.0
                currentW2 = 0.0
                currentH = 1.0
                currentH2 = 0.0
        inViaKeyboardInfo.append(inViaRow)
        currentY += 1
except:
    print("The file",sys.argv[1], "is missing requested VIA information or is malformed")
    sys.exit(1)

# If an alternate layout is selected we exchange it with the default (index 0) one.
# This includes exchanging layout index and also layout keys positions.

layoutIndex = 0
for layout in layoutOptions:
    selectedLayout = 0
    if not isinstance(layout["selected"], int):
        selectedLayout = 1
    else:
        selectedLayout = layout["selected"]
    for position in layout["position"]:
        position["transX"] = 0.0
        position["transY"] = 0.0
    if selectedLayout != 0:
        defaultToSelectedOffsetX = layout["position"][selectedLayout]["x"] - layout["position"][0]["x"]
        defaultToSelectedOffsetY = layout["position"][selectedLayout]["y"] - layout["position"][0]["y"]
        layout["position"][0]["transX"] = defaultToSelectedOffsetX
        layout["position"][0]["transY"] = defaultToSelectedOffsetY
        layout["position"][selectedLayout]["transX"] = -defaultToSelectedOffsetX
        layout["position"][selectedLayout]["transY"] = -defaultToSelectedOffsetY
        if not isinstance(layout["selected"], int):
            outViaJson["layouts"]["labels"][layoutIndex] = layout["selected"]
        else:
            tmp = outViaJson["layouts"]["labels"][layoutIndex][1]
            outViaJson["layouts"]["labels"][layoutIndex][1] = outViaJson["layouts"]["labels"][layoutIndex][selectedLayout + 1]
            outViaJson["layouts"]["labels"][layoutIndex][selectedLayout + 1] = tmp
        tmp = layout["index"][0]
        layout["index"][0] = layout["index"][selectedLayout]
        layout["index"][selectedLayout] = tmp

    layoutIndex += 1

for row in inViaKeyboardInfo:
    for key in row:
        if key["option_choice"][0] != -1:
            key["x"] += layoutOptions[key["option_choice"][0]]["position"][key["option_choice"][1]]["transX"]
            key["y"] += layoutOptions[key["option_choice"][0]]["position"][key["option_choice"][1]]["transY"]
            key["option_choice"][1] = layoutOptions[key["option_choice"][0]]["index"][key["option_choice"][1]]

# Find minimal X and Y positions for the keys

minX = 666.0
minY = 666.0
for row in inViaKeyboardInfo:
    for key in row:
        minX = min (minX, key["x"])
        minY = min (minY, key["y"])

# Translate keys to zero position

for row in inViaKeyboardInfo:
    for key in row:
        key["x"] -= minX
        key["y"] -= minY

# Put all keys in a pool and sort them

unsortedKeyPool = []
for row in inViaKeyboardInfo:
    for key in row:
        unsortedKeyPool.append(key)

def compareKey(key0, key1):
    if key0["y"] > key1["y"]:
        return 1
    if key0["y"] < key1["y"]:
        return -1
    if key0["x"] > key1["x"]:
        return 1
    if key0["x"] < key1["x"]:
        return -1
    return 0

sortedKeyPool = sorted(unsortedKeyPool, key=functools.cmp_to_key(compareKey))

# put back the key in an array of rows

outViaKeyboardInfo = []
currentY = -1.0
currentRow = None
for key in sortedKeyPool:
    if key["y"] != currentY:
        if currentRow is not None:
            outViaKeyboardInfo.append(currentRow)
        currentRow = []
        currentY = key["y"]
    currentRow.append(key)
if currentRow is not None and len(currentRow) > 0:
    outViaKeyboardInfo.append(currentRow)

# Generating target keymap

currentY = 0.0
outKeymap = []

#for row in inViaKeyboardInfo:
for row in outViaKeyboardInfo:
    currentX = 0.0
    currentX2 = 0.0
    currentW = 1.0
    currentW2 = 0.0
    currentH = 1.0
    currentH2 = 0.0
    currentColor = "#cccccc"
    outRow = []
    for elem in row:
        outDict = dict()
        if elem["x"] != currentX:
            outDict["x"] = elem["x"] - currentX
            currentX = elem["x"]
        if elem["y"] != currentY:
            outDict["y"] = elem["y"] - currentY
            currentY = elem["y"]
        if elem["w"] != currentW:
            outDict["w"] = elem["w"]
            currentW = elem["w"]
        if elem["h"] != currentH:
            outDict["h"] = elem["h"]
        if elem["w2"] != currentW2:
            outDict["w2"] = elem["w2"]
        if elem["h2"] != currentH2:
            outDict["h2"] = elem["h2"]
        if elem["x2"] != currentX2:
            outDict["x2"] = elem["x2"]

        if bool(outDict) is True:
           outRow.append(outDict)

        if elem["option_choice"][0] != -1:
            outRow.append(str(elem["row_col"][0]) + "," + str(elem["row_col"][1]) + "\n\n\n" + 
                          str(elem["option_choice"][0])  + "," + str(elem["option_choice"][1]))
        else:
            outRow.append(str(elem["row_col"][0]) + "," + str(elem["row_col"][1]))

        currentX += currentW
        currentX2 = 0.0
        currentW = 1.0
        currentW2 = 0.0
        currentH = 1.0
        currentH2 = 0.0

    outKeymap.append(outRow)
    currentY += 1

outViaJson["layouts"]["keymap"] = outKeymap

# Writing output JSON file

try:
    outVia = open(sys.argv[3], "w")
except FileNotFoundError:
    print("Cannot open",sys.argv[3], "for writing")
    sys.exit(1)

json.dump(outViaJson, outVia, indent=4)
outVia.close()

