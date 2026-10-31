from pathlib import Path
p=Path(__file__).resolve().parent
s=(p.parents[1]/'main.cpp').read_text();(p/'policy2_agent_perf_base.cpp').write_text(s)
a=s.index('inline std::pair<int,int> best_move(');b=s.index('\n}\n}',a)+2
t=s[a:b]
t=t.replace('std::vector<int> owner(N+2,-1), next(K), pos(K), suffix(K+1,N+1);','std::array<int,66> owner;owner.fill(-1);std::array<int,192> next,pos;std::array<int,193> suffix;suffix.fill(N+1);')
t=t.replace('std::vector<int> initial_bad(K), inversions(K);','std::array<int,192> initial_bad{},inversions{};')
t=t.replace('std::vector<char> initial_moved(K), initial_before(K);','std::array<char,192> initial_moved{},initial_before{};')
t=t.replace('std::vector<double> power(K+1,1.);','std::array<double,193> power;power[0]=1.;')
t=t.replace('std::vector<int> fenwick(N+2);','std::array<int,66> fenwick{};')
t=t.replace('std::vector<char> moved(K),before(K);std::vector<int>bad(K);','std::array<char,192> moved,before;std::array<int,192>bad;')
s=s[:a]+t+s[b:]
(p/'policy2_agent_perf_arrays.cpp').write_text(s)
s=s.replace('vector<array<int,193>> cnt;vector<int>reward;', 'vector<array<int,193>> cnt,dp,parent,which;vector<int>reward;')
s=s.replace('cnt(n+2){reward.assign(k,16);}', 'cnt(n+2),dp(n+1),parent(n+1),which(n+1){reward.assign(k,16);}')
s=s.replace('  vector<array<int,193>>dp(n+1),parent(n+1),which(n+1);\n','')
s=s.replace('vector<int>active;for(int h=0;h<=k;h++)if(dp[j][h]<1000000000)active.push_back(h);if(active.empty())continue;', 'int active[193],active_count=0;for(int h=0;h<=k;h++)if(dp[j][h]<1000000000)active[active_count++]=h;if(!active_count)continue;')
s=s.replace('vector<int>cand;for(int z=0;z<k;z++){int l=a[z].len;if(l>j&&!((a[z].color^target)&(low(l)^low(j))))cand.push_back(z);}', 'int cand[192],cand_count=0;for(int z=0;z<k;z++){int l=a[z].len;if(l>j&&!((a[z].color^target)&(low(l)^low(j))))cand[cand_count++]=z;}')
s=s.replace('for(int z:cand){int l=a[z].len,q=pos[z];', 'for(int ci=0;ci<cand_count;ci++){int z=cand[ci],l=a[z].len,q=pos[z];')
s=s.replace('for(int h:active){','for(int ai=0;ai<active_count;ai++){int h=active[ai];')
(p/'policy2_agent_perf_arrays_dp.cpp').write_text(s)
