#!/usr/bin/env bash

helpFunction()
{
   echo ""
   echo "Usage: $0 -version version_string"
   echo -e "\t-version The version string, usually a date with the syntax YYYY_MM_DD"
   exit 1 # Exit script after printing help
}

if [ $# -lt 2 ]
then
    helpFunction
fi

rm -r ./leyden_jar_release_package

mkdir ./leyden_jar_release_package
mkdir ./leyden_jar_release_package/f122
mkdir ./leyden_jar_release_package/f104
mkdir ./leyden_jar_release_package/f77
mkdir ./leyden_jar_release_package/b122
mkdir ./leyden_jar_release_package/b104

cp ./leyden_jar_f122*.uf2 ./leyden_jar_release_package/f122
cp ./leyden_jar_f104*.uf2 ./leyden_jar_release_package/f104
cp ./leyden_jar_f77*.uf2 ./leyden_jar_release_package/f77
cp ./leyden_jar_b122*.uf2 ./leyden_jar_release_package/b122
cp ./leyden_jar_b104*.uf2 ./leyden_jar_release_package/b104

cd ./leyden_jar_release_package

zip -r ../leyden_jar_firmware_package_$2.zip ./*

cd ..
