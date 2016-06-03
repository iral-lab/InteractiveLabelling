#include "model.h"
#include <vector>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>
#include <string>
#include <iostream>
#include <algorithm>
#include <fstream>
#include "boost/regex.hpp"
#include "boost/algorithm/string/regex.hpp"
using namespace std;
#include <set>
#include <map>
using namespace boost::filesystem;
float basej = 0.0;
int spFlag = 0;
int threshArff = 2;

string tNoC = "25";
string trName = "Train" + tNoC;

string dataFile = "/home/kamala/catkin_ws/src/jm/src/" + trName + "/trainArff"+tNoC+".txt";
string arffName = "/home/kamala/catkin_ws/src/jm/src/" + trName + "/jmModel"+tNoC+".arff";
string modelName = "/home/kamala/catkin_ws/src/jm/src/" + trName + "/jm"+tNoC+".model";

string fDataFile = "/home/kamala/catkin_ws/src/jm/src/" + trName + "/trainData"+tNoC+".txt";

string classifier = "weka.classifiers.functions.Logistic";
string smoClassifier = "weka.classifiers.functions.SMO";
string wekaLoc = "/home/kamala/catkin_ws/src/jm/src/weka.jar";

string results = "/home/kamala/catkin_ws/src/jm/src/" + trName + "/testResults.log"; 
string totResults = "/home/kamala/catkin_ws/src/jm/src/" + trName + "/overallTestResults.log";

string testArffName = "/home/kamala/catkin_ws/src/jm/src/" + trName + "/test"+tNoC+".arff";

string trainFolder = "/home/kamala/catkin_ws/src/jm/src/"+ trName;

string trainFiles =  trainFolder + "/files";
string testArffFilesLoc = trainFolder + "/TestArffFiles";
string trainArff = trainFolder + "/Arff";
string trainModel = trainFolder + "/Models";


void clearArff() {
   string cmd = "rm -rf " + trainArff;
   int ret = system(cmd.c_str());  
}
//Function that prepares vector of R G B values from string
vector<int> getRGB(string temp) {
   vector <string> feature = split_string(temp,",");
   vector<int> rgb;
   for (unsigned ij = 0; ij < 3; ij++) {
      if(feature[ij] != "") {
         int digit = atoi(feature[ij].c_str());
         rgb.push_back(digit);
      }
   }
   return rgb;
}

// Preparing Arff file header with R,G,B as attributes and class as all classes
string arffhead(vector<string> classNames) {
   string arf ("@RELATION jmmodel \n\n@attribute 'r' numeric\n@attribute 'g' numeric\n@attribute 'b' numeric\n@attribute 'class' {");
   set<string> clsNames(classNames.begin(), classNames.end());
   ostringstream dt;
   for (set<string>::iterator it = clsNames.begin() ; it != clsNames.end(); ++it) {
      string sth;
      sth = *it;
      if(sth != "")
         dt << *it << ",";
   }
   arf += dt.str();
   arf.erase(arf.end() - 1,arf.end());   
   arf +=  "}\n";

   arf += "\n@data\n";

  return arf;

}

// Function that returns a string that joins all tokens with '-'
string testArfLoc(vector<string> tokens,string arFile) {
     string data;
     data = "";
     ostringstream dat;
     for (std::vector<string>::iterator itb = tokens.begin() ; itb != tokens.end(); ++itb) {
          dat << *itb;
          dat <<  "-";
     }

     data = dat.str();
     data.erase(data.end() - 1,data.end());
     return data;
     string cmd = "mkdir -p " + testArffFilesLoc ;
     int ret = system(cmd.c_str());
     cmd = "cp " + arFile + " " + testArffFilesLoc + "/" + data + ".arff ";
}

// A function that generates Arff body section in 'R,G,B, <token>' format
string arffAdd(vector<int> rgb, vector<string> tokens) {
   string data("");
   for (std::vector<string>::iterator it = tokens.begin() ; it != tokens.end(); ++it) {
       ostringstream dt;
       dt << "\n";
       if(*it != "") {
        
   	for (std::vector<int>::iterator itb = rgb.begin() ; itb != rgb.end(); ++itb) {
          dt << *itb;
          dt << ",";
        }
        data += dt.str();
        data.erase(data.end() - 1,data.end());
        data += ",";
        string cls = *it;
        boost::erase_all(cls, ",");
        data += cls;
       }  
    }
    return data;


}

// Funcction that appends Arff Body section to file
void prepareData(vector<int> rgb, vector<string> tokens) {
    string data = arffAdd(rgb,tokens) ;
    fstream fs;
    fs.open (dataFile,fstream::app);
    fs << data;
    fs.close();
}

