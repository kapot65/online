#!/bin/bash
mkdir build
cd build
qmake ../../online.pro
make -j $(python -c "from multiprocessing import cpu_count; print(cpu_count())")
cd ..
cp CamacClientSettings.ini build/CamacClient/
cp HV_ServerSettings.ini build/HV_Server/
cp CCPC7_ServerSettings.ini build/CCPC7_Server/
