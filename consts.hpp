#include <iostream>
#include <windows.h>
using std::cout;
using std::endl;
using std::string;
using std::to_string;

const int pageSize = 4096,
    pageCount = 13,
    hProcessCount = 5;
    //суммарное количество одновременно работающих читателей и писателей должно
    //быть не менее числа страниц буферной памяти.

const string mutexName = "IOMutex",
    mapName = "mappingFile",
    fileName = "text.txt";
