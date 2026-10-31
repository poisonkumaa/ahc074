from pathlib import Path

root = Path(__file__).resolve().parents[2]
old = (root / 'tools/experiments/main_26492.cpp').read_text()
prefix = old[:old.index('\nint main()')]
prefix = prefix.replace('iteration < 32', 'iteration < 24')
code = (root / 'tools/experiments/direct_groups.cpp').read_text()
code = code[code.index('struct CostModel'):]
code = code.replace('    vector<double> initial;',
                    '    vector<double> initial;\n    mutable vector<double> workspace;')
code = code.replace('n=states.size();initial.assign(n,1./n);',
                    'n=states.size();initial.assign(n,1./n);workspace.resize(3*n);')
code = code.replace('double now[100],updated[100],reward[100];',
                    'double* now=workspace.data();\n'
                    '        double* updated=now+n;\n'
                    '        double* reward=updated+n;')
code = code.replace('copy(updated,updated+n,now);', 'swap(now,updated);')
code = code.replace('reward[s]-=double(length[s][display[s]])/(products-1);',
                    'if(count[display[s]]) reward[s]-=double(length[s][display[s]])/(products-1);\n'
                    '            else reward[s]*=double(products-1)/products;')
code = code.replace('int main(int argc,char**argv){', 'int main(){')
code = code.replace('int iterations=argc>1?atoi(argv[1]):30000;',
                    'constexpr int iterations=100000;\n'
                    '    constexpr double search_cpu_budget=0.95;')
code = code.replace('    auto started=chrono::steady_clock::now();\n', '')
code = code.replace('    bestgroups=groups;double best=current;\n    for(int it=0;it<iterations;it++){',
                    '    bestgroups=groups;double best=current;\n'
                    '    // CPU time excludes reactive I/O waits and limits the expensive search.\n'
                    '    const clock_t search_start=clock();\n'
                    '    double timed_progress=0;\n'
                    '    for(int it=0;it<iterations;it++){\n'
                    '        if((it&127)==0){\n'
                    '            double used=double(clock()-search_start)/CLOCKS_PER_SEC;\n'
                    '            if(used>=search_cpu_budget) break;\n'
                    '            timed_progress=used/search_cpu_budget;\n'
                    '        }')
code = code.replace('double temp=0.018*pow(0.015,double(it)/iterations);',
                    'double temp=0.018*pow(0.015,max(double(it)/iterations,timed_progress));')
code = '\n'.join(line for line in code.splitlines() if 'cerr<<"cost="' not in line) + '\n'
(root / 'main.cpp').write_text(prefix + '\n' + code)
