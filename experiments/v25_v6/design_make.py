from pathlib import Path
import hashlib

HERE=Path(__file__).resolve().parent
baseline=HERE/'main_baseline.cpp'
assert hashlib.sha256(baseline.read_bytes()).hexdigest()=='a9e5cb6b45ee1562c60721be09bde2d0246ad8eeb95585c7721048d74143d7da'
s=baseline.read_text(encoding='utf-8')

def once(old,new):
    global s
    assert s.count(old)==1,(old[:90],s.count(old))
    s=s.replace(old,new,1)

s='''#if defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC target("popcnt")
#endif
'''+s
once('int main() {','#include "design_flip.inc"\n\nint main() {')
once('''            double base_energy=0;
            for(int w=C;w<T;w++)base_energy+=F(pc[w],sc[w],paired_ones[w]);
            for(int x=0;x<n;x++)base_energy+=beta*hole[x][at[x]];''','''            const double exact_flip_mix=std::clamp(search_budget::real("DESIGN_FLIP",0.),0.,1.);
            using ExactFlip=design_flip::Model<decltype(prices)>;
            std::optional<ExactFlip> exact_flip;
            if(exact_flip_mix>0) {
                exact_flip.emplace(C,T,prices);
                for(int x=0;x<n;x++)exact_flip->add(at[x],li[x].bits,li[x].diff,li[x].type);
            }
            double base_energy=0,linear_exposure=0;
            for(int w=C;w<T;w++)base_energy+=F(pc[w],sc[w],paired_ones[w]);
            for(int x=0;x<n;x++){double cost=beta*hole[x][at[x]];base_energy+=cost;linear_exposure+=cost;}
            if(exact_flip)base_energy+=exact_flip_mix*(exact_flip->value()-linear_exposure);''')
once('''            double joint_best=joint_energy;
            auto joint_best_at=at;''','''            double joint_best=joint_energy;
            const double exact_flip_initial=joint_energy;
            int exact_flip_proposals=0,exact_flip_accepted=0;
            auto joint_best_at=at;''')
once('''                if(exchange)delta_base+=beta*(hole[y][u]-hole[y][v]);
                vector<int> nw0=cw0,nw1=cw1;''','''                if(exchange)delta_base+=beta*(hole[y][u]-hole[y][v]);
                if(exact_flip) {
                    exact_flip->begin_delta();
                    exact_flip->change(u,li[x].bits,li[x].diff,li[x].type,-1.);
                    exact_flip->change(v,li[x].bits,li[x].diff,li[x].type,1.);
                    double linear_delta=beta*(hole[x][v]-hole[x][u]);
                    if(exchange) {
                        exact_flip->change(v,li[y].bits,li[y].diff,li[y].type,-1.);
                        exact_flip->change(u,li[y].bits,li[y].diff,li[y].type,1.);
                        linear_delta+=beta*(hole[y][u]-hole[y][v]);
                    }
                    delta_base+=exact_flip_mix*(exact_flip->delta()-linear_delta);
                    exact_flip_proposals++;
                }
                vector<int> nw0=cw0,nw1=cw1;''')
once('''                at[x]=v;if(exchange)at[y]=u;
                vector<int> changed_products=products[x];''','''                at[x]=v;if(exchange)at[y]=u;
                if(exact_flip)exact_flip->check(li,at);
                vector<int> changed_products=products[x];''')
once('''                    cw0=move(nw0);cw1=move(nw1);
                    base_energy+=delta_base;''','''                    cw0=move(nw0);cw1=move(nw1);
                    if(exact_flip){exact_flip->commit();exact_flip_accepted++;}
                    base_energy+=delta_base;''')
once('''            best_at=joint_best_at;
        }

        for(int w=C;w<T;w++){bucket_pairs[w].clear();bucket_sings[w].clear();}''','''            best_at=joint_best_at;
            if(exact_flip)cerr<<"DESIGN_FLIP mix="<<exact_flip_mix<<" W="<<W<<" offset="<<graph_seed_offset
                <<" proposals="<<exact_flip_proposals<<" accepted="<<exact_flip_accepted
                <<" objective="<<exact_flip_initial<<"->"<<joint_best
                <<" verified="<<exact_flip->verified<<" max_error="<<exact_flip->largest_error<<"\\n";
        }

        for(int w=C;w<T;w++){bucket_pairs[w].clear();bucket_sings[w].clear();}''')
(HERE/'design_candidate.cpp').write_text(s,encoding='utf-8',newline='\n')
print('Wrote design_candidate.cpp')
