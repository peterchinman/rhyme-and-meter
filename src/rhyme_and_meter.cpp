#ifdef __EMSCRIPTEN__
#include <emscripten/bind.h>
#endif

#include "rhyme_and_meter.hpp"

#include "Hirschberg.hpp"
#include "distance.hpp"

#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <expected>

std::expected<std::vector<std::string>, Phonetic::Error> Rhyme_and_Meter::word_to_phones(const std::string& word){
    return dict.word_to_phones(word);
}

std::string getErrorMessage(MeterError error) {
    switch(error) {
        case MeterError::NestedOptional:
            return "Invalid meter: nested optional sections are not allowed";
        case MeterError::UnclosedOptional:
            return "Invalid meter: unclosed optional section";
        case MeterError::UnrecognizedCharacter:
            return "Invalid meter: unrecognized character";
    }
    return "Unknown meter error";
}

std::expected<std::set<std::vector<int>>, MeterError> Rhyme_and_Meter::fuzzy_meter_to_binary_set(const std::string& meter){
    // the pair here is so we can record whether this specific optional path is currently ACTIVE
    // initialize it with an empty vec and false, so that it's loaded up and ready to go;
    std::vector<std::pair<std::vector<int>, bool>> possible_paths{std::make_pair(std::vector<int>{}, false)};
    bool in_optional{false};

    // start iterating over the meter
    for(const auto& c : meter){

        if (std::isspace(static_cast<unsigned char>(c))) continue;

        else if (c == 'x'){
            if (in_optional){
                for (auto & path : possible_paths){
                    if (path.second == true) {
                        path.first.emplace_back(0);
                    }
                }
            }
            else{
                for (auto & path : possible_paths) {
                    path.first.emplace_back(0);
                }
            }
        }
        else if (c == '/'){
            if (in_optional) {
                for (auto & path : possible_paths){
                    if (path.second == true) {
                        path.first.emplace_back(1);
                    }
                }
            }
            else {
                for (auto & path : possible_paths) {
                    path.first.emplace_back(1);
                }
            }
        }
        else if (c == '(') {
            if (in_optional) {
                return std::unexpected(MeterError::NestedOptional);
            }
            in_optional = true;
            // copy all the current optional paths, setting their bool flag to true
            // so that every optional path we come to, we fork to both take the option and ignore it

            std::vector<std::pair<std::vector<int>, bool>> new_paths;
            for(auto & path : possible_paths) {
                new_paths.emplace_back(std::make_pair(path.first, true));
            }

            possible_paths.insert(possible_paths.end(), new_paths.begin(), new_paths.end());
        }
        else if(c == ')') {
            if(in_optional != true) {
                return std::unexpected(MeterError::UnclosedOptional);
            }
            in_optional = false;

            // set all optional paths bool flags to false
            for(auto & path : possible_paths) {
                path.second = false;
            }

        }
        else {
            // unrecognized character input
            return std::unexpected(MeterError::UnrecognizedCharacter);
        }
    }

    // record results, using a set so that we don't get duplicates
    if (in_optional) {
        return std::unexpected(MeterError::UnclosedOptional);
    }

    std::set<std::vector<int>> meters_set;
    for(const auto & path : possible_paths) {
        meters_set.insert(path.first);
    }

    return meters_set;
}

