// Student name : ZanLi
// Student ID : a1750906
// Date : 16/09/2019
// Version : 46

#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <algorithm>

using namespace std;

//Declearing***********************************************************************************************************************************************************************
//Declearing customer structure
struct Customer 
{
	//input
	string name; //Customer's ID
	int arrival; //Customer's arrival time
	int priority; //Customer's priority
	int age; //For queue1, every time you buy a ticket +1, if age=3 then priority+1
	int tickets; //The number of votes the user currently needs
	//output
	int end; //User end time
	int ready; //The first time a user buys a ticket
	int running; //How long has the user been running together?
	int waiting; //How long does the user wait for a long time (only the user who is ready will start calculating the waiting)
	//other
	int wait; //How long does the user wait for a long time (only the user who is ready will start calculating the waiting)
	int kind; //The type of user used for sorting, 0: new user; 1: downgraded user; 2: upgrade user;
	int lastTime; //User status for sorting, if you just bought the ticket lastTime=1
	int arrivalQueue2; //Used to participate in queue2 sorting
};

//Declearing virables
unsigned long i=0;
unsigned long j=0;
unsigned long numberOfCustomer = 0; //Total number of users
int runningTime = 0; //The time required for the user to buy a ticket this time, each time the loop starts to be initialized to 0
int systemTime = 0; //Record system time
int small_time=0; //To calculate the smallest running time 
bool resort = false;//Initialize the resort variable

//Declearing vectors
vector<struct Customer> queue1;
vector<struct Customer> queue2;
vector<struct Customer> finishedQueue; //finishedQueue is used to store users who have already bought tickets.

//Declearing fonctions
vector<Customer> getContent();//get the input.txt
bool queue1comp(const Customer &cus1,const Customer &cus2);//sort the queue1
bool queue2comp(const Customer &cus1,const Customer &cus2);//sort the queue2
void print_finish();//print the result headers
void print_fin(vector<Customer> customer);//print the final result
void printinfo(int systemtime,vector<Customer> cus1,vector<Customer> cus2);//only for debug mode
void print_header();//only for debug mode
void print_customer1(struct Customer customer);//only for debug mode
void print_customer2(struct Customer customer);//only for debug mode
void print_customer3(struct Customer customer);//only for debug mode

