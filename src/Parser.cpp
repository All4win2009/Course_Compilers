#include "Parser.h"
#include "regex.cpp"

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <iomanip>
using namespace std;

Parser::Parser() {}

Parser::Parser(Lexer* lex, Tokenizer* t, string outputName){
    ofstream outfile;
    string locationPath = t->getLocationPath();
    outfile.open(locationPath+outputName+".output", ios::app);
    string filePath = t->getFilePath();
    outfile << "Processing "+ filePath + "\n";
    cout << "Processing "+ filePath + "\n";
    outfile.close();
    this->outputName = outputName;
    this->lexer = lex;
    this->tokenizer = t;
    this->look = lexer->scan();
}

Parser::~Parser() {}

void Parser::match(int t){
    if (look->tag == t){
        move();
    }
    else{
        error();
    }
}

void Parser::error(){
    cout <<"An error has occured in line: "<< look->line << " column: " << look->column
    <<"  '" <<look->toString() <<"'" <<endl;
    exit(1);
}

void Parser::move(){
    this->look = lexer->scan();
    if (look->tag == Tag::END) { }
}

void Parser::program(){
    while(look->tag != Tag::END){
        this->aql_stmt();
    }
}

void Parser::aql_stmt(){
    if (look->tag == Tag::END) {
        exit(1);   
    }    
    if( this->look->tag == Tag::CREATE ){
        this->create_stmt();
    }
    else if (this->look->tag == Tag::OUTPUT) {
        this->output_stmt();
    }
    this->match(';');
}

void Parser::create_stmt(){
    this->match(Tag::CREATE);
    this->match(Tag::VIEW);
    string ID_name = this->look->toString();
    this->move();
    this->match(Tag::AS);
    vector<Column> view_columns = this->view_stmt();
    View v = View(ID_name);
    v.view_ = view_columns;
    this->view_vector.push_back(v);
}

vector<Column> Parser::view_stmt(){
    if (look->tag == Tag::EXTRACT){
        return this->extract_stmt();
    }
    else{
        return this->select_stmt();
    }
}

vector<Column> Parser::extract_stmt(){
    this->match(Tag::EXTRACT);
    if (this->look->tag == Tag::REGEX) {
        return regex_spec();
    }
    else if (this->look->tag == Tag::PATTERN){
        return pattern_spec();
    }
    else{
        error();
        exit(1);
    }
}

vector<Column> Parser::regex_spec(){
    this->match(Tag::REGEX);
    string regex_str = this->look->toString();
    move();
    match(Tag::ON);
    vector<Token*> col = this->column();
    map<int, string> group_map = this->name_spec();
    match(Tag::FROM);
    map<string, string> m = this->from_list();
    exec_regx(regex_str);
    return createView(group_map); 
}

vector<Token*> Parser::column(){
    vector<Token*> vec;
    vec.push_back(look);
    move();
    vec.push_back(look);
    move();
    vec.push_back(look);
    move();
    return vec;
}

map<int, string> Parser::name_spec(){
    if (this->look->tag == Tag::AS) {
        match(Tag::AS);
        map<int, string> m;
        m[0] = this->look->toString();
        move();
        return m;
    }
    else if(this->look->tag == Tag::RETURN){
        match(Tag::RETURN);
        return group_spec();
    }
    else{
        error();
        exit(1);
    }
}

map<int, string> Parser::group_spec(){
    map<int, string> m =  this->single_group();
    map<int, string> n =  this->group_other();
    for (map<int, string>::iterator it = n.begin(); it!= n.end(); ++it) {
        m[it->first] = it->second;
    }
    return m;
}

