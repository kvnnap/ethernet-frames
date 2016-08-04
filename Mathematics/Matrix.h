/* 
 * File:   Matrix.h
 * Author: Kevin
 *
 * Created on 19 April 2014, 12:57
 * Edited on 19 April 2014, 12:57 PM
 */

#ifndef MATRIX_H
#define	MATRIX_H

#include <cstdint>
#include "Vector3.h"

namespace Mathematics
{
    template <class T>
    class Matrix;

    template <class T>
    Matrix<T> operator* (T scalar, const Matrix<T>& matrix);

    template <class T>
    class Matrix {
    public:
        Matrix(size_t rows, size_t columns);
        Matrix(const Matrix& orig);
        ~Matrix();
        //getters
        size_t getRows() const;
        size_t getColumns() const;
        bool isSquare() const;
        Matrix getIdentity() const;
        //static getters
        static Matrix getRotationX(T theta);
        static Matrix getRotationY(T theta);
        static Matrix getRotationZ(T theta);
        static Matrix getScaling(const Vector& scale);
        static Matrix getTranslation(const Vector& translation);

        const Matrix& operator = (const Matrix& matrix);
        //math operators
        Matrix operator * (const Matrix& matrix) const;
        //accepts only 3 or 4 columns as the 'this' matrix
        Vector operator * (const Vector& vector) const;
        Matrix operator * (T scalar) const;

        //this is global in this namespace
        template <class Y> //could use T above and remove this but it is more confusing to me
        friend Matrix<Y> operator* (Y scalar, const Matrix<Y>& matrix);

        Matrix operator + (const Matrix& matrix) const;
        Matrix operator - (const Matrix& matrix) const;

        const Matrix& operator *= (const Matrix& matrix);
        const Matrix& operator += (const Matrix& matrix);
        const Matrix& operator -= (const Matrix& matrix);
        //http://my.safaribooksonline.com/book/programming/cplusplus/0201309831/operator-overloading/ch23lev1sec5
        T& operator() (size_t row, size_t column);
        T operator() (size_t row, size_t column) const;
    private:
        const size_t rows_, columns_;//y,x
        T * const data_;
    };

    typedef Matrix<float> Matrix_F;
    //no need to redefine here, although you should but anyway..
    //Matrix operator * (T scalar, const Matrix& matrix);

    template <class T>
    std::ostream& operator<< (std::ostream&, const Matrix<T>&);
}



#endif	/* MATRIX_H */

