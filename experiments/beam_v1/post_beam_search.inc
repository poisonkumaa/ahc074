        // A beam node is the deck before the final independent prefix repairs.
        // Evaluate its STOP action by actually completing those repairs; expand
        // the uncompleted node so first/second/third donor choices interact.
        if(use_post_beam) {
            const clock_t beam_started=clock();
            const double remaining=max(0.0,post_beam_budget-post_beam_cpu);
            clock_t beam_deadline=beam_started+clock_t(remaining*CLOCKS_PER_SEC);
            if(post_beam_call_budget>0)
                beam_deadline=min(beam_deadline,beam_started+clock_t(post_beam_call_budget*CLOCKS_PER_SEC));
            if(route_deadline>0)beam_deadline=min(beam_deadline,route_deadline);
            auto expired=[&](){return clock()>=beam_deadline;};
            post_beam_calls++;
            const int greedy_count=int(turn_moves.size());
            const double greedy_score=turn_moves.size()+route_state_value();
            double best_score=greedy_score;
            RouteState best_state=save_route_state();
            vector<pair<int,int>> best_moves=turn_moves;
            struct BeamNode {
                RouteState state;
                vector<pair<int,int>> moves;
                double score=1.e100;
                int depth=0;
            };
            struct BeamOption {int gain,paper,cut;};
            auto state_key=[&](const RouteState& state) {
                string key;key.reserve(K+1+2*state.supports.size());
                for(int k:state.order)key.push_back(char(k));
                auto supports=state.supports;sort(supports.begin(),supports.end());
                key.push_back(char(supports.size()));
                for(int k:supports){key.push_back(char(k));key.push_back(char(state.cuts[k]+1));}
                return key;
            };
            auto complete_prefix=[&]() {
                uint64_t diff=sim.current_display()^target_mask;
                for(int j=0;j<T;j++)if(diff>>j&1) {
                    auto bp=get_bucket_papers(j,active_main);
                    const char want=(target_mask>>j&1)?'1':'0';
                    while(!bp.empty() && sim.S[bp[0]][j]!=want) {
                        if(!visible_now(bp[0]))return false;
                        int target=-1;
                        for(int i=1;i<int(bp.size());i++)if(sim.S[bp[i]][j]==want){target=i;break;}
                        if(target<0 || turn_moves.size()>=2*K)return false;
                        vector<int> others(bp.begin()+1,bp.end());
                        insert_best(bp[0],others,target,-1,false);
                        bp=get_bucket_papers(j,active_main);
                    }
                    if(bp.empty())return false;
                }
                return turn_moves.size()<=2*K && sim.current_display()==target_mask;
            };
            auto evaluate_stop=[&](BeamNode& node) {
                restore_route_state(node.state);turn_moves=node.moves;
                if(!complete_prefix()){post_beam_invalid++;return false;}
                post_beam_completed++;
                node.score=turn_moves.size()+route_state_value();
                if(node.score<best_score-1.e-9) {
                    best_score=node.score;best_state=save_route_state();best_moves=turn_moves;
                }
                return true;
            };
            map<string,int> visited;
            BeamNode root{post_beam_root,post_beam_prefix,1.e100,0};
            visited.emplace(state_key(root.state),int(root.moves.size()));
            vector<BeamNode> beam;
            // The pre-existing greedy completion is always retained, even if
            // a deadline arrives before this first STOP action can be scored.
            if(!expired() && evaluate_stop(root))beam.push_back(move(root));
            bool timed_out=false;
            for(int depth=0;depth<post_beam_depth && !beam.empty();depth++) {
                vector<BeamNode> next;
                for(const auto& node:beam) {
                    if(expired()){timed_out=true;break;}
                    if(int(node.state.supports.size())>=runtime_support_limit)continue;
                    restore_route_state(node.state);turn_moves=node.moves;
                    post_beam_expanded++;
                    uint64_t diff=sim.current_display()^target_mask;
                    array<uint64_t,K+1> prefix_opaque{};
                    array<uint64_t,65> above_cut{};
                    array<int,64> first_depth;first_depth.fill(K);
                    array<bool,K> loans{};
                    for(int k:active_supports)loans[k]=true;
                    uint64_t locked=0,seen=0;
                    for(int d=0;d<K;d++) {
                        int k=sim.order[d],w=paper_bucket[k];prefix_opaque[d]=seen;
                        if(k>=tot_B && k!=p_new && !loans[k] && first_depth[w]==K) {
                            first_depth[w]=d;locked|=seen&(1ULL<<w);
                        }
                        seen|=opaque[k];
                    }
                    int first_later=K;
                    for(int cut=T-1;cut>=C-1;cut--) {
                        if(cut+1<T)first_later=min(first_later,first_depth[cut+1]);
                        above_cut[cut+1]=first_later<K?prefix_opaque[first_later]:~0ULL;
                    }
                    array<int,64> rank_in_bucket{};
                    vector<BeamOption> options;
                    for(int d=0;d<K;d++) {
                        int k=sim.order[d],w=paper_bucket[k];
                        if(k==p_new || k<tot_B || loans[k])continue;
                        int rank=rank_in_bucket[w]++;
                        if((locked>>w&1) || !(holes>>w&1))continue;
                        uint64_t bad=(black[k]^target_mask)&opaque[k]&index_main_mask;
                        if(bad&~above_cut[w])continue;
                        int cut=C-1,hi=w-1;
                        while(cut<hi) {
                            int mid=cut+(hi-cut)/2;
                            if(bad&~above_cut[mid+1])cut=mid+1;else hi=mid;
                        }
                        uint64_t covered=index_main_mask&opaque[k]&~above_cut[cut+1];
                        int gain=__builtin_popcountll(diff&covered)-rank-1-runtime_return_penalty;
                        // Unlike the greedy rule, nonpositive marginal gains
                        // remain candidates: later donors can change their value.
                        options.push_back({gain,k,cut});
                    }
                    stable_sort(options.begin(),options.end(),[](const auto& x,const auto& y){return x.gain>y.gain;});
                    if(post_beam_options>0 && int(options.size())>post_beam_options)
                        options.resize(post_beam_options);
                    for(const auto option:options) {
                        if(expired()){timed_out=true;break;}
                        restore_route_state(node.state);turn_moves=node.moves;
                        const int chosen=option.paper,chosen_cut=option.cut,w=paper_bucket[chosen];
                        bool legal=true,placed=false;
                        // These are exactly the ordinary donor extraction and
                        // placement operations, with explicit visibility checks.
                        for(int attempts=0;attempts<K;attempts++) {
                            auto bp=get_bucket_papers(w);
                            if(bp.empty() || !visible_now(bp[0]) || turn_moves.size()>=2*K){legal=false;break;}
                            int top=bp[0];
                            if(top==chosen) {
                                int dest=sim.depth_of(chosen);
                                for(int k:sim.order) {
                                    if(k==p_new || k<tot_B)continue;
                                    if(find(active_supports.begin(),active_supports.end(),k)!=active_supports.end())continue;
                                    if(paper_bucket[k]>chosen_cut){dest=sim.depth_of(k);break;}
                                }
                                if(dest>sim.depth_of(chosen)){legal=false;break;}
                                sim.move_paper(chosen,dest,turn_moves);
                                active_supports.push_back(chosen);support_cut[chosen]=chosen_cut;
                                placed=true;break;
                            }
                            if(find(bp.begin(),bp.end(),chosen)==bp.end()){legal=false;break;}
                            send_below_target(top,chosen,bp);
                        }
                        if(!legal || !placed){post_beam_invalid++;continue;}
                        post_beam_generated++;
                        if(option.gain<MIN_GAIN)post_beam_lower_gain++;
                        BeamNode child{save_route_state(),turn_moves,1.e100,depth+1};
                        auto key=state_key(child.state);
                        auto found=visited.find(key);
                        if(found!=visited.end() && found->second<=int(child.moves.size())) {
                            post_beam_duplicates++;continue;
                        }
                        visited[key]=int(child.moves.size());
                        if(!evaluate_stop(child))continue;
                        post_beam_max_depth=max(post_beam_max_depth,child.depth);
                        next.push_back(move(child));
                    }
                    if(timed_out)break;
                }
                stable_sort(next.begin(),next.end(),[](const auto& x,const auto& y){return x.score<y.score;});
                if(int(next.size())>post_beam_width)next.resize(post_beam_width);
                beam=move(next);
                if(timed_out)break;
            }
            if(timed_out)post_beam_timeouts++;
            restore_route_state(best_state);turn_moves=move(best_moves);
            if(best_score<greedy_score-1.e-9)post_beam_changed++;
            post_beam_saved+=greedy_count-int(turn_moves.size());
            post_beam_cpu+=double(clock()-beam_started)/CLOCKS_PER_SEC;
        }
