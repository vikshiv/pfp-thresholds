/* ms_rle_string - Extension of the r-index rle_string to compute matching statistics
    Copyright (C) 2020 Massimiliano Rossi

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see http://www.gnu.org/licenses/ .
*/
/*!
   \file ms_rle_string.hpp
   \brief ms_rle_string.hpp Extension of the r-index rle_string to compute matching statistics.
   \author Massimiliano Rossi
   \date 10/07/2020
*/

#ifndef _MS_RLE_STRING_HH
#define _MS_RLE_STRING_HH

#include <common.hpp>

#include <rle_string.hpp>


template <
    class sparse_bitvector_t = ri::sparse_sd_vector, //predecessor structure storing run length
    class string_t = ri::huff_string                 //run heads
    >
class ms_rle_string : public ri::rle_string<sparse_bitvector_t, string_t>
{
    public:

    ms_rle_string() : 
        ri::rle_string<sparse_bitvector_t, string_t>()
    {
        //NtD
    }

    /*
     * constructor: build structure on the input string
     * \param input the input string without 0x0 bytes in it.
     * \param B block size. The main sparse bitvector has R/B bits set (R being number of runs)
     *
     */
    ms_rle_string(string &input, ulint B = 2) : 
        ri::rle_string<sparse_bitvector_t, string_t>(input, B)
    {
        // NtD
    }

    ms_rle_string(std::ifstream &ifs, ulint B = 2) : 
        ri::rle_string<sparse_bitvector_t, string_t>(ifs, B)
    {

    }

    // Construction from run-length encoded BWT
    ms_rle_string(std::ifstream& heads, std::ifstream& lengths, ulint B = 2) {
        heads.clear(); heads.seekg(0);
        lengths.clear(); lengths.seekg(0);
        // assert(not contains0(input)); // We're hacking the 0 away :)
        this->B = B;
        // n = input.size();
        auto runs_per_letter_bv = vector<vector<bool> >(256);
        //runs in main bitvector
        vector<bool> runs_bv;

        // Reads the run heads
        string run_heads_s;
        heads.seekg(0, heads.end);
        run_heads_s.resize(heads.tellg());
        heads.seekg(0, heads.beg);
        heads.read(&run_heads_s[0], run_heads_s.size());

        size_t pos = 0;
        this->n = 0;
        this->R = run_heads_s.size();
        // Compute runs_bv and runs_per_letter_bv
        for (size_t i = 0; i < run_heads_s.size(); ++i)
        {
            size_t length;
            lengths.read((char*)&length, 5);
            if(run_heads_s[i]<=TERMINATOR) // change 0 to 1
                run_heads_s[i]=TERMINATOR;

            std::fill_n( std::back_inserter(runs_bv), length-1, false);
            runs_bv.push_back(i%B==B-1);

            std::fill_n( std::back_inserter(runs_per_letter_bv[run_heads_s[i]]), length-1, false);
            runs_per_letter_bv[run_heads_s[i]].push_back(true);

            this->n += length;
        }
        runs_bv.push_back(false);

        //now compact structures
        assert(runs_bv.size()==this->n);
        ulint t = 0;
        for(ulint i=0;i<256;++i)
            t += runs_per_letter_bv[i].size();
        assert(t==this->n);
        this->runs = sparse_bitvector_t(runs_bv);
        //a fast direct array: char -> bitvector.
        this->runs_per_letter = vector<sparse_bitvector_t>(256);
        for(ulint i=0;i<256;++i)
            this->runs_per_letter[i] = sparse_bitvector_t(runs_per_letter_bv[i]);
        this->run_heads = string_t(run_heads_s);
        assert(this->run_heads.size()==this->R);
    }

    size_t number_of_runs_of_letter(uint8_t c)
    {
        return this->runs_per_letter[c].number_of_1();
    }

    size_t number_of_letter(uint8_t c)
    {
        return this->runs_per_letter[c].size();
    }

    /* serialize the structure to the ostream
     * \param out     the ostream
     */
    ulint serialize(std::ostream &out) 
    {
        return ri::rle_string<sparse_bitvector_t, string_t>::serialize(out);
    }

    /* load the structure from the istream
     * \param in the istream
     */
    void load(std::istream &in)
    {
        ri::rle_string<sparse_bitvector_t, string_t>::load(in);
    }

    private :
};

typedef ms_rle_string<ri::sparse_sd_vector> ms_rle_string_sd;
typedef ms_rle_string<ri::sparse_hyb_vector> ms_rle_string_hyb;

#endif /* end of include guard: _MS_RLE_STRING_HH */