//main function********************************************************************************************************************************************************************
int main()
{
	vector<struct Customer> customers = getContent(); //Read input.txt via fstream

	//runningTime = 0; 
	systemTime = 0; //Record system time

	//Traverse the customers and move the eligible users to the appropriate array. Need to view the user's priority
	i=0;
	while(i<customers.size())
	{
		if(customers[i].arrival<=systemTime)
		{
			if(customers[i].priority <= 3) //If the priority is less than or equal to 3, move to the tail of the queue1
				queue1.push_back(customers[i]);
			else //If the priority is greater than 3, move to the tail of the queue2
				queue2.push_back(customers[i]);
			customers.erase(customers.begin()+i);
		}
		else i++;
	}

	//Sort two queues
	sort(queue1.begin(),queue1.end(),queue1comp);
	sort(queue2.begin(),queue2.end(),queue2comp);

	//When the number of people completing the queue is not equal to the total number of people, it means that the user has not bought the ticket. 
	//If the number of people completing the queue is equal to the total number of people, the loop ends.
	while(finishedQueue.size() != numberOfCustomer)
	{

		resort = false;//Initialize the resort variable
		runningTime=0;//The time required for the user to buy a ticket this time, each time the loop starts to be initialized to 0

		if(queue1.size()==0 && queue2.size()==0)
		{
			small_time=1<<20;//To calculate the smallest running time 
			for(int i=0;i<customers.size();i++){
				if(small_time>customers[i].arrival){
					small_time=customers[i].arrival;
				}
			}
			systemTime=small_time;

			i=0;
			while(i<customers.size()){
				if(customers[i].arrival<=systemTime){
					if(customers[i].priority <= 3) //If the priority is less than or equal to 3, move to the tail of the queue1
						queue1.push_back(customers[i]);
					else //If the priority is greater than 3, move to the tail of the queue2
						queue2.push_back(customers[i]);
					customers.erase(customers.begin()+i);
				}
				else i++;
			}
		}

		//handle queue1: find if there is a process in queue1 will runs in the cpu///////////////////////////////////////////////
		for(i=0;i<queue1.size();i++){
			if(queue1[i].arrival <= systemTime){	
				if(queue1[i].ready == -1)
					queue1[i].ready = systemTime;
				break;
			}
		}
		if(i!=queue1.size()){
			if (queue1[i].tickets <= (8-queue1[i].priority)*2){
				runningTime = queue1[i].tickets * 5;
				queue1[i].running += runningTime;
				queue1[i].end = systemTime + runningTime;
				finishedQueue.push_back(queue1[i]);
				queue1.erase(queue1.begin()+i);
			}
			else if (queue1[i].tickets > (8-queue1[i].priority)*2){
				runningTime = (8-queue1[i].priority)*10;
				queue1[i].tickets -= runningTime/5;
				queue1[i].running += runningTime;
				//age, priority, kind
				queue1[i].age += 1;
				queue1[i].waiting -= runningTime;
				if (queue1[i].age == 3) {
					queue1[i].priority += 1;
					queue1[i].age = 0;

					if (queue1[i].priority == 4) {	
						queue1[i].kind = 1;//Degrader
						queue1[i].arrivalQueue2 = systemTime + runningTime;
						customers.push_back(queue1[i]);
						queue1.erase(queue1.begin()+i);
					}
				}
			}
			for(j=0; j<queue1.size(); j++){
				if(queue1[j].ready != -1)
					queue1[j].waiting += runningTime;
			}
			for(j=0; j<queue2.size(); j++){
				if(queue2[j].ready != -1){
					queue2[j].waiting += runningTime;
				}
				if(queue2[j].arrival <= systemTime){	
					queue2[j].wait += 1;

					if(queue2[j].wait == 8){
						queue2[j].wait = 0;
						queue2[j].priority -= 1;

						if(queue2[j].priority == 3){
							queue2[j].kind = 2;//Upgrader
							customers.push_back(queue2[j]);
							queue2.erase(queue2.begin()+j);
						}
					}
				}
			}

			queue1[i].lastTime = 1;
			systemTime += runningTime;

			i=0;
			while(i<customers.size()){
				if(customers[i].arrival<=systemTime){
					if(customers[i].priority <= 3) //If the priority is less than or equal to 3, move to the tail of the queue1
						queue1.push_back(customers[i]);
					else //If the priority is greater than 3, move to the tail of the queue2
						queue2.push_back(customers[i]);
					customers.erase(customers.begin()+i);
				}
				else i++;
			}
			sort(queue1.begin(),queue1.end(),queue1comp);
		}
		else{
			//if there is a downgrader, no process in queue2 will run

			
			//handle queue2: find if there is a process in queue2 will runs in the cpu///////////////////////////////////////////////
			for(i=0; i<queue2.size(); i++)
			{
				if(queue2[i].arrival <= systemTime)
				{//If the conditions of execution are met
					//Ready (only modified once, the time of the first ticket purchase, no longer modified)
					if(queue2[i].ready == -1)
						queue2[i].ready = systemTime;
					break;
				}
			}
			//Running (this parameter is used to determine the length of time that the queue2 user buys the ticket, 
			//which is used to determine if it will be interrupted by the user who just came from queue1)
			if(queue2[i].tickets < 20)
				runningTime = queue2[i].tickets*5;
			else
				runningTime = 100;

			//check if the running process in queue2 would be interrupted
			int oldsystemTime = 0;
			for(j=0; j<customers.size(); j++){
				if(customers[j].arrival > systemTime && customers[j].arrival < systemTime+runningTime && customers[j].priority <= 3){
					resort = true;
					oldsystemTime = systemTime;
					systemTime = customers[j].arrival;//Set the system time to the time the interrupter arrives
					break;
				}
			}
			// if the process is interrupted by downgrader
			if(resort == true){
				runningTime = customers[j].arrival - oldsystemTime;
				queue2[i].tickets -= runningTime/5;
				queue2[i].running += runningTime;

				queue2[i].lastTime = 1;//Mark this is the last person who bought the ticket, used to sort
				queue2[i].arrivalQueue2 = systemTime;//Set the time for this user to go back to queue2 to participate in the sorting

				queue2[i].wait += 1;
				if(queue2[i].wait == 8)//After +1 is exactly equal to 8
				{
					queue2[i].wait = 0;
					queue2[i].priority -= 1;
					if(queue2[i].priority == 3){
						queue2[i].kind = 2;//Upgrader
						queue1.push_back(queue2[i]);//move to queue1
						queue2.erase(queue2.begin()+i);//Deleted in queue2
					}
				}
				sort(queue2.begin(),queue2.end(),queue2comp);
			}
			//If it won’t be interrupted
			else{
				//If tickets are less than 20, then this is the last time the user bought the ticket.
				if(queue2[i].tickets <= 20){
					runningTime = queue2[i].tickets * 5;//The current round of ticket purchase time
					queue2[i].running += runningTime;
					queue2[i].end = systemTime + runningTime;//End time of this user
					finishedQueue.push_back(queue2[i]);//Move to finishedQueue
					queue2.erase(queue2.begin()+i);//Delete this user in queue2
				}
				//If tickets are greater than 20
				else {
					queue2[i].tickets -= 20;//The number of tickets minus 20
					queue2[i].running += 100;//Because each ticket consumes 5 points of systemtime, a total of 100 is consumed.
					runningTime = 100;
					//wait, waiting（By traversing, add the corresponding values to all eligible users. Because this purchaser must also meet the conditions, 
					//in order to avoid duplication, first reduce the corresponding value to this purchaser)
					queue2[i].waiting -= runningTime;//First subtract the corresponding value
					queue2[i].wait -= 1;//The buyer of queue2 buys a ticket, his wait is reduced by 1, because the next member must be +1, 
										//so this purchaser needs -2 to reach everyone +1, but he-1
					queue2[i].lastTime = 1;//Mark this is the last person who bought the ticket, used to sort
					queue2[i].arrivalQueue2 = systemTime+runningTime;//Set the time for this user to go back to queue2 to participate in the sorting
				}

				j=0;
				while(j<queue2.size()){	
					if(queue2[j].ready != -1){//Only people who have bought tickets plus waiting
						queue2[j].waiting += runningTime;
					}
					if(queue2[j].arrival <= systemTime){	
						queue2[j].wait += 1;//As long as the person arrives, add wait
						if(queue2[j].wait == 8) {
							queue2[j].wait = 0;
							queue2[j].priority -= 1;
							if(queue2[j].priority == 3) {
								queue2[j].kind = 2;//Upgrader
								customers.push_back(queue2[j]);
								queue2.erase(queue2.begin()+j);
								j--;
							}
						}
					}
					j++;
				}
				systemTime += runningTime;//Billing current systemTime
				i=0;
				while(i<customers.size()){
					if(customers[i].arrival<systemTime){
						if(customers[i].priority <= 3) //If the priority is less than or equal to 3, move to the tail of the queue1
							queue1.push_back(customers[i]);
						else //If the priority is greater than 3, move to the tail of the queue2
							queue2.push_back(customers[i]);
						customers.erase(customers.begin()+i);
					}
					else i++;
				}
				sort(queue2.begin(),queue2.end(),queue2comp);
			}
		}
		i=0;
		while(i<customers.size()){
			if(customers[i].arrival==systemTime){
				if(customers[i].priority <= 3) //If the priority is less than or equal to 3, move to the tail of the queue1
					queue1.push_back(customers[i]);
				else //If the priority is greater than 3, move to the tail of the queue2
					queue2.push_back(customers[i]);
				customers.erase(customers.begin()+i);
			}
			else i++;
		}
	}
	print_finish();
	print_fin(finishedQueue);

	return 0;
}
//Implmenting functions************************************************************************************************************************************************************
vector<Customer> getContent(){
	fstream file("input.txt",ios::in);
	//The first time I only read how many \n in the file, because there is one row for each user's information, so how many rows means how many users
	char c;
	while(file.get(c)){	
		if(c=='\n')
			numberOfCustomer++;
	}
	vector<Customer> customers(numberOfCustomer);
	file.close();

	//Read the information of each user for the second time, and enter the information into the vector of customers.
	file.open("input.txt",ios::in);
	for(i=0; i<numberOfCustomer; i++){
		file >> customers[i].name >> customers[i].arrival >> customers[i].priority >> customers[i].age >> customers[i].tickets;
		customers[i].end = 0;
		customers[i].ready = -1;//The initial value is -1, because some users may buy 0 for the first time. 
								//Therefore, when the ready is not -1, the user's ready has been changed, and the parameter is no longer needed.
		customers[i].running = 0;
		customers[i].waiting = 0;
		customers[i].wait = 0;
		customers[i].kind = 0;
		customers[i].lastTime = 0;
		customers[i].arrivalQueue2 = customers[i].arrival;//The default time to reach queue2 is the user arrival time, which will change after the operation.
	}
	file.close();

	return customers;
}

