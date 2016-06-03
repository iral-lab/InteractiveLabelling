#ifndef MODEL_H   
#define MODEL_H
#include <stdlib.h>
#include <sstream>
#include <iostream>
#include <string>
#include <vector>
#include <map>

using namespace std;

void train(vector<int> rgb, vector<string> tokens);

void test(vector<int> rgb, vector<string> tokens);
vector<string> getFiles(string fPath);
void learnModel();

void trainFromFile(string fName);
void testConf(string fName);
map<string,float> testAll(vector<int> rgb,vector<string> tokens,map<string,float> &probs) ;

vector<string> split_string(const string& str,
                                      const string& delimiter);
void testing();
void addTokens(vector<int> rgb, vector<string> tokens, bool pos,string fName);
map<string,float> checkProbsSparse(vector<int> rgb,vector<string> tokens,vector<string> fs,map<string,float> &probs);
#endif
