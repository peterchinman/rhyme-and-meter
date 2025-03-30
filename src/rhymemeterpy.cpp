#include<nanobind/nanobind.h>
#include<nanobind/stl/string.h>
#include<nanobind/stl/vector.h>
#include<nanobind/stl/pair.h>

#include "rhyme_and_meter.hpp"
#include "consonant_distance.hpp"
#include "distance.hpp"
#include "Hirschberg.hpp"
#include "vowel_hex_graph.hpp"

namespace nb = nanobind;

NB_MODULE(rhymemeter, m)
{
    nb::class_<Rhyme_and_Meter> RM(m, "Rhyme_and_Meter");

    RM.def(nb::init<>())
        .def("word_to_phones", &Rhyme_and_Meter::word_to_phones)
        .def("fuzzy_meter_to_binary_set", &Rhyme_and_Meter::fuzzy_meter_to_binary_set);

    nb::class_<Rhyme_and_Meter::Check_Validity_Result>(RM, "Check_Validity_Result")
        .def_rw("is_valid", &Rhyme_and_Meter::Check_Validity_Result::is_valid)
        .def_rw("unrecognized_words", &Rhyme_and_Meter::Check_Validity_Result::unrecognized_words);

    RM.def("check_meter_validity", &Rhyme_and_Meter::check_meter_validity)
        .def("check_syllable_validity", &Rhyme_and_Meter::check_syllable_validity)
        .def("compare_end_line_rhyming_parts", &Rhyme_and_Meter::compare_end_line_rhyming_parts)
        .def("minimum_end_rhyme_distance", &Rhyme_and_Meter::minimum_end_rhyme_distance)
        .def("get_end_rhyme_distance", &Rhyme_and_Meter::get_end_rhyme_distance);

    nb::enum_<Manner>(m, "Manner")
        .value("affricate", Manner::affricate)
        .value("approximant", Manner::approximant)
        .value("fricative", Manner::fricative)
        .value("lateral_approximant", Manner::lateral_approximant)
        .value("nasal", Manner::nasal)
        .value("plosive", Manner::plosive)
        .export_values();

    nb::class_<Consonant>(m, "Consonant")
        .def(nb::init<>())
        .def_rw("arpabet", &Consonant::arpabet)
        .def_rw("manner", &Consonant::manner)
        .def_rw("sibilant", &Consonant::sibilant)
        .def_rw("voiced", &Consonant::voiced)
        .def_rw("place", &Consonant::place);

    nb::class_<ConsonantDistance>(m, "ConsonantDistance")
        .def(nb::init<>())
        .def_static("initialize", &ConsonantDistance::initialize)
        .def_static("get_consonant", &ConsonantDistance::get_consonant)
        .def_static("get_distance", &ConsonantDistance::get_distance);

    m.def("GAP_PENALTY", &GAP_PENALTY);
    m.def("SUBSTITUTION_SCORE", &SUBSTITUTION_SCORE);
    m.def("levenshtein_distance", &levenshtein_distance);
    m.def("levenshtein_distance_with_multiplier", &levenshtein_distance_with_multiplier);

    nb::class_<Alignment_And_Distance>(m, "Alignment_And_Distance")
        .def(nb::init<>())
        .def_rw("ZWpair", &Alignment_And_Distance::ZWpair)
        .def_rw("distance", &Alignment_And_Distance::distance);

    m.def("Hirschberg", &Hirschberg);

    nb::class_<VowelHexGraph>(m, "VowelHexGraph")
        .def(nb::init<>())
        .def_static("add_edge", &VowelHexGraph::add_edge)
        .def_static("calculate_shortest_distance", &VowelHexGraph::calculate_shortest_distance)
        .def_static("calculate_all_distances", &VowelHexGraph::calculate_all_distances)
        .def_static("initialize", &VowelHexGraph::initialize)
        .def_static("get_connected_vowels", &VowelHexGraph::get_connected_vowels)
        .def_static("get_distance", &VowelHexGraph::get_distance);
}
