rootpath=

g++ -fdiagnostics-color=always -g -O3 -flto \
    -I "$rootpath/header" \
    -I "$rootpath/header/spp" \
    "$rootpath/src/main.cpp" \
    "$rootpath/src/graph.cpp" \
    "$rootpath/src/graphSymInitiation.cpp" \
    "$rootpath/src/match.cpp" \
    "$rootpath/src/matchEnumeration.cpp" \
    "$rootpath/src/newMatchSymEnumeration.cpp" \
    "$rootpath/src/print.cpp" \
    -o "$rootpath/MQSymMatchSO"