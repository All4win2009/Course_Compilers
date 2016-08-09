#ifndef LEXER_H
#define LEXER_H

#include <iostream>
#include <cstdio>
#include <string>
#include <cstring>
#include <map> 
using namespace std;

class Tag{
public:
    static int const CREATE = 250, VIEW = 251, AS = 252, OUTPUT = 253,
    SELECT = 254, FROM = 255, EXTRACT = 256, REGEX = 257,
    ON = 258, RETURN = 259, AND = 260, TOKEN = 261, PATTERN = 262,
    ID = 263, NUM = 264, REG = 265, GROUP = 266, END = 267;
};

class Token{
public:
    int tag;
    int line, column;
    Token() {
        tag = line = column = 0;
    }
    Token(int t, int l, int c){
        tag = t;
        line = l;
        column = c;
    }
    string intToString(int num){
        string s = "";
        while(num > 0){
            
            s = (char)(num % 10 + '0') + s;
            
            num /= 10;
        }
        if(s == "")  s = "0";
        return s;
    }
    virtual string toString(){
        char c = tag;
        string str = ".";
        str[0] = c;
        return str;
    }
};



class Num : public Token{
public:
    int value;
    Num(int v, int l, int c):Token(Tag::NUM, l, c){
        value = v;
    }
    string toString(){
        return intToString(value);
    }
};

class Word : public Token{
public:
    string lexeme;
    Word(string s, int tag, int l, int c):Token(tag, l, c){
        lexeme = s;
    }
    string toString(){
        return lexeme;
    }
    
};

class Regex : public Token{
public:
    string content;
    Regex(string s, int tag, int l, int c): Token(tag, l, c){
        content = s;
    }
    string toString(){
        return content;
    }
};

class End : public Token {
public:
    End(int l, int c):Token(Tag::END, l, c){
    }
};

class Lexer{
public:

    Lexer();
    Lexer(string aqlFile);
    void reserve (Word* w);
    void readch();
    bool readch(char c);
    bool isLetter(char c);
    bool isDigit(char c);
    Token* scan();

private:
    FILE *file;
    int line;
    int column;
    char peek;
    map<string, Word*> words;
};

#endif