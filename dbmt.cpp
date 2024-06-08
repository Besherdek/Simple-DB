#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <map>
#include <cstring>      //for strtok
#include <cstdio>       //for std::perror
#include <cctype>       //for isalnum

const std::string drop_str = "DROP TABLE";
const std::string create_str = "CREATE TABLE";
const std::string insert_str = "INSERT INTO";
const std::string select_str = "SELECT";

const std::string help_message = "DROP TABLE table name - delete table with given name (if exists)\nCREATE TABLE table name (column name: column type(, ...)) - create table with given name and columns(if table with that name doesn't exist)\nINSERT INTO table name VALUES (column name: column type(, ...)) - insert into table with given name followed values(if table exists and order of values is correct)\nSELECT column name(s) FROM table name (WHERE column name = value) - search and type given columns in table(if table and column names exist, * means all columns)\n";

enum val_type {
    INT = 0,
    FLOAT = 1,
    STR = 2
};

struct table {  
    std::vector<std::pair<std::string, val_type>> columns;   //name of column -> value type
};

struct meta_table {
    std::unordered_map<std::string, table> tables;        //name of table -> table struct
};

struct drop_struct {
    std::string table_name;
};

struct create_struct {
    std::string table_name;
    table columns;
};

struct insert_struct {
    std::string table_name;
    std::vector<std::string> values;
    std::vector<val_type> value_types;
};

struct select_struct {
    std::vector<std::string> expressions;
    std::string table_name;
    std::string statement;
    std::string to_be_equal;
    int index;
};

void read_meta(std::ifstream& strm, meta_table& meta) {
    std::string line;
    if(strm.is_open()) {
        while(std::getline(strm, line)) {
            std::string table_name = strtok(line.data(), "( ");     //.data() is to get char*, .c_str() returns const char*
            char* column_name = strtok(NULL, "( :,");
            while(column_name != NULL) {
                std::string column_content = strtok(NULL, " ,)");
                val_type content;
                if(column_content == "int") {
                    content = INT;
                }
                else if(column_content == "float") {
                    content = FLOAT;
                }
                else if(column_content == "str") {
                    content = STR;
                }
                meta.tables[table_name].columns.emplace_back(column_name, content);
                column_name = strtok(NULL, "( :,");
            }
        }
    }
    else {
        std::perror("Could not open meta_table file!\n");
        return;
    }
}

std::string ret_type(val_type type) {
    switch(type) {
        case INT:
            return "int";
        case FLOAT:
            return "float";
        case STR:
            return "str";
        default:
            return "unknown type";
    }
}

int write_meta(std::string table_name, std::vector<std::pair<std::string, val_type>> columns, const meta_table& meta) {
    if(meta.tables.contains(table_name)) {
        std::cout << "Table with name \"" << table_name << "\" already exists\n";
        return -1;
    }
    std::ofstream my_file;
    my_file.open("meta_table.txt", std::ios::app);
    my_file << std::endl << table_name << "(";
    for(int i = 0; i < columns.size(); ++i) {
        if(i != 0) {
            my_file << ", ";
        }
        my_file << columns[i].first << ": " << ret_type(columns[i].second);
    }
    my_file << ")";
    my_file.close();
    return 0;
}

bool is_empty(std::string input) {
    int offset = 0;
    for(int i = 0; i < input.length() && input[i] == ' '; ++i) {       //skipping empty spaces
        ++offset;
    }
    input = input.substr(offset);
    if(!input.empty()) {
        return false;
    }
    return true;
}

int parse_string(std::string& input, std::string to_parse) {
    std::string input_clone = input;
    int offset = 0;
    for(int i = 0; input_clone[i] == ' '; ++i) {       //skipping empty spaces
        ++offset;
    }
    input_clone = input_clone.substr(offset);
    offset = 0;

    for(int i = 0; i < to_parse.length() && i < input_clone.length(); ++i) {
        if(to_parse[i] != input_clone[i]) {
            return -1;
        }
        ++offset;
    }

    if(to_parse != input_clone.substr(0, offset)) {
        return -1;
    }

    input_clone = input_clone.substr(offset);
    input = input_clone;
    return 0;
}

