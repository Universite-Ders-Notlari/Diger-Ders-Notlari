#pragma once

#include "Vertex.h"
#include <vcclr.h>
#include <string>

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
			gcroot<Color> ShapeColor;

			virtual float Intersect(Vertex Ro, Vertex Rd) = 0;

			virtual Vertex NormalAt(Vertex p) = 0;
	};
}