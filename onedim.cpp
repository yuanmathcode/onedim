#include<iostream>
#include<set>
#include<vector>
#include<map>
#include<memory>
#include<array>
#include<string>
#include<sstream>
#include<fstream>
#include<cstring>
#include<iomanip>
#include<algorithm>
#include "gurobi_c++.h"
#define M 1000000000000000

using namespace std;
class node;
class edge;

class node{
private:
	int _id;
public:
	node()=default;
	node(int given_id)
	:_id(given_id){}
	const int &id() {return _id;}
	void define_id(int g_id){
		_id=g_id;}
};

class edge{
private:
	int _id;
	shared_ptr<node> _origin;
	shared_ptr<node> _end;
public:
	edge()=default;
	edge(int given_id,shared_ptr<node> given_origin,shared_ptr<node> given_end)
	:_id(given_id),_origin(given_origin),_end(given_end){}
	const int &id() {return _id;}
	shared_ptr<node> &origin() {return _origin;}
	shared_ptr<node> &end() {return _end;}
	void define_id(int g_id){
		_id=g_id;}
	void define_origin(shared_ptr<node> g_origin){
		_origin=g_origin;}
	void define_end(shared_ptr<node> g_end){
		_end=g_end;}
};




class variable{
public:
        int id;
        int type;//0=bool,1=int,2=double
        double upbound;
        double lobound;
        bool bool_value;
        int int_value;
        double double_value;
        set<int> constr_involved;
        map<int,double> constr_coeff;

};

int objcons;
int sum_constr;
map<int,int> constr_sense_map;
map<int,double> constr_rightside_map;
map<int,set<shared_ptr<variable>>> constr_variable_map;
vector<shared_ptr<variable>> vvar;
map<int,shared_ptr<variable>> color_map;
vector<shared_ptr<node>> vecnode;
vector<shared_ptr<edge>> vecedge;
shared_ptr<variable> Colornumber;
void readfile(const char* filename){
//	Graph g;
	ifstream read(filename);
	string str_temp;
	int int_temp,int_temp1,int_temp2;
	double double_temp;
	while (read>> str_temp){
		//read node
		if(str_temp=="Node:"){
			while(read>>str_temp){
				if(str_temp=="end")
					break;
			int_temp=stoi(str_temp);
			shared_ptr<node> node_temp=make_shared<node>(int_temp);
			vecnode.push_back(node_temp);
		//	g.vecnode.push_back(node_temp);
			}
		}
		//Read edge
		if(str_temp=="Edge:"){
			int index_temp=0;
			while(getline(read,str_temp)){
				if(str_temp=="end")
					break;
				if(str_temp.empty())
					continue;
			index_temp++;
			istringstream edge_record(str_temp);
			edge_record>>int_temp1>>int_temp2;
			shared_ptr<node> node_temp1, node_temp2;
		//	for(auto &i:g.vecnode){
			for(auto &i:vecnode){
				if(i->id()==int_temp1)
					node_temp1=i;
			}
		//	for(auto &j:g.vecnode)
			for(auto &j:vecnode){
				if(j->id()==int_temp2)
					node_temp2=j;
			}
			shared_ptr<edge> edge_temp=make_shared<edge>(index_temp,node_temp1,node_temp2);
		//	g.vecedge.push_back(edge_temp);
			vecedge.push_back(edge_temp);
			}
		}
	}
//	for(auto &i:vecedge)
//		cout<<i->end()->id()<<" "<<endl;
        
}
//define color_map is binary variable;
void set_color_map(){
        for(auto &i:vecnode){
		color_map[i->id()]=make_shared<variable>();
		color_map[i->id()]->id=vvar.size();
		color_map[i->id()]->type=1;
		color_map[i->id()]->upbound=25;
		color_map[i->id()]->lobound=1;
		vvar.push_back(color_map[i->id()]);
		
	}
}

