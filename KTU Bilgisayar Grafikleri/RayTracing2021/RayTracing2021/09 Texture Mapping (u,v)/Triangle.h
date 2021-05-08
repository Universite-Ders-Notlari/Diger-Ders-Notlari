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

		Triangle(Vertex V0, Vertex V1, Vertex V2, float ambient, float dif, float spec, float refl, float trans, float refrac, Color shapeColor, int tex)
		{
			this->V0         = V0;
			this->V1         = V1;
			this->V2         = V2;
			this->Ambient    = ambient;
			this->Dif        = dif;
			this->Spec       = spec;
			this->Refl       = refl;
			this->Trans      = trans;
			this->Refrac     = refrac;
			this->Tex        = tex;
			this->ShapeColor = shapeColor;
		}

		// http://www.graphics.cornell.edu/pubs/1997/MT97.html
		float Intersect(Vertex Ro, Vertex Rd)
		{
			Vertex edge1, edge2, tvec, pvec, qvec;
			double det, inv_det;
			double u, v;

			edge1 = V1 - V0;
			edge2 = V2 - V0;

			/* begin calculating determinant - also used to calculate U parameter */
			pvec = Rd.CrossProduct(edge2);

			/* if determinant is near zero, ray lies in plane of triangle */
			det = edge1 * pvec;

			if (det > -0.00001F && det < 0.00001F)	return 0;
			//if (det < 0.00001F)	return 0;

			/* calculate distance from vert0 to ray origin */
			tvec = Ro - V0;


			/* calculate U parameter and test bounds */
			u = (tvec * pvec);
			if (u < 0.0 || u > det) return 0;

			/* prepare to test V parameter */
			qvec = tvec.CrossProduct(edge1);

			/* calculate V parameter and test bounds */
			v = (Rd*qvec);
			if (v < 0.0 || u + v > det) return 0;

			inv_det = 1.0 / det;

			/* calculate t, ray intersects triangle */
			return (edge2 * qvec) * (float)inv_det;
		};

		float* IntersectUV(Vertex Ro, Vertex Rd)
		{
			Vertex edge1, edge2, tvec, pvec, qvec;
			double det, inv_det;
			double u, v;

			edge1 = V1 - V0;
			edge2 = V2 - V0;

			/* begin calculating determinant - also used to calculate U parameter */
			pvec = Rd.CrossProduct(edge2);

			/* if determinant is near zero, ray lies in plane of triangle */
			det = edge1 * pvec;

			//if (det > -0.00001F && det < 0.00001F)	return 0;
			if (det < 0.00001F) return NULL;

			/* calculate distance from vert0 to ray origin */
			tvec = Ro - V0;

			/* calculate U parameter and test bounds */
			u = (tvec * pvec);
			if (u < 0.0 || u > det)
				return NULL;

			/* prepare to test V parameter */
			qvec = tvec.CrossProduct(edge1);

			/* calculate V parameter and test bounds */
			v = (Rd * qvec);
			if (v < 0.0 || u + v > det)
				return NULL;

			inv_det = 1.0 / det;

			/* calculate t, ray intersects triangle */
			float* uv = new float[2];
			uv[0] = (float)u * (float)inv_det;
			uv[1] = (float)v * (float)inv_det;
			return uv;
		}

		Vertex NormalAt(Vertex p)
		{
			return (V1-V0).CrossProduct(V2-V1);
		}
	};
}