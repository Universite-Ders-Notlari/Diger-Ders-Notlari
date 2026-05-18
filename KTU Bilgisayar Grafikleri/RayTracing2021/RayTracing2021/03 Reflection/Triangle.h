#pragma once

#include "Vertex.h"
#include "Shape.h"
#include <vcclr.h>

namespace RayTracer
{
	using namespace System;
	using namespace System::Drawing;

	class Triangle : public Shape
	{
		public:
			Vertex V0;
			Vertex V1;
			Vertex V2;

		Triangle(Vertex V0, Vertex V1, Vertex V2, float ambient, float dif, float spec, float refl, Color shapeColor)
		{
			this->V0 = V0;
			this->V1 = V1;
			this->V2 = V2;
			this->Ambient = ambient;
			this->Dif = dif;
			this->Spec = spec;
			this->Refl = refl;
			this->ShapeColor = shapeColor;
		}

		float Intersect(Vertex Ro, Vertex Rd)
		{
			Vertex normal;
			Vertex R;
			float S, s1, s2, s3;

			normal = (V1-V0).CrossProduct(V2-V1);

			float D = -(normal * V0);
			float t = -(normal * Ro + D) / (normal * Rd);

			if (t > 0)
			{
				R = Ro + t * Rd;

				S  = (V1 - V0).CrossProduct(V2 - V1).Length();
				s1 = ( R - V0).CrossProduct(V2 -  R).Length();
				s2 = (V1 - V0).CrossProduct( R - V1).Length();
				s3 = (V1 -  R).CrossProduct(V2 - V1).Length();

				float fark = (float)Math::Abs(S - (s1 + s2 + s3));
				float epsilon = 0.005F;

				if (fark <= epsilon) return t; else return 0;
			}
			else return 0;
		}

		Vertex NormalAt(Vertex p)
		{
			return (V1-V0).CrossProduct(V2-V1);
		}
	};
}