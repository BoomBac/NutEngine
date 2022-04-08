#ifndef NUT_MATH_H
#define NUT_MATH_H

#include <math.h>
#include <sstream>
#include <string>
#include <limits>

constexpr float kPi = 3.1415926f;
constexpr float kDPi = kPi * 2.f;

namespace Engine
{
	template<typename T, size_t size_of_arr>
	constexpr size_t CountOf(T(&)[size_of_arr]) { return size_of_arr; }

	template<typename T, size_t row, size_t col>
	constexpr size_t CountOf(T(&)[row][col]) { return row * col; }

	template<typename T>
	constexpr float normalize(T var)
	{
		return var < 0 ? -static_cast<float>(var) / (std::numeric_limits<T>::min)()
			: static_cast<float>(var) / (std::numeric_limits<T>::max)();
	}
	template<typename T>
	void Normalize(T& var)
	{
		size_t len = CountOf(var.data);
		float temp[4];
		for (uint32_t i = 0; i < len; i++)
		{
			temp[i] = normalize(var.data[i]);
			var[i] = temp[i];
		}

	}

	template <template<typename> class TT, typename T, int ... Indexes>
	class Swizzle
	{
		T v[sizeof...(Indexes)];
	public:
		TT<T>& operator=(const TT<T>& rhs)
		{
			int indexes[] = { Indexes... };
			for (int i = 0; i < sizeof...(Indexes); i++)
			{
				v[indexes[i]] = rhs[i];
			}
			return *(TT<T>*)this;
		}
		operator TT<T>() const
		{
			return TT<T>(v[Indexes]...);
		}
	};
	template<typename T>
	struct Vector2D
	{
		union
		{
			T data[2];
			struct { T x, y; };
			struct { T r, g; };
			struct { T u, v; };
			Swizzle<Vector2D, T, 0, 1> xy;
			Swizzle<Vector2D, T, 1, 0> yx;
		};
		Vector2D<T>() {};
		Vector2D<T>(const T& v) : x(v), y(v) {};
		Vector2D<T>(const T& v, const T& w) : x(v), y(w) {};
		operator T* () { return data; };
		operator const T* () { return static_cast<const T*>(data); };
	};
	using Vector2f = Vector2D<float>;

	template <typename T>
	struct Vector3D
	{
		union {
			T data[3];
			struct { T x, y, z; };
			struct { T r, g, b; };
			Swizzle<Vector2D, T, 0, 1> xy;
			Swizzle<Vector2D, T, 1, 0> yx;
			Swizzle<Vector2D, T, 0, 2> xz;
			Swizzle<Vector2D, T, 2, 0> zx;
			Swizzle<Vector2D, T, 1, 2> yz;
			Swizzle<Vector2D, T, 2, 1> zy;
			Swizzle<Vector3D, T, 0, 1, 2> xyz;
			Swizzle<Vector3D, T, 1, 0, 2> yxz;
			Swizzle<Vector3D, T, 0, 2, 1> xzy;
			Swizzle<Vector3D, T, 2, 0, 1> zxy;
			Swizzle<Vector3D, T, 1, 2, 0> yzx;
			Swizzle<Vector3D, T, 2, 1, 0> zyx;
		};

		Vector3D<T>() {};
		Vector3D<T>(const T& _v) : x(_v), y(_v), z(_v) {};
		Vector3D<T>(const T& _x, const T& _y, const T& _z) : x(_x), y(_y), z(_z) {};

		operator T* () { return data; };
		operator const T* () const { return static_cast<const T*>(data); };
	};

	using Vector3f = Vector3D<float>;

	template <typename T>
	struct Vector4D
	{
		union {
			T data[4];
			struct { T x, y, z, w; };
			struct { T r, g, b, a; };
			Swizzle<Vector3D, T, 0, 2, 1> xzy;
			Swizzle<Vector3D, T, 1, 0, 2> yxz;
			Swizzle<Vector3D, T, 1, 2, 0> yzx;
			Swizzle<Vector3D, T, 2, 0, 1> zxy;
			Swizzle<Vector3D, T, 2, 1, 0> zyx;
			Swizzle<Vector4D, T, 2, 1, 0, 3> bgra;
		};

