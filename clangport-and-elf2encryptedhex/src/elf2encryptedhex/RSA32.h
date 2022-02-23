// Ported from https://github.com/jimmiebergmann/RSA32
// Names, nicknames or e-mails are not related to us
///////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2013 Jimmie Bergmann - jimmiebergmann@gmail.com
//
// This software is provided 'as-is', without any express or
// implied warranty. In no event will the authors be held
// liable for any damages arising from the use of this software.
// 
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute
// it freely, subject to the following restrictions:
// 
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but
//    is not required.
// 
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
// 
// 3. This notice may not be removed or altered from any
//    source distribution.
// 
///////////////////////////////////////////////////////////////////////////////////

#ifndef __RSA_HPP__
#define __RSA_HPP__

// Notes:
// 1.	We are using the srand and rand function from stdlib.h
//		but we could roll our own FASTER randomization function.
// 2.	I don't think you should [en][de]crypt any message that's larger than
//		the two primes that we are using to calculate n and z ( the primes p and q ).
// 3.	It's taking way too long time to calculate e and d when we are using high primes.
//		FIXED since EuclideanAlgorithm( ) was fixed?
// 4.	What happends when are getting out of range in EuclideanAlgorithm( ) ? Stuck in loop?
//		FIXED now by using unsigned ints instead?

// 32 bit version of RSA.
class RSA32
{

public:

	// Constructors
	RSA32( );
	RSA32( const unsigned long long int p_e, const unsigned long long int p_d, const unsigned long long int p_n );
	RSA32( const unsigned long long int p_e, const unsigned long long int p_d,
		const unsigned long long int p_n, const unsigned long long int p_z );

	// Initialization functions

	////////////////////
	bool RandomizeKeysa( const unsigned long long int p_seed, unsigned long long int pkey, unsigned long long int qkey, unsigned long long int ekey );
	///////////////////

	bool RandomizeKeys( const unsigned long long int p_seed ); // Randomly generate the keys.
	bool CalculateKeys( const unsigned long long int p_p, const unsigned long long int p_q ); // p_p and p_q must be 2 different primes.
	bool CalculatePublicKey( ); // Calculate the public key by using n and z which already should be set.
	bool CalculatePrivateKey( ); // Calculate the private key by using e, n and z which already should be set.
	bool CrackPrivateKey( ); // Requires e and n
	void Clear( ); // Clear all the varaibles.

	// Cryptographic functions

	/////////////////////////
	unsigned long long int Encrypta( const unsigned long long int p_message, unsigned long long int pkey, unsigned long long int qkey, unsigned long long int ekey );
	////////////////////////

	unsigned long long int Encrypt( const unsigned long long int p_message );
	unsigned long long int Decrypt( const unsigned long long int p_message );
	

	// Set functions
	void SetE( const unsigned long long int p_e );
	void SetD( const unsigned long long int p_d );
	void SetP( const unsigned long long int p_p );
	void SetQ( const unsigned long long int p_q );
	void SetN( const unsigned long long int p_n );
	void SetZ( const unsigned long long int p_z );

	// Get functions
	unsigned long long int GetE( ) const;
	unsigned long long int GetD( ) const;
	unsigned long long int GetP( ) const;
	unsigned long long int GetQ( ) const;
	unsigned long long int GetN( ) const;
	unsigned long long int GetZ( ) const;

private:

	// Private functions
	void CalculateNZ( const unsigned long long int p_p, const unsigned long long int p_q ); // p and q should be 2 "large" primes.
	bool CalculateNZED( const unsigned long long int p_p, const unsigned long long int p_q ); // Combining the upper calculation functions.

	// Variable members.
	unsigned long long int m_e;	// Encryption key.
	unsigned long long int m_d; // Decryption key.
	unsigned long long int m_p; // Prime 1.
	unsigned long long int m_q; // Prime 2.
	unsigned long long int m_n; // Shared key = p * q
	unsigned long long int m_z; // Phi( n ) = ( p - 1 ) * ( q - 1 )

};

#include "RSA32.h"
#include <stdlib.h>
#include <cmath>
#include <stdint.h>
//#include <boost/multiprecision/cpp_int.hpp>

///////////////////////////////////////////////////////////////////////////////////
// General functions used that's being defined on the bottom of this file
unsigned long long int PowMod( unsigned long long int base, unsigned long long int exponent, unsigned long long int modulus );
unsigned long long int RandomNumber( unsigned long long int min, unsigned long long int max );
unsigned long long int RandomPrime( unsigned long long int min, unsigned long long int max );
//int EuclideanAlgorithm ( int a, int b ); // Euclid's algorithm
unsigned long long int EuclideanAlgorithm ( unsigned long long int a, unsigned long long int b ); // Euclid's algorithm
bool IsPrime( unsigned long long int number );