vector<string> split_string(const string& str,
                                      const string& delimiter)
{
    vector<string> strings;

    string::size_type pos = 0;
    string::size_type prev = 0;
    while ((pos = str.find(delimiter, prev)) != string::npos)
    {
        strings.push_back(str.substr(prev, pos - prev));
        prev = pos + 1;
    }

    // To get the last substring (or only, if delimiter is not found)
    strings.push_back(str.substr(prev));

    return strings;
}
// A function that creates classNames vector by taking all last elements from the lines of  dataFile
vector<string> getClassNames() {
    fstream fs(dataFile,fstream::in);
    vector<string> classNames;
    string line;
    getline(fs,line,'\0');
    vector <string> cls = split_string(line,"\n");
    for (std::vector<string>::iterator itb = cls.begin() ; itb != cls.end(); ++itb) {
           string temp;
           temp = *itb;

           if(temp != "") {
              vector<string> chs ;
              chs = split_string(temp,",");
              string name = chs.back();
              classNames.push_back(name);
        }
    }
    return classNames;

}


void preparetestArff(vector<int> rgb, vector<string> tokens) {
    prepareData(rgb,tokens);
    ofstream ofs;
    ofs.open (testArffName, ofstream::trunc);

    fstream fs(dataFile,fstream::in);
    vector<string> classNames;
    classNames = getClassNames();
    string arf = arffhead(classNames);
    ofs << arf;
    ofs << "\n";
    ostringstream dt;   
    for (std::vector<int>::iterator itb = rgb.begin() ; itb != rgb.end(); ++itb) {
          dt << *itb;
          dt << ",";
    }
    string data;
    data = dt.str();
    //cout << data;
    for (vector<string>::iterator itb = classNames.begin() ; itb != classNames.end(); ++itb) {
	  ostringstream dat;
          dat << *itb;
          string value;
          value = dat.str();
          boost::erase_all(value, ",");
          ofs << data << value << endl;
    }
    ofs.close();
    testArfLoc(tokens,testArffName);
}

vector<string> getSpecFolders(string trFiles) {
    vector<string> fNames;
    string cmd = "mkdir -p " + trFiles;
    int ret = system(cmd.c_str());

    path p(trFiles);
    for (auto i = directory_iterator(p); i != directory_iterator(); i++)
    {
        if (is_directory(i->path())) //we eliminate directories
            //cout << i->path().filename().string() << endl;
            fNames.push_back(i->path().filename().string());
    }
    

    return fNames;

}

vector<string> getFolders() {
   string name = trainFiles;
   return getSpecFolders(name);

}


vector<string> getFiles(string fPath) {
    vector<string> fNames;
    string cmd = "mkdir -p " + fPath;
    int ret = system(cmd.c_str());

    path p(fPath);
    for (auto i = directory_iterator(p); i != directory_iterator(); i++)
    {
        if (! is_directory(i->path())) //we eliminate directories
           // cout << i->path().filename().string() << endl;
            fNames.push_back(i->path().filename().string());
    }


    return fNames;

}

vector<string> eraseValue(vector<string> fNames,string token) {
/*
    vector<int>::iterator position = find(fNames.begin(), fNames.end(), token);
    if (position != fNames.end())
        fNames.erase(position);  
*/
    fNames.erase(remove(fNames.begin(), fNames.end(), token), fNames.end());
    return fNames;
}

string prepareRGB(vector<int> rgb) {
     string data;
     data = "";
     ostringstream dat;
     for (std::vector<int>::iterator itb = rgb.begin() ; itb != rgb.end(); ++itb) {
          dat << *itb;
          dat <<  ",";
        }
        data = dat.str();       
        data.erase(data.end() - 1,data.end());
        return data;
}


string prepareToken(vector<int> rgb,vector<string> tokens) {
     string data;
     data = "";
     ostringstream dat;
     for (std::vector<int>::iterator itb = rgb.begin() ; itb != rgb.end(); ++itb) {
          dat << *itb;
          dat <<  ",";
     }
     for (std::vector<string>::iterator itb = tokens.begin() ; itb != tokens.end(); ++itb) {
          dat << *itb;
          dat <<  ",";
     }
    
     data = dat.str();
     data.erase(data.end() - 1,data.end());
     return data;
}

void featureAdd(string cmd,string tkValue) {
    fstream fs;
    fs.open (cmd,fstream::app);
    fs << tkValue << "\n";
    fs.close();

}

void addTokens(vector<int> rgb, vector<string> tokens, bool pos,string fName) {
    string rgbValue,tokenString ;
    rgbValue = prepareRGB(rgb);
    tokenString = prepareToken(rgb,tokens);
    if (fName == "") 
        fName = trainFiles;
    if(rgbValue != "") {
       for (std::vector<string>::iterator itb = tokens.begin() ; itb != tokens.end(); ++itb) {
           string temp;
           temp = *itb;
           string cmd = "mkdir -p " + fName + "/" + temp;
           int ret = system(cmd.c_str());
           cmd = trainFiles + "/" + temp ;
           if(pos)
              cmd += "/pos.txt";
           else
              cmd += "/neg.txt";

           if(spFlag)
              featureAdd(cmd,tokenString);
           else
               featureAdd(cmd,rgbValue);
       }
    }
}