		Vector4D<T>() {};
		Vector4D<T>(const T& _v) : x(_v), y(_v), z(_v), w(_v) {};
		Vector4D<T>(const T& _x, const T& _y, const T& _z, const T& _w) : x(_x), y(_y), z(_z), w(_w) {};
		Vector4D<T>(const Vector3D<T>& v3) : x(v3.x), y(v3.y), z(v3.z), w(1.0f) {};
		Vector4D<T>(const Vector3D<T>& v3, const T& _w) : x(v3.x), y(v3.y), z(v3.z), w(_w) {};

		operator T* () { return data; };
		operator const T* () const { return static_cast<const T*>(data); };
	};

	using Vector4f = Vector4D<float>;
	using Quaternion = Vector4D<float>;
	using R8G8B8A8Unorm = Vector4D<uint8_t>;
	using Vector4i = Vector4D<uint8_t>;

	template<template<typename> typename TT, typename T>
	std::string VectorToString(TT<T> arr)
	{
		std::string str;
		std::stringstream ss;
		ss << "(";
		for (uint32_t i = 0; i < CountOf(arr.data); i++)
		{
			ss << arr.data[i];
			if (CountOf(arr.data) - 1 == i) ss << " ";
			else ss << ",";
		}
		ss >> str; str.append(")\n");
		return str;
	}
	template<template<typename> typename TT, typename T>
	TT<T> operator+(const TT<T> v1, const TT<T> v2)
	{
		TT<T> res;
		for (uint32_t i = 0; i < CountOf(v1.data); i++)
		{
			res.data[i] = v1.data[i] + v2.data[i];
		}
		return res;
	}
	/// <summary>
	/// Any number of vector additions, with at least two parameters passed in
	/// </summary>
	template<template<typename> typename FF, typename F, template<typename> typename... TT, typename T>
	FF<F> VectorAdd(const FF<F> first, const TT<T>&... arg)
	{
		return (first + ... + arg);
	}
	template<template<typename> typename TT, typename T>
	TT<T> operator-(const TT<T> v1, const TT<T> v2)
	{
		TT<T> res;
		for (uint32_t i = 0; i < CountOf(v1.data); i++)
		{
			res.data[i] = v1.data[i] - v2.data[i];
		}
		return res;
	}
	template<template<typename> typename FF, typename F, template<typename> typename... TT, typename T>
	FF<F> VectorSub(const FF<F> first, const TT<T>&... arg)
	{
		return (first - ... - arg);
	}
	template<template<typename> typename TT, typename T>
	T DotProduct(const TT<T>& first, const TT<T>& second)
	{
		T res{};
		for (uint32_t i = 0; i < CountOf(first.data); i++)
		{
			res += first.data[i] * second.data[i];
		}
		return res;
	}
	/// <summary>
	/// Only 3D vectors can be input for now
	/// </summary>
	template<template<typename> typename TT, typename T>
	TT<T> CrossProduct(const TT<T>& first, const TT<T>& second)
	{
		return TT<T>(first.data[1] * second.data[2] - first.data[2] * second.data[1],
			first.data[2] * second.data[0] - first.data[0] * second.data[2],
			first.data[0] * second.data[1] - first.data[1] * second.data[0]);
	}

	template <typename T, int rows, int cols>
	struct Matrix
	{
		union
		{
			T data[rows][cols];
		};
		T* operator[](int row_index)
		{
			return data[row_index];
		}
		const T* operator[](int row_index) const
		{
			return data[row_index];
		}
		operator T* () { return &data[0][0]; };
		operator const T* () const { return static_cast<const T*>(&data[0][0]); };
	};
	using Matrix4x4f = Matrix<float, 4, 4>;

