/**
 * Adapted by Peter Chinman, in November 2024, to process vector<string>s instead of strings, allowing for sequence alignment of CMU Pronouncing Dictionary ARPABET symbols.
 * 
 * Also cleaned up code for pedantic compiler warnings, (e.g. std::size_t for int, vectors of vectors for variable length arrays)
 * 
 * Uses externally defined functions SUBSTITUTION_SCORE() and GAP_SCORE() to evaluate scores.
 * 
 * Outputs a struct containing output.ZWpair => the aligned words, as a pair of vectors of strings, as well as output.distance => the weighted levenshtein edit distance between strings.
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
 */

#pragma once

#include "distance.hpp"

#include <iostream>
#include <vector>
#include <cstring>
#include <cmath>

struct Alignment_And_Distance {
   std::pair< std::vector<std::string>, std::vector<std::string> > ZWpair{};
   int distance{};
};

//Useful tools
inline int min3(int a, int b, int c);

// Prints out the ZWpair that we get back from hirschberg
inline void print_pair(std::pair< std::vector<std::string>, std::vector<std::string> > ZWpair ) {
    for (const auto & symbol : ZWpair.first) {
        std::cout << symbol;
        for(std::size_t i{}; i < 4 - symbol.size(); ++i){
            std::cout << " ";
        }
    }
    std::cout << std::endl;
    for (const auto & symbol : ZWpair.second) {
        std::cout << symbol;
        for(std::size_t i{}; i < 4 - symbol.size(); ++i){
            std::cout << " ";
        }
    }
    std::cout << std::endl;
}

//overload pair sum
inline std::pair<std::vector<std::string>, std::vector<std::string>> operator+(std::pair<std::vector<std::string>, std::vector<std::string>> const& one, std::pair<std::vector<std::string>, std::vector<std::string>> const& two);

//sum_vectors: sum vector function
inline std::vector <int> sum_vectors(const std::vector<int>& v1, const std::vector<int>& v2);

//NWScore: return last line of score matrix
inline std::vector<int> NWScore(const std::vector<std::string>& X, const std::vector<std::string>& Y);

//NeedlemanWunsch: returns the alignment pair with standard algorithm
inline Alignment_And_Distance NeedlemanWunsch(const std::vector<std::string>& X, const std::vector<std::string>& Y);

//argmax_element: returns position of max element in the vector argument
inline std::size_t argmin_element(const std::vector<int> score);



//hirschberg: main algorithm; returns alignments-pair space-efficiently
// Feed it two vectors of strings of ARPABET phones.
// TODO standardize this to use space-separated strings so that it aligns with CMUdict
inline Alignment_And_Distance hirschberg(const std::vector<std::string>& X, const std::vector<std::string>& Y);

//Functions
//Return minimum of three integers
int min3(int a, int b, int c)
{
    if (a <= b && a <= c) return a;
    else if (b <= a && b <= c) return b;
    else return c;
}

