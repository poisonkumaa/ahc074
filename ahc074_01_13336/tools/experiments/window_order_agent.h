#pragma once
inline void optimize_window_columns(const vector<U64>&A,vector<int>&cols,const int correlation[64][64],int iterations,int mode,double pairweight,int width,double temperature){
 const int P=A.size(),N=64,W=N-width+1;mt19937_64 rng(163417);vector<vector<uint16_t>>code(W,vector<uint16_t>(P));vector<double>value(W);vector<int>hist(1<<width);
 auto score=[&](const vector<uint16_t>&c){fill(hist.begin(),hist.end(),0);for(int x:c)hist[x]++;double s=0;for(int n:hist)if(mode==0)s+=n>0;else s-=n*n*.25;return s;};
 double current=0;for(int w=0;w<W;w++){for(int v=0;v<P;v++)for(int b=0;b<width;b++)code[w][v]|=((A[v]>>cols[w+b])&1)<<b;value[w]=score(code[w]);current+=value[w];}
 auto edge=[&](int x,int y){int d=correlation[x][y];return min(d,P-d);};int pairs=0;for(int x=1;x<N;x++)pairs+=edge(cols[x-1],cols[x]);current+=pairweight*pairs;double initial=current,best=current;auto bestcols=cols;
 vector<uint8_t>different(P);vector<uint16_t>temporary(P);int accepted=0;clock_t begun=clock();
 for(int it=0;it<iterations;it++){
  int x=rng()%N,y=rng()%N;if((rng()%4)==0)y=(x+1)%N;if(x==y)continue;if(x>y)swap(x,y);
  vector<int>edges;for(int e:{x-1,x,y-1,y})if(e>=0&&e<N-1&&find(edges.begin(),edges.end(),e)==edges.end())edges.push_back(e);
  int deltaPair=0;for(int e:edges)deltaPair-=edge(cols[e],cols[e+1]);swap(cols[x],cols[y]);for(int e:edges)deltaPair+=edge(cols[e],cols[e+1]);swap(cols[x],cols[y]);
  for(int v=0;v<P;v++)different[v]=((A[v]>>cols[x])^(A[v]>>cols[y]))&1;
  int affected[24],mask[24],count=0;double nextvalue[24];double delta=pairweight*deltaPair;
  for(int w=0;w<W;w++){bool a=w<=x&&x<w+width,b=w<=y&&y<w+width;if(!a&&!b)continue;int m=(a?1<<(x-w):0)|(b?1<<(y-w):0);affected[count]=w;mask[count]=m;nextvalue[count]=value[w];if(a!=b){for(int v=0;v<P;v++)temporary[v]=code[w][v]^(different[v]*m);nextvalue[count]=score(temporary);delta+=nextvalue[count]-value[w];}count++;}
  double temp=temperature*pow(.005,double(it)/max(1,iterations));
  if(delta<=0||double(rng()>>11)*0x1p-53<exp(-delta/temp)){
   accepted++;swap(cols[x],cols[y]);pairs+=deltaPair;current+=delta;for(int i=0;i<count;i++){int w=affected[i],m=mask[i];value[w]=nextvalue[i];for(int v=0;v<P;v++)code[w][v]^=different[v]*m;}
   if(current<best){best=current;bestcols=cols;}
  }
 }
 cols=bestcols;cerr<<"Window objective "<<initial<<" -> "<<best<<" accepted="<<accepted<<" CPU="<<double(clock()-begun)/CLOCKS_PER_SEC<<"\n";
}