void printinfo(int systemtime,vector<Customer> cus1,vector<Customer> cus2){
	cout<<"Time:"<<systemtime<<endl;
	cout<<"name arrival_time tickets_required running priority_number age/runs"<<endl;
	cout<<"queue1"<<endl;
	for(i=0;i<cus1.size();i++)
		print_customer1(cus1[i]);
	cout<<"queue2"<<endl;
	for(i=0;i<cus2.size();i++)
		print_customer2(cus2[i]);
}

//Sort queue1, according to priority, whether you just bought a ticket, arrival time, kind, ID sort
bool queue1comp(const Customer &cus1,const Customer &cus2){
	if(cus1.priority!=cus2.priority)
		return cus1.priority<cus2.priority;
	if(cus1.arrival!=cus2.arrival)
		return cus1.arrival<cus2.arrival;
	if(cus1.running!=cus2.running)
		return cus1.running<cus2.running;
	if(cus1.name!=cus2.name)
		return cus1.name<cus2.name;
	return true;
}

//Sort queue2, according to the time of arrival queue2 (not the arrival time), whether you just bought the ticket, priority, type, ID sort
bool queue2comp(const Customer &cus1,const Customer &cus2){
	if(cus1.arrivalQueue2!=cus2.arrivalQueue2)
		return cus1.arrivalQueue2<cus2.arrivalQueue2;
	if(cus1.name!=cus2.name)
		return cus1.name<cus2.name;
	return true;
}

