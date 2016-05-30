/* 
 * File:   Matrix.cpp
 * Author: Kevin
 * 
 * Created on September 8, 2012, 5:58 PM
 * Edited on 19 April 2014, 12:57 PM
 */

#include "Matrix.h"
#include <cmath>
#include "string.h"
#include <iostream>

using namespace Mathematics;

template <class T>
Matrix<T>::Matrix(size_t rows, size_t columns)
        : rows_ ( rows ), columns_ ( columns ), data_ ( new T[columns * rows]() )
{}

template <class T>
Matrix<T>::Matrix(const Matrix& orig)
        : rows_ ( orig.rows_ ), columns_ ( orig.columns_ ), data_ ( new T[rows_ * columns_] )
{
    memcpy(data_, orig.data_, rows_ * columns_ * sizeof(T));
}

template <class T>
Matrix<T>::~Matrix()
{
    delete[] data_;
}

//getters
template <class T>
size_t Matrix<T>::getRows() const
{
    return rows_;
}

template <class T>
size_t Matrix<T>::getColumns() const
{
    return columns_;
}

template <class T>
bool Matrix<T>::isSquare() const
{
    return rows_ == columns_;
}

template <class T>
Matrix<T> Matrix<T>::getIdentity() const
{
    Matrix tempMatrix (rows_, rows_);
    for(size_t i = 0; i < rows_; i++)
    {
        ((T (*) [tempMatrix.columns_])tempMatrix.data_)[i][i] = 1;
    }
    return tempMatrix;
}

//static getters
template <class T>
Matrix<T> Matrix<T>::getRotationX(T theta)
{
    Matrix temp (4 ,4);
    T compCos = cos(theta);
    T compSin = sin(theta);
    temp (0,0) = 1;
    temp (1,1) = compCos;
    temp (1,2) = -compSin;
    temp (2,1) = compSin;
    temp (2,2) = compCos;
    temp (3,3) = 1; //if 4x4
    return temp;
}

template <class T>
Matrix<T> Matrix<T>::getRotationY(T theta)
{
    Matrix temp (4 ,4);
    T compCos = cos(theta);
    T compSin = sin(theta);
    temp (0,0) = compCos;
    temp (0,2) = compSin;
    temp (1,1) = 1;
    temp (2,0) = -compSin;
    temp (2,2) = compCos;
    temp (3,3) = 1; //if 4x4
    return temp;
}

template <class T>
Matrix<T> Matrix<T>::getRotationZ(T theta)
{
    Matrix temp (4 ,4);
    T compCos = cos(theta);
    T compSin = sin(theta);
    temp (0,0) = compCos;
    temp (0,1) = -compSin;
    temp (1,0) = compSin;
    temp (1,1) = compCos;
    temp (2,2) = 1;
    temp (3,3) = 1; //if 4x4
    return temp;
}

template <class T>
Matrix<T> Matrix<T>::getScaling(const Vector& scale)
{
    Matrix temp (4,4);
    temp(0,0) = scale.xyz[0];
    temp(1,1) = scale.xyz[1];
    temp(2,2) = scale.xyz[2];
    temp(3,3) = 1;
    return temp;
}

template <class T>
Matrix<T> Matrix<T>::getTranslation(const Vector& translation)
{
    Matrix temp (4,4);
    temp(0,0) = 1;
    temp(0,3) = translation.xyz[0];
    temp(1,1) = 1;
    temp(1,3) = translation.xyz[1];
    temp(2,2) = 1;
    temp(2,3) = translation.xyz[2];
    temp(3,3) = 1;
    return temp;
}

//operators
template <class T>
const Matrix<T>& Matrix<T>::operator = (const Matrix& matrix)
{
    if(rows_ == matrix.rows_ && columns_ == matrix.columns_)
    {
        memcpy(data_, matrix.data_, rows_ * columns_ * sizeof(T));
    }
    return *this;
}

template <class T>
Matrix<T> Matrix<T>::operator *(const Matrix& matrix) const
{
    if(columns_ == matrix.rows_)
    {   //multiplication is defined and valid
        Matrix tempMatrix (rows_, matrix.columns_);
        //T (*cell)[tempMatrix.columns_] = (T (*) [tempMatrix.columns_]) tempMatrix.data_;
        for(size_t i = 0; i < tempMatrix.rows_; i++)
        {
            for(size_t j = 0; j < tempMatrix.columns_; j++)
            {
                for(size_t k = 0; k < columns_; k++)
                {
                    ((T (*) [tempMatrix.columns_])tempMatrix.data_)[i][j] += ((T (*) [columns_])data_)[i][k] * ((T (*) [matrix.columns_])matrix.data_)[k][j];
                }
            }
        }
        return tempMatrix;
    }else
    {
        return Matrix(0,0);
    }
}

