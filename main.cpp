// The contents of this file are in the public domain. See LICENSE_FOR_EXAMPLE_PROGRAMS.txt
/*

    This is an example illustrating the use of the gui api from the dlib C++ Library.


    This is a pretty simple example.  It makes a window with a user
    defined widget (a draggable colored box) and a button.  You can drag the
    box around or click the button which increments a counter. 
*/




#include <dlib/gui_widgets.h>
#include <sstream>
#include <string>


#include "snake-pop.h"
#include "snake-ai.h"

extern int generation;
extern int highscore;
extern bool auto_capture_highest;
extern bool snake_warp;
double last_avg_score = 0.0;
extern std::string train_out_filename;
extern ofstream train_file;
extern bool end_it_all;
extern std::mutex train_out_mutex;
int    max_gens = 0;

using namespace std;
using namespace dlib;

//  ----------------------------------------------------------------------------

//  ----------------------------------------------------------------------------

int main( int argc, char** argv )
{

    if( argc < 5 )
    {
        printf("%s [width] [height] [population] [threads]\n ", argv[0] );
        exit(0);
    }

    ::initscr();
    ::cbreak();
    ::noecho();
    ::curs_set(0);
    ::clear();

 
    int wx = atoi( argv[1] );
    int wy = atoi( argv[2] );
    int population = atoi( argv[3] );
    int threads = atoi( argv[4] );
    std::string file, training_file;

    if( argc > 5 )
    {
        if( atoi( argv[5] ) == 1 )
        {
            if( argc > 7 )
            {
               training_file = argv[7];
            }
            

            ::refresh();
            ::endwin();
 
            Snake snake( ::time(NULL) );
            snake.init( wx, wy );

            snake.train( training_file );

/*
            ::initscr();
            ::cbreak();
            ::noecho();
            ::curs_set(0);
            ::clear();


            while(1)
            {
               snake.show();
            }
*/

            exit(0);

        }
        else
            file = argv[5];
    }

    if( argc > 6 )
    {
        ::max_gens = atoi( argv[6] );

        if( ::max_gens == 0 )
        {
            train_out_filename = argv[6];
            train_file.open( train_out_filename );
        }
        else
        {
            ::auto_capture_highest = true;
            ::snake_warp = true;
        }
    }
        

    


    Population pop( wx, wy, population, threads, file );

    pop.initializeSnakes( wx, wy );

    pop.update();

    
    pop.show();


    if( train_out_filename.length() > 0 )
    {
        end_it_all = true;
        train_out_mutex.lock();
        train_file.close();
        train_out_mutex.unlock();
        dlib::sleep( 500 );
        ::refresh();
        ::endwin();

        exit(0);
    }


    if( file.empty() )
        file = "nameless_snake.net";

    int localHighScore = pop.bestScore();

    while( true )
    {
        if( pop.done() )
        {
            pop.calculateFitness();
            ::last_avg_score = pop.avg_score();
            pop.naturalSelection();
            if( auto_capture_highest )
            {
               if( pop.bestScore() > localHighScore ) 
               {
                  std::stringstream num;
                  num << "." << pop.bestScore();
                  serialize( file + num.str()  ) << pop.bestSnake->get_net();
               }
            }
            //highscore = pop.bestSnake->score();
            pop.show();
        }
        else
        {
            //pop.update();
            pop.handle_snakes();
            //pop.show();
        }

        localHighScore = pop.bestScore();

        dlib::sleep( 10 );

        if( ::max_gens > 0 && ::generation > ::max_gens )
            break;
    }
        ::sleep(1);



    ::refresh();
    ::endwin();

    return 0;
}

//  ----------------------------------------------------------------------------

// Normally, if you built this application on MS Windows in Visual Studio you
// would see a black console window pop up when you ran it.  The following
// #pragma directives tell Visual Studio to not include a console window along
// with your application.  However, if you prefer to have the console pop up as
// well then simply remove these #pragma statements.
#ifdef _MSC_VER
#   pragma comment( linker, "/entry:mainCRTStartup" )
#   pragma comment( linker, "/SUBSYSTEM:WINDOWS" )
#endif

//  ----------------------------------------------------------------------------

