# Replacement policy options (second parameter): oldestfirst, largestfirst, leastrecent

main:
	gcc main.cpp file.cpp simulation.cpp event.cpp json/jsoncpp.cpp logger/Logger.cpp splay/splay.cpp -o out/program -lstdc++
	out/program parameters.json leastrecent