template <class T>
Vector Matrix<T>::operator *(const Vector& vector) const
{
    //slow version
    if(columns_ == 3 || columns_ == 4)
    {   //multiplication is defined and valid
        Matrix tempMatrix (rows_, 1);
        Matrix matrix(columns_, 1);
        
        ((T (*) [matrix.columns_])matrix.data_)[0][0] = vector.xyz[0];
        ((T (*) [matrix.columns_])matrix.data_)[1][0] = vector.xyz[1];
        ((T (*) [matrix.columns_])matrix.data_)[2][0] = vector.xyz[2];
        if(columns_ == 4) { ((T (*) [matrix.columns_])matrix.data_)[3][0] = 1; }
        //T (*cell)[tempMatrix.columns_] = (T (*) [tempMatrix.columns_]) tempMatrix.data_;
        for(size_t i = 0; i < tempMatrix.rows_; i++)
        {
            for(size_t k = 0; k < columns_; k++)
            {
                ((T (*) [tempMatrix.columns_])tempMatrix.data_)[i][0] += ((T (*) [columns_])data_)[i][k] * ((T (*) [matrix.columns_])matrix.data_)[k][0];
            }
        }
        return Vector(((T (*) [tempMatrix.columns_])tempMatrix.data_)[0][0], ((T (*) [tempMatrix.columns_])tempMatrix.data_)[1][0], ((T (*) [tempMatrix.columns_])tempMatrix.data_)[2][0]);
    }else
    {
        return Vector();
    }
    /*if(columns_ == 3 || columns_ == 4)
    {   //multiplication is defined and valid
        Matrix tempMatrix (rows_, 1);
        //T (*cell)[tempMatrix.columns_] = (T (*) [tempMatrix.columns_]) tempMatrix.data_;
        for(int i = 0; i < tempMatrix.rows_; i++)
        {
            for(int j = 0; j < tempMatrix.columns_; j++)
            {
                for(int k = 0; k < columns_; k++)
                {
                    ((T (*) [tempMatrix.columns_])tempMatrix.data_)[i][j] += ((T (*) [columns_])data_)[i][k] * vector.xyz[j];
                }
            }
        }
        return tempMatrix;
    }else
    {
        return Matrix(0,0);
    }*/
}

template <class T>
Matrix<T> Matrix<T>::operator * (T scalar) const
{
    Matrix tempMatrix (*this);
    //T (*cell)[tempMatrix.columns_] = (T (*) [tempMatrix.columns_]) tempMatrix.data_;
    for(size_t i = 0; i < tempMatrix.rows_; i++)
    {
        for(size_t j = 0; j < tempMatrix.columns_; j++)
        {
            ((T (*) [tempMatrix.columns_])tempMatrix.data_)[i][j] *= scalar;   
        }
    }
    return tempMatrix;
}

template <class T>
Matrix<T> Matrix<T>::operator +(const Matrix& matrix) const
{
    if(rows_ == matrix.rows_ && columns_ == matrix.columns_)
    {
        Matrix tempMatrix (rows_, columns_);
        
        for(size_t i = 0; i < tempMatrix.rows_; i++)
        {
            for(size_t j = 0; j < tempMatrix.columns_; j++)
            {
                ((T (*) [tempMatrix.columns_])tempMatrix.data_)[i][j] = ((T (*) [columns_])data_)[i][j] + ((T (*) [matrix.columns_])matrix.data_)[i][j];
                //or use tempMatrix(i,j) = (*this)(i,j) + matrix(i,j);
            }
        }
        
        return tempMatrix;
    }else
    {
        return Matrix(0,0);
    }
}

