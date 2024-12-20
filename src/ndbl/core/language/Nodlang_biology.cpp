#include "Nodlang_biology.h"
#include "tools/core/reflection/reflection"

using namespace ndbl;
using namespace tools;

namespace // anonymous, only accessible from this file
{
    constexpr char                        k_codon_start = 'M';
    constexpr char                        k_codon_stop  = '_';
    std::unordered_map<std::string, char> codon_table   =
    {
        // T__
        {"TCA", 'S'}, {"TCC", 'S'}, {"TCG", 'S'}, {"TCT", 'S'}, {"TTC", 'F'}, {"TTT", 'F'}, {"TTA", 'L'},
        {"TAC", 'Y'}, {"TAT", 'Y'}, {"TGC", 'C'}, {"TGT", 'C'}, {"TGG", 'W'}, {"TTG", 'L'},
        {"TAG", k_codon_stop}, {"TAA", k_codon_stop }, {"TGA", k_codon_stop },

        // A__
        {"ATA", 'I'}, {"ATC", 'I'}, {"ATT", 'I'}, {"ATG", k_codon_start}, {"ACA", 'T'}, {"ACC", 'T'}, {"ACG", 'T'},
        {"ACT", 'T'}, {"AAC", 'N'}, {"AAT", 'N'}, {"AAA", 'K'}, {"AAG", 'K'}, {"AGC", 'S'}, {"AGT", 'S'},
        {"AGA", 'R'}, {"AGG", 'R'},

        // C__
        {"CTA", 'L'}, {"CTC", 'L'}, {"CTG", 'L'}, {"CTT", 'L'}, {"CCA", 'P'}, {"CCC", 'P'}, {"CCG", 'P'}, {"CCT", 'P'},
        {"CAC", 'H'}, {"CAT", 'H'}, {"CAA", 'Q'}, {"CAG", 'Q'}, {"CGA", 'R'}, {"CGC", 'R'}, {"CGG", 'R'}, {"CGT", 'R'},

        // G__
        {"GTA", 'V'}, {"GTC", 'V'}, {"GTG", 'V'}, {"GTT", 'V'}, {"GCA", 'A'}, {"GCC", 'A'}, {"GCG", 'A'}, {"GCT", 'A'},
        {"GAC", 'D'}, {"GAT", 'D'}, {"GAA", 'E'}, {"GAG", 'E'}, {"GGA", 'G'}, {"GGC", 'G'}, {"GGG", 'G'}, {"GGT", 'G'},
    };


    std::string dna_to_protein(const std::string& _base_string)
    {
        std::string protein_result;

        for (size_t i = 0; i < _base_string.size() / 3; i++)
        {
            std::string possibly_codon = _base_string.substr(i, 3);

            auto found = codon_table.find(possibly_codon );

            if( found != codon_table.cend() )
            {
                protein_result.push_back( found->second );
            }
        }

        return protein_result;
    }
}

REFLECT_STATIC_INITIALIZER
(
    DEFINE_REFLECT(Nodlang_biology)
        .add_method(&dna_to_protein, "dna_to_protein", "protein");
)

Nodlang_biology::Nodlang_biology(){} // necessary to execute static code