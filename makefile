# Replacement policy options (second parameter): oldestfirst, largestfirst

main:
	gcc main.cpp file.cpp simulation.cpp event.cpp json/jsoncpp.cpp logger/Logger.cpp splay/splay.cpp -o out/program -lstdc++
	out/program parameters.json largestfirst