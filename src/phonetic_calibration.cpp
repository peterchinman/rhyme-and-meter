#include "rhyme_and_meter.hpp"
#include "vowel_hex_graph.hpp"
#include "consonant_distance.hpp"
#include <iostream>
#include <iomanip>
#include <string>



void print_comparison_result(const std::string& test_name,
                           const std::string& word1, const std::string& word2a, const std::string& word2b,
                           double distance1, double distance2,
                           const std::string& intuition, bool passed) {
    std::cout << "\n=== " << test_name << " ===" << std::endl;
    std::cout << "Comparing: " << word1 << " & " << word2a << " vs " << word1 << " & " << word2b << std::endl;
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Distance 1: " << distance1 << " (" << word1 << " & " << word2a << ")" << std::endl;
    std::cout << "Distance 2: " << distance2 << " (" << word1 << " & " << word2b << ")" << std::endl;
    std::cout << "Intuition: " << intuition << std::endl;
    std::cout << "Result: " << (passed ? "✓ PASSES" : "✗ FAILS") << std::endl;
}

int main() {
    Rhyme_and_Meter dict;

    std::cout << "Phonetic Distance Calibration Tests" << std::endl;
    std::cout << "===================================" << std::endl;

    // 1. VOWEL DISTANCE TESTS (only vowel changes)
    std::cout << "\n1. VOWEL DISTANCE TESTS" << std::endl;
    std::cout << "======================" << std::endl;

    // ball & bull vs ball & bile
    auto ball_bull = dict.minimum_text_distance("ball", "bull");
    auto ball_bile = dict.minimum_text_distance("ball", "bile");
    if (ball_bull.has_value() && ball_bile.has_value()) {
        print_comparison_result("Ball vs Bull vs Bile", "ball", "bull", "bile",
                               ball_bull.value(), ball_bile.value(),
                               "bull should be closer to ball than bile (vowel distance)",
                               ball_bull.value() < ball_bile.value());
    }

    // buck & book vs buck & bake
    auto buck_book = dict.minimum_text_distance("buck", "book");
    auto buck_bake = dict.minimum_text_distance("buck", "bake");
    if (buck_book.has_value() && buck_bake.has_value()) {
        print_comparison_result("Buck vs Book vs Bake", "buck", "book", "bake",
                               buck_book.value(), buck_bake.value(),
                               "book should be closer to buck than bake (AH vs UW vs EY)",
                               buck_book.value() < buck_bake.value());
    }

    // luck & lack vs luck & leak
    auto luck_lack = dict.minimum_text_distance("luck", "lack");
    auto luck_leak = dict.minimum_text_distance("luck", "leak");
    if (luck_lack.has_value() && luck_leak.has_value()) {
        print_comparison_result("Luck vs Lack vs Leak", "luck", "lack", "leak",
                               luck_lack.value(), luck_leak.value(),
                               "lack should be closer to luck than leak (AH vs AE vs IY)",
                               luck_lack.value() < luck_leak.value());
    }

    // 2. CONSONANT DISTANCE TESTS (only consonant changes)
    std::cout << "\n2. CONSONANT DISTANCE TESTS" << std::endl;
    std::cout << "==========================" << std::endl;

    // bag & back vs bag & bad
    auto bag_back = dict.minimum_text_distance("bag", "back");
    auto bag_bad = dict.minimum_text_distance("bag", "bad");
    if (bag_back.has_value() && bag_bad.has_value()) {
        print_comparison_result("Bag vs Back vs Bad", "bag", "back", "bad",
                               bag_back.value(), bag_bad.value(),
                               "back should be closer to bag than bad (G vs K vs D)",
                               bag_back.value() < bag_bad.value());
    }

    // buck & bug vs buck & butt
    auto buck_bug = dict.minimum_text_distance("buck", "bug");
    auto buck_butt = dict.minimum_text_distance("buck", "butt");
    if (buck_bug.has_value() && buck_butt.has_value()) {
        print_comparison_result("Buck vs Bug vs Butt", "buck", "bug", "butt",
                               buck_bug.value(), buck_butt.value(),
                               "bug should be closer to buck than butt (K vs G vs T)",
                               buck_bug.value() < buck_butt.value());
    }

    // lack & lag vs lack & laugh
    auto lack_lag = dict.minimum_text_distance("lack", "lag");
    auto lack_laugh = dict.minimum_text_distance("lack", "laugh");
    if (lack_lag.has_value() && lack_laugh.has_value()) {
        print_comparison_result("Lack vs Lag vs Laugh", "lack", "lag", "laugh",
                               lack_lag.value(), lack_laugh.value(),
                               "lag should be closer to lack than laugh (K vs G vs F)",
                               lack_lag.value() < lack_laugh.value());
    }

    // tab & tag vs tab & tap
    auto tab_tag = dict.minimum_text_distance("tab", "tag");
    auto tab_tap = dict.minimum_text_distance("tab", "tap");
    if (tab_tag.has_value() && tab_tap.has_value()) {
        print_comparison_result("Tab vs Tag vs Tap", "tab", "tag", "tap",
                               tab_tag.value(), tab_tap.value(),
                               "tab should be closer to tag than tap (G vs B vs P)",
                               tab_tag.value() > tab_tap.value());
    }

    // 3. VOWEL COEFFICIENT TESTS (vowel change vs consonant change)
    std::cout << "\n3. VOWEL COEFFICIENT TESTS" << std::endl;
    std::cout << "=========================" << std::endl;

    // ball & bull vs ball & bog
    auto ball_bog = dict.minimum_text_distance("ball", "bog");
    if (ball_bull.has_value() && ball_bog.has_value()) {
        print_comparison_result("Ball vs Bull vs Bog", "ball", "bull", "bog",
                               ball_bull.value(), ball_bog.value(),
                               "bull should be closer to ball than bog (vowel change vs consonant change)",
                               ball_bull.value() > ball_bog.value());
    }

    // mere & mar vs mere & meal
    auto mere_meal = dict.minimum_text_distance("mere", "meal");
    auto mere_mar = dict.minimum_text_distance("mere", "mar");
    if (mere_meal.has_value() && mere_mar.has_value()) {
        print_comparison_result("Mere vs Mar vs Meal", "mere", "mar", "meal",
                               mere_mar.value(), mere_meal.value(),
                               "mar should be closer to mere than meal (vowel change vs consonant change)",
                               mere_mar.value() > mere_meal.value());
    }

    // buck & book vs buck & bug
    if (buck_book.has_value() && buck_bug.has_value()) {
        print_comparison_result("Buck vs Book vs Bug", "buck", "book", "bug",
                               buck_book.value(), buck_bug.value(),
                               "book should be closer to buck than bug (vowel change vs consonant change)",
                               buck_book.value() > buck_bug.value());
    }

    // tree & tray vs tree & treat
    auto tree_tray = dict.minimum_text_distance("tree", "tray");
    auto tree_treat = dict.minimum_text_distance("tree", "treat");
    if (tree_tray.has_value() && tree_treat.has_value()) {
        print_comparison_result("Tree vs Tray vs Treat", "tree", "tray", "treat",
                               tree_tray.value(), tree_treat.value(),
                               "tray should be closer to tree than treat (vowel change vs consonant change)",
                               tree_tray.value() > tree_treat.value());
    }

    std::cout << "\n" << std::string(50, '=') << std::endl;
    std::cout << "Calibration test completed!" << std::endl;
    std::cout << "Review the results above to verify phonetic intuition." << std::endl;

    return 0;
} 