Rhyme_and_Meter::Check_Validity_Result Rhyme_and_Meter::check_meter_validity(const std::string& text, const std::string& meter_to_check) {

    Check_Validity_Result result{};

    std::vector<std::vector<int>> reference_meters{};
    {
        const auto meters_set_result{fuzzy_meter_to_binary_set(meter_to_check)};
        if (!meters_set_result) {
            result.is_valid = false;
            return result;
        }
        const auto& meters_set = meters_set_result.value();

        for (const auto & meter : meters_set ) {
            reference_meters.emplace_back(meter);
        }
    }

    auto phones{dict.text_to_phones(text)};

    // as we go into this loop over each word in the text here we need two vec of vec<int>s:
        // 1. our reference copy of meters
        // 2. the one where we copy meters that get matched with a particular pronunciation
    // std::vector<std::vector<int>> reference_meters declared above
    std::vector<std::vector<int>> matched_meters{};
    // iterate over each word in our text
    for (const auto & word : phones) {

        // if there are unrecognized words, add them to our result Struct and skip
        if (!word.pronunciations) {
            result.unrecognized_words.emplace_back(word.word);
            result.is_valid = false;
            continue;
        }

        // we keep a record of stress patterns we've seen so that we don't cause unnecessary forks
        std::unordered_set<std::string> stress_patterns_observed{};
        // iterate over each pronunciation of this word
        for( const auto & pronunciation : word.pronunciations.value()) {
            std::string stress_pattern = dict.phone_to_stress(pronunciation);
            const auto check_insert = stress_patterns_observed.insert(stress_pattern);
            // if insert failed it's because we've seen this stress pattern before and we can skip this pronunciation;
            if (check_insert.second == false) {
                continue;
            }

            if (stress_pattern.size() == 1) {
                // single syllabe so yes it matches all of our Possible Meters
                for (const auto & meter : reference_meters) {
                    if (meter.size() < 1) {
                        continue;
                    }
                    auto copy{meter};
                    // trim the head
                    copy.erase(copy.begin());
                    matched_meters.emplace_back(copy);
                }
            }

            // check if word is multisyllabic, because these are the only ones we actually need to validate
            if (stress_pattern.size() > 1) {

                // Our question at this point is: does this particular stress pattern match any of our Possible Meter?

                // iterate over our reference_meters
                for(auto & this_meter : reference_meters) {
                    // first make sure meter.size() remaining is >= stress.size() remaining
                    if(this_meter.size() < stress_pattern.size()) {
                        // fail! skip.
                        continue;
                    }

                    // need a flag to be able to check if this meter was valid for this stress:
                    bool meter_stress_pair_valid = true;

                    // step thru the stress of this pronunciation of this word
                    // we want to check the stress_pattern against stess_pattern.size worth of each meter
                    for (std::size_t i{}; i < stress_pattern.size(); ++i ) {

                        // THIS IS THE HEART OF THE LOGIC. CONTAINING VARIOUS EXCEPTIONS.

                        if (stress_pattern.at(i) == '1' || stress_pattern.at(i) == '2'){
                            // If there's a 2 stress followed by a 1 stress, we should treat the 2 as ambiguously stressed, i.e. pass it regardless of the meter.
                            // e.g. "ISCHEMIC" ("210") could fit "//x" OR "x/x"
                            if (stress_pattern.size() > i + 2
                            && stress_pattern.at(i) == '2'
                            && stress_pattern.at(i+1) == '1') {
                                continue;
                            }

                            // Also want to pass a 2 stress following a 1 stress, because its also ambiguous.
                            if ( i > 1
                            && stress_pattern.at(i) == '2'
                            && stress_pattern.at(i-1) == '1') {
                                continue;
                            }

                            // Other than these ambiguous scenarios, if we find a stress, does it match a stress in the mter?
                            if (this_meter.at(i) == 1) {
                                // great success!
                                continue;
                            }
                            else {
                                meter_stress_pair_valid = false;
                            }
                        }
                        // TODO this else might make the validator too strict?? Figure out how you want to handle it.
                        // This else make it so that multi-syllable words unstressed syllables must strictly match unstressed syllables.
                        else {
                            if(this_meter.at(i) == 0) {
                                // success
                                continue;
                            }
                            else {
                                meter_stress_pair_valid = false;
                            }
                        }
                    }

                    if (meter_stress_pair_valid) {
                        // Success!
                        // Now, copy this meter, eat the front stress_pattern.size() of this meter and then move it over from reference to our collection of matched_meters
                        auto copy{this_meter};
                        copy.erase(copy.begin(), copy.begin() + stress_pattern.size());
                        matched_meters.emplace_back(copy);
                    }
                }
            }
        }

        // check if matched_meters is empty
        if (matched_meters.empty()){
            // Failure!
            result.is_valid = false;
            return result;
        }
        // before we move on to the next word, matched_meters needs to get copied over to reference_meters
        reference_meters = matched_meters;
        matched_meters.clear();
    }

    // OK great we've iterated thru all the words, reference_meters now contains all our remaining Possible Meters. If any of them are empty, that's a success!
    for (const auto & meter: reference_meters) {
        if (meter.empty()){
            result.is_valid = true;
            return result;
        }
    }
    result.is_valid = false;
    return result;

}

