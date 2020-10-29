#include<iostream>
#include<cstdio>
#include<cstdlib>
#include<sstream>
#include<fstream>
#include<vector>
#include<cstring>
#include<ctime>
#include<iomanip>
#include<pthread.h>
#include<mutex>
#define n_thread 8
using namespace std;
class Node{
	public:
		int item;
		int count;
		Node *parent;
		vector <Node*> child;
		Node *next;
};
typedef struct NODE{
	int item;
	int freq;
	vector <Node*> next;
}headerNode;
typedef struct out{
	vector <int> set;
	int count;
}outNode;
typedef struct pass{
    int max;
    vector <headerNode> *headerTable;
    int id;
}passNode;
float min_sup = 0.1;
int num_trans;
vector <outNode> output;
vector <vector <outNode> > order_out;
outNode *BSThead;
mutex m;
double total=0;
int findHeader(int target, vector <headerNode> &headerTable);
void countItem(int item_count[1000], vector <vector <int> > &transactions);
void constructOrdered(vector <vector <int> > &ordered,vector <vector <int> > &transactions,int item_count[1000],int &item_max);
void constructHeaderTable(vector <headerNode> &headerTable,int item_count[1000],int item_max);
void constructFPtree(vector <vector <int> > &ordered,vector <headerNode> &headerTable, Node* &head);
void conditionalFPtree(int target,vector <headerNode> &headerTable, Node* &newHead,vector <headerNode> &headerTable_new);
void FPGrowth(char pattern[300], Node* &head, vector <headerNode> &headerTable,int n);
int compareResult(vector <int> &v1, vector <int> &v2);
float round(int a);
void *fun(void *id){
    int i;
    int value=*(int*)id;
    int seg=output.size()/n_thread;
    int start=value*seg;
    int stop=(value+1)*seg-1;
    if(value==n_thread-1)
        stop=output.size()-1;
    //binary search
    for(i=start;i<=stop;i++){
        int first=0;
        int last=order_out[value].size()-1;
        int mid;
        while(first<=last){
            mid=first+(last-first)/2;
            if(compareResult(order_out[value][mid].set,output[i].set)==1){
                last=mid-1;
            }
            else if(compareResult(order_out[value][mid].set,output[i].set)==-1){
                first=mid+1;
            }
        }
        order_out[value].insert(order_out[value].begin()+first,output[i]);
    }
    //cout << value << endl;
    pthread_exit(NULL);
}
void *FP(void *node){
    int i;
    passNode *p=(passNode*) node;
    int seg=(p->max)/n_thread;
    int start=(p->id)*seg;
    int stop=((p->id)+1)*seg-1;
    if(p->id==n_thread-1)
        stop=p->max;
    for(i=start;i<=stop;i++){
		Node *nnn;
		vector <headerNode> newHeaderTable;
		int pos=findHeader(i,*(p->headerTable));
		int index=(*(p->headerTable))[pos].item;
		char num[5];
		sprintf(num,"%d",index);
		strcat(num,",");
		conditionalFPtree(index,*(p->headerTable),nnn,newHeaderTable);
		FPGrowth(num,nnn,newHeaderTable,p->id);

	}
	pthread_exit(NULL);
}
int main(int argc,char *argv[])
{
	int item, index;
	int item_max=0;
	int i, j, k;
	int item_count[1000];
	min_sup=atof(argv[1]);
	freopen(argv[2],"r",stdin);
	vector <vector <int> > transactions;
	vector <headerNode> headerTable;
	vector <vector <int> > ordered;
	headerNode *headerNodePtr;
	string line;
	//file input
	while(!getline(cin,line).eof()){
		num_trans++;
		vector<int> arr;
		istringstream ssline(line);
		string number;
		while(getline(ssline,number,',')){
            arr.push_back(atoi(number.c_str()));
		}
		if(arr.size()!=0)
            transactions.push_back(arr);
	}
	num_trans=transactions.size();
	//count the # of each elements
	countItem(item_count,transactions);
	//construct ordered transactions
	constructOrdered(ordered,transactions,item_count,item_max);
	//construct headerTable
	constructHeaderTable(headerTable,item_count,item_max);
	//construct FP-tree
	Node *head;
	Node *current;
	constructFPtree(ordered,headerTable,head);

	//multi-thread
	for(i=0;i<n_thread;i++){
        vector <outNode> b;
        order_out.push_back(b);
	}
	pthread_t pid[8];
	int id[n_thread];
	void *ret;
	//FPtree
	for(i=0;i<n_thread;i++){
        passNode *p=new passNode;
        p->id=i;
        p->max=item_max;
        p->headerTable=&headerTable;
        id[i]=i;
        pthread_create(&pid[i],NULL,FP,p);
	}
	for(i=0;i<n_thread;i++){
        pthread_join(pid[i],&ret);
	}
	//sort
	for(i=0;i<n_thread;i++){
        id[i]=i;
        pthread_create(&pid[i],NULL,fun,&id[i]);
	}
	for(i=0;i<n_thread;i++){
        pthread_join(pid[i],&ret);
	}
	fstream fp;
	fp.open(argv[3],ios::out);
	for(i=0;i<=item_max;i++){
		if(round(item_count[i])>=min_sup){
			int pos=findHeader(i,headerTable);
			fp << headerTable[pos].item << ":" << fixed << setprecision(4) << round(headerTable[pos].freq) << endl;
			fp.unsetf(ios::fixed);
		}
	}
	int counter[n_thread];
	bool emp[n_thread];
	for(i=0;i<n_thread;i++){
        counter[i]=0;
        emp[i]=false;
	}
	while(true){
        int min;
        bool flag=true;
        for(i=0;i<n_thread;i++){
            if(counter[i]>=order_out[i].size()){
                emp[i]=true;
            }
            flag=flag&&emp[i];
        }
        for(i=0;i<n_thread;i++){
            if(!emp[i]){
                min=i;
                break;
            }
        }
        if(flag) break;
        for(i=1;i<n_thread;i++){
            if(emp[i]) continue;
            if(compareResult(order_out[min][counter[min]].set,order_out[i][counter[i]].set)==1)
                min=i;
        }
        for(j=0;j<order_out[min][counter[min]].set.size();j++){
            if(j!=order_out[min][counter[min]].set.size()-1)
                fp << order_out[min][counter[min]].set[j] << ",";
            else
                fp << order_out[min][counter[min]].set[j] << ":";
		}
		fp << fixed << setprecision(4) << round(order_out[min][counter[min]].count) << endl;
		counter[min]++;
		fp.unsetf(ios::fixed);
	}
	fp.close();
	return 0;
}
int findHeader(int target, vector <headerNode> &headerTable){
	int i;
	for(i=0;i<headerTable.size();i++){
		if(headerTable[i].item==target)
			return i;
	}
	return -1;
}
void countItem(int item_count[1000], vector <vector <int> > &transactions){
	int i, j;
	for(i=0;i<1000;i++)
		item_count[i]=0;
	for(i=0;i<transactions.size();i++){
		for(j=0;j<transactions[i].size();j++){
			item_count[transactions[i][j]]++;
		}
	}
}
void constructOrdered(vector <vector <int> > &ordered,vector <vector <int> > &transactions,int item_count[1000],int &item_max){
	int i, j, item, index;
	for(i=0;i<transactions.size();i++){
		vector<int> arr;
		for(j=0;j<transactions[i].size();j++){
			item=transactions[i][j];
			if(((float)item_count[item]/num_trans)>=min_sup){
				index=0;
				if(arr.empty())
					arr.push_back(item);
				else{
					while(index<arr.size()){
						if(item_count[arr[index]]>item_count[item])
							index++;
						else
							break;
					}
					arr.insert(arr.begin()+index,item);
				}
				if(item>item_max)
					item_max=item;
			}
		}
		if(!arr.empty()){
			ordered.push_back(arr);
		}
	}
}
void constructHeaderTable(vector <headerNode> &headerTable,int item_count[1000],int item_max){
	int i, index;
	headerNode *headerNodePtr;
	for(i=0;i<=item_max;i++){
		if(item_count[i]>0){
			index=0;
			headerNodePtr = new headerNode;
			headerNodePtr->item=i;
			headerNodePtr->freq=item_count[i];
			if(headerTable.empty())
				headerTable.push_back(*headerNodePtr);
			else{
				while(index<headerTable.size()){
					if(item_count[headerTable[index].item]>item_count[i])
						index++;
					else
						break;
				}
				headerTable.insert(headerTable.begin()+index,*headerNodePtr);
			}
		}
	}
}
void constructFPtree(vector <vector <int> > &ordered, vector <headerNode> &headerTable, Node* &head){
	int i, j, k, index;
	head=new Node;
	head->item=-1;
	//construct FP-tree
	int c=0;
	Node *current;
	Node *ptr;
	for(i=0;i<ordered.size();i++){
		current=head;
		for(j=0;j<ordered[i].size();j++){
			bool find=false;
			for(k=0;k<current->child.size();k++){
				if((current->child)[k]->item==ordered[i][j]){
					find=true;
					(current->child)[k]->count++;
					current = (current->child)[k];

					break;
				}
			}
			if(!find){
				ptr = new Node;
				ptr->item=ordered[i][j];
				ptr->count=1;
				ptr->parent=current;
				ptr->next=NULL;
				(current->child).push_back(ptr);
				index=findHeader(ordered[i][j],headerTable);

				headerTable[index].next.push_back(ptr);  //index==-1

				current=ptr;
				c++;
			}
		}
	}
}
void conditionalFPtree(int target,vector <headerNode> &headerTable, Node* &newHead,vector <headerNode> &headerTable_new){
	int item_count[1000];
	vector <vector <int> > cond;
	vector <vector <int> > order_cond;
	int item_max=0, i, j;
	//create new transactions list
	int counter=0;
	Node *current;
	int index=findHeader(target,headerTable);
	for(i=0;i<headerTable[index].next.size();i++){
		vector<int> arr;
		current=headerTable[index].next[i];
		counter=current->count;
		while(current->parent->item!=-1){
			arr.insert(arr.begin(),current->parent->item);
			current=current->parent;
		}
		while((counter--)>0){
			cond.push_back(arr);
		}
		counter=0;
	}
	countItem(item_count,cond);
	constructOrdered(order_cond,cond,item_count,item_max);
	constructHeaderTable(headerTable_new,item_count,item_max);
	constructFPtree(order_cond,headerTable_new,newHead);
}
void FPGrowth(char pattern[300], Node* &head, vector <headerNode> &headerTable, int n){
	int i, j, index, parentItem, pos;
	Node *current;
	char p[300];
	outNode *ptr;
	int start_i, end_i;
	for(i=0;i<headerTable.size();i++){
		parentItem=headerTable[i].item;
		vector <headerNode> newHeaderTable;
		Node *newHead;
		conditionalFPtree(parentItem,headerTable,newHead,newHeaderTable);
		char num[4];
		sprintf(num,"%d",parentItem);
		strcpy(p,pattern);
		strcat(p,num);
		strcat(p,",");

		int add=0;
		index=findHeader(parentItem,headerTable);

		for(j=0;j<headerTable[index].next.size();j++){
			add+=headerTable[index].next[j]->count;
		}
		if(round(add)<min_sup){
            continue;
		}
		bool find=false;
		ptr=new outNode;

		char *pch;
		char t[300];
		strcpy(t,p);
		pch = strtok (t,",");

		while (pch != NULL){
		    pos=0;
		    if(!ptr->set.empty()){
		    	while(pos<ptr->set.size() && ptr->set[pos]<atoi(pch)){
			    	pos++;
				}
			}
			ptr->set.insert(ptr->set.begin()+pos,atoi(pch));
		    pch = strtok (NULL, ",");
		}
		ptr->count=add;
		m.lock();
		output.push_back(*ptr);
		m.unlock();
        FPGrowth(p,newHead,newHeaderTable,n);
	}
}
int compareResult(vector <int> &v1, vector <int> &v2){
	int j=0;
	if(v2.size()>v1.size())  //v2 larger
		return -1;
	else if(v2.size()<v1.size())  //v1 larger
		return 1;
	while(j<v1.size() && j<v2.size()){
		if(v1[j]>v2[j])
			return 1;
		else if(v1[j]<v2[j])
			return -1;
		j++;
	}
	return 1 ;
}
float round(int x){
	int i, index;
	int a[8];
	float b=(float)x/num_trans;
	int p=1, int_result=0;
	float result;
	for(i=0;i<7;i++){
        index=b*10;
        a[i]=index;
        b*=10;
        b-=index;
	}
	if(a[4]==4 && a[5]==9 && a[6]==9)
        a[3]++;
	if(a[4]>=5){
        a[3]++;
	}
	for(i=3;i>=0;i--){
        int_result+=a[i]*p;
        p*=10;
	}
	result=(float)int_result/10000;
	return result;
}

