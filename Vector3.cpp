/* 
 * File:   Vector3.cpp
 * Author: Kevin
 * 
 * Created on 14 December 2013, 17:13
 */

#include <cmath>
#include "Vector3.h"
#include <ostream>

using namespace Mathematics;

//default constructor
template<class T>
Vector3<T>::Vector3() 
        : xyz() //will zero initialise all elements - Tested
{
}

//constructor
template<class T>
Vector3<T>::Vector3(T p_xyz) 
        : x ( p_xyz ), y ( p_xyz ), z ( p_xyz )
{}

//other constructor
template<class T>
Vector3<T>::Vector3(T p_x, T p_y, T p_z) 
        : x ( p_x ), y ( p_y ), z ( p_z )
{}

//copy constructor - makes a copy, no shit.
template<class T>
Vector3<T>::Vector3(const Vector3& orig) 
        : x ( orig.x ), y ( orig.y ), z ( orig.z )
{}

//UNARY

template<class T>
Vector3<T> Vector3<T>::operator -() const
{
    return Vector3(-x, -y, -z);
}

//magnitude of a vector
template<class T>
T Vector3<T>::operator ! () const{
    return sqrt(x * x + y * y + z * z);
}

//normalised vector
template<class T>
Vector3<T> Vector3<T>::operator ~ () const{
    T invMag = 1 / !(*this); //get inverse, multiplication is faster
    return Vector3(x * invMag, y * invMag, z * invMag);
}

template<class T>
bool Vector3<T>::isNotSet() const
{
    return (x == 0) && (y == 0) && (z == 0);
}

//returns the angle
template<class T>
T Vector3<T>::angle(const Vector3 &v2) const{
    return acos((*this * v2) / (!*this * !v2));
}

//returns the cosine of the angle
template<class T>
T Vector3<T>::cosAngle(const Vector3 &v2) const{
    return (*this * v2) / (!*this * !v2);
}

//binary operators
template<class T>
Vector3<T> Vector3<T>::operator +(const Vector3& v) const
{
    return Vector3 (x + v.x, y + v.y, z + v.z);
}

template<class T>
Vector3<T> Vector3<T>::operator - (const Vector3 &v) const{
    return Vector3 (x - v.x, y - v.y, z - v.z);
}

//cross-product which will return n^ vector
template<class T>
Vector3<T> Vector3<T>::operator ^ (const Vector3 &v) const{
    return Vector3 (y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x); //return non-dynamically constructed object
}

//Hadamard product
template<class T>
Vector3<T> Vector3<T>::hadamard (const Vector3 &v) const{
    return Vector3 (x * v.x, y * v.y, z * v.z);
}

//mag product
template<class T>
Vector3<T> Vector3<T>::operator * (T f) const{
    return Vector3 (x * f, y * f, z * f);
}

//mag product -> 1 /  f
template<class T>
Vector3<T> Vector3<T>::operator / (T f) const{
    f = 1 / f;
    return Vector3 (x * f, y * f, z * f);
}

//dot product
template<class T>
T Vector3<T>::operator * (const Vector3 &v) const{
    return x * v.x + y * v.y + z * v.z;
}

template<class T>
bool Vector3<T>::operator ==(const Vector3& p_vector) const
{
    return xyz[0] == p_vector.xyz[0] &&
           xyz[1] == p_vector.xyz[1] &&
           xyz[2] == p_vector.xyz[2];
}

//self-binary operator
template<class T>
Vector3<T>& Vector3<T>::operator += (const Vector3 &v){
    x += v.x;
    y += v.y;
    z += v.z;
    return *this; // return reference to this object
}

template<class T>
Vector3<T>& Vector3<T>::operator -= (const Vector3 &v){
    x -= v.x;
    y -= v.y;
    z -= v.z;
    return *this; // return reference to this object
}

//self cross-product which will return n^ vector
template<class T>
Vector3<T>& Vector3<T>::operator ^= (const Vector3 &v) {
    const float aY = z * v.x - x * v.z;
    const float aZ = x * v.y - y * v.x;
    x = y * v.z - z * v.y;
    y = aY;
    z = aZ;
    return *this;
}

template<class T>
Vector3<T>& Vector3<T>::setHadamard (const Vector3 &v) {
    x *= v.x;
    y *= v.y;
    z *= v.z;
    return *this;
}

template<class T>
Vector3<T>& Vector3<T>::operator *= (T f){
    x *= f;
    y *= f;
    z *= f;
    return *this; // return reference to this object
}

template<class T>
Vector3<T>& Vector3<T>::operator /= (T f){
    f = 1 / f;
    x *= f;
    y *= f;
    z *= f;
    return *this; // return reference to this object
}

//friend definition
template<class T>
Vector3<T> Mathematics::operator* (T f, const Vector3<T> &v){
    return Vector3<T>(f * v.x, f * v.y, f * v.z);
}

template<class T>
std::ostream& Mathematics::operator<<(std::ostream& strm, const Vector3<T>& v) {
  return strm << "(" << v.x << ", " << v.y << ", " << v.z << ")";
}

//static variables
template<class T>
const Vector3<T> Vector3<T>::UnitX = Vector3<T>(1, 0, 0);
template<class T>
const Vector3<T> Vector3<T>::UnitY = Vector3<T>(0, 1, 0);
template<class T>
const Vector3<T> Vector3<T>::UnitZ = Vector3<T>(0, 0, 1);

//initialise as uint8_t

using InitType = uint8_t;
template class Vector3<InitType>;
template Vector3<InitType> operator*<InitType> (InitType f, const Vector3<InitType> &v);
template std::ostream& operator<< <InitType> (std::ostream&, const Vector3<InitType>&);

