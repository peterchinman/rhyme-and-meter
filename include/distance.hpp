#pragma once

#include "convenience.hpp"
#include "consonant_distance.hpp"
#include "vowel_hex_graph.hpp"
#include <cstdlib>

namespace CONSTANTS{ 

   //  We never want to compare a vowel to a consonant, so we can give this an arbitrarily high penalty.
   const int VOWEL_TO_CONSONANT_MISMATCH = 100;

   namespace VOWEL {
      // All vowel distances get multiplied by this coefficient.
      // Vowel distance ranges from 0-4, consonant distance range up to the CONSONANT::UNRELATED_PENALTY.
      // Naive assumption would be to set the coefficient so that these ranges are roughly equal.
      const int COEFFICIENT{5};

      // Insertion/deletion penalty for vowels
      const int INDEL_PENALTY = 20;

      // Penalty when comparing vowels with different stress
      const int STRESS_PENALTY = 1;
   }

   namespace CONSONANT {
      // Insertion/deletion penalty for consonants
      // This should be greater or equal to half of the CONSONANT::UNRELATED_PENALTY, otherwise it's cheaper to insert/delete a consonant than to compare consonants.
      const int INDEL_PENALTY = 5; 

      const int REPEATED_CONSONANT_PENALTY = 1;

      // furthest related consonants are 7 apart
      // unrelated consonants should be... somewhat further than that??
      const int UNRELATED_PENALTY = 10;

      // TIN, DIN
      const int VOICED_PENALTY = 1;

      // ROOT, LOOT
      const int R_L_DISTANCE = 1;

      // WHILE, VIAL
      // (these are similar right or am I crazy?)
      const int W_V_DISTANCE = 2;

      // CHIN, SHIN
      const int AFFRICATE_SIBILANT_FRICATIVE_PENALTY = 1;

      // CHIN, TIN
      const int AFFRICATE_PLOSIVE_PENALTY = 2;

      // CHIN, THIN
      const int AFFRICATE_NON_SIBILANT_FRICATIVE_PENALTY = 2;
   }
}

// FOR INSERTIONS AND DELETIONS WITH REPETITION DETECTION
// Gap penalty varies based on whether we're inserting/deleting a vowel or consonant
// and, if it's a consonant, if its a repetition of the prev consonant
inline int GAP_PENALTY(const std::string& phoneme = "", const std::string& prev_phoneme = "") {
   if (phoneme.empty()) {
      return CONSTANTS::CONSONANT::INDEL_PENALTY;
   }

   if (is_vowel(phoneme)) {
      return CONSTANTS::VOWEL::INDEL_PENALTY;
   }

   if (phoneme == prev_phoneme) {
      return CONSTANTS::CONSONANT::REPEATED_CONSONANT_PENALTY;
   }
   
   return CONSTANTS::CONSONANT::INDEL_PENALTY;
}

// TODO: This should probably use Damerau distance, i.e. include transposition of adjacent elements in addition to insertions, deletions, and mismatches.
inline int SUBSTITUTION_SCORE(const std::string& s1, const std::string& s2) {
   const int MATCH_SCORE = 0;

   if (s1 == s2) {
      return MATCH_SCORE;
   }

   // BOTH ARE VOWELS
   else if (std::isdigit(s1.back()) && std::isdigit(s2.back())) {

      std::string v1{s1};
      std::string v2{s2};
      int stress1 = v1.back();
      int stress2 = v2.back();
      v1.pop_back();
      v2.pop_back();

      // same vowel different stress
      if(v1 == v2 && stress1 != stress2) {
         // std::cout << "Same vowel but different stress." << std::endl;

         return CONSTANTS::VOWEL::STRESS_PENALTY;
      }
      else {
         // Check vowel distance.
         VowelHexGraph::initialize();
         int vowel_distance{VowelHexGraph::get_distance(v1, v2)};
         if (stress1 != stress2) {
            return (vowel_distance * CONSTANTS::VOWEL::COEFFICIENT) + CONSTANTS::VOWEL::STRESS_PENALTY;
         }
         return vowel_distance * CONSTANTS::VOWEL::COEFFICIENT;

      }
   }
   // AT LEAST ONE CONSONANT
   else {
      // Mismatch vowel to consonant, which we never want to happen??
      if (std::isdigit(s1.back()) || std::isdigit(s2.back())) {
         return CONSTANTS::VOWEL_TO_CONSONANT_MISMATCH;
      }

      // both consonants
      else {
         ConsonantDistance::initialize();
         return ConsonantDistance::get_distance(s1, s2);
      }
   }
}