int parse_name(std::string& input, std::string& name) {
    std::string input_clone = input;
    int offset = 0;
    for(int i = 0; input_clone[i] == ' '; ++i) {       //skipping empty spaces
        ++offset;
    }
    input_clone = input_clone.substr(offset);
    offset = 0;
    int i = 0;
    do {
        if(!std::isalnum(input_clone[i]) && input_clone[i] != '-' && input_clone[i] != '_') {
            return -1;
        }
        ++offset;
        ++i;
    }   while(std::isalnum(input_clone[i]) || input_clone[i] == '-' || input_clone[i] == '_');
    name = input_clone.substr(0, offset);
    input = input_clone.substr(offset);
    return 0;
}

int parse_name_for_select(std::string& input, std::string& name) {
    std::string input_clone = input;
    int offset = 0;
    for(int i = 0; input_clone[i] == ' '; ++i) {       //skipping empty spaces
        ++offset;
    }
    input_clone = input_clone.substr(offset);
    offset = 0;
    int i = 0;
    do {
        if(!std::isalnum(input_clone[i]) && input_clone[i] != '-' && input_clone[i] != '_' && input_clone[i] != '.') {
            return -1;
        }
        ++offset;
        ++i;
    }   while(std::isalnum(input_clone[i]) || input_clone[i] == '-' || input_clone[i] == '_' || input_clone[i] == '.');
    name = input_clone.substr(0, offset);
    input = input_clone.substr(offset);
    return 0;
}

int parse_type(std::string& input, val_type& type) {
    if(parse_string(input, "int") == 0) {
        type = INT;
    }
    else if(parse_string(input, "float") == 0) {
        type = FLOAT;
    }
    else if(parse_string(input, "str") == 0) {
        type = STR;
    }
    else {
        return -1;
    }
    return 0;
}

int parse_value(std::string& input, val_type& type, std::string& value) {
    std::string input_clone = input;
    int offset = 0;
    for(int i = 0; input_clone[i] == ' '; ++i) {       //skipping empty spaces
        ++offset;
    }
    input_clone = input_clone.substr(offset);
    offset = 0;
    if(input_clone[0] == '"') {
        offset += 2;
        for(int i = 1; input_clone[i] != '"'; ++i, ++offset) {}
        type = STR;
    }
    else if(input_clone[0] >= '0' && input_clone[0] <= '9') {
        for(int i = 0; input_clone[i] >= '0' && input_clone[i] <= '9'; ++i, ++offset) {}
        if(input_clone[offset] == '.') {
            ++offset;
            int i = offset;
            do {
                if(!(input_clone[i] >= '0' && input_clone[i] <= '9')) {
                    return -1;
                }
                ++offset;
                ++i;
            } while(input_clone[i] >= '0' && input_clone[i] <= '9');
            type = FLOAT;
        }
        else {
            type = INT;
        }
    }
    else {
        return -1;
    }
    value = input_clone.substr(0, offset);
    input = input_clone.substr(offset);
    return 0;
}

int parse_int(const std::string& input) {
    return std::stoi(input);
}

float parse_float(const std::string& input) {
    return std::stof(input);
}

std::string parse_string(const std::string& input) {
    return input.substr(1, input.length() - 2);
}

void parse_command(std::string& input, std::string& command) {
    if(input.substr(0, drop_str.length()) == drop_str) {
        command = drop_str;
        input = input.substr(drop_str.length());
    }
    else if(input.substr(0, create_str.length()) == create_str) {
        command = create_str;
        input = input.substr(create_str.length());
    }
    else if(input.substr(0, insert_str.length()) == insert_str) {
        command = insert_str;
        input = input.substr(insert_str.length());
    }
    else if(input.substr(0, select_str.length()) == select_str) {
        command = select_str;
        input = input.substr(select_str.length());
    }
    else {
        parse_name(input, command);
    }
}