void addTokenFiles(vector<int> rgb, vector<string> tokens) {
    prepareData(rgb,tokens);
    vector<string> fNames;
    fNames = getFolders();   
    for (std::vector<string>::iterator itb = tokens.begin() ; itb != tokens.end(); ++itb) {
           string temp;
           temp = *itb;
           fNames = eraseValue(fNames,temp);
    }    
    string rgbValue,tokenString ;
    rgbValue = prepareRGB(rgb);
    tokenString = prepareToken(rgb,tokens);
    if(rgbValue != "") {
       bool tFlag = false;
       for (std::vector<string>::iterator itb = tokens.begin() ; itb != tokens.end(); ++itb) {
           string temp;
           temp = *itb;
           tFlag = true;
           string cmd = "mkdir -p " + trainFiles + "/" + temp;
           int ret = system(cmd.c_str());
           cmd = trainFiles + "/" + temp + "/pos.txt";
           if(spFlag)       
              featureAdd(cmd,tokenString);
           else
              featureAdd(cmd,rgbValue);
           cmd = "touch " + trainFiles + "/" + temp + "/neg.txt";
           ret = system(cmd.c_str());
	}
        if(tFlag) {
          for (std::vector<string>::iterator itb = fNames.begin() ; itb != fNames.end(); ++itb) {
              string temp;
              temp = *itb;
           
              string cmd = trainFiles + "/" + temp + "/neg.txt";
	      if(spFlag)
                 featureAdd(cmd,tokenString);
	      else
                 featureAdd(cmd,rgbValue);

          }

        }

    }

}

string arffIndhead() {
   string arf ("@RELATION jmmodel \n\n@attribute 'r' numeric\n@attribute 'g' numeric\n@attribute 'b' numeric\n@attribute 'class' {0 ,1}");

   arf += "\n@data\n";

  return arf;

}

pair<string,int> prepareIndbodyArff(string fileName,string clsN) {

    fstream fs(fileName,fstream::in);
    string line;
    getline(fs,line,'\0');
    vector <string> cls = split_string(line,"\n");
    string arf = "";
    int count = 0;
    for (std::vector<string>::iterator itb = cls.begin() ; itb != cls.end(); ++itb) {
           string temp;
           temp = *itb;
           if(temp != "") {
             count++;
             arf += temp + "," + clsN + "\n";
           }
    }
    return make_pair(arf,count);


}
pair<string,int> prepareIndbodySparseArff(vector<string> cls,string fileName,string clsN,int elCount) {
    fstream fs(fileName,fstream::in);
    string line;
    getline(fs,line,'\0');
    vector <string> clsLines = split_string(line,"\n");
    string lineSparse;
    int count = 0;
    for (std::vector<string>::iterator itb = clsLines.begin() ; itb != clsLines.end(); ++itb) {
	   if(clsN == "0" and count >= elCount)
		continue;
           string temp;
           temp = *itb;
           if(temp != "") {
              count++;
              vector<string> chs ;
              chs = split_string(temp,",");
               int ij = 0;
               ostringstream dt;

               vector<int> indices;
               for (std::vector<string>::iterator it = chs.begin() ; it != chs.end(); ++it) {
               string temp2;
               temp2 = *it;

               if(ij < 3) {
                  if (ij > 0)
                     dt << ",";
                  dt <<  temp2 ;
               } else {
                  int index = find(cls.begin(),cls.end(),temp2) - cls.begin();
                  indices.push_back(index);
               }
               ij++;
              }
              set<int> ind(indices.begin(), indices.end());
              vector<int> indice(ind.begin(),ind.end());
              sort(indice.begin(),indice.end());
              int k = 0;
              for (std::vector<int>::iterator it = indice.begin() ; it != indice.end(); ++it) {
                 int ik = *it;
                 for (int rr = k; rr < ik; rr++) {
                    dt << ",0";
                 }
                 dt << ",1";

                 k = ik + 1;
              }
              lineSparse += dt.str();
              for (int rr = k; rr < cls.size(); rr++) {
                    lineSparse += ",0";
              }

              lineSparse +=  "," + clsN + "\n";
        }
    }

   return make_pair(lineSparse,count);
   
}


void prepareIndtestArff (vector<int> rgb) {
    string arf = arffIndhead();
    string rgbValue = prepareRGB(rgb);
    arf += rgbValue + ",1\n";
    ofstream ofs;
    ofs.open (testArffName, ofstream::trunc);
    ofs << arf;
    ofs.close();
 //   testArfLoc(rgb,testArffName);
}


void saveIndModel (string arff, string model) {
  ostringstream sM;
  sM << "java -cp ";
  sM << wekaLoc;
  sM << " ";
  sM << classifier;
  sM << " -x 2 -t ";
  sM << arff;
  sM << " -d ";
  sM << model;
  sM << " > saveResults.log";
  int ret = system(sM.str().c_str());
}

string arffheadSparse(vector<string> clsNames) {
   string arf ("@RELATION jmmodel \n\n@attribute R numeric\n@attribute G numeric\n@attribute B numeric\n");
   string sth;
   for (vector<string>::iterator it = clsNames.begin() ; it != clsNames.end(); ++it) {
      if(*it != "")
         sth += "@attribute "+ *it + " {0, 1}\n";
   }
   arf += sth;
   arf += "@attribute class {0,1}\n";
   arf += "\n@data\n";

  return arf;

}

void saveFullLog(string line) {
  fstream fs1(totResults,fstream::app);
  fs1 << line;
  fs1.close();
}

