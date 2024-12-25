#pragma once

#include <string>
#include <unordered_map>
#include <utility>
#include <vector>


/**
 * Uses Static Storage for Distances.
 * 
 * USAGE:
 * 
 * VowelHexGraph::initialize();
 * VowelHexGraph::get_distance(vowel1, vowel2);
*/
class VowelHexGraph {
private:

    /**
     * The Vowel Hexagonal Graph is a rendering of vowel space that I invented, by arranging the CMU Pronouncing Dictionary monophthong vowels into a hexagonal grid. Image the vowels dripping with honey.
     *                                                     
                                                      
     ..         .          .         ..           
  .::  .=.   .=.  ::.  .::  .=.   .=.  ::.        
.:.       .=.       .::.       .=.       .:       
..   IY    =    IH   .    UH    =    UW   .       
..         =         .          =         .       
..        .=.        ..        .=.        .       
   =.  .-.   ::   .=    +.   -:   .-.  .=         
     .-         -.        .-         -.           
      :   EH    :    AH    :    AO   -            
      :         :          :         -            
      -.       .-.        .-.        -            
       ..=   +..  .-.  .-.  ..=   =..             
          .=.        ..         =                 
           =    AE   .   AA    =                 
           =         .          =                 
           =:.      .::.      ..=                 
             .:: .-.    .=. ::.                                                           
     * 
     * 
     * Which is modeled here as an adjacency map where each vowel is keyed to a vector of connected vowels.
     * 
     * DIPTHONGS:
     * 
     *  "AW", "AY", "EY", "OW", "OY" // 5 dipthongs
     *  bout, bite, bait, boat, boy
     * 
     * dipthongs mapping
     *  AW  aʊ  ->  AU* UH
     *  AY  aɪ  ->  AU* IH
     *  EY  eɪ  ->  EE* IH
     *  OW  oʊ  ->  OH* UH
     *  OY  ɔɪ  ->  OH* IH
     * 
     * Where AU is IPA a, between AE and AA
     *       EE is IPA e, between EH and IY
     *       OH is IPA o, between AO and UW
     * 
     * NOTE: some of the diphthongs start in in-between locations on the hex-graph, but end in either UH or IH.
    
    */
    static inline bool initialized = false;
    static inline std::unordered_map<std::string, std::vector<std::string>>  adjacency_map{};

    // custom PairHash Function so that we can use std::unordered_map with a std::pair as a key
    struct PairHash {
        template <typename T1, typename T2>
        std::size_t operator()(const std::pair<T1, T2>& p) const {
            std::hash<T1> h1;
            std::hash<T2> h2;
            return h1(p.first) ^ h2(p.second);
        }
    };
    //  custom equality function for std::pairs where we don't care about order, so that we can use std::pair as a key for std::unordered_map
    struct PairEqual {
        template <typename T1, typename T2>
        bool operator()(const std::pair<T1, T2>& p1, const std::pair<T1, T2>& p2) const {
            return (p1.first == p2.first && p1.second == p2.second)
                || (p1.first == p2.second && p1.second == p2.first);
        }
    };

    static inline std::unordered_map<std::pair<std::string, std::string>, int, PairHash, PairEqual> distance_between_vowels_map{};

public:
    VowelHexGraph();

    /**
     * Adds edges between vowels to out adjacency_map.
     * 
     * @param vowel1 (string): arpabet Vowel
     * @param vowel2 (string): arpabet Vowel
    */
    static void add_edge(const std::string& vowel1, const std::string& vowel2);

    /**
     * Runs a Breadth-First-Search to find the shortest path between two vowels.
     * 
     * @param vowel1 (string): arpabet Vowel
     * @param vowel2 (string): arpabet Vowel
     * @return (int) hex-distance between vowels;
    */
    static int calculate_shortest_distance(const std::string& vowel1, const std::string& vowel2);

    /**
     * Calculates all the distances between vowels and adds them to distance_between_vowels_map
     * 
    */
    static void calculate_all_distances();

    /**
     * Initializes this class with hard-coded vowel connections.
     * Also pre-calculates all the distances between vowels.
     * 
    */
    static void initialize();

    /**
     * Returns a vector of vowels connected to a particular vowel node.
     * Currently only used for testing.
     * 
     * @param vowel (string): arpabet Vowel
     * @return vector of connected arpabet vowels
    */
    static std::vector<std::string> get_connected_vowels(const std::string& vowel);

    /**
     * This checks the pre-calculated distances using distance_between_vowels_map;
     * 
     * Speed test:
     * for 10,000 searches:
     * calculate_shortest_distance: 131 milliseconds
     * get_distance: 9 milliseconds
     * 
     * Note: same answers for running this in either direction.
     * 
     * 
     * @param vowel1 (string): arpabet vowel
     * @param vowel2 (string): arpabet vowel
     * @return (int) hex-distance from vowel1 to vowel2
    */
   
    static int get_distance(const std::string& vowel1, const std::string& vowel2);

    /**
     * TODO: How to define vowel distance between diphthongs?
     * What is the distance between MY and MAW (similar start), or MY and MIT (same end), or MY and ME (similar end)? Is one of those pairs further/closer?
     * Or BOY and BALL (same start), or BOY and BIH (same end), BOY and BULL or BOY-F(riend) and BUFF (in between).
     * My suspicion is that the end point of the diphthong plays a greater role in the "flavor" of the vowel.
     */
      
};
