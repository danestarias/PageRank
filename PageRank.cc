#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <math.h>
#include <time.h>
#include <algorithm>
#include <map>

#include <omp.h>
#define CHUNKSIZE 80

using namespace std;

class nodo{

    private:
    	int id;
        double pageRank;			

    public:
        //CONSTRUCTORES
        nodo(){}
        nodo(const int &id,const double &pageRank){
            this->id = id;
            this->pageRank = pageRank;
        }
        //GETTERS
        const int &getid() const{
            return id;
        }
        const double &getpageRank() const{
            return pageRank;
        }
};

bool comparator2(nodo a,nodo b)
{
    return (a.getpageRank() > b.getpageRank());
}

double pageRank(map<int,vector<int>> nodos, map<int,double> probabilidades, int nodo, double d){
	double PR=0;
	double result=0;

	for(map<int,vector<int>>::iterator indx=nodos.begin();indx!=nodos.end();++indx){
		if(indx->second.size() != 0){

			for(const int &i : indx->second){					//itero los apuntadores de los nodos
				if(i == nodo){	
					PR += float(probabilidades[indx->first]) / float(indx->second.size());
					//cout<<nodo<<"..."<<indx->first<<"++++"<<probabilidades[indx->first]<<endl;
				}
			}

		}
			
	}



	result = (float((1.0-d))/float( nodos.size()) ) + d*PR;
	return result;

}



map<int, vector<int>> loadFile(string name) {
	int index,post;
	map<int, vector<int>> dic;
	ifstream fe(name);
	
	while(fe >>index >> post){
	  dic[index].push_back(post);
	  dic[post].push_back(index);
	  /*if(dic[post].size() == 0){
	  	//dic[post];//dic[post].push_back(index);
	  }*/
	  
	  //cout<<index<<":"<< post<<endl;
	}
	fe.close();

  	return dic;
}


int main(int argc, char **argv) {


	int N;			
	double d=0.85;
	string pivo="";
	map<int,vector<int>>  nodos;
	map<int,double>  probabilidades;
	map<int,double>  probabilidades2;

	bool flag=true;
	vector<nodo> results;

	nodos= loadFile("Wiki-Vote.txt");
	N=nodos.size();
	cout<<"tamDic= "+to_string(N)<<endl;

	double probInit = float(1)/float(N);
	cout<<"prob init->"<<probInit<<endl;
	
	bool flagone=true;
	for(map<int,vector<int>>::iterator indx=nodos.begin();indx!=nodos.end();++indx){
		if(flagone){
			probabilidades[indx->first]=1;
			flagone=false;
		}else{
			probabilidades[indx->first]=0;
		}
		//cout<<indx->first<<"++++"<<probabilidades[indx->first]<<endl;

	}


	cout<<"Probabilidades= "<<probabilidades.size()<<endl;

	



 


	int nthreads, tid, i, chunk;
	chunk = CHUNKSIZE;

	map<int,vector<int>>::iterator ind;
	ind=nodos.begin();

  	clock_t t;
   	t = clock();

	int iteraciones=0;
	while(flag){
		int convergentes=0;
		double sum=0;


		//#pragma omp parallel shared(probabilidades,nodos,ind,convergentes,sum,N,d,nthreads,chunk) private(i,tid,anterior,res,auxp)
		//{
		    tid = omp_get_thread_num();
		    if (tid == 0){
		      nthreads = omp_get_num_threads();
		      printf("Number of threads = %d\n", nthreads);

		    }
		    printf("Thread %d starting...\n",tid);

		    //--------------------------------------------------------------------------
		    //#pragma omp for reduction(+:sum,convergentes)
		    for (i=0; i<N; i++){

		    	auto indx = next(ind, i);
		    	//cout<<indx->first<<endl;
		    	double anterior= probabilidades[indx->first];
		    	double auxp = pageRank(nodos, probabilidades, indx->first, d);	//PageRank
				probabilidades2[indx->first] = auxp;
				//cout<<probabilidades2[indx->first]<<"-"<<anterior<<endl;
				sum += auxp;
				double res= abs(auxp -anterior);
				//cout<<sum<<endl;
				if( res < 0.00000001){
					convergentes++;
				}
				//printf("Thread %d: probabilidades[%d]= %f\n",tid,indx->first,probabilidades[indx->first]);
		    }
		//}  


		if(convergentes >= N){
			flag=false;
			break;
		}



		for (map<int,double>::iterator indx=probabilidades2.begin(); indx!=probabilidades2.end(); ++indx ) {
			probabilidades[indx->first] = indx->second;
			//cout<<indx->first<<","<<probabilidades[indx->first]<<endl;
		}

		iteraciones++;
		cout<<"Iteracion "<<iteraciones<<" suma de probabilidades = "<<sum<<endl;
	}

  	//guardo resultados finales en un vector para ordenarlo
	for (map<int,double>::iterator indx=probabilidades.begin(); indx!=probabilidades.end(); ++indx ) {
		//cout<<indx->first<<"-->"<<indx->second<<";";
		nodo n(indx->first,indx->second);
		results.push_back(n);
	}
	make_heap (results.begin(),results.end(),comparator2);
	sort_heap(results.begin(),results.end(),comparator2);
	t = clock() - t;
  	printf ("Tiempo del algoritmo centralizado: (%f seconds).\n",((float)t)/CLOCKS_PER_SEC);
  	//imprimo resultados
  	for(const nodo &n : results){
  		cout<<"nodo : "<<n.getid()<<" Probabilidad :"<<n.getpageRank()<<endl;
  	}

	cout<<"\nfin del algoritmo"<<endl;

	return 0;
}


/*
secuencial 35 iteraciones: 55 min 27 sec
secuencial 35 iteraciones: 52 min 54 sec
paralelo chunksize=10 static 45 iteraciones: 27 min 30 sec -----12296.817383 seconds
paralelo chunksize=80 static 43 iteraciones: 26 min 10 sec -----11553.088867 
paralelo chunksize=80 static? 28 iteraciones: 16 min 32 sec -----7447.104980

paralelo chunksize=10 dynamic 61 iteraciones: >35 min 30 sec -----12296.817383 seconds
*/