///////////////////////////////////////////////////////////////////////////////////
// Constructors
RSA32::RSA32( ) :
	m_e( 0 ),
	m_d( 0 ),
	m_n( 0 ),
	m_z( 0 )
{
}

RSA32::RSA32( const unsigned long long int p_e, unsigned long long int p_d, const unsigned long long int p_n ) :
	m_e( p_e ),
	m_d( p_d ),
	m_n( p_n )
{
}

RSA32::RSA32( const unsigned long long int p_e, const unsigned long long int p_d,
		 const unsigned long long int p_n, const unsigned long long int p_z ) :
	m_e( p_e ),
	m_d( p_d ),
	m_n( p_n ),
	m_z( p_z )
{
}


bool RSA32::RandomizeKeysa( const unsigned long long int p_seed, unsigned long long int pkey, unsigned long long int qkey, unsigned long long int ekey )
{
	// You have to seed the rand function by yourself if you're not
	// providing any seed.
	if( p_seed != 0 )
	{
		srand( p_seed );
	}

	// Set p and q by generating random primes.
	m_p = 0;
	m_q = 0;

	// Do start at 46341 since it's closest to the lowest possible 32 bit number
	// by any number multiplied by itself. 46341^2 almost equals to the lowest 32 bit number...
	unsigned long long int prime_range_low = 46341; //32768;
	unsigned long long int prime_range_high = 65535;

	// Make sure that they're not equal to each other.
	while( m_p == m_q || m_p == 0 || m_q == 0 )
	{
		m_p = RandomPrime( prime_range_low, prime_range_high );
		m_q = RandomPrime( prime_range_low, prime_range_high );
	}
	
	//////////////////////////////////
	//m_p = 9661;
	//m_q = 48781;

	m_p = pkey;
	m_q = qkey;

	// n = 65537 * 65543 = 4.295.491.591 > 32 bit max number 4.294.967.295
	/////////////////////////////////

	// Calculate n, z, e and d by using two primes: p and q. Simple? :)
	if( !CalculateNZED( m_p, m_q ) )
	{	
		return false;
	}
	m_e = ekey;
	return true;
}

// Initialization functions
bool RSA32::RandomizeKeys( const unsigned long long int p_seed )
{
	// You have to seed the rand function by yourself if you're not
	// providing any seed.
	if( p_seed != 0 )
	{
		srand( p_seed );
	}

	// Set p and q by generating random primes.
	m_p = 0;
	m_q = 0;

	// Do start at 46341 since it's closest to the lowest possible 32 bit number
	// by any number multiplied by itself. 46341^2 almost equals to the lowest 32 bit number...
	unsigned long long int prime_range_low = 46341; //32768;
	unsigned long long int prime_range_high = 65535;

	// Make sure that they're not equal to each other.
	while( m_p == m_q || m_p == 0 || m_q == 0 )
	{
		m_p = RandomPrime( prime_range_low, prime_range_high );
		m_q = RandomPrime( prime_range_low, prime_range_high );
	}
	
	//////////////////////////////////
	//m_p = 9661;
	//m_q = 48781;

	m_p = 65537;
	m_q = 65543;

	// n = 65537 * 65543 = 4.295.491.591 > 32 bit max number 4.294.967.295
	/////////////////////////////////

	// Calculate n, z, e and d by using two primes: p and q. Simple? :)
	if( !CalculateNZED( m_p, m_q ) )
	{	
		return false;
	}

	return true;
}

// p_p and p_q should be 2 different primes
bool RSA32::CalculateKeys( const unsigned long long int p_p, const unsigned long long int p_q )
{
	// Set p and q
	m_p = p_p;
	m_q = p_q;

	// Calculate n, z, e and d by using two primes: p and q. Simple? :)
	if( !CalculateNZED( m_p, m_q ) )
	{	
		return false;
	}

	return true;
}

// Calculate the public key by using n and z which already should be set.
bool RSA32::CalculatePublicKey( )
{
	// e is the encryption key( public key )
	m_e = 0;

	// Let's find e
	// e is odd and small.
	// !NOTE !NOTE !NOTE !NOTE 
	// What about the prime 2?
	for( unsigned long long int i = 3; i < m_z; i+=2 )
	{
		// Find the greatest common divisor. We want it to be 1. (coprime)
		if( EuclideanAlgorithm( m_z, i ) == 1 )
		{
			m_e = i;
			break;
		}
	}

	// Make sure we've found e, it should not be 0
	if( m_e == 0 )
	{
		return false;
	}

	//////////////////////
	//m_e = 459226363;
	m_e = 65537;
	/////////////////////

	return true;
}

