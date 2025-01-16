#pragma once

#include "convenience.hpp"
#include "vowel_hex_graph.hpp"
#include "consonant_distance.hpp"

#include <iostream>
#include <vector>

inline int GAP_PENALTY() {
   return 10;
}

// TODO: This should probably use Damerau distance, i.e. include transposition of adjacent elements in addition to insertions, deletions, and mismatches.
inline int SUBSTITUTION_SCORE(const std::string& s1, const std::string& s2) {
   const int MATCH_SCORE = 0;
   const int SAME_VOWEL_DIFFERENT_STRESS = 1;
   const int VOWEL_TO_CONSONANT_MISMATCH = 30;
   const int CONSONANT_MISMATCH = 5;

   if (s1 == s2) {
      return MATCH_SCORE;
   }

   // BOTH ARE VOWELS
   else if (std::isdigit(s1.back()) && std::isdigit(s2.back())) {
      // std::cout<< "Both are vowels." << std::endl;

      std::string v1{s1};
      std::string v2{s2};
      int stress1 = v1.back();
      int stress2 = v2.back();
      v1.pop_back();
      v2.pop_back();

      // same vowel different stress
      if(v1 == v2) {
         // std::cout << "Same vowel but different stress." << std::endl;

         return SAME_VOWEL_DIFFERENT_STRESS;
      }
      else {
         // Check vowel distance.
         VowelHexGraph::initialize();
         int vowel_distance{VowelHexGraph::get_distance(v1, v2)};
         // std::cout << "Vowel distance is: " << vowel_distance << std::endl;

         // TODO should we return vowel_distance itself, or scale it in some way
         // Should there be a penalty for vowels having different stresses?
         if (stress1 != stress2) {
            return vowel_distance + 1; 
         }
         return vowel_distance;

      }
   }
   // AT LEAST ONE CONSONANT
   else {
      // Mismatch vowel to consonant, which we never want to happen??
      if (std::isdigit(s1.back()) || std::isdigit(s2.back())) {
         return VOWEL_TO_CONSONANT_MISMATCH; 
      }
   
      // both consonants
      else {
         // TODO implement consonant distance
         // for now
         return CONSONANT_MISMATCH;
      }
   }
}


/**
 * Implementation of Levenshtein distance algorithm, but comparing ARPABET symbols, instead of characters, using custom weights for gaps and substitutions based on phoneme distance.
 * 
 * 
 * TODO: this does not take consonant distance into account. E.g. /B/ and /P/ should be closer than /B/ and /SH/. This is sort of a hack to get something meaningful in the meantime. But ulitmately we should incorporate consonant distance.
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
    // Early return for trivial cases
    if (phones1.empty()) return phones2.size();
    if (phones2.empty()) return phones1.size();

    std::vector<std::string> symbols1{phones_string_to_vector(phones1)};
    std::vector<std::string> symbols2{phones_string_to_vector(phones2)};

    size_t len1 = symbols1.size();
    size_t len2 = symbols2.size();

    // Create two rows for dynamic programming
    std::vector<int> prev(len2 + 1, 0);
    std::vector<int> curr(len2 + 1, 0);

    // Initialize base cases with GAP_PENALTY()
    for (size_t j = 0; j <= len2; ++j) {
        prev[j] = j * GAP_PENALTY();
    }

    // Fill the DP table row by row
    for (size_t i = 1; i <= len1; ++i) {
        // Base case, other axis
        curr[0] = i * GAP_PENALTY();
        for (size_t j = 1; j <= len2; ++j) {
            // std::cout << "Comparing: " << symbols1[i-1];
            // std::cout << " and " << symbols2[j-1] << std::endl;
            int mismatch_score{SUBSTITUTION_SCORE(symbols1[i-1], symbols2[j-1])};
            
            curr[j] = std::min({ 
                prev[j] + GAP_PENALTY(),      // Deletion
                curr[j - 1] + GAP_PENALTY(),  // Insertion
                prev[j - 1] + mismatch_score // Substitution
            });

            // std::cout << "Minimum: " << curr[j] << std::endl << std::endl << std::endl;
        }
        prev.swap(curr);
    }

    return prev[len2];
}
