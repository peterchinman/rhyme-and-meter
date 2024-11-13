#include "VowelHexGraph.h"
#include <queue>
#include <unordered_map>
#include <unordered_set>



void VowelHexGraph::add_edge(const std::string& vowel1, const std::string& vowel2) {
   adjacency_map[vowel1].push_back(vowel2);
   adjacency_map[vowel2].push_back(vowel1);
}

std::vector<std::string> VowelHexGraph::get_connected_vowels(const std::string& vowel) {
   return adjacency_map[vowel];
}

int VowelHexGraph::calculate_shortest_distance(const std::string& vowel1, const std::string& vowel2){
   int shortest_distance{};
  
   // If both vowels are the same, the distance is 0
    if (vowel1 == vowel2) return 0;

    // Set up a queue for BFS and a set to track visited nodes
    std::queue<std::pair<std::string, int>> to_visit;
    std::unordered_set<std::string> visited;

    // Initialize BFS
    to_visit.push({vowel1, 0});
    visited.insert(vowel1);

    // Perform BFS
    while (!to_visit.empty()) {
        auto [current_vowel, distance] = to_visit.front();
        to_visit.pop();

        // Check all connected vowels
        for (const auto& neighbor : adjacency_map[current_vowel]) {
            if (neighbor == vowel2) {
                return distance + 1; // Found the target vowel
            }
            if (visited.find(neighbor) == visited.end()) {
                visited.insert(neighbor);
                to_visit.push({neighbor, distance + 1});
            }
        }
    }

    // If we complete the BFS without finding vowel2, return -1
    return -1;
}

void VowelHexGraph::calculate_all_distances() {
   // we need to get all pairs of vowels
   std::vector<std::string> vowels{"AE", "AA", "EH", "AH", "AO", "IY", "IH", "UH", "UW"};
   for (std::size_t i{}; i < vowels.size(); ++i) {
      for (std::size_t j{i}; j < vowels.size(); ++j){
         // add shortest distance between these two vowels to the map
         distance_between_vowels_map[std::make_pair(vowels[i], vowels[j])] = calculate_shortest_distance(vowels[i], vowels[j]);
      }
   }
}

void VowelHexGraph::initialize(){
   add_edge("AE", "AA");
   add_edge("AE", "AH");
   add_edge("AE", "EH");
   add_edge("AA", "AO");
   add_edge("AA", "AH");
   add_edge("EH", "AH");
   add_edge("EH", "IH");
   add_edge("EH", "IY");
   add_edge("AH", "AO");
   add_edge("AH", "UH");
   add_edge("AH", "IH");
   add_edge("AO", "UW");
   add_edge("AO", "UH");
   add_edge("IY", "IH");
   add_edge("IH", "UH");
   add_edge("UH", "UW");

   calculate_all_distances();
}

int VowelHexGraph::get_distance(const std::string& vowel1, const std::string& vowel2) {
   return distance_between_vowels_map[std::make_pair(vowel1, vowel2)];
}



