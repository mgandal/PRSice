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

#include "snp.hpp"

SNP::SNP()
{
	basic.chr=-1;
	basic.loc=-1; // default is -1 to indicate that it is not provided
    statistic.flipped = false;
    statistic.stat=0.0;
    statistic.se=0.0;
    statistic.p_value=2.0; // set this to 2 such that only SNPs in base file have valid P-value
    threshold.p_threshold=0.0;
    threshold.category=0;
    m_bit_size = sizeof(long_type)*CHAR_BIT;
    clump_info.clumped=false;
	clump_info.genotype = nullptr;
	clump_info.contain_missing = false;
}


SNP::SNP(const std::string rs_id, const int chr, const int loc,
		const std::string ref_allele, const std::string alt_allele,
		const std::string file_name, const int num_line)
{
	basic.ref=ref_allele;
	basic.alt=alt_allele;
	basic.rs=rs_id;
	basic.chr=chr;
	basic.loc=loc;
	statistic.se = 0.0;
	statistic.p_value = 0.0;
	statistic.stat = 0.0;
	statistic.flipped = false;
    threshold.p_threshold=0.0;
    threshold.category=0;
    m_bit_size = sizeof(long_type)*CHAR_BIT;
    clump_info.clumped=false;
	clump_info.genotype = nullptr;
	clump_info.contain_missing = false;
    file_info.file = file_name;
    file_info.id = num_line;
}




SNP::~SNP(){
    if(clump_info.genotype!=nullptr)
    {
        delete [] clump_info.genotype;
        clump_info.genotype = nullptr;
    }


}

std::vector<size_t> SNP::sort_by_p(const std::vector<SNP> &input)
{
    std::vector<size_t> idx(input.size());
    std::iota(idx.begin(), idx.end(),0);
    std::sort(idx.begin(), idx.end(), [&input](size_t i1, size_t i2)
    {
    	// plink do it with respect to the location instead of statistic
        if(input[i1].statistic.p_value==input[i2].statistic.p_value)
        {
        	if(input[i1].basic.chr==input[i2].basic.chr)
        	{
        		if(input[i1].basic.loc == input[i2].basic.loc)
        		{
        			if(fabs(input[i1].statistic.stat)==fabs(input[i2].statistic.stat))
        			{
        				return input[i1].statistic.se < input[i2].statistic.se;
        			}
        			else return fabs(input[i1].statistic.stat) > fabs(input[2].statistic.stat);
        		}
        		else return input[i1].basic.loc < input[i2].basic.loc;
        	}
			else return input[i1].basic.chr<input[i2].basic.chr;
        }
        else return input[i1].statistic.p_value < input[i2].statistic.p_value;
    });
    return idx;
}


void SNP::clump(std::vector<SNP> &snp_list)
{
	// for some reason, some pathway might contain 0 SNPs after clumping
	/* check:
2	rs113447001	0	178051659	A	G
2	rs12988177	0	178051987	A	G
2	rs2588875	0	178066334	G	A
2	rs2251139	0	178068159	A	G
2	rs13029183	0	178087851	G	A
2	rs139080704	0	178098088	-	A

	 */
	uintptr_t check=0;
	bool print = false;
	for(auto &&target : clump_info.target){
		if(!snp_list[target].clumped())
		{
			int sum_total = 0;
			for(size_t i_flag = 0; i_flag < m_max_flag_index; ++i_flag)
			{
				if(snp_list[target].rs().compare("rs35926996")==0 ||
						snp_list[target].rs().compare("rs35059065")==0 )

				{
					print=true;
					check=snp_list[target].m_flags[i_flag];
				}
				snp_list[target].m_flags[i_flag] = snp_list[target].m_flags[i_flag] ^
						(m_flags[i_flag] & snp_list[target].m_flags[i_flag]);
				sum_total+=snp_list[target].m_flags[i_flag]; //unless all unset, this will always be non-zero
				if(snp_list[target].m_flags[i_flag] != check &&print ){
					std::cerr << "Clump change: " << basic.rs << "\t" <<snp_list[target].rs() << std::endl;
					print=false;
				}
			}
			if(sum_total==0)
			{
				if(snp_list[target].rs().compare("rs35926996")==0 ||
						snp_list[target].rs().compare("rs35059065")==0 )

				{
					std::cerr << "Gone!" << std::endl;
				}
				snp_list[target].set_clumped();
			}
		}
	}
	clump_info.clumped=true; // protect from other SNPs tempering its flags
}

void SNP::proxy_clump(std::vector<SNP> &snp_list, double r2_threshold)
{
    for(size_t i_target = 0; i_target < clump_info.target.size(); ++i_target)
    {
    	auto &&target = clump_info.target[i_target];
        if(!snp_list[target].clumped())
        {
            if(clump_info.r2[i_target] >= r2_threshold)
            {
            	// Both SNP now considered to be in the same set
                for(size_t i_flag = 0; i_flag < m_max_flag_index; ++i_flag)
                {
                	m_flags[i_flag] |= snp_list[target].m_flags[i_flag];
                }
                // we will set clumped as we now fully represent the target SNP
                snp_list[target].set_clumped();
            }
            else
            {
            	int sum_total = 0;
            	for(size_t i_flag = 0; i_flag < m_max_flag_index; ++i_flag)
            	{
            		snp_list[target].m_flags[i_flag] = snp_list[target].m_flags[i_flag] ^
            				(m_flags[i_flag] & snp_list[target].m_flags[i_flag]);
            		sum_total+=snp_list[target].m_flags[i_flag]; //unless all unset, this will always be non-zero
            	}
            	if(sum_total==0)  snp_list[target].set_clumped();
            }
        }
    }
	clump_info.clumped=true; // protect from other SNPs tempering its flags
}
