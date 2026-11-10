from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'post_beam.cpp').read_text(encoding='utf-8')
s=s.replace('struct BeamOption {int gain,paper,cut;};','struct BeamOption {int gain,paper,cut;uint64_t cover;};',1)
needle='                        options.push_back({gain,k,cut});'
assert s.count(needle)==1
s=s.replace(needle,'                        options.push_back({gain,k,cut,covered&diff});',1)
needle='''                    if(post_beam_options>0 && int(options.size())>post_beam_options)
                        options.resize(post_beam_options);'''
replacement='''                    const int diversity=search_budget::integer("POST_BEAM_DIVERSITY",1);
                    if(diversity>0 && post_beam_options>0){
                        vector<BeamOption> diverse,remainder;
                        map<uint64_t,int> count;
                        for(auto option:options){
                            if(count[option.cover]++<diversity)diverse.push_back(option);
                            else remainder.push_back(option);
                        }
                        for(auto option:remainder)if(int(diverse.size())<post_beam_options)diverse.push_back(option);
                        options=move(diverse);
                    }
                    if(post_beam_options>0 && int(options.size())>post_beam_options)
                        options.resize(post_beam_options);'''
assert needle in s
s=s.replace(needle,replacement,1)
needle='''                if(int(next.size())>post_beam_width)next.resize(post_beam_width);'''
replacement='''                const int diversity=search_budget::integer("POST_BEAM_DIVERSITY",1);
                if(diversity>0 && int(next.size())>post_beam_width){
                    vector<BeamNode> diverse,remainder;map<uint64_t,int> count;
                    for(auto& candidate:next){
                        uint64_t seen=0,display=0;
                        for(int k:candidate.state.order){display|=black[k]&~seen;seen|=opaque[k];}
                        if(count[display^target_mask]++<diversity)diverse.push_back(move(candidate));
                        else remainder.push_back(move(candidate));
                    }
                    for(auto& candidate:remainder)if(int(diverse.size())<post_beam_width)diverse.push_back(move(candidate));
                    next=move(diverse);
                }
                if(int(next.size())>post_beam_width)next.resize(post_beam_width);'''
assert needle in s
s=s.replace(needle,replacement,1)
(p/'post_diverse.cpp').write_text(s,encoding='utf-8')