// Calculate the private key by using e, n and z which already should be set.
bool RSA32::CalculatePrivateKey( )
{
	// d is the decryption key( private key ).
	// Do not let anyone see it except you.
	m_d = 0;

	// Use temporary 64 bit varaibles
	//unsigned long long int temp_d = m_d;
	//const unsigned long long int temp_n = m_n;
	//const unsigned long long int temp_z = m_z;
	//const unsigned long long int temp_e = m_e;

	__int128 temp_d = m_d;
	__int128 temp_n = m_n;
	__int128 temp_z = m_z;
	__int128 temp_e = m_e;
	
	// Let's find d
	bool found_d = false;
	for( /*unsigned long long int*/ __int128 i = 0; i < temp_n; i++ )
	{
		// We've found d when ((i * z) + 1) / e is 0
		if( ( ( i * temp_z ) + 1 ) % temp_e == 0 )
		{
			temp_d = ( ( i * temp_z ) + 1 ) / temp_e;
			found_d = true;
			break;
		}
	}

	// Error check to make sure we've found d.
	if( !found_d )
	{
		return false;
	}

	// Set d.
	m_d = static_cast< unsigned long long int >( temp_d );
	return true;
}

// Requires e and n
bool RSA32::CrackPrivateKey( )
{
	// Resources:
	// http://stackoverflow.com/questions/4078902/cracking-short-rsa-keys

	// p and q might get flipped, but it doesn't matter at al.

	// Get the square root of n and floor it.
	unsigned long long int temp_p = static_cast<unsigned long long int>( floor( sqrt( static_cast<double>( m_n ) ) ) );
	bool found_p = false;

	// Get the closest prime below temp_p;
	while( temp_p >= 2 )
	{
		if( IsPrime( temp_p ) )
		{
			found_p = true;
			break;
		}

		// Decrease p by 2.
		temp_p--;
	}

	// Error check if we didn't find p
	if( !found_p )
	{
		return false;
	}

	// Reset the found p flag and do another last test in order to make sure we've found p
	found_p = false;

	while( temp_p >= 2 )
	{
		// n mod temp_p should be 0 if we've found p
		if( m_n % temp_p == 0 )
		{
			m_p = temp_p;
			found_p = true;
			break;
		}
		
		// Decrease p by 2.
		temp_p -= 2;
	}

	// Error check if we didn't find p, once again.
	if( !found_p )
	{
		return false;
	}

	// Calculate the second prime q and then finall z as well.
	m_q = m_n / m_p;
	m_z = ( m_p - 1 ) * ( m_q - 1 );

	// The last thing to do is to calculate the private key d
	return CalculatePrivateKey( );
}

// Clear all the varaibles.
void RSA32::Clear( )
{
	m_e = 0;
	m_d = 0;
	m_p = 0;
	m_q = 0;
	m_n = 0;
	m_z = 0;
}

// Cryptographic functions

unsigned long long int RSA32::Encrypta( const unsigned long long int p_message, unsigned long long int pkey, unsigned long long int qkey, unsigned long long int ekey )
{
	RandomizeKeysa(0, pkey, qkey, ekey);
	return PowMod( p_message, m_e, m_n );
}

unsigned long long int RSA32::Encrypt( const unsigned long long int p_message )
{
	return PowMod( p_message, m_e, m_n );
}

unsigned long long int RSA32::Decrypt( const unsigned long long int p_message )
{
	return PowMod( p_message, m_d, m_n );
}

// Set functions
void RSA32::SetE( const unsigned long long int p_e )
{
	m_e = p_e;
}

void RSA32::SetD( const unsigned long long int p_d )
{
	m_d = p_d;
}

void RSA32::SetP( const unsigned long long int p_p )
{
	m_p = p_p;
}
void RSA32::SetQ( const unsigned long long int p_q )
{
	m_q = p_q;
}

void RSA32::SetN( const unsigned long long int p_n )
{
	m_n = p_n;
}

void RSA32::SetZ( const unsigned long long int p_z )
{
	m_z = p_z;
}

// Get functions
unsigned long long int RSA32::GetE( ) const
{
	return m_e;
}

unsigned long long int RSA32::GetD( ) const
{
	return m_d;
}

