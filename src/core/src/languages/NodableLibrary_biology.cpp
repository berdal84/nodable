#include <nodable/core/languages/NodableLibrary_biology.h>

#include <nodable/core/Language_MACROS.h>

using namespace Nodable;

std::unordered_map<std::string, char> NodableLibrary_biology::codon_table =
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


std::string NodableLibrary_biology::dna_to_protein(std::string _base_string)
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

void NodableLibrary_biology::bind_to_language(ILanguage* _language) const
{
    BIND_TO(_language)
    BIND_FUNCTION(dna_to_protein)
}
