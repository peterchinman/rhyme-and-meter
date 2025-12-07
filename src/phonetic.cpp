#ifdef __EMSCRIPTEN__
#include <emscripten/bind.h>
#endif

#include "phonetic.hpp"
#include "phonetic_utils.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

Phonetic::Phonetic() {
    import_dictionary();
}

bool Phonetic::import_dictionary() {

    std::string file_path = CMU_DICT_PATH;
    
    std::ifstream cmudict{file_path};
    if (!cmudict.is_open()) {
        std::cerr << "Failed to open the dictionary." << '\n';
        return false;
    }
    std::string line;
    while (std::getline(cmudict, line)) {
        // ignore leading ;
        if(line.empty() || line[0] == ';') {
            continue;
        }

        std::istringstream iss(line);

        // extract word up to white space   
        std::string word;
        iss >> word;

        // strip variation "(n)", used in CMU DICT for multiple entries of same word
        // instead each pronunciation is added to the vector m_dictionary[word]
        if (word.back() == ')'){
            word.pop_back();
            word.pop_back();
            word.pop_back();
        }
       
        // pronunciation is ARPABET symbols, separated by spaces
        // vowels end with a number indicating stress, 0 no stress, 1 primary stress, 2 secondary stress
        std::string pronunciation;
        std::getline(iss, pronunciation);

        // trim white space
        ltrim(pronunciation);
        rtrim(pronunciation);

        m_dictionary[word].push_back(pronunciation);
    }

    cmudict.close();
    return true;
}

std::expected<std::vector<std::string>, Phonetic::Error> Phonetic::word_to_phones(std::string word) {
    // capitalize all queries
    std::transform(word.begin(), word.end(), word.begin(), ::toupper);
    auto it = m_dictionary.find(word);
    if (it != m_dictionary.end()) {
        return it->second;
    }
    else {
        return std::unexpected(Error{word});
    }
}

Phonetic::TextToPhonesResult Phonetic::text_to_phones(const std::string & text) {
    TextToPhonesResult results{};
    std::vector<std::string> words {strip_punctuation(text)};

    for (const auto & w : words) {
        auto phones = word_to_phones(w);
        if (!phones.has_value()) {
            results.failed_words.push_back(phones.error().unidentified_word);
        }
        
        results.words_with_pronunciations.push_back(std::make_pair(w, phones.value_or(std::vector<std::string>{})));

    }

    return results;
}

std::string Phonetic::phones_to_stresses(const std::string& phones) {
    std::string stresses{};
    for (const auto & c : phones){
        if (c == '0' || c == '1' || c == '2') {
            stresses.push_back(c);
        }
    }
    return stresses;
}

std::expected<std::vector<std::string>, Phonetic::Error> Phonetic::word_to_stresses(const std::string& word) {
    std::vector<std::string> stresses{};

    auto phones = word_to_phones(word);
    if (!phones) {
        return std::unexpected(phones.error());
    }

    for (const auto & p : phones.value()) {
        stresses.emplace_back(phones_to_stresses(p));
    }
    return stresses;
}

int Phonetic::phone_to_syllable_count(const std::string& phones) {
    return static_cast<int>(phones_to_stresses(phones).length());
}

std::expected<std::vector<int>, Phonetic::Error> Phonetic::word_to_syllable_counts(const std::string& word) {
    std::vector<int> syllables;
    
    auto phones = word_to_phones(word);
    if (!phones) {
        return std::unexpected(phones.error());
    }

    for(const auto & p : phones.value()) {
        syllables.emplace_back(phone_to_syllable_count(p));
    }
    return syllables;
}

std::string Phonetic::get_rhyming_part(const std::string& phones) {
    std::string result{};
    // if we were using C++23 we could use std::ranges::find_last_if, but we're not

    // get a reverse iterator pointing at the number in the last stressed vowel
    auto r_it {std::find_if(phones.rbegin(), phones.rend(), [](char c) {
        return std::isdigit(c) && c != '0';
    })};
    // if we found it
    if (r_it != phones.rend()) {
        // move it up to the first char of our vowel
        r_it += 2;
        // turn it around
        // base moves us one to the right, so we got to step it back
        auto f_it = r_it.base() - 1;
        std::copy(f_it, phones.end(), std::back_inserter(result));
        return result;
    }
    // but if the word has no stresses, we'll return from just the last vowel onward
    auto no_stress_r_it {std::find_if(phones.rbegin(), phones.rend(), [](char c) {
        return std::isdigit(c);
    })};

    if (no_stress_r_it != phones.rend()) {
        // move it up to the first char of our vowel
        no_stress_r_it += 2;
        // turn it around
        // base moves us one to the right, so we got to step it back
        auto f_it = no_stress_r_it.base() - 1;
        std::copy(f_it, phones.end(), std::back_inserter(result));
        return result;
    }

    // else, there are no vowels at all, so we return an empty string
    return result;
}

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_BINDINGS(my_module) {
    emscripten::register_vector<std::string>("StringVector");

    emscripten::class_<Phonetic::Error>("Error")
        .property("unidentified_word", &Phonetic::Error::unidentified_word);

    emscripten::class_<Phonetic::TextToPhonesResult>("TextToPhonesResult")
        .property("words_with_pronunciations", &Phonetic::TextToPhonesResult::words_with_pronunciations)
        .property("failed_words", &Phonetic::TextToPhonesResult::failed_words);

    emscripten::class_<Phonetic>("Phonetic")
        .constructor<>()
        .function("word_to_phones", &Phonetic::word_to_phones)
        .function("text_to_phones", &Phonetic::text_to_phones)
        .function("phones_to_stresses", &Phonetic::phones_to_stresses)
        .function("word_to_stresses", &Phonetic::word_to_stresses)
        .function("phone_to_syllable_count", &Phonetic::phone_to_syllable_count)
        .function("word_to_syllable_counts", &Phonetic::word_to_syllable_counts)
        .function("get_rhyming_part", &Phonetic::get_rhyming_part)
        ;
}
#endif
