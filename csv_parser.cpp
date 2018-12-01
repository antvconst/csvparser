#include <iostream>
#include <vector>
#include <sstream>
#include <string>
#include <exception>
#include <algorithm>
#include <fstream>
#include <iomanip>

class parsing_error : public std::logic_error {
    std::string msg_;
    
public:
    parsing_error(const std::string& msg) : std::logic_error(msg) {
        msg_ = msg;
    }

    const char* what() {
        return msg_.c_str();
    }
};

using Row = std::vector<double>;
using Table = std::vector<Row>;
using Tokens = std::vector<std::string>;

void verify(bool condition, const std::string& message) {
    if (!condition) {
        throw parsing_error(message);
    }
}

std::string get_line(std::istream& is) {
    std::string s;
    std::getline(is, s);

    return s;
} 

Tokens split(const std::string& str, char delim) {
   Tokens tokens;
   std::string token;
   std::istringstream token_stream(str);

   while (std::getline(token_stream, token, delim)) {
       tokens.push_back(token);
   }

   return tokens;
}

Row parse_line(const std::string& line) {
    auto tokens = split(line, ',');
    
    Row r(tokens.size());
    std::transform(tokens.begin(), tokens.end(), r.begin(), [i=0](const std::string& token) mutable {
        ++i;
        try {
            return std::stod(token);
        } catch (...) {
            throw parsing_error("invalid value \"" + token + "\" in column " + std::to_string(i));
        }
    });

    return r;
}

std::pair<Tokens, Table> parse(std::ifstream& is) {
    std::vector<std::vector<double>> data;

    size_t i_line = 1;
    std::string line;

    std::getline(is, line);
    auto column_names = split(line, ',');
    size_t ncol = column_names.size();
    verify(ncol > 0, "Invalid column names");

    try {
        while (std::getline(is, line)) {
            ++i_line;

            auto values = parse_line(line);
            verify(values.size() == ncol,
                   "invalid number of columns (expected " + std::to_string(ncol) + 
                   ", got " + std::to_string(values.size()) + ")");

            data.push_back(values);
        } 
    } catch (parsing_error& e) {
        throw parsing_error("Line " + std::to_string(i_line) + ": " + e.what());
    }

    return {column_names, data};
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cout << "Usage: csv_parser <input>" << std::endl;
        return 1;
    }

    try {
        std::ifstream is(argv[1]);

        if (!is) {
            throw parsing_error("Unable to open " + std::string(argv[1]));
        }

        auto r = parse(is);
        
        size_t total_len = 0;
        for (auto colname : r.first) {
            std::cout << colname << " ";
            total_len += colname.size()+1;
        }
        std::cout << '\n' << std::string(total_len-1, '=') << '\n' << std::endl;

        for (auto row : r.second) {
            for (auto val : row) {
                std::cout << val << " ";
            }
            std::cout << std::endl;
        }
        
    } catch (parsing_error& e) {
        std::cerr << "ERROR\t" << e.what() << std::endl;
    }

    return 0;
}