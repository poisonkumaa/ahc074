from pathlib import Path

here=Path(__file__).resolve().parent
s=(here/'main_baseline.cpp').read_text(encoding='utf-8')
old='''        if(init-curprice>best.gain){best={cur,lo,n,int(credits[w]!=0),passes,init-curprice};}
'''
new='''        static const bool joint_enabled=search_budget::integer("STRUCT_SHELF_TWO",1)!=0;
        static clock_t joint_spent=0;
        static const clock_t joint_cap=clock_t(search_budget::seconds(.025)*CLOCKS_PER_SEC);
        if(joint_enabled && paid>=2 && n<=16 && joint_spent<joint_cap){
            const clock_t joint_start=clock(),joint_stop=joint_start+(joint_cap-joint_spent);
            bool exhausted=false;int ticks=0;
            // Two paid insertions can exchange which paper carries the spare
            // color and free a useful donor at the same time. Retain exactly
            // the one-level home-return credit of the original neighborhood.
            for(int i=0;i<n && !exhausted;i++)if(last[base[i]]>=0){
                for(int dir:{-1,1}){
                    auto first=base;
                    for(int j=i+dir;j>=0 && j<n && !exhausted;j+=dir){
                        swap(first[j],first[j-dir]);
                        for(int p=0;p<n && !exhausted;p++)if(p!=j && last[first[p]]>=0){
                            for(int direction:{-1,1}){
                                auto trial=first;
                                for(int q=p+direction;q>=0 && q<n;q+=direction){
                                    if((++ticks&63)==0 && clock()>=joint_stop){exhausted=true;break;}
                                    swap(trial[q],trial[q-direction]);
                                    if(depth(trial,ret)!=f+1 || depth(trial,1-ret)!=0)continue;
                                    double c=price(trial);
                                    if(c<curprice-1e-7){cur=trial;curprice=c;passes=2;}
                                }
                                if(exhausted)break;
                            }
                        }
                    }
                    if(exhausted)break;
                }
            }
            joint_spent+=clock()-joint_start;
        }
        if(init-curprice>best.gain){best={cur,lo,n,int(credits[w]!=0),passes,init-curprice};}
'''
assert s.count(old)==1
s=s.replace(old,new)
(here/'struct_two.cpp').write_text(s,encoding='utf-8')