void arffBodySparse(vector<string> cls,string arf) {

   for (std::vector<string>::iterator itb = cls.begin() ; itb != cls.end(); ++itb) {
      string temp;
      temp = *itb;
      string fileName = trainFiles + "/" + temp + "/pos.txt";
      string clsN = "1";
      string tarf,t;
      int count,k;
      pair<string, int> ans = prepareIndbodySparseArff(cls,fileName,clsN,0);
      tarf = arf + ans.first;
      count = ans.second;
      fileName = trainFiles + "/" + temp + "/neg.txt";
      clsN = "0";
      ans = prepareIndbodySparseArff(cls,fileName,clsN,count);
      tarf += ans.first;
      //count += ans.second;
      if(count > threshArff) {
         string cmd = "mkdir -p " + trainArff;
         int ret = system(cmd.c_str());
         cmd = "chmod -R 777 " + trainArff;
         ret = system(cmd.c_str());
         
         fileName = trainArff + "/" + temp + ".arff";
         fstream fs;
         fs.open(fileName,fstream::app);
         fs << tarf ;
         fs.close();
         cmd = "mkdir -p " + trainModel;
         ret = system(cmd.c_str());

         string model = trainModel + "/" + temp + ".model";
         saveIndModel(fileName,model);
      }
   }


}

void prepareIndvArff() {
   string cmd = "mkdir -p " + trainArff;
   int ret = system(cmd.c_str());
   vector<string> fNames;
   fNames = getFolders();
   set<string> clsNames(fNames.begin(), fNames.end());
   vector<string> cls(clsNames.begin(),clsNames.end());
   string arf = arffheadSparse(cls);
   arffBodySparse(cls,arf);

}
string sparseIt(vector<string> cls,vector<int> rgb,vector<string> tokens) {
   ostringstream dt;
   for (std::vector<int>::iterator it = rgb.begin() ; it != rgb.end(); ++it) {
      dt << *it << ",";
   }
   vector<int> indices;
   for (std::vector<string>::iterator it = tokens.begin() ; it != tokens.end(); ++it) {
      string temp2;
      temp2 = *it;
      int index = find(cls.begin(),cls.end(),temp2) - cls.begin();
      indices.push_back(index);
   }
   set<int> ind(indices.begin(), indices.end());
   vector<int> indice(ind.begin(),ind.end());
   sort(indice.begin(),indice.end());
   int k = 0;
   string lineSparse;
   for (std::vector<int>::iterator it = indice.begin() ; it != indice.end(); ++it) {
      int ik = *it;
      for (int rr = k; rr < ik; rr++) {
         dt << "0,";
      //     dt << "1,";
      }
      dt << "1,";
      k = ik + 1;
   }
   lineSparse = dt.str();
   for (int rr = k; rr < cls.size(); rr++) {
      lineSparse += "0,";
    //    lineSparse += "1,";
   }
   lineSparse +=  "1\n";
   return lineSparse;
}

void prepareIndtestSparseArff(vector<int> rgb,vector<string> tokens) {
   vector<string> fNames;
   fNames = getFolders();
   set<string> clsNames(fNames.begin(), fNames.end());
   vector<string> cls(clsNames.begin(),clsNames.end());
   string arf = arffheadSparse(cls);
   string tarf = sparseIt(cls,rgb,tokens);
   ofstream ofs;
   ofs.open (testArffName, ofstream::trunc);
   ofs << arf ;
   ofs << tarf;
   ofs.close();
   testArfLoc(tokens,testArffName);
}

