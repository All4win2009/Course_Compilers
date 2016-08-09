#include "Lexer.h"
#include <iostream>
#include <string>
#include <cstring>
#include <map>
#include <stdlib.h>
using namespace std;


Lexer::Lexer(){
    line = 1;
    column = 0;
    peek = ' ';
    reserve(new Word("create", Tag::CREATE, 0, 0));
    reserve(new Word("view", Tag::VIEW, 0, 0));
    reserve(new Word("as", Tag::AS, 0, 0));
    reserve(new Word("output", Tag::OUTPUT, 0, 0));
    reserve(new Word("select", Tag::SELECT, 0, 0));
    reserve(new Word("from", Tag::FROM, 0, 0));
    reserve(new Word("extract", Tag::EXTRACT, 0, 0));
    reserve(new Word("regex", Tag::REGEX, 0, 0));
    reserve(new Word("on", Tag::ON, 0, 0));
    reserve(new Word("return", Tag::RETURN, 0, 0));
    reserve(new Word("and", Tag::AND, 0, 0));
    reserve(new Word("Token", Tag::TOKEN, 0, 0));
    reserve(new Word("pattern", Tag::PATTERN, 0, 0));
    reserve(new Word("group", Tag::GROUP, 0, 0));
}

Lexer::Lexer(string aqlFile) {
    line = 1;
    column = 0;
    peek = ' ';
	file = fopen(aqlFile.c_str(), "r");
    reserve(new Word("create", Tag::CREATE, 0, 0));
    reserve(new Word("view", Tag::VIEW, 0, 0));
    reserve(new Word("as", Tag::AS, 0, 0));
    reserve(new Word("output", Tag::OUTPUT, 0, 0));
    reserve(new Word("select", Tag::SELECT, 0, 0));
    reserve(new Word("from", Tag::FROM, 0, 0));
    reserve(new Word("extract", Tag::EXTRACT, 0, 0));
    reserve(new Word("regex", Tag::REGEX, 0, 0));
    reserve(new Word("on", Tag::ON, 0, 0));
    reserve(new Word("return", Tag::RETURN, 0, 0));
    reserve(new Word("and", Tag::AND, 0, 0));
    reserve(new Word("Token", Tag::TOKEN, 0, 0));
    reserve(new Word("pattern", Tag::PATTERN, 0, 0));
    reserve(new Word("group", Tag::GROUP, 0, 0));
}

void Lexer::reserve (Word* w){
    words[w->lexeme] = w;
}
    
void Lexer::readch(){
    column++;
    peek = fgetc(file);
}
    
bool Lexer::readch(char c){
    readch();
    if(peek != c) return false;
    peek = ' ';
    return true;
}
    
bool Lexer::isLetter(char c){
    if((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')){
        return true;
    }
    return false;
}

bool Lexer::isDigit(char c) {
    if (c >= '0' && c <= '9') {
        return true;
    }
    return false;
}
    
Token* Lexer::scan(){


    for(;;readch()){
        if(peek == ' ' || peek == '\t') continue;
        else if (peek == '\n'){
            line ++;
            column = 0;
            }
        else break;
    }
    if (peek == EOF) {
        return new End(0, 0);
    }
    
    
    if(isDigit(peek)){
        int v = 0;
        do{
            v = 10*v + (peek - '0');
            readch();
        }while(isDigit(peek));
        return new Num(v, line, column);
    }

    if(peek == '/'){
        string buffer = "";
        readch();
        while(peek != '/'){
            buffer += peek;
            readch();
        }
        readch();
        string s = buffer;
        return new Regex(s, Tag::REG, line, column);
    }
    
    if( isLetter(peek) ){
        string buffer = "";
        do{
            buffer += peek;
            readch();
        } while(isLetter(peek) || isDigit(peek));
        
        string s = buffer;
        if(words.count(s) == 0){
            Word* w = new Word(s, Tag::ID, line, column);
            words[s] = w;
            return w;
        }
        else{
            return new Word(words[s]->lexeme, words[s]->tag, line, column);
        }
    }
    Token* tok = new Token(peek, line, column); peek = ' ';
    return tok;
}
