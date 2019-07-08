

//class Snake : public Fl_Double_Window

#include <iostream>
#include <string>
#include "snake-ai.h"
#include <mutex>

//std::vector< la::la_vector<double> > vecs;

extern int snake_delay;
extern bool snake_warp;
extern int mutation;
extern int generation;
extern int highscore;
extern bool end_it_all;
extern bool auto_capture_highest;
extern double last_avg_score;

std::mutex train_out_mutex;
std::string train_out_filename;
std::ofstream train_file;

Snake::Snake( unsigned int seed )
	: rnd( seed )
    //, net(prelu_(rnd.get_double_in_range(.001,.999)), prelu_(rnd.get_double_in_range(.001,.999)))
    //, net(relu_(rnd.get_double_in_range(.001,.999)), relu_(rnd.get_double_in_range(.001,.999)))
    , trainer( net )
{
    //cout << " $ ";
    randomize( this );
}

Snake::Snake( Snake* s, unsigned int seed )

	: rnd( seed )
    //, net(prelu_(rnd.get_double_in_range(.001,.999)), prelu_(rnd.get_double_in_range(.001,.999)))
    //, net(relu_(rnd.get_double_in_range(.001,.999)), relu_(rnd.get_double_in_range(.001,.999)))
    , trainer( net )

{
    net = s->net;
    //cout << " n ";

}

Snake::~Snake()
{
   // cout << " * ";
}

void Snake::init( int wx, int wy, int x, int y )
{

    m_wx = wx;
    m_wy = wy;

    if( x < 0 )
        x = rnd.get_integer(wx);

    if( y < 0 )
        y = rnd.get_integer(wy);

    m_moves_left = 500;
    m_moves = 0;
    
    m_snake.clear();

    //cout << x << " : " << y << endl;    
 
    m_snake.push_back( part(x,y) );

    if( x <= 1 ) x++;
    else x--;

    if( y <= 1 ) y++;
    else y--;

    m_snake.push_back( part(x,y) );


    set_food( );

}


Snake* Snake::clone()
{

    Snake* snake = new Snake( this, rnd.get_random_32bit_number() );

    return snake;

}

Snake* Snake::procreate( Snake* m )
{

    int len = gather_dna( m_fx, m );
    gather_dna( m_fy, this );

    float dna[len];

    combine( m_fx, m_fy, dna, len );
    mutate( dna, (float)::mutation, len );

    Snake* baby = new Snake( rnd.get_random_32bit_number() );
    place_dna( dna, baby );


    return baby;
    
}


void Snake::combine( float* x, float* y, float* z, int len )
{

    for( int i=0; i < len; i++ )
        z[i] = x[i];

    for( int i=0; i < len; i++ )
        if( ( rnd.get_integer(1000) % 2 ) == 0 )
            z[i] = y[i];

    return;

    int i,r,c,rR,rC;

    i=0; r=4; c=LAYERS_2;
    rR = rnd.get_integer(r-1);
    rC = rnd.get_integer(c-1);
    for( int j=0; j<r; j++ )
       for( int k=0; k<c; k++ )
          if((j < rR) || (j == rR && k <= rC))
             z[i+(r*j+k)] = y[i+(r*j+k)];

    i+=r*c; r=LAYERS_2; c=LAYERS_1;
    rR = rnd.get_integer(r-1);
    rC = rnd.get_integer(c-1);
    for( int j=0; j<r; j++ )
       for( int k=0; k<c; k++ )
          if((j < rR) || (j == rR && k <= rC))
            z[i+(r*j+k)] = y[i+(r*j+k)];

    i+=r*c; r=LAYERS_1; c=24;
    rR = rnd.get_integer(r-1);
    rC = rnd.get_integer(c-1);
    for( int j=0; j<r; j++ )
       for( int k=0; k<c; k++ )
          if((j < rR) || (j == rR && k <= rC))
            z[i+(r*j+k)] = y[i+(r*j+k)];


}

void Snake::mutate( float* dna, float percent, int len )
{

    float prc = (percent / 100.0 );

    for( int i=0; i < len; i++ )
    {
        if(  rnd.get_random_float() < prc  )
	{
            //dna[i] = rnd.get_double_in_range( -.9999, .9999 );
            dna[i] = rnd.get_random_gaussian() / 5.0;
            dna[i] = ( dna[i] > 1.0 ) ? 1.0 : dna[i];
            dna[i] = ( dna[i] < -1.0 ) ? -1.0 : dna[i];
        }
    }

}


int Snake::read_snake( const char* snake_file, char* membuf )
{
    int x=0;

    FILE* fp = fopen( snake_file, "r+" );

    for( int i=0; feof( fp ) == 0; i++ ) 
	x += ::fread( &membuf[i], 1, 1, fp );

    fclose( fp );
    return x;
}