void addNegativeData(string temp,string fileName,int count) {
    int negNeeded = count;
    if (negNeeded == 0)
       negNeeded = 1;
    fstream fs(fileName,fstream::in);
    string line;
    getline(fs,line,'\0');
    vector <string> cls = split_string(line,"\n");
    vector <string> negData;
    for (std::vector<string>::iterator it = cls.begin() ; it != cls.end(); ++it) {
	    string temp2;
	    temp2 = *it;
            if(temp2 != "") {
               negData.push_back(temp);
            }
    }
    fs.close();
    int reqC = negNeeded - negData.size();
    fstream fs1(fDataFile,fstream::in);
    line;
    getline(fs1,line,'\0');
    cls = split_string(line,"\n");
    for (std::vector<string>::iterator it = cls.begin() ; it != cls.end(); ++it) {
	if(reqC > 0) {
            string temp2;
            temp2 = *it;
            if(temp2 != "") {
                 size_t found = temp2.find(temp);
		 if (found == std::string::npos) {
		    vector<int> rgb = getRGB(temp2);
                    string nRgb = prepareRGB(rgb); 
		    vector<string>::iterator itA;

		    itA = find(negData.begin(), negData.end(),nRgb);
		    if (itA == negData.end()) {
			    fstream fs3;
			    fs3.open (fileName,fstream::app);
			    fs3.close();
			    reqC--;
		    }
		 }
	    }
         }
    }
    fs1.close();
   
}
void prepareIndvArff1 () {
   string cmd = "mkdir -p " + trainArff;
   int ret = system(cmd.c_str());
   vector<string> fNames;
   fNames = getFolders();
   string arf = arffIndhead();
   for (std::vector<string>::iterator itb = fNames.begin() ; itb != fNames.end(); ++itb) {
      string temp;
      temp = *itb;
      string fileName = trainFiles + "/" + temp + "/pos.txt";
      string clsN = "1";    
      string tarf,t;
      int count,k;
      pair<string, int> ans = prepareIndbodyArff(fileName,clsN);
      tarf = arf + ans.first;
      count = ans.second;
      fileName = trainFiles + "/" + temp + "/neg.txt";
      //addNegativeData(temp,fileName,count);
      clsN = "0";
      ans = prepareIndbodyArff(fileName,clsN);
      tarf += ans.first;
      //count += ans.second;
      if(count > threshArff) {
         fileName = trainArff + "/" + temp + ".arff";
	 fstream fs;
	 fs.open (fileName,fstream::app);
	 fs << tarf ;
	 fs.close(); 
	 cmd = "mkdir -p " + trainModel;
	 ret = system(cmd.c_str());
   
	 string model = trainModel + "/" + temp + ".model"; 
	 saveIndModel(fileName,model);
      }
   }  

}
void prepareArff(vector<int> rgb, vector<string> tokens) {
    prepareData(rgb,tokens);
    
    ofstream ofs;
    ofs.open (arffName, ofstream::trunc);
    
    vector<string> existingValues;
    vector<string> completeDB;
    vector<string> classNames;
    fstream fs(dataFile,fstream::in);
    string line;
    getline(fs,line,'\0');
    vector <string> cls = split_string(line,"\n");
    for (std::vector<string>::iterator itb = cls.begin() ; itb != cls.end(); ++itb) {
           string temp;
           temp = *itb;

           if(temp != "") {
              completeDB.push_back(temp);

              vector<string> chs ;
              chs = split_string(temp,",");
              string name = chs.back();
              classNames.push_back(name);
	      boost::erase_all(temp,name);

              for (std::vector<string>::iterator it = existingValues.begin() ; it != existingValues.end(); ++it) {
                  string ext;
                  ext = *it;
                  if(ext != temp) {
                  
                     ext += name + "-syn";
                     bool chk = true;
                     for (std::vector<string>::iterator itb = completeDB.begin() ; itb != completeDB.end(); ++itb) {
                  		string extc;
                  		extc = *itb;
                                if(ext == extc) 
					chk = false;

			}
                        if(chk)
                    	 completeDB.push_back(ext);
                  }
	      }

              existingValues.push_back(temp);
           }     
    }
    string arf = arffhead(classNames);
    ofs << arf;
    ofs << "\n";
    for (vector<string>::iterator it = completeDB.begin() ; it != completeDB.end(); ++it) {
       string ext;
       ext = *it;
       ofs << ext;
       ofs << "\n";
    }
    fs.close();

    ofs.close();
    
}

void saveModel () {
  ostringstream sM;
  sM << "java -cp ";
  sM << wekaLoc;
  sM << " ";
  sM << classifier;
  sM << " -x 2 -t ";
  sM << arffName;
  sM << " -d ";
  sM << modelName;
  sM << " > saveResults.log";
  int ret = system(sM.str().c_str());
}

void testModel(string testarfName) {
  ostringstream sM;
  sM << "java -cp ";
  sM << wekaLoc;
  sM << " ";
  sM << classifier;
  sM << " -l ";
  sM << modelName;
  sM << " -T ";
  sM << testarfName;
  sM << " > ";
  sM << results;
  int ret = system(sM.str().c_str());
  fstream fs(results,fstream::in);
  string line;
  getline(fs,line,'\0');
  saveFullLog(sM.str());
  saveFullLog(line);
  //cout << line;
  fs.close();
  cout << "Results are saved in " << results << " ...." << endl;;
}

string testIndModelSparse(string testarfName,string learntArff) {
  ostringstream cM;
  cM << "echo " << learntArff << " >> " << results;
  int ret = system(cM.str().c_str());
  ostringstream sM;
  sM << "java -cp ";
  sM << wekaLoc;
  sM << " ";
  sM << smoClassifier;
  sM << " -C 1.0 -L 0.0010 -P 1.0E-12 -N 0 -V -1 -W 1 -K \"weka.classifiers.functions.supportVector.PolyKernel -C 250007 -E 2.0\"";
  sM << " -t ";
  sM << learntArff;
  sM << " -T ";
  sM << testarfName;
  if(spFlag) {
   vector<string> fNames;
   fNames = getFolders();
   sM << " -p 1-";
   int no = fNames.size();
   no += 3;
   sM << no;
  } else
     sM << " -p 1-3";

  sM << " > ";
  sM << results;
  ret = system(sM.str().c_str());
  fstream fs(results,fstream::in);
  string line;
  getline(fs,line,'\0');
  saveFullLog(sM.str());
  saveFullLog(line);
  fs.close();
  //cout << "Results are saved in " << results << " ...." << endl;;
  return line;
}