Rhyme_and_Meter::Check_Validity_Result Rhyme_and_Meter::check_syllable_validity(const std::string& text, int syllable_count) {

    Check_Validity_Result result{};

    // go thru each word in text
        // if not found in dictionary, add it to the Result

        // go thru each pronunciation of each word in text
            // if pronunciations have different syllable counts, record these
            // we need a different branch for each possible syllable count

    std::vector<int> syllable_count_paths{syllable_count};
    auto phones{dict.text_to_phones(text)};

    // as we go into this loop over each word in the text here we need two vec<int>s:
        // 1. our reference copy of the syllable_count
        // 2. the one where we copy syllable_counts that get matched with a particular pronunciation
    std::vector<int> matched_syllable_paths{};

    // iterate over each word in our text
    for (const auto & word : phones) {

        // if there are unrecognized words, add them to our result Struct and skip
        if (!word.pronunciations) {
            result.unrecognized_words.emplace_back(word.word);
            result.is_valid = false;
            continue;
        }

        // record any syllable counts we have already seen for this word, so that we don't cause unnecessary forks
        std::vector<int> syllable_counts_observed {};

        for (const auto & pronunciation : word.pronunciations.value()) {

            int pronunciation_syllable_count = dict.phone_to_syllable_count(pronunciation);

            // if we've already come across this syllable count, skip
            if (std::find(syllable_counts_observed.begin(), syllable_counts_observed.end(), pronunciation_syllable_count) != syllable_counts_observed.end()) {
                continue;
            }

            // otherwise this is a new syllable count for this pronunciation
            syllable_counts_observed.push_back(pronunciation_syllable_count);

            // iterate thru reference syllables
            for (const auto path : syllable_count_paths) {
                // if the syllable count for this pronuncitation is larger than the count, we can skip
                if (path < pronunciation_syllable_count) {
                    continue;
                }
                // otherwise, copy this syllable-count-path, subtract the number of syllables in this pronunciation, and move it over to our collection of matched_syllable_paths
                else {
                    int copy{path - pronunciation_syllable_count};
                    matched_syllable_paths.emplace_back(copy);
                }
            }
        }

        // check if matched_syllable_paths is empty
        if (matched_syllable_paths.empty()) {
            // failure!
            result.is_valid = false;
            return result;
        }

        // before we move on to the next word, matched_meters needs to get copied over to reference_meters
        syllable_count_paths = matched_syllable_paths;
        matched_syllable_paths.clear();
    }

    // OK great we've iterated thru all the words, syllable_count_ahts now contains all our remaining Possible Meters. If any of them == 0, that's a success!
    for (const int syllable_count : syllable_count_paths) {
        if (syllable_count == 0){
            result.is_valid = true;
            return result;
        }
    }
    result.is_valid = false;
    return result;
}

std::expected<std::pair<std::vector<std::string>, std::vector<std::string>>, Rhyme_and_Meter::UnidentifiedWords> 
Rhyme_and_Meter::compare_end_line_rhyming_parts(const std::string& line1, const std::string& line2) {
    std::pair<std::vector<std::string>, std::vector<std::string>> result{};
    std::vector<std::string> unindentified_words{};

    // get the last word of each line
    std::istringstream iss1{line1};
    std::string last_word1;
    while(iss1 >> last_word1){
        continue;
    }

    std::istringstream iss2{line2};
    std::string last_word2;
    while(iss2 >> last_word2){
        continue;
    }

    // get pronunciations of each word
    auto phones1 = dict.word_to_phones(last_word1);
    auto phones2 = dict.word_to_phones(last_word2);
    if (!phones1 || !phones2) {
        unindentified_words.emplace_back(phones1.error().unidentified_word);
        unindentified_words.emplace_back(phones2.error().unidentified_word);
        return std::unexpected(UnidentifiedWords{unindentified_words});
    }

    std::vector<std::string> pronunciations1 = phones1.value();
    std::vector<std::string> pronunciations2 = phones2.value();

    // get rhyming part of each pronunciation
    std::vector<std::string> rhyming_parts1{};
    std::vector<std::string> rhyming_parts2{};

    for (const auto& p : pronunciations1) {
        rhyming_parts1.emplace_back(dict.get_rhyming_part(p));
    }
    for (const auto& p : pronunciations2) {
        rhyming_parts2.emplace_back(dict.get_rhyming_part(p));
    }

    // get syllable length of shortest rhyming part
    int shortest_length{dict.phone_to_syllable_count(rhyming_parts1[0])};
    for(const auto& r : rhyming_parts1){
        if (dict.phone_to_syllable_count(r) < shortest_length) {
            shortest_length = dict.phone_to_syllable_count(r);
        }
    }
    for(const auto& r : rhyming_parts2){
        if (dict.phone_to_syllable_count(r) < shortest_length) {
            shortest_length = dict.phone_to_syllable_count(r);
        }
    }

    // cut off front of each rhyming part until they are the length of shortest rhyming part
    for(auto& r : rhyming_parts1) {
        if (dict.phone_to_syllable_count(r) > shortest_length) {
            // get a reverse iterator pointing at the number in the last stressed vowel
            auto r_it {std::find_if(r.rbegin(), r.rend(), [](char c) {
                return std::isdigit(c) && c != '0';
            })};
            // progress to correct vowel
            for(int i{}; i < shortest_length - 1; ++i) {
                ++r_it;
                auto r_it_more{std::find_if(r_it, r.rend(), [](char c) {
                    return std::isdigit(c) && c != '0';
                })};
                std::swap(r_it, r_it_more);
            }
            // move it up to the first char of our vowel
            r_it += 2;
            // turn it around
            // base moves us one to the right, so we got to step it back
            auto f_it = r_it.base() - 1;
            std::string clipped_string(f_it, r.end());
            r = clipped_string;
        }
    }
    // TODO: DRY
    for(auto& r : rhyming_parts2) {
        if (dict.phone_to_syllable_count(r) > shortest_length) {
            // get a reverse iterator pointing at the number in the last stressed vowel
            auto r_it {std::find_if(r.rbegin(), r.rend(), [](char c) {
                return std::isdigit(c);
            })};
            // progress to correct vowel
            for(int i{}; i < shortest_length - 1; ++i) {
                ++r_it;
                auto r_it_more{std::find_if(r_it, r.rend(), [](char c) {
                    return std::isdigit(c);
                })};
                std::swap(r_it, r_it_more);
            }
            // move it up to the first char of our vowel
            r_it += 2;
            // turn it around
            // base moves us one to the right, so we got to step it back
            auto f_it = r_it.base() - 1;
            std::string clipped_string(f_it, r.end());
            r = clipped_string;
        }
    }

    result = std::make_pair(rhyming_parts1, rhyming_parts2);
    return result;
}