int parse_drop_command(std::string& input, drop_struct& fields) {
    std::string table_name;
    if(parse_name(input, table_name) == -1) {
        return -1;
    }
    fields.table_name = table_name;
    return 0;
}

int drop_command(const drop_struct& fields, meta_table& meta) {
    if(!meta.tables.contains(fields.table_name)) {
        std::cout << "ERROR! Table with name \"" << fields.table_name << "\" doesn't exist\n";
        return -1;
    }
    meta.tables.erase(fields.table_name);
    std::fstream my_file("meta_table.txt");
    std::vector<std::string> lines;
    std::string line;
    bool found = false;
    while(std::getline(my_file, line)) {
        if(!found) {
            std::string name = line.substr(0, fields.table_name.length());
            if(name == fields.table_name) {
                found = true;
                continue;
            }
        }
        lines.push_back(line);
    }
    my_file.close();
    my_file.open("meta_table.txt", std::fstream::trunc | std::fstream::out);
    my_file << lines[0];
    for(int i = 1; i < lines.size(); ++i) {
        my_file << "\n" << lines[i];
    }
    my_file.close();
    std::string name = fields.table_name;
    name += ".txt";
    std::remove(name.data());
    return 0;
}

int parse_create_command(std::string& input, create_struct& fields) {
    std::string table_name;
    if(parse_name(input, table_name) == -1) {
        return -1;
    }
    std::string input_clone = input;
    fields.table_name = table_name;

    if(parse_string(input_clone, "(") != 0) {
        std::cout << "ERROR! ( not found\n";
        return -1;
    }
    std::string column_name;
    val_type type;
    if(parse_name(input_clone, column_name) == -1) {
        std::cout << "ERROR! column name problem\n";
        return -1;
    }
    if(parse_string(input_clone, ":") != 0) {
        std::cout << "ERROR! : not found\n";
        return -1;
    }
    if(parse_type(input_clone, type) == -1) {
        std::cout << "ERROR! type problem\n";
        return -1;
    }
    fields.columns.columns.emplace_back(column_name, type);

    while(true) {
        if(parse_string(input_clone, ")") == 0) {
            break;
        }

        if(parse_string(input_clone, ",") != 0) {
            std::cout << "ERROR! , not found\n";
            return -1;
            break;
        }
        if(parse_name(input_clone, column_name) == -1) {
            std::cout << "ERROR! name problem\n";
            return -1;
        }
        if(parse_string(input_clone, ":") != 0) {
            std::cout << "ERROR! : not found\n";
            return -1;
        }
        if(parse_type(input_clone, type) == -1) {
            std::cout << "ERROR! type problem\n";
            return -1;
        }
        fields.columns.columns.emplace_back(column_name, type);
    }

    input = input_clone;
    return 0;
}

int create_command(const create_struct& fields, meta_table& meta) {
    if(write_meta(fields.table_name, fields.columns.columns, meta) != 0) {
        return -1;
    }
    meta.tables[fields.table_name] = fields.columns;
    std::string name = fields.table_name;
    name += ".txt";
    std::ofstream create_file(name);
    create_file.close();
    return 0;
}

int parse_insert_command(std::string& input, insert_struct& fields) {
    std::string table_name;
    std::string input_clone = input;
    if(parse_name(input_clone, table_name) == -1) {
        std::cout << "ERROR! Table name parse error!\n";
        return -1;
    }

    if(parse_string(input_clone, "VALUES") != 0) {
        std::cout << "ERROR! VALUES not found\n";
        return -1;
    }

    if(parse_string(input_clone, "(") != 0) {
        std::cout << "ERROR! ( not found\n";
        return -1;
    }

    std::string value_written;
    val_type val_type_written;

    if(parse_value(input_clone, val_type_written, value_written) != 0) {
        std::cout << "ERROR! Value read problem\n";
        return -1;
    }

    fields.values.push_back(value_written);
    fields.value_types.push_back(val_type_written);

    while(true) {
        if(parse_string(input_clone, ")") == 0) {
            break;
        }

        if(parse_string(input_clone, ",") != 0) {
            std::cout << "ERROR! , not found\n";
            return -1;
            break;
        }
        if(parse_value(input_clone, val_type_written, value_written) != 0) {
            std::cout << "ERROR! Value read problem\n";
            return -1;
        }

        fields.values.push_back(value_written);
        fields.value_types.push_back(val_type_written);
    }

    fields.table_name = table_name;
    input = input_clone;
    return 0;
}

