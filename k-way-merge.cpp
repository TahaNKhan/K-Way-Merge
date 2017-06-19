#include <boost/mpi/environment.hpp>
#include <boost/mpi/communicator.hpp>
#include <boost/mpi/collectives.hpp>
#include <boost/serialization/vector.hpp>

namespace mpi = boost::mpi;

using namespace std;

int main()
{
  mpi::environment env;
  mpi::communicator comm;

  if (comm.rank() == 0)
  {
    //generate random numbers
    vector<int> numbers(100);

    for (int i = 0; i < 100; i++)
    {
      numbers[i] = (std::rand() % 100) + 1;
    }
	
	cout << "Initial unsorted array:" << endl;
	
	for (int i = 0; i < 100; i++)
    {
      printf("%4d ",numbers[i]);
      if((i+1)%10 == 0) cout << endl;
    }
	
	
    // total nodes - main node
    int nodes = comm.size() - 1;

    // how much of the array needs to be sent to each node
    int numbers_per_node = numbers.size() / nodes;

    // if there are left over numbers
    int extra = numbers.size() % nodes;

	// Create smaller vectors to send out for each node
    for (int i = 0, j = 0; i < nodes; i++)
    {
      // create a vector to send to each node
      vector<int> to_send(numbers_per_node);
	  
	  // fill up the vector
      for (int k = 0; k < numbers_per_node; k++)
      {
        to_send[k] = numbers[j];
        j++;
      }
	  
	  // if its the last node, add in the extra numbers
      if (i == nodes - 1)
      {
        to_send.resize(numbers_per_node + extra);
        for (int o = 0; o < extra; o++)
        {
          to_send[o + numbers_per_node] = numbers[j];
          j++;
        }
      }
	  // send out the vector
      comm.send(i + 1, 1, to_send);
    }
  }
  // other nodes: get the message and sort locally
  if (comm.rank() != 0)
  {
    vector<int> receive(100);
    // receive the vector
	comm.recv(0, 1, receive);
	// sort locally
    sort(receive.begin(), receive.end());
	// send it out.
    comm.send(0, 1, receive);
  }

  // back to node 0: k way merge
  if (comm.rank() == 0)
  {

    // gathers:  2d array that stores [node number-1] [sorted values]
    vector< vector<int> > gathers(comm.size() - 1);

    // get all the stuff back
    for (int i = 0; i < comm.size() - 1; i++)
    {
      comm.recv(i + 1, 1, gathers[i]);
    }

    // this keeps track of the index for the nodes
    vector<int> keep_track(comm.size() - 1);

    // sorted array with merges will be stored here
    vector<int> result(100);

    // k way merge
    for (int i = 0; i < 100; i++)
    {
      int low = 101;
      int index_of_lowest;
      for (int j = 0; j < comm.size() - 1; j++)
      {
        if (gathers[j][keep_track[j]] < low)
        {
          low = gathers[j][keep_track[j]];
          index_of_lowest = j;
        }
      }
      keep_track[index_of_lowest]++;
      result[i] = low;
    }

    //print result
	
	cout << "Sorted Array: " << endl;
    for (int i = 0; i < 100; i++)
    {
      printf("%4d ",result[i]);
      if((i+1)%10 == 0) cout << endl;
    }
  }

  return 0;
}