std::vector<int> NWScore(const std::vector<std::string>& X, const std::vector<std::string>& Y)
{
    const int n = X.size();
    const int m = Y.size();
    std::vector<std::vector<int>> Score(n+1, std::vector<int>(m+1, 0));
    std::vector<int> Lastline;
    
    //Step 1: start from zero
    Score[0][0]=0;
    
    //Step 1.1: first row penalties
    for (int j=1;j<=m;j++)
    {
        Score[0][j] = Score[0][j-1] + GAP_PENALTY(Y[j-1]);
    }
   
    for (int i=1; i<=n;i++)
    {
        Score[1][0] = Score[0][0] + GAP_PENALTY(X[i-1]);
        for (int j=1; j<=m;j++)
        {
            Score[1][j] = min3(
                               Score[1][j-1] + GAP_PENALTY(Y[j-1]),
                               Score[0][j] + GAP_PENALTY(X[i-1]),
                               Score[0][j-1] + SUBSTITUTION_SCORE(X[i-1], Y[j-1])
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

Alignment_And_Distance NeedlemanWunsch (const std::vector<std::string>& X, const std::vector<std::string>& Y)
{
    Alignment_And_Distance alignment_and_distance{};

    const int n = X.size(), m = Y.size();

    std::vector<std::vector<int>> M(n+1, std::vector<int>(m+1, 0));
    // int M[n+1][m+1];
    //STEP 1: assign first row and column
    M[0][0] = 0;
    for (int i=1;i<n+1;i++)
    {
        M[i][0] = M[i-1][0] + GAP_PENALTY(X[i-1]);
    }
    for (int i=1;i<m+1;i++)
    {
        M[0][i] = M[0][i-1] + GAP_PENALTY(Y[i-1]);
    }
    
    //STEP 2: Needelman-Wunsch
    for (int i=1;i<n+1;i++)
    {
        for (int j=1;j<m+1;j++)
        {
            M[i][j] = min3(M[i-1][j-1] + SUBSTITUTION_SCORE(X[i-1], Y[j-1]),
                          M[i][j-1] + GAP_PENALTY(Y[j-1]),
                          M[i-1][j] + GAP_PENALTY(X[i-1]));
        }
    }

    // Set distance
    alignment_and_distance.distance = M[n][m];

    
    //STEP 3: Reconstruct alignment
    std::vector<std::string> A_1{};
    std::vector<std::string> A_2{};
    int i = n, j = m;
    while (i>0 || j>0)
    {
        if (i>0
            && j>0
            && (M[i][j] == M[i-1][j-1] + SUBSTITUTION_SCORE(X[i-1], Y[j-1])))
        {
            A_1.insert(A_1.begin(), X[i-1]);
            A_2.insert(A_2.begin(), Y[j-1]);
            i--;
            j--;
        }

        else if (i>0
            && (M[i][j] == M[i-1][j] + GAP_PENALTY(X[i-1])))
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
    
    alignment_and_distance.ZWpair.first = A_1;
    alignment_and_distance.ZWpair.second = A_2;
    return alignment_and_distance;
}


std::vector<int> sum_vectors(const std::vector<int>& v1, const std::vector<int>& v2)
{
    std::vector<int> vector_sum;
    if(v1.size() != v2.size())
    {
        std::cerr << "In vector sum: vector dimensions are not equal!" << std::endl;
        std::exit(EXIT_FAILURE);
    }
    for (std::size_t i=0; i < v1.size();i++)
    {
        vector_sum.push_back(v1[i] + v2[i]);
    }
    
    return vector_sum;
}


std::size_t argmin_element(const std::vector<int> score)
{
    int min = score[0];
    std::size_t min_index=0;
    for (std::size_t i=1; i < score.size();i++)
    {
        if(min > score[i])
        {
            min = score[i];
            min_index = i;
        }
    }
    
    return min_index;
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



Alignment_And_Distance hirschberg(const std::vector<std::string>& X, const std::vector<std::string>& Y)
{
    Alignment_And_Distance alignment_and_distance{};

    const int n = X.size();
    const int m = Y.size();
    std::pair< std::vector<std::string>, std::vector<std::string> > ZWpair{};
    
    
    if (n==0)
    {
        for (int i=1; i<=m; i++)
        {
            ZWpair.first.emplace_back("-");
            ZWpair.second.emplace_back(Y[i-1]);
            alignment_and_distance.distance += GAP_PENALTY(Y[i-1]);
        }
        
    }
    
    else if (m==0)
    {
        for (int i=1; i<=n; i++)
        {
            ZWpair.first.emplace_back(X[i-1]);
            ZWpair.second.emplace_back("-");
            alignment_and_distance.distance += GAP_PENALTY(X[i-1]);
        }
    }
    
    else if (n==1 || m ==1)
    {
        alignment_and_distance = NeedlemanWunsch(X,Y);
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
        for (std::size_t i=1;i<=scoreR.size();i++)
        {
            scoreR_rev.push_back(scoreR[scoreR.size()-i]);
        }
        
        auto vector_sum = sum_vectors(scoreL, scoreR_rev);
        const std::size_t ymid = argmin_element(vector_sum);
        alignment_and_distance.distance = *std::min_element(vector_sum.begin(), vector_sum.end());
        
        //DEBUG
        #ifdef DEBUG
            std::cout << "ymid : " << ymid << std::endl;
        #endif //DEBUG
        
        //generate Y[1...ymid]
        for (std::size_t i=0;i<ymid;i++)
        {
            Y_to_ymid.emplace_back(Y[i]);
        }
        
        //reverse X[xmid+1 ... n]
        // ^^ original comment, but seems wrong
        for (int i=ymid;i<m;i++)
        {
            Y_from_ymid.emplace_back(Y[i]);
        }
        
        
        ZWpair = hirschberg(X_to_xmid, Y_to_ymid).ZWpair + hirschberg(X_from_xmid, Y_from_ymid).ZWpair;
        alignment_and_distance.ZWpair = ZWpair;
    }
    return alignment_and_distance;
}


#ifdef __EMSCRIPTEN__
EMSCRIPTEN_BINDINGS(hirschberg) {

    emscripten::register_vector<std::string>("StringVector");

    // Bind the pair of vectors
    emscripten::value_object<std::pair<std::vector<std::string>, std::vector<std::string>>>("StringVectorPair")
        .field("first", &std::pair<std::vector<std::string>, std::vector<std::string>>::first)
        .field("second", &std::pair<std::vector<std::string>, std::vector<std::string>>::second)
    ;

    // Bind the struct
    emscripten::value_object<Alignment_And_Distance>("Alignment_And_Distance")
        .field("ZWpair", &Alignment_And_Distance::ZWpair)
        .field("distance", &Alignment_And_Distance::distance)
    ;

    // Bind the function
    emscripten::function("hirschberg", &hirschberg);

    
}
#endif
