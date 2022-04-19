#pragma once
#include <nodable/core/ILibrary.h>
#include <string>
#include <unordered_map>

namespace Nodable
{
    class NodableLibrary_biology : public ILibrary
    {
    public:
        void                 bind_to_language(ILanguage* _language)const override;
        static std::string   dna_to_protein(std::string _base_string);
    private:
        static constexpr char                        k_codon_start = 'M';
        static constexpr char                        k_codon_stop  = '_';
        static std::unordered_map<std::string, char> codon_table;
    };
}