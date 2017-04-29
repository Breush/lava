#pragma once

namespace lava {

    /// \brief Utility template class for manipulating
    ///        2-dimensional vectors
    template <typename T>
    class Vector2 {
    public:
        /// \brief Default constructor
        ///
        /// Creates a Vector2(0, 0).
        Vector2();

        /// \brief Construct the vector from its coordinates
        ///
        /// \param X X coordinate
        /// \param Y Y coordinate
        Vector2(T X, T Y);

        /// \brief Construct the vector from another type of vector
        ///
        /// This constructor doesn't replace the copy constructor,
        /// it's called only when U != T.
        /// A call to this constructor will fail to compile if U
        /// is not convertible to T.
        ///
        /// \param vector Vector to convert
        ///

        template <typename U>
        explicit Vector2(const Vector2<U>& vector);

        // Member data

        T x; ///< X coordinate of the vector
        T y; ///< Y coordinate of the vector
    };

    /// \relates Vector2
    /// \brief Overload of unary operator -
    ///
    /// \param right Vector to negate
    ///
    /// \return Memberwise opposite of the vector
    ///

    template <typename T>
    Vector2<T> operator-(const Vector2<T>& right);

    /// \relates Vector2
    /// \brief Overload of binary operator +=
    ///
    /// This operator performs a memberwise addition of both vectors,
    /// and assigns the result to \a left.
    ///
    /// \param left  Left operand (a vector)
    /// \param right Right operand (a vector)
    ///
    /// \return Reference to \a left
    ///

    template <typename T>
    Vector2<T>& operator+=(Vector2<T>& left, const Vector2<T>& right);

    /// \relates Vector2
    /// \brief Overload of binary operator -=
    ///
    /// This operator performs a memberwise subtraction of both vectors,
    /// and assigns the result to \a left.
    ///
    /// \param left  Left operand (a vector)
    /// \param right Right operand (a vector)
    ///
    /// \return Reference to \a left
    ///

    template <typename T>
    Vector2<T>& operator-=(Vector2<T>& left, const Vector2<T>& right);

    /// \relates Vector2
    /// \brief Overload of binary operator +
    ///
    /// \param left  Left operand (a vector)
    /// \param right Right operand (a vector)
    ///
    /// \return Memberwise addition of both vectors
    ///

    template <typename T>
    Vector2<T> operator+(const Vector2<T>& left, const Vector2<T>& right);

    /// \relates Vector2
    /// \brief Overload of binary operator -
    ///
    /// \param left  Left operand (a vector)
    /// \param right Right operand (a vector)
    ///
    /// \return Memberwise subtraction of both vectors
    ///

    template <typename T>
    Vector2<T> operator-(const Vector2<T>& left, const Vector2<T>& right);

    /// \relates Vector2
    /// \brief Overload of binary operator *
    ///
    /// \param left  Left operand (a vector)
    /// \param right Right operand (a scalar value)
    ///
    /// \return Memberwise multiplication by \a right
    ///

    template <typename T>
    Vector2<T> operator*(const Vector2<T>& left, T right);

    /// \relates Vector2
    /// \brief Overload of binary operator *
    ///
    /// \param left  Left operand (a scalar value)
    /// \param right Right operand (a vector)
    ///
    /// \return Memberwise multiplication by \a left
    ///

    template <typename T>
    Vector2<T> operator*(T left, const Vector2<T>& right);

    /// \relates Vector2
    /// \brief Overload of binary operator *=
    ///
    /// This operator performs a memberwise multiplication by \a right,
    /// and assigns the result to \a left.
    ///
    /// \param left  Left operand (a vector)
    /// \param right Right operand (a scalar value)
    ///
    /// \return Reference to \a left
    ///

    template <typename T>
    Vector2<T>& operator*=(Vector2<T>& left, T right);

    /// \relates Vector2
    /// \brief Overload of binary operator /
    ///
    /// \param left  Left operand (a vector)
    /// \param right Right operand (a scalar value)
    ///
    /// \return Memberwise division by \a right
    ///

    template <typename T>
    Vector2<T> operator/(const Vector2<T>& left, T right);

    /// \relates Vector2
    /// \brief Overload of binary operator /=
    ///
    /// This operator performs a memberwise division by \a right,
    /// and assigns the result to \a left.
    ///
    /// \param left  Left operand (a vector)
    /// \param right Right operand (a scalar value)
    ///
    /// \return Reference to \a left
    ///

    template <typename T>
    Vector2<T>& operator/=(Vector2<T>& left, T right);

    /// \relates Vector2
    /// \brief Overload of binary operator ==
    ///
    /// This operator compares strict equality between two vectors.
    ///
    /// \param left  Left operand (a vector)
    /// \param right Right operand (a vector)
    ///
    /// \return True if \a left is equal to \a right
    ///

    template <typename T>
    bool operator==(const Vector2<T>& left, const Vector2<T>& right);

    /// \relates Vector2
    /// \brief Overload of binary operator !=
    ///
    /// This operator compares strict difference between two vectors.
    ///
    /// \param left  Left operand (a vector)
    /// \param right Right operand (a vector)
    ///
    /// \return True if \a left is not equal to \a right
    ///

    template <typename T>
    bool operator!=(const Vector2<T>& left, const Vector2<T>& right);

    // Define the most common types
    typedef Vector2<int> Vector2i;
    typedef Vector2<unsigned int> Vector2u;
    typedef Vector2<float> Vector2f;
}

#include <lava/chamber/Vector2.inl>
