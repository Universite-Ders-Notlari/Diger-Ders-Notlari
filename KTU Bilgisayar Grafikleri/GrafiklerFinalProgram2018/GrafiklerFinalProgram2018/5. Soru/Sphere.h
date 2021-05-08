#pragma once

#include "Vertex.h"
#include "Shape.h"
#include <vcclr.h>

namespace RayTracer
{
	using namespace System;
	using namespace System::Drawing;

	class Sphere : public Shape
	{
		public:
			Vertex Center;
			float Radius;

		Sphere(Vertex Center, float radius, float amb, float dif, float spec, float refl, float trans, float refrac, Color shapeColor, int tex)
		{
			this->Center     = Center;
			this->Radius     = radius;
			this->Ambient	 = amb;
			this->Dif		 = dif;
			this->Spec		 = spec;
			this->Refl	     = refl;
			this->Trans      = trans;
			this->Refrac     = refrac;
			this->Tex        = tex;
			this->ShapeColor = shapeColor;
		}

		float Intersect(Vertex Ro, Vertex Rd)
		{
			Vertex l = Center - Ro;
			float s = l * Rd;
			float l2 = l * l;
			float r2 = Radius * Radius;
			if (s < 0 && l2 > r2) return 0;
			float s2 = s * s;
			float m2 = l2 - s2;
			if (m2 > r2) return 0;
			float q = (float)Math::Sqrt(r2 - m2);
			if (l2 > r2 + 0.001) return s - q;	// ((int)l2 > (int)r2) de olur
			else return s + q;
		}

		Vertex NormalAt(Vertex p)
		{
			return (p - Center) / Radius;
		}
	};
}