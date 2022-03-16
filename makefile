main:
	gcc main.cpp file.cpp oldestfirst.cpp request.cpp json/jsoncpp.cpp logger/Logger.cpp -o out/program -lstdc++
	out/program parameters.json