#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <optional>

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
