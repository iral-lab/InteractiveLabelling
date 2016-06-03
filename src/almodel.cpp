#include "almodel.h"
#include "model.h"

#include <stdlib.h>
#include <sstream>
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <string>
#include <iostream>
#include <algorithm>
#include <fstream>

using namespace std;
string tNoD = "25";
string trainFolderAL = "/home/kamala/catkin_ws/src/jm/src/Train" + tNoD;
string fDataFileAL = trainFolderAL + "/trainData"+tNoD+".txt";
string trainModelAL = trainFolderAL + "/Models";
float base = 0.0;
float threshold = 0.5;
string qnLabel = "";
vector<string> cLabels;

int qnZNo = 0;
int qnANo = 0;
const string qnA[7] = {
"I am not certain about this object. Could you please type the description for me?",
"Would you describe the object using keyboard, please?",
"Can you type what object is this , please",
"I would be grateful if you could enter the information about this object",
"I could not remember this object. Could you please label it using keyboard",
"Would you please write definition of this object ",
"Could you enter the description, please"
};
string qnBa = "It looks like ";
string qnBb = "Is that right ?. Please type 'yes' or 'no'";

string qnC = "Okay. Then, Could you please enter the description?";
const string qnZ[10] = {
"I appreciate your help",
"Thank You",
"Many thanks for the response",
"Thanks a lot",
"Thank you for spending time with me. I do appreciate it",
"Thanks for your guidance and support",
"Many thanks for your assistance",
"Thank you. You're so helpful",
"Thank you for being patient and helping me to improve",
"I wholeheartedly appreciate all your help."};

bool qnBlank = false;


int altrain(vector<int> rgb, vector<string> tokens,int qnType) {
   qnBlank = false;

   if(qnType == 0) {
      learnModel();
      vector<string> labels;
      map<string,float> probs;
      map<string,float> match = testAll(rgb,tokens,probs);

// display matched and predicted probabilities
      cout << endl << endl << "Prediction Probabilities :: " << endl;
      for (auto i : probs)
         cout << i.first << " : " << i.second << " ";
      cout << endl << endl << "Matched Labels and Probabilities : ";
      for (auto i : match)
         cout << i.first << " : " << i.second << " ";

      for (auto i : match) {
         if (i.second >= threshold) 
            labels.push_back(i.first);
      }  
      cLabels = labels;
// If any object matched above some threshold with current object,
// verifiying with user 'is this 'that' object? '

      if(labels.size() > 0) {
         string qn  = qnBa;
          int c = 0;
          for (std::vector<string>::iterator it = labels.begin() ; it != labels.end(); ++it) {
               string lb;
               lb = *it;
               qn += " " + lb;
               c++;
               if(c == labels.size() - 1)
                  qn += " or ";
               else if(c > 0)
                  qn += ", ";

         }
         qn += qnBb; 
         qn += "?";
         //string i = labels.front();
         //std::cout << i << ' '; 
         qnLabel = qn;
                 
      } else {
// if the object is new or 
//properties of current one did not match with any exisiting one,
// ask the user to describe the object
         qnBlank = true;

         int qnAMod = qnANo % 7;
         qnLabel = qnA[qnAMod];
         qnANo++;
         
    //     cout << "What is this ?" ;
     }
     cout << endl << qnLabel << endl;
     string cmd = "echo \""+ qnLabel +"\"  |festival --tts";
//     int ret = system(cmd.c_str());

     return 1;
   } else if(qnType == 1) {
//      if(tokens.size() == 0) {
      //   cout << "What is this,then ?" ;
//         qnLabel = qnC;
//	 return 1;
//      }
      char c;
      string tk = tokens.front();
//      for(int i = 0;tk[i];i++) 
//         tk[i] = toupper(tk[i]);
      for (auto & c: tk) c = toupper(c);
      if(tk != "YES") {
         if(tk == "NO") {
      //      vector<string> tks{qnLabel};
           vector<string> tks;
           for(size_t i = 1; i != tokens.size(); i++) {
              tks.push_back(tokens[i]);

	   }
            
            qnLabel = qnC;
            cout << endl << qnLabel << endl;
	    string cmd = "echo \""+ qnLabel +"\"  |festival --tts";
//	    int ret = system(cmd.c_str());
            qnBlank = true;
            return 1;
         } else 
            train(rgb,tokens);
            int qnZMod = qnZNo % 10;
	    string cmd = "echo \""+ qnZ[qnZMod] +"\"  |festival --tts";
//	    int ret = system(cmd.c_str());
            qnZNo++;

      } else {
         vector<string> tks;
         for(size_t i = 1; i != tokens.size(); i++) {
            tks.push_back(tokens[i]);
        
         }
         train(rgb,tks);
         int qnZMod = qnZNo % 10;
	 string cmd = "echo \""+ qnZ[qnZMod] +"\"  |festival --tts";
//	 int ret = system(cmd.c_str());
	 qnZNo++;

      }

      return 0;     
   }
}

