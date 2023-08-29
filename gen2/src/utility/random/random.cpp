#include "random.h"
using namespace std;

struct rng_gen{
    random_device rd;
    mt19937_64 generator;
    uniform_real_distribution<double> distribution;

    rng_gen() : generator(mt19937_64(rd())),distribution(uniform_real_distribution<double>(0,100)){}

    long long get(){
        return distribution(generator);
    }
} rng[threads::CPU_COUNT];

struct rng_picker{
	int size;
	double max;
	vector<double> vals;
	
	rng_picker(int size) : vals(vector<double>(size)){}
	
	void _update(int pos,double val,int cur_pos){
		for(int jmp=cur_pos/2;jmp>0;jmp>>=1){
			if(cur_pos<pos){
				cur_pos+=jmp;
				continue;
			}
			vals[cur_pos]+=val;
			if(cur_pos>pos) cur_pos-=jmp;
		}
		
	}

    void update(int pos,double val){_update(pos,val,size/2);}
};