void print_header(){
	cout<<"Name"<<"\t"<<"ariv"<<"\t"<<"pri"<<"\t"<<"age"<<"\t"<<"tickets"<<endl;
}
void print_customer1(struct Customer customer){
	cout<<customer.name<<"\t"<<customer.arrival<<"\t"<<customer.tickets<<"\t"<<customer.running<<"\t"<<customer.priority<<"\t"<<customer.age<<endl;
}

void print_customer2(struct Customer customer){
	cout<<customer.name<<"\t"<<customer.arrival<<"\t"<<customer.tickets<<"\t"<<customer.running<<"\t"<<customer.priority<<"\t"<<customer.wait<<"\t"<<customer.arrivalQueue2<<endl;
}
void print_customer3(struct Customer customer){
	cout<<customer.name<<"\t"<<customer.arrival<<"\t"<<customer.tickets<<"\t"<<customer.running<<"\t"<<customer.priority<<"\t"<<customer.wait<<"\t"<<customer.arrivalQueue2<<endl;
}
void print_finish(){
	cout<<"name   arrival   end   ready   running   waiting"<<endl;
}
void print_fin(vector<Customer>  customer){
	for(unsigned long i=0;i<customer.size();i++){
		customer[i].waiting=customer[i].end-customer[i].ready-customer[i].running;
		cout<<customer[i].name<<"\t"<<customer[i].arrival<<"\t"<<customer[i].end<<"\t"<<customer[i].ready<<"\t"<<customer[i].running<<"\t"<<customer[i].waiting<<"\t"<<endl;
	}
}