//function that calls test function in Normal traditional method
void altest(vector<int> rgb, vector<string> tokens) {
   test(rgb,tokens);
}

//function that trains using the RGB , description pair from a file
// Same as traditional method

void altrainFromFile1(string fName) {
   trainFromFile(fName);
}

// function that trains using RGB,description pair from a file
// in interative way. 

void altrainFromFile(string fName) {

    fstream fs(fName,fstream::in);
    string line;
    getline(fs,line,'\0');
    vector <string> cls = split_string(line,"\n");
    int totalCount = 0;
    int yesCount = 0;
    for (std::vector<string>::iterator itb = cls.begin() ; itb != cls.end(); ++itb) {
           string temp;
           temp = *itb;
           if(temp != "") {
	      cLabels = {};
              totalCount++;
              vector <string> feature = split_string(temp,",");
              vector<int> rgb;
              vector<string> tokensP;
              for (unsigned ij = 0; ij < 3; ij++) {
                 if(feature[ij] != "") {
                    int digit = atoi(feature[ij].c_str());
                    rgb.push_back(digit);
                 }
              }

              for (unsigned ij = 3; ij < feature.size(); ij++) {
                 if(feature[ij] != "")
                    tokensP.push_back(feature[ij]);
              }
	      cout << temp << endl;

            string str = "Please place new object and press 'Enter'.. ";
            cout << str << endl << ">>";
      	    string input;
            int anType = 1;
            int qnType = 0;
	    while (anType > 0 ) {
      	       getline(cin,input);
               string annt = input;
                for(int i = 0;annt[i];i++)
                   annt[i] = toupper(annt[i]);
                   vector<string> tokens;
		if (annt.find("EXIT") != string::npos) {
			exit(0);
                } else if (annt.find("YES") != string::npos) {
                   tokens.push_back("yes");
		   yesCount++;
		   for (unsigned ij = 0; ij < tokensP.size(); ij++) {
			tokens.push_back(tokensP[ij]);
		   }		   
                } else if(annt.find("NO") != string::npos) {
                   
                   tokens.push_back("no");
                   for (unsigned ij = 0; ij < cLabels.size(); ij++) {
                      int found = 0;
                      for (unsigned ik = 0; ik < tokensP.size(); ik++) {
                         if(tokensP[ik] == cLabels[ij]) {
                             found = 1;
                         }
                      }
                      if(found == 0) {
                         tokens.push_back(cLabels[ij]);
                      }    
		   }
                } else
                   tokens= tokensP;
		cout << "Ans : ";
		for (unsigned ij = 0; ij < tokens.size(); ij++) {
			cout << tokens[ij] << " ";
		}
		cout << endl;
		anType = altrain(rgb,tokens,qnType);
                qnType = anType;

     //         train(rgb,tokens);
	    }
		cout << "Count "<< totalCount << " , Yes Count " << yesCount << endl;
	   }

    }
    cout << "Count "<< totalCount << " , Yes Count " << yesCount << endl;

}
// function that creates RGB values(vector of integers ) 
//using first 3 digits of the string
vector<int> getRGBAL(string temp) {
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
// function that creates description values(vector of strings ) 
//using all strings except first 3

vector<string> getTokenAL(string temp) {
   vector <string> feature = split_string(temp,",");
   vector<string> tokens;
   for (unsigned ij = 3; ij < feature.size(); ij++) {
      if(feature[ij] != "")
         tokens.push_back(feature[ij]);
   }
   return tokens;
}
//function that stores each line in a file as an element in a vector

vector<string> saveFileinVectorAL(string fName) {
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

vector<string> getClassifersAL() {
   string fName = "/home/kamala/catkin_ws/all_arff.txt";
   return saveFileinVectorAL(fName);
}

// function that deletes existing trained models, arff files

void delTrainAL() {
   string fName = "/home/kamala/catkin_ws/src/jm/src/Train" + tNoD;
   string cmd = "chmod 777 " + fName;
   int ret = system(cmd.c_str());
   cmd = "rm -rf " + fName;
   ret = system(cmd.c_str());

}
// A functin that train automatically, not interactively.
// It train positive and negaive samples automatically after comparing 
// the right labels and the labels that are matched by previously learned model.
void trickaltest(vector<int> rgb,vector<string> tokensP) {
      learnModel();
      vector<string> labels;
      map<string,float> probs;
      vector<string> colors = {"red","yellow","black","blue","orange","purple","green"};
      map<string,float> match = testAll(rgb,tokensP,probs);
      for (auto i : probs) {
         if (i.second >= threshold) {
               vector<string>::iterator it;
	       it = find(tokensP.begin(), tokensP.end(), i.first);
	       if (it == tokensP.end()) {
		       labels.push_back(i.first);
	       }
               it = find(colors.begin(), colors.end(), i.first);
               if (it == colors.end()) {
                       labels.push_back(i.first);
               }
         }
      }

      train(rgb,tokensP);

      addTokens(rgb, labels,false,trainFolderAL); 

}

// function that train automatically , but in interactive fashion
// it generates 'yes' or 'no' answers automatically after comparing the right tokens and the matched tokens

void trickaltest1(vector<int> rgb,vector<string> tokensP) {
   string input;
   int anType = 1;
   int qnType = 0;
   input = "";
   vector<string> uselessLabels = {};
   vector<string> colors = {"red","yellow","black","blue","orange","purple","green"};
   while (anType > 0 ) {
      string annt = input;
      for(int i = 0;annt[i];i++)
	      annt[i] = toupper(annt[i]);
      vector<string> tokens;
      if (annt.find("YES") != string::npos) {
	      tokens.push_back("yes");
	      for (unsigned ij = 0; ij < tokensP.size(); ij++) {
		      tokens.push_back(tokensP[ij]);
	      }		   
      } else if(annt.find("NO") != string::npos) {

	      tokens.push_back("no");
	      for (unsigned ij = 0; ij < cLabels.size(); ij++) {
		      int found = 0;
		      for (unsigned ik = 0; ik < tokensP.size(); ik++) {
			      if(tokensP[ik] == cLabels[ij]) {
				      found = 1;
			      }
		      }
		      if(found == 0) {
			      tokens.push_back(cLabels[ij]);
		      }    
	      }
              for (unsigned ij = 0; ij < uselessLabels.size(); ij++) {
                 tokens.push_back(uselessLabels[ij]);
              }
      } else
	      tokens= tokensP;
      cout << "Tokens : ";
      for (unsigned ij = 0; ij < tokens.size(); ij++) {
	      cout << tokens[ij] << " ";
      }
      cout << endl;
      anType = altrain(rgb,tokens,qnType);
      qnType = anType;
      input = "";
      uselessLabels = {};
      if(qnBlank == true) 
         input = "";
      else {
      
      int noColors = 0;
      int found = 0;
      if(cLabels.size() > 1) {
      for (unsigned ij = 0; ij < cLabels.size(); ij++) {
         bool lbfnd = false;
         for (unsigned ik = 0; ik < colors.size(); ik++) {
            if(colors[ik] == cLabels[ij]) {
              lbfnd = true;
               noColors++;
               for (unsigned il = 0; il < tokensP.size(); il++) {
                  if(cLabels[ij] == tokensP[il]) {
                     found = 1;
                  }
               }
            }
         }
         if(!lbfnd)
            uselessLabels.push_back(cLabels[ij]);
 
                 
      }
      }
      if(noColors == 1 && found == 1)
         input = "yes";
      else 
         input = "no";
    }  
   }

}

// A function that does cross validation at intervals 10.
// It takes hours and hours for execution
// it takes one element for testing and 9 element for training,
// incase of total 10 elements. Like that it does for all 10 elements in that group
// It does the same kind of evaluation for 10,20,30....240 elements.
void testConfAL(string fName) {
    int interval = 10;
    vector<string> allClassifiers = getClassifersAL();
//    allClassifiers = {"red","yellow","black","blue","orange","purple","green"};
  
    int noCls = allClassifiers.size();
    fstream myfile;
    string resFName = "/home/kamala/catkin_ws/ConfResults/ALResults-lessneg-newrun.csv";
    myfile.open(resFName,ios::app);
    myfile << "Samples,";
    for(int ij=0;ij < noCls; ij++) {
          myfile << allClassifiers[ij] << ",";
    }
    myfile << "\n";
    myfile.close();
    vector<string> allDesc = saveFileinVectorAL(fName);
    int descNo = allDesc.size();
 //   descNo = 30;
    cout << "Execution is started with : " << descNo << endl;

    for (int i = 120; i <= descNo;i = i + interval) {
       myfile.open(resFName,ios::app);
       cout << "interval" << i << endl;  
       vector<string> testDesc = allDesc;
       testDesc.resize(i);
       float confs[noCls] = {};
       int confCount[noCls] = {};
       for(int j = 0;j < i; j++) {
          delTrainAL();
          string cmd = "mkdir -p " + trainFolderAL;
          int ret = system(cmd.c_str());
          cmd = "chmod -R 777 " + trainFolderAL;
          ret = system(cmd.c_str());
          ofstream outfile;
          outfile.open(fDataFileAL, ios::out | ios::trunc );
          for(int jk = 0;jk < i; jk++) {
             if(jk != j)
                outfile << testDesc[jk] << "\n";
          }
          outfile.close();

          vector<int> primeRGB = getRGBAL(testDesc[j]);
          vector<string> primeTokens = getTokenAL(testDesc[j]);     
          if(primeTokens.size() == 0)
             continue;    
          for(int k = 0; k < i ; k++) {
             if(j == k) 
                continue;
             vector<string> tokens = getTokenAL(testDesc[k]);
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
	        vector<int> rgb = getRGBAL(testDesc[k]);
		trickaltest(rgb,tks);
		//train(rgb,tks);
             }
          }
          learnModel();
	  vector<string> modelFiles = getFiles(trainModelAL);
          vector<string> tk = {};
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
			confCount[index]++;
			
		}
	  }
       }
       myfile << i << ", ";
       for(int ij=0;ij < allClassifiers.size(); ij++) {
          if(confCount[ij] != 0)
		confs[ij] /= confCount[ij];
	  myfile << confs[ij] << ",";
       }
       myfile << "\n";
       myfile.close();
      ostringstream dat;
       if(i  % 30 == 0) {
          string cmd1 = "cp -r "+ trainFolderAL + " " + trainFolderAL + "_AL";
          dat << cmd1;
          dat << i;
          cmd1 = dat.str();
          int ret1 = system(cmd1.c_str());
       }

      }
}
