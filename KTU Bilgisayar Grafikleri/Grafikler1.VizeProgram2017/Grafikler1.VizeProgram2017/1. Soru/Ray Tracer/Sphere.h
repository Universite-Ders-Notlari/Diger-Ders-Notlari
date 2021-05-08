#pragma once

#include "Vertex.h"
#include "Shape.h"
#include <vcclr.h>

namespace RayTracer
{
	using namespace System;
	using namespace System::Drawing;
	using namespace System::Windows::Forms;

	class Sphere : public Shape
	{
		public:
			Vertex Center;
			float Radius;

		Sphere(Vertex Center, float radius, Color shapeColor)
		{
			this->Center     = Center;
			this->Radius     = radius;
			this->ShapeColor = shapeColor;
		}

		float Intersect(Vertex Ro, Vertex Rd)
		{
			Vertex l = Center - Ro; 						//String^ message = "l : ( " + l.X.ToString() + ", " + l.Y.ToString() + ", " + l.Z.ToString() + " )"; 	MessageBox::Show(message);
			float s = l * Rd;								//message = "s : " + s.ToString(); 	MessageBox::Show(message);
			float l2 = l * l;								//message = "l2 : " + l2.ToString(); 	MessageBox::Show(message);
			float r2 = Radius * Radius;						//message = "r2 : " + r2.ToString(); 	MessageBox::Show(message);
			if (s < 0 && l2 > r2) return 0;
			float s2 = s * s;								//message = "s2 : " + s2.ToString(); 	MessageBox::Show(message);
			float m2 = l2 - s2;								//message = "m2 : " + m2.ToString(); 	MessageBox::Show(message);
			if (m2 > r2) return 0;
			float q = (float)Math::Sqrt(r2 - m2);			//message = "q : " + q.ToString(); 	MessageBox::Show(message);
			if (l2 > r2) return s - q;
			else return s + q;
		}
	};
}