void Snake::write_snake( const char* snake_file, const char* membuf, int len )
{

   FILE* fp = fopen( snake_file, "w+" );

   for( int i=0; i < len; i++ )
      ::fwrite( membuf, 1, 1, fp );

   fclose( fp );
}


int Snake::gather_dna( float* dna, Snake* s )
{

    int x=0;

    auto w1 = layer< tag1, 1 >( s->get_net() ).layer_details().get_weights();
    //cout << "(" << w1.nr() << "," << w1.nc() << ":" << w1.size() << ")" << w1.k() << " ";
    int s1 = w1.size();
    //int x1 = w1.k();
    //int y1 = s1 / x1;
    for( int i=0; i < s1; i++ )
       dna[x++] = w1.host()[i];

    auto w2 = layer< tag2, 1 >( s->get_net() ).layer_details().get_weights();
    //cout << "(" << w2.nr() << "," << w2.nc() << ":" << w2.size() << ")" << w2.k() << " ";
    int s2 = w2.size();
    for( int i=0; i < s2; i++ )
       dna[x++] = w2.host()[i];

    auto w3 = layer< tag3, 1 >( s->get_net() ).layer_details().get_weights();
    //cout << "(" << w3.nr() << "," << w3.nc() << ":" << w3.size() << ")" << w3.k() << " ";
    int s3 = w3.size();
    for( int i=0; i < s3; i++ )
       dna[x++] = w3.host()[i];


    return x;
}


int Snake::place_dna( float* dna, Snake* s )
{

    int x=0;

    auto w1 = layer< tag1, 1 >( s->get_net() ).layer_details().get_weights();
    int s1 = w1.size();
    for( int i=0; i < s1; i++ )
       w1.host()[i] = dna[x++];

    auto w2 = layer< tag2, 1 >( s->get_net() ).layer_details().get_weights();
    int s2 = w2.size();
    for( int i=0; i < s2; i++ )
       w2.host()[i] = dna[x++];

    auto w3 = layer< tag3, 1 >( s->get_net() ).layer_details().get_weights();
    int s3 = w3.size();
    for( int i=0; i < s3; i++ )
       w3.host()[i] = dna[x++];


    return x;
}


void Snake::randomize( Snake* s )
{


    //auto w1 = layer< tag1 >( s->get_net() ).subnet().layer_details().get_weights();
    auto w1 = layer< tag1, 1 >( s->get_net() ).layer_details().get_weights();
    int s1 = w1.size();
    for( int i=0; i < s1; i++ )
       //w1.host()[i] = rnd.get_double_in_range( 0.0, 1.0 );
       w1.host()[i] = (float)rnd.get_double_in_range( -1.0, 1.0 );
       //w1.host()[i] = (float)rnd.get_random_gaussian() / 5.0;

    //auto w2 = layer< tag2 >( s->get_net() ).subnet().layer_details().get_weights();
    auto w2 = layer< tag2, 1 >( s->get_net() ).layer_details().get_weights();
    int s2 = w2.size();
    for( int i=0; i < s2; i++ )
       //w2.host()[i] = rnd.get_double_in_range( 0.0, 1.0 );
       w2.host()[i] = (float)rnd.get_double_in_range( -1.0, 1.0 );
       //w2.host()[i] = (float)rnd.get_random_gaussian() / 5.0;

    //auto w3 = layer< tag3 >( s->get_net() ).subnet().layer_details().get_weights();
    auto w3 = layer< tag3, 1 >( s->get_net() ).layer_details().get_weights();
    int s3 = w3.size();
    for( int i=0; i < s3; i++ )
       //w3.host()[i] = rnd.get_double_in_range( 0.0, 1.0 );
       w3.host()[i] = (float)rnd.get_double_in_range( -1.0, 1.0 );
       //w3.host()[i] = (float)rnd.get_random_gaussian() / 5.0;


}





