#ifndef PARSER_H
#define PARSER_H

#include "Tokenizer.h"
#include "Lexer.h"

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <map>

using namespace std;
// Struct of A View
// View: View_Name
// +----------------------------------+------------------------+----------------------+
// | Column1                          | Column2                | Column3              |
// +----------------------------------+------------------------+----------------------+
// | Span1: (l, r)                    | Span1: (l, r)          | Span1: (l, r)        |
// | Span2: (l, r)                    | Span2: (l, r)          | Span2: (l, r)        |
// | Span3: (l, r)                    | Span3: (l, r)          | Span3: (l, r)        |
// +----------------------------------+------------------------+----------------------+
struct Span
{
    string span_;
    string span_with_num;
    int left, right;
    Span(string s, int a, int b){
        Token tok;
        span_with_num = s + ": (" + tok.intToString(a) + ", " + tok.intToString(b) + ")";
        span_ = s;
        left = a;
        right = b;
    }
};

struct Column
{
    int output_length;
    //the max length of one span;
    vector<Span> column_;
    string column_name;
    Column(string name) {
        column_name = name;
        output_length = 0;
    }
    Column(const Column& c){
        output_length = c.output_length;
        column_ = c.column_;
        column_name = c.column_name;
    }
    void setName(string str){
        column_name = str;
    }
};

struct View
{
    vector<Column> view_;
    string view_name;
    View(string name){
        view_name = name;
    }
    unsigned long size(){
        return view_.size();
    }
};

class Parser {
public:
    Parser();
    Parser(Lexer* lex, Tokenizer* t, string outputName);
    ~Parser();
    //匹配保留字
    void match(int tag);
    //检测到错误，打印行列号，退出程序
    void error();
    //调用lexer的scan向后扫描下一个token
    void move();

    //开始处理
    void program();
    void aql_stmt();

    //create大分支
    void create_stmt();
    vector<Column> view_stmt();
    
    //select分支，返回select 选取的行
    vector<Column> select_stmt();
    vector < vector<string> > select_list();
    vector < vector<string> > select_other();
    vector<string> select_item();

    //extract分支与其公用部分
    vector<Column> extract_stmt();
    string alias();
    map<int, string> name_spec();
    vector<Token*> column();

    //group部分，返回group的id与列名的映射
    map<int, string> group_spec();
    map<int, string> single_group();
    map<int, string> group_other();
    //from分支，返回view或者document的真名和别名的map
    map<string,string> from_list_other();
    map<string,string> from_list();
    map<string, string> from_item();
    
    //正则提取部分，返回参与生成view的列
    vector<Column> regex_spec();
    void exec_regx(string regex);
    vector<Column> createView(map<int, string> m);


    //pattern匹配部分，返回参与生成view的列
    vector<Column> pattern_spec();
    //pattern读取部分，得到pattern字段
    vector<Token*> pattern_expr();
    vector<Token*> pattern_pkg();
    vector<Token*> expr_other();
    vector<Token*> atom();
    vector<Token*> pattern_group();
    //pattern处理部分 
    vector<Column> pattern_spec_handler(map<int, string> grouplist, int &current,  vector<Token*> vec, map<string, string> fromlist);
    vector<Column> pattern_expr_handler(map<int, string> grouplist, int &current,  vector<Token*> vec, map<string, string> fromlist);
    vector<Column> pattern_pkg_handler(map<int, string> grouplist, int &current,  vector<Token*> vec, map<string, string> fromlist);
    vector<Column> atom_handler(map<int, string> grouplist, int &current,  vector<Token*> vec, map<string, string> fromlist);
    vector<Column> pattern_group_handler(map<int, string> grouplist, int &currrent,  vector<Token*> vec, map<string, string> fromlist);
    vector<Column> expr_other_handler(map<int, string> grouplist, int &current,  vector<Token*> vec, map<string, string> fromlist);
    vector<Column> join(vector<Column> col1, vector<Column> col2, int min, int max);
    int getSpanWb(Span s);
    int getSpanWe(Span s);


    //output大分支
    void output_stmt();
    void outputView(string s1, string s2);
    vector<Column> selectFromView(vector<vector<string> >, map<string, string>);


private:
    string outputName;
    Lexer* lexer;
    Tokenizer* tokenizer;
    Token* look;
    vector<View> view_vector;
    ifstream infile;
    /* result stores a collections of pairs, which are the bound of a matched token or pattern */
    vector< vector<int> > result;
    string content;

    vector<vector<int>> capture_pos;
    int current_col_pos;
};

#endif
