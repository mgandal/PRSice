// This file is part of PRSice2.0, copyright (C) 2016-2017
// Shing Wan Choi, Jack Euesden, Cathryn M. Lewis, Paul F. O’Reilly
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef SNP_H
#define SNP_H

#include <algorithm>
#include <limits.h>
#include <numeric>
#include <stdexcept>
#include <string>
#include "commander.hpp"
#include "misc.hpp"
#include "plink_common.hpp"
#include "region.hpp"
#include "storage.hpp"

class SNP
{
public:
    SNP();
    SNP(const std::string rs_id, const int chr, const int loc,
    		const std::string ref_allele, const std::string alt_allele,
			const std::string file_name, const int num_line);
    virtual ~SNP();

    void set_statistic(const double stat, const double se, const double p_value, const int category,
    		const double p_threshold)
    {
    		statistic.stat = stat;
    		statistic.se = se;
    		statistic.p_value = p_value;
    		threshold.category = category;
    		threshold.p_threshold = p_threshold;
    };
    void set_flipped() { statistic.flipped = true; };
    std::string get_rs() const { return basic.rs; };
    static std::vector<size_t> sort_by_p(const std::vector<SNP> &input);

    bool operator == (const SNP &Ref) const
    {
    	if(basic.chr == Ref.basic.chr && basic.loc==Ref.basic.loc && basic.rs.compare(Ref.basic.rs)==0)
    	{
    		if(basic.ref.compare(Ref.basic.ref)==0){
    			if(!basic.alt.empty() && !Ref.basic.alt.empty())
    			{
    				return basic.alt.compare(Ref.basic.alt)==0;
    			}else return true;
    		}
    		else if(complement(basic.ref).compare(Ref.basic.ref)==0)
    		{
    			if(!basic.alt.empty() && !Ref.basic.alt.empty())
    			{
    				return complement(basic.alt).compare(Ref.basic.alt)==0;
    			}else return true;
    		}
    		else if(!basic.alt.empty() && !Ref.basic.alt.empty())
    		{
    			if(basic.ref.compare(Ref.basic.alt)==0 && basic.alt.compare(Ref.basic.ref)==0)
    			{
    				return true;
    			}
    			if(complement(basic.ref).compare(Ref.basic.alt)==0 &&
    					complement(basic.alt).compare(Ref.basic.alt)==0)
    			{
    				return true;
    			}
    			return false;
    		}
    		else return false; // cannot flip nor match
    	}
    	else
    	{
    		return false;
    	}
    };

    inline void flipped(){ statistic.flipped = true; };
    inline void fill_info(int chr, int loc, std::string alt)
    {
    	if(basic.chr==-1) basic.chr = chr;
    	if(basic.loc==-1) basic.loc = loc;
    	if(basic.alt.empty()) basic.alt=alt;
    };
    inline bool matching (int chr, int loc, std::string ref, std::string alt, bool &flipped){
    	misc::trim(ref);
    	misc::trim(alt);
    	misc::trim(basic.ref);
    	misc::trim(basic.alt);
    	if(chr != -1 && basic.chr != -1 && chr != basic.chr) return false;
    	if(loc != -1 && basic.loc != -1 && loc != basic.loc) return false;
    	if(basic.ref.compare(ref)==0){
    		if(!basic.alt.empty() && !alt.empty())
    		{
    			return basic.alt.compare(alt)==0;
    		}else return true;
    	}
    	else if(complement(basic.ref).compare(ref)==0)
    	{
    		if(!basic.alt.empty() && !alt.empty())
    		{
    			return complement(basic.alt).compare(alt)==0;
    		}else return true;
    	}
    	else if(!basic.alt.empty() && !alt.empty())
    	{
    		if(basic.ref.compare(alt)==0 && basic.alt.compare(ref)==0)
    		{
    			flipped = true;
    			return true;
    		}
    		if(complement(basic.ref).compare(alt)==0 && complement(basic.alt).compare(ref)==0)
    		{
    			flipped = true;
    			return true;
    		}
    		return false;
    	}
    	else return false; // cannot flip nor match
    };