	template<typename T, int rows, int cols>
	std::string MatrixToString(Matrix<T, rows, cols> mat)
	{
		std::stringstream ss;
		for (uint32_t i = 0; i < rows; i++)
		{
			ss << "[ ";
			for (uint32_t j = 0; j < cols; j++)
			{
				if (j != cols - 1) ss << mat[i][j] << ",";
				else ss << mat[i][j] << " ]" << "\r\n";
			}
		}
		std::string temp;
		std::string res;
		while (!ss.eof())
		{
			getline(ss, temp);
			res.append(temp);
		}
		return res;
	}

	template<typename T, int rows, int cols>
	void MatrixAdd(Matrix<T, rows, cols>& ret, const Matrix<T, rows, cols>& m1, const Matrix<T, rows, cols>& m2)
	{
		for (uint32_t i = 0; i < CountOf(m1.data); i++)
		{
			for (uint32_t j = 0; j < CountOf(m2.data); j++)
			{
				ret[i][j] = m1[i][j] + m2[i][j];
			}
		}
	}

	template<typename T, int rows, int cols>
	Matrix<T, rows, cols> operator+(const Matrix<T, rows, cols>& m1, const Matrix<T, rows, cols>& m2)
	{
		Matrix<T, rows, cols> ret;
		for (uint32_t i = 0; i < CountOf(m1.data); i++)
		{
			for (uint32_t j = 0; j < CountOf(m2.data); j++)
			{
				ret[i][j] = m1[i][j] + m2[i][j];
			}
		}
		return ret;
	}

	template<typename T, int rows, int cols>
	void MatrixSub(Matrix<T, rows, cols>& ret, const Matrix<T, rows, cols>& m1, const Matrix<T, rows, cols>& m2)
	{
		for (uint32_t i = 0; i < CountOf(m1.data); i++)
		{
			for (uint32_t j = 0; j < CountOf(m2.data); j++)
			{
				ret[i][j] = m1[i][j] - m2[i][j];
			}
		}
	}

	template<typename T, int rows, int cols>
	Matrix<T, rows, cols> operator-(const Matrix<T, rows, cols>& m1, const Matrix<T, rows, cols>& m2)
	{
		Matrix<T, rows, cols> ret;
		for (uint32_t i = 0; i < CountOf(m1.data); i++)
		{
			for (uint32_t j = 0; j < CountOf(m2.data); j++)
			{
				ret[i][j] = m1[i][j] - m2[i][j];
			}
		}
		return ret;
	}

	template<typename T, int rows, int cols>
	Matrix<T, cols, rows> Transpose(const Matrix<T, rows, cols>& mat)
	{
		Matrix<T, cols, rows> ret;
		for (uint32_t i = 0; i < rows; i++)
		{
			for (uint32_t j = 0; j < cols; j++)
			{
				ret[j][i] = mat[i][j];
			}
		}
		return ret;
	}

