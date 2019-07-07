#include <iostream>
//#include "snake-ai.h"


#include <math.h>
#include <vector>
#include <deque>

#include <dlib/matrix.h>
#include <dlib/dnn.h>
#include <thread>
#include <chrono>
#include <sstream>

using namespace dlib;
using namespace std;


using input_matrix_type = matrix<double, 3, 8>;

#if 0
using net_type = loss_multiclass_log<
                                fc<4,        
                                relu<fc<18,   
                                relu<fc<18,  
                                input<  input_matrix_type
                                >>>>>>>;


#else
using net_type = loss_multiclass_log<
                                tag1<fc<4,relu<        
                                tag2<fc<18,relu<   
                                tag3<relu<fc<18,  
                                input<  input_matrix_type
                                >>>>>>>>>>>;
#endif                                
                                


int main()
{


        dlib::rand  rnd( ::time(NULL) );

        for( int i=0; i < 100; i++ )
            //cout << rnd.get_random_gaussian()/5.0 << endl;
            cout << rnd.get_random_float() << endl;



	char membuf[10000] = {0};
/*
	FILE* fp = fopen( "snake.net", "r+" );

	for( int i; feof( fp ) == 0; i++ ) 
		::fread( &membuf[i], 1, 1, fp );

	cout.precision(9);


	for( int j=0; j < 24; j++ )
	{
		for( int i=0; i < 18; i++ )
		{
			cout << ((float*)(&membuf[0x49]))[18*j+i] << ' ';
		}
		cout << endl;
	}
	cout << endl;


	for( int j=0; j < 18; j++ )
	{
		for( int i=0; i < 18; i++ )
		{
			cout << ((float*)(&membuf[0x8fc]))[18*j+i] << ' ';
		}
		cout << endl;
	}
	cout << endl;


	for( int j=0; j < 4; j++ )
	{
		for( int i=0; i < 18; i++ )
		{
			cout << ((float*)(&membuf[0xffd]))[18*j+i] << ' ';
		}
		cout << endl;
	}
	cout << endl;
*/



	//Snake s;
	net_type net;

	//deserialize( "/tmp/snake.male.net" ) >> net;

	cout << net << endl << endl;

	cout << layer< tag2 >(net).subnet().layer_details()        << endl;
	cout << layer< tag3 >(net).subnet().layer_details()        << endl;

	cout << layer< tag2 >(net).subnet().get_output().nc()   << endl;
	cout << layer< tag2 >(net).subnet().get_output().host()   << endl;


	//cout << net.subnet().subnet().subnet().details() <<  endl;
	//cout << layer<3>(net).get_parameter_gradient().host()[0]        <<  endl;
	//const tensor &t = layer<3>(net).get_output();
	//cout << t.host() << endl << endl;


}




