#include "Lexer.h"
#include "Parser.h"

#include <iostream>
#include <vector>
#include <set>
#include <string>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>

using namespace std;

bool isInputFile(string inputPath) {
    bool isFile = true;
    string post = ".input";
    for (int i = inputPath.length()-1, j = post.length()-1; i >= 0 && j >= 0; i--, j--) {
        if (inputPath[i] != post[j]) {
            isFile = false;
            break;
        }
    }
    return isFile;
}

string getFilePath(string inputPath) {
    string filepath;
    int i;
    for (i = inputPath.length()-1; i >= 0; i--) {
        if (inputPath[i] == '/') {
            break;
        }
    }
    for (int j = 0; j < i; j++) {
        filepath += inputPath[j];
    }
    if (i == 0) {
        filepath = ".";
    }
    filepath += '/';
    return filepath;
}

string getOutputName(string aqlFileName) {
    int index1, index2;
    string outputName = "";
    for (index1 = aqlFileName.length()-1; index1 >= 0; index1--) {
        if (aqlFileName[index1] == '.') {
            break;
        }
    }
    for (index2 = index1-1; index2 >= 0; index2 --) {
        if (aqlFileName[index2] == '/') {
            break;
        }
    }
    for(index2 += 1; index2 < index1; index2++) {
        outputName += aqlFileName[index2];
    }
    return outputName;
}

int main(int args, char *argv[]) {
    if (args != 3) {
        cout << "The paraments is not correct." << endl;
        return 0;
    }
    string aqlFile = argv[1];
    string inputPath = argv[2];
    string outputName = getOutputName(aqlFile);
    // cout << getOutputName(aqlFile) << endl;
    if (isInputFile(inputPath)) {
        string locationPath = getFilePath(inputPath);
        ofstream fileout(locationPath+outputName+".output", ios::out);
        fileout.close();
        Tokenizer *t = new Tokenizer(inputPath);
        Lexer *l = new Lexer(aqlFile);
        Parser *p = new Parser(l, t, outputName);
        p->program();
    } else {
        string locationPath = inputPath;
        ofstream fileout(locationPath+outputName+".output", ios::out);
        fileout.close();
        
        DIR *dp;
        struct dirent *dirp;
        string fname;
        if( ( dp = opendir(inputPath.c_str()) ) == NULL) {
            cout << "can not open the " << inputPath << endl;
            return 0;
        }
        while ( (dirp = readdir(dp)) != NULL) {
            fname = dirp->d_name;
            if (fname != "." && fname != "..") {
                inputPath += inputPath[inputPath.length()-1] == '/'?"":"/";
                fname = inputPath + fname;
                if (isInputFile(fname)) {
                    Tokenizer *t = new Tokenizer(fname);
                    Lexer *l = new Lexer(aqlFile);
                    Parser *p = new Parser(l, t, outputName);
                    p->program();
                }
            }
        }
        closedir(dp);
    }
}

