/* 
 * File:   Vector3.h
 * Author: Kevin
 *
 * Created on 14 December 2013, 17:13
 */

#ifndef VECTOR3_H
#define	VECTOR3_H

#include <ostream>

namespace Mathematics
{
    template<class T>
    class Vector3 {
    public:

        //unnamed unions is part of C++ standard
        union {
            //warning - unnamed structures are not C++ standards (as as side note C standard does not support both)
            struct {
                T x, y, z;
            };
            T xyz[3];
        };

        static const Vector3 UnitX;
        static const Vector3 UnitY;
        static const Vector3 UnitZ;

        //constructors
        Vector3();
        Vector3(T xyz); //will implicitly convert
        Vector3(T x, T y, T z);

        //copy constructor
        Vector3(const Vector3& orig);

        //unary operators
        Vector3 operator - () const;
        T operator ! () const; //magnitude
        Vector3 operator ~ () const; //normalise
        bool isNotSet() const;
        T angle(const Vector3 &v) const;
        T cosAngle(const Vector3 &) const;

        //binary operators
        Vector3 operator + (const Vector3 &) const;
        Vector3 operator - (const Vector3 &) const;
        Vector3 operator ^ (const Vector3 &) const; //cross-product
        Vector3 hadamard(const Vector3 &) const;
        Vector3 operator * (T) const; //scalar product
        Vector3 operator / (T) const; //scalar product
        T operator * (const Vector3 &) const; //dot product
        bool operator == (const Vector3 &) const; // not using ULP

        //self binary operators
        Vector3& operator += (const Vector3 &);
        Vector3& operator -= (const Vector3 &);
        Vector3& operator ^= (const Vector3 &);
        Vector3& setHadamard(const Vector3 &);
        Vector3& operator *= (T);
        Vector3& operator /= (T);

    private:

    };

    typedef Vector3<uint8_t> Vector;

    //Vector Multiplication Operator for
    template <class T>
    Vector3<T> operator* (T, const Vector3<T> &);

    template <class T>
    std::ostream& operator<< (std::ostream&, const Vector3<T>&);
}




#endif	/* VECTOR3_H */

