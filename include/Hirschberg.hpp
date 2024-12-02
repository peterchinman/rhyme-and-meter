/**
 * Adapted by Peter Chinman, in November 2024, to process vector<string>s instead of strings, allowing for sequence alignment of CMU Pronouncing Dictionary ARPABET symbols, which strings between 1 and 3 characters.
 * 
 * MATCH_OR_MISMATCH_SCORE also updated to get vowel and (TODO) consonant distance.
 * 
 * 
 * Hirschberg Algorithm for Sequence Alignment
 * Author: Francesco Borando
 * Date: May 2022
 * University of Milan, Department of Physics
 *
 * This C++ code implements the Hirschberg algorithm for global sequence alignment.
 * The algorithm is an improvement over the Needleman-Wunsch algorithm in terms of space complexity.
 * It uses a divide-and-conquer strategy to achieve linear space complexity, making it more memory-efficient.
 * The Hirschberg algorithm is particularly suitable for aligning long sequences.
 *
 * References:
 * - Hirschberg, D. S. (1975). A linear space algorithm for computing maximal common subsequences.
 *   Communications of the ACM, 18(6), 341–343.
 *
 * Usage:
 * - Compile and run the code, providing input sequences as argv[1] and argv[2].
 * - Adjust parameter scores as desired.
 * - The output will include the aligned sequences.
 *
 */



#pragma once

#include "VowelHexGraph.h"

#include <iostream>
#include <vector>
#include <cstring>
#include <cmath>

inline int GAP_PENALTY() {
   return -10;
}

inline int MATCH_OR_MISMATCH_SCORE(const std::string& s1, const std::string& s2) {
   const int MATCH_SCORE = 0;
   const int SAME_VOWEL_DIFFERENT_STRESS = -1;
   const int VOWEL_TO_CONSONANT_MISMATCH = -30;
   const int CONSONANT_MISMATCH = -5;

   if (s1 == s2) {
      return MATCH_SCORE;
   }

   // std::cout << "Comparing" << s1 << " & " << s2 << std::endl;
   // Check whether both are vowels
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
            return vowel_distance - 1; 
         }
         return -1 * vowel_distance;

      }
   }
   // At least one of them is a consonant
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



//Useful tools
inline int max3(int a, int b, int c);
inline int match_or_mismatch(const std::string& s1, const std::string& s2);

//overload pair sum
inline std::pair<std::vector<std::string>, std::vector<std::string>> operator+(std::pair<std::vector<std::string>, std::vector<std::string>> const& one, std::pair<std::vector<std::string>, std::vector<std::string>> const& two);

//sum_vectors: sum vector function
inline std::vector <int> sum_vectors(const std::vector<int>& v1, const std::vector<int>& v2);

//NWScore: return last line of score matrix
inline std::vector<int> NWScore(const std::vector<std::string>& X, const std::vector<std::string>& Y);

//NeedlemanWunsch: returns the alignment pair with standard algorithm
inline std::pair < std::vector<std::string>, std::vector<std::string> > NeedlemanWunsch(const std::vector<std::string>& X, const std::vector<std::string>& Y);

//argmax_element: returns position of max element in the vector argument
inline int argmax_element(const std::vector<int> score);

struct HirschbergReturn {
   std::pair< std::vector<std::string>, std::vector<std::string> > ZWpair{};
   int ymid{};

};

//Hirschberg: main algorithm; returns alignments-pair space-efficiently
inline HirschbergReturn Hirschberg(const std::vector<std::string>& X, const std::vector<std::string>& Y);


// int main(int argc, char* argv[])
// {
//     if(!argv[1] || !argv[2])
//     {
//         std::cerr << "Please, insert sequences to confront:" << std::endl
//                 <<"• Sequence1 as argv[1]" << std::endl
//                 <<"• Sequence1 as argv[2]" << std::endl;
//         std::exit(EXIT_FAILURE);
//     }
    
//     const std::string s1 = argv[1], s2 = argv[2];
//     const int n = s1.size(), m = s2.size();
    
//     std::pair<std::string, std::string> ZWpair = Hirschberg(s1,s2);
//     std::cout << ZWpair.first << std::endl << ZWpair.second << std::endl;
     
//     return 0;
// }


