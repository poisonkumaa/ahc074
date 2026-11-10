// Experimental selection controls. Include after OnlineRouter/order_adjustment.
// Each feature is averaged over the conditional support of the next synthetic
// request, so its mean-minus-observation is a conditional zero-mean control.
namespace selection_controls {
using Feature = std::array<double,3>;

static Feature get(const OnlineRouter& r,int item,const std::vector<int>& support) {
    std::array<int,K> depth{};
    std::array<int,64> count{};
    std::array<uint8_t,9> back{};
    int seen=0;
    uint64_t background=0;
    for(int k:r.sim.order) {
        if(k<r.tot_B) { back[seen++]=k;continue; }
        if(k==r.active_main)continue;
        const int w=r.paper_bucket[k];
        depth[k]=count[w]++;
        if(depth[k]==0)background|=r.black[k]&(1ULL<<w);
    }
    const int state=r.graph && seen==r.tot_B?r.graph->table.rank(back):-1;
    auto feature=[&](int a) {
        Feature f{double(depth[r.prod_to_paper[a]]),
            double(__builtin_popcountll((background^r.A[a])&r.index_main_mask&~r.opaque[r.prod_to_paper[a]])),0};
        if(state>=0) {
            const auto& req=r.graph->req[a];
            if(req.cached_distance)f[2]=req.cached_distance->distance[state];
        }
        return f;
    };
    Feature expected{};
    for(int a:support) {
        const auto f=feature(a);
        for(int j=0;j<3;j++)expected[j]+=f[j];
    }
    const auto observed=feature(item);
    if(support.empty())return {};
    for(int j=0;j<3;j++)expected[j]=expected[j]/support.size()-observed[j];
    return expected;
}

// Fit one common coefficient vector to paired layout differences. Statistics
// are updated AFTER scoring the whole current query. Thus coefficients chosen
// before the next draw are predictable and preserve the zero-mean property.
// The common query difficulty is removed by centering over live candidates.
struct PairedFit {
    Feature prior{.9,.54,.9},beta{.9,.54,.9},xy{};
    std::array<Feature,3> xx{};
    int observations=0;
    double ridge=32,maximum=2;

    PairedFit() {
        ridge=std::max(1e-6,search_budget::real("SEL4_RIDGE",32));
        maximum=std::max(.1,search_budget::real("SEL4_COEF_MAX",2));
    }

    void prepare() {
        if(observations<32 || observations%16)return;
        double a[3][4]{};
        for(int i=0;i<3;i++) {
            for(int j=0;j<3;j++)a[i][j]=xx[i][j]+(i==j?ridge:0);
            a[i][3]=xy[i]+ridge*prior[i];
        }
        for(int col=0;col<3;col++) {
            int pivot=col;
            for(int row=col+1;row<3;row++)
                if(std::abs(a[row][col])>std::abs(a[pivot][col]))pivot=row;
            if(std::abs(a[pivot][col])<1e-12)return;
            if(pivot!=col)for(int j=col;j<4;j++)std::swap(a[col][j],a[pivot][j]);
            const double divisor=a[col][col];
            for(int j=col;j<4;j++)a[col][j]/=divisor;
            for(int row=0;row<3;row++)if(row!=col) {
                const double factor=a[row][col];
                for(int j=col;j<4;j++)a[row][j]-=factor*a[col][j];
            }
        }
        Feature proposal{};
        for(int j=0;j<3;j++) {
            if(!std::isfinite(a[j][3]))return;
            proposal[j]=std::clamp(a[j][3],0.0,maximum);
        }
        beta=proposal;
    }

    double correction(const Feature& f)const {
        double z=0;for(int j=0;j<3;j++)z+=beta[j]*f[j];return z;
    }

    void observe(const std::vector<Feature>& control,const std::vector<double>& cost,
                 const std::vector<bool>& live) {
        int n=0;Feature mean{};double cost_mean=0;
        for(int i=0;i<int(live.size());i++)if(live[i]) {
            n++;cost_mean+=cost[i];
            for(int j=0;j<3;j++)mean[j]+=control[i][j];
        }
        if(n<2)return;
        cost_mean/=n;for(double& x:mean)x/=n;
        for(int i=0;i<int(live.size());i++)if(live[i]) {
            Feature z{};for(int j=0;j<3;j++)z[j]=control[i][j]-mean[j];
            for(int j=0;j<3;j++) {
                xy[j]-=z[j]*(cost[i]-cost_mean)/n;
                for(int k=0;k<3;k++)xx[j][k]+=z[j]*z[k]/n;
            }
        }
        observations++;
    }
};
} // namespace selection_controls