map<int, string> Parser::single_group(){
    map<int, string> res;
    match(Tag::GROUP);
    int index = ((Num*)look)->value;
    move();
    match(Tag::AS);
    string name = this->look->toString();
    move();
    res[index] = name;
    return  res;
}
map<int, string> Parser::group_other(){
    map<int, string> m;
    if(this->look->tag == Tag::AND){
        match(Tag::AND);
        m = this->single_group();
        map<int, string> n = this->group_other();
        for (map<int, string>::iterator it = n.begin(); it!= n.end(); ++it) {
            m[it->first] = it->second;
        }
    }
    return m;
    
}
vector<Column> Parser::pattern_spec(){
    this->match(Tag::PATTERN);
    vector<Token*> temp_vec;
    
    while (look->tag != Tag::RETURN) {
        Token t = *look;
        temp_vec.push_back(look);
        move();
    }
    //temp_vec = this->pattern_expr();
    map<int, string> group_map = this->name_spec();
    match(Tag::FROM);
    map<string, string> from_list= this->from_list();
    
    int current =  0;
    capture_pos.clear();
    current_col_pos = 0;
    vector<int> holder;
    capture_pos.push_back(holder);
    vector<Column> col = pattern_spec_handler(group_map, current, temp_vec, from_list);
    capture_pos[0].push_back(0);
    capture_pos[0].push_back((int)col.size());
    //--------------------------------
    
    vector<Column> final_res;
    for (int i = 0; i < capture_pos.size(); i++) {
        Column cap_ = Column(group_map[i]);
        int begin = capture_pos[i][0];
        int end = capture_pos[i][1];
        for (int row = 0; row < col[0].column_.size(); row++) {
            int left_index = col[begin].column_[row].left;
            int right_index = col[end - 1].column_[row].right;
            string temp_str = "";
            for (int x = left_index; x < right_index; x++) {
                temp_str += content[x];
            }
            cap_.column_.push_back(Span(temp_str, left_index, right_index));
        }
        final_res.push_back(cap_);
    }
    return final_res;
    
}

vector<Token*> Parser::pattern_expr(){
    vector<Token*> vec = this->pattern_pkg();
    vector<Token*> vec1 = this->expr_other();
    
    vec.insert(vec.end(), vec1.begin(), vec1.end());
    return vec;
}


vector<Token*> Parser::pattern_pkg(){
    vector<Token*> vec;
    if (this->look->tag == '<' || this->look->tag == Tag::REG) {
        vec = this->atom();
        if (this->look->tag == '{') {
            vec.push_back(look);
            move();
            if (this->look->tag == Tag::NUM) {
                vec.push_back(look);
                move();
            }
            else{
                error();
            }
            if (this->look->tag == ',') {
                vec.push_back(look);
                move();
            }
            else{
                error();
            }
            if (this->look->tag == Tag::NUM) {
                vec.push_back(look);
                move();
            }
            else{
                error();
            }
            if (this->look->tag == '}') {
                vec.push_back(look);
                move();
            }
            else{
                error();
            }
        }
    }else if (this->look->tag == '('){
        vec = this->pattern_group();
    }
    else{
        error();
    }
    return vec;
}

vector<Token*> Parser::atom(){
    vector<Token*> v;
    if (this->look->tag == '<') {
        v.push_back(look);
        move();
        if (this->look->tag == Tag::TOKEN) {
            v.push_back(look);
            move();
        }
        else if (this->look->tag == Tag::ID){
            vector<Token*> temp = this->column();
            v.insert(v.end(), temp.begin(), temp.end());
        }
        else{
            error();
        }
        v.push_back(look);
        move();
    }
    else if (look->tag == Tag::REG){
        v.push_back(look);
        move();
    }
    return v;
}
vector<Token*> Parser::pattern_group(){
    vector<Token*> vec;
    if(look->tag == '('){
        vec.push_back(look);
        move();
    }
    else{
        error();
    }
    vector<Token*> vec2 = pattern_expr();
    vec.insert(vec.end(), vec2.begin(), vec2.end());
    if(look->tag == ')'){
        vec.push_back(look);
        move();
    }
    else{
        error();
    }
    return vec;
}
vector<Token*> Parser::expr_other(){
    vector<Token*> vec;
    if (this->look->tag == '(' || this->look->tag == '<' || this->look->tag == Tag::REG) {
        vec = this->pattern_pkg();
        vector<Token*> vec1 = this->expr_other();
        vec.insert(vec.end(), vec1.begin(), vec1.end());
    }
    return vec;
}


vector<Column> Parser::select_stmt(){

    match(Tag::SELECT);

    vector < vector<string> > columnNames = select_list();

    match(Tag::FROM);
    map<string, string> views = from_list();
    for(map<string, string>:: iterator it = views.begin(); it != views.end(); it++){
    }
    return selectFromView(columnNames, views);
    //todo
}

vector < vector<string> > Parser::select_list(){
    
    vector < vector<string> > items;
    items.push_back(select_item());
    vector < vector<string> > temp = select_other();
    items.insert(items.end(), temp.begin(), temp.end());
    
    return items;
}

vector < vector<string> > Parser::select_other() {
    vector < vector<string> > items;
    // Match the ',' in case that there are more select lists.
    
    if(look->tag == ','){
        match(',');
        items.push_back(select_item());
        vector < vector<string> > temp = select_other();
        items.insert(items.end(), temp.begin(), temp.end());
    }
    return items;
}

