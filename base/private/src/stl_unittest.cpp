/*

*/

#include "test.h"

#ifdef DO_UNITTEST

#pragma warning(disable: 4786 4127)

#include <iostream>
#include <algorithm>
#include <functional>
#include <vector>
#include <map>
#include <hash_map>
#include <hash_set>

using namespace std;


int upper_bound_vector() {

  const int VECTOR_SIZE = 8 ;

  // Define a template class vector of int
  typedef vector<int > IntVector ;

  //Define an iterator for template class vector of strings
  typedef IntVector::iterator IntVectorIt ;

  IntVector Numbers(VECTOR_SIZE) ;

  IntVectorIt start, end, it, location ;

  // Initialize vector Numbers
  Numbers[0] = 4 ;
  Numbers[1] = 12;
  Numbers[2] = 12 ;
  Numbers[3] = 30 ;
  Numbers[4] = 69 ;
  Numbers[5] = 70 ;
  Numbers[6] = 96 ;
  Numbers[7] = 100;

  start = Numbers.begin() ;   // location of first
  // element of Numbers

  end = Numbers.end() ;       // one past the location
  // last element of Numbers

  // print content of Numbers
  cout << "Numbers { " ;
  for(it = start; it != end; it++)
    cout << *it << " " ;
  cout << " }\n" << endl ;

  //return the last location at which 10 can be inserted
  // in Numbers
  location = lower_bound(start, end, 10) ;
  location = upper_bound(start, end, 10) ;

  cout << "Element 10 can be inserted at index "
    << location - start << endl ;

  return 0;
}

// http://www.cplusplus.com/reference/map/map/upper_bound/
int upper_bound_map() {

  typedef std::map<int,int> my_map_t;
  //typedef stdext::hash_map<int,int> my_map_t;

  my_map_t mymap;
  my_map_t::iterator itlow,itup;

  mymap[0]=20;
  mymap[1]=21;
  mymap[2]=40;
  mymap[31]=60;
  mymap[4]=80;
  mymap[5]=100;
  mymap[6]=200;

  itlow=mymap.lower_bound (2);  // itlow points to b
  itup=mymap.upper_bound (5);   // itup points to e (not d!) {

  --itlow;

  mymap.erase(itlow,itup);        // erases [itlow,itup) {

  // print content:
  for (my_map_t::iterator it=mymap.begin(); it!=mymap.end(); ++it)
    std::cout << it->first << " => " << it->second << '\n';

  return 0;
}

int upper_bound_set() {

  //typedef std::map<int,int> my_map_t;
  typedef stdext::hash_set<int> my_set_t;

  my_set_t myset;
  my_set_t::iterator itlow,itup;

  myset.insert(1);
  myset.insert(2);
  myset.insert(3);
  myset.insert(4);
  myset.insert(5);
  myset.insert(6);

  itlow=myset.lower_bound (3);  // itlow points to b
  itup=myset.upper_bound (4);   // itup points to e (not d!) {

  myset.erase(itlow,itup);        // erases [itlow,itup) {

  // print content:
  for (my_set_t::iterator it=myset.begin(); it!=myset.end(); ++it)
    std::cout << (*it) << " => " << (*it) << '\n';

  return 0;
}


TEST_BEGIN(base, stl) {
  
  upper_bound_vector();
  upper_bound_map();
  upper_bound_set();

} TEST_END


#endif // DO_UNITTEST