string testIndModel(string testarfName,string model) {
  ostringstream cM;
  cM << "echo " << model << " >> " << results;
  int ret = system(cM.str().c_str());
  ostringstream sM;
  sM << "java -cp ";
  sM << wekaLoc;
  sM << " ";
  sM << classifier;
  sM << " -l ";
  sM << model;
  sM << " -T ";
  sM << testarfName;
  if(spFlag) {
   vector<string> fNames;
   fNames = getFolders();
   sM << " -p 1-";
   int no = fNames.size();
   no += 3;
   sM << no;
  } else 
     sM << " -p 1-3";

  sM << " > ";
  sM << results;
  ret = system(sM.str().c_str());
  fstream fs(results,fstream::in);
  string line;
  getline(fs,line,'\0');
  saveFullLog(sM.str());
  saveFullLog(line);
  fs.close();
  //cout << "Results are saved in " << results << " ...." << endl;;
  return line;
}

void train(vector<int> rgb, vector<string> tokens) {
    addTokenFiles(rgb,tokens);
 //  prepareArff(rgb,tokens);
 //  saveModel();
}


void test1(vector<int> rgb, vector<string> tokens) {
//   prepareArff(rgb,tokens);
 //  saveModel();
   if(spFlag)
      prepareIndtestSparseArff(rgb, tokens);
   else 
      prepareIndtestArff(rgb);

   vector<string> fs = getFiles(trainModel);
   vector<string> correctCls;
   string cmd = "rm -rf " + results;
   int ret = system(cmd.c_str());
   for (vector<string>::iterator it = fs.begin() ; it != fs.end(); ++it) {
       string ext;
       ext = *it;
       string model = trainModel + "/" + ext;
       cout << ext << endl;
       string res = testIndModel(testArffName,model);

       vector <string> lines = split_string(res,"\n");
       for (std::vector<string>::iterator itb = lines.begin() ; itb != lines.end(); ++itb) {
           string temp;
           temp = *itb;

           if(temp.string::find("Correctly Classified Instances") != string::npos) {
              boost::erase_all(temp, "%"); 
              vector <string> ts = split_string(temp," ");
              string prob = "";
              for (unsigned ij = ts.size() - 1; ij >= 0;ij--) {
                 prob = ts[ij];
                 boost::erase_all(prob," ");
                 if(prob != "")
                    break;
              }
              int ab = atoi(prob.c_str());
	      string label = ext;
	      boost::erase_all(label,".model");
              cout << label << " -- " << ab << endl;
              if(ab > 75) 
                 correctCls.push_back(label);
             
          }
   }

  cout << res << endl;       
  }

  cout << endl << "Prediction :: ";
  for (int ij = 0; ij < correctCls.size();ij++) {
     string label = correctCls[ij];
     cout << label << " ";
  }

  cout << "\n";

}
map<string,float> checkProbsSparse(vector<int> rgb,vector<string> tokens,vector<string> fs,map<string,float> &probs) {
   if(spFlag)
      prepareIndtestSparseArff(rgb, tokens);
   else
      prepareIndtestArff(rgb);

   vector<string> correctCls;
   map<string,float> match;
   string cmd = "rm -rf " + results;
   int ret = system(cmd.c_str());
   string rgbValue = prepareRGB(rgb);
   for (vector<string>::iterator it = fs.begin() ; it != fs.end(); ++it) {
       string ext;
       ext = *it;
       string label = ext;
       boost::erase_all(label,".model");

   //    string model = trainArff + "/" + label + ".arff";
   //    string result = testIndModelSparse(testArffName,model);
       string model = trainModel + "/" + ext;
       string result = testIndModel(testArffName,model);

       bool fFlag = false;

           string temp;
           temp = result;
              fFlag = true;
              vector <string> ts = split_string(temp," ");
              string prob = "";
              for (unsigned ij = ts.size() - 2; ij >= 0;ij--) {
                 prob = ts[ij];
                 boost::erase_all(prob," ");
                 if(prob != "")
                    break;
              }
              float ab = atof(prob.c_str());

              bool fMatch = true;
              if(temp.string::find(" + ") != string::npos) {
                 ab = 1.0 - ab;
                 fMatch = false;
              }
              probs[label] = ab;
              std::ostringstream ss;
              ss << label << " : " << ab;
              std::string s(ss.str());
              if(fMatch)
                 match[label] = ab;
   }
   return match;
}


map<string,float> checkProbs(vector<int> rgb,vector<string> tokens,vector<string> fs,map<string,float> &probs) {
   if(spFlag)
      prepareIndtestSparseArff(rgb, tokens);
   else
      prepareIndtestArff(rgb);
   vector<string> correctCls;
   map<string,float> match;
   string cmd = "rm -rf " + results;
   int ret = system(cmd.c_str());
   string rgbValue = prepareRGB(rgb);
   for (vector<string>::iterator it = fs.begin() ; it != fs.end(); ++it) {
       string ext;
       ext = *it;
       string model = trainModel + "/" + ext;
       string result = testIndModel(testArffName,model);
       bool fFlag = false;

           string temp;
           temp = result;
              fFlag = true;
              vector <string> ts = split_string(temp," ");
              string prob = "";
              for (unsigned ij = ts.size() - 2; ij >= 0;ij--) {
                 prob = ts[ij];
                 boost::erase_all(prob," ");
                 if(prob != "")
                    break;
              }
              string label = ext;
              boost::erase_all(label,".model");
              float ab = atof(prob.c_str());

              bool fMatch = true;
              if(temp.string::find(" + ") != string::npos) {
                 ab = 1.0 - ab;
                 fMatch = false;
              }
              probs[label] = ab;
              std::ostringstream ss;
              ss << label << " : " << ab;
              std::string s(ss.str());
              if(fMatch)
                 match[label] = ab;
   }
   return match;
}

