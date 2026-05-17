import sys

if len(sys.argv) != 3:
    print("Usage: set_leyden_jar_firmware_date <year_month_day> <keyboard.json>")
    sys.exit(1)

releaseDate = sys.argv[1].split('_')

f = None
try:
    f = open(sys.argv[2], "r")
except FileNotFoundError:
    print("Cannot open",sys.argv[2], "for reading")
    sys.exit(1)

inKeyboardJsonStr = f.read()
f.close()

releaseIndex = inKeyboardJsonStr.find("(release")
oldkeyboardNameString = inKeyboardJsonStr[releaseIndex:releaseIndex+20]
newKeyboardNameString = "(release " + releaseDate[0] + '.' + releaseDate[1] + '.' + releaseDate[2] + ')'
outKeyboardJsonStr = inKeyboardJsonStr.replace(oldkeyboardNameString, newKeyboardNameString)

try:
    f = open(sys.argv[2], "w")
except FileNotFoundError:
    print("Cannot open",sys.argv[2], "for writing")
    sys.exit(1)

f.write(outKeyboardJsonStr)
f.close()
