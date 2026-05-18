#pragma once

#include "Vertex.h"
#include <vcclr.h>

namespace RayTracer
{
	using namespace System;
	using namespace System::Drawing;

	class Shape
	{
		public:
			float Ambient;
			float Dif;
			float Spec;
			float Refl;
			float Trans;
			float Refrac;
			int Tex;
			gcroot<Color> ShapeColor;
			Vertex V0;					// for Texture Mapping iPoint
			Vertex V1;
			Vertex V2;

			virtual float Intersect(Vertex Ro, Vertex Rd) = 0;

			virtual Vertex NormalAt(Vertex p) = 0;
	};
}