unsigned long long int RSA32::GetP( ) const
{
	return m_p;
}

unsigned long long int RSA32::GetQ( ) const
{
	return m_q;
}

unsigned long long int RSA32::GetN( ) const
{
	return m_n;
}

unsigned long long int RSA32::GetZ( ) const
{
	return m_z;
}

// p and q should be 2 "large" primes
void RSA32::CalculateNZ( const unsigned long long int p_p, const unsigned long long int p_q )
{
	m_p = p_p;
	m_q = p_q;

	m_n = m_p * m_q;
	m_z = ( m_p - 1 ) * ( m_q - 1 );
}

bool RSA32::CalculateNZED( const unsigned long long int p_p, const unsigned long long int p_q )
{
	m_p = p_p;
	m_q = p_q;

	// Calculate n and z
	CalculateNZ( m_p, m_q );

	// Calculate e
	if( !CalculatePublicKey( ) )
	{
		return false;
	}

	// Calculate D
	if( !CalculatePrivateKey( ) )
	{
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////
// General function definitions

// Power modulus functions. 
unsigned long long int PowMod( unsigned long long int base, unsigned long long int exponent, unsigned long long int modulus )
{
	// Resource:
	// http://en.wikipedia.org/wiki/Modular_exponentiation

	if( base < 1 || exponent < 0 || modulus < 1 )
	{
		return 0;
	}

	// use 64 bit integers to make sure we're not getting out of range.
	//unsigned long long  int result = 1;
	//unsigned long long  int new_base = base;
	//unsigned long long  int new_exponent = exponent;
	//unsigned long long  int new_modulus = modulus;

	//boost::multiprecision::int128_t result = 1;
	//boost::multiprecision::int128_t new_base = base;
	//boost::multiprecision::int128_t new_exponent = exponent;
	//boost::multiprecision::int128_t new_modulus = modulus;

	__int128 result = 1;
	__int128 new_base = base;
	__int128 new_exponent = exponent;
	__int128 new_modulus = modulus;

	// Keep on looping until the exponent which is being divided
	// every single loop is larger than 0.
	while( new_exponent > 0 )
	{
		if( ( new_exponent % 2 ) == 1 )
		{
			result = ( result * new_base ) % new_modulus;
		}

		new_exponent = new_exponent >> 1;
		new_base = ( new_base * new_base ) % new_modulus;
	}

	// return the result as an unsigned int.
	return static_cast< unsigned long long int >( result );
}

unsigned long long int RandomNumber( unsigned long long int min, unsigned long long int max )
{
	return ( rand( ) % ( max - ( min - 1 ) ) ) + min;
}

// The min and max values is not the absolute min/max values
// The outcome from this function might give a highter prime than the max value.
unsigned long long int RandomPrime( unsigned long long int min, unsigned long long int max )
{
	unsigned long long int diff = 0;
	unsigned long long int number = RandomNumber( min, max );

	// Make sure the number is odd
	if( number % 2 == 0 )
	{
		number++;
	}

	// Swap min and max if min is larger than max
	if( min > max)
	{
		unsigned long long int temp_min = min;
		min = max;
		max = temp_min;
	}

	// Calculate the diff
	diff = max - min;

	// Find a prime by using the randomly generated number as a start number.
	for( unsigned long long int i = 0; i < diff; i++ )
	{
		if( IsPrime( number ) )
		{
			return number;
		}

		number += 2;
	}

	// We did not find any prime.
	return 0;
}

// Euclid's algorithm
//	a > b
unsigned long long int EuclideanAlgorithm( unsigned long long int a, unsigned long long int b )
{
	if( b >= a )
	{
		return a;
	}

	// The actual euclidean algorithm	
	unsigned long long int c = 0;
	while( b != 0 )
	{
		c = a % b;
		a = b;
		b = c;
	}

	return a;
}

bool IsPrime( unsigned long long int number )
{
	// Resource:
	// http://holmezideas.com/programming/optimization-techniques/fastest-algorithm-to-check-if-a-given-number-is-prime/

	if( number <= 1 )
	{
		return false;
	}
	else if( number == 2 )
	{
		return true;
	}
	else if( ( number % 2 ) == 0 )
	{
		return false;
	}
	else
	{
		float square_root = sqrt( static_cast< float >( number ) );

		// Let's loop sqrt( number ) times and check if the number isn't a prime
		for( unsigned long long int i = 3; i <= square_root; i+=2 )
		{
			if( ( number % i ) == 0 )
			{
				return false;
			}
		}
	}

	return true;
}

#endif