//Functions
//Return maximum of three integers
int max3(int a, int b, int c)
{
    if (a >= b && a >= c) return a;
    else if (b >= a && b >= c) return b;
    else return c;
}

//Evaluate if diagonal outcome of Needleman-Wunsch
int match_or_mismatch(const std::string& s1, const std::string& s2)
{
    return MATCH_OR_MISMATCH_SCORE(s1, s2);
}

std::vector<int> NWScore(const std::vector<std::string>& X, const std::vector<std::string>& Y)
{
    const int n = X.size();
    const int m = Y.size();
    int Score[n+1][m+1];
    std::vector<int> Lastline;
    
    //Step 1: start from zero
    Score[0][0]=0;
    
    //Step 1.1: first row penalties
    for (int j=1;j<=m;j++)
    {
        Score[0][j] = Score[0][j-1] + GAP_PENALTY();
    }
   
    for (int i=1; i<=n;i++)
    {
        Score[1][0] = Score[0][0] + GAP_PENALTY();
        for (int j=1; j<=m;j++)
        {
            Score[1][j] = max3(
                               Score[1][j-1] + GAP_PENALTY(),
                               Score[0][j] + GAP_PENALTY(),
                               Score[0][j-1] + match_or_mismatch(X[i-1],Y[j-1])
                               );
        }
        
        //useless, da portare sotto!
        for (int j=0;j<=m;j++)
        {
            Score[0][j] = Score[1][j];
        }
    }
    
    for (int j=0;j<=m;j++)
    {
        Lastline.push_back( Score[1][j] );
    }
    
    return Lastline;
    
}

std::pair < std::vector<std::string>, std::vector<std::string> > NeedlemanWunsch (const std::vector<std::string>& X, const std::vector<std::string>& Y)
{
    const int n = X.size(), m = Y.size();
    int M[n+1][m+1];
    //STEP 1: assign first row and column
    M[0][0] = 0;
    for (int i=1;i<n+1;i++)
    {
        M[i][0] = M[i-1][0] + GAP_PENALTY();
    }
    for (int i=1;i<m+1;i++)
    {
        M[0][i] = M[0][i-1] + GAP_PENALTY();
    }
    
    //STEP 2: Needelman-Wunsch
    for (int i=1;i<n+1;i++)
    {
        for (int j=1;j<m+1;j++)
        {
            M[i][j] = max3(M[i-1][j-1] + match_or_mismatch(X[i-1], Y[j-1]),
                          M[i][j-1] + GAP_PENALTY(),
                          M[i-1][j] + GAP_PENALTY());
        }
    }

    
    //STEP 3: Reconstruct alignment
    std::vector<std::string> A_1{};
    std::vector<std::string> A_2{};
    int i = n, j = m;
    while (i>0 || j>0)
    {
        if (i>0
            && j>0
            && (M[i][j] == M[i-1][j-1] + match_or_mismatch(X[i-1], Y[j-1])))
        {
            A_1.insert(A_1.begin(), X[i-1]);
            A_2.insert(A_2.begin(), Y[j-1]);
            i--;
            j--;
        }

        else if (i>0
            && (M[i][j] == M[i-1][j] + GAP_PENALTY()))
        {
            A_1.insert(A_1.begin(), X[i-1]);
            A_2.insert(A_2.begin(), "-");
            i--;
        }

        else
        {
            A_1.insert(A_1.begin(), "-");
            A_2.insert(A_2.begin(), Y[j-1]);
            j--;
        }
    }
    
    std::pair < std::vector<std::string>, std::vector<std::string> > alignment_pair;
    alignment_pair.first = A_1;
    alignment_pair.second = A_2;
    return alignment_pair;
}


std::vector<int> sum_vectors(const std::vector<int>& v1, const std::vector<int>& v2)
{
    std::vector<int> vector_sum;
    if(v1.size() != v2.size())
    {
        std::cerr << "In vector sum: vector dimensions are not equal!" << std::endl;
        std::exit(EXIT_FAILURE);
    }
    for (int i=0; i<v1.size();i++)
    {
        vector_sum.push_back(v1[i] + v2[i]);
    }
    
    return vector_sum;
}


int argmax_element(const std::vector<int> score)
{
    int max = score[0];
    int max_index=0;
    for (int i=1; i<score.size();i++)
    {
        if(max < score[i])
        {
            max = score[i];
            max_index = i;
        }
    }
    
    return max_index;
}

