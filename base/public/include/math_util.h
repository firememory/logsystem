/*

*/

#ifndef MATH_UTIL_H_
#define MATH_UTIL_H_

//////////////////////////////////////////////////////////////////////////
BEGIN_NAMESPACE_BASE

template<typename _EuclideanRingElement>
static _EuclideanRingElement gcd(_EuclideanRingElement __m, _EuclideanRingElement __n) {
  while (__n != 0){
    _EuclideanRingElement __t = __m % __n;
    __m = __n;
    __n = __t;
  }
  return __m;
}


END_NAMESPACE_BASE

#endif // MATH_UTIL_H_
