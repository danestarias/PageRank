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
//#define CHUNKSIZE 80

using namespace std;
// estructura para guardar los nodos y su PR
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
// comparador para ordenar pr 
bool comparator2(nodo a,nodo b)
{
    return (a.getpageRank() > b.getpageRank());
}

//------------------------------------------------made in yamith-------------------------------------------------
map<int,vector<int>> GenListaPi(map<int,vector<int>> nodos){// lista cada nodo con la cantidad de enlaces salientes y me retorna el numerode nodos nulos 
	
		for(map<int,vector<int>>::iterator indx=nodos.begin();indx!=nodos.end();++indx){
			if(indx->second.size() == 0){
				for(map<int,vector<int>>::iterator indx2=nodos.begin();indx2!=nodos.end();++indx2){
					if(indx->first!=indx2->first )
						indx->second.push_back(indx2->first);
			
					}
					
			}
		}
	return nodos;
}
//----------------------------
map<int,vector<int>> GenListpadre(map<int,vector<int>> nodos){
		map<int,vector<int>>  padres;
		
		for(map<int,vector<int>>::iterator indx=nodos.begin();indx!=nodos.end();++indx){
				for(const int &i : indx->second){					
					padres[i].push_back(indx->first);	
				}
		}
		return padres;

}





// iteracion para sacar nuevo pr de cada noddo 
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
	  dic[post];
	  //dic[post].push_back(index);
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
	map<int,vector<int>>  padres;
	map<int,double>  probabilidades;
	map<int,double>  probabilidades2;
	//-----------------------------------------
	double pr=0;
	
	//-------------------------------------------
	bool flag=true;
	vector<nodo> results;

	nodos= loadFile("Wiki-Vote.txt");
	nodos= GenListaPi(nodos);
	padres= GenListpadre(nodos);
			

	
	
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
	





	map<int,vector<int>>::iterator ind;
	ind=nodos.begin();

  	clock_t t;
   	t = clock();

	int iteraciones=0;
	while(flag){
		int convergentes=0;
		double error=0;
		double sum=0;
	

		double auxp=0;
		float  tam = nodos.size();
		for(map<int,vector<int>>::iterator indx=nodos.begin();indx!=nodos.end();++indx){
				
				//auto indx = next(ind, i);
		    	//cout<<indx->first<<endl;
		    	double anterior= probabilidades[indx->first];
		    	
				pr=0;
				for(const int &i : padres[indx->first]){
					  pr +=probabilidades[i] /float(nodos[i].size());				
				}
				
				auxp=(float((1.0-d))/tam ) + d*pr;
				//cout<<auxp<<endl;
				probabilidades2[indx->first] = auxp;
				//cout<<probabilidades2[indx->first]<<"-"<<anterior<<endl;
				sum += auxp;
				//cout<<sum<<endl;
				double res= abs(auxp -anterior);
				error+=res;
				//cout<<sum<<endl;
				if( res < 0.00000001){
					convergentes++;
				}
				//i++;
				//probabilidades2[indx->first] = (float((1.0-d))/tam ) + d*PR;
		
		}

		if(convergentes >= N){
			flag=false;
			break;
		}



		for (map<int,double>::iterator indx=probabilidades2.begin(); indx!=probabilidades2.end(); ++indx ) {
			probabilidades[indx->first] = indx->second;
			//cout<<indx->first<<","<<probabilidades[indx->first]<<endl;
		}

		iteraciones++;
		cout<<"Iteracion "<<iteraciones<<"error="<<error<<" suma de probabilidades = "<<sum<<endl;
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


