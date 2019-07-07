#include <vector>
#include "snake-ai.h"
#include <dlib/dnn.h>

using namespace std;

class Population {
public:
   dlib::rand rnd;

   std::string filename;
  
   std::vector<Snake*> snakes;
   Snake *bestSnake;
   
   int bestSnakeScore = 0;
   int gen = 0;
   int samebest = 0;
  
   int m_xx;
   int m_wx, m_wy;
 
   float bestFitness = 0;
   float fitnessSum = 0;
   
   Population(int wx, int wy, int size, int threads, std::string file );
   float random( float r );
   int random( int r );
   bool done(); //check if all the snakes in the population are dead
   void update(); //update all the snakes in the generation
   void show();  //show either the best snake or all the snakes
   void setBestSnake();  //set the best snake of the generation
   Snake* selectParent(); //selects a random number in range of the fitnesssum and if a snake falls in that range then select it
   void naturalSelection();
   void mutate();
   void calculateFitness();  //calculate the fitnesses for each snake
   void calculateFitnessSum();  //calculate the sum of all the snakes fitnesses
   double avg_score();
   void initializeSnakes( int wx, int wy, int x=-1, int y=-1 );
   void handle_snakes();
   int bestScore() { return bestSnakeScore; }
};