vector<string> Parser::select_item(){
    vector<string> item;
    item.push_back(look->toString());
    move();
    match('.');
    item.push_back(look->toString());
    move();
    string alias_name_  = this->alias();
    if (alias_name_ == "")
    {
        alias_name_ = item[1];
    }
    item.push_back(alias_name_);
    return item;
}



map<string, string> Parser::from_item(){
    string viewName = look->toString();
    move();
    string name = look->toString();
    move();
    map<string, string> m;
    m[name] = viewName;
    return m;
}

map<string,string> Parser::from_list(){
    
    map<string,string> list = from_item();
    map<string,string> list2 = from_list_other();
    for(map<string, string>:: iterator it = list2.begin(); it != list2.end(); it++){
        list[it->first] = it->second;
    }
    return list;
}

map<string,string> Parser::from_list_other(){
    
    map<string,string> list;
    if(look->tag == ','){
        match(',');
        list = from_item();
        map<string,string> list2 = from_list_other();
        for(map<string, string>:: iterator it = list2.begin(); it != list2.end(); it++){
            list[it->first] = it->second;
        }
    }
    return list;
}

void Parser::output_stmt(){
    this->match(Tag::OUTPUT);
    this->match(Tag::VIEW);
    string out_name = this->look->toString();
    this->move();
    string alias_name = this->alias();
    outputView(out_name, alias_name);
}

string Parser::alias(){
    string res;
    if (this->look->tag == Tag::AS)
    {
        this->match(Tag::AS);
        res = this->look->toString();
        this->move();
        return res;
    }
    else{
        res = "";
        return res;
    }
}

/* Output a view in a uniform format */
void Parser::outputView(string out_name, string alias_name) {
    string locationPath = tokenizer->getLocationPath();
    ofstream outfile;
    outfile.open(locationPath+outputName+".output", ios::app);
    vector<View>::iterator it;
    for (it = view_vector.begin(); it != view_vector.end(); it++) {
        // Find the target View
        if (it->view_name == out_name) {
            // viewName is the view name to be printed, it depends on the two parameter strings
            string viewName;
            if (alias_name == "")
                viewName = out_name;
            else
                viewName = alias_name;
            outfile << "\nView: " << viewName << endl;
            cout << "\nView: " << viewName << endl;
            vector<Column> cols = it->view_;
            // Output each column
            int columnNumber = cols.size();
            // Output below part based on the column's width
            /*
            +--------------------+--------------------------------+------------------------------------------------+
            */
            string firstPrintedRow = "";
            for (int i = 0; i < columnNumber; i++) {
                firstPrintedRow += "+";
                /* Iterate all spans and get the column's max width */
                vector<Span> spans = cols[i].column_;
                int spansSize = spans.size();
                for (int index = 0; index < spansSize; index++) {
                    if (spans[index].span_with_num.size() > cols[i].output_length) {
                        cols[i].output_length = spans[index].span_with_num.size();
                    }
                }

                int outputLength = cols[i].output_length;
                //int outputLength = 20;
                // outfile << "outputLength = " << outputLength << endl;
                for (int j = 0; j <= outputLength+1; j++) {
                    firstPrintedRow += '-';
                }
            }
            firstPrintedRow += "+\n";
            // cout << "firstPrintedRow: " << firstPrintedRow << endl;
            outfile << firstPrintedRow;
            cout << firstPrintedRow;
            
            /*
            | Loc                | Per                            | PerLoc                                         |
            */
            int outputLength = 0;
            for (int i = 0; i < columnNumber; i++) {
                cout << "| ";
                outfile << "| ";
                //outputLength = 20;

                outputLength = cols[i].output_length;
                cout << setiosflags(ios::left) << setw(outputLength+1) << cols[i].column_name;
                outfile << setiosflags(ios::left) << setw(outputLength+1) << cols[i].column_name;
            }
            cout << "|\n";
            outfile << "|\n";
            // Output below part based on the column's width
            /*
            +--------------------+--------------------------------+------------------------------------------------+
            */
            cout << firstPrintedRow;
            outfile << firstPrintedRow;
            // Output below part based on the column's width
            /*
            | Carter:(0,6)       | Plains, Georgia:(12,27)        | Carter from Plains, Georgia:(0,27)             |
            | Plains:(12,18)     | Georgia, Washington:(20,39)    | Plains, Georgia, Washington:(12,39)            |
            | Washington:(29,39) | Westmoreland, Virginia:(45,67) | Washington from Westmoreland, Virginia:(29,67) |
            */
            // Find the max row of each column since we should at least print that much rows.
            int max_row = 0;
            for (int i = 0; i < columnNumber; i++) {
                if (cols[i].column_.size() > max_row)
                    max_row = cols[i].column_.size();
            }
            
            int k = 0;
            for (int j = 0; j < max_row; j++) {
                
                for (int i = 0; i < columnNumber; i++) {
                    /* extraEmptyLines used as a flag to print some extra empty rows */
                    bool extraEmptyLines = false;
                    outputLength = cols[i].output_length;
                    vector<Span> rows = cols[i].column_;
                    int rowNumber = rows.size();
                    //outfile << "rowNumber : " << rowNumber << endl;
                    cout << "| ";
                    outfile << "| ";
                    if (k < rowNumber) {
                        cout << setiosflags(ios::left) << setw(outputLength+1) << rows[k].span_with_num;
                        outfile << setiosflags(ios::left) << setw(outputLength+1) << rows[k].span_with_num;
                    } else if (k <= max_row) {
                        extraEmptyLines = true;
                    }
                    if (extraEmptyLines == true) {
                        cout << setiosflags(ios::left) << setw(outputLength+1) << " ";
                        outfile << setiosflags(ios::left) << setw(outputLength+1) << " ";
                    }
                }
                k++;

                cout << "|\n";
                outfile << "|\n";
            }
            // Output below part based on the column's width
            /*
            +--------------------+--------------------------------+------------------------------------------------+
            */
            cout << firstPrintedRow;
            outfile << firstPrintedRow;

            // Output below part based on the column's width
            /*
            6 rows in set
            */
            cout << max_row << " rows in set\n";
            outfile << max_row << " rows in set\n";
        }

    }
}