//overload pair sum
std::pair<std::vector<std::string>, std::vector<std::string>> operator+(std::pair<std::vector<std::string>, std::vector<std::string>> const& one, std::pair<std::vector<std::string>, std::vector<std::string>> const& two)
{
    std::pair<std::vector<std::string>, std::vector<std::string>> pair_sum;
   //  pair_sum.first = one.first + two.first;
   pair_sum.first = one.first;
   pair_sum.first.insert(pair_sum.first.end(), two.first.begin(), two.first.end());
   //  pair_sum.second = one.second + two.second;
   pair_sum.second = one.second;
   pair_sum.second.insert(pair_sum.second.end(), two.second.begin(), two.second.end());
    return pair_sum;
}



HirschbergReturn Hirschberg(const std::vector<std::string>& X, const std::vector<std::string>& Y)
{
    const int n = X.size();
    const int m = Y.size();
    std::pair< std::vector<std::string>, std::vector<std::string> > ZWpair{};
    HirschbergReturn hirschberg_return{};
    int ymid{};
    
    if (n==0)
    {
        for (int i=1; i<=m; i++)
        {
            ZWpair.first.emplace_back("-");
            ZWpair.second.emplace_back(Y[i-1]);
        }
        
    }
    
    else if (m==0)
    {
        for (int i=1; i<=n; i++)
        {
            ZWpair.first.emplace_back(X[i-1]);
            ZWpair.second.emplace_back("-");
        }
    }
    
    else if (n==1 || m ==1)
    {
        ZWpair = NeedlemanWunsch(X,Y);
    }
    
    else
    {
        const int xmid = n/2; //defect truncation (.5 -> .0)
        std::vector<std::string> X_to_xmid{},
                                 X_from_xmid{},
                                 X_from_xmid_rev{},
                                 Y_to_ymid{},
                                 Y_from_ymid{},
                                 Y_rev{};
        
        //generate x[1...xmid]
        for (int i=0;i<xmid;i++)
        {
            X_to_xmid.emplace_back(X[i]);
        }
        
        //reverse X[xmid+1 ... n] and get normal
        for (int i=0;i<(n-xmid);i++)
        {
            X_from_xmid_rev.emplace_back(X[n-1-i]);
            X_from_xmid.emplace_back(X[xmid+i]);
        }
        
        //reverse Y
        for (int i=1;i<=m;i++)
        {
             Y_rev.emplace_back(Y[m-i]);
        }
        
        std::vector<int> scoreL = NWScore(X_to_xmid,Y);
        std::vector<int> scoreR = NWScore(X_from_xmid_rev,Y_rev);
        std::vector<int> scoreR_rev;
        
        //DEBUG
        #ifdef DEBUG
            std::cout << "ScoreL : ";
            for (int i=0; i<scoreL.size();i++)
            {
                std::cout << scoreL[i] << "\t";
            }
            std::cout << std::endl;
            
            //DEBUG
            std::cout << "ScoreR : ";
            for (int i=0; i<scoreR.size();i++)
            {
                std::cout << scoreR[i] << "\t";
            }
            std::cout << std::endl;
        #endif //DEBUG
        
        //reverse ScoreR
        for (int i=1;i<=scoreR.size();i++)
        {
            scoreR_rev.push_back(scoreR[scoreR.size()-i]);
        }
        
        
        ymid = argmax_element(sum_vectors(scoreL, scoreR_rev));
        hirschberg_return.ymid = ymid;
        
        //DEBUG
        #ifdef DEBUG
            std::cout << "ymid : " << ymid << std::endl;
        #endif //DEBUG
        
        //generate Y[1...ymid]
        for (int i=0;i<ymid;i++)
        {
            Y_to_ymid.emplace_back(Y[i]);
        }
        
        //reverse X[xmid+1 ... n]
        for (int i=ymid;i<m;i++)
        {
            Y_from_ymid.emplace_back(Y[i]);
        }
        
        
        ZWpair = Hirschberg(X_to_xmid, Y_to_ymid).ZWpair + Hirschberg(X_from_xmid, Y_from_ymid).ZWpair;
    }
    hirschberg_return.ZWpair = ZWpair;
    return hirschberg_return;
}
