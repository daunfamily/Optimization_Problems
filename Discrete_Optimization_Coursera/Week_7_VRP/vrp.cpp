#include <bits/stdc++.h>
#include <algorithm>

using namespace std;
#ifdef DEBUG
    #define LOG(x) std::cerr << x << std::endl
	#define LOG_S(x) std::cerr << x
#else
    #define LOG(x)
	#define LOG_S(x)
#endif
#define PI 3.14159265

typedef struct Customer
{
	int demand;
	int left;
	float x;
	float y;
	int id;
	double angle; // in radians , from depot
	double coneAngle;// cone angle formed by bisection of ith and i+1 customer.
}C;
typedef struct VehicleCone
{
	double start;
	double end;
	vector<int> customer;
}VC;
typedef struct Cluster
{
	int capacity;
	float cost ;// tour cost
	vector<int> customer; //list of customer id
	vector<int> tspTourOrder;
}Cluster;
int n;// # of customer
int v, v_orignal;// # of vehicle
int q;// capacity of each vehicle
float **adjMatrix; // distance matrix
float **custToSeed;
float **seedToCust;
vector<Cluster> cluster; // cluster for each vehicle which contains which customer each vehicle will visit.
vector<VC> vc;
float total_demand;
float total_capacity;
float perV;
vector<pair<float, float> > seedPoint;
vector<C> c;
bool sortbyAngle(C a, C b)
{
	return a.angle < b.angle;
}
bool sortbyAngleWithID(C a, C b)
{
	return c[a.id].angle < c[b.id].angle;
}
bool sortbyDemand(C a, C b)
{
	return a.demand > b.demand;
}
bool sortbyID(C a, C b)
{
	return a.id < b.id;
}
void calculateSeedDistance();
void assignCustomerToVehicle();
#define next(i) ((i==n-1)?1:i+1)
#define prev(i) ((i==1)?n-1:i-1)
int main(int argc, char* argv[])
{
	/*
		http://www.bernabe.dorronsoro.es/vrp/index.html?/algorithms/ClustRout.html
		Cluster First, Route Second.
		Sweep Algorithm to find cluster
	*/
	ifstream cin(argv[1]);
	
	
	cin >> n;
	cin >> v;
	cin >> q;
	total_capacity = q * v;
	vc.reserve(v);
	cluster.reserve(v);

	for (int i=0; i<n ;i++)
	{
		C t;
		cin >> t.demand;
		cin >> t.x;
		cin >> t.y;
		t.id = i;
		if(i!=0)
		{
			t.angle = atan2 (t.x - c[0].x, t.y-c[0].y);
			if(t.angle<0)
				t.angle += (2*PI);
		}
		t.left = t.demand;
		total_demand += t.demand;
		c.push_back(t);
	}
	v_orignal = v;
	#ifdef USE_MIN
		perV = ceil((total_demand / total_capacity )*q);
		v = ceil(total_demand/q); // If we have to use just enough vehicle to service all customer
	#else
		perV = ((total_demand / total_capacity )*q);
	#endif
	
	for (int i=0; i<v; i++)
	{
		#ifdef USE_MIN
			cluster[i].capacity = perV;//Use all
		#else
			cluster[i].capacity = q;
		#endif
		
		cluster[i].customer.reserve(v);
	}
	
	LOG("Total Demand= "<< total_demand<<" Total Capacity "<< total_capacity<<" per Vehicle= "<<perV<<" Minimum Vehicle="<<v);
	// Adjacency matrix between customers & depot.
	adjMatrix = new float* [n];
	for (int i=0; i<n; i++)
		adjMatrix[i] = new float[n];
	for (int i=0; i<n; i++)
	{
		
		adjMatrix[i][i]=0;
		for (int j=i+1; j<n; j++)
			adjMatrix [i][j] = adjMatrix [j][i] = sqrt(pow(abs(c[i].x-c[j].x), 2) + pow(abs(c[i].y-c[j].y), 2));
	}
	// Find the angle between customer and depot.
	sort(c.begin()+1, c.end(), sortbyAngle); // c[0] is depot so dont include that in sorting
	
	for (int i=1; i<n; i++)
	{
		/*if((c[next(i)].angle-c[i].angle)==0)
			c[i].coneAngle = c[i].angle + (0.5);
		else */if((c[next(i)].angle-c[i].angle)<0)
			c[i].coneAngle = c[i].angle + ((2*PI) - c[i].angle)/2;
		else
			c[i].coneAngle = c[i].angle + (c[next(i)].angle-c[i].angle)/2;
	}
	LOG("ID   Demand x y angle coneAngle"); 
	for (int i=1; i<n; i++)
		LOG("i= "<<c[i].id<<" "<<c[i].demand<<" x="<<c[i].x<<" y="<<c[i].y<<" "<<c[i].angle* 180 / PI<<" "<<c[i].coneAngle* 180 / PI);
	
	// ith customer cone is defined as  coneAngle of prev(i) & i
	// Calculate seed points
	int prev =-1;
	int index=1;
	double start = c[n-1].coneAngle;
	vector<double> seedAngle;
	for (int i=0; i < v; i++)
	{
		int sum =0, j;
		if(c[j].left)
			j = index;
		else
			j = next(j);
		for (; c[j].left && (sum+c[j].left)<= perV; j = next(j))
		{
			sum += c[j].left;
			c[j].left =0;
			vc[i].customer.push_back(j);
		}
		double end;
		if(perV-sum)
		{
			end = c[prev(j)].coneAngle + (c[j].coneAngle - c[prev(j)].coneAngle)*(perV-sum)/c[j].demand;
			c[j].left -= c[j].left*(perV-sum)/c[j].demand;
		}	
		else
			end = c[prev(j)].coneAngle;
		
		if((j !=1 ) && (end > c[j].coneAngle))
			vc[i].customer.push_back(j);
		LOG_S("[vehicle] cone angle start ="<<start*180/PI<<" end = " << end*180/PI<<" size= " << (end-start)*180/PI);
		for (auto &k:vc[i].customer)
			LOG_S(" "<< k);
		LOG_S(endl);
		#if 1
		if((end-start)<0) //cross over
			seedAngle.push_back((start + (2*PI + (end-start))/2)-2*PI);
		else
			seedAngle.push_back(start + (end-start)/2);	
		#endif
		vc[i].start = start;
		vc[i].end = end;
		
		start =end;
		index =j;
	}
	for (auto & i: seedAngle)
		LOG("seedAngle "<< i*180/PI);
	/*
	   Customer angle = angle from y-axis, can be calculated as atan2 	
	   Create customer cone : i & i-1 customer angle.
	1. Each vehicle to serve total_demand/total_capacity percentage load, so based on that vehicle cone, 
	   sum of customer cone and some fraction of last customer.
	2. Construct vehicle cone, bisect the vehicle cone and get the seedAngle
	3. Arc so that 75% of vehicle load is covered and that will give seedDistance.
	*/
	// Step 3:
	
	for (int i = 0; i<v ; i++)
	{
		float capacity = (perV*3.0)/4.0;
		float sum = 0;
		float distance = 0;
		for (int j =0; (j< vc[i].customer.size()) && (sum < capacity); j++)
		{
			sum += c[vc[i].customer[j]].demand;
			if(adjMatrix[0][c[vc[i].customer[j]].id] > distance)// store fartherest customer to create arc.
			{
				distance = adjMatrix[0][c[vc[i].customer[j]].id];
			}
		}
		float seedAngle;
		if((vc[i].end-vc[i].start)<0) //cross over
			seedAngle = ((vc[i].start + (2*PI + (vc[i].end-vc[i].start))/2)-2*PI);
		else
			seedAngle = (vc[i].start + (vc[i].end-vc[i].start)/2);	
		seedPoint.push_back(make_pair(c[0].x+(distance * sin(seedAngle)), c[0].y+(distance*cos(seedAngle))));
	}
	for (int i=0; i<v; i++)
		LOG("seedPoint x="<<seedPoint[i].first<<" y="<<seedPoint[i].second);
	calculateSeedDistance();
	assignCustomerToVehicle();
	
}
typedef struct 
{
	float cost;
	vector<int> city;
}TSP;
// Solve TSP for a given list of city 
TSP solveTSP(vector<int> customer)
{
		TSP t;
		ofstream fout;
		fout.open("in.txt"); 
		fout << customer.size()+1<<endl;
		fout << c[0].x<<" "<<c[0].y<<endl;
				
		for (auto &j: customer)
			fout <<c[j].x<<" " << c[j].y<<endl;
		
		system("./tsp in.txt > out.txt"); // execute TSP solver
		
		ifstream fin;
		fin.open("out.txt");
		int opt;
		fin >> t.cost >> opt;
		int temp;
		for (int i =0 ; i< customer.size()+1; i++)
		{
			fin >> temp;
			t.city.push_back(temp);
		}
		return t;;
}
// Solve TSP for a given cluster and store the result in cluster structure starting with 0(depot)
void evalCluster(int i) // cluster id
{

		
		vector<int> mapping; // help in converting tsp id to vrp id mapping
		mapping.push_back(0);
		int total_load=0;
		for (auto &j: cluster[i].customer)
		{
			LOG_S(j<<" ");
			mapping.push_back(j);
			total_load += c[j].demand;
		}
		LOG_S(" demand: "<<total_load);
		LOG_S(endl);
			
		TSP tsp = solveTSP(cluster[i].customer);
		cluster[i].cost = tsp.cost;	
		LOG("TSP cost is "<<tsp.cost);
		int rotate_index; //used to rotate around depot since all tour should start from depot
		// Map tsp id to real customer id
		for (int j=0; j<cluster[i].customer.size()+1; j++)
		{
			if(tsp.city[j]==0)
				rotate_index = j;
			cluster[i].tspTourOrder.push_back(mapping[tsp.city[j]]);
			LOG_S(tsp.city[j]<<"->"<< mapping[tsp.city[j]]<<" ");
		}
		LOG_S(endl);
		LOG("done "<<cluster[i].tspTourOrder.size()<<" "<<rotate_index);
		// tour should always start with 0 i.e. depot and also end with 0 (explicit)
		rotate(cluster[i].tspTourOrder.begin(), cluster[i].tspTourOrder.begin()+rotate_index, cluster[i].tspTourOrder.end() );
		cluster[i].tspTourOrder.push_back(0);
}
//calculate adjMatrix for seedpoint;
void calculateSeedDistance()
{
	sort(c.begin()+1, c.end(), sortbyID);
	custToSeed = new float* [n];
	seedToCust = new float* [v];
	for (int i=0; i<n; i++)
		custToSeed[i] = new float[v];
	for (int i=0; i<v; i++)
		seedToCust[i] = new float[n];
		
	for (int i=0; i<n; i++)
		for(int j=0; j<v ;j++)
			custToSeed [i][j] = seedToCust[j][i] = sqrt(pow(abs(seedPoint[j].first-c[i].x), 2) + pow(abs(seedPoint[j].second-c[i].y), 2));
}
// This function assign each customer to vehicle by checking from which seed point the customer is near.
void assignCustomerToVehicle()
{
	sort(c.begin()+1, c.end(), sortbyDemand);
	vector<int> left;
	for (int i =1; i<n ; i++)
	{
		float minFound = numeric_limits<float>::infinity();
		int assign=-1;
		int j;
		for (j=0; j<v; j++)
		{
			//if vehicle capacity is larger than customer i demand 
			if(cluster[j].capacity < c[i].demand)
				continue;
			float cost = min(adjMatrix[0][c[i].id]+custToSeed[c[i].id][j]+seedToCust[j][0], custToSeed[0][j]+seedToCust[j][c[i].id]+adjMatrix[c[i].id][0]) - (custToSeed[0][j] + seedToCust[j][0]);
			if(cost < minFound)
			{
				minFound = cost;
				assign = j; // customer assign to jth vehicle
			}
		}
		if(assign==-1)
		{
			LOG("No cluster can be assigned "<<c[i].id << " "<<c[i].demand<<" " <<c[c[i].id].x<<" "<< c[c[i].id].y);///;
			left.push_back(c[i].id);
		}
		else
		{
			
			cluster[assign].customer.push_back(c[i].id);
			// Decrease capacity of this vehicle;
			cluster[assign].capacity -= c[i].demand;
			LOG("customer "<< c[i].id<<" cluster "<<assign<< " capacity left "<<cluster[assign].capacity);
		}
	}
	sort(c.begin()+1, c.end(), sortbyID);
	if(left.size() && (q-perV))
	{
		//in crease capacity
		for (int j=0; j<v; j++)
			cluster[j].capacity+=(q-perV);
		for (auto &i:left)
		{
			float minFound = numeric_limits<float>::infinity();
			int assign=-1;
			int j;
			for (j=0; j<v; j++)
			{
				//if vehicle capacity is larger than customer i demand 
				if(cluster[j].capacity < c[i].demand)
					continue;
				float cost = min(adjMatrix[0][i]+custToSeed[i][j]+seedToCust[j][0], custToSeed[0][j]+seedToCust[j][i]+adjMatrix[i][0]) - (custToSeed[0][j] + seedToCust[j][0]);
				if(cost < minFound)
				{
					minFound = cost;
					assign = j; // customer assign to jth vehicle
				}
			}
			if(assign==-1)
			{
				LOG("No cluster can be assigned "<<i << " "<<c[i].demand<<" " <<c[i].x<<" "<< c[i].y);
			}
			else
			{
				
				cluster[assign].customer.push_back(i);
				// Decrease capacity of this vehicle;
				cluster[assign].capacity -= c[i].demand;
				LOG("customer "<< i<<" cluster "<<assign<< " capacity left "<<cluster[assign].capacity);
			}
		}
	}
	
	// Solve TSP for each cluster
	float total_cost=0;
	for (int i =0; i<v;i++)
	{
		LOG("start of cluster "<<i<<endl);
		evalCluster(i);
		total_cost += cluster[i].cost;// sum up the cluster tour cost
	}
	#if 0
	LOG("start local search");
	// Start local search exchange customer and see if that can result in shorter cost overall.
	for (int i=0; i<v ; i++)
	{
		// sort the cluster by its angle
		sort(cluster[i].customer.begin(), cluster[i].customer.end(), sortbyAngleWithID);
		int n = next(i);
		sort(cluster[n].customer.begin(), cluster[n].customer.end(), sortbyAngleWithID);
		//Try swap current cluster last with next cluster first on basis of demand/capacity fulfillment
		int current = cluster[i].left + cluster[i].customer[cluster[i].customer.size()-1];
		int next = cluster[n].left + cluster[n].customer[0];
		if((cluster[n].customer[0] < current) && (cluster[i].customer[cluster[i].customer.size()-1] < next))
		{
			//swapping possible
			LOG("swapping possible on "<< i <<" "<< n <<" cluster");
		}
		
		
	}
	#endif
	cout << total_cost<<" 0"<<endl;
	for (int i = 0; i<v; i++)
	{
		for(auto &j:cluster[i].tspTourOrder)
			cout <<j<<" ";
		cout <<endl;
	}
	for (int i = v; i<v_orignal; i++)
		cout <<"0 0"<<endl;
	
}		
