#pragma once

#ifdef __EMSCRIPTEN__
#include <emscripten/bind.h>
#endif

#include "distance.hpp"
#include "convenience.hpp"
#include "vowel_hex_graph.hpp"
#include "consonant_distance.hpp"

#include <iostream>
#include <vector>
#include <algorithm>

/**
 * Implementation of Levenshtein distance algorithm, but comparing ARPABET symbols, instead of characters, using custom weights for gaps and substitutions based on phoneme distance.
 *
 * TODO: This should probably use Damerau distance, i.e. include transposition of adjacent elements in addition to insertions, deletions, and mismatches, because "most" and "moats" are more similar than the double sub penalty would seem?
 *
 * TODO: Should take into account whether an insertion is a repetition of the same symbol and weight that less. For e.g. comparing "pulley" and "full lee", "lee" should be penalized less (or not at all) for repeating the /L/ symbol.
 *
 * @param phones1 (string): string of space-separated phones
 * @param phones2 (stinrg): string of space-separated phones
 * @return (int): levenshtein distance between the sets of phones
 */
inline int levenshtein_distance(const std::string& phones1, const std::string& phones2) {

    std::vector<std::string> symbols1{phones_string_to_vector(phones1)};
    std::vector<std::string> symbols2{phones_string_to_vector(phones2)};

   // Handle empty cases
    if (symbols1.empty() || symbols2.empty()) {
      if (symbols1.empty() && symbols2.empty()) {
        return 0;
      }
      else if (symbols1.empty()) {
         int distance{0};
         for (const auto& symbol : symbols2) {
            distance += GAP_PENALTY(symbol);
         }
         return distance;
      }
      else {
         int distance{0};
         for (const auto& symbol : symbols1) {
            distance += GAP_PENALTY(symbol);
         }
         return distance;
      }
    }

    size_t len1 = symbols1.size();
    size_t len2 = symbols2.size();

    // Create two rows for dynamic programming
    std::vector<int> prev(len2 + 1, 0);
    std::vector<int> curr(len2 + 1, 0);

    // Initialize base cases with GAP_PENALTY()
    prev[0] = 0;
    for (size_t j = 1; j <= len2; ++j) {
      prev[j] = j * GAP_PENALTY(symbols2.at(j-1));
    }

    // Fill the DP table row by row
    for (size_t i = 1; i <= len1; ++i) {
        // Base case, other axis
        curr[0] = i * GAP_PENALTY(symbols1.at(i-1));
        for (size_t j = 1; j <= len2; ++j) {
            // std::cout << "Comparing: " << symbols1[i-1];
            // std::cout << " and " << symbols2[j-1] << std::endl;
            int mismatch_score{SUBSTITUTION_SCORE(symbols1[i-1], symbols2[j-1])};

            curr[j] = std::min({
                prev[j] + GAP_PENALTY(symbols1.at(i-1)),      // Deletion of symbol from phones1
                curr[j - 1] + GAP_PENALTY(symbols2.at(j-1)),  // Insertion of symbol from phones2
               prev[j - 1] + mismatch_score // Substitution
            });

            // std::cout << "Minimum: " << curr[j] << std::endl << std::endl << std::endl;
        }
        prev.swap(curr);
    }

    return prev[len2];
}

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_BINDINGS(distance_functions) {
   emscripten::function("levenshtein_distance", &levenshtein_distance);
}
#endif