//if edge (i,j) exists,then color_map[i]!=color_map[j], color_map[i]-color_map[j]-Mq<=-1,color_map[i]-color_map[j]+M*p>=1;
void set_constraint_group1(){
	for(auto &j:vecedge){
		sum_constr=sum_constr+3;
		shared_ptr<variable> q;
		q=make_shared<variable>();
		q->id=vvar.size();
		q->type=0;
		q->upbound=1;
		q->lobound=0;
		vvar.push_back(q);
		shared_ptr<variable> p;
		p=make_shared<variable>();
		p->id=vvar.size();
		p->type=0;
		p->upbound=1;
		p->lobound=0;
		vvar.push_back(p);

                constr_sense_map[sum_constr-2]=-1;
                constr_rightside_map[sum_constr-2]=-1;
		constr_variable_map[sum_constr-2].insert(color_map[j->origin()->id()]);
		color_map[j->origin()->id()]->constr_involved.insert(sum_constr-2);
		color_map[j->origin()->id()]->constr_coeff[sum_constr-2]=1;	
		constr_variable_map[sum_constr-2].insert(color_map[j->end()->id()]);
		color_map[j->end()->id()]->constr_involved.insert(sum_constr-2);
		color_map[j->end()->id()]->constr_coeff[sum_constr-2]=-1;
		constr_variable_map[sum_constr-2].insert(q);
		q->constr_involved.insert(sum_constr-2);
		q->constr_coeff[sum_constr-2]=-1*M;	
			
		constr_sense_map[sum_constr-1]=1;
		constr_rightside_map[sum_constr-1]=1;
		constr_variable_map[sum_constr-1].insert(color_map[j->origin()->id()]);
		color_map[j->origin()->id()]->constr_involved.insert(sum_constr-1);
		color_map[j->origin()->id()]->constr_coeff[sum_constr-1]=1;	
		constr_variable_map[sum_constr-1].insert(color_map[j->end()->id()]);
		color_map[j->end()->id()]->constr_involved.insert(sum_constr-1);
		color_map[j->end()->id()]->constr_coeff[sum_constr-1]=-1;
		constr_variable_map[sum_constr-1].insert(p);
		p->constr_involved.insert(sum_constr-1);
		p->constr_coeff[sum_constr-1]=M;
		
		constr_sense_map[sum_constr]=0;
		constr_rightside_map[sum_constr]=1;	
		constr_variable_map[sum_constr].insert(p);
		p->constr_involved.insert(sum_constr);
		p->constr_coeff[sum_constr]=1;
		constr_variable_map[sum_constr].insert(q);
		q->constr_involved.insert(sum_constr);
		q->constr_coeff[sum_constr]=1;

	}
}

//Set colornumber constraints: for all i,colornumber >=colormap[i].
void set_constraint_group2(){	
	Colornumber=make_shared<variable>();
	Colornumber->id=vvar.size();
	Colornumber->type=1;
	Colornumber->upbound=25;
	Colornumber->lobound=1;
	vvar.push_back(Colornumber);	
	for(auto &i:vecnode){
        	sum_constr++;
		constr_sense_map[sum_constr]=1;
		constr_rightside_map[sum_constr]=0;
        	constr_variable_map[sum_constr].insert(Colornumber);
        	Colornumber->constr_involved.insert(sum_constr);
       	 	Colornumber->constr_coeff[sum_constr]=1;
		constr_variable_map[sum_constr].insert(color_map[i->id()]);
		color_map[i->id()]->constr_involved.insert(sum_constr);
		color_map[i->id()]->constr_coeff[sum_constr]=-1;
	}
	sum_constr++;
	constr_variable_map[sum_constr].insert(Colornumber);
	Colornumber->constr_involved.insert(sum_constr);
	Colornumber->constr_coeff[sum_constr]=1;
}