int insert_command(const insert_struct& fields, meta_table& meta) {
    if(!meta.tables.contains(fields.table_name)) {
        std::cout << "ERROR! Table with name \"" << fields.table_name << "\" doesn't exist\n";
        return -1;
    }

    std::string name = fields.table_name;
    if(fields.value_types.size() != meta.tables[name].columns.size()) {
        std::cout << "ERROR! Value types do not match\n";
        return -1;
    }
    for(int i = 0; i < fields.value_types.size(); ++i) {
        if(fields.value_types[i] != meta.tables[name].columns[i].second) {
            std::cout << "ERROR! Value types do not match\n";
            return -1;
        }
    }

    std::ofstream my_file;
    name += ".txt";
    my_file.open(name, std::ios::app | std::ios::out);

    if(fields.value_types[0] == STR) {
        if(my_file.tellp() != 0) {                  //if file is empty do not print newline
            my_file << std::endl;
        }
        my_file << parse_string(fields.values[0]);
    }
    else {
        if(my_file.tellp() != 0) {
            my_file << std::endl;
        }
        my_file << fields.values[0];
    }

    for(int i = 1; i < fields.values.size(); ++i) {
        if(fields.value_types[i] == STR) {
            my_file << ' ' << parse_string(fields.values[i]);
        }
        else {
            my_file << ' ' << fields.values[i];
        }
    }

    my_file.close();
    return 0;
}

int parse_select_command(std::string& input, select_struct& fields) {
    std::vector<std::string> expressions;
    std::string expression;
    std::string table_name;
    std::string input_clone = input;
    if(parse_string(input_clone, "*") == 0) {
        expression = "*";
    }
    else {
        if(parse_name(input_clone, expression) == -1) {
            std::cout << "ERROR! Expression parse error!\n";
            return -1;
        }
        if(expression == "FROM") {
            std::cout << "ERROR! No expression given\n";
            return -1;
        }
    }
    expressions.push_back(expression);

    while(parse_string(input_clone, ",") == 0 && expression != "*") {
        if(parse_name(input_clone, expression) == -1) {
            std::cout << "ERROR! Expression parse error!\n";
            return -1;
        }
        expressions.push_back(expression);
    }

    if(parse_string(input_clone, "FROM") != 0) {
        std::cout << "ERROR! FROM not found\n";
        return -1;
    }

    if(parse_name(input_clone, table_name) != 0) {
        std::cout << "ERROR! Table name parse error\n";
        return -1;
    }

    if(parse_string(input_clone, "WHERE") == 0) {
        std::string statement;
        std::string to_be_equal;
        if(parse_name(input_clone, statement) != 0) {
            std::cout << "ERROR! Incorrect column name\n";
            return -1;
        }
        if(parse_string(input_clone, "=") != 0) {
            std::cout << "ERROR! = is not found\n";
            return -1;
        }
        val_type dummy;
        if(parse_value(input_clone, dummy, to_be_equal) != 0) {
            std::cout << "ERROR! Incorrect value given\n";
            return -1;
        }
        if(dummy == STR) {
            to_be_equal = parse_string(to_be_equal);
        }
        fields.statement = statement;
        fields.to_be_equal = to_be_equal;
    }

    fields.expressions = expressions;
    fields.table_name = table_name;
    input = input_clone;
    return 0;
}

