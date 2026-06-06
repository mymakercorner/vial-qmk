helpFunction()
{
   echo ""
   echo "Usage: $0 -release-date date_string"
   echo -e "\t-release-date The release date string, a date with the syntax YYYY_MM_DD"
   exit 1 # Exit script after printing help
}

if [ $# -lt 2 ]
then
    helpFunction
fi

cd ./keyboards/leyden_jar

python ./scripts/set_leyden_jar_firmware_date.py $2 ./b104/keyboard.json
python ./scripts/set_leyden_jar_firmware_date.py $2 ./b122/keyboard.json
python ./scripts/set_leyden_jar_firmware_date.py $2 ./f62/keyboard.json
python ./scripts/set_leyden_jar_firmware_date.py $2 ./f77/keyboard.json
python ./scripts/set_leyden_jar_firmware_date.py $2 ./f104/keyboard.json
python ./scripts/set_leyden_jar_firmware_date.py $2 ./f122/keyboard.json
python ./scripts/set_leyden_jar_firmware_date.py $2 ./f50/keyboard.json
