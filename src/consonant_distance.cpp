#include "consonant_distance.hpp"
#include "distance.hpp"
#include <cstdlib>
#include <iostream>
#include <stdexcept>

ConsonantDistance::ConsonantDistance(){
   if (consonants.empty()) {
            initialize();
        }
}

void ConsonantDistance::initialize(){
   consonants = {
      {"CH", Manner::affricate, true, false, 5},
      {"JH", Manner::affricate, true, true, 5},
      {"R", Manner::approximant, false, true, 4},
      {"W", Manner::approximant, false, true, 9},
      {"Y", Manner::approximant, false, true, 6},
      {"DH", Manner::fricative, false, true, 3},
      {"F", Manner::fricative, false, false, 2},
      {"HH", Manner::fricative, false, false, 8},
      {"S", Manner::fricative, true, false, 4},
      {"SH", Manner::fricative, true, false, 5},
      {"TH", Manner::fricative, false, false, 3},
      {"V", Manner::fricative, false, true, 2},
      {"Z", Manner::fricative, true, true, 4},
      {"ZH", Manner::fricative, true, true, 5},
      {"L", Manner::lateral_approximant, false, true, 4},
      {"M", Manner::nasal, false, true, 1},
      {"N", Manner::nasal, false, true, 4},
      {"NG", Manner::nasal, false, true, 7},
      {"B", Manner::plosive, false, true, 1},
      {"D", Manner::plosive, false, true, 4},
      {"G", Manner::plosive, false, true, 7},
      {"K", Manner::plosive, false, false, 7},
      {"P", Manner::plosive, false, false, 1},
      {"T", Manner::plosive, false, false, 4}
   };

   // Build lookup map
   for (size_t i = 0; i < consonants.size(); ++i) {
      consonantIndices[consonants[i].arpabet] = i;
   }
}

const Consonant& ConsonantDistance::get_consonant(const std::string& arpabet){
   auto it = consonantIndices.find(arpabet);
   if (it == consonantIndices.end()) {
      throw std::out_of_range("Consonant not found");
   }
   return consonants[it->second];
}

int ConsonantDistance::get_distance(const std::string& phone1, const std::string& phone2){

   auto consonant1{get_consonant(phone1)};
   auto consonant2{get_consonant(phone2)};

   if (consonant1.arpabet == consonant2.arpabet) {
      return 0;
   }

   // deal with approximant weirdness

   // if W & V (these are similar right or am I crazy?)
   else if (  (consonant1.arpabet == "W" || consonant1.arpabet == "V")
   &&         (consonant2.arpabet == "W" || consonant2.arpabet == "V")){
      return CONSTANTS::CONSONANT::W_V_DISTANCE;
   }
   // if both are 'R' || 'Y' || 'W' || 'L'
   else if(   (consonant1.manner == Manner::approximant || consonant1.manner == Manner::lateral_approximant)
   &&  (consonant2.manner == Manner::approximant || consonant2.manner == Manner::lateral_approximant)){
      // if R & L
      if (  (consonant1.arpabet == "R" || consonant1.arpabet == "L")
      &&    (consonant2.arpabet == "R" || consonant2.arpabet == "L")){
         return CONSTANTS::CONSONANT::R_L_DISTANCE;
      }
      else {
         return std::abs(consonant1.place - consonant2.place);
      }
   }

   // are two consonants in the same manner?
   else if(consonant1.manner == consonant2.manner){
      int distance = std::abs(consonant1.place - consonant2.place);
      distance += consonant1.voiced == consonant2.voiced ? 0 : CONSTANTS::CONSONANT::VOICED_PENALTY;
      return distance;
   }

   // is one an affricate and the other either fricative or plosive?
   else if(
      (consonant1.manner == Manner::affricate && consonant2.manner == Manner::fricative)
   || (consonant1.manner == Manner::affricate && consonant2.manner == Manner::plosive)
   || (consonant2.manner == Manner::affricate && consonant1.manner == Manner::fricative)
   || (consonant2.manner == Manner::affricate && consonant1.manner == Manner::plosive)
   ){
       // if one is affricate & other is sibilant fricative
      if((consonant1.manner == Manner::affricate && consonant2.manner == Manner::fricative && consonant2.sibilant == true)
      || (consonant2.manner == Manner::affricate && consonant1.manner == Manner::fricative && consonant1.sibilant == true)){
         int distance = std::abs(consonant1.place - consonant2.place);
         distance += consonant1.voiced == consonant2.voiced ? 0 : CONSTANTS::CONSONANT::VOICED_PENALTY;
         distance += CONSTANTS::CONSONANT::AFFRICATE_SIBILANT_FRICATIVE_PENALTY;
         return distance;
      }
      // if affricate & plosive
      else if((consonant1.manner == Manner::affricate && consonant2.manner == Manner::plosive)
      || (consonant2.manner == Manner::affricate && consonant1.manner == Manner::plosive)){
         int distance = std::abs(consonant1.place - consonant2.place);
         distance += consonant1.voiced == consonant2.voiced ? 0 : CONSTANTS::CONSONANT::VOICED_PENALTY;
         distance += CONSTANTS::CONSONANT::AFFRICATE_PLOSIVE_PENALTY;
         return distance;
      }

      // if affricate & non-sibilant fricative
      else if((consonant1.manner == Manner::affricate && consonant2.manner == Manner::fricative)
      || (consonant2.manner == Manner::affricate && consonant1.manner == Manner::fricative)){
         int distance = std::abs(consonant1.place - consonant2.place);
         distance += consonant1.voiced == consonant2.voiced ? 0 : CONSTANTS::CONSONANT::VOICED_PENALTY;
         distance += CONSTANTS::CONSONANT::AFFRICATE_NON_SIBILANT_FRICATIVE_PENALTY;
         return distance;
      }

      else {
         std::cout << "This path should never happen..." << std::endl;
         return CONSTANTS::CONSONANT::UNRELATED_PENALTY;
      }
   }
   // otherwise, consonants are unrelated
   else {
      return CONSTANTS::CONSONANT::UNRELATED_PENALTY;
   }
}
