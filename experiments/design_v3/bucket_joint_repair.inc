        // Jointly reinsert the wrong-color prefix of one ordinary home bucket.
        // The first matching sheet stays at the front, all untouched tail
        // sheets retain their relative order, and every blocker moves once.
        // This is a subset DP, not a beam or a future-request rollout.
        auto joint_prefix_repair_plan=[&](const vector<int>& bucket,int w,char want) {
            vector<int> empty;
            if(!bucket_joint_enabled || !route_branching || w<C || w>=T || w==w_new)return empty;
            for(int k:active_supports)if(paper_bucket[k]==w)return empty;
            const int n=int(bucket.size());
            int q=0;while(q<n && sim.S[bucket[q]][w]!=want)q++;
            if(q<2 || q>8 || q>=n || n>16)return empty;
            string key;key.reserve(n+2);key.push_back(char(w));key.push_back(want);
            for(int k:bucket)key.push_back(char(k));
            auto cached=bucket_joint_cache.find(key);
            if(cached!=bucket_joint_cache.end()){bucket_joint_hits++;return cached->second;}
            if(bucket_joint_cpu>=bucket_joint_budget || (route_deadline>0 && clock()>=route_deadline))return empty;
            const clock_t started=clock();
            struct CpuScope {double& total;clock_t start;~CpuScope(){total+=double(clock()-start)/CLOCKS_PER_SEC;}};
            CpuScope scope{bucket_joint_cpu,started};
            clock_t deadline=started+clock_t((bucket_joint_budget-bucket_joint_cpu)*CLOCKS_PER_SEC);
            if(route_deadline>0)deadline=min(deadline,route_deadline);
            bucket_joint_calls++;bucket_joint_blockers+=q;
            const int head=bucket[q],head_color=want-'0';
            vector<int> free(bucket.begin(),bucket.begin()+q);
            vector<int> tail(bucket.begin()+q+1,bucket.end());
            const int m=int(tail.size()),masks=1<<q,stride=m+1;
            vector<int> tail_seen(m+1,1<<head_color);
            for(int j=0;j<m;j++)tail_seen[j+1]=tail_seen[j]|(1<<(sim.S[tail[j]][w]-'0'));
            vector<double> dp(masks*stride,1.e100);
            vector<int8_t> parent(masks*stride,-1);
            dp[0]=(1+int(paper_is_paired[head]))-support_order_weight*support_utility[head][0];
            for(int mask=0;mask<masks;mask++) {
                if((mask&31)==0 && clock()>=deadline){bucket_joint_aborts++;return empty;}
                for(int j=0;j<=m;j++) {
                    const int state=mask*stride+j,position=1+__builtin_popcount(unsigned(mask))+j;
                    const double old=dp[state];if(old>1.e90)continue;
                    const int seen=tail_seen[j]|(mask?(1<<(head_color^1)):0);
                    auto append=[&](int paper,int choice,int next) {
                        const int color=sim.S[paper][w]-'0';
                        double delta=(1+int(paper_is_paired[paper]))*(position+1);
                        if(!(seen>>color&1))delta+=.5*hole_request_count[w][color]*position;
                        if(position==1 && color!=head_color)
                            delta+=.5*(int(paper_is_paired[head])-int(paper_is_paired[paper]));
                        if(position<32)delta-=support_order_weight*support_utility[paper][position];
                        const double value=old+delta;
                        if(value<dp[next]-1.e-10){dp[next]=value;parent[next]=int8_t(choice);}
                    };
                    for(int i=0;i<q;i++)if(!(mask>>i&1))append(free[i],i,(mask|(1<<i))*stride+j);
                    if(j<m)append(tail[j],q,mask*stride+j+1);
                }
            }
            vector<int> plan;plan.reserve(n);
            int mask=masks-1,j=m;
            while(mask || j) {
                const int choice=parent[mask*stride+j];assert(choice>=0);
                if(choice==q){plan.push_back(tail[j-1]);j--;}
                else {plan.push_back(free[choice]);mask^=1<<choice;}
            }
            reverse(plan.begin(),plan.end());plan.insert(plan.begin(),head);

            // Reconstruct the existing greedy repair on local bucket vectors.
            // Its insertion objective (including the MDP correction) is exactly
            // the same as insert_best(..., omitted=-1, is_return=false).
            vector<int> greedy=bucket;
            while(sim.S[greedy[0]][w]!=want) {
                const int paper=greedy[0];greedy.erase(greedy.begin());
                int first=0;while(first<int(greedy.size()) && sim.S[greedy[first]][w]!=want)first++;
                int where=first+1;double best=1.e100;
                for(int pos=first+1;pos<=int(greedy.size());pos++) {
                    auto candidate=greedy;candidate.insert(candidate.begin()+pos,paper);
                    double value=future_order_cost(candidate,-1,true);
                    if(value<best){best=value;where=pos;}
                }
                greedy.insert(greedy.begin()+where,paper);
            }
            const double old_proxy=future_order_cost(greedy,-1,false);
            const double new_proxy=future_order_cost(plan,-1,false);
#ifdef BUCKET_JOINT_VERIFY
            assert(abs(new_proxy-dp.back())<1.e-6);
#endif
            // Keep the ordinary continuation on ties and if its existing MDP
            // insertion model prefers it. The DP's additive objective is exact;
            // the future cost represented by that objective remains a proxy.
            if(new_proxy>=old_proxy-1.e-7 ||
               future_order_cost(plan,-1,true)>future_order_cost(greedy,-1,true)+1.e-7) {
                bucket_joint_cache.emplace(move(key),empty);return empty;
            }
            bucket_joint_improved++;bucket_joint_proxy_gain+=old_proxy-new_proxy;
            bucket_joint_cache.emplace(move(key),plan);
            return plan;
        };
