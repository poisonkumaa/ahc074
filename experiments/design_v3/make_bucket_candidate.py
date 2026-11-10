from pathlib import Path
import argparse

HERE = Path(__file__).resolve().parent
parser = argparse.ArgumentParser()
parser.add_argument('--source', type=Path, default=HERE / 'main_before_append.cpp')
parser.add_argument('--target', type=Path, default=HERE / 'bucket_candidate.cpp')
args = parser.parse_args()
source = args.source.read_text(encoding='utf-8')

def replace_once(old, new):
    global source
    assert source.count(old) == 1, (old[:100], source.count(old))
    source = source.replace(old, new)

anchor = '    map<string,vector<int>> joint_extract_cache;\n'
replace_once(anchor, anchor + '''    bool bucket_joint_enabled=search_budget::integer("BUCKET_JOINT_REPAIR",0)!=0;
    double bucket_joint_budget=search_budget::seconds(max(0.0,search_budget::real("BUCKET_JOINT_CPU",.12)));
    double bucket_joint_cpu=0,bucket_joint_proxy_gain=0;
    long long bucket_joint_calls=0,bucket_joint_hits=0,bucket_joint_improved=0;
    long long bucket_joint_applied=0,bucket_joint_aborts=0,bucket_joint_blockers=0;
    map<string,vector<int>> bucket_joint_cache;
''')
anchor = '    vector<pair<int,int>> query(int a) {\n        used_tails=0;\n'
replace_once(anchor, anchor + '        bucket_joint_cache.clear();\n')
anchor = '        uint64_t cur_disp = sim.current_display();\n'
replace_once(anchor, (HERE / 'bucket_joint_repair.inc').read_text(encoding='utf-8') + '\n' + anchor)
anchor = '''                char want_bit = ((target_mask >> j) & 1) ? '1' : '0';
                while (sim.S[bp[0]][j] != want_bit) {
'''
replace_once(anchor, '''                char want_bit = ((target_mask >> j) & 1) ? '1' : '0';
                auto joint_plan=joint_prefix_repair_plan(bp,j,want_bit);
                if(!joint_plan.empty()) {
                    int q=0;while(sim.S[bp[q]][j]!=want_bit)q++;
                    array<bool,K> placed{};
                    for(int i=q;i<int(bp.size());i++)placed[bp[i]]=true;
                    for(int i=0;i<q;i++) {
                        const int paper=bp[i];
                        const int index=int(find(joint_plan.begin(),joint_plan.end(),paper)-joint_plan.begin());
                        int next=-1,last=-1;
                        for(int t=index+1;t<int(joint_plan.size());t++)if(placed[joint_plan[t]]){next=joint_plan[t];break;}
                        if(next<0)for(int t=int(joint_plan.size())-1;t>=0;t--)if(placed[joint_plan[t]]){last=joint_plan[t];break;}
                        assert(next>=0 || last>=0);
                        const int destination=next>=0?sim.depth_of(next)-1:sim.depth_of(last);
                        sim.move_paper(paper,destination,turn_moves);placed[paper]=true;
                    }
#ifdef BUCKET_JOINT_VERIFY
                    assert(get_bucket_papers(j,active_main)==joint_plan);
                    assert(((sim.current_display()^target_mask)>>j&1)==0);
#endif
                    bucket_joint_applied++;
                    continue;
                }
                while (sim.S[bp[0]][j] != want_bit) {
''')
anchor = '    cerr<<"EXTRACT_DP calls="<<actual.joint_extract_calls'
replace_once(anchor, '''    if(actual.bucket_joint_enabled)cerr<<"BUCKET_JOINT calls="<<actual.bucket_joint_calls
        <<" cache_hits="<<actual.bucket_joint_hits<<" improved="<<actual.bucket_joint_improved
        <<" applied="<<actual.bucket_joint_applied<<" blockers="<<actual.bucket_joint_blockers
        <<" proxy_gain="<<actual.bucket_joint_proxy_gain<<" aborted="<<actual.bucket_joint_aborts
        <<" cpu="<<actual.bucket_joint_cpu<<"\\n";
''' + anchor)
args.target.write_text(source, encoding='utf-8', newline='\n')
print(args.target)
