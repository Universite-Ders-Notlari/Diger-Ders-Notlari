#pragma once

namespace RayTracer
{
	using namespace System;

	public class Vertex
	{
		public:
			float X;
			float Y;
			float Z;

		public:

		Vertex()
		{

		}

		Vertex(float x, float y, float z)
		{
			X = x;
			Y = y;
			Z = z;
		}

		Vertex Normalize()
		{
			float length = (float)Math::Sqrt(X*X + Y*Y + Z*Z);
			X /= length;
			Y /= length;
			Z /= length;
			return *this;
		}

		float Length()
		{
			return (float)Math::Sqrt(X*X + Y*Y + Z*Z);;
		}

		Vertex CrossProduct(Vertex p)
		{
			return Vertex(
				Y*p.Z - Z*p.Y,
				Z*p.X - X*p.Z,
				X*p.Y - Y*p.X);
		}

		Vertex operator + (Vertex p)
		{
			return Vertex(X + p.X, Y + p.Y, Z + p.Z);
		}

		Vertex operator - (Vertex p)
		{
			return Vertex(X - p.X, Y - p.Y, Z - p.Z);
		}

		float operator * (Vertex p)
		{
			return X * p.X + Y * p.Y + Z * p.Z;
		}

		Vertex operator * (float f)
		{
			return Vertex(f*X, f*Y, f*Z);
		}

		Vertex operator / (float f)
		{
			return Vertex(X / f, Y / f, Z / f);
		}
	};

	Vertex operator * (float f, Vertex p)
	{
		return Vertex(f * p.X, f * p.Y, f * p.Z);
	}
}
