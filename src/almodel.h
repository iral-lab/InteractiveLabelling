#ifndef ALMODEL_H   
#define ALMODEL_H
#include <stdlib.h>
#include <sstream>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

int altrain(vector<int> rgb, vector<string> tokens,int qnType);

void altest(vector<int> rgb, vector<string> tokens);

void testConfAL(string fName);
void altrainFromFile(string fName);
#endif
