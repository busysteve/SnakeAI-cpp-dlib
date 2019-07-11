#include <thread>
#include <mutex>

#include <vector>
#include <dlib/dnn.h>
#include "snake-pop.h"
#include "snake-ai.h"

int snake_delay = 40;
bool snake_warp = false;
int mutation = 5;
int generation = 0;
int highscore = 0;
bool end_it_all = false;
bool auto_capture_highest = false;


using namespace std;

std::mutex mx;
int snake_ix = 0;

   void Population::handle_snakes()
   {
       unsigned int snake_ix = 0;

       auto work = [&] {

           while(true)
           {

               mx.lock();
               unsigned int ix = snake_ix;
               snake_ix++;
               mx.unlock();

               if( ix >= snakes.size() )
                  return;

               while( !snakes[ix]->dead() && !end_it_all )
                   snakes[ix]->move();           
           }
       };

   
       std::vector<std::thread*> workers;

       for( int i=0; i<m_xx; i++ )
          workers.push_back( new std::thread( work ) );

       for( auto t : workers )
          t->join();

       for( auto t : workers )
          delete t;

       if( end_it_all == true )
       {
          refresh();
          endwin();
          exit(0);
       }
    
   }

   Population::Population(int wx, int wy, int size, int xx, std::string name ) 
        : filename( name )
        , rnd( ::time(NULL) )
        , m_wx( wx ), m_wy( wy )
   {


      m_xx = xx;

      //::srand( ::time(NULL) );

      for( int i=0; i < size; i++ )
          snakes.push_back( new Snake( rnd.get_random_32bit_number()  ) );
    

      bestSnake = snakes[0]->clone();
      bestSnake->init( m_wx, m_wy );
      bestSnake->replay = true;

      initializeSnakes( m_wx, m_wy, m_wx, m_wy );

      if( filename.length() > 0 )
      try {
          deserialize( filename ) >> bestSnake->get_net();

          for( auto s : snakes )
             deserialize( filename ) >> s->get_net();

      } catch(...){};


      auto user_control = [&] {

          std::string file = filename;

          if( file.empty() )
              file = "nameless_snake.net";

          while( true )
          {
              int ch = ::getch();
              
              if( ch == '+' || ch == '=' )
              {
                  ::snake_warp = false;
                  if( --::snake_delay < 1 )
                      ::snake_delay = 0;
              }    
              else if( ch == '-' || ch == '_' )
              {
                  ::snake_warp = false;
                  if( ++::snake_delay > 99 )
                      ::snake_delay = 100;
              }
              else if( ch == 'w' || ch == 'W' )
              {
                  ::snake_warp = true;
              }
              else if( ch == 'e' || ch == 'E' )
              {
                  ::snake_warp = false;
              }
              else if( ch == 'a' )
              {
                  ::auto_capture_highest = false;
              }
              else if( ch == 'A' )
              {
                  ::auto_capture_highest = true;
              }
              else if( ch == 'q' || ch == 'Q' )
              {
                  ::end_it_all = true;
              }
              else if( ch == 'm' )
              {
                  if( --::mutation < 1 )
                      ::mutation = 0;
              }    
              else if( ch == 'M' )
              {
                  if( ++::mutation > 99 )
                      ::mutation = 100;
              }
              else if( ch == 's' )
              {
                  try {
                  	serialize( file ) << bestSnake->get_net();
                  } catch(...) {}
              }
              else if( ch == 'R' )
              {
                  ;//reset_population();
              }


              dlib::sleep( 20 );
          }

      };

      
      thread *t = new std::thread( user_control );
      t->detach();

      

   }
  
   float Population::random( float r )
   {
      return (float)rnd.get_double_in_range( 0.0, r );
   }
 
   bool Population::done() {  //check if all the snakes in the population are dead
      for(unsigned int i = 0; i < snakes.size(); i++) {
         if(!snakes[i]->dead())
           return false;
      }
      if(!bestSnake->dead()) {
         return false; 
      }
      return true;
   }
   
   void Population::update() {  //update all the snakes in the generation
      if(!bestSnake->dead()) {  //if the best snake is not dead update it, this snake is a replay of the best from the past generation
         bestSnake->show();
         //bestSnake->move();
      }
      for(unsigned int i = 0; i < snakes.size(); i++) {
        if(!snakes[i]->dead()) {
           snakes[i]->move(); 
        }
      }
   }
   
   void Population::show() {  //show either the best snake or all the snakes
      bool replayBest = true;
      if(replayBest) {
        bestSnake->show();
      } else {
         for(unsigned int i = 0; i < snakes.size(); i++) {
            snakes[i]->show(); 
         }
      }
   }
   
   void Population::setBestSnake() {  //set the best snake of the generation
       float max = 0;
       int maxIndex = 0;
       for(unsigned int i = 0; i < snakes.size(); i++) {
          if(snakes[i]->fitness() > max) {
             max = snakes[i]->fitness();
             maxIndex = i;
          }
       }
       if(max > bestFitness) {
         bestFitness = max;
         //bestSnake = snakes[maxIndex]->cloneForReplay();
         bestSnake =  snakes[maxIndex]->clone();
         bestSnakeScore = snakes[maxIndex]->score();
         //samebest = 0;
         //mutationRate = defaultMutation;
       } else {
         //bestSnake = bestSnake->cloneForReplay(); 
         Snake *hold = bestSnake->clone();
         delete bestSnake;
         bestSnake = hold; 
         /*
         samebest++;
         if(samebest > 2) {  //if the best snake has remained the same for more than 3 generations, raise the mutation rate
            mutationRate *= 2;
            samebest = 0;
         }*/
       }
       bestSnake->init( m_wx, m_wy );

       if( bestSnakeScore > ::highscore )
           ::highscore = bestSnakeScore;
   }
  
 
   Snake* Population::selectParent() {  //selects a random number in range of the fitnesssum and if a snake falls in that range then select it
      float rand = rnd.get_double_in_range(0.0, (float)fitnessSum);
      float summation = 0;
      for(unsigned int i = 0; i < snakes.size(); i++) {
         summation += snakes[i]->fitness();
         if(summation > rand) {
           return snakes[i];
         }
      }
      return snakes[0];
   }
   
   void Population::naturalSelection() {
      std::vector<Snake*> newSnakes;

      setBestSnake();
      calculateFitnessSum();
      
      newSnakes.push_back( bestSnake->clone() );  //add the best snake of the prior generation into the new generation
      //newSnakes[0]->init( 38, 42 );
      for(unsigned int i = 1; i < snakes.size(); i++) {
         Snake* child = selectParent()->procreate(selectParent());
         //child->mutate();
         newSnakes.push_back( child );
      }

      //cout << endl << snakes.size() << endl;

      //for( int i=0; i<snakes.size(); i++ )
      for( auto p : snakes )
         delete p;

      //cout << endl << snakes.size() << ":" << newSnakes.size() << endl;

      for( unsigned int i=0; i<snakes.size(); i++ )
      {
         snakes[i] =  newSnakes[i];
      }
      //evolution.add(bestSnakeScore);

      //cout << endl << snakes.size() << ":" << newSnakes.size() << endl;

      initializeSnakes( m_wx, m_wy, m_wx, m_wy );

      gen+=1;
      ::generation = gen;
   }
   
   void Population::mutate() {
       for(unsigned int i = 1; i < snakes.size(); i++) {  //start from 1 as to not override the best snake placed in index 0
          ; //snakes[i]->mutate(); 
       }
   }
   
   void Population::calculateFitness() {  //calculate the fitnesses for each snake
      for(unsigned int i = 0; i < snakes.size(); i++) {
         snakes[i]->calculateFitness(); 
      }
   }

   double Population::avg_score()
   {
      double total = 0.0;
      double count = snakes.size();
      for( auto s : snakes )
         total += s->score();

      return total / count;
   }

   
   void Population::calculateFitnessSum() {  //calculate the sum of all the snakes fitnesses
       fitnessSum = 0;
       for(unsigned int i = 0; i < snakes.size(); i++) {
         fitnessSum += snakes[i]->fitness(); 
      }
   }

   void Population::initializeSnakes( int wx, int wy, int x, int y )
   {
      for( auto s : snakes )
         s->init( wx, wy );
         //s->init( wx, wy, ::rand()%x, ::rand()%y );
   }