int select_command(select_struct& fields, meta_table& meta) {
    if(!meta.tables.contains(fields.table_name)) {
        std::cout << "ERROR! Table with name \"" << fields.table_name << "\" doesn't exist\n";
        return -1;
    }

    std::string name = fields.table_name;
    std::ifstream my_file;
    name += ".txt";
    my_file.open(name);

    std::vector<int> positions;
    if(fields.expressions[0] == "*") {
        for(int i = 0; i < meta.tables[fields.table_name].columns.size(); ++i) {
            positions.push_back(i);
        }
    }
    else {
        for(int i = 0; i < fields.expressions.size(); ++i) {
            bool found = false;
            for(int j = 0; j < meta.tables[fields.table_name].columns.size(); ++j) {
                if(!found) {
                    if(fields.expressions[i] == meta.tables[fields.table_name].columns[j].first) {
                        positions.push_back(j);
                        found = true;
                        break;
                    }
                }
            }
            if(!found) {
                std::cout << "ERROR! No column with \"" << fields.expressions[i] << "\" name\n";
                return -1;
            }
        }
    }

    if(fields.statement != "") {
        bool found = false;
        for(int i = 0; i < meta.tables[fields.table_name].columns.size(); ++i) {
            if(!found) {
                if(fields.statement == meta.tables[fields.table_name].columns[i].first) {
                    fields.index = i;
                    found = true;
                    break;
                }
            }
        }
        if(!found) {
            std::cout << "ERROR! No column with \"" << fields.statement << "\" name\n";
            return -1;
        }
    }

    std::string line;
    while(std::getline(my_file, line)) {
        std::vector<std::string> contents;
        std::string one_piece;
        bool found = false;
        int cur_index = 0;
        while(parse_name_for_select(line, one_piece) == 0) {
            if(fields.statement != "") {
                if(one_piece == fields.to_be_equal && cur_index == fields.index) {
                    found = true;
                }
            }
            contents.push_back(one_piece);
            ++cur_index;
        }
        if(fields.statement != "") {
            if(!found) {
                continue;
            }        
        }
        for(auto index: positions) {
            std::cout << contents[index] << ' ';
        }
        std::cout << std::endl;
    }
    return 0;
}

int main() {
    meta_table meth;
    std::ifstream myfile;
    myfile.open("meta_table.txt");
    read_meta(myfile, meth);
    myfile.close();

    while(true) {
        std::string input;
        std::string command;
        std::cout << ">";
        std::getline(std::cin, input);
        parse_command(input, command);
        
        drop_struct a;
        create_struct b;
        insert_struct c;
        select_struct d;

        if(command == drop_str) {
            if(parse_drop_command(input, a) != 0) {
                std::cout << "ERROR! Drop command parse error\n";
                continue;
            }
            if(!is_empty(input)) {
                std::cout << "ERROR! Some garbage after command\n";
                continue;
            }
            if(drop_command(a, meth) == 0) {
                std::cout << "Table dropped succesfully\n";
            }
        }

        else if(command == create_str) {
            if(parse_create_command(input, b) != 0) {
                std::cout << "ERROR! Create command parse error\n";
                continue;
            }
            if(!is_empty(input)) {
                std::cout << "ERROR! Some garbage after command\n";
                continue;
            }
            if(create_command(b, meth) == 0) {
                std::cout << "Table created succesfully\n";
                continue;
            }
        }

        else if(command == insert_str) {
            if(parse_insert_command(input, c) != 0) {
                std::cout << "ERROR! Insert command parse error\n";
                continue;
            }
            if(!is_empty(input)) {
                std::cout << "ERROR! Some garbage after command\n";
                continue;
            }
            if(insert_command(c, meth) == 0) {
                std::cout << "Column inserted succesfully\n";
            }
        }

        else if(command == select_str) {
            if(parse_select_command(input, d) != 0) {
                std::cout << "ERROR! Select command parse error\n";
                continue;
            }
            if(!is_empty(input)) {
                std::cout << "ERROR! Some garbage after command\n";
                continue;
            }
            select_command(d, meth);
        }

        else if(command == "exit" || command == "EXIT") {
            break;
        }

        else if(command == "clear" || command == "CLEAR") {
            system("clear");
        }

        else if(command == "help" || command == "HELP") {
            std::cout << help_message;
        }

        else if(command == "") {
            continue;
        }

        else {
            std::cout << "ERROR! No such command as \"" << command << "\"\n";
        }
    }
}