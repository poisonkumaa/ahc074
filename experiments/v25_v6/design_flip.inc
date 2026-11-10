// Exact nonlinear repair-demand objective for the final support-aware anchor SA.
// z/o are the same weighted exposed-product counts used by count_demand.
// The marginal of 2*z*o/(z+o) w.r.t. z is 2*(o/(z+o))^2, equal to
// beta(.5) * adjust_prices' factor4 when mix=1. No extra beta applies here.
namespace design_flip {
template<class Rows> struct Model {
    int C,T;
    const Rows& prices;
    std::array<double,64> z{},o{},dz{},doo{};
    bool verify=false;
    long long verified=0;
    double largest_error=0;

    Model(int c,int t,const Rows& p):C(c),T(t),prices(p) {
        z.fill(4.);o.fill(4.);
        verify=search_budget::integer("DESIGN_FLIP_VERIFY",0)!=0;
    }
    static double flip(double zero,double one) {
        assert(zero>0 && one>0);
        return 2*zero*one/(zero+one);
    }
    void contribute(std::array<double,64>& zero,std::array<double,64>& one,
                    int w,uint64_t bits,uint64_t diff,bool paired,double sign)const {
        for(int j=C;j<T;j++)if(j>w || (diff>>j&1)) {
            const double weight=sign*prices[w][j/8][1<<(j%8)];
            ((bits>>j&1)?one[j]:zero[j])+=weight;
            if(paired)(((bits^diff)>>j&1)?one[j]:zero[j])+=weight;
        }
    }
    void add(int w,uint64_t bits,uint64_t diff,bool paired) {
        contribute(z,o,w,bits,diff,paired,1.);
    }
    double value()const {
        double answer=0;for(int j=C;j<T;j++)answer+=flip(z[j],o[j]);return answer;
    }
    void begin_delta(){dz.fill(0.);doo.fill(0.);}
    void change(int w,uint64_t bits,uint64_t diff,bool paired,double sign) {
        contribute(dz,doo,w,bits,diff,paired,sign);
    }
    double delta()const {
        double answer=0;
        for(int j=C;j<T;j++)answer+=flip(z[j]+dz[j],o[j]+doo[j])-flip(z[j],o[j]);
        return answer;
    }
    void commit() {for(int j=C;j<T;j++){z[j]+=dz[j];o[j]+=doo[j];}}
    template<class Items> double recompute(const Items& items,const std::vector<int>& positions)const {
        std::array<double,64> zero{},one{};zero.fill(4.);one.fill(4.);
        for(int x=0;x<int(items.size());x++) {
            const auto& item=items[x];
            contribute(zero,one,positions[x],item.bits,item.diff,item.type,1.);
        }
        double answer=0;for(int j=C;j<T;j++)answer+=flip(zero[j],one[j]);return answer;
    }
    template<class Items> void check(const Items& items,const std::vector<int>& proposed) {
        if(!verify)return;
        const double reference=recompute(items,proposed),incremental=value()+delta();
        const double error=std::abs(reference-incremental);
        verified++;largest_error=std::max(largest_error,error);
        if(error>1e-7*std::max(1.,std::abs(reference))) {
            std::cerr<<"DESIGN_FLIP_VERIFY failure reference="<<reference<<" incremental="<<incremental<<"\n";
            std::abort();
        }
    }
};
} // namespace design_flip
