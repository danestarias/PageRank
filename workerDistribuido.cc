#include <iostream>
#include <string>
#include <cassert>
#include <zmqpp/zmqpp.hpp>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <math.h>
#include <cmath>

#include <omp.h>
#define CHUNKSIZE 10

using namespace std;
using namespace zmqpp;

class Kcenters{
  private:
    string name; 
    vector<double> kdimension;

  public:
    // CONSTRUCTOR
    Kcenters(){}
    Kcenters(const string name) {
      this->name = name;
    }
    // GETTERS
    const string &getname() const { return name; }
    
    void insert(string d){
      this->kdimension.push_back(stod(d));
    }

    vector<double> getvector(){
      return kdimension; 
    }

};

double get_distance(int x1, int x2, int y1, int y2){
  return sqrt( (x2-x1)*(x2-x1) + (y2-y1)*(y2-y1) );
}


string doubleToString(double dbl){
  ostringstream strs;
  strs << dbl;
  return strs.str();
}

vector<int> splitInt(string str, char delimiter) {
  vector<int> internal;
  stringstream ss(str); // Turn the string into a stream.
  string tok;

  while (getline(ss, tok, delimiter)) {
    internal.push_back(atoi(tok.c_str()));
  }

  return internal;
}

vector<string> split(string str, char delimiter) {
  vector<string> internal;
  stringstream ss(str); // Turn the string into a stream.
  string tok;

  while (getline(ss, tok, delimiter)) {
    internal.push_back(tok);
  }

  return internal;
}



double pageRank(map<int,vector<int>> nodos, map<int,double> probabilidades, int nodo, double d){
  double PR=0;
  double result=0;

  for(map<int,vector<int>>::iterator indx=nodos.begin();indx!=nodos.end();++indx){
    if(indx->second.size() != 0){

      for(const int &i : indx->second){         //itero los apuntadores de los nodos
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

//------------------------------------------------------------------------------------------
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
//-------------------------------------------------------------------------
map<int,vector<int>> GenListpadre(map<int,vector<int>> nodos){
		map<int,vector<int>>  padres;
		
		for(map<int,vector<int>>::iterator indx=nodos.begin();indx!=nodos.end();++indx){
				for(const int &i : indx->second){					
					padres[i].push_back(indx->first);	
				}
		}
		return padres;

}



map<int, vector<int>> loadFile(string name) {
  int index,post;
  map<int, vector<int>> dic;
  ifstream fe(name);
  
  while(fe >>index >> post){
    dic[index].push_back(post);
	dic[post];
  }
  fe.close();

    return dic;
}

//o<<cantK(int)<<"k1:2,3&k2:2,3"<<canRelaciones<<"1:2,3,5,7,10,2000&2:1,3,5,7,10,2000"

int main(int argc, char **argv){
  if (argc != 3) {
    cout << "error en la llamada" << endl;
    return 1; // ./server 2222
  }

  string direccionServer(argv[1]);
  string address("tcp://" + direccionServer);
  cout << "suscrito a server: " << address << endl;

  string direccionServer2(argv[2]);
  string address2("tcp://" + direccionServer2);
  cout << "Envio a server: " << address2 << endl;

  cout<<"SOY UN WORKER\n";

  map<int,vector<int>>  nodos;
  map<int,vector<int>>  padres;
  double pr=0;
  //cargo nodos -- lista de adya
  nodos= loadFile("Wiki-Vote.txt");
  nodos= GenListaPi(nodos);
  padres= GenListpadre(nodos);
  
  int N=nodos.size();
  cout<<"tamDic= "+to_string(N)<<endl;
  double d=0.85;

  // SOCKET RECIBO
  context ctx;
  socket s(ctx,socket_type::pull);
  cout << "Conectando al puerto tcp 5220\n";
  s.connect(address); //digo direccion del socket del servidor
  poller p;
  p.add(s, poller::poll_in);
  cout<<"Listo para recibir"<<endl;
  

  context ctx2;
  socket t(ctx2,socket_type::xreq);
  t.connect(address2); //DIRECCION para enviar



  while(true){
    if (p.poll(500)) {
      cout<< "\nLLEGO ALGO!!"<<endl;
      cout<<"Recibo probabilidades"<<endl;
      vector<string> listProb;
      string strProbs="";   //string para recibir probabilidades
      string strNodos="";   //string para recibir nodos a sacarles el pageRank

      message r;
      s.receive(r);
      r>>strProbs;
      r>>strNodos;
      //cout<<"probablididades->"<<strProbs<<endl;

      // Extraigo informacion de las probabilidades
      map<int,double>  probabilidades;
      map<int,double>  probabilidades2;
      vector<string> auxNodo;

      listProb = split(strProbs,'&');               // 1:2
      cout<<"listProb = "<<listProb.size()<<endl;

      for (string &nodo: listProb) {
        auxNodo = split(nodo,':');                  // 1 | 2
        probabilidades[atoi(auxNodo[0].c_str())] = stod(auxNodo[1]);
      }
      cout<<"cantidad de probabilidades = "<<probabilidades.size()<<endl;



      // Extraigo informacion de los nodos
      vector<int> listNodos;
      listNodos = splitInt(strNodos,'&');               // 1 | 2
      int listNodosSize = listNodos.size();
      cout<<"listNodos = "<<listNodosSize<<endl;

      //le asigno parte de los nodos a cada procesador
      int nthreads, tid, i, chunk;

      int convergentes=0;
      double sum=0;
      double error=0;

      double res=0;
      double anterior=0;
      double auxp=0;

      chunk = CHUNKSIZE;
      string results="";

      //#pragma omp parallel shared(probabilidades,nodos,convergentes,sum,results,N,d,nthreads) private(i,tid,res,anterior,auxp)
      {
          tid = omp_get_thread_num();
          if (tid == 0){
            nthreads = omp_get_num_threads();
            printf("Number of threads = %d\n", nthreads);

          }
          printf("Thread %d starting...\n",tid);

          //--------------------------------------------------------------------------
          //#pragma omp for reduction(+:sum,convergentes,error,results)
          //#pragma omp for schedule(static,chunk)
          //double auxp=0;
          float  tam = nodos.size();
          for (i=0; i<listNodosSize; i++){

            int node = listNodos[i];     //contiene la llave (nodo a sacarle el pageRank)
            anterior = probabilidades[node];//extraigo la probabilidad para luego compararla con la nueva


			pr=0;
			for(const int &z : padres[node]){
				  pr +=probabilidades[z] /float(nodos[z].size());				
			}
			
			auxp =(float((1.0-d))/tam ) + d*pr;
            
            probabilidades2[node] = auxp;

            sum += auxp;
            res= abs(auxp - anterior);
            error+= res;
            if( res < 0.00000001){
              convergentes++;
            }
            results += to_string(node) + ":" + doubleToString(auxp) + "&";
          //printf("Thread %d: probabilidades[%d]= %f\n",tid,indx->first,probabilidades[indx->first]);
          }
          
       /* double auxp=0;
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
				results += to_string(node) + ":" + doubleToString(auxp) + "&";
				//probabilidades2[indx->first] = (float((1.0-d))/tam ) + d*PR;
		
		}*/
          
      }  

      results += "!"+to_string(error);
      if(convergentes >= listNodosSize){
        results += "/1";
      }else{
        results += "/0";
      }
      results += "_"+to_string(sum);    //suma de las probabilidades calculadas


      cout<<"Error= "<<error<<" suma de probabilidades = "<<sum<<endl;
      

      message m;
      m<<results;
      t.send(m);
    }
          
  }


  return 0;
}