    int chr() const { return basic.chr; };
    int loc() const { return basic.loc; };
    int snp_id() const { return file_info.id; };
    int category() const { return threshold.category; };
    double p_value() const { return statistic.p_value; };
    double stat() const { return statistic.stat; };
    double get_threshold() const { return threshold.p_threshold; };
    std::string file_name() const { return file_info.file; };
    std::string rs() const { return basic.rs; };
    std::string ref() const { return basic.ref; };
    std::string alt() const { return basic.alt; };
    bool is_flipped(){ return statistic.flipped; };
    /*
    inline bool in(size_t i) const
    {
    		if(i/m_bit_size >= m_flags.size()) throw std::out_of_range("Out of range for flag");
    		return (m_flags[i/m_bit_size] >> i%m_bit_size) & ONE; // 1 = true, 0 = false
    }
     */
    inline bool in(size_t i) const
    {
    	if(i / BITCT >= m_max_flag_index) throw std::out_of_range("Out of range for flag");
    	return ((m_flags[i / BITCT] >> (i % BITCT)) & 1);
    }

    //void set_flag(std::vector<long_type> flag) { m_flags = flag; };

    void set_flag(Region &region)
    {
    	m_max_flag_index = BITCT_TO_WORDCT(region.size());
    	m_flags.resize(m_max_flag_index);
    	//m_flag = new uintptr_t[m_max_flag_index];
    	region.check(basic.chr, basic.loc, m_flags);
    	if((m_flags[179 / BITCT] >> (179 % BITCT)) & 1)
    	{
    		std::cerr << basic.rs << std::endl;
    	}
    };

    void add_clump( std::vector<size_t> &i) { clump_info.target.insert( clump_info.target.end(), i.begin(), i.end() ); };
    void add_clump_r2( std::vector<double> &i) { clump_info.r2.insert( clump_info.r2.end(), i.begin(), i.end() ); };
    void set_clumped() { clump_info.clumped = true; };
    void proxy_clump(std::vector<SNP> &snp_list, double r2_threshold);
    void clump(std::vector<SNP> &snp_list);
    bool clumped() const { return clump_info.clumped; };
    void set_clump_geno(uintptr_t *geno, int contain_miss)
    {
    	clump_info.genotype = geno;
    	clump_info.contain_missing = (contain_miss==3);
    }
    uintptr_t *clump_geno() const { return clump_info.genotype; };
    bool clump_missing() const { return clump_info.contain_missing; };
    void clean_clump()
    {
    	if(clump_info.genotype!=nullptr)
    	{
    		delete [] clump_info.genotype;
    		clump_info.genotype = nullptr;
    	}
    }
 private:
    //basic info
    size_t m_bit_size;
    struct{
    	bool clumped;
    	std::vector<size_t> target;
    	std::vector<double> r2;
    	uintptr_t *genotype;
    	bool contain_missing;
    } clump_info;

    struct{
    	std::string ref;
    	std::string alt;
    	std::string rs;
    	int chr;
    	int loc;
    }basic;

    struct{
    	std::string file;
    	int id;
    }file_info;

    struct{
    	double stat;
        double se;
        double p_value;
        bool flipped;
    }statistic;

    struct{
    	int category;
        double p_threshold;
    }threshold;

    // This indicate where this SNP's bound is at
    // useful for PRSlice and also clumping
    // thinking about it. Even if the location isn't given for
    // PRSet or PRSlice, we can still use the coordinates from
    // the target / reference file
    // the bound is [ )
    // prset related
    size_t m_max_flag_index=0;
    std::vector<uintptr_t> m_flags;

    inline std::string complement(const std::string &allele) const
    {
    	if(allele.compare("A")==0 || allele.compare("a")==0) return "T";
    	if(allele.compare("T")==0 || allele.compare("t")==0) return "A";
    	if(allele.compare("G")==0 || allele.compare("g")==0) return "C";
    	if(allele.compare("C")==0 || allele.compare("c")==0) return "G";
    	else return allele; // Cannot flip, so will just return it as is
    }

};

#endif // SNP_H
