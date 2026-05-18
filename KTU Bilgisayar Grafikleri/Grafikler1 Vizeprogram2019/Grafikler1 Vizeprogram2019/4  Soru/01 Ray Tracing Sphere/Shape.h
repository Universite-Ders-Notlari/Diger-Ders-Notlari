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
			gcroot<Color> ShapeColor;

			virtual float Intersect(Vertex Ro, Vertex Rd) = 0;
	};
}