void Snake::move()
{
    int wx = m_wx;
    int wy = m_wy;
    
    m_moves++;
    m_moves_left--;
    m_got_food = false;
    
    unsigned int direction;// = ::rand() % 4;
    //sqrt( (x2 − x1)2 + (y2 − y1)2 )
    
    part p = m_snake.front();

    
    auto hit_wall = [&] (const part& h) -> bool {

        
        if( h.x <= 0 )
            return true;
        else if( h.y <= 0 )
            return true;
        else if( h.x >= wx )
            return true;
        else if( h.y >= wy )
            return true;
        
        return false;
    };
    
    auto hit_body = [&] (const part& h) -> bool {
        
            int i=0;
            for( auto s : m_snake )
            {
                if( i++ > 0  && h.x == s.x && h.y == s.y )
                {
                    return true;
                }
            }
        
        return false;
    };
    

    auto look = [&] ( int x, int y ) -> pair< float, float > {
        float df = 1000.0;
        float dw = 1000.0;
        float db = 1000.0;

        int i=0;
        int lx = p.x;
        int ly = p.y;
        
        while( i < 1000 )
        {
            i++;
            
            lx += x;
            ly += y;
            
            for( auto s : m_snake )
            {
                if( lx == s.x && ly == s.y )
                {
                    db = sqrt( std::pow( (p.x - lx), 2.0) + std::pow( (p.y - ly), 2.0) );
                    return make_pair( 1, db );
                }
            }

            
            if( lx == m_food.x && ly == m_food.y )
            {
                df = sqrt( std::pow( (p.x - lx), 2.0 ) + std::pow( (p.y - ly), 2.0 ) );
                return make_pair( 2, df );
            }

            
            if( lx == wx || ly == wy || lx == 0 || ly == 0 )
            {
                dw = sqrt( std::pow( (p.x - lx), 2.0) + std::pow( (p.y - ly), 2.0) );
                return make_pair( 0, dw );
            }
            
        }

        //cerr << "look() error" << endl;

        return make_pair( -1, 0.0 );
    };


    int views[][2] = {
	{ 0, -1 }, // up
	{ 0,  1 }, // down
	{-1,  0 }, // left
	{ 1,  0 }, // right 
	{-1, -1 }, // up / left
	{ 1, -1 }, // up / right
	{-1,  1 }, // down / left
	{ 1,  1 }  // down / right
    };

    for( int d=0; d < 8; d++ )
    {
        auto saw = look( views[d][0], views[d][1] );
        m_imat(0,d) = (float)( saw.first == 1.0 ? 1.0 : 0.0 ); // body
        m_imat(1,d) = (float)( saw.first == 2.0 ? 1.0 : 0.0 ); // food
        m_imat(2,d) = 1.0 / saw.second; // distance
    }
    
    std::vector< input_matrix_type > vecMats = { m_imat };
    direction = net( vecMats )[0];
   
  
 
    //cout << " " << m_snake.size() << endl;
 
    part back_part( m_snake.back() );


    m_oldpart = back_part;
    
    switch( direction )
    {
        case 0: // up
            p.y--;
            break;
        case 1: // down
            p.y++;
            break;
        case 2: // left
            p.x--;
           break;
        case 3: // right
            p.x++;
            break;
    }
    
    
    
    //if( hit_wall( p )  )
    if( hit_wall( p ) || hit_body( p ) || m_moves_left <= 0)
    {
        m_dead = true;
        return;
    }
    else if( p.x == m_food.x && p.y == m_food.y )
    {
        m_got_food = true;
        m_moves_left += 100;
        set_food();
    }
    else
        m_snake.pop_back();
    
    m_snake.push_front( p );


    if( train_out_filename.length() > 0 ) 
    {

        train_out_mutex.lock();
        cerr.precision(9);
	for( int i=0; i < 8; i++ )
            train_file << m_imat(0,i) << "," << m_imat(1,i) << "," << m_imat(2,i) << ","; 

        train_file << direction << endl;

        train_out_mutex.unlock();
    }

}

void Snake::set_food( int x, int y )
{
    if( x < 0 )
        x = 1+rnd.get_integer(m_wx-1);

    if( y < 0 )
        y = 1+rnd.get_integer(m_wy-1);

    m_food.x = x;
    m_food.y = y;
}

void Snake::show()
{

    ::clear();
    ::refresh();

    for( int i=0; i<m_wx; i++)
        ::mvprintw( i, 0, "*" );

    for( int i=0; i<m_wx; i++)
        ::mvprintw( i, m_wy, "*" );

    for( int i=0; i<m_wy; i++)
        ::mvprintw( 0, i, "*" );

    for( int i=0; i<m_wy; i++)
        ::mvprintw( m_wx, i, "*" );

    ::mvprintw( m_wy + 1, 5, "avg score: %0.2f", ::last_avg_score );
    ::mvprintw( m_wy + 2, 5, "high score: %d", ::highscore );
    ::mvprintw( m_wy + 3, 5, "generation: %d", ::generation );
    ::mvprintw( m_wy + 4, 5, "mutation:   %d%%", ::mutation );
    ::mvprintw( m_wy + 5, 5, "auto capture: %s", ::auto_capture_highest ? "on" : "off" );
    

    do
    {
        move();

        mvaddch( m_food.x, m_food.y, '#' );

        mvaddch( m_oldpart.x, m_oldpart.y, ' ' );

        for( auto p : m_snake )
            mvaddch( p.x, p.y, '0' );

        ::refresh();

        if( ::snake_warp == false )
            dlib::sleep( ::snake_delay );

        if( end_it_all == true )
        {
            refresh();
            endwin();
            train_out_mutex.lock();
            train_file.close();
            train_out_mutex.unlock();
            exit(0);
        }

    } while( !dead() );
}