template <class T>
Matrix<T> Matrix<T>::operator -(const Matrix& matrix) const
{
    if(rows_ == matrix.rows_ && columns_ == matrix.columns_)
    {
        Matrix tempMatrix (rows_, columns_);
        
        for(size_t i = 0; i < tempMatrix.rows_; i++)
        {
            for(size_t j = 0; j < tempMatrix.columns_; j++)
            {
                ((T (*) [tempMatrix.columns_])tempMatrix.data_)[i][j] = ((T (*) [columns_])data_)[i][j] - ((T (*) [matrix.columns_])matrix.data_)[i][j];
                //or use tempMatrix(i,j) = (*this)(i,j) + matrix(i,j);
            }
        }
        
        return tempMatrix;
    }else
    {
        return Matrix(0,0);
    }
}

template <class T>
const Matrix<T>& Matrix<T>::operator *=(const Matrix& matrix)
{
    //(rows_, matrix.columns_);
    if(columns_ == matrix.rows_ && columns_ == matrix.columns_)
    {   //multiplication is defined and valid
        Matrix tempMatrix (*this);
        //T (*cell)[tempMatrix.columns_] = (T (*) [tempMatrix.columns_]) tempMatrix.data_;
        for(size_t i = 0; i < tempMatrix.rows_; i++)
        {
            for(size_t j = 0; j < tempMatrix.columns_; j++)
            {
                ((T (*) [columns_])data_)[i][j] = 0;
                for(size_t k = 0; k < columns_; k++)
                {
                    ((T (*) [columns_])data_)[i][j]  += ((T (*) [tempMatrix.columns_])tempMatrix.data_)[i][k] * ((T (*) [matrix.columns_])matrix.data_)[k][j];
                }
            }
        }
        
        return (*this);
    }else
    {
        return (*this);
    }
}

template <class T>
const Matrix<T>& Matrix<T>::operator +=(const Matrix& matrix)
{
    if(rows_ == matrix.rows_ && columns_ == matrix.columns_)
    {   
        for(size_t i = 0; i < rows_; i++)
        {
            for(size_t j = 0; j < columns_; j++)
            {
                ((T (*) [columns_])data_)[i][j] += ((T (*) [matrix.columns_])matrix.data_)[i][j];
                //or use (*this)(i,j) += matrix(i,j);
            }
        }
        return (*this);
    }else
    {
        return (*this);
    }
}

template <class T>
const Matrix<T>& Matrix<T>::operator -=(const Matrix& matrix)
{
    if(rows_ == matrix.rows_ && columns_ == matrix.columns_)
    {   
        for(size_t i = 0; i < rows_; i++)
        {
            for(size_t j = 0; j < columns_; j++)
            {
                ((T (*) [columns_])data_)[i][j] -= ((T (*) [matrix.columns_])matrix.data_)[i][j];
                //or use (*this)(i,j) += matrix(i,j);
            }
        }
        return (*this);
    }else
    {
        return (*this);
    }
}

template <class T>
T Matrix<T>::operator ()(size_t row, size_t column) const
{
    return ((T (*) [columns_])data_)[row][column];
}

template <class T>
T& Matrix<T>::operator ()(size_t row, size_t column)
{
    return ((T (*) [columns_])data_)[row][column];
}

//getters


//globals in Mathematics - friends of Matrix
template <class T>
Matrix<T> Mathematics::operator* (T scalar, const Matrix<T>& matrix)
{
    Matrix<T> tempMatrix (matrix);
    //T (*cell)[tempMatrix.columns_] = (T (*) [tempMatrix.columns_]) tempMatrix.data_;
    for(size_t i = 0; i < tempMatrix.rows_; i++)
    {
        for(size_t j = 0; j < tempMatrix.columns_; j++)
        {
            ((T (*) [tempMatrix.columns_])tempMatrix.data_)[i][j] *= scalar;   
        }
    }
    return tempMatrix;
}

template<class T>
std::ostream& Mathematics::operator<<(std::ostream& strm, const Matrix<T>& matrix) {
    size_t rows = matrix.getRows(), columns = matrix.getColumns();
    for(size_t i = 0; i < rows; i++)
    {
        strm << "(";
        for(size_t j = 0; j < columns; j++)
        {
            strm << matrix(i, j);
            if (j + 1 != columns) {
                strm << ", ";
            }
        }
        strm << ")";

        if (i + 1 != rows) {
            strm << std::endl;
        }
    }
    return strm;
}

using InitType = uint8_t;
template class Matrix<InitType>;
template Matrix<InitType> operator*<InitType> (InitType, const Matrix<InitType>&);
template std::ostream& operator<< <InitType> (std::ostream&, const Matrix<InitType>&);