map<string,float> testAll(vector<int> rgb,vector<string> tokens,map<string,float> &probs) {
   vector<string> modelFiles = getFiles(trainModel);
   map<string , float> match = checkProbsSparse(rgb,tokens,modelFiles,probs);
   return match;
}

map<string,float> testOne(vector<int> rgb,vector<string> tokens,map<string,float> &probs) {
   vector<string> modelFiles = getFiles(trainModel);
   vector<string> fs;
   vector<string> noList;
   map<string,float> match;
      for (vector<string>::iterator it = tokens.begin() ; it != tokens.end(); ++it) {
       string exta;
       exta = *it;
       boost::erase_all(exta, ",");
       string ext = exta + ".model";
       if ( std::find(modelFiles.begin(), modelFiles.end(), ext) != modelFiles.end() )
          fs.push_back(ext);
       else
          noList.push_back(exta);
    }
    match = checkProbsSparse(rgb,tokens,fs,probs);
    if(noList.size() > 0) {
     // cout << "Descriptions that are not learnt :: " << endl;
      for (vector<string>::iterator it = noList.begin() ; it != noList.end(); ++it) {
     //    cout << *it << " ";
      }
   }
   return match;
}

void test(vector<int> rgb, vector<string> tokens) {
   vector<string> modelFiles = getFiles(trainModel);
   vector<string> fs;
   vector<string> noList;
   map<string,float> match;
   map<string,float> probs;
   if(tokens.front() == "1" or tokens.front() == "") {
     match = testAll(rgb,tokens,probs);
   } else {
      for (vector<string>::iterator it = tokens.begin() ; it != tokens.end(); ++it) {
       string exta;
       exta = *it;
       boost::erase_all(exta, ","); 
       string ext = exta + ".model";
       if ( std::find(modelFiles.begin(), modelFiles.end(), ext) != modelFiles.end() )
          fs.push_back(ext);
       else
          noList.push_back(exta);
    }
    match = checkProbsSparse(rgb,tokens,fs,probs);
    if(noList.size() > 0) {
      cout << "Descriptions that are not learnt :: " << endl;
      for (vector<string>::iterator it = noList.begin() ; it != noList.end(); ++it) {
         cout << *it << " ";
      }
   }
   }

   cout << endl << endl << "Prediction Probabilities :: " << endl;
   for (auto i : probs)
       cout << i.first << " : " << i.second << " ";
   cout << endl << endl << "Matched Labels and Probabilities : ";
    for (auto i : match)
       cout << i.first << " : " << i.second << " ";

/*
   cout << endl << "Matched Labels and Probabilities : ";
   for (vector<string>::iterator it = match.begin() ; it != match.end(); ++it) {
         cout << *it << " ";
   }
*/
   cout << endl;
}

void learnModel() {
   clearArff();
   if(spFlag)
      prepareIndvArff();
   else
      prepareIndvArff1();
//   saveModel();
}

void trainFromFile(string fName) {
    fstream fs(fName,fstream::in);
    string line;
    getline(fs,line,'\0');
    vector <string> cls = split_string(line,"\n");
    int count = 0;
    for (std::vector<string>::iterator itb = cls.begin() ; itb != cls.end(); ++itb) {
           string temp;
           temp = *itb;
           if(temp != "") {
              vector <string> feature = split_string(temp,",");
	      vector<int> rgb;
	      vector<string> tokens;
	      for (unsigned ij = 0; ij < 3; ij++) {
                 if(feature[ij] != "") {
                    int digit = atoi(feature[ij].c_str());
		    rgb.push_back(digit);
                 }
	      }
           
	      for (unsigned ij = 3; ij < feature.size(); ij++) {
                 if(feature[ij] != "")
                    tokens.push_back(feature[ij]);
	      }
	      train(rgb,tokens);
              count++;
              cout << "Element " << count << endl;
              if(count % 25 == 0) {
                 int no;
                 cout << "Please enter no";
                 cin >> no;
              }
	   }
    }

}


vector<string> getToken(string temp) {
   vector <string> feature = split_string(temp,",");
   vector<string> tokens;
   for (unsigned ij = 3; ij < feature.size(); ij++) {
      if(feature[ij] != "")
         tokens.push_back(feature[ij]);
   }
   return tokens;
}
vector<string> saveFileinVector(string fName) {
   vector<string> fileVec;
   fstream fs(fName,fstream::in);
   string line;
   getline(fs,line,'\0');
   vector <string> cls = split_string(line,"\n");
   for (std::vector<string>::iterator itb = cls.begin() ; itb != cls.end(); ++itb) {
      string temp;
      temp = *itb;
      if(temp != "") {
        fileVec.push_back(temp);
      }
   }
   return fileVec;
}