/*
vector<Column> Parser::select_stmt(){
    match(Tag::SELECT);
    // innerest has 3 strings: view's alias name, column's name, column's alias
    vector < vector<string> > columnNames = select_list();
    match(Tag::FROM);
    // s1 is alias, s2 is real name
    map<string, string> views = from_item();
    return selectFromView(columnNames, views);
}*/


/* Added by lsq */
vector<Column> Parser::selectFromView(vector<vector<string> > columnNames, map<string, string> views) {
    vector<Column> returnVector;
    /* Operate on each View */
    string aliasView;
    string realView;
    /* Iterator each column after the AQL statement "SELECT" */
    for (int i = 0; i < columnNames.size(); i++) {
        aliasView = columnNames[i][0];
        /* Extract the target View name */
        realView = views[aliasView];
        
        for (int j = 0; j < view_vector.size(); j++) {
            if (view_vector[j].view_name == realView) {
                /* Extract the target column */
                vector<Column>::iterator it2 = view_vector[j].view_.begin();
                for (; it2 != view_vector[j].view_.end(); it2++) {
                    /* columnNames[i][1] is the selected column name */
                    if (it2->column_name == columnNames[i][1]) {
                        Column c = *it2;
                        /* Whether the column has a alias name. */
                        if (columnNames[i][2] != "") { 
                            c.column_name = columnNames[i][2];
                        }
                        else {
                            c.column_name = columnNames[i][1];
                        }
                        returnVector.push_back(c);
                    }
                }
            }
        }
    }
    return returnVector;
}

void Parser::exec_regx(string regex) {
    tokenizer->run();
    vector<TextToken> v = tokenizer->getTextTokens();
    content = tokenizer->getContent();
    result = findall(regex.c_str(), content.c_str());
}

vector<Column> Parser::createView(map<int, string> m) {
    map<int, string>::iterator m_it;
    vector<Column> r;
    for (m_it = m.begin(); m_it != m.end(); m_it++) {
        Column c = Column(m_it->second);
        string span_value;
        for (int i = 0; i < result.size(); i++) {
            int index = m_it->first*2;
            span_value = "";
            for (int j = result[i][index]; j < result[i][index+1]; j++) {
                span_value += content[j];
            }
            Span s = Span(span_value, result[i][index], result[i][index+1]);
            c.column_.push_back(s);
            if (s.span_with_num.size() > c.output_length){
                c.output_length = s.span_with_num.size();
            }
        }
        r.push_back(c);
    }
    return r;
}

