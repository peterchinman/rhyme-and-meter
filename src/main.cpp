#include "rhyme_and_meter.hpp"
#include <iostream>
#include <string>

int main() {
    Rhyme_and_Meter dict;
    Phonetic phonetic_dict;
    
    std::cout << "Rhyme and Meter Analysis Tool" << std::endl;
    std::cout << "Enter 'quit' or 'exit' to stop" << std::endl << std::endl;
    
    while (true) {
        std::string line1, line2;
        
        std::cout << "Enter first line of text: ";
        std::getline(std::cin, line1);
        
        // Check for quit command
        if (line1 == "quit" || line1 == "exit") {
            std::cout << "Goodbye!" << std::endl;
            break;
        }
        
        std::cout << "Enter second line of text: ";
        std::getline(std::cin, line2);
        
        // Check for quit command
        if (line2 == "quit" || line2 == "exit") {
            std::cout << "Goodbye!" << std::endl;
            break;
        }
        
        auto alignment_result = dict.minimum_text_alignment(line1, line2);
        
        if (!alignment_result.has_value()) {
            std::cout << "Error: Unrecognized words: ";
            const auto& error = alignment_result.error();
            for (size_t i = 0; i < error.words.size(); ++i) {
                if (i > 0) std::cout << ", ";
                std::cout << error.words[i];
            }
            std::cout << std::endl << std::endl;
            continue;
        }
        
        const auto& alignment = alignment_result.value();
        
        std::cout << "Distance: " << alignment.distance << std::endl;
        std::cout << "Alignment:" << std::endl;
        print_pair(alignment.ZWpair);
        std::cout << std::endl;
    }
    
    return 0;
}
