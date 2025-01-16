#include "vowel_hex_graph.hpp"
#include <queue>
#include <unordered_map>
#include <unordered_set>

VowelHexGraph::VowelHexGraph() {
    initialize();
}

void VowelHexGraph::add_edge(const std::string& vowel1, const std::string& vowel2) {
   adjacency_map[vowel1].push_back(vowel2);
   adjacency_map[vowel2].push_back(vowel1);
}

std::vector<std::string> VowelHexGraph::get_connected_vowels(const std::string& vowel) {
   return adjacency_map[vowel];
}

int VowelHexGraph::calculate_shortest_distance(const std::string& vowel1, const std::string& vowel2){
  
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
   std::vector<std::string> vowels{"AE", "AA", "EH", "AH", "AO", "IY", "IH", "UH", "UW", "ER", "AW", "AY", "EY", "OW", "OY"};
   for (std::size_t i{}; i < vowels.size(); ++i) {
      for (std::size_t j{i}; j < vowels.size(); ++j){
         // add shortest distance between these two vowels to the map
         distance_between_vowels_map[std::make_pair(vowels[i], vowels[j])] = calculate_shortest_distance(vowels[i], vowels[j]);
      }
   }
}

void VowelHexGraph::initialize(){
   if (!initialized) {
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

      /**
       * OPIONATED /ER/ ADJACENCY:
       * 
       * 2. ER as in BIRD is adjacent to:
       *    AH as in BUT
       */
      add_edge("ER", "AH");


      /** 
       * 
       * DIPTHONG ADJACENCIES
       * "AW", "AY", "EY", "OW", "OY" // 5 dipthongs
       *  bout, bite, bait, boat, boy
       * 
       * I am making some extremely opinionated decisions here:
       * 
       * 1. AW as in BOUT is adjacent to:
       *    UH as in BUSH
       *    OW as in BOAT
       *    AH as in BUT
       *    AA : 2 (satisfied by AH adjacency) ((if you were to get rid of that you'd need to somehow set a distance, e.g. by introducing a notion of distance into edges))
       *    AE : 2 (see above)
       */
      add_edge("AW", "UH");
      add_edge("AW", "OW");
      add_edge("AW", "AH");
      /**
       * 
       * 2. AY as in BITE is adjacent to:
       *    IH as in BIT 
       *    EY as in BAIT
       *    AH as in BUT
       *    AA : 2 (satisfied by AH adjacency) ((if you were to get rid of that you'd need to somehow set a distance, e.g. by introducing a notion of distance into edges))
       *    AE : 2 (see above)
       */
      add_edge("AY", "IH");
      add_edge("AY", "EY");
      add_edge("AY", "AH");
      /**
       * 3. EY as in BAIT is adjacent to:
       *    AY as in BITE *redundant
       *    IH as in BIT
       *    EH as in BET
       *    IY as in BEAT
       */
      add_edge("EY", "IH");
      add_edge("EY", "EH");
      add_edge("EY", "IY");
      /** 4. OW as in BOAT is adjacent to:
       *    OY as in BOY
       *    AW as in BOUT *redundant
       *    UH as in BUSH
       *    UW as in BOOT
       *    AO as in BOMB
       */
      add_edge("OW", "OY");
      add_edge("OW", "UH");
      add_edge("OW", "UW");
      add_edge("OW", "AO");
      /* 5. OY as in BOY is adjacent to:
      *    IH as in BIT
      *    OW as in BOAT *redundant
      */
      add_edge("OY", "IH");

      calculate_all_distances();
      
      initialized = true;
   }
   
}

int VowelHexGraph::get_distance(const std::string& vowel1, const std::string& vowel2) {
   if (!initialized) {
      throw std::runtime_error("VowelHexGraph not initialized. Call initialize() first.");
   }
   return distance_between_vowels_map[std::make_pair(vowel1, vowel2)];
}