vector<string> getClassifers() {
   string fName = "/home/kamala/catkin_ws/all_arff.txt";
   return saveFileinVector(fName);
}

void delTrain() {
   string fName = "/home/kamala/catkin_ws/src/jm/src/"+ trName;
   string cmd = "chmod 777 " + fName;
   int ret = system(cmd.c_str());
   cmd = "rm -rf " + fName;
//   cout << "Clearing Existing trained data ..." << endl;
   ret = system(cmd.c_str());

}
void testConf(string fName) {
    int interval = 10;
    vector<string> allClassifiers = getClassifers();
    int noCls = allClassifiers.size();
    fstream myfile;
    string resFile = "/home/kamala/catkin_ws/ConfResults/JMResults-4times.csv";
    myfile.open(resFile,ios::app);
    myfile << "Samples,";
    for(int ij=0;ij < noCls; ij++) {
          myfile << allClassifiers[ij] << ",";
    }
    myfile << "\n";
    myfile.close();
    vector<string> allDesc = saveFileinVector(fName);
    int descNo = allDesc.size();
    
    cout << "Execution is started with : " << descNo << endl;
    for (int i = interval; i <= descNo;i = i + interval) {
       cout << "interval" << i << endl;
       myfile.open(resFile,ios::app);
       vector<string> testDesc = allDesc;
       testDesc.resize(i);
       
       float confs[noCls] = {};
       int confCount[noCls] = {};
       for(int j = 0;j < i; j++) {
          delTrain();
	  string cmd = "mkdir -p " + trainFolder;
	  int ret = system(cmd.c_str());
	  cmd = "chmod -R 777 " + trainFolder;
	  ret = system(cmd.c_str());
	  ofstream outfile;
	  outfile.open(fDataFile, ios::out | ios::trunc );
	  for(int jk = 0;jk < i; jk++) {
             if(jk != j)
                outfile << testDesc[jk] << "\n";
	  }
	  outfile.close();

//          cout <<  "Data " << j << ", ";
//  << testDesc[j] <<endl;
          vector<int> primeRGB = getRGB(testDesc[j]);
          vector<string> primeTokens = getToken(testDesc[j]);   
          if(primeTokens.size() == 0)
             continue;
          
          for(int k = 0; k < i ; k++) {
             if(j == k) 
                continue;
             vector<string> tokens = getToken(testDesc[k]);
             vector<string> tks = {};
	     for (vector<string>::iterator itb = primeTokens.begin() ; itb != primeTokens.end(); ++itb) {
                string temp;
		temp = *itb;
		vector<string>::iterator it;

		it = find(tokens.begin(), tokens.end(), temp);
		if (it != tokens.end()) {
                   tks.push_back(temp);
                }
	     }
             if(tks.size() > 0) {
	        vector<int> rgb = getRGB(testDesc[k]);
		train(rgb,tks);
             }
          }
          learnModel();
	  vector<string> modelFiles = getFiles(trainModel);
          vector<string> tk = {};
	  for (vector<string>::iterator itb = primeTokens.begin() ; itb != primeTokens.end(); ++itb) {
		  string temp;
		  temp = *itb;
	  }
	  for (vector<string>::iterator itb = modelFiles.begin() ; itb != modelFiles.end(); ++itb) {
                string temp;
                temp = *itb;
                vector<string> m = {};
                m.push_back(temp);
		map<string,float> probs;
		map<string , float> match = checkProbsSparse(primeRGB,tk,m,probs);
		for (auto im : probs) {
			auto it = find(allClassifiers.begin(), allClassifiers.end(), im.first);
			auto index = distance(allClassifiers.begin(), it);
			confs[index] += im.second;
	//		if(i.second > basej)
			confCount[index]++;
		}	
	//		cout << i.first << " " << i.second << " " << confCount[index] << endl;
	  }
       }
       myfile << i << ", ";
       for(int ij=0;ij < allClassifiers.size(); ij++) {
          if(confCount[ij] != 0) {
		confs[ij] /= confCount[ij];
          }
	  myfile << confs[ij] << ",";
       }
       myfile << "\n";
       myfile.close();
       ostringstream dat;
       if(i  % 30 == 0) {
          string cmd1 = "cp -r "+ trainFolder + " " + trainFolder + "_JM";
          dat << cmd1;
          dat << i;
          cmd1 = dat.str();  
          int ret1 = system(cmd1.c_str());
       }
    }
}

void testing() {
   learnModel();
   int kData = 0;
   string fName = "/home/kamala/catkin_ws/testdata.txt";
   vector<string> tData = saveFileinVector(fName);
   for(int j = 0;j < tData.size(); j++) {
	   map<string,float> match;
	   map<string,float> probs;

      vector<int> primeRGB = getRGB(tData[j]);
      vector<string> primeTokens = getToken(tData[j]);
      match = testOne(primeRGB,primeTokens,probs);
      for (auto i : probs) {
      //   cout << i.first << " : " << i.second << " " << kData;
        if(i.second >= 0.6) {
          kData++;
       }
      }

   }
   cout << endl << "No of new visual scenes learnt " << kData;
}
