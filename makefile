main:
	gcc main.cpp file.cpp oldestfirst.cpp json/jsoncpp.cpp -o out/program -lstdc++
	out/program parameters.json