vector<Column> Parser::join(vector<Column> cols1, vector<Column> cols2, int min, int max) {
    vector<Column> vs;
    for (int i = 0; i < cols1.size()+cols2.size()+1; i++) {
        vs.push_back(Column(""));
    }
    vector<TextToken> v = tokenizer->getTextTokens();
    
    int b1, e1, b2, e2, wb1, we1, wb2, we2;
    if (min == 0 && max == 0) {
        for (int i = 0; i < cols1[0].column_.size(); i++) {
            b1 = cols1[0].column_[i].left;
            e1 = cols1[cols1.size()-1].column_[i].right;
            wb1 = getSpanWb(cols1[0].column_[i]);
            we1 = getSpanWe(cols1[cols1.size()-1].column_[i]);
            for (int j = 0; j < cols2[0].column_.size(); j++) {
                b2 = cols2[0].column_[j].left;
                e2 = cols2[cols2.size()-1].column_[j].right;
                wb2 = getSpanWb(cols2[0].column_[j]);
                we2 = getSpanWe(cols2[cols2.size()-1].column_[j]);
                if (e1 <= b2 && wb2-we1 >= 0 && wb2-we1<=1) {
                    string span_string = "";
                    for (int h = b1; h < e2; h++) {
                        span_string += content[h];
                    }
                    Span s(span_string, b1, e2);
                    for (int h = 0; h < cols1.size(); h++) {
                        vs[h].column_.push_back(cols1[h].column_[i]);
                    }
                    for (int h = (int)cols1.size(); h < cols1.size()+cols2.size(); h++ ) {
                        vs[h].column_.push_back(cols2[h-cols1.size()].column_[j]);
                    }
                    vs[vs.size()-1].column_.push_back(s);
                }
            }
        }
        return vs;
    }
    for (int i = 0; i < cols1[0].column_.size(); i++) {
        b1 = cols1[0].column_[i].left;
        e1 = cols1[cols1.size()-1].column_[i].right;
        wb1 = getSpanWb(cols1[0].column_[i]);
        we1 = getSpanWe(cols1[cols1.size()-1].column_[i]);
        for (int j = 0; j < cols2[0].column_.size(); j++) {
            b2 = cols2[0].column_[j].left;
            e2 = cols2[cols2.size()-1].column_[j].right;
            wb2 = getSpanWb(cols2[0].column_[j]);
            we2 = getSpanWe(cols2[cols2.size()-1].column_[j]);
            if (e1<=b2 && wb2-we1>min && wb2-we1<=max+1) {
                string span_string = "";
                for (int h = b1; h < e2; h++) {
                    span_string += content[h];
                }
                Span s(span_string, b1, e2);
                for (int h = 0; h < cols1.size(); h++) {
                    vs[h].column_.push_back(cols1[h].column_[i]);
                }
                for (int h = (int)cols1.size(); h < cols1.size()+cols2.size(); h++ ) {
                    vs[h].column_.push_back(cols2[h-cols1.size()].column_[j]);
                }
                vs[vs.size()-1].column_.push_back(s);
            }
        }
    }
    return vs;
}

int Parser::getSpanWb(Span s) {
    vector<TextToken> v = tokenizer->getTextTokens();
    int i;
    for (i = 0; i < v.size(); i++) {
        if (s.left >= v[i].start && s.left < v[i].end ) {
            return i;
        }
    }
    return -1;
}

int Parser::getSpanWe(Span s) {
    vector<TextToken> v = tokenizer->getTextTokens();
    int i;
    for (i = 0; i < v.size(); i++) {
        if (s.right-1 >= v[i].start && s.right-1 < v[i].end) {
            return i;
        }
    }
    return -1;
}

vector<Column> Parser::pattern_spec_handler(map<int, string> grouplist,int &current,  vector<Token*> vec, map<string, string> fromlist){
    return pattern_expr_handler(grouplist, current, vec, fromlist);
}

vector<Column>  Parser::pattern_expr_handler(map<int, string> grouplist,int &current,  vector<Token*> vec, map<string, string> fromlist){
    
    vector<Column> temp = pattern_pkg_handler(grouplist, current, vec, fromlist);
    int a = 0, b = 0;
    if (current + 1 < vec.size() && vec[current+1]->tag == Tag::TOKEN) {
        if (current + 3 < vec.size() &&vec[current+3]->tag == '{') {
            current += 4;
            a = ((Num*)vec[current])->value;
            
            current += 2;
            b = ((Num*)vec[current])->value;
            current+=2;
        }
        else{
            a = 1;
            b = 1;
            current += 3;
        }
        
    }
    

    vector<Column> temp2 = expr_other_handler(grouplist, current, vec, fromlist);
    if (temp2.size() == 0) {
            return temp;
        }
    else if (temp2[0].column_.size() == 0) {
        for (int i = 0; i < temp.size(); ++i)
        {
            temp2.push_back(Column(""));
        }
        return temp2;
    }
    else{
        vector<Column> r =  join(temp, temp2, a ,b);
        r.pop_back();
        return r;
    }
}

