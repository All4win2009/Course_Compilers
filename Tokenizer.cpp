#include "Tokenizer.h"

#include <iostream>
#include <string>
#include <cstdio>
#include <dirent.h>
using namespace std;

Tokenizer::Tokenizer() {}

Tokenizer::Tokenizer(string file_path) {
    this->file_path = file_path;
}

Tokenizer::~Tokenizer() {}

void Tokenizer::run() {
    content = "";
    textTokens.clear();
    FILE *file;
    char c;
    int start = 0, end = 0, index = -1;
    string str = "";

    file = fopen(file_path.c_str(), "r");
    if (!file) {
        cout << "file error" << endl;
        return;
    }
    do {
        c = fgetc(file);
        if (c != EOF) {
            content += c;
        }
        index++;
        if (c == '\t' || c == '\n' || c == '\r' || c == ' ') {
            if (str != "") {
                TextToken t = TextToken(str, start, end);
                textTokens.push_back(t);
                str = "";
            }
            start = index+1;
            end = start;
            continue;
        }
        if (isDigit(c) || isLetter(c)) {
            str += c;
            end ++;
        } else if (c != EOF) {
            if (str != "") {
                TextToken t = TextToken(str, start, end);
                textTokens.push_back(t);
                start = index+1;
                end = start;
                str = "";
            }
            str += c;
            TextToken t = TextToken(str, index, index+1);
            textTokens.push_back(t);
            str = "";
            start = index+1;
            end = start;
        } else if (c == EOF) {
            if (str != "") {
                TextToken t = TextToken(str, start, end);
                textTokens.push_back(t);
                start = index+1;
                end = start;
                str = "";
            }
        }
    } while (c != EOF);
}

bool Tokenizer::isDigit(const char c) {
    if (c >= '0' && c <= '9') {
        return true;
    }
    return false;
}

bool Tokenizer::isLetter(const char c) {
    if ( (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ) {
        return true;
    }
    return false;
}

void Tokenizer::setFilePath(string file_path) {
    this->file_path = file_path;
}

string Tokenizer::getFilePath() {
    return file_path;
}

string Tokenizer::getLocationPath() {
    string locationPath;
    int i;
    for (i = file_path.length()-1; i >= 0; i--) {
        if (file_path[i] == '/') {
            break;
        }
    }
    for (int j = 0; j < i; j++) {
        locationPath += file_path[j];
    }
    if (i == 0) {
        locationPath = ".";
    }
    locationPath += '/';
    return locationPath;
}

string Tokenizer::getContent() {
    return content;
}

vector<TextToken> Tokenizer::getTextTokens() {
    return textTokens;
}
// main function for test
// int main(int args, char *argv[]) {
//     Tokenizer t = Tokenizer();
//     t.setFilePath("../dataset/Perc/was.input");
//     t.run();
//     // cout << t.getContent() << endl;
//     std::vector<TextToken> ts;
//     ts = t.getTextTokens();
//     for (int i = 0; i < ts.size(); i++) {
//         cout << ts[i].textToken << "(" << ts[i].start << "," << ts[i].end << ")" << endl;
//     }
// }

// main function for test 
// int main(int args, char *argv[]) {
//     if (args != 2) {
//         cout << "a path argument is needed" << endl;
//         return 0;
//     }
//     bool isFile = true;
//     string post = ".input";
//     string path = argv[1];

//     Tokenizer t = Tokenizer();

//     for (int i = path.length()-1, j = post.length()-1; i >= 0, j >= 0; i--, j--) {
//         if (path[i] != post[j]) {
//             isFile = false;
//             break;
//         }
//     }
//     if (isFile) {
//         t.setFilePath(path);
//         t.run();
//         vector<TextToken> ts = t.getTextTokens();
//         for (int i = 0; i < ts.size(); i++) {
//             cout << ts[i].textToken << " (" << ts[i].start << "," << ts[i].end << ")" << endl;
//         }
//     } else {
//         DIR *dp;
//         struct dirent *dirp;
//         string fname;
//         if( ( dp = opendir(path.c_str()) ) == NULL) {
//             cout << "can not open the " << path << endl;
//             return 0;
//         }
//         vector<TextToken> ts;
//         while ( (dirp = readdir(dp)) != NULL) {
//             fname = dirp->d_name;
//             if (fname != "." && fname != "..") {
//                 path += path[path.length()-1] == '/'?"":"/";
//                 fname = path + fname;
//                 t.setFilePath(fname);
//                 t.run();
//                 vector<TextToken> v = t.getTextTokens();
//                 ts.insert(ts.end(), v.begin(), v.end());
//             }
//         }
//         for (int i = 0; i < ts.size(); i++) {
//             cout << ts[i].textToken << " (" << ts[i].start << "," << ts[i].end << ")" << endl;
//         }
//         closedir(dp);
//     }
// }
