#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <sstream>
#include <math.h>
#include <WinSock2.h>
using namespace std ;

int nx,ny,sx,sy,res;

struct pointlink{
	int i,j,idx;
	int nup;
	int *upidx;
	string rline;
};

//坐標起點為圖幅左下角，但是j的起算從上面
int get_idx(int i, int j);
int get_i(int cx);
int get_j(int cy);
int get_i_from_idx(int idx);
int get_j_from_idx(int idx);
void traceUpstream(pointlink *pldata, int cx, int cy);
void split(char **arr, char *str, const char *del);

int main(int argc, char **argv){
	fstream FDary;
	fstream UpTrace;
	char input[200];
	//FDary.open("D://python//SBIRDEM//CFDarray.txt",ios::in);
	FDary.open("D://SBIR//FDarray.txt",ios::in);
	int np;
	FDary>>nx>>ny>>sx>>sy>>res;
	np=nx*ny;
	pointlink *pldata = new pointlink[np];

	// Read grid upstream data
	for(int l=0;l<np;l=l+1){
		FDary>>pldata[l].i>>pldata[l].j>>pldata[l].idx>>pldata[l].nup;
		pldata[l].upidx=new int[pldata[l].nup];
		for(int u=0;u<pldata[l].nup;u=u+1)
			FDary>>pldata[l].upidx[u];
	}

	FDary.close();

	// Build Socket Server
	// Reference: http://xyz.cinc.biz/2014/02/c-socket-server-client.html
	int r;
    WSAData wsaData;
    WORD DLLVSERION;
    DLLVSERION = MAKEWORD(2,1);
	r = WSAStartup(DLLVSERION, &wsaData);
    SOCKADDR_IN addr;
    int addrlen = sizeof(addr);
	SOCKET sListen;
	SOCKET sConnect;

	sConnect = socket(AF_INET, SOCK_STREAM, NULL);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_family = AF_INET;
    addr.sin_port = htons(49951);

    sListen = socket(AF_INET, SOCK_STREAM, NULL);
    bind(sListen, (SOCKADDR*)&addr, sizeof(addr));
    listen(sListen, SOMAXCONN);

    SOCKADDR_IN clinetAddr;

	while(true){
		cout << "waiting connection..." << endl;

		if(sConnect = accept(sListen, (SOCKADDR*)&clinetAddr, &addrlen)){
            cout << "a connection was found" << endl;
            printf("server: got connection from %s\n", inet_ntoa(addr.sin_addr));
			
            r = recv(sConnect, input, sizeof(input), 0);
            cout << input << endl;
			char *pos_arr[2];
			const char *del = ",";
			split(pos_arr, input, ",");
			int cx = atoi(pos_arr[0]);
			int cy = atoi(pos_arr[1]);

			cout << "x coordinate:" << cx << endl;
			cout << "y coordinate:" << cy << endl;
			
			traceUpstream(UpTrace, pldata, cx, cy);

			const char *sendback = "Traceback Done";
            send(sConnect, sendback, sizeof(sendback), 0);             
        }
	}

	cout<<"test"<<endl;

	//int cx = 288040;int cy = 2753360;

	// traceUpstream(UpTrace, pldata, cx, cy);

	// cin.get();
	return(0);
}

void split(char **arr, char *str, const char *del) {
  char *s = strtok(str, del);
  
  while(s != NULL) {
    *arr++ = s;
    s = strtok(NULL, del);}
}

void traceUpstream(pointlink *pldata, int cx, int cy){
	UpTrace.open("D://SBIR//UParray.txt",ios::out|ios::trunc);
	int oi,oj,oidx;
	vector <int> idxpool;
	vector <int> idxcchk;
	oi = get_i(cx);
	oj = get_j(cy);
	oidx = get_idx(oi, oj);
	// cout<<oi<<" "<<oj<<" "<<oidx<<endl;
	// cout<<pldata[oidx].idx<<endl;
	idxpool.push_back(pldata[oidx].idx);
	idxcchk.push_back(0);
	cout<<idxpool.max_size()<<endl;
	for(int len=0;len<int(idxpool.size());len=len+1){
		//cout<<len<<" "<<idxpool.size()<<endl;
		if(idxcchk[len]==0){
			for(int u=0;u<pldata[idxpool[len]].nup;u=u+1){
				idxpool.push_back(pldata[idxpool[len]].upidx[u]);
				idxcchk.push_back(0);
			}
			idxcchk[len]=1;
		}
		cout<<len<<" "<<idxpool.size()<<endl;
	}

	int ip, jp, ic, jc;
	for(int len=0;len<int(idxpool.size());len=len+1){
		//cout<<idxcchk[len]<<" "<<idxpool[len]<<endl;
		ip = get_i_from_idx(idxpool[len]);
		jp = get_j_from_idx(idxpool[len]);
		ic = sx+ip*res;
		jc = sy+(ny-jp)*res;
		UpTrace<<ic<<" "<<jc<<" "<<ip<<" "<<jp<<" "<<idxcchk[len]<<" "<<idxpool[len]<<endl;
	}
    UpTrace.close()
}


int get_idx(int i, int j){
	int idx;
	idx=j*nx+i;
	if (i<0||j<0||i>=nx||j>=ny)
		idx=-1;
	return idx;
}


int get_i(int cx){
	int i = (cx-sx)/res;
	return i;
}

int get_j(int cy){
	int j = ny-(cy-sy)/res;
	return j;
}

int get_i_from_idx(int idx){
	int i=0;
	i=idx%nx;
	return(i);
}

int get_j_from_idx(int idx){
	int j=0;
	j=idx/nx;
	return(j);
}
