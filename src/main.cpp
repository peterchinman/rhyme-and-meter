#include "CMU_Dict.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

int main() {
    CMU_Dict dict;
    bool dictBool = dict.import_dictionary();
    std::vector<std::string> word_lower = dict.find_phones("associate");
    std::cout << "number of words returned: " << word_lower.size() << std::endl;
    // for (auto & word : word_lower) {
    //     std::cout << "[" << word << "]" << std::endl;
    // }
    for (auto & c : word_lower[0]) {
        std::cout << c << std::endl;
    }
    return 0;
}
