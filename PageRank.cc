#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <math.h>
#include <time.h>
#include <algorithm>
#include <map>

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

	for(map<int,vector<int>>::iterator indx=nodos.begin();indx!=nodos.end();++indx){
		for(const int &i : indx->second){					//itero los apuntadores de los nodos
			if(i == nodo){						//busco el nodo
				PR += float(probabilidades[indx->first]) / float(indx->second.size());
			}
		}
	}

	PR = (float(1-d)/float( probabilidades.size()) ) + d * PR;
	return PR;

}

map<int,double> initProbabilidades(map<int, vector<int>> nodos, double initValue){
	map<int,double> probabilidades;
	for(map<int,vector<int>>::iterator indx=nodos.begin();indx!=nodos.end();++indx){
		probabilidades[indx->first]=initValue;
	}
	return probabilidades;
}

map<int, vector<int>> loadFile(string name) {
	int index,post;
	map<int, vector<int>> dic;
	ifstream fe(name);
	
	while(fe >>index >> post){
	  dic[index].push_back(post);
	  dic[post].push_back(index);
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
	bool flag=true;
	vector<nodo> results;

	nodos= loadFile("Wiki-Vote.txt");
	N=nodos.size();
	cout<<"tamDic= "+to_string(N)<<endl;

	double probInit = float(1)/float(N);
	cout<<"prob init->"<<probInit<<endl;
	probabilidades = initProbabilidades(nodos,probInit);	//asigno probabilidades iguales

	cout<<"Probabilidades= "<<probabilidades.size()<<endl;

	clock_t t;
   	t = clock();

	int iteraciones=0;
	while(flag){
		int convergentes=0;
		double sum=0;

		for (map<int,vector<int>>::iterator indx=nodos.begin(); indx!=nodos.end(); ++indx ) {
			double anterior= probabilidades[indx->first];

			probabilidades[indx->first] = pageRank(nodos, probabilidades, indx->first, d);	//PageRank
			//cout<<probabilidades[indx->first]<<"-"<<anterior<<endl;
			sum += probabilidades[indx->first];
			double res= abs(probabilidades[indx->first] -anterior);
			//cout<<sum<<endl;
			if( res < 0.00000001){
				convergentes++;
				//cout<<"convergentes : "<<convergentes<<endl;
				if(convergentes >= N){
					flag=false;
					break;
				}
			}
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