int Rhyme_and_Meter::minimum_rhyme_distance(const std::pair<std::vector<std::string>, std::vector<std::string>>& pair_of_possible_pronunciations) {
    int minimum_distance{};
    bool first_flag{true};

    for(const auto& p1 : pair_of_possible_pronunciations.first) {
        for(const auto & p2 : pair_of_possible_pronunciations.second) {
            int distance = levenshtein_distance(p1, p2);
            if(first_flag) {
                minimum_distance = distance;
                first_flag = false;
            }
            else {
                if(distance < minimum_distance){
                    minimum_distance = distance;
                }
            }
        }
    }

    return minimum_distance;
}

Alignment_And_Distance Rhyme_and_Meter::minimum_alignmment(const std::pair<std::vector<std::string>, std::vector<std::string>>& pair_of_possible_pronunciations) {
    Alignment_And_Distance minimum_alignment{};
    bool first_flag{true};

    std::vector<std::vector<std::string>> pronunciations1{};


    for(const auto& p1 : pair_of_possible_pronunciations.first) {
        for(const auto & p2 : pair_of_possible_pronunciations.second) {
            Alignment_And_Distance pair_alignment = Hirschberg(phones_string_to_vector(p1), phones_string_to_vector(p2));
            if(first_flag) {
                minimum_alignment = pair_alignment;
                first_flag = false;
            }
            else {
                if(pair_alignment.distance < minimum_alignment.distance){
                    minimum_alignment = pair_alignment;
                }
            }
        }
    }

    return minimum_alignment;
}

std::expected<int, Rhyme_and_Meter::UnidentifiedWords> 
Rhyme_and_Meter::get_end_rhyme_distance(const std::string& line1, const std::string& line2) {
    auto rhyming_parts = compare_end_line_rhyming_parts(line1, line2);
    if (!rhyming_parts) {
        return std::unexpected(rhyming_parts.error());
    }
    return minimum_rhyme_distance(rhyming_parts.value());
}

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_BINDINGS(my_module) {

    emscripten::register_vector<std::string>("StringVector");

    emscripten::enum_<MeterError>("MeterError")
        .value("NestedOptional", MeterError::NestedOptional)
        .value("UnclosedOptional", MeterError::UnclosedOptional)
        .value("UnrecognizedCharacter", MeterError::UnrecognizedCharacter);

    emscripten::function("getErrorMessage", &getErrorMessage);

    emscripten::class_<Rhyme_and_Meter>("Rhyme_and_Meter")
        .constructor<>()
        .function("word_to_phones", &Rhyme_and_Meter::word_to_phones)
        .function("check_syllable_validity", &Rhyme_and_Meter::check_syllable_validity)
        .function("check_meter_validity", &Rhyme_and_Meter::check_meter_validity)
        .function("get_end_rhmye_distance", &Rhyme_and_Meter::get_end_rhyme_distance)
        ;

    emscripten::value_object<Rhyme_and_Meter::Check_Validity_Result>("Check_Validity_Result")
        .field("is_valid", &Rhyme_and_Meter::Check_Validity_Result::is_valid)
        .field("unrecognized_words", &Rhyme_and_Meter::Check_Validity_Result::unrecognized_words)
        ;


}
#endif