	template <typename T, int Da, int Db, int Dc>
	void MatrixMultipy(Matrix<T, Da, Dc>& ret, const Matrix<T, Da, Db>& m1, const Matrix<T, Dc, Db>& m2)
	{
		Matrix<T, Db, Dc> m2_t;
		m2_t = Transpose(m2);
		for (uint32_t i = 0; i < Da; i++)
		{
			for (uint32_t j = 0; j < Dc; j++)
			{
				for (uint32_t k = 0; k < Dc; k++)
					ret[i][j] += m1[i][k] * m2_t[j][k];
			}
		}
	}
	template <typename T, int row, int col>
	Matrix<T, row, col> operator*(const Matrix<T, row, col>& m1, const Matrix<T, row, col>& m2)
	{
		Matrix<T, row, col> ret;
		MatrixMultipy(ret, m1, m2);
		return m1;
	}
	static void MatrixRotationYawPitchRoll(Matrix4x4f& matrix, const float yaw, const float pitch, const float roll)
	{
		float cYaw, cPitch, cRoll, sYaw, sPitch, sRoll;
		// Get the cosine and sin of the yaw, pitch, and roll.
		cYaw = cosf(yaw);
		cPitch = cosf(pitch);
		cRoll = cosf(roll);
		sYaw = sinf(yaw);
		sPitch = sinf(pitch);
		sRoll = sinf(roll);
		// Calculate the yaw, pitch, roll rotation matrix.
		Matrix4x4f tmp = { {{
			{ (cRoll * cYaw) + (sRoll * sPitch * sYaw), (sRoll * cPitch), (cRoll * -sYaw) + (sRoll * sPitch * cYaw), 0.0f },
			{ (-sRoll * cYaw) + (cRoll * sPitch * sYaw), (cRoll * cPitch), (sRoll * sYaw) + (cRoll * sPitch * cYaw), 0.0f },
			{ (cPitch * sYaw), -sPitch, (cPitch * cYaw), 0.0f },
			{ 0.0f, 0.0f, 0.0f, 1.0f }
		}} };
		matrix = tmp;
		return;
	}
	static void Transform(Vector4f& vector, const Matrix4x4f& matrix)
	{
		Vector4f temp;
		for (uint32_t i = 0; i < 4; i++)
		{
			for (uint32_t j = 0; j < 4; j++)
			{
				temp[j] += vector[i] * matrix[i][j];
			}
		}
		vector = temp;
		return;
	}
	static void TransformCoord(Vector3f& vector, const Matrix4x4f& matrix)
	{
		Vector4f temp;
		Transform(temp, matrix);
		vector[0] = temp[0];
		vector[1] = temp[1];
		vector[2] = temp[2];
		return;
	}
	static void BuildViewMatrix(Matrix4x4f& result, const Vector3f position, const Vector3f lookAt, const Vector3f up)
	{
		Vector3f zAxis, xAxis, yAxis;
		float result1, result2, result3;
		zAxis = lookAt - position;
		Normalize(zAxis);
		xAxis = CrossProduct(up, zAxis);
		Normalize(xAxis);
		yAxis = CrossProduct(zAxis, xAxis);
		result1 = DotProduct(xAxis, position);
		result1 = -result1;
		result2 = DotProduct(yAxis, position);
		result2 = -result2;
		result3 = DotProduct(zAxis, position);
		result3 = -result3;
		Matrix4x4f tmp = { {{
			{ xAxis.x, yAxis.x, zAxis.x, 0.0f },
			{ xAxis.y, yAxis.y, zAxis.y, 0.0f },
			{ xAxis.z, yAxis.z, zAxis.z, 0.0f },
			{ result1, result2, result3, 1.0f }
		}} };
		result = tmp;
	}
	static void BuildIdentityMatrix(Matrix4x4f& matrix)
	{
		Matrix4x4f identity = { {{
			{ 1.0f, 0.0f, 0.0f, 0.0f},
			{ 0.0f, 1.0f, 0.0f, 0.0f},
			{ 0.0f, 0.0f, 1.0f, 0.0f},
			{ 0.0f, 0.0f, 0.0f, 1.0f}
		}} };
		matrix = identity;
		return;
	}
	static void BuildPerspectiveFovLHMatrix(Matrix4x4f& matrix, const float fieldOfView, const float screenAspect, const float screenNear, const float screenDepth)
	{
		Matrix4x4f perspective = { {{
			{ 1.0f / (screenAspect * tanf(fieldOfView * 0.5f)), 0.0f, 0.0f, 0.0f },
			{ 0.0f, 1.0f / tanf(fieldOfView * 0.5f), 0.0f, 0.0f },
			{ 0.0f, 0.0f, screenDepth / (screenDepth - screenNear), 1.0f },
			{ 0.0f, 0.0f, (-screenNear * screenDepth) / (screenDepth - screenNear), 0.0f }
		}} };
		matrix = perspective;
		return;
	}
	static void MatrixTranslation(Matrix4x4f& matrix, const float x, const float y, const float z)
	{
		Matrix4x4f translation = { {{
			{ 1.0f, 0.0f, 0.0f, 0.0f},
			{ 0.0f, 1.0f, 0.0f, 0.0f},
			{ 0.0f, 0.0f, 1.0f, 0.0f},
			{    x,    y,    z, 1.0f}
		}} };
		matrix = translation;
		return;
	}
	static void MatrixRotationX(Matrix4x4f& matrix, const float angle)
	{
		float c = cosf(angle), s = sinf(angle);

		Matrix4x4f rotation = { {{
			{  1.0f, 0.0f, 0.0f, 0.0f },
			{  0.0f,    c,    s, 0.0f },
			{  0.0f,   -s,    c, 0.0f },
			{  0.0f, 0.0f, 0.0f, 1.0f },
		}} };

		matrix = rotation;

		return;
	}
	static void MatrixRotationY(Matrix4x4f& matrix, const float angle)
	{
		float c = cosf(angle), s = sinf(angle);

		Matrix4x4f rotation = { {{
			{    c, 0.0f,   -s, 0.0f },
			{ 0.0f, 1.0f, 0.0f, 0.0f },
			{    s, 0.0f,    c, 0.0f },
			{ 0.0f, 0.0f, 0.0f, 1.0f },
		}} };
		matrix = rotation;
		return;
	}
	static void MatrixRotationZ(Matrix4x4f& matrix, const float angle)
	{
		float c = cosf(angle), s = sinf(angle);
		Matrix4x4f rotation = { {{
			{    c,    s, 0.0f, 0.0f },
			{   -s,    c, 0.0f, 0.0f },
			{ 0.0f, 0.0f, 1.0f, 0.0f },
			{ 0.0f, 0.0f, 0.0f, 1.0f }
		}} };
		matrix = rotation;
		return;
	}
	static void MatrixRotationAxis(Matrix4x4f& matrix, const Vector3f& axis, const float angle)
	{
		float c = cosf(angle), s = sinf(angle), one_minus_c = 1.0f - c;
		Matrix4x4f rotation = { {{
			{   c + axis.x * axis.x * one_minus_c,  axis.x * axis.y * one_minus_c + axis.z * s, axis.x * axis.z * one_minus_c - axis.y * s, 0.0f    },
			{   axis.x * axis.y * one_minus_c - axis.z * s, c + axis.y * axis.y * one_minus_c,  axis.y * axis.z * one_minus_c + axis.x * s, 0.0f    },
			{   axis.x * axis.z * one_minus_c + axis.y * s, axis.y * axis.z * one_minus_c - axis.x * s, c + axis.z * axis.z * one_minus_c, 0.0f },
			{   0.0f,  0.0f,  0.0f,  1.0f   }
		}} };
		matrix = rotation;
	}