void funcGurobi(double time, double absgap, int idisplay)
{
        GRBEnv env = GRBEnv();
        GRBModel model = GRBModel(env);
        model.getEnv().set(GRB_DoubleParam_TimeLimit, 102); // case 900
//      model.getEnv().set(GRB_DoubleParam_MIPGapAbs, 0); // case 0.02
//      model.getEnv().set(GRB_DoubleParam_Heuristics, 1); // case none
//      model.getEnv().set(GRB_DoubleParam_ImproveStartGap, 0.02); // case 0.02
        model.getEnv().set(GRB_IntParam_OutputFlag, idisplay);
        map<shared_ptr<variable>,string> mapvs;
        for(int i=0;i<vvar.size();i++)
        {
                ostringstream convi;
                convi<<i;
                mapvs[vvar[i]]="x"+convi.str();
        }
//      shared_ptr<GRBVar> x(new GRBVar(vvar.size()+1));
        GRBVar *x=new GRBVar [vvar.size()+1];
//      auto x=make_shared<GRBVar>(vvar.size()+1);      
        for(int i=0;i<vvar.size();i++)
        {
                if(vvar[i]->type==0)
                x[i]=model.addVar(vvar[i]->lobound,vvar[i]->upbound,0.0,GRB_BINARY,mapvs[vvar[i]]);
                else if(vvar[i]->type==1)
                x[i]=model.addVar(vvar[i]->lobound,vvar[i]->upbound,0.0,GRB_INTEGER,mapvs[vvar[i]]);
                else if(vvar[i]->type==2)
                x[i]=model.addVar(vvar[i]->lobound,vvar[i]->upbound,0.0,GRB_CONTINUOUS,mapvs[vvar[i]]);
        }
        model.update();
        for(int i=1;i<=sum_constr;i++)
        if(constr_variable_map[i].size()!=0) // cons with 0 var is eliminated
        {
                ostringstream convi;
                convi<<i;
                GRBLinExpr expr;

                if(i!=objcons)
                {
                        for(auto setit:constr_variable_map[i])
                        //for(set<variable*>::iterator setit=constr_variable_map[i].begin();setit!=constr_variable_map[i].end();setit++)
                        {
                                expr+=x[setit->id]*(setit->constr_coeff[i]);
                                //expr+=x[(*setit)->index]*((*setit)->constr_coeff[i]);
                        }
                        string name='c'+convi.str();
                        if(constr_sense_map[i]==1)
                                model.addConstr(expr,GRB_GREATER_EQUAL,constr_rightside_map[i],name);
                        else if(constr_sense_map[i]==-1)
                                model.addConstr(expr,GRB_LESS_EQUAL,constr_rightside_map[i],name);
                        else if(constr_sense_map[i]==0)
                                model.addConstr(expr,GRB_EQUAL,constr_rightside_map[i],name);
                }
                else
                {
                        for(auto setit:constr_variable_map[i]){
//                        for(set<variable*>::iterator setit=constr_variable_map[i].begin();setit!=constr_variable_map[i].end();setit++)
                                expr+=x[setit->id]*(setit->constr_coeff[i]);
                //      model.setObjective(expr,GRB_MAXIMIZE);
                                model.setObjective(expr,GRB_MINIMIZE);
                }
        }
//      mycallback cb(time, absgap);
//      model.setCallback(&cb);
        model.optimize();

        while(model.get(GRB_IntAttr_SolCount)==0)
        {
                time+=5;
                model.getEnv().set(GRB_DoubleParam_TimeLimit, time); // case 900
                model.optimize();
        }
        for(int i=0;i<vvar.size();i++)
        {
                if(vvar[i]->type==0||vvar[i]->type==1)
                {
                        if(x[i].get(GRB_DoubleAttr_X)-(int)x[i].get(GRB_DoubleAttr_X)<0.5)
                                vvar[i]->int_value=(int)x[i].get(GRB_DoubleAttr_X);
                        else
                                vvar[i]->int_value=(int)x[i].get(GRB_DoubleAttr_X)+1;
                }
                else if(vvar[i]->type==2)
                        vvar[i]->double_value=x[i].get(GRB_DoubleAttr_X);
                else
                        cout<<"new type"<<endl;
        }
}}




int main(int argc, char * argv[]){
	readfile("input.txt");
//	cout<<vecedge.size()<<" "<<endl;
	set_color_map();
	set_constraint_group1();
	set_constraint_group2();
	objcons=sum_constr;
	funcGurobi(30,0,1);
}
