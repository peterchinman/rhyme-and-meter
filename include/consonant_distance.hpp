#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <optional>

namespace HARDCODED{
   // TIN, DIN
   const int VOICED_PENALTY = 1;

   // REASONING for this value:
   // furthest related consonants are 7 apart
   // unrelated consonants are… twice that far?
   const int UNRELATED_CONSONANT_PENALTY = 14;

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

enum class Manner{
   affricate,
   approximant,
   fricative,
   lateral_approximant,
   nasal,
   plosive
};

struct Consonant{
   std::string arpabet{};
   Manner manner{};
   bool sibilant{};
   bool voiced{};
   int place{};
};

class ConsonantDistance {
private:
   static inline std::vector<Consonant> consonants{};
   static inline std::unordered_map<std::string, size_t> consonantIndices{};

public: 
   ConsonantDistance();

   static void initialize();

   static const Consonant& get_consonant(const std::string& arpabet);

   static int get_distance(const std::string& consonant_1, const std::string& consonant_2);
};