vector<Column> Parser::pattern_pkg_handler(map<int, string> grouplist,int &current,  vector<Token*> vec, map<string, string> fromlist){
    if (current < vec.size() && (vec[current]->tag == '<' || vec[current]->tag == Tag::REG)) {
        vector<Column> atom = atom_handler(grouplist, current, vec, fromlist);
        if (current < vec.size() &&(vec[current]->tag == '{' && vec[current -2 ]->tag != Tag::TOKEN)) {
            
            int m , n;
            current++;
            m = ((Num*)vec[current])->value;
            current += 2;
            n = ((Num*)vec[current])->value;
            current += 2;
            vector<Column> current_Col = atom;
            vector<Column> result_here = atom;
            for (int i = 1; i < m; i++) {
                vector<Column> temp;
                temp.push_back(join(current_Col,  atom, 0, 0)[2]);
                current_Col = temp;
            }
            result_here[0] = current_Col[0];
            for (int i = m; i < n; i++) {
                vector<Column> temp;
                temp.push_back(join(current_Col,  atom, 0, 0)[2]);
                current_Col = temp;
                result_here[0].column_.insert(result_here[0].column_.end(), current_Col[0].column_.begin(), current_Col[0].column_.end());
            }
            return result_here;
        }
        else{
            return atom;
        }
    }
    else{
        return pattern_group_handler(grouplist, current, vec, fromlist);
    }
}

vector<Column> Parser::atom_handler(map<int, string> grouplist,int &currrent,  vector<Token*> vec, map<string, string> fromlist){
    current_col_pos++;
    if(vec[currrent]->tag == '<'){
        currrent++;
        if (vec[currrent]->tag != Tag::ID)
        {   
            error();
        }
        vector<string> name_vec;
        name_vec.push_back(vec[currrent]->toString());
        currrent += 2;
        
        name_vec.push_back(vec[currrent]->toString());
        currrent += 2;
        name_vec.push_back("");
        vector< vector<string> > name_vs;
        name_vs.push_back(name_vec);
            
        return selectFromView(name_vs,fromlist);
    }else{
        result = findall(vec[currrent]->toString().c_str(), content.c_str());
        currrent++;
        map<int, string> m;
        m[0] = "";
        return createView(m);
    }
}

vector<Column> Parser::pattern_group_handler(map<int, string> grouplist,int &currrent,  vector<Token*> vec, map<string, string> fromlist){
    int capture_size = (int)capture_pos.size();
    currrent++;
    int begin = current_col_pos;
    vector<int> hold_pos;
    capture_pos.push_back(hold_pos);
    vector<Column> temp = pattern_expr_handler(grouplist, currrent, vec, fromlist);
    currrent++;
    int end = current_col_pos;
    capture_pos[capture_size].push_back(begin);
    capture_pos[capture_size].push_back(end);
    return temp;
}

vector<Column> Parser::expr_other_handler(map<int, string> grouplist,int &current,  vector<Token*> vec, map<string, string> fromlist){
    vector<Column> tt;
    //
    if (current < vec.size() && (vec[current]->tag == '(' || vec[current]->tag  == '<' || vec[current]->tag  == Tag::REG)) {
        vector<Column> temp =  pattern_pkg_handler(grouplist, current, vec, fromlist);
        int a= 0, b = 0;
        if (current+1 < vec.size() && vec[current+1]->tag == Tag::TOKEN) {
            if (current + 3 < vec.size() &&vec[current+3]->tag == '{') {
                current += 4;
                a = ((Num*)vec[current])->value;
                current += 2;
                b = ((Num*)vec[current])->value;
                current+=2;
            }
            else{
                a = 1;
                b = 1;
                current += 3;
            }
            
        }
        vector<Column> temp2 = expr_other_handler(grouplist, current, vec, fromlist);



        if (temp2.size() == 0) {
            return temp;
        }
        else if (temp2[0].column_.size() == 0) {
            for (int i = 0; i < temp.size(); ++i)
            {
                temp2.push_back(Column(""));
            }
            return temp2;
        }
        else{
            tt = join(temp, temp2, a ,b);
            tt.pop_back();
            return tt;
        }
    }
    return tt;
}