	static void MatrixScale(Matrix4x4f& matrix, const float x, const float y, const float z)
	{
		Matrix4x4f scale = { {{
		{    x, 0.0f, 0.0f, 0.0f},
		{ 0.0f,    y, 0.0f, 0.0f},
		{ 0.0f, 0.0f,    z, 0.0f},
		{ 0.0f, 0.0f, 0.0f, 1.0f},
		}} };
		matrix = scale;
		return;
	}

	static void MatrixRotationQuaternion(Matrix4x4f& matrix, Quaternion q)
	{
		Matrix4x4f rotation = { {{
			{   1.0f - 2.0f * q.y * q.y - 2.0f * q.z * q.z,  2.0f * q.x * q.y + 2.0f * q.w * q.z,   2.0f * q.x * q.z - 2.0f * q.w * q.y,    0.0f    },
			{   2.0f * q.x * q.y - 2.0f * q.w * q.z,    1.0f - 2.0f * q.x * q.x - 2.0f * q.z * q.z, 2.0f * q.y * q.z + 2.0f * q.w * q.x,    0.0f    },
			{   2.0f * q.x * q.z + 2.0f * q.w * q.y,    2.0f * q.y * q.z - 2.0f * q.y * q.z - 2.0f * q.w * q.x, 1.0f - 2.0f * q.x * q.x - 2.0f * q.y * q.y, 0.0f    },
			{   0.0f,   0.0f,   0.0f,   1.0f    }
		}} };
		matrix = rotation;
	}

}

#endif // NUT_MATH_H

