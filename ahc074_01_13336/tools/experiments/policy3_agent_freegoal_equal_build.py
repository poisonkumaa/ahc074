from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'policy3_agent_freegoal.cpp').read_text()
s=s.replace('const vector<uint64_t>&future){','const vector<uint64_t>&future,int basefrom,int baseto,int&baseq,int&baseinv){',1)
s=s.replace(' sort(out.begin(),out.end(),[](auto a,auto b){return a.quality==b.quality?a.inv<b.inv:a.quality<b.quality;});return out;', ''' auto less=[](auto a,auto b){return a.quality==b.quality?a.inv<b.inv:a.quality<b.quality;};
 vector<array<FreeChoice,2>>best(K);for(auto&pair:best)for(auto&c:pair)c.quality=c.inv=INT_MAX;
 for(auto c:out){if(c.from==basefrom&&c.to==baseto){baseq=c.quality;baseinv=c.inv;}auto&b=best[c.from];if(less(c,b[0])){b[1]=b[0];b[0]=c;}else if(less(c,b[1]))b[1]=c;}
 out.clear();for(auto&b:best)for(auto c:b)if(c.quality<INT_MAX)out.push_back(c);sort(out.begin(),out.end(),less);return out;''')
s=s.replace('auto candidates=free_moves(order,length,future);int baseq=INT_MAX,baseinv=INT_MAX;for(auto c:candidates)if(c.from==from&&c.to==to){baseq=c.quality;baseinv=c.inv;break;}', 'int baseq=INT_MAX,baseinv=INT_MAX;auto candidates=free_moves(order,length,future,from,to,baseq,baseinv);int oldreward=0;for(int k=0;k<int(papers.size());k++)if(selected[k])oldreward+=dp.reward[k];')
s=s.replace('auto goal=dp.solve(next,A[product],distance);fill(selected.begin()', 'auto goal=dp.solve(next,A[product],distance);int newreward=0;for(int z:goal)newreward+=dp.reward[z];if(newreward+(argc>2?atoi(argv[2]):0)<oldreward)continue;fill(selected.begin()')
(p/'policy3_agent_freegoal_equal.cpp').write_text(s)
